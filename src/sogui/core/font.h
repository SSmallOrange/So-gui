#pragma once

#include <imgui.h>

#include <span>

namespace SoGui {

enum class FontWeight { Regular, Medium, SemiBold, COUNT };

struct FontSource {
    const char* path = nullptr;
    int index = 0; // TTC 中的字体序号
};

// 需在帧外调用。fallbacks 依次补齐缺失字符，无法读取的会被跳过。
bool AddFont(FontWeight weight, const FontSource& source, std::span<const FontSource> fallbacks = {});
// 图标字体（Lucide），SoGui::Icon 使用。需在帧外调用。
bool AddIconFont(const FontSource& source);
// 未注册或不属于当前 ImGui 图集时返回 nullptr
ImFont* GetFont(FontWeight weight);

} // namespace SoGui
