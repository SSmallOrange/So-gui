#include <sogui/app.h>
#include <sogui/sogui.h>

#include <array>
#include <cstdio>

namespace {

using SoGui::FontSource;
using SoGui::FontWeight;

constexpr const char* kGeistRegular = SOGUI_FONT_DIR "/geist/Geist-Regular.ttf";
constexpr const char* kGeistMedium = SOGUI_FONT_DIR "/geist/Geist-Medium.ttf";
constexpr const char* kGeistSemiBold = SOGUI_FONT_DIR "/geist/Geist-SemiBold.ttf";
constexpr const char* kLucide = SOGUI_FONT_DIR "/lucide/lucide.ttf";

// 中文先用自带的 Noto Sans SC 常用字子集，缺字再由系统中文字体补；缺失的字体文件会被跳过
constexpr FontSource kNotoRegular{SOGUI_FONT_DIR "/noto-sans-sc/NotoSansSC-Regular.otf"};
constexpr FontSource kNotoMedium{SOGUI_FONT_DIR "/noto-sans-sc/NotoSansSC-Medium.otf"};

// 系统中文字体仅 macOS 已验证
#if defined(__APPLE__)
constexpr std::array kCjkRegular = {kNotoRegular, FontSource{"/System/Library/Fonts/Hiragino Sans GB.ttc", 0}};
constexpr std::array kCjkBold = {kNotoMedium, FontSource{"/System/Library/Fonts/Hiragino Sans GB.ttc", 2}};
#elif defined(_WIN32)
constexpr std::array kCjkRegular = {kNotoRegular, FontSource{"C:/Windows/Fonts/msyh.ttc", 0}};
constexpr std::array kCjkBold = {kNotoMedium, FontSource{"C:/Windows/Fonts/msyhbd.ttc", 0}};
#else
constexpr std::array kCjkRegular = {kNotoRegular,
                                    FontSource{"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", 2},
                                    FontSource{"/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc", 2}};
constexpr std::array kCjkBold = {kNotoMedium, FontSource{"/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc", 2},
                                 FontSource{"/usr/share/fonts/noto-cjk/NotoSansCJK-Bold.ttc", 2}};
#endif

void LoadFonts() {
    if (!SoGui::AddFont(FontWeight::Regular, {kGeistRegular}, kCjkRegular))
        std::fprintf(stderr, "Failed to load font: %s\n", kGeistRegular);
    SoGui::AddFont(FontWeight::Medium, {kGeistMedium}, kCjkBold);
    SoGui::AddFont(FontWeight::SemiBold, {kGeistSemiBold}, kCjkBold);
    SoGui::AddIconFont({kLucide});
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
