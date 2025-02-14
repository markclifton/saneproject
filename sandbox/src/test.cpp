#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

class ThreadPool
{
public:
    ThreadPool(size_t inNumberThreads = std::thread::hardware_concurrency())
    {
        workers.reserve(inNumberThreads);
        for (size_t i = 0; i < inNumberThreads; ++i)
        {
            workers.emplace_back([this, i] {
                for (;;)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });

                        if (this->stop && this->tasks.empty()) return;

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
                });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();
        for (auto& worker : workers)
            worker.join();
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [func = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return func(std::forward<Args>(args)...);
            }
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) abort(); // ("enqueue on stopped ThreadPool\n");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    inline bool isEmpty() { return tasks.size() == 0; }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop{ false };
};

ThreadPool& getEventThreadPool()
{
    static ThreadPool instance;
    return instance;
}

template <typename EventType>
class EventSubscriberBase;

template <class EventType>
class EventMemberSubscriberBase;

template <typename EventType>
using EventHandler = std::function<void(const EventType&)>;
using EventSubscriberContainer = std::vector<std::pair<std::weak_ptr<void>, uint64_t>>;
using MemberSubscriberContainer = std::vector<std::pair<std::weak_ptr<void>, uint64_t>>;

class ISubscriberBase
{
    uint64_t mUuid{ 0 };

public:
    uint64_t getUUID()
    {
        if (!mUuid)
        {
            static std::mutex sMutex;
            static uint64_t sNextUUID{ 1 };
            std::scoped_lock lk(sMutex);
            mUuid = mUuid ? mUuid : sNextUUID++;
        }
        return mUuid;
    }
};

template<typename EventType>
class EventManager
{
    friend class EventSubscriberBase<EventType>;
    friend class EventMemberSubscriberBase<EventType>;

    struct EventInternalData
    {
        std::recursive_mutex eventMutex;
        std::queue<std::pair<std::function<void()>, std::promise<void>>> eventQueue;
        std::optional<std::thread::id> processingThreadId;

        EventSubscriberContainer changeSubscribers;
        EventSubscriberContainer eventSubscribers;
        std::unordered_map<std::type_index, MemberSubscriberContainer> changeMemberSubscribers;
        std::unordered_map<std::type_index, MemberSubscriberContainer> memberSubscribers;

        std::optional<EventType> currentState;
    };

    struct MemberInfoBase
    {
        virtual ~MemberInfoBase() = default;
        virtual bool compare(
            const void* inLhs,
            const void* inRhs) = 0;
        virtual void process(
            uint64_t inUUID,
            const void* inEventData,
            EventInternalData& inInternalData,
            bool inEventDataChanged) = 0;
    };

    template<typename MemberType>
    struct MemberInfo : MemberInfoBase
    {
        MemberType EventType::* memberPointer;

        MemberInfo(
            MemberType EventType::* inMemberPointer)
            : memberPointer(inMemberPointer)
        {
        }

        bool compare(
            const void* inLhs,
            const void* inRhs) override
        {
            const auto lhsEvent = static_cast<const EventType*>(inLhs);
            const auto rhsEvent = static_cast<const EventType*>(inRhs);
            return lhsEvent->*memberPointer != rhsEvent->*memberPointer;
        }

        void process(
            uint64_t inSenderUUID,
            const void* inEventData,
            EventInternalData& inEventInternalData,
            bool inMemberChanged) override
        {
            auto aIncomingEventData = static_cast<const EventType*>(inEventData);
            if (inEventInternalData.currentState.has_value())
            {
                getInstance().notifyMemberSubscribers(
                    inSenderUUID,
                    memberPointer,
                    aIncomingEventData->*memberPointer,
                    inEventInternalData,
                    inMemberChanged);

                auto& aEventStateData = inEventInternalData.currentState.value();
                aEventStateData.*memberPointer = aIncomingEventData->*memberPointer;
            }
        }
    };

public:
    template<class... EventMembers>
    static void registerEventMembers(
        EventMembers... inEventMembers)
    {
        (getInstance().registerMember(inEventMembers), ...);
    }

