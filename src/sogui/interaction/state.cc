#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace SoGui {

// sogui_internal.h
Interaction ResolveInteraction(const ImRect& bb, ImGuiID id, ImGuiButtonFlags flags) {
    bool hovered = false;
    bool held = false;
    // 键盘选中不算悬停，焦点环另外绘制
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags | ImGuiButtonFlags_NoHoveredOnFocus);

    Interaction result;
    result.clicked = pressed;
    result.focusVisible = IsNavCursorVisible(id);
    if ((GImGui->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0)
        result.state = InteractionState::Disabled;
    else if (held && hovered)
        result.state = InteractionState::Active;
    else if (hovered)
        result.state = InteractionState::Hover;
    return result;
}

} // namespace SoGui
