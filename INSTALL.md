# Installation

## PlatformIO

To use this library in PlatformIO project, put the following in `platformio.ini` configuration file.

```ini
lib_deps =
    embedded-cpp=https://gitlab.com/vtneil/embedded-cpp.git
```

Make sure your platform supports C++17.
For more information, please visit your platform's documentation
or related forums on setting up C++17 support in your PlatformIO project.

## Arduino IDE

# Usage

## Including the header

This library has the main library header file `embedded_cpp` which includes almost all
basic features into your project. Other than that, there are other modules you can include.
To use basic utilities, put this at the top of your project source/header file.

```c++
#include "embedded_cpp"
```

To use task dispatcher, also include the following.

```c++
#include "dispatcher"
```

For more information, see the Wiki.