    static void emit(
        const std::string& inTopicName,
        const void* inSender,
        const EventType& inEvent)
    {
        auto& aInstance = getInstance();
        auto aInternalDataOpt = getEventInternalData(inTopicName);
        if (aInternalDataOpt.has_value())
        {
            auto& aInternalData = aInternalDataOpt.value().get();
            auto aFuture = aInstance.queueEvent(inSender, inEvent, aInternalData);
            if (!aInstance.tryToProcessEvents(aInternalData))
            {
                aFuture.get();
            }
        }
    }

    static std::future<void> emitFromThreadpool(
        const std::string& inTopicName,
        const void* inSender,
        const EventType& inEvent)
    {
        auto& aInstance = getInstance();
        auto aInternalDataOpt = getEventInternalData(inTopicName);
        if (aInternalDataOpt.has_value())
        {
            auto& aInternalData = aInternalDataOpt.value().get();
            auto aFuture = aInstance.queueEvent(inSender, inEvent, aInternalData);
            aInstance.tryToProcessEventsViaThreadpool(aInternalData);
            return aFuture;
        }
        return getCompletedFuture();
    }

    template<class... EventMembers>
    static void emitMembers(
        const std::string& inTopicName,
        const void* inSender,
        const EventType& inEvent,
        EventMembers... inMembers)
    {
        auto& aInstance = getInstance();
        auto aInternalDataOpt = getEventInternalData(inTopicName);
        if (aInternalDataOpt.has_value())
        {
            auto& aInternalData = aInternalDataOpt.value().get();
            auto aFuture = aInstance.queueMemberEvents(
                inSender,
                inEvent,
                aInternalData,
                inMembers...);
            if (!aInstance.tryToProcessEvents(aInternalData))
            {
                aFuture.get();
            }
        }
    }

    template<class... EventMembers>
    static std::future<void> emitMembersFromThreadpool(
        const std::string& inTopicName,
        const void* inSender,
        const EventType& inEvent,
        EventMembers... inMembers)
    {
        auto& aInstance = getInstance();
        auto aInternalDataOpt = getEventInternalData(inTopicName);
        if (aInternalDataOpt.has_value())
        {
            auto& aInternalData = aInternalDataOpt.value().get();
            auto aFuture = aInstance.queueMemberEvents(
                inSender,
                inEvent,
                aInternalData,
                inMembers...);
            aInstance.tryToProcessEventsViaThreadpool(aInternalData);
            return aFuture;
        }
        return getCompletedFuture();
    }

    static void setInitialState(
        const std::string& inTopicName,
        const EventType& inInitialState)
    {
        auto aInternalDataOpt = getEventInternalData(inTopicName, true);
        auto& aInternalData = aInternalDataOpt.value().get();

        std::scoped_lock lk(aInternalData.eventMutex);
        aInternalData.currentState = inInitialState;
    }

    static std::optional<EventType> getCurrentState(
        const std::string& inTopicName)
    {
        auto aInternalDataOpt = getEventInternalData(inTopicName);
        if (aInternalDataOpt.has_value())
        {
            auto& aInternalData = aInternalDataOpt.value().get();
            std::scoped_lock lk(aInternalData.eventMutex);
            return aInternalData.currentState;
        }
        return std::nullopt;
    }

private:
    EventManager()
    {
        static_assert(std::is_assignable<EventType&, EventType>::value,
            "EventType must be assignable");
    }

    static EventManager& getInstance()
    {
        static EventManager sInstance;
        return sInstance;
    }

    static std::optional<std::reference_wrapper<EventInternalData>> getEventInternalData(
        const std::string& inTopicName,
        bool inCreateIfEmpty = false)
    {

        std::scoped_lock lk(getInstance().mEventDataMapMutex);
        auto& aDataMap = getInstance().mEventDataMap;

        auto aIt = std::find_if(
            aDataMap.begin(),
            aDataMap.end(),
            [&inTopicName](const auto& inPair)
            {
                return inPair.first == inTopicName;
            });

        if (aIt != aDataMap.end())
        {
            return { *aIt->second };
        }
        else if (inCreateIfEmpty)
        {
            aDataMap.emplace_back(
                inTopicName,
                std::make_unique<EventInternalData>());
            return { *aDataMap.back().second };
        }
        return std::nullopt;
    }

