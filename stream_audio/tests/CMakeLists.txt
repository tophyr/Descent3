set(CMAKE_FOLDER "tests")

add_executable(stream_audio_tests
  osfarchive_tests.cpp
  audiostream_tests.cpp
  support/chrono_timer_env.cpp
  support/fake_sound.h
  support/osf_writer.cpp
  support/osf_writer.h
  support/test_helpers.h
  support/test_stream.cpp
  support/test_stream.h
)
target_link_libraries(stream_audio_tests PRIVATE
  GTest::gtest_main
  stream_audio
  # sndlib provides ssl_lib.h and the llsSystem base class used by the fake sound system
  sndlib
  cfile
  ddio
  mem
)
gtest_discover_tests(stream_audio_tests)
