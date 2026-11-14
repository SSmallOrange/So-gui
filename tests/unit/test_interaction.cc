#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

namespace {

using SoGui::InteractionState;

constexpr float kItemWidth = 100.0f;
constexpr float kItemHeight = 40.0f;

struct InteractionFixture : ThemeFixture {
    SoGui::Interaction last;
    ImVec2 center;
    bool disabled = false;

    void Frame() {
        RunFrame([&] {
            if (disabled)
                ImGui::BeginDisabled();
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            const ImGuiID id = window->GetID("item");
            const ImVec2 min = window->DC.CursorPos;
            const ImRect bb(min, ImVec2(min.x + kItemWidth, min.y + kItemHeight));
            center = ImVec2(min.x + kItemWidth * 0.5f, min.y + kItemHeight * 0.5f);
            ImGui::ItemSize(bb);
            if (ImGui::ItemAdd(bb, id))
                last = SoGui::ResolveInteraction(bb, id);
            if (disabled)
                ImGui::EndDisabled();
        });
    }

    // 键盘导航到第一个控件
    void TabToItem() {
        PressKey(ImGuiKey_Tab);
        Frame();
        PressKey(ImGuiKey_Tab, false);
        Frame();
        Frame();
    }
};

TEST_CASE_FIXTURE(InteractionFixture, "interaction follows hover, press and release") {
    Frame();
    CHECK(last.state == InteractionState::Default);

    MoveMouse(center);
    Frame();
    CHECK(last.state == InteractionState::Hover);
    CHECK_FALSE(last.clicked);

    MouseDown();
    Frame();
    CHECK(last.state == InteractionState::Active);
    CHECK_FALSE(last.clicked);

    MouseUp();
    Frame();
    CHECK(last.clicked);
    CHECK(last.state == InteractionState::Hover);

    Frame();
    CHECK_FALSE(last.clicked);

    MoveMouse(ImVec2(center.x + 500.0f, center.y));
    Frame();
    CHECK(last.state == InteractionState::Default);
}

TEST_CASE_FIXTURE(InteractionFixture, "releasing outside the item does not click") {
    Frame();
    MoveMouse(center);
    Frame();
    MouseDown();
    Frame();
    MoveMouse(ImVec2(center.x + 500.0f, center.y));
    Frame();
    CHECK_FALSE(last.clicked);
    CHECK(last.state != InteractionState::Active);
    MouseUp();
    Frame();
    CHECK_FALSE(last.clicked);
}

TEST_CASE_FIXTURE(InteractionFixture, "disabled items never click or hover") {
    disabled = true;
    Frame();
    CHECK(last.state == InteractionState::Disabled);

    MoveMouse(center);
    Frame();
    CHECK(last.state == InteractionState::Disabled);
    MouseDown();
    Frame();
    CHECK(last.state == InteractionState::Disabled);
    CHECK_FALSE(last.clicked);
    MouseUp();
    Frame();
    CHECK_FALSE(last.clicked);
    CHECK(last.state == InteractionState::Disabled);
}

TEST_CASE_FIXTURE(InteractionFixture, "keyboard focus shows the focus ring until the mouse is used") {
    Frame();
    CHECK_FALSE(last.focusVisible);

    TabToItem();
    CHECK(last.focusVisible);
    CHECK(last.state == InteractionState::Default);

    // 键盘导航后第一次移动鼠标会清除 NavHighlightItemUnderNav，第二次才产生 hover
    MoveMouse(ImVec2(center.x + 2.0f, center.y));
    Frame();
    MoveMouse(center);
    Frame();
    CHECK(last.focusVisible);
    CHECK(last.state == InteractionState::Hover);

    MouseDown();
    Frame();
    MouseUp();
    Frame();
    CHECK(last.clicked);
    CHECK_FALSE(last.focusVisible);
}

TEST_CASE_FIXTURE(InteractionFixture, "space and enter activate the focused item") {
    Frame();
    TabToItem();
    REQUIRE(last.focusVisible);

    PressKey(ImGuiKey_Space);
    Frame();
    CHECK(last.clicked);
    PressKey(ImGuiKey_Space, false);
    Frame();
    CHECK_FALSE(last.clicked);

    PressKey(ImGuiKey_Enter);
    Frame();
    CHECK(last.clicked);
    PressKey(ImGuiKey_Enter, false);
    Frame();
    CHECK_FALSE(last.clicked);
}

TEST_CASE_FIXTURE(InteractionFixture, "disabled items cannot be focused with Tab") {
    disabled = true;
    Frame();
    TabToItem();
    CHECK_FALSE(last.focusVisible);
}

TEST_CASE_FIXTURE(InteractionFixture, "item base alpha cancels the disabled alpha") {
    Frame();
    ImGui::GetStyle().Alpha = 0.8f;
    BeginFrame();
    ImGui::Begin("Alpha");
    CHECK(SoGui::ItemBaseAlpha() == doctest::Approx(0.8f));
    ImGui::BeginDisabled();
    CHECK(ImGui::GetStyle().Alpha == doctest::Approx(0.8f * ImGui::GetStyle().DisabledAlpha));
    CHECK(SoGui::ItemBaseAlpha() == doctest::Approx(0.8f));
    ImGui::BeginDisabled();
    CHECK(SoGui::ItemBaseAlpha() == doctest::Approx(0.8f));
    ImGui::EndDisabled();
    ImGui::EndDisabled();
    CHECK(SoGui::ItemBaseAlpha() == doctest::Approx(0.8f));
    ImGui::End();
    EndFrame();
}

} // namespace
