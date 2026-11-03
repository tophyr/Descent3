set(CMAKE_FOLDER "tests")

# The editor executable cannot be linked into a test, so the editor/game globals
# and functions these sources reference are provided by editor_test_stubs.cpp.
add_executable(editor_tests
  room_geometry_tests.cpp
  support/editor_test_stubs.cpp
  support/room_test_expect.cpp
  support/room_test_rooms.cpp
  support/room_test_tempdir.cpp
  ../Erooms.cpp
  ../RoomUVs.cpp
  ../../Descent3/room.cpp
)
target_compile_definitions(editor_tests PRIVATE EDITOR)
target_include_directories(editor_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/editor
  ${PROJECT_SOURCE_DIR}/Descent3
  ${PROJECT_SOURCE_DIR}/lib
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/manage
  ${PROJECT_SOURCE_DIR}/physics
  ${PROJECT_SOURCE_DIR}/2dlib
  ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(editor_tests PRIVATE
  GTest::gtest_main
  cfile
  ddebug
  logger
  mem
  misc
  vecmat
)
gtest_discover_tests(editor_tests)
