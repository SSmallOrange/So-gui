add_library(sogui_imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp)
add_library(sogui::imgui ALIAS sogui_imgui)
target_include_directories(sogui_imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(sogui_imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR})
target_compile_definitions(sogui_imgui PUBLIC IMGUI_USER_CONFIG=\"sogui/sogui_config.h\")

# sogui
add_library(sogui INTERFACE)
add_library(sogui::sogui ALIAS sogui)
target_include_directories(sogui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sogui INTERFACE sogui::imgui)

# sogui_imgui_backends
add_library(sogui_imgui_backends STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(sogui_imgui_backends PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(sogui_imgui_backends SYSTEM PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(sogui_imgui_backends PUBLIC sogui::imgui glfw OpenGL::GL)
if(APPLE)
    target_compile_definitions(sogui_imgui_backends PUBLIC GL_SILENCE_DEPRECATION)
endif()

# sogui_app
add_library(sogui_app
    app/app.cc
    app/backends/glfw_opengl3.cc)
add_library(sogui::app ALIAS sogui_app)
target_include_directories(sogui_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sogui_app PUBLIC sogui::sogui sogui_imgui_backends)
sogui_set_warnings(sogui_app)
