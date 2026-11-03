set(CMAKE_FOLDER "tests")

add_executable(
  AudioEncode_tests
  support/acm_io.cpp
  support/acm_io.h
  support/signal_gen.cpp
  support/signal_gen.h
  support/test_files.cpp
  support/test_files.h
  audio_encode_tests.cpp
)
target_link_libraries(
  AudioEncode_tests
  GTest::gtest_main
  AudioEncode
  libacm
  logger
)
gtest_discover_tests(AudioEncode_tests)
