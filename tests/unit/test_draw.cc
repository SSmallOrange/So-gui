#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

namespace {

struct DrawFixture : ThemeFixture {
    explicit DrawFixture(float framebufferScale = 1.0f) {
        ImGui::GetMainViewport()->FramebufferScale = ImVec2(framebufferScale, framebufferScale);
        ImGui::GetStyle().AntiAliasedLines = false; // 顶点位置精确，便于断言
    }
};

TEST_CASE_FIXTURE(DrawFixture, "SnapStroke rounds down to whole physical pixels with a minimum of one") {
    RunFrame([] {
        CHECK(SoGui::SnapStroke(1.0f) == doctest::Approx(1.0f));
        CHECK(SoGui::SnapStroke(1.5f) == doctest::Approx(1.0f));
        CHECK(SoGui::SnapStroke(2.9f) == doctest::Approx(2.0f));
        CHECK(SoGui::SnapStroke(0.2f) == doctest::Approx(1.0f));
    });
}

TEST_CASE("SnapStroke and SnapToPixel follow the framebuffer scale") {
    DrawFixture fixture(2.0f);
    fixture.RunFrame([] {
        CHECK(SoGui::SnapStroke(1.0f) == doctest::Approx(1.0f));
        CHECK(SoGui::SnapStroke(0.75f) == doctest::Approx(0.5f));
        CHECK(SoGui::SnapStroke(0.25f) == doctest::Approx(0.5f));
        const ImVec2 snapped = SoGui::SnapToPixel(ImVec2(10.3f, 20.8f));
        CHECK(snapped.x == doctest::Approx(10.5f));
        CHECK(snapped.y == doctest::Approx(21.0f));
    });
}

TEST_CASE_FIXTURE(DrawFixture, "SnapToPixel rounds to whole pixels at scale one") {
    RunFrame([] {
        const ImVec2 snapped = SoGui::SnapToPixel(ImVec2(10.4f, 20.6f));
        CHECK(snapped.x == doctest::Approx(10.0f));
        CHECK(snapped.y == doctest::Approx(21.0f));
        const ImVec2 negative = SoGui::SnapToPixel(ImVec2(-3.4f, -3.6f));
        CHECK(negative.x == doctest::Approx(-3.0f));
        CHECK(negative.y == doctest::Approx(-4.0f));
    });
}

TEST_CASE_FIXTURE(DrawFixture, "RectRing keeps all vertices between offset and offset plus width") {
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 min(100.0f, 100.0f);
        const ImVec2 max(140.0f, 130.0f);
        constexpr float kOffset = 2.0f;
        constexpr float kWidth = 3.0f;
        constexpr float kTolerance = 0.01f;

        const int first = drawList->VtxBuffer.Size;
        SoGui::RectRing(drawList, min, max, IM_COL32_WHITE, 0.0f, kWidth, kOffset);
        REQUIRE(drawList->VtxBuffer.Size > first);
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            const ImVec2 p = drawList->VtxBuffer[i].pos;
            CHECK(p.x >= min.x - kOffset - kWidth - kTolerance);
            CHECK(p.x <= max.x + kOffset + kWidth + kTolerance);
            CHECK(p.y >= min.y - kOffset - kWidth - kTolerance);
            CHECK(p.y <= max.y + kOffset + kWidth + kTolerance);
            const bool insideInner = p.x > min.x - kOffset + kTolerance && p.x < max.x + kOffset - kTolerance &&
                                     p.y > min.y - kOffset + kTolerance && p.y < max.y + kOffset - kTolerance;
            CHECK_FALSE(insideInner);
        }

        // 圆角环：外沿不会超出外扩 offset + width 的矩形
        const int roundedFirst = drawList->VtxBuffer.Size;
        SoGui::RectRing(drawList, min, max, IM_COL32_WHITE, 6.0f, kWidth, kOffset);
        REQUIRE(drawList->VtxBuffer.Size > roundedFirst);
        float minX = max.x;
        for (int i = roundedFirst; i < drawList->VtxBuffer.Size; ++i) {
            const ImVec2 p = drawList->VtxBuffer[i].pos;
            CHECK(p.x >= min.x - kOffset - kWidth - kTolerance);
            CHECK(p.x <= max.x + kOffset + kWidth + kTolerance);
            CHECK(p.y >= min.y - kOffset - kWidth - kTolerance);
            CHECK(p.y <= max.y + kOffset + kWidth + kTolerance);
            minX = std::min(minX, p.x);
        }
        CHECK(minX == doctest::Approx(min.x - kOffset - kWidth).epsilon(0.001));
    });
}

