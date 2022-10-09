
# CMakeList.txt : CMake project for templTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(voltmetersrv VERSION 0.1.0)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


# Remove for compiler-specific features
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# clangd completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/source)
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/source/*.cpp")

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(${PROJECT_NAME} ${SOURCES})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)



add_subdirectory("${CMAKE_SOURCE_DIR}/source/Client")

enable_testing()
add_subdirectory(tests)

