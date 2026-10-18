#include <sogui/sogui.hpp>
#include <sogui/app/app.hpp>

int main() {
    return SoGui::RunApp({
        .title = "So-gui minimal",
        .onFrame = [] {
            ImGui::ShowDemoWindow();
            SoGui::ShowDemoWindow();
        },
    });
}
