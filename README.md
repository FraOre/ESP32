# ESP32

This project provides a C++ wrapper around Espressif's ESP-IDF framework, simplifying ESP32 development by leveraging object-oriented programming. The goal is to make it easier to create modular, maintainable, and event-driven applications.

## Features

- **Modular Design**: The project is based on ESP-IDF components, promoting clean code separation into distinct modules for better organization and scalability.
- **Event-Driven Architecture**: Simplifies event management, enhancing system responsiveness and providing an efficient way to handle asynchronous tasks such as button presses or sensor data.
- **Object-Oriented**: Focuses on using modern C++ practices to create reusable, easy-to-understand classes that wrap ESP-IDF functionality.

## Project Status

This project is currently in its early stages and is not recommended for production use. It is, however, ideal for prototyping, experimentation, and educational purposes.

## Getting Started

### Prerequisites

Before using this project, ensure you have the following installed on your system:

- ESP-IDF (Espressif IoT Development Framework)
- A C++ compiler compatible with ESP-IDF (GCC for Xtensa or RISC-V based ESP32 microcontrollers)

### Installation

1. Clone the repository:
```sh
git clone https://github.com/FraOre/ESP32.git
```

### Usage

To use the components and features of this project in your application:

**Include the necessary headers** in your source files. For example, to use a custom button handler:

```cpp
#include "Button/Button.h"
```

**Link your project against the provided libraries**, ensuring that the necessary components are included in the CMakeLists.txt or component.mk files.

### License

This project is licensed under the Apache 2.0 License. See the LICENSE file for more details.