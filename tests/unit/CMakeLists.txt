add_executable(sogui_unit_tests
    test_main.cc
    test_theme.cc
    test_demo.cc
    test_text.cc
    test_draw.cc
    test_anim.cc
    test_interaction.cc
    test_button.cc
    test_separator.cc
    test_spinner.cc
    test_badge.cc
    test_note.cc
    test_skeleton.cc)
target_link_libraries(sogui_unit_tests PRIVATE sogui::sogui doctest::doctest)
target_compile_definitions(sogui_unit_tests PRIVATE
    SOGUI_TEST_FONT_DIR="${PROJECT_SOURCE_DIR}/assets/fonts/geist"
    SOGUI_TEST_CJK_FONT_DIR="${PROJECT_SOURCE_DIR}/assets/fonts/noto-sans-sc")
sogui_set_warnings(sogui_unit_tests)

include("${PROJECT_SOURCE_DIR}/third_party/doctest/scripts/cmake/doctest.cmake")
doctest_discover_tests(sogui_unit_tests)
