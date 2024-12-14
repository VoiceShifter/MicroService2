cmake_minimum_required(VERSION 3.14)

project(MicroService03 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Sql)

add_executable(MicroService03
  main.cpp
  Functions.cpp
  Functions.hpp
)
target_link_libraries(MicroService03 Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(MicroService03 Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(MicroService03 Qt${QT_VERSION_MAJOR}::Sql)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Users.sqlite
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include(GNUInstallDirs)
install(TARGETS MicroService03
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