    static void subscribeToEvent(
        const std::string& inTopicName,
        uint64_t inSubscriberUUID,
        std::shared_ptr<EventHandler<EventType>> inSubscriber,
        bool inNotifyOnChangeOnly)
    {
        auto aInternalDataOpt = getEventInternalData(inTopicName, true);
        auto& aInternalData = aInternalDataOpt.value().get();

        std::scoped_lock lk(aInternalData.eventMutex);
        inNotifyOnChangeOnly
            ? aInternalData.changeSubscribers.emplace_back(inSubscriber, inSubscriberUUID)
            : aInternalData.eventSubscribers.emplace_back(inSubscriber, inSubscriberUUID);
    }

    template<typename MemberType>
    static void subscribeToMember(
        const std::string& inTopicName,
        uint64_t inSubscriberUUID,
        std::shared_ptr<EventHandler<MemberType>> inSubscriber,
        MemberType EventType::* inMemberPointer,
        bool inNotifyOnChangeOnly)
    {
        auto aInternalDataOpt = getEventInternalData(inTopicName, true);
        auto& aInternalData = aInternalDataOpt.value().get();

        std::scoped_lock lk(aInternalData.eventMutex);
        inNotifyOnChangeOnly
            ? aInternalData.changeMemberSubscribers[std::type_index(typeid(MemberType))].emplace_back(inSubscriber, inSubscriberUUID)
            : aInternalData.memberSubscribers[std::type_index(typeid(MemberType))].emplace_back(inSubscriber, inSubscriberUUID);
    }

    uint64_t getSubscriberUUID(const void* inSubscriber)
    {
        if (auto aSubscriberBase = const_cast<EventSubscriberBase<EventType>*>(
            static_cast<const EventSubscriberBase<EventType>*>(inSubscriber)))
        {
            return aSubscriberBase->getUUID();
        }
        return 0;
    }

    uint64_t getMemberSubscriberUUID(const void* inSubscriber)
    {
        if (auto aSubscriberBase = const_cast<EventMemberSubscriberBase<EventType>*>(
            static_cast<const EventMemberSubscriberBase<EventType>*>(inSubscriber)))
        {
            return aSubscriberBase->getUUID();
        }
        return 0;
    }

    std::future<void> queueEvent(
        const void* inSender,
        const EventType& inEvent,
        EventInternalData& inInternalData)
    {
        auto aSenderUUID = getSubscriberUUID(inSender);
        std::scoped_lock lk(inInternalData.eventMutex);

        std::promise<void> aPromise;
        auto aFuture = aPromise.get_future();

        inInternalData.eventQueue.emplace(
            [aSenderUUID, inEvent, &inInternalData]() mutable
            {
                std::scoped_lock lk(inInternalData.eventMutex);
                getInstance().emitEventCommon(aSenderUUID, inEvent, inInternalData);
            },
            std::move(aPromise));

        return aFuture;
    }

    template<class... EventMembers>
    std::future<void> queueMemberEvents(
        const void* inSender,
        const EventType& inEvent,
        EventInternalData& inInternalData,
        EventMembers... inEventMembers)
    {
        auto aSenderUUID = getMemberSubscriberUUID(inSender);
        std::scoped_lock lk(inInternalData.eventMutex);

        inInternalData.eventQueue.emplace(
            [aSenderUUID, inEvent, &inInternalData, inEventMembers...]() mutable
            {
                auto& aInstance = getInstance();
                std::scoped_lock lk(inInternalData.eventMutex);

                bool aChanged = false;
                if (inInternalData.currentState.has_value())
                {
                    EventType& aCurrentState = inInternalData.currentState.value();
                    (aInstance.emitEventMemberHelper(
                        aSenderUUID,
                        inEvent,
                        inEventMembers,
                        inInternalData,
                        aChanged), ...);
                }
                else
                {
                    aChanged = true;
                    (aInstance.notifyMemberSubscribers(
                        aSenderUUID,
                        inEventMembers,
                        inEvent.*inEventMembers,
                        inInternalData,
                        true), ...);
                }

                aInstance.notifyEventSubscribers(
                    aSenderUUID,
                    inInternalData.currentState.value(),
                    inInternalData,
                    aChanged);
            },
            std::promise<void>());

        return inInternalData.eventQueue.back().second.get_future();
    }

