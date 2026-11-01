#include <sogui/core/scale.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace SoGui {

float GetScale() {
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "No current ImGui context");
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiWindow* window = ImGui::GetCurrentWindowRead();
    return style.FontScaleMain * style.FontScaleDpi * (window != nullptr ? window->FontWindowScale : 1.0f);
}

float Px(float value) { return value * GetScale(); }

} // namespace SoGui
