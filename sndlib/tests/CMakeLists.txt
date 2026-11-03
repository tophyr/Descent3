set(CMAKE_FOLDER "tests")

add_executable(sndlib_tests
  mixer_tests.cpp
  soundload_tests.cpp
  wave_load_tests.cpp
  support/mixer_test_support.cpp
  support/sndlib_test_stubs.cpp
  support/soundload_test_support.cpp
  support/wave_load_test_support.cpp
)
target_link_libraries(sndlib_tests PRIVATE
  GTest::gtest_main
  sndlib
  cfile
  ddebug
  ddio
  logger
  mem
  physics
  SDL3::SDL3
)
target_include_directories(sndlib_tests PRIVATE ${PROJECT_SOURCE_DIR}/manage)

gtest_discover_tests(sndlib_tests)
