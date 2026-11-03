set(CMAKE_FOLDER "tests")

add_executable(2dlib_tests
  surface_tests.cpp
  text_tests.cpp
  support/stubs.cpp
  support/surface.cpp
  support/test_font.cpp
  support/text_draw.cpp
)
target_link_libraries(2dlib_tests PRIVATE
  GTest::gtest_main
  2dlib
  cfile
  ddebug
)

gtest_discover_tests(2dlib_tests)
