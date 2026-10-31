add_executable(sogui_unit_tests
    test_main.cc
    test_theme.cc
    test_demo.cc)
target_link_libraries(sogui_unit_tests PRIVATE sogui::sogui doctest::doctest)
sogui_set_warnings(sogui_unit_tests)

include("${PROJECT_SOURCE_DIR}/third_party/doctest/scripts/cmake/doctest.cmake")
doctest_discover_tests(sogui_unit_tests)
