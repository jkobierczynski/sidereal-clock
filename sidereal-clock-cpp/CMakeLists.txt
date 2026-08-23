cmake_minimum_required(VERSION 3.16)
project(SiderealClock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(SiderealClock
  src/main.cpp
  src/MainWindow.cpp
  src/MainWindow.h
  src/DialWidget.cpp
  src/DialWidget.h
  src/PolarisWidget.cpp
  src/PolarisWidget.h
  src/PlaqueWidget.cpp
  src/PlaqueWidget.h
  src/Astro.cpp
  src/Astro.h
  src/AppState.cpp
  src/AppState.h
  src/SettingsPanel.cpp
  src/SettingsPanel.h
  src/SntpClient.cpp
  src/SntpClient.h
  src/Theme.h
)

target_link_libraries(SiderealClock PRIVATE Qt6::Widgets Qt6::Network)

if(WIN32)
  set_target_properties(SiderealClock PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS SiderealClock RUNTIME DESTINATION bin)
