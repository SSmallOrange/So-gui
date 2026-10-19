#include <sogui/app.h>

namespace SoGui {

// 由 backends/glfw_opengl3.cc 实现
namespace backend {
int RunGlfwOpengl3(const AppConfig& config);
}

int RunApp(const AppConfig& config) {
    return backend::RunGlfwOpengl3(config);
}

} // namespace SoGui
