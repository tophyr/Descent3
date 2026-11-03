set(CMAKE_FOLDER "tests")

add_executable(libmve_tests
  decoder8_tests.cpp
  decoder16_tests.cpp
  mve_audio_tests.cpp
  mvelib_tests.cpp
  mveplay_tests.cpp
  support/chrono_timer_env.cpp
)
target_link_libraries(libmve_tests PRIVATE
  GTest::gtest_main
  ddio
  libmve
  SDL3::SDL3
)
gtest_discover_tests(libmve_tests)
