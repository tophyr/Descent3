set(CMAKE_FOLDER "tests")

add_executable(dd_grwin32_tests
  ddgr_dx_tests.cpp
  ddgr_gdi_tests.cpp
  support/dx_surface.cpp
  support/fail_fast.cpp
  support/generic_surface.cpp
  support/locked_pixels.cpp
)
# application.h selects the Win32 application class only when DX_APP is defined.
target_compile_definitions(dd_grwin32_tests PRIVATE DX_APP)
target_include_directories(dd_grwin32_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(dd_grwin32_tests PRIVATE
  GTest::gtest_main
  dd_grwin32
  win32
  ddebug
)
gtest_discover_tests(dd_grwin32_tests)
