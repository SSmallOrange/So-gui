#include <sogui/sogui.h>
#include <sogui/app.h>

int main() {
    return SoGui::RunApp({
        .title = "So-gui minimal",
        .onFrame = [] {
            ImGui::ShowDemoWindow();
            SoGui::ShowDemoWindow();
        },
    });
}
