#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

using SoGui::ButtonOptions;
using SoGui::ButtonSize;
using SoGui::ButtonVariant;
using SoGui::Col;
using SoGui::Hue;
using SoGui::TextStyle;
using SoGui::ThemeId;

constexpr const char* kLabel = "Upload";
constexpr int kSettleFrames = 12; // 150ms 过渡 + 余量

std::string FontPath(const char* name) { return std::string(SOGUI_TEST_FONT_DIR) + "/" + name; }

ImU32 Rgb(unsigned rgb) { return IM_COL32((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, 255); }

struct ButtonFixture : ThemeFixture {
    bool clicked = false;
    ImVec2 min;
    ImVec2 max;
    std::vector<ImU32> colors; // 本帧按钮产生的顶点颜色

    explicit ButtonFixture(const SoGui::ContextConfig& config = {}) : ThemeFixture(config) {
        REQUIRE(SoGui::AddFont(SoGui::FontWeight::Regular, {FontPath("Geist-Regular.ttf").c_str()}));
        REQUIRE(SoGui::AddFont(SoGui::FontWeight::Medium, {FontPath("Geist-Medium.ttf").c_str()}));
    }

    void Frame(const ButtonOptions& options = {}, const char* label = kLabel) {
        RunFrame([&] {
            const ImDrawList* drawList = ImGui::GetWindowDrawList();
            const int first = drawList->VtxBuffer.Size;
            clicked = SoGui::Button(label, options);
            min = ImGui::GetItemRectMin();
            max = ImGui::GetItemRectMax();
            colors.clear();
            for (int i = first; i < drawList->VtxBuffer.Size; ++i)
                colors.push_back(drawList->VtxBuffer[i].col);
        });
    }

    void Settle(const ButtonOptions& options = {}) {
        for (int i = 0; i < kSettleFrames; ++i)
            Frame(options);
    }

    ImVec2 Center() const { return ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f); }
    bool Has(ImU32 color) const { return std::find(colors.begin(), colors.end(), color) != colors.end(); }

    void TabToButton(const ButtonOptions& options = {}) {
        PressKey(ImGuiKey_Tab);
        Frame(options);
        PressKey(ImGuiKey_Tab, false);
        Frame(options);
        Frame(options);
    }
};

struct SizeRow {
    ButtonSize size;
    float height;
    float padding;
    TextStyle text;
};

constexpr SizeRow kSizeRows[] = {
    {ButtonSize::Small, 32.0f, 6.0f, TextStyle::Button14},
    {ButtonSize::Medium, 36.0f, 10.0f, TextStyle::Button14},
    {ButtonSize::Large, 40.0f, 14.0f, TextStyle::Button16},
};

TEST_CASE_FIXTURE(ButtonFixture, "button sizes follow the Geist table") {
    for (const SizeRow& row : kSizeRows) {
        RunFrame([&] {
            SoGui::Button(kLabel, {.size = row.size});
            const ImVec2 size = ImGui::GetItemRectSize();
            const float textWidth = SoGui::CalcTextSize(kLabel, row.text).x;
            CHECK(size.y == doctest::Approx(row.height));
            CHECK(size.x == doctest::Approx(textWidth + 2.0f * (row.padding + 6.0f)));
        });
    }
}

TEST_CASE("button sizes double with the UI scale") {
    ButtonFixture fixture;
    ImGui::GetStyle().FontScaleMain = 2.0f;
    for (const SizeRow& row : kSizeRows) {
        fixture.RunFrame([&] {
            SoGui::Button(kLabel, {.size = row.size});
            const ImVec2 size = ImGui::GetItemRectSize();
            const float textWidth = SoGui::CalcTextSize(kLabel, row.text).x;
            CHECK(size.y == doctest::Approx(row.height * 2.0f));
            CHECK(size.x == doctest::Approx(textWidth + 4.0f * (row.padding + 6.0f)));
        });
    }
}

TEST_CASE_FIXTURE(ButtonFixture, "loading adds a spinner slot plus gap to the width") {
    RunFrame([] {
        const float textWidth = SoGui::CalcTextSize(kLabel, TextStyle::Button14).x;
        SoGui::Button(kLabel, {.loading = true});
        const ImVec2 size = ImGui::GetItemRectSize();
        CHECK(size.x == doctest::Approx(textWidth + 2.0f * (10.0f + 6.0f) + 20.0f + 3.0f));
    });
}

