#include "test_context.h"

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>

namespace {

using std::max;

using SoGui::Col;
using SoGui::FontSource;
using SoGui::FontWeight;
using SoGui::TextStyle;

std::string FontPath(const char* name) { return std::string(SOGUI_TEST_FONT_DIR) + "/" + name; }
std::string CjkFontPath(const char* name) { return std::string(SOGUI_TEST_CJK_FONT_DIR) + "/" + name; }

struct TextFixture : ThemeFixture {
    std::string regular = FontPath("Geist-Regular.ttf");
    std::string semiBold = FontPath("Geist-SemiBold.ttf");
    bool windowOpen = false;

    TextFixture() {
        REQUIRE(SoGui::AddFont(FontWeight::Regular, {regular.c_str()}));
        REQUIRE(SoGui::AddFont(FontWeight::SemiBold, {semiBold.c_str()}));
    }

    ~TextFixture() {
        if (windowOpen)
            ImGui::End();
    }

    void BeginWindow(float width = 400.0f) {
        BeginFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(width, 600.0f));
        ImGui::Begin("Text");
        windowOpen = true;
    }
};

float Width(const char* text) { return SoGui::CalcTextSize(text, TextStyle::Label14).x; }

void CheckGlyphsInside(int firstVertex, ImVec2 min, ImVec2 max) {
    const ImDrawList* drawList = ImGui::GetWindowDrawList();
    REQUIRE(drawList->VtxBuffer.Size > firstVertex);
    for (int i = firstVertex; i < drawList->VtxBuffer.Size; ++i) {
        const ImVec2 pos = drawList->VtxBuffer[i].pos;
        CHECK(pos.y >= min.y);
        CHECK(pos.y <= max.y);
        CHECK(pos.x >= min.x - 1.0f);
        CHECK(pos.x <= max.x + 1.0f);
    }
}

TEST_CASE_FIXTURE(ThemeFixture, "font registration rejects missing files without replacing fonts") {
    const std::string regular = FontPath("Geist-Regular.ttf");
    CHECK_FALSE(SoGui::AddFont(FontWeight::Regular, {"/nonexistent/sogui/missing.ttf"}));
    CHECK(SoGui::GetFont(FontWeight::Regular) == nullptr);

    REQUIRE(SoGui::AddFont(FontWeight::Regular, {regular.c_str()}));
    ImFont* font = SoGui::GetFont(FontWeight::Regular);
    REQUIRE(font != nullptr);
    CHECK(font->ContainerAtlas == ImGui::GetIO().Fonts);

    CHECK_FALSE(SoGui::AddFont(FontWeight::Regular, {"/nonexistent/sogui/missing.ttf"}));
    CHECK(SoGui::GetFont(FontWeight::Regular) == font);
    CHECK(SoGui::GetFont(FontWeight::Medium) == nullptr);
}

TEST_CASE_FIXTURE(ThemeFixture, "font registration rejects an invalid collection index") {
    const std::string regular = FontPath("Geist-Regular.ttf");
    CHECK_FALSE(SoGui::AddFont(FontWeight::Regular, {regular.c_str(), 1}));
    CHECK(SoGui::GetFont(FontWeight::Regular) == nullptr);
}

TEST_CASE_FIXTURE(ThemeFixture, "font registration merges available fallbacks and skips missing ones") {
    const std::string regular = FontPath("Geist-Regular.ttf");
    const std::string medium = FontPath("Geist-Medium.ttf");
    const std::array fallbacks = {FontSource{"/nonexistent/sogui/missing.ttc"}, FontSource{medium.c_str()}};
    REQUIRE(SoGui::AddFont(FontWeight::Regular, {regular.c_str()}, fallbacks));
    CHECK(SoGui::GetFont(FontWeight::Regular)->Sources.Size == 2);
}

TEST_CASE_FIXTURE(TextFixture, "every text style reports the line height of the Geist table") {
    constexpr std::array<float, static_cast<std::size_t>(TextStyle::COUNT)> kLineHeights = {
        72, 64, 56, 56, 48, 40, 32, 26, 24, 20, // Heading
        20, 20, 16,                             // Button
        32, 20, 20, 20, 20, 16, 20, 16, 16,     // Label
        36, 36, 28, 24, 20, 18, 18,             // Copy
    };
    BeginWindow();
    for (int i = 0; i < static_cast<int>(TextStyle::COUNT); ++i) {
        const TextStyle style = static_cast<TextStyle>(i);
        INFO("style: ", SoGui::GetTextStyleName(style));
        CHECK(SoGui::GetTextMetrics(style).lineHeight == doctest::Approx(kLineHeights[static_cast<std::size_t>(i)]));
    }
    CHECK(std::string(SoGui::GetTextStyleName(TextStyle::Button14)) == "Button14");
    CHECK(std::string(SoGui::GetTextStyleName(TextStyle::Copy13Mono)) == "Copy13Mono");
}

