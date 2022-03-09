add_library(lpbadger INTERFACE)

target_sources(lpbadger INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lpbadger.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lpbclocks.c)

target_include_directories(lpbadger INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(lpbadger INTERFACE badger2040 hardware_flash)

set(LPBADGER_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)
function(lpbadger_set_linker TARGET)
  if (PICO_COPY_TO_RAM)
    set_target_properties(${TARGET} PROPERTIES PICO_TARGET_LINKER_SCRIPT ${LPBADGER_DIR}/lpbadger_copy_to_ram.ld)
    pico_add_link_depend(${TARGET} ${LPBADGER_DIR}/lpbadger_copy_to_ram.ld)
  else()
    set_target_properties(${TARGET} PROPERTIES PICO_TARGET_LINKER_SCRIPT ${LPBADGER_DIR}/lpbadger.ld)
    pico_add_link_depend(${TARGET} ${LPBADGER_DIR}/lpbadger.ld)
  endif()
endfunction()
