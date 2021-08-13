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
    ImGui_ImplGlfwGL3_Init(window, true);
  } else {
    ImGui::CreateContext();
  }

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true; icons_config.FontDataOwnedByAtlas = false;
  io.Fonts->AddFontFromMemoryTTF((void*)s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), 12.0f, &icons_config, icons_ranges);
  
  // this initializes the texture
  if (io.Fonts->ConfigData.empty())
    io.Fonts->AddFontDefault();
  io.Fonts->Build();
  io.DisplaySize = ImVec2(1280, 720);
  
  ImPlot::CreateContext();
  return window;
}

// fills a stream with drawing data in JSON format
// the JSON is composed of a list of draw commands
/// FIXME: document the actual schema of the format.
void getFrameJSON(void *data, std::ostream& json_data)
{
  ImDrawData* draw_data = (ImDrawData*)data;

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


struct vtxContainer {
  float posX, posY, uvX, uvY;
  int col;
};

struct cmdContainer {
  int count;
  float rectX, rectY, rectZ, rectW;
};

// given draw data, returns a mallocd buffer containing formatted frame data
// ready to be sent and its size.
// the returned buffer must be freed by the caller.
/// FIXME: document actual schema of the format
void getFrameRaw(void *data, void **raw_data, int *size)
{
  ImDrawData* draw_data = (ImDrawData*)data;

  // compute sizes
  int buffer_size = sizeof(int)*3,
      vtx_count = 0,
      idx_count = 0,
      cmd_count = 0;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    vtx_count += cmd_list->VtxBuffer.size();
    buffer_size += cmd_list->VtxBuffer.size() * (sizeof(vtxContainer));

    idx_count += cmd_list->IdxBuffer.size();
    buffer_size += cmd_list->IdxBuffer.size() * (sizeof(short));

    cmd_count += cmd_list->CmdBuffer.size();
    buffer_size += cmd_list->CmdBuffer.size() * (sizeof(cmdContainer));
  }

  void *local_data_base = malloc(buffer_size);

  int *data_header_ptr = (int*) local_data_base;
  *data_header_ptr = vtx_count; data_header_ptr++;
  *data_header_ptr = idx_count; data_header_ptr++;
  *data_header_ptr = cmd_count; data_header_ptr++;

  vtxContainer* data_vtx_ptr = (vtxContainer*) data_header_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const& vtx : cmd_list->VtxBuffer) {
      data_vtx_ptr->posX = vtx.pos.x;
      data_vtx_ptr->posY = vtx.pos.y;
      data_vtx_ptr->uvX = vtx.uv.x;
      data_vtx_ptr->uvY = vtx.uv.y;
      data_vtx_ptr->col = vtx.col;
      data_vtx_ptr++;
    }
  }

  int idx_offset = 0;
  short* data_idx_ptr = (short*) data_vtx_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const& idx : cmd_list->IdxBuffer) {
      *data_idx_ptr = idx + (short)idx_offset;
      data_idx_ptr++;
    }

    idx_offset += cmd_list->VtxBuffer.size();
  }

  cmdContainer* data_cmd_ptr = (cmdContainer*) data_idx_ptr;
  for (int cmd_id = 0; cmd_id < draw_data->CmdListsCount; ++cmd_id) {
    const auto cmd_list = draw_data->CmdLists[cmd_id];

    for (auto const& cmd : cmd_list->CmdBuffer) {
      data_cmd_ptr->count = cmd.ElemCount;
      data_cmd_ptr->rectX = cmd.ClipRect.x;
      data_cmd_ptr->rectY = cmd.ClipRect.y;
      data_cmd_ptr->rectZ = cmd.ClipRect.z;
      data_cmd_ptr->rectW = cmd.ClipRect.w;
      data_cmd_ptr++;
    }
  }

  *size = buffer_size;
  *raw_data = local_data_base;
}

bool pollGUIPreRender(void* context)
{
  if (context) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(context);

    if (glfwWindowShouldClose(window)) {
      return false;
    }
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    // Clearing the viewport
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImVec4 clear_color = ImColor(114, 144, 154);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    // Just initialize new frame
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = (float)(1.0f/60.0f);
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

    ImGui_ImplGlfwGL3_RenderDrawLists((ImDrawData*)draw_data);
    glfwSwapBuffers(window);
  }
}
  
/// @return true if we do not need to exit, false if we do.
bool pollGUI(void* context, std::function<void(void)> guiCallback)
{
  if (!pollGUIPreRender(context)) {
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
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();
}

} // namespace framework
} // namespace o2