TEST_CASE_FIXTURE(TextFixture, "text metrics follow CSS em sizes and line heights") {
    BeginWindow();
    // Geist: unitsPerEm 1000, ascender 1005, descender -295
    const SoGui::TextMetrics label = SoGui::GetTextMetrics(TextStyle::Label14);
    CHECK(label.fontSize == doctest::Approx(18.0f / 1.3f));
    CHECK(label.lineHeight == doctest::Approx(20.0f));
    CHECK(label.baseline == doctest::Approx(15.0f));

    const SoGui::TextMetrics heading = SoGui::GetTextMetrics(TextStyle::Heading24);
    CHECK(heading.fontSize == doctest::Approx(31.0f / 1.3f));
    CHECK(heading.lineHeight == doctest::Approx(32.0f));
    CHECK(heading.baseline == doctest::Approx(24.0f));
}

TEST_CASE_FIXTURE(TextFixture, "text metrics and design pixels scale once with ImGui font scales") {
    ImGui::GetStyle().FontScaleMain = 2.0f;
    BeginWindow();
    CHECK(SoGui::GetScale() == doctest::Approx(2.0f));
    CHECK(SoGui::Px(10.0f) == doctest::Approx(20.0f));
    const SoGui::TextMetrics label = SoGui::GetTextMetrics(TextStyle::Label14);
    CHECK(label.fontSize == doctest::Approx(36.0f / 1.3f));
    CHECK(label.lineHeight == doctest::Approx(40.0f));
    CHECK(label.baseline == doctest::Approx(30.0f));
}

TEST_CASE_FIXTURE(TextFixture, "text scale combines main and DPI font scales") {
    ImGui::GetStyle().FontScaleMain = 1.25f;
    ImGui::GetStyle().FontScaleDpi = 1.2f;
    BeginWindow();
    CHECK(SoGui::GetScale() == doctest::Approx(1.5f));
    const SoGui::TextMetrics label = SoGui::GetTextMetrics(TextStyle::Label14);
    CHECK(label.fontSize == doctest::Approx(27.0f / 1.3f));
    CHECK(label.lineHeight == doctest::Approx(30.0f));
}

TEST_CASE_FIXTURE(TextFixture, "single-line labels keep explicit newlines and ignore available width") {
    BeginWindow(120.0f);
    SoGui::Text("Label\nSecond line", TextStyle::Label14);
    CHECK(ImGui::GetItemRectSize().y == doctest::Approx(40.0f));
    CHECK(ImGui::GetItemRectSize().x == doctest::Approx(Width("Second line")));

    const char* longLabel = "A single line label that is wider than the window";
    SoGui::Text(longLabel, TextStyle::Label14);
    CHECK(ImGui::GetItemRectSize().y == doctest::Approx(20.0f));
    CHECK(ImGui::GetItemRectSize().x == doctest::Approx(Width(longLabel)));
}

TEST_CASE_FIXTURE(TextFixture, "copy text wraps to available width and measurement matches layout") {
    BeginWindow(260.0f);
    const char* paragraph = "Geist copy text wraps across several lines when the available width is narrow.";
    const float available = ImGui::GetContentRegionAvail().x;
    SoGui::Text(paragraph, TextStyle::Copy14);
    const ImVec2 size = ImGui::GetItemRectSize();
    const ImVec2 measured = SoGui::CalcTextSize(paragraph, TextStyle::Copy14, available);
    CHECK(size.x == doctest::Approx(measured.x));
    CHECK(size.y == doctest::Approx(measured.y));
    CHECK(size.y >= 60.0f);
    CHECK(size.x <= available);
}

