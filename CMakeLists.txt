cmake_minimum_required(VERSION 3.16)
    project(bookget LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Network
        WebEngineWidgets
    )

    add_executable(bookget
        src/main.cpp
        src/MainWindow.h
        src/MainWindow.cpp
        src/HashStore.h
        src/HashStore.cpp
    )

    target_link_libraries(bookget PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::WebEngineWidgets
    )