#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/core/icon.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cfloat>

namespace SoGui {

// 码点全部落在 PUA（0xE000~0xEFFF），编码为 3 字节 UTF-8
void EncodeIcon(unsigned codepoint, char (&out)[4]) {
    out[0] = static_cast<char>(0xE0 | (codepoint >> 12));
    out[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    out[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
    out[3] = '\0';
}

// sogui_internal.h
void IconDraw(ImDrawList* drawList, ImVec2 center, IconId id, float size, ImU32 color) {
    if (id == IconId::None)
        return;
    const FontEntry* entry = FindIconFont();
    if (entry == nullptr)
        return;

    ImGui::PushFont(entry->font, size * entry->heightPerEm);
    ImFont* font = ImGui::GetFont();
    const ImFontBaked* baked = ImGui::GetFontBaked();
    const float pixelSize = ImGui::GetFontSize();
    const float em = pixelSize / entry->heightPerEm;
    const float baseline = baked->Ascent * pixelSize / baked->Size; // ImGui 把基线放在绘制位置 + Ascent 处

    char text[4];
    EncodeIcon(static_cast<unsigned>(id), text);
    // em 方框位于 pos + (0, baseline - em) ~ (em, baseline)。ImGui 会截断绘制位置，这里先四舍五入
    const ImVec2 pos(IM_ROUND(center.x - em * 0.5f), IM_ROUND(center.y - baseline + em * 0.5f));
    drawList->AddText(font, pixelSize, pos, color, text, text + 3);
    ImGui::PopFont();
}

// icon.h
void Icon(IconId id, float size, Col color) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    const ImVec2 pxSize(Px(size), Px(size));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + pxSize);
    ImGui::ItemSize(pxSize);
    if (!ImGui::ItemAdd(bb, 0))
        return;
    IconDraw(window->DrawList, bb.GetCenter(), id, size, GetColorU32(color));
}

} // namespace SoGui