TEST_CASE_FIXTURE(DrawFixture, "RectRing and RectFilled skip fully transparent colors") {
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::RectRing(drawList, ImVec2(10, 10), ImVec2(20, 20), IM_COL32(255, 0, 0, 0), 4.0f, 1.0f);
        SoGui::RectFilled(drawList, ImVec2(10, 10), ImVec2(20, 20), IM_COL32(255, 0, 0, 0), 4.0f);
        CHECK(drawList->VtxBuffer.Size == first);
    });
}

TEST_CASE_FIXTURE(DrawFixture, "FocusRing draws a background gap and a focus colored ring") {
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 min(100.0f, 100.0f);
        const ImVec2 max(140.0f, 136.0f);
        const int first = drawList->VtxBuffer.Size;
        SoGui::FocusRing(drawList, min, max, 6.0f);
        REQUIRE(drawList->VtxBuffer.Size > first);

        const ImU32 gap = ImGui::ColorConvertFloat4ToU32(SoGui::ResolveColor(SoGui::Sem(SoGui::Col::Bg1)));
        const ImU32 ring = ImGui::ColorConvertFloat4ToU32(SoGui::ResolveColor(SoGui::Sem(SoGui::Col::FocusRing)));
        bool hasGap = false;
        bool hasRing = false;
        float minX = max.x;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            const ImDrawVert& v = drawList->VtxBuffer[i];
            hasGap = hasGap || v.col == gap;
            hasRing = hasRing || v.col == ring;
            minX = std::min(minX, v.pos.x);
        }
        CHECK(hasGap);
        CHECK(hasRing);
        // 2px 间隔 + 2px 焦点色
        CHECK(minX == doctest::Approx(min.x - 4.0f).epsilon(0.001));
    });
}

TEST_CASE_FIXTURE(DrawFixture, "FocusRing widths scale with the UI scale") {
    ImGui::GetStyle().FontScaleMain = 2.0f;
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::FocusRing(drawList, ImVec2(100, 100), ImVec2(140, 136), 12.0f);
        float minX = 140.0f;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i)
            minX = std::min(minX, drawList->VtxBuffer[i].pos.x);
        CHECK(minX == doctest::Approx(100.0f - 8.0f).epsilon(0.001));
    });
}

TEST_CASE("Spinner draws eight bars inside the diameter and keeps requesting frames") {
    DrawFixture fixture;
    fixture.RunFrame([] {
        SoGui::ConsumeRedrawRequest();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 center(200.0f, 150.0f);
        for (float diameter : {12.0f, 16.0f, 20.0f, 40.0f}) {
            const int first = drawList->VtxBuffer.Size;
            SoGui::Spinner(drawList, center, diameter, IM_COL32(100, 100, 100, 255));
            CHECK(SoGui::ConsumeRedrawRequest());
            REQUIRE(drawList->VtxBuffer.Size > first);
            for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
                const ImVec2 p = drawList->VtxBuffer[i].pos;
                const float distance = std::hypot(p.x - center.x, p.y - center.y);
                CHECK(distance <= diameter * 0.5f + 0.5f);
            }
        }
    });
}

TEST_CASE("Spinner bar opacity falls from 1 to 0.15 and is staggered by 125ms") {
    DrawFixture fixture;
    fixture.RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Spinner(drawList, ImVec2(200.0f, 150.0f), 20.0f, IM_COL32(200, 200, 200, 255));

        const double time = std::fmod(ImGui::GetTime(), 1.0);
        std::multiset<int> actual;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i) {
            const int alpha = static_cast<int>((drawList->VtxBuffer[i].col >> IM_COL32_A_SHIFT) & 0xFFU);
            if (alpha > 0)
                actual.insert(alpha);
        }
        std::vector<int> expected;
        for (int i = 0; i < 8; ++i) {
            const double phase = time + 0.875 - 0.125 * i;
            const double alpha = 1.0 - 0.85 * (phase - std::floor(phase));
            expected.push_back(static_cast<int>(255.0 * alpha + 0.5));
        }
        // 允许 ±1 的取整差
        REQUIRE(actual.size() >= expected.size());
        for (int value : expected) {
            const bool found = std::any_of(actual.begin(), actual.end(), [&](int a) { return std::abs(a - value) <= 1; });
            INFO("expected alpha ", value);
            CHECK(found);
        }
        CHECK(*actual.begin() >= static_cast<int>(255 * 0.15) - 1);
        CHECK(*actual.rbegin() <= 255);
    });
}

} // namespace
