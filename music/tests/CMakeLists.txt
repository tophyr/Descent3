set(CMAKE_FOLDER "tests")

add_executable(music_tests
  music_tests.cpp
  support/music_seq_harness.cpp
  support/sdl_assert_counter.cpp
)
target_link_libraries(music_tests PRIVATE
  GTest::gtest_main
  music
  SDL3::SDL3
)

gtest_discover_tests(music_tests)
