#pragma once

#include <memory>

#include <saneengine/application.hpp>

extern sane::Application* createApplication();

int main(int argc, char** argv) {
    createApplication()->run();
    return 0;
}