set(CMAKE_FOLDER "tests")

# The renderer library as a whole needs an OpenGL context, so these tests
# compile only the device-independent 3D pipeline sources (frame setup,
# point rotation/projection, clipping, instancing and transform matrices)
# and stub out the few renderer entry points they reference.
add_executable(
  renderer_tests
  support/renderer_checks.cpp
  support/renderer_stubs.cpp
  projection_tests.cpp
  clipper_tests.cpp
  instance_tests.cpp
  ../HardwareClipper.cpp
  ../HardwareGlobalVars.cpp
  ../HardwareInstance.cpp
  ../HardwarePoints.cpp
  ../HardwareSetup.cpp
  ../HardwareTransforms.cpp
)
target_include_directories(renderer_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(
  renderer_tests
  GTest::gtest_main
  vecmat
  ddebug
  logger
  misc
)
gtest_discover_tests(renderer_tests)