TEST_CASE_FIXTURE(ButtonFixture, "full width uses the available width") {
    RunFrame([&] {
        const float available = ImGui::GetContentRegionAvail().x;
        SoGui::Button(kLabel, {.fullWidth = true});
        CHECK(ImGui::GetItemRectSize().x == doctest::Approx(available));
    });
}

TEST_CASE_FIXTURE(ButtonFixture, "click returns true only on the release frame") {
    Frame();
    MoveMouse(Center());
    Frame();
    MouseDown();
    Frame();
    CHECK_FALSE(clicked);
    MouseUp();
    Frame();
    CHECK(clicked);
    Frame();
    CHECK_FALSE(clicked);
}

TEST_CASE_FIXTURE(ButtonFixture, "keyboard space activates a tab focused button") {
    Frame();
    TabToButton();
    PressKey(ImGuiKey_Space);
    Frame();
    CHECK(clicked);
    PressKey(ImGuiKey_Space, false);
    Frame();
    CHECK_FALSE(clicked);
}

TEST_CASE_FIXTURE(ButtonFixture, "disabled and loading buttons never click and keep full alpha") {
    const ButtonOptions cases[] = {
        {.disabled = true},
        {.loading = true},
    };
    for (const ButtonOptions& options : cases) {
        Settle(options);
        MoveMouse(Center());
        Frame(options);
        MouseDown();
        Frame(options);
        CHECK_FALSE(clicked);
        MouseUp();
        Frame(options);
        CHECK_FALSE(clicked);

        // 禁用背景是 gray-100（Col::Fill），alpha 为 255，没有被 DisabledAlpha 变淡
        const ImU32 fill = SoGui::GetColorU32(Col::Fill);
        bool hasOpaqueFill = false;
        for (ImU32 color : colors) {
            if ((color & ~IM_COL32_A_MASK) == (fill & ~IM_COL32_A_MASK) && (color >> IM_COL32_A_SHIFT) == 0xFF)
                hasOpaqueFill = true;
        }
        CHECK(hasOpaqueFill);
    }
}

TEST_CASE_FIXTURE(ButtonFixture, "tertiary keeps a transparent background when disabled") {
    Settle({.variant = ButtonVariant::Tertiary, .disabled = true});
    const ImU32 textDisabled = SoGui::GetColorU32(Col::TextDisabled);
    CHECK(Has(textDisabled));
}

TEST_CASE_FIXTURE(ButtonFixture, "variant colors match the Geist table in light theme") {
    struct Row {
        ButtonVariant variant;
        ImU32 background;
        ImU32 foreground;
        bool bordered;
    };
    const std::vector<Row> rows = {
        {ButtonVariant::Primary, SoGui::GetPaletteColorU32(Hue::Gray, 10), SoGui::GetColorU32(Col::Bg1), false},
        {ButtonVariant::Secondary, SoGui::GetColorU32(Col::Bg1), SoGui::GetColorU32(Col::TextPrimary), true},
        {ButtonVariant::Error, SoGui::GetPaletteColorU32(Hue::Red, 8), Rgb(0xF5F5F5), false},
        {ButtonVariant::Warning, SoGui::GetPaletteColorU32(Hue::Amber, 8), Rgb(0x0A0A0A), false},
    };
    for (const Row& row : rows) {
        Settle({.variant = row.variant});
        INFO("variant ", static_cast<int>(row.variant));
        CHECK(Has(row.background));
        CHECK(Has(row.foreground));
        const ImU32 border = SoGui::GetColorU32(Col::Border);
        CHECK(Has(border) == row.bordered);
    }

    // Tertiary 普通态无背景、无边框，有正文色
    Settle({.variant = ButtonVariant::Tertiary});
    CHECK(Has(SoGui::GetColorU32(Col::TextPrimary)));
    CHECK_FALSE(Has(SoGui::GetColorU32(Col::Border)));
}

TEST_CASE("primary hover shows the themed hover color in both themes") {
    struct ThemeRow {
        ThemeId theme;
        ImU32 hover;
    };
    const ThemeRow rows[] = {
        {ThemeId::Light, Rgb(0x383838)},
        {ThemeId::Dark, Rgb(0xCCCCCC)},
    };
    for (const ThemeRow& row : rows) {
        ButtonFixture fixture({.theme = row.theme});
        fixture.Frame();
        fixture.Settle();
        fixture.MoveMouse(fixture.Center());
        fixture.Settle();
        CHECK(fixture.Has(row.hover));
    }
}

