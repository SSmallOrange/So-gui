#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <vector>

namespace {

using SoGui::Hue;
using SoGui::SkeletonOptions;
using SoGui::SkeletonVariant;

ImU32 Gray(int level) { return SoGui::GetPaletteColorU32(Hue::Gray, level); }

bool AlphaOf(const ImDrawList* drawList, int first, int* minAlpha, int* maxAlpha) {
    *minAlpha = 255;
    *maxAlpha = 0;
    for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
        const int a = static_cast<int>((drawList->VtxBuffer[i].col >> IM_COL32_A_SHIFT) & 0xFFU);
        *minAlpha = std::min(*minAlpha, a);
        *maxAlpha = std::max(*maxAlpha, a);
    }
    return drawList->VtxBuffer.Size > first;
}

TEST_CASE_FIXTURE(ThemeFixture, "Skeleton occupies the requested size") {
    RunFrame([] {
        SoGui::Skeleton(ImVec2(200.0f, 20.0f));
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        CHECK(max.x - min.x == doctest::Approx(200.0f).epsilon(0.01));
        CHECK(max.y - min.y == doctest::Approx(20.0f).epsilon(0.01));
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Skeleton requests redraw only while animating") {
    RunFrame([] {
        SoGui::ConsumeRedrawRequest();
        SoGui::Skeleton(ImVec2(200.0f, 20.0f));
        CHECK(SoGui::ConsumeRedrawRequest());
    });
    RunFrame([] {
        SoGui::ConsumeRedrawRequest();
        SoGui::Skeleton(ImVec2(200.0f, 20.0f), SkeletonOptions{.animate = false});
        CHECK(!SoGui::ConsumeRedrawRequest());
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Skeleton shimmer colors stay within gray-100 and gray-200") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Skeleton(ImVec2(300.0f, 24.0f));
        int minA, maxA;
        REQUIRE(AlphaOf(drawList, first, &minA, &maxA));
        // shimmer 顶点色应该在 gray-100 与 gray-200 之间（两段渐变插值）
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            const ImU32 c = drawList->VtxBuffer[i].col;
            const ImU32 opaque = c | IM_COL32_A_MASK;
            const ImU32 from = Gray(1) | IM_COL32_A_MASK;
            const ImU32 via = Gray(2) | IM_COL32_A_MASK;
            const bool between = (opaque == from) || (opaque == via);
            CHECK(between);
        }
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Skeleton static fill uses gray-200") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Skeleton(ImVec2(200.0f, 20.0f), SkeletonOptions{.animate = false});
        REQUIRE(drawList->VtxBuffer.Size > first);
        CHECK(drawList->VtxBuffer[first].col == Gray(2));
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Skeleton variants render without touching layout") {
    for (SkeletonVariant variant :
         {SkeletonVariant::Default, SkeletonVariant::Pill, SkeletonVariant::Rounded, SkeletonVariant::Squared}) {
        RunFrame([&] {
            const ImDrawList* drawList = ImGui::GetWindowDrawList();
            const int first = drawList->VtxBuffer.Size;
            SoGui::Skeleton(ImVec2(200.0f, 20.0f), SkeletonOptions{.variant = variant});
            REQUIRE(drawList->VtxBuffer.Size > first);
            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();
            CHECK(max.x - min.x == doctest::Approx(200.0f).epsilon(0.01));
            CHECK(max.y - min.y == doctest::Approx(20.0f).epsilon(0.01));
        });
    }
}

} // namespace
