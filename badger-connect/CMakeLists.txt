project(badger-connect C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(badger-connect badger-connect.cpp)

pico_set_program_name(badger-connect "badger-connect")
pico_set_program_version(badger-connect "0.1")

pico_enable_stdio_uart(badger-connect 0)
pico_enable_stdio_usb(badger-connect 0)

lpbadger_set_linker(badger-connect)

# Add the standard library to the build
target_link_libraries(badger-connect pico_stdlib)

# Add any user requested libraries
target_link_libraries(badger-connect
        lpbadger
        )

pico_add_extra_outputs(badger-connect)

include_directories(../pimoroni-pico ../assets)
