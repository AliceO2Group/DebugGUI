#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw_gl3.h"
#include "icons_font_awesome.h"
// Needed by icons_font_awesome.ttf.h
#include <cstdint>
#include "icons_font_awesome.ttf.h"
#include "GL/gl3w.h" // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <cstdio>
#include <functional>

static void default_error_callback(int error, const char* description)
{
  fprintf(stderr, "Error %d: %s\n", error, description);
}

namespace o2
{
namespace framework
{

// @return an object of kind GLFWwindow* as void* to avoid having a direct dependency
void* initGUI(const char* name, void(*error_callback)(int, char const*description))
{
  // Setup window
  if (error_callback == nullptr) {
    glfwSetErrorCallback(default_error_callback);
  }
  if (!glfwInit())
    return nullptr;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  GLFWwindow* window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);
  glfwMakeContextCurrent(window);
  gl3wInit();

  // Setup ImGui binding
  ImGui_ImplGlfwGL3_Init(window, true);

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true; icons_config.FontDataOwnedByAtlas = false;
  io.Fonts->AddFontFromMemoryTTF((void*)s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), 12.0f, &icons_config, icons_ranges);

  ImPlot::CreateContext();
  return window;
}

/// @return true if we do not need to exit, false if we do.
bool pollGUI(void* context, std::function<void(void)> guiCallback)
{
  GLFWwindow* window = reinterpret_cast<GLFWwindow*>(context);
  if (glfwWindowShouldClose(window)) {
    return false;
  }
  glfwPollEvents();
  ImGui_ImplGlfwGL3_NewFrame();


  // Rendering
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  ImVec4 clear_color = ImColor(114, 144, 154);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  // This is where the magic actually happens...
  if (guiCallback) {
    guiCallback();
  }
  ImGui::Render();
  ImGui_ImplGlfwGL3_RenderDrawLists(ImGui::GetDrawData());
  glfwSwapBuffers(window);
  return true;
}

void disposeGUI()
{
  ImPlot::DestroyContext();
  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();
}

} // namespace framework
} // namespace o2