TEST_CASE_FIXTURE(TextFixture, "wrapping breaks between Latin words and cuts only oversized words") {
    BeginWindow();
    const float wrap = Width("alpha beta") + 0.5f;
    const ImVec2 words = SoGui::CalcTextSize("alpha beta gamma", TextStyle::Copy14, wrap);
    CHECK(words.y == doctest::Approx(40.0f));
    CHECK(words.x == doctest::Approx(Width("alpha beta")));

    const ImVec2 decimal = SoGui::CalcTextSize("Uptime 99.9%", TextStyle::Copy14, Width("Uptime 99.") + 0.5f);
    CHECK(decimal.y == doctest::Approx(40.0f));
    CHECK(decimal.x == doctest::Approx(max(Width("Uptime"), Width("99.9%"))));

    const ImVec2 cut = SoGui::CalcTextSize("abcdefghij", TextStyle::Copy14, Width("abcde") + 0.5f);
    CHECK(cut.y == doctest::Approx(40.0f));
    CHECK(cut.x == doctest::Approx(max(Width("abcde"), Width("fghij"))));
}

TEST_CASE_FIXTURE(TextFixture, "wrapping allows breaks between CJK characters except before closing punctuation") {
    BeginWindow();
    const float wrap = Width("中文中文") + 0.5f;
    const ImVec2 cjk = SoGui::CalcTextSize("中文中文中文", TextStyle::Copy14, wrap);
    CHECK(cjk.y == doctest::Approx(40.0f));
    CHECK(cjk.x == doctest::Approx(Width("中文中文")));

    const ImVec2 punct = SoGui::CalcTextSize("中文中文，", TextStyle::Copy14, wrap);
    CHECK(punct.y == doctest::Approx(40.0f));
    CHECK(punct.x == doctest::Approx(Width("中文中")));

    const ImVec2 afterWord = SoGui::CalcTextSize("ab 中文中文中文中文", TextStyle::Copy14, Width("ab 中文中文") + 0.5f);
    CHECK(afterWord.y == doctest::Approx(40.0f));
    CHECK(afterWord.x == doctest::Approx(Width("ab 中文中文")));

    const float mixedWrap = max(Width("部署De"), Width("Deploy")) + 0.5f;
    const ImVec2 mixed = SoGui::CalcTextSize("部署Deploy", TextStyle::Copy14, mixedWrap);
    CHECK(mixed.y == doctest::Approx(40.0f));
    CHECK(mixed.x == doctest::Approx(max(Width("部署"), Width("Deploy"))));
}