    void emitEventCommon(
        uint64_t inSenderUUID,
        const EventType& inEvent,
        EventInternalData& inInternalData)
    {
        auto& aInstance = getInstance();
        bool aChanged = aInstance.mEventMembers.empty();
        if (inInternalData.currentState.has_value())
        {
            EventType& aCurrentState = inInternalData.currentState.value();

            for (const auto& memberInfo : aInstance.mEventMembers)
            {
                auto aMemberChanged = memberInfo->compare(&aCurrentState, &inEvent);
                aChanged |= aMemberChanged;
                memberInfo->process(inSenderUUID, &inEvent, inInternalData, aMemberChanged);
            }
        }
        else
        {
            for (const auto& memberInfo : aInstance.mEventMembers)
            {
                memberInfo->process(inSenderUUID, &inEvent, inInternalData, true);
            }
        }

        notifyEventSubscribers(inSenderUUID, inEvent, inInternalData, aChanged);
        inInternalData.currentState = inEvent;
    }

    template<typename MemberType>
    void emitEventMemberHelper(
        uint64_t inSenderUUID,
        const EventType& inEvent,
        MemberType EventType::* inMemberPointer,
        EventInternalData& inInternalData,
        bool& inChanged)
    {
        auto aMemberChanged = inInternalData.currentState.has_value() ?
            inInternalData.currentState.value().*inMemberPointer != inEvent.*inMemberPointer :
            true;
        inChanged |= aMemberChanged;
        notifyMemberSubscribers(
            inSenderUUID,
            inMemberPointer,
            inEvent.*inMemberPointer,
            inInternalData,
            aMemberChanged);

        if (aMemberChanged)
        {
            inInternalData.currentState.value().*inMemberPointer = inEvent.*inMemberPointer;
        }
    }

    template <typename MemberType>
    void registerMember(MemberType EventType::* inMemberPointer)
    {
        mEventMembers.emplace_back(std::make_unique<MemberInfo<MemberType>>(inMemberPointer));
    }

    void notifyEventSubscribers(
        uint64_t inSenderUUID,
        const EventType& inEvent,
        EventInternalData& inInternalData,
        bool inChanged)
    {
        for (auto aSubscribers : { inChanged ?
            inInternalData.changeSubscribers : EventSubscriberContainer(),
            inInternalData.eventSubscribers })
        {
            aSubscribers.erase(
                std::remove_if(
                    aSubscribers.begin(),
                    aSubscribers.end(),
                    [&](const auto& inSubscriber)
                    {
                        if (auto aSubscriber = inSubscriber.first.lock())
                        {
                            if (!inSubscriber.second || inSenderUUID != inSubscriber.second)
                            {
                                auto aEventHandler = std::static_pointer_cast<EventHandler<EventType>>(aSubscriber);
                                (*aEventHandler)(inEvent);
                            }
                            return false;
                        }
                        return true;
                    }),
                aSubscribers.end());
        }
    }

    template<typename MemberType>
    void notifyMemberSubscribers(
        uint64_t inSenderUUID,
        MemberType EventType::* inMemberPointer,
        const MemberType& inMemberValue,
        EventInternalData& inInternalData,
        bool inChanged)
    {
        for (auto aSubscribers : { (inChanged ?
            inInternalData.changeMemberSubscribers[std::type_index(typeid(MemberType))] : MemberSubscriberContainer()),
            inInternalData.memberSubscribers[std::type_index(typeid(MemberType))] })
        {
            aSubscribers.erase(
                std::remove_if(
                    aSubscribers.begin(),
                    aSubscribers.end(),
                    [&](const auto& inSubscriber)
                    {
                        if (auto aSubscriber = inSubscriber.first.lock())
                        {
                            if (!inSubscriber.second || inSenderUUID != inSubscriber.second)
                            {
                                auto aEventHandler = std::static_pointer_cast<EventHandler<MemberType>>(aSubscriber);
                                (*aEventHandler)(inMemberValue);
                            }
                            return false;
                        }
                        return true;
                    }),
                aSubscribers.end());
        }
    }

