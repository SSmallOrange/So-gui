#include <sogui/demo/sogui_demo.h>

#include <sogui/core/theme.h>

#include <imgui.h>

#include <array>
#include <cstdio>

namespace SoGui {

namespace {

std::array<char, 10> ColorHex(const ImVec4& color) {
    const ImU32 packed = ImGui::ColorConvertFloat4ToU32(color);
    std::array<char, 10> text{};
    std::snprintf(text.data(), text.size(), "#%02X%02X%02X%02X", (packed >> IM_COL32_R_SHIFT) & 0xFFU,
                  (packed >> IM_COL32_G_SHIFT) & 0xFFU, (packed >> IM_COL32_B_SHIFT) & 0xFFU,
                  (packed >> IM_COL32_A_SHIFT) & 0xFFU);
    return text;
}

void ShowSwatch(const char* name, const ImVec4& color, int level = 0) {
    ImGui::ColorButton(name, color, ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip,
                       ImVec2(ImGui::GetFrameHeight() * 1.5f, ImGui::GetFrameHeight()));
    if (ImGui::BeginItemTooltip()) {
        if (level > 0)
            ImGui::Text("%s-%d", name, level);
        else
            ImGui::TextUnformatted(name);
        ImGui::Text("%s (RGBA)", ColorHex(color).data());
        ImGui::EndTooltip();
    }
}

void ShowThemeSection() {
    ImGui::SeparatorText("Theme");
    const ThemeId theme = GetTheme();
    if (ImGui::RadioButton("Light", theme == ThemeId::Light))
        SetTheme(ThemeId::Light);
    ImGui::SameLine();
    if (ImGui::RadioButton("Dark", theme == ThemeId::Dark))
        SetTheme(ThemeId::Dark);
}

void ShowPaletteSection() {
    ImGui::SeparatorText("Palette");
    ShowSwatch("Background 1", GetColor(Col::Bg1));
    ImGui::SameLine();
    ImGui::TextUnformatted("Background 1");
    ImGui::SameLine();
    ShowSwatch("Background 2", GetColor(Col::Bg2));
    ImGui::SameLine();
    ImGui::TextUnformatted("Background 2");

    constexpr std::array names = {"gray", "gray-alpha", "blue", "red", "amber", "green", "teal", "purple", "pink"};
    static_assert(names.size() == static_cast<std::size_t>(Hue::COUNT));
    const ImGuiStyle& style = ImGui::GetStyle();
    const float tableHeight =
        ImGui::GetTextLineHeight() + 2.0f * style.CellPadding.y +
        static_cast<float>(names.size()) * (ImGui::GetFrameHeight() + 2.0f * style.CellPadding.y) + style.ScrollbarSize;
    if (ImGui::BeginTable("Palette", 11, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX,
                          ImVec2(0.0f, tableHeight))) {
        ImGui::TableSetupColumn("Hue");
        for (int level = 1; level <= 10; ++level) {
            std::array<char, 3> label{};
            std::snprintf(label.data(), label.size(), "%d", level);
            ImGui::TableSetupColumn(label.data());
        }
        ImGui::TableHeadersRow();
        for (int hue = 0; hue < static_cast<int>(Hue::COUNT); ++hue) {
            ImGui::PushID(hue);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(names[hue]);
            for (int level = 1; level <= 10; ++level) {
                ImGui::TableSetColumnIndex(level);
                ImGui::PushID(level);
                ShowSwatch(names[hue], GetPaletteColor(static_cast<Hue>(hue), level), level);
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

void ShowSemanticSection() {
    ImGui::SeparatorText("Semantic colors");
    if (!ImGui::BeginTable("SemanticColors", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
        return;
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Color");
    ImGui::TableSetupColumn("Hex (RRGGBBAA)");
    ImGui::TableHeadersRow();
    for (int i = 0; i < static_cast<int>(Col::COUNT); ++i) {
        const Col col = static_cast<Col>(i);
        const char* name = GetColorName(col);
        const ImVec4 color = GetColor(col);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(name);
        ImGui::TableSetColumnIndex(1);
        ShowSwatch(name, color);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(ColorHex(color).data());
    }
    ImGui::EndTable();
}

void ShowPreviewSection() {
    ImGui::SeparatorText("Native ImGui preview");
    static bool disabled = false;
    static bool checked = true;
    static float value = 0.5f;
    static std::array<char, 128> input{};
    ImGui::Checkbox("Disable preview", &disabled);
    ImGui::BeginDisabled(disabled);
    ImGui::Button("Button");
    ImGui::SameLine();
    ImGui::Checkbox("Checkbox", &checked);
    ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f);
    ImGui::InputText("Input", input.data(), input.size());
    if (ImGui::BeginTabBar("PreviewTabs")) {
        if (ImGui::BeginTabItem("Overview")) {
            ImGui::TextUnformatted("Tab content");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Details")) {
            ImGui::TextUnformatted("Hover controls or use Tab to inspect focus.");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    if (ImGui::BeginTable("PreviewTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Item");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();
        for (int row = 0; row < 3; ++row) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Item %d", row + 1);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted("Ready");
        }
        ImGui::EndTable();
    }
    ImGui::EndDisabled();
}

} // namespace

void ShowDemoWindow(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(760.0f, 820.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("So-gui demo", open)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    ShowThemeSection();
    ShowPaletteSection();
    ShowSemanticSection();
    ShowPreviewSection();
    ImGui::End();
}

} // namespace SoGui