TEST_CASE_FIXTURE(TextFixture, "text glyphs stay inside line boxes") {
    BeginWindow();
    for (TextStyle style : {TextStyle::Label14, TextStyle::Copy14, TextStyle::Heading24}) {
        const int firstVertex = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        SoGui::Text("Typography gyp", style);
        CheckGlyphsInside(firstVertex, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }
}

TEST_CASE_FIXTURE(TextFixture, "text aligns baselines with native widgets on the same line") {
    BeginWindow();
    const float nativeAscent = ImGui::GetFontBaked()->Ascent;
    ImGui::Button("Native");
    const float baseline = ImGui::GetItemRectMin().y + ImGui::GetStyle().FramePadding.y + nativeAscent;
    ImGui::SameLine();
    SoGui::Text("Label", TextStyle::Label14);
    CHECK(ImGui::GetItemRectMin().y + SoGui::GetTextMetrics(TextStyle::Label14).baseline == doctest::Approx(baseline));
    ImGui::SameLine();
    ImGui::TextUnformatted("native");
    CHECK(ImGui::GetItemRectMin().y + nativeAscent == doctest::Approx(baseline));

    SoGui::Text("Heading", TextStyle::Heading24);
    const float headingBaseline = ImGui::GetItemRectMin().y + SoGui::GetTextMetrics(TextStyle::Heading24).baseline;
    ImGui::SameLine();
    ImGui::TextUnformatted("native");
    CHECK(ImGui::GetItemRectMin().y + nativeAscent == doctest::Approx(headingBaseline));
}

TEST_CASE_FIXTURE(TextFixture, "text uses semantic colors and honors disabled alpha") {
    BeginWindow();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    int firstVertex = drawList->VtxBuffer.Size;
    SoGui::Text("Primary", TextStyle::Label14);
    REQUIRE(drawList->VtxBuffer.Size > firstVertex);
    CHECK(drawList->VtxBuffer[firstVertex].col == SoGui::GetColorU32(Col::TextPrimary));

    firstVertex = drawList->VtxBuffer.Size;
    SoGui::Text("Secondary", TextStyle::Label14, {.color = Col::TextSecondary});
    REQUIRE(drawList->VtxBuffer.Size > firstVertex);
    CHECK(drawList->VtxBuffer[firstVertex].col == SoGui::GetColorU32(Col::TextSecondary));

    ImGui::BeginDisabled();
    const ImU32 disabled = SoGui::GetColorU32(Col::TextPrimary);
    firstVertex = drawList->VtxBuffer.Size;
    SoGui::Text("Disabled", TextStyle::Label14);
    ImGui::EndDisabled();
    REQUIRE(drawList->VtxBuffer.Size > firstVertex);
    CHECK(drawList->VtxBuffer[firstVertex].col == disabled);
    CHECK(((disabled >> IM_COL32_A_SHIFT) & 0xFFU) < 255U);
}

TEST_CASE_FIXTURE(ThemeFixture, "text falls back to the ImGui font when no SoGui font is registered") {
    BeginFrame();
    ImGui::Begin("Fallback");
    SoGui::Text("Fallback\ntext", TextStyle::Label14);
    CHECK(ImGui::GetItemRectSize().y == doctest::Approx(40.0f));
    CHECK(SoGui::GetTextMetrics(TextStyle::Heading24).lineHeight == doctest::Approx(32.0f));
    ImGui::End();
}

TEST_CASE_FIXTURE(ThemeFixture, "bundled CJK subset shares the main font em size and covers common hanzi") {
    const std::string regular = FontPath("Geist-Regular.ttf");
    const std::string cjk = CjkFontPath("NotoSansSC-Regular.otf");
    const std::array fallbacks = {FontSource{cjk.c_str()}};
    REQUIRE(SoGui::AddFont(FontWeight::Regular, {regular.c_str()}, fallbacks));
    ImFont* font = SoGui::GetFont(FontWeight::Regular);
    REQUIRE(font->Sources.Size == 2);
    BeginFrame();
    ImGui::Begin("CJK");
    CHECK(font->IsGlyphInFont(0x4E2D));       // 中：一级字
    CHECK(font->IsGlyphInFont(0x6E32));       // 渲：二级字
    CHECK(font->IsGlyphInFont(0xFF0C));       // ，
    CHECK_FALSE(font->IsGlyphInFont(0x5586)); // 喆：不在 GB2312 中，子集不收录
    const ImFontGlyph* glyph = font->GetFontBaked(16.0f)->FindGlyphNoFallback(0x4E2D);
    REQUIRE(glyph != nullptr);
    CHECK(static_cast<int>(glyph->SourceIdx) == 1);
    CHECK(static_cast<bool>(glyph->Visible)); // 子集后的 CFF 轮廓能正常光栅化
    // Noto Sans SC 的全角字宽为 1em
    const float em = SoGui::GetTextMetrics(TextStyle::Label14).fontSize;
    CHECK(SoGui::CalcTextSize("中", TextStyle::Label14).x == doctest::Approx(std::ceil(em)));
    ImGui::End();
}

TEST_CASE_FIXTURE(ThemeFixture, "system CJK fonts only fill glyphs missing from the bundled subset") {
    constexpr const char* kSystem = "/System/Library/Fonts/Hiragino Sans GB.ttc";
    std::FILE* file = std::fopen(kSystem, "rb");
    if (file == nullptr) {
        MESSAGE("Skipped: system CJK font not found");
        return;
    }
    std::fclose(file);
    const std::string regular = FontPath("Geist-Regular.ttf");
    const std::string cjk = CjkFontPath("NotoSansSC-Regular.otf");
    const std::array fallbacks = {FontSource{cjk.c_str()}, FontSource{kSystem, 0}};
    REQUIRE(SoGui::AddFont(FontWeight::Regular, {regular.c_str()}, fallbacks));
    ImFont* font = SoGui::GetFont(FontWeight::Regular);
    REQUIRE(font->Sources.Size == 3);
    BeginFrame();
    ImFontBaked* baked = font->GetFontBaked(16.0f);
    const ImFontGlyph* common = baked->FindGlyphNoFallback(0x6E32); // 渲
    const ImFontGlyph* rare = baked->FindGlyphNoFallback(0x5586);   // 喆
    REQUIRE(common != nullptr);
    REQUIRE(rare != nullptr);
    CHECK(static_cast<int>(common->SourceIdx) == 1);
    CHECK(static_cast<int>(rare->SourceIdx) == 2);
}

} // namespace
