set(CMAKE_FOLDER "tests")

# The manage sources are compiled directly into the test (rather than linking
# the manage library) so that only manage's own objects are pulled in; the
# game-side symbols they reference are satisfied by stubs in manage_stubs.cpp.
add_executable(manage_tests
  page_tests.cpp
  pagelock_tests.cpp
  support/manage_page_builders.cpp
  support/manage_page_io.cpp
  support/manage_pagelock_records.cpp
  support/manage_stubs.cpp
  ../doorpage.cpp
  ../gamefilepage.cpp
  ../generic.cpp
  ../manage.cpp
  ../megapage.cpp
  ../pagelock.cpp
  ../shippage.cpp
  ../soundpage.cpp
  ../texpage.cpp
  ../weaponpage.cpp
  ${PROJECT_SOURCE_DIR}/Descent3/args.cpp
)
target_link_libraries(manage_tests PRIVATE
  GTest::gtest_main
  cfile
  ddebug
  ddio
  logger
  mem
)
target_include_directories(manage_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/manage
  $<TARGET_PROPERTY:model,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:sndlib,INTERFACE_INCLUDE_DIRECTORIES>
  $<TARGET_PROPERTY:stream_audio,INTERFACE_INCLUDE_DIRECTORIES>
)
gtest_discover_tests(manage_tests)
