#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

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

TEST_CASE_FIXTURE(ThemeFixture, "IconDraw centers the em box at the requested size") {
    REQUIRE(SoGui::AddIconFont({kLucide}));
    RunFrame([] {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const SoGui::FontEntry* entry = SoGui::FindIconFont();
        REQUIRE(entry != nullptr);
        constexpr float kSize = 20.0f;
        const ImVec2 center(100.0f, 100.0f);
        // chevron-down 扁、check 接近方形：都按同一个 em 方框放置，字号不随字形边界框变化
        for (const IconId id : {IconId::ChevronDown, IconId::Check}) {
            CAPTURE(static_cast<unsigned>(id));
            const int first = drawList->VtxBuffer.Size;
            SoGui::IconDraw(drawList, center, id, kSize, IM_COL32_WHITE);
            REQUIRE(drawList->VtxBuffer.Size > first);
            ImVec2 quadMin = drawList->VtxBuffer[first].pos;
            ImVec2 quadMax = quadMin;
            for (int i = first + 1; i < drawList->VtxBuffer.Size; ++i) {
                quadMin = ImMin(quadMin, drawList->VtxBuffer[i].pos);
                quadMax = ImMax(quadMax, drawList->VtxBuffer[i].pos);
            }

            ImGui::PushFont(entry->font, kSize * entry->heightPerEm);
            ImFontBaked* baked = ImGui::GetFontBaked();
            const float pixelSize = ImGui::GetFontSize();
            const float scale = pixelSize / baked->Size;
            const float em = pixelSize / entry->heightPerEm;
            const float baseline = baked->Ascent * scale;
            const ImFontGlyph* glyph = baked->FindGlyph(static_cast<ImWchar>(static_cast<unsigned>(id)));
            ImGui::PopFont();
            REQUIRE(glyph != nullptr);

            // 四边形尺寸等于字形在 kSize 字号下的尺寸，即没有按字形边界框换算字号
            CHECK(quadMax.x - quadMin.x == doctest::Approx((glyph->X1 - glyph->X0) * scale));
            CHECK(quadMax.y - quadMin.y == doctest::Approx((glyph->Y1 - glyph->Y0) * scale));
            // 由贴图四边形反推绘制位置 pos，em 方框 pos + (0, baseline - em) ~ (em, baseline) 的中心应落在 center
            const ImVec2 pos(quadMin.x - glyph->X0 * scale, quadMin.y - glyph->Y0 * scale);
            CHECK(pos.x + em * 0.5f == doctest::Approx(center.x));
            CHECK(pos.y + baseline - em * 0.5f == doctest::Approx(center.y));
        }
    });
}

} // namespace

