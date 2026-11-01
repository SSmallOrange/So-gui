add_executable(sogui_unit_tests
    test_main.cc
    test_theme.cc
    test_demo.cc
    test_text.cc)
target_link_libraries(sogui_unit_tests PRIVATE sogui::sogui doctest::doctest)
target_compile_definitions(sogui_unit_tests PRIVATE SOGUI_TEST_FONT_DIR="${PROJECT_SOURCE_DIR}/assets/fonts/geist")
sogui_set_warnings(sogui_unit_tests)

include("${PROJECT_SOURCE_DIR}/third_party/doctest/scripts/cmake/doctest.cmake")
doctest_discover_tests(sogui_unit_tests)
