cmake_minimum_required(VERSION 3.12)

project(Voltcli VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)


add_compile_definitions("VOLTMETER_CLIENT")

add_executable( ${PROJECT_NAME}
    dialog.cpp
    main.cpp
    woker_thread.cpp
    
)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)


install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
    
