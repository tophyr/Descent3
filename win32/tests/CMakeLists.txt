set(CMAKE_FOLDER "tests")

add_executable(
  win32_tests
  windata_tests.cpp
)
# DX_APP selects the Win32 implementation in AppDatabase.h, matching how the
# win32 library itself is compiled.
target_compile_definitions(win32_tests PRIVATE DX_APP)
target_link_libraries(
  win32_tests
  GTest::gtest_main
  win32
)
gtest_discover_tests(win32_tests)
