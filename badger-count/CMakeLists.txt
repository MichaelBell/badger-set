project(badger-count C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(badger-count badger-count.cpp)

pico_set_program_name(badger-count "badger-count")
pico_set_program_version(badger-count "0.1")

pico_enable_stdio_uart(badger-count 0)
pico_enable_stdio_usb(badger-count 0)

#set_source_files_properties(badger-count.c PROPERTIES COMPILE_FLAGS -O2)

# Add the standard library to the build
target_link_libraries(badger-count pico_stdlib)

# Add any user requested libraries
target_link_libraries(badger-count
        lpbadger
        )

pico_add_extra_outputs(badger-count)

include_directories(../pimoroni-pico)
