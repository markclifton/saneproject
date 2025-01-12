#include "saneengine/application.hpp"
#include "saneengine/ecs/system.hpp"
#include "saneengine/ecs/systemmanager.hpp"
#include "saneengine/layer/layerstack.hpp"
#include "saneengine/window.hpp"
#include <chrono>
#include <thread>

#include <atomic>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "saneengine/layer/layer.hpp"
#include "saneengine/utils/uuid.hpp"

namespace sane {
    class Application::Impl {
    public:
        std::unique_ptr<Window> window;
        std::unique_ptr<LayerStack> layerStack;
        std::unique_ptr<ecs::SystemManager> systemManager;
        std::thread renderThread;
        std::vector<std::unique_ptr<Layer>> pendingLayers;
        std::chrono::steady_clock::time_point lastFrameTime;
        bool running{ true };
    };

    Application::Application(const char* title, uint32_t width, uint32_t height)
        : mImpl(new Impl)
    {
        mImpl->window = std::make_unique<Window>(title, width, height);
        mImpl->layerStack = std::make_unique<LayerStack>();
        mImpl->systemManager = std::make_unique<ecs::SystemManager>();
        mImpl->lastFrameTime = std::chrono::steady_clock::now();

        mImpl->systemManager->startup();
        setupRenderThread();
    }

    Application::~Application() {
        mImpl->systemManager->shutdown();
        delete mImpl;
    }

    void Application::pushLayer(std::unique_ptr<Layer> layer) {
        mImpl->pendingLayers.push_back(std::move(layer));
    }

    void Application::popLayer() {
        mImpl->layerStack->popLayer();
    }

    void Application::run() {
        mainLoop();
    }

    void Application::mainLoop() {
        glfwMakeContextCurrent(nullptr);

        while (!mImpl->window->shouldClose()) {
            glfwPollEvents();
        }

        mImpl->running = false;
        if (mImpl->renderThread.joinable()) {
            mImpl->renderThread.join();
        }
    }

    void Application::setupRenderThread() {
        mImpl->renderThread = std::thread([this]() {
            glfwMakeContextCurrent(mImpl->window->getNativeWindow());
            while (mImpl->running) {
                auto currentTime = std::chrono::steady_clock::now();
                float deltaTime = std::chrono::duration<float>(
                    currentTime - mImpl->lastFrameTime).count();
                mImpl->lastFrameTime = currentTime;

                mImpl->systemManager->update(deltaTime);

                for (const auto& layer : mImpl->layerStack->getLayers()) {
                    layer->onUpdate(deltaTime);
                }

                for (const auto& layer : mImpl->layerStack->getLayers()) {
                    layer->onRender();
                }

                mImpl->window->swapBuffers();
                glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                for (auto& pendingLayer : mImpl->pendingLayers) {
                    mImpl->layerStack->pushLayer(std::move(pendingLayer));
                }
                mImpl->pendingLayers.clear();
            }
            glfwMakeContextCurrent(nullptr);
            });
    }

    utils::UUID Application::startSystem(std::unique_ptr<ecs::System> system) {
        if (!system) {
            throw std::runtime_error("Cannot start null system");
        }
        utils::UUID systemId = utils::generateUUID();
        system->setId(systemId);
        mImpl->systemManager->addSystem(std::move(system));
        return systemId;
    }

    void Application::stopSystem(utils::UUID systemId) {
        mImpl->systemManager->removeSystem(systemId);
    }

    ecs::System* Application::getSystem(utils::UUID systemId) {
        return mImpl->systemManager->getSystem(systemId);
    }

    ecs::SystemManager& Application::getSystemManager() {
        return *mImpl->systemManager;
    }
} // namespace sane