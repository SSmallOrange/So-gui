#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace {

using SoGui::Col;
using SoGui::IconId;

constexpr const char* kLucide = SOGUI_TEST_LUCIDE_FONT_DIR "/lucide.ttf";

TEST_CASE_FIXTURE(ThemeFixture, "Icon renders nothing without the icon font") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Icon(IconId::Check);
        CHECK(drawList->VtxBuffer.Size == first); // 未注册图标字体时静默跳过
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Icon draws a glyph with the requested size and color") {
    REQUIRE(SoGui::AddIconFont({kLucide}));
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Icon(IconId::Check, 16.0f, Col::Accent);
        REQUIRE(drawList->VtxBuffer.Size > first);
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        CHECK(max.x - min.x == doctest::Approx(16.0f).epsilon(0.01));
        CHECK(max.y - min.y == doctest::Approx(16.0f).epsilon(0.01));
        const ImU32 expected = SoGui::GetColorU32(Col::Accent);
        bool found = false;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i)
            found |= drawList->VtxBuffer[i].col == expected;
        CHECK(found);
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Icon None draws nothing") {
    REQUIRE(SoGui::AddIconFont({kLucide}));
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Icon(IconId::None);
        CHECK(drawList->VtxBuffer.Size == first);
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "IconDraw centers the glyph") {
    REQUIRE(SoGui::AddIconFont({kLucide}));
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 center(100.0f, 100.0f);
        const int first = drawList->VtxBuffer.Size;
        SoGui::IconDraw(drawList, center, IconId::Check, 20.0f, IM_COL32_WHITE);
        REQUIRE(drawList->VtxBuffer.Size > first);
        float minX = center.x, maxX = center.x, minY = center.y, maxY = center.y;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            minX = std::min(minX, drawList->VtxBuffer[i].pos.x);
            maxX = std::max(maxX, drawList->VtxBuffer[i].pos.x);
            minY = std::min(minY, drawList->VtxBuffer[i].pos.y);
            maxY = std::max(maxY, drawList->VtxBuffer[i].pos.y);
        }
        // 字形墨迹盒居中于 center，且墨迹尺寸等于请求的图标盒（Lucide 字形墨迹约 1.55em，
        // 必须按墨迹占比换算字号，否则图标会明显偏大）
        CHECK((minX + maxX) * 0.5f == doctest::Approx(center.x).epsilon(0.5));
        CHECK((minY + maxY) * 0.5f == doctest::Approx(center.y).epsilon(0.5));
        CHECK(maxY - minY == doctest::Approx(20.0f).epsilon(0.1));
    });
}

} // namespace

