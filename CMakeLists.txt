cmake_minimum_required(VERSION 3.27)
project(Drift)

set(CMAKE_CXX_STANDARD 17)

add_executable(Drift main.cpp)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "${CMAKE_CURRENT_LIST_DIR}/SFML-2.6.1/lib/cmake/SFML")
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(SFML/include)
target_link_libraries(Drift sfml-system sfml-window sfml-graphics sfml-audio)