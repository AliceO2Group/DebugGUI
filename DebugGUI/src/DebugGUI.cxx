#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GL/gl3w.h"
#include "icons_font_awesome.h"
// Needed by icons_font_awesome.ttf.h
#include <cstdint>
#include "icons_font_awesome.ttf.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <functional>
#include <ostream>

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
  GLFWwindow* window = nullptr;
  if (name) {
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
    window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
  } else {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    // This is needed to support more than 2**16 vertices
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
  }

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true; icons_config.FontDataOwnedByAtlas = false;
  icons_config.GlyphOffset = {0, 2};
  icons_config.GlyphMinAdvanceX = 17;
  io.Fonts->AddFontFromMemoryTTF((void *)s_iconsFontAwesomeTtf,
                                 sizeof(s_iconsFontAwesomeTtf), 13.0f,
                                 &icons_config, icons_ranges);

  // this initializes the texture
  if (io.Fonts->ConfigData.empty())
    io.Fonts->AddFontDefault();
  io.Fonts->Build();
  io.DisplaySize = ImVec2(1280, 720);
  
  ImPlot::CreateContext();
  return window;
}

bool pollGUIPreRender(void* context, float delta)
{
  if (context) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(context);

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Clearing the viewport
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImVec4 clear_color = ImColor(114, 144, 154);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    if (glfwWindowShouldClose(window)) {
      return false;
    }
  } else {
    // Just initialize new frame
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = delta;
    ImGui::NewFrame();
  }


  return true;
}

/// @return draw data as void* to avoid dependencies
void *pollGUIRender(std::function<void(void)> guiCallback)
{
  // This is where the magic actually happens...
  if (guiCallback) {
    guiCallback();
  }
  ImGui::Render();

  return ImGui::GetDrawData();
}

void pollGUIPostRender(void* context, void *draw_data)
{
  if (context) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(context);

    ImGui_ImplOpenGL3_RenderDrawData((ImDrawData*)draw_data);
    glfwSwapBuffers(window);
  }
}
  
/// @return true if we do not need to exit, false if we do.
bool pollGUI(void* context, std::function<void(void)> guiCallback)
{
  if (!pollGUIPreRender(context, 1.0f/60.0f)) {
    return false;
  }
  void *draw_data = pollGUIRender(guiCallback);
  pollGUIPostRender(context, draw_data);
  
  return true;
}

void disposeGUI()
{
  ImPlot::DestroyContext();
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  glfwTerminate();
}

} // namespace framework
} // namespace o2
