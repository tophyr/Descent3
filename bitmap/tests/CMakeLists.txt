set(CMAKE_FOLDER "tests")

add_executable(bitmap_tests
  support/image_builders.cpp
  support/image_builders.h
  support/image_loading.cpp
  support/image_loading.h
  support/test_support.cpp
  support/test_support.h
  bitmain_tests.cpp
  image_loader_tests.cpp
  newbitmap_lightmap_tests.cpp
)
target_link_libraries(bitmap_tests PRIVATE
  GTest::gtest_main
  bitmap
  cfile
  ddebug
  ddio
  logger
  mem
  misc
  SDL3::SDL3
)
gtest_discover_tests(bitmap_tests)
