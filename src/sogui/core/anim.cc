#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>
#include <iterator>

namespace SoGui {
namespace {

struct MotionSpec {
    float duration; // 秒
    float x1, y1, x2, y2;
};

// Geist 的 transition-[...] duration-[150ms] ease-in-out
constexpr MotionSpec kMotions[] = {
    {0.15f, 0.42f, 0.0f, 0.58f, 1.0f},
};

static_assert(std::size(kMotions) == static_cast<size_t>(Motion::COUNT), "Every Motion needs a spec");

constexpr unsigned long long kStateLifetimeFrames = 60;

const MotionSpec& SpecOf(Motion motion) {
    const int index = static_cast<int>(motion);
    IM_ASSERT(index >= 0 && index < static_cast<int>(Motion::COUNT) && "Invalid Motion");
    return kMotions[index];
}

Context& CurrentContext() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "No current SoGui context. Did you call SoGui::CreateContext()?");
    return *ctx;
}

float Bezier(float a, float b, float t) {
    const float u = 1.0f - t;
    return 3.0f * a * u * u * t + 3.0f * b * u * t * t + t * t * t;
}

float BezierSlope(float a, float b, float t) {
    const float u = 1.0f - t;
    return 3.0f * a * u * u + 6.0f * (b - a) * u * t + 3.0f * (1.0f - b) * t * t;
}

ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

bool Equal(const ImVec4& a, const ImVec4& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

// 目标变化的那一帧不推进时间，之后每帧按 DeltaTime 推进；同一帧重复访问只推进一次
ImVec4 Step(ImGuiID id, const ImVec4& target, Motion motion) {
    Context& ctx = CurrentContext();
    AnimState* state = ctx.anims.GetByKey(id);
    if (state == nullptr) {
        state = ctx.anims.GetOrAddByKey(id);
        state->id = id;
        state->from = state->current = state->target = target;
        state->lastFrame = ctx.frameCount;
        return state->current;
    }

    const bool firstVisitThisFrame = state->lastFrame != ctx.frameCount;
    state->lastFrame = ctx.frameCount;
    if (!Equal(state->target, target)) {
        state->from = state->current;
        state->target = target;
        state->elapsed = 0.0f;
        state->animating = true;
    } else if (state->animating && firstVisitThisFrame) {
        state->elapsed += ImGui::GetIO().DeltaTime;
    }

    if (state->animating) {
        const MotionSpec& spec = SpecOf(motion);
        const float progress = state->elapsed / spec.duration;
        if (progress >= 1.0f) {
            state->current = state->target;
            state->animating = false;
        } else {
            const float eased = CubicBezier(spec.x1, spec.y1, spec.x2, spec.y2, progress);
            state->current = Lerp(state->from, state->target, eased);
        }
    }
    if (state->animating)
        RequestRedraw();
    return state->current;
}

} // namespace

// sogui_internal.h
float CubicBezier(float x1, float y1, float x2, float y2, float x) {
    if (x <= 0.0f)
        return 0.0f;
    if (x >= 1.0f)
        return 1.0f;

    // 牛顿迭代求 Bezier(x1, x2, t) = x，不收敛时改用二分
    constexpr float kEpsilon = 1e-6f;
    float t = x;
    for (int i = 0; i < 8; ++i) {
        const float error = Bezier(x1, x2, t) - x;
        if (std::fabs(error) < kEpsilon)
            return Bezier(y1, y2, t);
        const float slope = BezierSlope(x1, x2, t);
        if (std::fabs(slope) < 1e-6f)
            break;
        t -= error / slope;
    }

    float low = 0.0f;
    float high = 1.0f;
    t = x;
    for (int i = 0; i < 64; ++i) {
        const float value = Bezier(x1, x2, t);
        if (std::fabs(value - x) < kEpsilon)
            break;
        (value < x ? low : high) = t;
        t = (low + high) * 0.5f;
    }
    return Bezier(y1, y2, t);
}

void AnimEndFrame(ImPool<AnimState>& anims, unsigned long long frameCount) {
    for (int n = 0; n < anims.GetMapSize(); ++n) {
        AnimState* state = anims.TryGetMapData(n);
        if (state != nullptr && frameCount - state->lastFrame >= kStateLifetimeFrames)
            anims.Remove(state->id, state);
    }
}

int GetAnimStateCount() { return CurrentContext().anims.GetAliveCount(); }

// anim.h
float Animate(ImGuiID id, float target, Motion motion) { return Step(id, ImVec4(target, 0.0f, 0.0f, 0.0f), motion).x; }

ImVec4 AnimateColor(ImGuiID id, const ImVec4& target, Motion motion) {
    const float alpha = target.w;
    const ImVec4 premultiplied(target.x * alpha, target.y * alpha, target.z * alpha, alpha);
    const ImVec4 value = Step(id, premultiplied, motion);
    if (Equal(value, premultiplied))
        return target;
    if (value.w <= 0.0f)
        return ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    return ImVec4(value.x / value.w, value.y / value.w, value.z / value.w, value.w);
}

} // namespace SoGui
