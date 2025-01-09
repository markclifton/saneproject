#pragma once

// STL headers
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#define EXPAND(x) x
#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

#define GET_MACRO(_1,_2,_3,NAME,...) NAME
#define FOR_EACH(what, event, ...) EXPAND(GET_MACRO(__VA_ARGS__, FOR_EACH_3, FOR_EACH_2, FOR_EACH_1)(what, event, __VA_ARGS__))
#define FOR_EACH_1(what, event, x) what(event, x)
#define FOR_EACH_2(what, event, x, ...) what(event, x), FOR_EACH_1(what, event, __VA_ARGS__)
#define FOR_EACH_3(what, event, x, ...) what(event, x), FOR_EACH_2(what, event, __VA_ARGS__)

#define OBSERVE_FIELD(event, field) decltype(event::field) event::*
#define REGISTER_FIELD(event, field) &event::field

#define OBSERVE_EVENT_FIELDS(event, ...) EventFieldSubscriber<event, FOR_EACH(OBSERVE_FIELD, event, __VA_ARGS__)>
#define START_EVENT_FIELDS(event, ...) connectFields(FOR_EACH(REGISTER_FIELD, event, __VA_ARGS__))

template <typename EventType>
class EventSubscriber;

template <typename T>
class EventEmitter
{
public:
    static EventEmitter<T>& getInstance()
    {
        static EventEmitter<T> instance;
        return instance;
    }

    static const T& getData()
    {
        return getInstance().oldData;
    }

    static void emit(const T& data)
    {
        std::unique_lock<std::mutex> lk(getInstance().mDataMutex);
        auto& aSubscribers = getInstance().mSubscribers;
        aSubscribers.erase(std::remove_if(aSubscribers.begin(), aSubscribers.end(), [=](const std::weak_ptr<std::function<void(const T&)>>& wp) {
            if (auto sp = wp.lock())
            {
                (*sp)(data);
            }
            return wp.expired();
            }), aSubscribers.end());

        getInstance().oldData = data;
    }

    static void subscribe(std::shared_ptr<std::function<void(const T&)>> inSubscriber)
    {
        std::unique_lock<std::mutex> lk(getInstance().mDataMutex);
        getInstance().mSubscribers.push_back(inSubscriber);
    }

private:
    std::mutex mDataMutex;
    T oldData;
    std::vector<std::weak_ptr<std::function<void(const T&)>>> mSubscribers;
};

template <typename EventType>
class EventSubscriber
{
public:
    void subscribe(std::shared_ptr<std::function<void(const EventType&)>> inCallback)
    {
        mCallback = inCallback;
        EventEmitter<EventType>::subscribe(mCallback);
    }

    virtual void onEvent(const EventType& inEvent) = 0;
    std::shared_ptr<std::function<void(const EventType&)>> mCallback;
};

template <typename EventType, typename... MemberTypes>
class EventFieldSubscriber : public EventSubscriber<EventType>
{
    std::tuple<MemberTypes...> mMembers;

    template <std::size_t... Indices>
    void onEventInternal(const EventType& newObj, std::index_sequence<Indices...>)
    {
        const EventType& oldObj = EventEmitter<EventType>::getData();
        if (((oldObj.*std::get<Indices>(mMembers) != newObj.*std::get<Indices>(mMembers)) || ...)) {
            this->onEvent(newObj);
        }
    }

public:
    void connectFields(MemberTypes... members)
    {
        EventSubscriber<EventType>::subscribe(std::make_shared<std::function<void(const EventType&)>>([this](const EventType& inData) {
            onEventInternal(inData, std::index_sequence_for<MemberTypes...>{});
            }));
        mMembers = std::tuple<MemberTypes...>{ members... };
    }
};