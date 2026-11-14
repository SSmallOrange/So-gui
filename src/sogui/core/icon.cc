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

    // Lucide 字形的墨迹盒与 em 不成标准比例（约 1.55em 高），先用参考字号测出墨迹占比，
    // 再换算出让墨迹正好填满 size x size 图标盒的字号
    ImGui::PushFont(entry->font, size * entry->heightPerEm);
    ImFontBaked* reference = ImGui::GetFontBaked();
    const ImFontGlyph* probe = reference->FindGlyph(static_cast<ImWchar>(static_cast<unsigned>(id)));
    const float inkRatio = (probe->Y1 - probe->Y0) / reference->Size; // 墨迹高 / 字号，与字号无关
    ImGui::PopFont();
    if (inkRatio <= 0.0f)
        return;

    const float fontPx = size / inkRatio;
    ImGui::PushFont(entry->font, fontPx * entry->heightPerEm);
    ImFont* font = ImGui::GetFont();
    ImFontBaked* baked = ImGui::GetFontBaked();
    const ImFontGlyph* glyph = baked->FindGlyph(static_cast<ImWchar>(static_cast<unsigned>(id)));
    const float scale = fontPx / baked->Size;

    char text[4];
    EncodeIcon(static_cast<unsigned>(id), text);
    // AddText 的 pos 是光标位置（基线），字形墨迹在 pos + (X0,Y0) ~ (X1,Y1)；平移使墨迹盒中心 = center
    const ImVec2 pos(center.x - (glyph->X0 + glyph->X1) * 0.5f * scale,
                     center.y - (glyph->Y0 + glyph->Y1) * 0.5f * scale);
    drawList->AddText(font, fontPx, pos, color, text, text + 3);
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
