#pragma once

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
};

// 胶囊形徽标，显示短标签文字。规格见 doc/components/badge.md
void Badge(std::string_view label, const BadgeOptions& options = {});

} // namespace SoGui
