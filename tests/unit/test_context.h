#pragma once

#include <sogui/sogui.h>

#include <utility>

struct ThemeFixture {
    ImGuiContext* previousImGui = ImGui::GetCurrentContext();
    SoGui::Context* previousSoGui = SoGui::GetCurrentContext();
    ImGuiContext* imguiContext = nullptr;
    SoGui::Context* soguiContext = nullptr;
    bool inFrame = false;

    explicit ThemeFixture(const SoGui::ContextConfig& config = {}) {
        imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(imguiContext);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DisplaySize = ImVec2(1280.0f, 960.0f);
        io.DeltaTime = 1.0f / 60.0f;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        soguiContext = SoGui::CreateContext(config);
        SoGui::SetCurrentContext(soguiContext);
    }

    ~ThemeFixture() {
        ImGui::SetCurrentContext(imguiContext);
        SoGui::SetCurrentContext(soguiContext);
        if (inFrame)
            EndFrame();
        SoGui::DestroyContext(soguiContext);
        ImGui::DestroyContext(imguiContext);
        SoGui::SetCurrentContext(previousSoGui);
        ImGui::SetCurrentContext(previousImGui);
    }

    ThemeFixture(const ThemeFixture&) = delete;
    ThemeFixture& operator=(const ThemeFixture&) = delete;

    void BeginFrame() {
        ImGui::NewFrame();
        SoGui::NewFrame();
        inFrame = true;
    }

    void EndFrame() {
        SoGui::EndFrame();
        ImGui::EndFrame();
        inFrame = false;
    }

    // 跑一帧，在固定位置的无标题窗口里执行 fn
    template <typename Fn> void RunFrame(Fn&& fn, ImVec2 windowSize = ImVec2(400.0f, 300.0f)) {
        BeginFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(windowSize);
        ImGui::Begin("Test", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
        std::forward<Fn>(fn)();
        ImGui::End();
        EndFrame();
    }

    // 鼠标和按键事件在下一帧开始时才被处理；ImGui 每帧只处理一次按键变化，按下和松开要分两帧
    void MoveMouse(ImVec2 pos) { ImGui::GetIO().AddMousePosEvent(pos.x, pos.y); }
    void MouseDown() { ImGui::GetIO().AddMouseButtonEvent(0, true); }
    void MouseUp() { ImGui::GetIO().AddMouseButtonEvent(0, false); }
    void PressKey(ImGuiKey key, bool down = true) { ImGui::GetIO().AddKeyEvent(key, down); }
};
