add_library(lpbadger INTERFACE)

target_sources(lpbadger INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lpbadger.cpp)

target_include_directories(lpbadger INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(lpbadger INTERFACE badger2040 hardware_flash)
