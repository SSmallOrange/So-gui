#include "test_context.h"

#include <sogui/sogui_internal.h>

#include <doctest/doctest.h>

namespace {

using SoGui::Motion;

constexpr ImGuiID kId = 0x1234;

float Ease(float x) { return SoGui::CubicBezier(0.42f, 0.0f, 0.58f, 1.0f, x); }

// 跑一帧并返回 Animate 的结果
struct AnimFixture : ThemeFixture {
    float StepFloat(float target, float deltaTime = 0.075f) {
        ImGui::GetIO().DeltaTime = deltaTime;
        BeginFrame();
        const float value = SoGui::Animate(kId, target, Motion::Control);
        EndFrame();
        return value;
    }

    ImVec4 StepColor(const ImVec4& target, float deltaTime = 0.075f) {
        ImGui::GetIO().DeltaTime = deltaTime;
        BeginFrame();
        const ImVec4 value = SoGui::AnimateColor(kId, target, Motion::Control);
        EndFrame();
        return value;
    }
};

TEST_CASE("ease-in-out follows the CSS cubic-bezier curve") {
    CHECK(Ease(0.0f) == doctest::Approx(0.0f));
    CHECK(Ease(0.25f) == doctest::Approx(0.1292f).epsilon(0.002));
    CHECK(Ease(0.5f) == doctest::Approx(0.5f).epsilon(0.002));
    CHECK(Ease(0.75f) == doctest::Approx(0.8708f).epsilon(0.002));
    CHECK(Ease(1.0f) == doctest::Approx(1.0f));
}

TEST_CASE("cubic-bezier handles curves where Newton iteration is unreliable") {
    // 起点斜率为 0 的曲线
    for (float x : {0.05f, 0.3f, 0.6f, 0.95f}) {
        const float y = SoGui::CubicBezier(0.0f, 0.0f, 1.0f, 1.0f, x);
        CHECK(y >= 0.0f);
        CHECK(y <= 1.0f);
    }
    CHECK(SoGui::CubicBezier(0.0f, 0.0f, 1.0f, 1.0f, 0.5f) == doctest::Approx(0.5f));
}

TEST_CASE_FIXTURE(AnimFixture, "animate returns the target on first use") {
    CHECK(StepFloat(3.0f) == doctest::Approx(3.0f));
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    CHECK(StepFloat(3.0f) == doctest::Approx(3.0f));
}

TEST_CASE_FIXTURE(AnimFixture, "animate starts from the old value and reaches the target in 150ms") {
    CHECK(StepFloat(0.0f) == doctest::Approx(0.0f));
    SoGui::ConsumeRedrawRequest();

    CHECK(StepFloat(1.0f) == doctest::Approx(0.0f));
    CHECK(SoGui::ConsumeRedrawRequest());
    CHECK(StepFloat(1.0f) == doctest::Approx(0.5f));
    CHECK(SoGui::ConsumeRedrawRequest());
    CHECK(StepFloat(1.0f) == doctest::Approx(1.0f));
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
    CHECK(StepFloat(1.0f) == doctest::Approx(1.0f));
    CHECK_FALSE(SoGui::ConsumeRedrawRequest());
}

TEST_CASE_FIXTURE(AnimFixture, "a long idle delta time does not skip the transition") {
    StepFloat(0.0f);
    CHECK(StepFloat(1.0f, 10.0f) == doctest::Approx(0.0f));
    CHECK(StepFloat(1.0f) == doctest::Approx(0.5f));
}

TEST_CASE_FIXTURE(AnimFixture, "retargeting mid transition continues from the displayed value") {
    StepFloat(0.0f);
    StepFloat(1.0f);
    CHECK(StepFloat(1.0f) == doctest::Approx(0.5f));
    CHECK(StepFloat(0.0f) == doctest::Approx(0.5f));
    CHECK(StepFloat(0.0f) == doctest::Approx(0.25f));
    CHECK(StepFloat(0.0f) == doctest::Approx(0.0f));
}

TEST_CASE_FIXTURE(AnimFixture, "animate advances once per frame when visited repeatedly") {
    StepFloat(0.0f);
    StepFloat(1.0f);
    ImGui::GetIO().DeltaTime = 0.075f;
    BeginFrame();
    CHECK(SoGui::Animate(kId, 1.0f, Motion::Control) == doctest::Approx(0.5f));
    CHECK(SoGui::Animate(kId, 1.0f, Motion::Control) == doctest::Approx(0.5f));
    EndFrame();
}

TEST_CASE_FIXTURE(AnimFixture, "colors interpolate in premultiplied space") {
    const ImVec4 transparent(0.0f, 0.0f, 0.0f, 0.0f);
    const ImVec4 target(0.2f, 0.4f, 0.6f, 1.0f);
    const ImVec4 start = StepColor(transparent);
    CHECK(start.w == doctest::Approx(0.0f));

    StepColor(target);
    const ImVec4 middle = StepColor(target);
    CHECK(middle.x == doctest::Approx(target.x));
    CHECK(middle.y == doctest::Approx(target.y));
    CHECK(middle.z == doctest::Approx(target.z));
    CHECK(middle.w == doctest::Approx(0.5f));

    const ImVec4 end = StepColor(target);
    CHECK(end.x == target.x);
    CHECK(end.w == target.w);
}

TEST_CASE_FIXTURE(AnimFixture, "states unused for 60 frames are recycled") {
    StepFloat(1.0f);
    CHECK(SoGui::GetAnimStateCount() == 1);
    for (int frame = 0; frame < 59; ++frame) {
        BeginFrame();
        EndFrame();
    }
    CHECK(SoGui::GetAnimStateCount() == 1);
    BeginFrame();
    EndFrame();
    CHECK(SoGui::GetAnimStateCount() == 0);

    // 访问会刷新寿命
    StepFloat(1.0f);
    for (int frame = 0; frame < 59; ++frame) {
        BeginFrame();
        EndFrame();
    }
    StepFloat(1.0f);
    for (int frame = 0; frame < 59; ++frame) {
        BeginFrame();
        EndFrame();
    }
    CHECK(SoGui::GetAnimStateCount() == 1);
}

} // namespace
