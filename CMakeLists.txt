cmake_minimum_required(VERSION 3.16)
project(homework2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)

add_executable(main
        src/main.cpp
        src/Detector.cpp
)

target_link_libraries(main
        ${OpenCV_LIBS}
)
