set(CMAKE_FOLDER "tests")

# The ui sources are compiled directly into the test so that the drawing layer (UIDraw.cpp, which needs a renderer)
# and the window manager (UISystem.cpp, which needs an application and renderer) can be replaced by the no-op stubs in
# ui_test_stubs.cpp. Everything tested here is gadget/window state logic, not rendering.
add_executable(ui_tests
  support/chrono_timer_env.cpp
  support/ui_test_stubs.cpp
  support/ui_test_util.cpp
  support/ui_test_util.h
  uiedit_tests.cpp
  uilistbox_tests.cpp
  uiwindow_tests.cpp
  uicombo_slider_tests.cpp
  ../UICombo.cpp
  ../UIEdit.cpp
  ../UIGadget.cpp
  ../UIListBox.cpp
  ../UIObject.cpp
  ../UIRes.cpp
  ../UISlider.cpp
  ../UIWindow.cpp
)
target_include_directories(ui_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/ui
  ${PROJECT_SOURCE_DIR}/grtext
)
target_link_libraries(ui_tests PRIVATE
  GTest::gtest_main
  ddebug
  ddio
  logger
  mem
)
gtest_discover_tests(ui_tests)
