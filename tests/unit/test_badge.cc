#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace {

using SoGui::BadgeOptions;
using SoGui::BadgeSize;
using SoGui::BadgeVariant;
using SoGui::Hue;
using SoGui::ThemeId;

std::string FontPath(const char* name) { return std::string(SOGUI_TEST_FONT_DIR) + "/" + name; }

struct BadgeFixture : ThemeFixture {
    ImVec2 min;
    ImVec2 max;
    std::vector<ImU32> colors;

    explicit BadgeFixture() {
        REQUIRE(SoGui::AddFont(SoGui::FontWeight::Medium, {FontPath("Geist-Medium.ttf").c_str()}));
        REQUIRE(SoGui::AddIconFont({SOGUI_TEST_LUCIDE_FONT_DIR "/lucide.ttf"}));
    }

    void Frame(const BadgeOptions& options, const char* label = "Ready") {
        RunFrame([&] {
            const ImDrawList* drawList = ImGui::GetWindowDrawList();
            const int first = drawList->VtxBuffer.Size;
            SoGui::Badge(label, options);
            min = ImGui::GetItemRectMin();
            max = ImGui::GetItemRectMax();
            colors.clear();
            for (int i = first; i < drawList->VtxBuffer.Size; ++i)
                colors.push_back(drawList->VtxBuffer[i].col);
        });
    }

    bool Has(ImU32 color) const { return std::find(colors.begin(), colors.end(), color) != colors.end(); }
};

TEST_CASE_FIXTURE(BadgeFixture, "Badge sizes are 20 / 24 / 32 tall") {
    const BadgeSize sizes[] = {BadgeSize::Small, BadgeSize::Medium, BadgeSize::Large};
    const float heights[] = {20.0f, 24.0f, 32.0f};
    for (int i = 0; i < 3; ++i) {
        Frame({.size = sizes[i]});
        CHECK(max.y - min.y == doctest::Approx(heights[i]).epsilon(0.01));
    }
}

TEST_CASE_FIXTURE(BadgeFixture, "Badge width follows the label") {
    Frame({.variant = BadgeVariant::Gray}, "Ready");
    const float shortWidth = max.x - min.x;
    Frame({.variant = BadgeVariant::Gray}, "Deploy");
    const float longWidth = max.x - min.x;
    CHECK(longWidth > shortWidth);
}

TEST_CASE_FIXTURE(BadgeFixture, "Badge filled variants use the documented palette") {
    // 亮色：gray = gray-900；blue = blue-800；amber = amber-700（黑字）；inverted = gray-1000
    Frame({.variant = BadgeVariant::Gray});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 9)));
    Frame({.variant = BadgeVariant::Blue});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Blue, 8)));
    Frame({.variant = BadgeVariant::Amber});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Amber, 7)));
    CHECK(Has(IM_COL32(0, 0, 0, 255))); // 黑字
    Frame({.variant = BadgeVariant::Inverted});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 10)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 1)));
}

TEST_CASE_FIXTURE(BadgeFixture, "Badge filled variant background follows the dark theme") {
    SoGui::SetTheme(ThemeId::Dark);
    Frame({.variant = BadgeVariant::Gray}); // 暗色 gray-500
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 5)));
}

TEST_CASE_FIXTURE(BadgeFixture, "Badge subtle variants draw no background") {
    Frame({.variant = BadgeVariant::GraySubtle});
    // 透明背景：没有 gray 系列的填充色顶点，文字为 gray-1000
    CHECK(!Has(SoGui::GetPaletteColorU32(Hue::Gray, 9)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 10)));
}

} // namespace

