project(sandbox)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(sandbox src/main.cpp)

# Link against saneengine
target_link_libraries(sandbox PRIVATE saneengine)

# Add include directories
target_include_directories(sandbox PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/saneengine/include
)