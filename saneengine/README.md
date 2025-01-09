# SaneEngine

SaneEngine is a lightweight C++ shared library designed for game engine development. It provides a simple interface for initializing, running, and shutting down an engine.

## Features

- Easy-to-use `Engine` class with methods for lifecycle management.
- Modular design for easy integration into existing projects.

## Installation

To build the library, ensure you have CMake installed. Clone the repository and run the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Include the engine header in your project:

```cpp
#include <saneengine/engine.hpp>
```

Create an instance of the `Engine` class and use its methods:

```cpp
Engine engine;
engine.initialize();
engine.run();
engine.shutdown();
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.