    bool tryToProcessEvents(EventInternalData& inInternalData)
    {
        std::unique_lock lk(inInternalData.eventMutex);
        if (!inInternalData.processingThreadId.has_value())
        {
            inInternalData.processingThreadId = std::this_thread::get_id();
            lk.unlock();

            processEvents(inInternalData);
            return true;
        }
        return inInternalData.processingThreadId == std::this_thread::get_id();
    }

    void tryToProcessEventsViaThreadpool(EventInternalData& inInternalData)
    {
        {
            std::scoped_lock lk(inInternalData.eventMutex);
            if (inInternalData.processingThreadId.has_value()
                && inInternalData.processingThreadId.value() != std::this_thread::get_id())
            {
                return;
            }
        }

        auto& aThreadPool = getEventThreadPool();
        aThreadPool.enqueue([this, &inInternalData]
            {
                std::unique_lock lk(inInternalData.eventMutex);
                if (!inInternalData.processingThreadId.has_value())
                {
                    inInternalData.processingThreadId = std::this_thread::get_id();
                    lk.unlock();

                    processEvents(inInternalData);
                }
            });
    }

    void processEvents(EventInternalData& inInternalData)
    {
        std::unique_lock lk(inInternalData.eventMutex);
        while (!inInternalData.eventQueue.empty())
        {
            auto [aHandler, aPromise] = std::move(inInternalData.eventQueue.front());
            inInternalData.eventQueue.pop();
            lk.unlock();

            aHandler();
            aPromise.set_value();
            lk.lock();
        }
        inInternalData.processingThreadId.reset();
    }

    static std::future<void> getCompletedFuture()
    {
        std::promise<void> aPromise;
        aPromise.set_value();
        return aPromise.get_future();
    }

    std::mutex mEventDataMapMutex;
    std::list<std::pair<std::string, std::unique_ptr<EventInternalData>>> mEventDataMap;
    std::vector<std::unique_ptr<MemberInfoBase>> mEventMembers;
};

template<typename EventType>
class EventEmitter
{
public:
    static void emit(
        const EventType& inEvent,
        const void* inSender = nullptr,
        const std::string& inTopicName = "")
    {
        EventManager<EventType>::emit(inTopicName, inSender, inEvent);
    }

    static std::future<void> emitFromThreadpool(
        const EventType& inEvent,
        const void* inSender = nullptr,
        const std::string& inTopicName = "")
    {
        return EventManager<EventType>::emitFromThreadpool(inTopicName, inSender, inEvent);
    }

    template<class... EventMembers>
    static void emitMembers(
        const EventType& inEvent,
        const void* inSender,
        const std::string& inTopicName,
        EventMembers... inEventMembers)
    {
        EventManager<EventType>::emitMembers(inTopicName, inSender, inEvent, inEventMembers...);
    }

    template<class... EventMembers>
    static std::future<void> emitMembersFromThreadpool(
        const EventType& inEvent,
        const void* inSender,
        const std::string& inTopicName,
        EventMembers... inEventMembers)
    {
        return EventManager<EventType>::emitMembersFromThreadpool(inTopicName, inSender, inEvent, inEventMembers...);
    }
};

template<typename EventType>
class EventSubscriberBase : public ISubscriberBase
{
    std::shared_ptr<EventHandler<EventType>> mEventHandler;

public:
    EventSubscriberBase(const std::string& inTopicName, bool inNotifyOnChangeOnly)
        : mEventHandler(std::make_shared<EventHandler<EventType>>([this](const EventType& inEvent)
            {
                onEvent(inEvent);
            }))
    {
        EventManager<EventType>::subscribeToEvent(
            inTopicName,
            getUUID(),
            mEventHandler,
            inNotifyOnChangeOnly);
    }

    virtual void onEvent(const EventType& inEvent) = 0;
};

template<class... EventTypes>
class EventSubscriber : public EventSubscriberBase<EventTypes>...
{
public:
    EventSubscriber(const std::string& inTopicName = "", bool inNotifyOnChangeOnly = true)
        : EventSubscriberBase<EventTypes>(inTopicName, inNotifyOnChangeOnly)...
    {
    }
};

