set(CMAKE_FOLDER "tests")

add_executable(grtext_tests
  support/grtext_test_support.h
  support/grtext_test_support.cpp
  support/test_fonts.h
  support/test_fonts.cpp
  support/textaux_test_support.h
  support/textaux_test_support.cpp
  grfont_tests.cpp
  grtext_tests.cpp
  textaux_tests.cpp
)
# grtext needs cfile plus the bitmap and renderer libraries at link time. The latter two pull in most of
# the game, so grtext_test_support.cpp provides small in-memory stand-ins for the handful of functions used.
target_link_libraries(grtext_tests PRIVATE
  GTest::gtest_main
  grtext
  cfile
  ddebug
  ddio
  logger
  mem
)
gtest_discover_tests(grtext_tests)
