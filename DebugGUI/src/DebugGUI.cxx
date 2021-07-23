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

// fills a stream with drawing data in JSON format
// the JSON is composed of a list of draw commands
/// FIXME: document the actual schema of the format.
void getFrameJSON(void *data, std::ostream& json_data)
{
  auto draw_data = (ImDrawData*)data;

  json_data << "[";

  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];
    const auto vtx_buffer = cmd_list->VtxBuffer;
    const auto idx_buffer = cmd_list->IdxBuffer;
    const auto cmd_buffer = cmd_list->CmdBuffer;

    json_data << "{\"vtx\":[";
    for (int i = 0; i < vtx_buffer.size(); ++i) {
      auto v = vtx_buffer[i];
      json_data << "[" << v.pos.x << "," << v.pos.y << "," << v.col << ',' << v.uv.x << "," << v.uv.y << "]";
      if (i < vtx_buffer.size() - 1) json_data << ",";
    }

    json_data << "],\"idx\":[";
    for (int i = 0; i < idx_buffer.size(); ++i) {
      auto id = idx_buffer[i];
      json_data << id;
      if (i < idx_buffer.size() - 1) json_data << ",";
    }

    json_data << "],\"cmd\":[";
    for (int i = 0; i < cmd_buffer.size(); ++i) {
      auto cmd = cmd_buffer[i];
      json_data << "{\"cnt\":" << cmd.ElemCount << ", \"clp\":[" << cmd.ClipRect.x << "," << cmd.ClipRect.y << "," << cmd.ClipRect.z << "," << cmd.ClipRect.w << "]}";
      if (i < cmd_buffer.size() - 1) json_data << ",";
    }
    json_data << "]}";
    if (cmd_id < draw_data->CmdListsCount - 1) json_data << ",";
  }

  json_data << "]";
}

bool pollGUI_gl_init(void* context)
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

  return true;
}

/// @return draw data as void* to avoid dependencies
void *pollGUI_render(std::function<void(void)> guiCallback)
{
  // This is where the magic actually happens...
  if (guiCallback) {
    guiCallback();
  }
  ImGui::Render();

  return ImGui::GetDrawData();
}

void pollGUI_gl_end(void* context, void *draw_data)
{
  GLFWwindow* window = reinterpret_cast<GLFWwindow*>(context);

  ImGui_ImplGlfwGL3_RenderDrawLists((ImDrawData*)draw_data);
  glfwSwapBuffers(window);
}

/// @return true if we do not need to exit, false if we do.
bool pollGUI(void* context, std::function<void(void)> guiCallback)
{
  if (!pollGUI_gl_init(context)) {
    return false;
  }
  auto draw_data = pollGUI_render(guiCallback);
  pollGUI_gl_end(context, draw_data);
  
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
