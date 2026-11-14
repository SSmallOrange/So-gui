#pragma once

#include <sogui/core/icon.h>

#include <string_view>

namespace SoGui {

enum class BadgeVariant {
    Gray, GraySubtle, Blue, BlueSubtle, Purple, PurpleSubtle, Amber, AmberSubtle,
    Red, RedSubtle, Pink, PinkSubtle, Green, GreenSubtle, Teal, TealSubtle, Inverted,
    COUNT
};
enum class BadgeSize { Small, Medium, Large, COUNT };

struct BadgeOptions {
    BadgeVariant variant = BadgeVariant::Gray;
    BadgeSize    size    = BadgeSize::Medium;
    IconId       icon    = IconId::None; // None 时不显示图标
};

// 胶囊形徽标，显示短标签文字。
void Badge(std::string_view label, const BadgeOptions& options = {});

} // namespace SoGui
