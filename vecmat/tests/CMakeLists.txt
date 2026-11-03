set(CMAKE_FOLDER "tests")

add_executable(
  vecmat_tests
  vecmat_tests.cpp
  support/vecmat_test_helpers.cpp
)
target_link_libraries(
  vecmat_tests
  GTest::gtest_main
  vecmat
  fix
)
gtest_discover_tests(vecmat_tests)
