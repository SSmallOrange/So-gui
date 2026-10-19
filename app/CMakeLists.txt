# sogui_imgui_backends
add_library(sogui_imgui_backends STATIC
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(sogui_imgui_backends SYSTEM PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(sogui_imgui_backends PUBLIC sogui::imgui glfw OpenGL::GL)
if(APPLE)
    target_compile_definitions(sogui_imgui_backends PUBLIC GL_SILENCE_DEPRECATION)
endif()

# sogui_app
file(GLOB_RECURSE SOGUI_APP_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/sogui/**")
add_library(sogui_app ${SOGUI_APP_FILES})
add_library(sogui::app ALIAS sogui_app)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${SOGUI_APP_FILES})
target_include_directories(sogui_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sogui_app PUBLIC sogui::sogui sogui_imgui_backends)
sogui_set_warnings(sogui_app)
