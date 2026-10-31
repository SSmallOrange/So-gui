#include "test_context.h"

#include <doctest/doctest.h>

#include <cmath>
#include <initializer_list>

namespace {

using SoGui::Col;
using SoGui::Hue;
using SoGui::ThemeId;

void CheckColor(const ImVec4& actual, const ImVec4& expected) {
    CHECK(actual.x == doctest::Approx(expected.x).epsilon(0.0001));
    CHECK(actual.y == doctest::Approx(expected.y).epsilon(0.0001));
    CHECK(actual.z == doctest::Approx(expected.z).epsilon(0.0001));
    CHECK(actual.w == doctest::Approx(expected.w).epsilon(0.0001));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme initializes light colors before the first frame") {
    CHECK(SoGui::GetTheme() == ThemeId::Light);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CheckColor(SoGui::GetColor(Col::TextPrimary), ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 255));
}

TEST_CASE("theme initializes configured dark colors before the first frame") {
    ThemeFixture fixture({.theme = ThemeId::Dark});
    CHECK(SoGui::GetTheme() == ThemeId::Dark);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    CheckColor(SoGui::GetColor(Col::TextPrimary), ImVec4(0.93f, 0.93f, 0.93f, 1.0f));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(10, 10, 10, 255));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme switches resolved colors and style on the next frame") {
    BeginFrame();
    SoGui::SetTheme(ThemeId::Dark);
    CHECK(SoGui::GetTheme() == ThemeId::Dark);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CheckColor(SoGui::GetPaletteColor(Hue::Gray, 1), ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    EndFrame();

    BeginFrame();
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    CheckColor(SoGui::GetPaletteColor(Hue::Gray, 1), ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(10, 10, 10, 255));
    SoGui::SetTheme(ThemeId::Light);
    EndFrame();

    BeginFrame();
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 255));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme requests redraw only when the requested theme changes") {
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    SoGui::SetTheme(ThemeId::Light);
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    SoGui::SetTheme(ThemeId::Dark);
    CHECK(SoGui::ConsumeRedrawRequest());
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    SoGui::SetTheme(ThemeId::Dark);
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    BeginFrame();
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    SoGui::SetTheme(ThemeId::Light);
    CHECK(SoGui::ConsumeRedrawRequest());
}

TEST_CASE_FIXTURE(ThemeFixture, "theme keeps the last request made before a frame") {
    SoGui::SetTheme(ThemeId::Dark);
    SoGui::SetTheme(ThemeId::Light);
    BeginFrame();
    CHECK(SoGui::GetTheme() == ThemeId::Light);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme resolves focus and backdrop differently in each theme") {
    CheckColor(SoGui::GetColor(Col::FocusRing), ImVec4(0.0f, 0.448f, 0.96f, 1.0f));
    CheckColor(SoGui::GetColor(Col::Backdrop), ImVec4(0.95f, 0.95f, 0.95f, 0.8f));
    SoGui::SetTheme(ThemeId::Dark);
    BeginFrame();
    CheckColor(SoGui::GetColor(Col::FocusRing), ImVec4(0.32f, 0.66f, 1.0f, 1.0f));
    CheckColor(SoGui::GetColor(Col::Backdrop), ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme restores nested overrides and packed color caches in reverse order") {
    SoGui::PushColor(Col::Bg1, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 0, 0, 128));
    SoGui::PushColor(Col::Bg1, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    SoGui::PushColor(Col::Fill, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(0, 255, 0, 255));
    CHECK(SoGui::GetColorU32(Col::Fill) == IM_COL32(0, 0, 255, 255));

    SoGui::PopColor(2);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 0, 0, 128));
    CheckColor(SoGui::GetColor(Col::Fill), ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Fill) == IM_COL32(242, 242, 242, 255));
    SoGui::PopColor(0);
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 0, 0, 128));
    SoGui::PopColor();
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 255));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme overrides leave the palette and ImGui style unchanged") {
    SoGui::PushColor(Col::Fill, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    CheckColor(SoGui::GetPaletteColor(Hue::Gray, 1), ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    CHECK(SoGui::GetPaletteColorU32(Hue::Gray, 1) == IM_COL32(242, 242, 242, 255));
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_Button], ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    SoGui::PopColor();
}

