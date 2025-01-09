#include <saneengine/entrypoint.hpp>
#include <saneengine/event.hpp>

struct TestEvent {
    int x;
    int y;
    float z;
};

class SandboxApplication : public sane::Application, OBSERVE_EVENT_FIELDS(TestEvent, TestEvent::x) {
public:
    SandboxApplication() : sane::Application("Sandbox", 1280, 720) {
        START_EVENT_FIELDS(TestEvent, TestEvent::x);
    }

    void onUpdate(float deltaTime) override {
        EventEmitter<TestEvent>::emit({ 1, 2, deltaTime });
    }

    void onEvent(const TestEvent& event) override {
        printf("Received event: %d, %d, %f\n", event.x, event.y, event.z);
    }
};

sane::Application* createApplication() {
    return new SandboxApplication();
}
