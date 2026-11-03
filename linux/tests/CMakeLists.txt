set(CMAKE_FOLDER "tests")

# The registry and app database sources are compiled directly (rather than linking the linux library) so that
# ddio_GetPrefPath can be replaced by a stub that points the database at a per-test temporary directory instead of the
# user's real preferences directory.
add_executable(linux_tests
  registry_tests.cpp
  lnxdata_tests.cpp
  support/registry_records.cpp
  support/test_pref_path.cpp
  ../registry.cpp
  ../lnxdata.cpp
)
target_include_directories(linux_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/linux
  ${PROJECT_SOURCE_DIR}/ddio
  ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(linux_tests PRIVATE
  GTest::gtest_main
  ddebug
  logger
  SDL3::SDL3
)
gtest_discover_tests(linux_tests)
