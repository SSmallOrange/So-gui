#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <cmath>
#include <vector>

namespace {

using SoGui::Col;
using SoGui::Hue;
using SoGui::SpinnerColor;
using SoGui::SpinnerOptions;

ImU32 SpinnerColorU32(SpinnerColor color) {
    switch (color) {
    case SpinnerColor::Red:
        return SoGui::GetPaletteColorU32(Hue::Red, 7);
    case SpinnerColor::Green:
        return SoGui::GetPaletteColorU32(Hue::Green, 7);
    case SpinnerColor::Blue:
        return SoGui::GetPaletteColorU32(Hue::Blue, 7);
    default:
        return SoGui::GetColorU32(Col::TextDisabled);
    }
}

TEST_CASE_FIXTURE(ThemeFixture, "Spinner occupies a square item and requests redraw every frame") {
    for (float diameter : {12.0f, 16.0f, 20.0f, 24.0f, 32.0f, 40.0f, 56.0f}) {
        RunFrame([&] {
            SoGui::ConsumeRedrawRequest();
            SoGui::Spinner(diameter);
            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();
            CHECK(max.x - min.x == doctest::Approx(diameter).epsilon(0.01));
            CHECK(max.y - min.y == doctest::Approx(diameter).epsilon(0.01));
            CHECK(SoGui::ConsumeRedrawRequest()); // 转动期间持续请求重绘
        });
    }
}

TEST_CASE_FIXTURE(ThemeFixture, "Spinner colors map to gray-700 and scale level 7") {
    for (SpinnerColor color : {SpinnerColor::Default, SpinnerColor::Red, SpinnerColor::Green, SpinnerColor::Blue}) {
        RunFrame([&] {
            const ImDrawList* drawList = ImGui::GetWindowDrawList();
            const int first = drawList->VtxBuffer.Size;
            SoGui::Spinner(20.0f, SpinnerOptions{.color = color});
            const ImU32 expected = SpinnerColorU32(color) | IM_COL32_A_MASK; // 短条 alpha 随相位变化，只比较 RGB
            bool found = false;
            for (int i = first; i < drawList->VtxBuffer.Size; ++i)
                found |= (drawList->VtxBuffer[i].col | IM_COL32_A_MASK) == expected;
            CHECK(found);
        });
    }
}

TEST_CASE_FIXTURE(ThemeFixture, "Spinner draws bars for arbitrary diameters") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Spinner(28.0f); // 非标准直径，按 20 档缩放
        REQUIRE(drawList->VtxBuffer.Size > first);
        const ImVec2 itemMin = ImGui::GetItemRectMin();
        const ImVec2 center(itemMin.x + 14.0f, itemMin.y + 14.0f);
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            const ImVec2 p = drawList->VtxBuffer[i].pos;
            CHECK(std::hypot(p.x - center.x, p.y - center.y) <= 14.0f + 0.5f);
        }
    });
}

} // namespace
