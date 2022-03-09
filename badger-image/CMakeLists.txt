project(badger-image C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(badger-image badger-image.cpp)

pico_set_program_name(badger-image "badger-image")
pico_set_program_version(badger-image "0.1")

pico_enable_stdio_uart(badger-image 0)
pico_enable_stdio_usb(badger-image 0)

lpbadger_set_linker(badger-image)

# Add the standard library to the build
target_link_libraries(badger-image pico_stdlib)

# Add any user requested libraries
target_link_libraries(badger-image
        lpbadger
        )

pico_add_extra_outputs(badger-image)

include_directories(../pimoroni-pico)
