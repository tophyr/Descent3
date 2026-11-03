set(CMAKE_FOLDER "tests")

add_executable(
  fix_tests
  fix_tests.cpp
)
target_link_libraries(
  fix_tests
  GTest::gtest_main
  fix
)
gtest_discover_tests(fix_tests)
