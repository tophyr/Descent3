set(CMAKE_FOLDER "tests")

add_executable(
  unzip_tests
  unzip_tests.cpp
  support/temp_dir.cpp
  support/temp_dir.h
  support/zip_builder.cpp
  support/zip_builder.h
  support/zip_reader.cpp
  support/zip_reader.h
)
target_link_libraries(
  unzip_tests
  GTest::gtest_main
  unzip
  ZLIB::ZLIB
)

gtest_discover_tests(unzip_tests)
