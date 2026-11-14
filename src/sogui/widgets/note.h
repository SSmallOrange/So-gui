#pragma once

#include <sogui/core/icon.h>

#include <string_view>

namespace SoGui {

enum class NoteType { Default, Success, Error, Warning, Secondary, Violet, Cyan, COUNT };
enum class NoteSize { Small, Medium, COUNT };

struct NoteOptions {
    NoteType type   = NoteType::Default;
    NoteSize size   = NoteSize::Medium;
    bool     filled = false;       // 填充底色（Secondary 无 filled 效果）
    IconId   icon   = IconId::None; // None 时使用 type 的默认图标
};

// label 可为空（空串时只显示 content）；content 自动换行。
void Note(std::string_view label, std::string_view content, const NoteOptions& options = {});

} // namespace SoGui
