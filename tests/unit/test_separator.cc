#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <vector>

namespace {

using SoGui::Hue;
using SoGui::ThemeId;

TEST_CASE_FIXTURE(ThemeFixture, "Separator is a 1px horizontal line spanning the content width") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Separator();
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        CHECK(max.x - min.x == doctest::Approx(ImGui::GetContentRegionAvail().x));
        CHECK(max.y - min.y == doctest::Approx(1.0f));

        const ImU32 expected = SoGui::GetPaletteColorU32(Hue::Gray, 2);
        bool found = false;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i)
            found |= drawList->VtxBuffer[i].col == expected;
        CHECK(found);
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Separator thickness scales with the UI scale") {
    RunFrame([] {
        ImGui::GetStyle().FontScaleMain = 2.0f;
        SoGui::Separator();
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        CHECK(max.y - min.y == doctest::Approx(2.0f));
    });
}

TEST_CASE_FIXTURE(ThemeFixture, "Separator color follows the theme") {
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Separator();
        bool found = false;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i)
            found |= drawList->VtxBuffer[i].col == SoGui::GetPaletteColorU32(Hue::Gray, 2);
        CHECK(found);
    });
    SoGui::SetTheme(ThemeId::Dark);
    RunFrame([] {
        const ImDrawList* drawList = ImGui::GetWindowDrawList();
        const int first = drawList->VtxBuffer.Size;
        SoGui::Separator();
        bool found = false;
        for (int i = first; i < drawList->VtxBuffer.Size; ++i)
            found |= drawList->VtxBuffer[i].col == SoGui::GetPaletteColorU32(Hue::Gray, 2);
        CHECK(found);
    });
}

} // namespace
