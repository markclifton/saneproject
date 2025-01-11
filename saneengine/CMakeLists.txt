cmake_minimum_required(VERSION 3.14)
project(saneengine VERSION 1.0)

include(FetchContent)

# Fetch ImGui first before any targets
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)

FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)

# Fetch GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
set(GLAD_API "gl=3.3" CACHE STRING "API type/version pairs")
set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the binding for")
set(GLAD_INSTALL ON)

# Fetch EnTT
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.12.2
)

FetchContent_MakeAvailable(glfw glm glad EnTT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Configure shared library build
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(BUILD_SHARED_LIBS ON)

# Glob source files
file(GLOB_RECURSE SANEENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE SANEENGINE_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/public/saneengine/*.hpp"
)

file(GLOB_RECURSE SANEENGINE_PRIVATE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/detail/saneengine/*.hpp"
)

# Add library target
add_library(saneengine
    ${SANEENGINE_SOURCES}
    ${SANEENGINE_PUBLIC_HEADERS}
    ${SANEENGINE_PRIVATE_HEADERS}
)

target_include_directories(saneengine
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/detail>/
        ${glad_SOURCE_DIR}/include
)

target_link_libraries(saneengine 
    PUBLIC 
        glfw
        glm
        EnTT::EnTT
        imgui
    PRIVATE
        glad
)

# Set library properties
set_target_properties(saneengine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/saneengine/engine.hpp"
)

# Set compile definitions
target_compile_definitions(saneengine PRIVATE SANEENGINE_EXPORTS)

# Install rules
install(TARGETS saneengine
    EXPORT saneengine-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include/saneengine
)