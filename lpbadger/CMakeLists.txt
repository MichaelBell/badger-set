add_library(lpbadger INTERFACE)

target_sources(lpbadger INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lpbadger.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lpbclocks.c)

target_include_directories(lpbadger INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(lpbadger INTERFACE badger2040 hardware_flash)
