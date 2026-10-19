# imgui
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
file(GLOB_RECURSE SOGUI_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/sogui/**")
add_library(sogui STATIC ${SOGUI_FILES})
add_library(sogui::sogui ALIAS sogui)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${SOGUI_FILES})
target_include_directories(sogui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sogui PUBLIC sogui::imgui)
sogui_set_warnings(sogui)
