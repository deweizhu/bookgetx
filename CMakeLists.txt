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
        Gui
)

# Create a GUI application: on Windows use WIN32 to avoid a console window,
# on macOS use MACOSX_BUNDLE to create a .app bundle. On other platforms keep default.
if (WIN32)
  add_executable(bookget WIN32
          src/main.cpp
          src/MainWindow.h
          src/MainWindow.cpp
          src/HashStore.h
          src/HashStore.cpp
  )
elseif(APPLE)
  add_executable(bookget MACOSX_BUNDLE
          src/main.cpp
          src/MainWindow.h
          src/MainWindow.cpp
          src/HashStore.h
          src/HashStore.cpp
  )
else()
  add_executable(bookget
          src/main.cpp
          src/MainWindow.h
          src/MainWindow.cpp
          src/HashStore.h
          src/HashStore.cpp
  )
endif()

target_link_libraries(bookget PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        Qt6::WebEngineWidgets
)