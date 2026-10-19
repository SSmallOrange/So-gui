#include <sogui/app.h>
#include <sogui/sogui.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <cstdio>
#include <print>

namespace SoGui {
namespace backend {

static void GlfwErrorCallback(int error, const char* description) {
    std::println(stderr, "GLFW error {}: {}", error, description);
}

int RunGlfwOpengl3(const AppConfig& config) {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit())
        return 1;

    // macOS 只支持 3.2 Core Profile
#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
#endif

    GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SoGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    if (config.onInit)
        config.onInit();

    int framesToRender = 1;
    while (!glfwWindowShouldClose(window)) {
        if (framesToRender == 0 && !SoGui::ConsumeRedrawRequest()) {
            if (config.idleFps > 0.0f)
                glfwWaitEventsTimeout(1.0f / config.idleFps);
            else {
                glfwWaitEvents();
                framesToRender = 3;
            }
        } else {
            glfwPollEvents();
        }

        if (glfwWindowShouldClose(window))
            break;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        SoGui::NewFrame();
        if (config.onFrame)
            config.onFrame();
        SoGui::EndFrame();
        ImGui::Render();

        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (framesToRender > 0)
            framesToRender--;
    }

    if (config.onShutdown)
        config.onShutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    SoGui::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace backend
} // namespace SoGui
