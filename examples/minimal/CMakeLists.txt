add_executable(sogui_minimal main.cc)
target_link_libraries(sogui_minimal PRIVATE sogui::app)
target_compile_definitions(sogui_minimal PRIVATE SOGUI_FONT_DIR="${PROJECT_SOURCE_DIR}/assets/fonts")
sogui_set_warnings(sogui_minimal)
