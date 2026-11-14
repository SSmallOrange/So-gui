#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace SoGui {

// sogui_internal.h
bool IsNavCursorVisible(ImGuiID id) {
    const ImGuiContext& g = *GImGui;
    if (id != g.NavId || !g.NavCursorVisible)
        return false;
    if (id == g.LastItemData.ID && (g.LastItemData.ItemFlags & ImGuiItemFlags_NoNav))
        return false;
    return !g.CurrentWindow->DC.NavHideHighlightOneFrame;
}

float ItemBaseAlpha() {
    const ImGuiContext& g = *GImGui;
    return (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0 ? g.DisabledAlphaBackup : g.Style.Alpha;
}

} // namespace SoGui
