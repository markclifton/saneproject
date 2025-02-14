project(sandbox)
cmake_minimum_required(VERSION 3.10)

# Add executable
add_executable(sandbox src/test.cpp)

# Link against saneengine
target_link_libraries(sandbox PRIVATE saneengine)

# Add include directories
target_include_directories(sandbox PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/saneengine/include
)