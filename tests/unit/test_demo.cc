#include "test_context.h"

#include <doctest/doctest.h>

#include <string>

namespace {

TEST_CASE_FIXTURE(ThemeFixture, "color demo exposes palette values and native previews in both themes") {
    std::string output;
    ImGuiPlatformIO& platform = ImGui::GetPlatformIO();
    platform.Platform_ClipboardUserData = &output;
    platform.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) {
        auto* buffer = static_cast<std::string*>(ImGui::GetPlatformIO().Platform_ClipboardUserData);
        *buffer = text;
    };
    platform.Platform_GetClipboardTextFn = [](ImGuiContext*) {
        return static_cast<std::string*>(ImGui::GetPlatformIO().Platform_ClipboardUserData)->c_str();
    };
    ImGui::GetIO().DisplaySize = ImVec2(1280.0f, 2400.0f);

    BeginFrame();
    SoGui::ShowDemoWindow();
    EndFrame();
    ImGui::SetWindowSize("So-gui demo", ImVec2(1100.0f, 2300.0f));
    ImGui::SetWindowPos("So-gui demo", ImVec2(0.0f, 0.0f));

    for (SoGui::ThemeId theme : {SoGui::ThemeId::Light, SoGui::ThemeId::Dark}) {
        SoGui::SetTheme(theme);
        SoGui::ConsumeRedrawRequest();
        BeginFrame();
        output.clear();
        ImGui::LogToClipboard();
        SoGui::ShowDemoWindow();
        ImGui::LogFinish();
        for (const char* label : {"gray",
                                  "gray-alpha",
                                  "blue",
                                  "red",
                                  "amber",
                                  "green",
                                  "teal",
                                  "purple",
                                  "pink",
                                  "Native ImGui preview",
                                  "Disable preview",
                                  "Button",
                                  "Checkbox",
                                  "Slider",
                                  "Input",
                                  "Overview",
                                  "Details",
                                  "Item 1",
                                  "Item 2",
                                  "Item 3",
                                  "Ready"}) {
            INFO("Missing preview: ", label);
            CHECK(output.find(label) != std::string::npos);
        }
        for (int i = 0; i < static_cast<int>(SoGui::Col::COUNT); ++i)
            CHECK(output.find(SoGui::GetColorName(static_cast<SoGui::Col>(i))) != std::string::npos);
        if (theme == SoGui::ThemeId::Light) {
            CHECK(output.find("#FFFFFFFF") != std::string::npos);
            CHECK(output.find("#F2F2F2CC") != std::string::npos);
        } else {
            CHECK(output.find("#0A0A0AFF") != std::string::npos);
            CHECK(output.find("#000000CC") != std::string::npos);
        }
        CHECK(SoGui::GetTheme() == theme);
        CHECK_FALSE(SoGui::ConsumeRedrawRequest());
        CHECK(ImGui::GetStyle().Alpha == 1.0f);
        EndFrame();
    }
    platform.Platform_ClipboardUserData = nullptr;
    platform.Platform_SetClipboardTextFn = nullptr;
    platform.Platform_GetClipboardTextFn = nullptr;
}

TEST_CASE_FIXTURE(ThemeFixture, "color demo content fits a tall window without logging") {
    ImGui::GetIO().DisplaySize = ImVec2(1280.0f, 2400.0f);
    BeginFrame();
    SoGui::ShowDemoWindow();
    EndFrame();
    ImGui::SetWindowPos("So-gui demo", ImVec2(0.0f, 0.0f));

    float previousHeight = 0.0f;
    for (float height : {1800.0f, 2300.0f}) {
        ImGui::SetWindowSize("So-gui demo", ImVec2(1100.0f, height));
        for (int frame = 0; frame < 3; ++frame) {
            BeginFrame();
            SoGui::ShowDemoWindow();
            EndFrame();
        }
        BeginFrame();
        SoGui::ShowDemoWindow();
        ImGui::Begin("So-gui demo");
        CHECK(ImGui::GetScrollMaxY() == 0.0f);
        const float contentHeight = ImGui::GetCursorPosY();
        if (previousHeight > 0.0f)
            CHECK(contentHeight == doctest::Approx(previousHeight));
        previousHeight = contentHeight;
        ImGui::End();
        EndFrame();
    }
}

} // namespace
