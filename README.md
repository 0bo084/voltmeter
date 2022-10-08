# Voltmeter

Voltmeter is client-server example application for emulating the process of measuring volts on the channels of the device.

Server side application uses Linux [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to interact with clients. Client is [Qt5](https://www.qt.io) based. They use Domain Unix sockets, so server and client have to run on the same machine.  

## Prerequisites

1. Linux OS
2. [CMake](https://cmake.org) and standart c++ developer's compiler (gcc and others) 
3. [Qt5](https://www.qt.io) library
  
## Build


```ch
git clone https://github.com/0bo084/voltmeter
cd voltmeter
cmake --build .
```
## Installation
```ch
cmake --install .
```

## Usage

Simple run voltmeter server

```ch
./bin/voltmeter
```
Then run client application Voltcli

```ch
./source/Client/bin/voltcli
```

## License
[UNLICENSE](http://unlicense.org/)