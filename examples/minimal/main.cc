#include <sogui/app.h>
#include <sogui/sogui.h>

#include <array>
#include <cstdio>
#include <string>

namespace {

using SoGui::FontSource;
using SoGui::FontWeight;

// 系统中文字体，缺失时跳过；仅 macOS 已验证
#if defined(__APPLE__)
constexpr std::array kCjkRegular = {FontSource{"/System/Library/Fonts/Hiragino Sans GB.ttc", 0}};
constexpr std::array kCjkBold = {FontSource{"/System/Library/Fonts/Hiragino Sans GB.ttc", 2}};
#elif defined(_WIN32)
constexpr std::array kCjkRegular = {FontSource{"C:/Windows/Fonts/msyh.ttc", 0}};
constexpr std::array kCjkBold = {FontSource{"C:/Windows/Fonts/msyhbd.ttc", 0}};
#else
constexpr std::array kCjkRegular = {FontSource{"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", 2},
                                    FontSource{"/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc", 2}};
constexpr std::array kCjkBold = {FontSource{"/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc", 2},
                                 FontSource{"/usr/share/fonts/noto-cjk/NotoSansCJK-Bold.ttc", 2}};
#endif

void LoadFonts() {
    const std::string dir = SOGUI_FONT_DIR;
    const std::string regular = dir + "/Geist-Regular.ttf";
    const std::string medium = dir + "/Geist-Medium.ttf";
    const std::string semiBold = dir + "/Geist-SemiBold.ttf";
    if (!SoGui::AddFont(FontWeight::Regular, {regular.c_str()}, kCjkRegular))
        std::fprintf(stderr, "Failed to load font: %s\n", regular.c_str());
    SoGui::AddFont(FontWeight::Medium, {medium.c_str()}, kCjkRegular);
    SoGui::AddFont(FontWeight::SemiBold, {semiBold.c_str()}, kCjkBold);
    // 原生 ImGui 控件也使用 Geist
    if (ImFont* font = SoGui::GetFont(FontWeight::Regular))
        ImGui::GetIO().FontDefault = font;
}

} // namespace

int main() {
    return SoGui::RunApp({
        .title = "So-gui minimal",
        .onInit = LoadFonts,
        .onFrame =
            [] {
                ImGui::ShowDemoWindow();
                SoGui::ShowDemoWindow();
            },
    });
}
