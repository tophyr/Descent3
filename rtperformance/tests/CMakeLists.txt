set(CMAKE_FOLDER "tests")

# Run-time profiling is only compiled in on Windows (see USE_RTP in rtperformance.h); on other platforms the whole
# library is a set of no-ops, so there is nothing to test there.
if(WIN32)
  add_executable(rtperformance_tests
    rtperformance_tests.cpp
    support/chrono_timer_env.cpp
    support/rtp_log.cpp
  )
  target_link_libraries(rtperformance_tests PRIVATE
    GTest::gtest_main
    rtperformance
    cfile
    ddio
  )
  gtest_discover_tests(rtperformance_tests)
endif()
