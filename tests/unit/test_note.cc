#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace {

using SoGui::Hue;
using SoGui::NoteOptions;
using SoGui::NoteSize;
using SoGui::NoteType;

std::string FontPath(const char* name) { return std::string(SOGUI_TEST_FONT_DIR) + "/" + name; }

struct NoteFixture : ThemeFixture {
    ImVec2 min;
    ImVec2 max;
    float avail = 0.0f;
    std::vector<ImU32> colors;

    explicit NoteFixture() {
        REQUIRE(SoGui::AddFont(SoGui::FontWeight::Regular, {FontPath("Geist-Regular.ttf").c_str()}));
        REQUIRE(SoGui::AddFont(SoGui::FontWeight::SemiBold, {FontPath("Geist-SemiBold.ttf").c_str()}));
        REQUIRE(SoGui::AddIconFont({SOGUI_TEST_LUCIDE_FONT_DIR "/lucide.ttf"}));
    }

    void Frame(const NoteOptions& options, const char* label = "Note", const char* content = "This is a note.") {
        RunFrame([&] {
            const ImDrawList* drawList = ImGui::GetWindowDrawList();
            const int first = drawList->VtxBuffer.Size;
            SoGui::Note(label, content, options);
            min = ImGui::GetItemRectMin();
            max = ImGui::GetItemRectMax();
            avail = ImGui::GetContentRegionAvail().x;
            colors.clear();
            for (int i = first; i < drawList->VtxBuffer.Size; ++i)
                colors.push_back(drawList->VtxBuffer[i].col);
        });
    }

    bool Has(ImU32 color) const { return std::find(colors.begin(), colors.end(), color) != colors.end(); }
};

TEST_CASE_FIXTURE(NoteFixture, "Note spans the content width with a minimum height") {
    Frame({.type = NoteType::Default});
    CHECK(max.x - min.x == doctest::Approx(avail));
    CHECK(max.y - min.y == doctest::Approx(36.0f).epsilon(0.01)); // min-h 36

    Frame({.type = NoteType::Default, .size = NoteSize::Small});
    CHECK(max.y - min.y == doctest::Approx(32.0f).epsilon(0.01)); // min-h 32
}

TEST_CASE_FIXTURE(NoteFixture, "Note types use the documented colors") {
    // default：border gray-400 + fg gray-900
    Frame({.type = NoteType::Default});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 4)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Gray, 9)));
    // error：border red-400 + fg red-900
    Frame({.type = NoteType::Error});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Red, 4)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Red, 9)));
    // warning：border amber-400 + fg amber-900
    Frame({.type = NoteType::Warning});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Amber, 4)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Amber, 9)));
    // secondary：bg gray-alpha-200 + fg gray-alpha-900，无边框
    Frame({.type = NoteType::Secondary});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::GrayAlpha, 2)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::GrayAlpha, 9)));
}

TEST_CASE_FIXTURE(NoteFixture, "Note filled variants use the lighter background") {
    // default filled 用蓝色系：bg blue-200 + border blue-100（官网行为）
    Frame({.type = NoteType::Default, .filled = true});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Blue, 2)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Blue, 1)));
    // error filled：bg red-200 + border red-100
    Frame({.type = NoteType::Error, .filled = true});
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Red, 2)));
    CHECK(Has(SoGui::GetPaletteColorU32(Hue::Red, 1)));
}

TEST_CASE_FIXTURE(NoteFixture, "Note grows with wrapped content") {
    Frame({.type = NoteType::Default}, "", "Short.");
    const float single = max.y - min.y;
    Frame({.type = NoteType::Default}, "",
          "This is a much longer note content that will definitely wrap onto several lines in a 400 pixel window.");
    const float wrapped = max.y - min.y;
    CHECK(wrapped > single);
}

TEST_CASE_FIXTURE(NoteFixture, "Note with empty label renders only the content") {
    Frame({.type = NoteType::Default}, "", "Content only.");
    CHECK(max.y - min.y == doctest::Approx(36.0f).epsilon(0.01));
}

} // namespace