template<class EventType>
class EventMemberSubscriberBase : public ISubscriberBase
{
    const std::string mKey;
    std::vector<std::shared_ptr<void>> mEventHandlers;
public:
    EventMemberSubscriberBase(const std::string& inTopicName = "")
        : mKey(inTopicName)
    {
    }

    template<typename MemberType>
    void subscribeToMember(
        MemberType EventType::* inMemberPointer,
        std::function<void(const MemberType&)> inMemberChanged,
        bool inNotifyOnChangeOnly = true)
    {
        auto aEventHandler = std::make_shared<EventHandler<MemberType>>(inMemberChanged);
        mEventHandlers.push_back(aEventHandler);

        EventManager<EventType>::subscribeToMember(
            mKey,
            getUUID(),
            aEventHandler,
            inMemberPointer,
            inNotifyOnChangeOnly);
    }
};

template<class...  MemberTypes>
class EventMemberSubscriber : public EventMemberSubscriberBase<MemberTypes>...
{
public:
    EventMemberSubscriber(const std::string& inTopicName = "")
        : EventMemberSubscriberBase<MemberTypes>(inTopicName)...
    {
    }

    template<typename EventType, typename MemberType>
    void subscribeToMember(
        MemberType EventType::* inMemberPointer,
        std::function<void(const MemberType&)> inMemberChanged,
        bool inNotifyOnChangeOnly = true)
    {
        EventMemberSubscriberBase<EventType>::subscribeToMember(
            inMemberPointer,
            inMemberChanged,
            inNotifyOnChangeOnly);
    }
};

// Test event structure
struct TestEvent {
    int a{ 0 };
    float b{ 0.0f };
};

class TestEventSubscriber : EventSubscriber<TestEvent> {
public:
    TestEventSubscriber()
        : EventSubscriber<TestEvent>("TestTopic") {}

    void onEvent(const TestEvent& event) override {
        std::cout << "Event updated with value: " << event.a << ", " << event.b << std::endl;
    }
};

class TestEventMemberSubscriber : public EventMemberSubscriber<TestEvent> {
public:
    TestEventMemberSubscriber()
        : EventMemberSubscriber<TestEvent>("TestTopic") {
        subscribeToMember<TestEvent, int>(&TestEvent::a, [](const auto& value) {
            std::cout << "Member a updated with value: " << value << std::endl;
            });

        subscribeToMember<TestEvent, float>(&TestEvent::b, [this](const auto& value) {
            std::cout << "Member b updated with value: " << value << std::endl;
            EventEmitter<TestEvent>::emitMembersFromThreadpool({ 0, value + 1.f }, this, "TestTopic", &TestEvent::b);
            });
    }
};

int main() {
    EventManager<TestEvent>::registerEventMembers(&TestEvent::a, &TestEvent::b);

    TestEventSubscriber subscriber;
    TestEventMemberSubscriber memberSubscriber;

    EventManager<TestEvent>::setInitialState("TestTopic", { 0, 0.0 });
    EventEmitter<TestEvent>::emitFromThreadpool({ 1, 1.0 }, nullptr, "TestTopic").wait();
    // EventEmitter<TestEvent>::emitFromThreadpool({ 1, 1.0 }, nullptr, "TestTopic");
    // EventEmitter<TestEvent>::emitMembersFromThreadpool({ 1, 1.0 }, nullptr, "TestTopic", &TestEvent::a);
    // EventEmitter<TestEvent>::emitMembersFromThreadpool({ 2, 2.0 }, nullptr, "TestTopic", &TestEvent::b);
    // EventEmitter<TestEvent>::emitMembersFromThreadpool({ 3, 2.0 }, nullptr, "TestTopic", &TestEvent::a, &TestEvent::b);
    // EventEmitter<TestEvent>::emitMembersFromThreadpool({ 4, 3.0 }, nullptr, "TestTopic", &TestEvent::a, &TestEvent::b);
    EventEmitter<TestEvent>::emitMembersFromThreadpool({ 5, 4.0 }, nullptr, "TestTopic", &TestEvent::a, &TestEvent::b).wait();

    return 0;
}
