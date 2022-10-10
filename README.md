# Voltmeter

Voltmeter is client-server example application for emulating the process of measuring volts on the channels of the device.

Server side application uses Linux [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to interact with clients. Client is [Qt5](https://www.qt.io) based. They use Domain Unix sockets, so server and client have to run on the same machine.  

## Prerequisites

1. Linux OS
2. [CMake](https://cmake.org) and standart c++ developer's compiler (gcc and others) 
3. [Qt5](https://www.qt.io) library
4. Optionaly [Catch2](https://github.com/catchorg/Catch2) to build tests
  
## Build
By default the tests is excluded from the build. 
Set BUILD_TESTS to ON if you whant to build tests. For example cmake  -S. -Bbuild -DBUILD_TESTS=ON 

```ch
git clone https://github.com/0bo084/voltmeter
cd voltmeter
cmake  -S. -Bbuild
cmake --build build
```
## Installation
```ch
cmake --install build
```

## Usage

Simple run voltmeter server

```ch
./bin/voltmetersrv
```
Then run client application Voltcli. Press Ctrl+C to exit

```ch
./bin/voltcli
```

To run test (if you builded project with the -DBUILD_TESTS=ON param):

```ch
./bin/volttests
```

## License
[UNLICENSE](http://unlicense.org/)