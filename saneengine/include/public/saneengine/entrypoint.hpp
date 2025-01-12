#pragma once

#include <memory>

#include <saneengine/application.hpp>

extern std::unique_ptr<sane::Application> createApplication();

int main(int argc, char** argv) {
    createApplication()->run();
    return 0;
}