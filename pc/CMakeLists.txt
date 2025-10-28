cmake_minimum_required(VERSION 3.10)
project(ESP32_AIO_AG_PC_Testing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../src
)

# Add executable
add_executable(autosteer_test
    main.cpp
    autosteer_simulator.cpp
    mock_was.cpp
    mock_motor.cpp
    mock_buttons.cpp
    mock_networking.cpp
    mock_imu.cpp
)

# Add any required libraries (like SDL2 for visualization if needed)
# find_package(SDL2 REQUIRED)
# target_link_libraries(autosteer_test PRIVATE SDL2::SDL2)

# Copy original autosteer library sources (excluding hardware-specific implementations)
add_library(autosteer_lib
    ${CMAKE_SOURCE_DIR}/../src/autosteer/autosteer.cpp
    ${CMAKE_SOURCE_DIR}/../src/autosteer/pid_controller.cpp
    ${CMAKE_SOURCE_DIR}/../src/autosteer/motor.cpp
    ${CMAKE_SOURCE_DIR}/../src/autosteer/was.cpp
    ${CMAKE_SOURCE_DIR}/../src/autosteer/buttons.cpp
    ${CMAKE_SOURCE_DIR}/../src/autosteer/udp_io.cpp
)

target_link_libraries(autosteer_test PRIVATE autosteer_lib) 