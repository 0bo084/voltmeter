# CMakeList.txt : CMake project for templTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(volttests VERSION 0.1.0)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


# Remove for compiler-specific features
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# clangd completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
include_directories(${CMAKE_SOURCE_DIR}/tests)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

add_compile_options(-Wall -Wextra -Wpedantic -Werror)


add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})


install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# включаем CTest
#enable_testing()
# в качестве фреймворка для тестирования используем GoogleTest
# find_package(GTest REQUIRED)
#add_subdirectory(Test)

