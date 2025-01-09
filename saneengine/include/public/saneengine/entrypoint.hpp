#pragma once

#include <memory>

#include "application.hpp"

extern sane::Application* createApplication();

int main(int argc, char** argv) {
    createApplication()->run();
    return 0;
}