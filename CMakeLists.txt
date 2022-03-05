# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(SKIP_PICO_RUNTIME 1)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(badger-set)

add_subdirectory(pico_runtime)
add_subdirectory(lpbadger)
add_subdirectory(pimoroni-pico)

add_subdirectory(badger-image)
add_subdirectory(badger-count)


