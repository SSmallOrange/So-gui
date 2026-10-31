#include <sogui/demo/sogui_demo.h>

#include <sogui/core/theme.h>

#include <imgui.h>

namespace SoGui {

namespace {

void ShowThemeSection() {
    ImGui::SeparatorText("Theme");
    const ThemeId theme = GetTheme();
    if (ImGui::RadioButton("Light", theme == ThemeId::Light))
        SetTheme(ThemeId::Light);
    ImGui::SameLine();
    if (ImGui::RadioButton("Dark", theme == ThemeId::Dark))
        SetTheme(ThemeId::Dark);

    ImGui::SeparatorText("Semantic colors");
    for (int i = 0; i < static_cast<int>(Col::COUNT); i++) {
        const Col   col = static_cast<Col>(i);
        const char* name = GetColorName(col);
        ImGui::ColorButton(name, GetColor(col), ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::SameLine();
        ImGui::TextUnformatted(name);
    }
}

} // namespace

void ShowDemoWindow(bool* open) {
    if (!ImGui::Begin("So-gui demo", open)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    ShowThemeSection();
    ImGui::End();
}

} // namespace SoGui