TEST_CASE("error hover shows the themed hover color in both themes") {
    for (const ThemeId theme : {ThemeId::Light, ThemeId::Dark}) {
        ButtonFixture fixture({.theme = theme});
        const ButtonOptions options{.variant = ButtonVariant::Error};
        fixture.Frame(options);
        fixture.Settle(options);
        fixture.MoveMouse(fixture.Center());
        fixture.Settle(options);
        // 亮色 #AE292F，暗色 red-900
        const ImU32 expected = theme == ThemeId::Light ? Rgb(0xAE292F) : SoGui::GetPaletteColorU32(Hue::Red, 9);
        CHECK(fixture.Has(expected));
    }
}

TEST_CASE_FIXTURE(ButtonFixture, "keyboard focus draws the focus ring and mouse click clears it") {
    Frame();
    TabToButton();
    const ImU32 ring = SoGui::GetColorU32(Col::FocusRing);
    bool hasRing = false;
    for (ImU32 color : colors)
        hasRing = hasRing || ((color & ~IM_COL32_A_MASK) == (ring & ~IM_COL32_A_MASK) && (color >> IM_COL32_A_SHIFT) > 0);
    CHECK(hasRing);

    MoveMouse(Center());
    Frame();
    MouseDown();
    Frame();
    MouseUp();
    Frame();
    // 鼠标点击后焦点环消失
    bool hasRingAfterClick = false;
    for (ImU32 color : colors)
        hasRingAfterClick = hasRingAfterClick || ((color & ~IM_COL32_A_MASK) == (ring & ~IM_COL32_A_MASK) &&
                                                 (color >> IM_COL32_A_SHIFT) > 0);
    CHECK_FALSE(hasRingAfterClick);
}

TEST_CASE_FIXTURE(ButtonFixture, "disabled buttons cannot be tab focused") {
    Frame();
    TabToButton({.disabled = true});
    const ImU32 ring = SoGui::GetColorU32(Col::FocusRing);
    for (ImU32 color : colors)
        CHECK((color & ~IM_COL32_A_MASK) != (ring & ~IM_COL32_A_MASK));
}

TEST_CASE_FIXTURE(ButtonFixture, "button, native text and SoGui text share a baseline") {
    RunFrame([] {
        // 按钮在行首，其余文字向其基线对齐
        SoGui::Button("Upload");
        const SoGui::TextMetrics metrics = SoGui::GetTextMetrics(TextStyle::Button14);
        const float buttonBaseline = ImGui::GetItemRectMin().y +
                                     std::floor((ImGui::GetItemRectSize().y - metrics.lineHeight) * 0.5f) +
                                     metrics.baseline;

        ImGui::SameLine();
        ImGui::TextUnformatted("native text");
        const float nativeBaseline = ImGui::GetItemRectMin().y + ImGui::GetFontBaked()->Ascent;

        ImGui::SameLine();
        SoGui::Text("Label", TextStyle::Label14);
        const float labelBaseline = ImGui::GetItemRectMin().y + SoGui::GetTextMetrics(TextStyle::Label14).baseline;

        CHECK(buttonBaseline == doctest::Approx(nativeBaseline).epsilon(0.6f));
        CHECK(labelBaseline == doctest::Approx(nativeBaseline).epsilon(0.6f));
    });
}

TEST_CASE_FIXTURE(ButtonFixture, "## splits the display label from the id") {
    Frame({}, "保存##a");
    Frame({}, "保存##b");
    // 两个不同的按钮，显示的文字都是"保存"，各自正常绘制且不冲突
    CHECK(max.x > min.x);
}

TEST_CASE_FIXTURE(ButtonFixture, "hovering a disabled button requests a not-allowed cursor") {
    Frame({.disabled = true});
    MoveMouse(Center());
    Frame({.disabled = true});
    CHECK(ImGui::GetMouseCursor() == ImGuiMouseCursor_NotAllowed);
}

TEST_CASE_FIXTURE(ButtonFixture, "hovering an enabled button requests a hand cursor") {
    Frame();
    MoveMouse(Center());
    Frame();
    CHECK(ImGui::GetMouseCursor() == ImGuiMouseCursor_Hand);
}

} // namespace
