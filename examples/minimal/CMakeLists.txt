add_executable(sogui_minimal main.cc)
target_link_libraries(sogui_minimal PRIVATE sogui::app)
sogui_set_warnings(sogui_minimal)