TEST_CASE_FIXTURE(ThemeFixture, "theme packed colors combine source alpha multiplier and style alpha") {
    SoGui::PushColor(Col::Fill, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
    CHECK(SoGui::GetColorU32(Col::Fill) == IM_COL32(255, 0, 0, 128));
    CHECK(SoGui::GetColorU32(Col::Fill, 0.5f) == IM_COL32(255, 0, 0, 64));
    ImGui::GetStyle().Alpha = 0.5f;
    CHECK(SoGui::GetColorU32(Col::Fill) == IM_COL32(255, 0, 0, 64));
    CHECK(SoGui::GetColorU32(Col::Fill, 0.5f) == IM_COL32(255, 0, 0, 32));
    CHECK(SoGui::GetColorU32(Col::Fill, 0.0f) == IM_COL32(255, 0, 0, 0));
    CHECK(SoGui::GetPaletteColorU32(Hue::Gray, 1, 0.5f) == IM_COL32(242, 242, 242, 64));
    SoGui::PopColor();
}

TEST_CASE_FIXTURE(ThemeFixture, "theme packed colors honor disabled scopes without changing cached colors") {
    BeginFrame();
    ImGui::GetStyle().Alpha = 0.5f;
    ImGui::GetStyle().DisabledAlpha = 0.5f;
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 128));
    ImGui::BeginDisabled();
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 64));
    CHECK(SoGui::GetPaletteColorU32(Hue::Gray, 1) == IM_COL32(242, 242, 242, 64));
    ImGui::BeginDisabled();
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 64));
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 128));
    ImGui::GetStyle().Alpha = 1.0f;
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 255, 255, 255));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme mapping fills every ImGui color slot but preserves non-color style") {
    ImGuiStyle style;
    style.Alpha = 0.5f;
    style.WindowPadding = ImVec2(13.0f, 17.0f);
    style.FrameRounding = 9.0f;
    for (ImVec4& color : style.Colors)
        color = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
    SoGui::ApplyToImGuiStyle(style);
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        CAPTURE(i);
        const ImVec4 color = style.Colors[i];
        for (float channel : {color.x, color.y, color.z, color.w}) {
            CHECK(std::isfinite(channel));
            CHECK(channel >= 0.0f);
            CHECK(channel <= 1.0f);
        }
    }
    CHECK(style.Alpha == 0.5f);
    CHECK(style.WindowPadding.x == 13.0f);
    CHECK(style.WindowPadding.y == 17.0f);
    CHECK(style.FrameRounding == 9.0f);
    CheckColor(style.Colors[ImGuiCol_Button], ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
    CheckColor(style.Colors[ImGuiCol_Text], ImVec4(0.09f, 0.09f, 0.09f, 1.0f));
    CheckColor(style.Colors[ImGuiCol_ChildBg], ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    CheckColor(style.Colors[ImGuiCol_DockingPreview], ImVec4(0.0f, 0.448f, 0.96f, 0.4f));
    CheckColor(style.Colors[ImGuiCol_NavWindowingHighlight], ImVec4(0.09f, 0.09f, 0.09f, 0.7f));

    SoGui::SetTheme(ThemeId::Dark);
    BeginFrame();
    SoGui::ApplyToImGuiStyle(style);
    CheckColor(style.Colors[ImGuiCol_NavCursor], ImVec4(0.32f, 0.66f, 1.0f, 1.0f));
    CheckColor(style.Colors[ImGuiCol_ModalWindowDimBg], ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    CheckColor(style.Colors[ImGuiCol_TableRowBg], ImVec4(0.04f, 0.04f, 0.04f, 0.0f));
}

TEST_CASE("theme automatic style mapping can be disabled") {
    ThemeFixture fixture({.applyToImGuiStyle = false});
    const ImGuiStyle defaults;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        CheckColor(ImGui::GetStyle().Colors[i], defaults.Colors[i]);
    SoGui::SetTheme(ThemeId::Dark);
    fixture.BeginFrame();
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
        CheckColor(ImGui::GetStyle().Colors[i], defaults.Colors[i]);
    CheckColor(SoGui::GetColor(Col::Bg1), ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
    SoGui::ApplyToImGuiStyle(ImGui::GetStyle());
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_WindowBg], ImVec4(0.04f, 0.04f, 0.04f, 1.0f));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme preserves host style adjustments until a theme switch") {
    const ImVec4 custom(1.0f, 0.0f, 1.0f, 0.5f);
    ImGui::GetStyle().Colors[ImGuiCol_Button] = custom;
    BeginFrame();
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_Button], custom);
    SoGui::SetTheme(ThemeId::Light);
    EndFrame();
    BeginFrame();
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_Button], custom);
    SoGui::SetTheme(ThemeId::Dark);
    EndFrame();
    BeginFrame();
    CheckColor(ImGui::GetStyle().Colors[ImGuiCol_Button], ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
}

TEST_CASE_FIXTURE(ThemeFixture, "theme contexts retain independent colors overrides and redraw requests") {
    SoGui::Context* other = SoGui::CreateContext({.theme = ThemeId::Dark, .applyToImGuiStyle = false});
    CHECK(SoGui::GetCurrentContext() == soguiContext);
    CHECK(SoGui::GetTheme() == ThemeId::Light);
    SoGui::PushColor(Col::Bg1, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    SoGui::SetTheme(ThemeId::Dark);

    SoGui::SetCurrentContext(other);
    CHECK(SoGui::GetTheme() == ThemeId::Dark);
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(10, 10, 10, 255));
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    SoGui::SetCurrentContext(soguiContext);
    CHECK(SoGui::GetColorU32(Col::Bg1) == IM_COL32(255, 0, 0, 255));
    CHECK(SoGui::ConsumeRedrawRequest());
    SoGui::PopColor();
    SoGui::DestroyContext(other);
    CHECK(SoGui::GetCurrentContext() == soguiContext);
}

} // namespace
