#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include "icons_font_awesome.h"
// Needed by icons_font_awesome.ttf.h
#include <cstdint>
#include "icons_font_awesome.ttf.h"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

#include <cstdio>
#include <functional>

static void default_error_callback(int error, const char* description)
{
  fprintf(stderr, "Error %d: %s\n", error, description);
}

namespace o2::framework
{

struct DebugGUIContext {
  GLFWwindow *window;
  id <MTLDevice> device;
  id <MTLCommandQueue> commandQueue;
  MTLRenderPassDescriptor* renderPassDescriptor;
};

CAMetalLayer *layer;
GLFWwindow *window;

// @return an object of kind GLFWwindow* as void* to avoid having a direct dependency
void* initGUI(const char* name, void(*error_callback)(int, char const*description))
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

  // Setup style
  ImGui::StyleColorsDark();

  // Setup window
  if (error_callback == nullptr) {
    glfwSetErrorCallback(default_error_callback);
  }
  if (!glfwInit())
    return nullptr;
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  DebugGUIContext *context = (DebugGUIContext*)(malloc(sizeof(DebugGUIContext)));
  window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);

  if (window == NULL)
      return 0;

  context->window = window;
  context->device = MTLCreateSystemDefaultDevice();;
  context->commandQueue = [context->device newCommandQueue];

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(context->window, true);
  ImGui_ImplMetal_Init(context->device);

  NSWindow *nswin = glfwGetCocoaWindow(context->window);
  layer = [CAMetalLayer layer];
  layer.device = context->device;
  layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  nswin.contentView.layer = layer;
  nswin.contentView.wantsLayer = YES;

  context->renderPassDescriptor = [MTLRenderPassDescriptor new];

  // Load Fonts
  // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
  io.Fonts->AddFontDefault();
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
  ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true; icons_config.FontDataOwnedByAtlas = false;
  io.Fonts->AddFontFromMemoryTTF((void*)s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf), 12.0f, &icons_config, icons_ranges);
  ImGui_ImplMetal_CreateDeviceObjects(layer.device);

  ImPlot::CreateContext();
  return context;
}

float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

/// @return true if we do not need to exit, false if we do.
bool pollGUI(void* context, std::function<void(void)> guiCallback)
{
  DebugGUIContext* ctx = reinterpret_cast<DebugGUIContext*>(context);
  NSWindow* nswin = glfwGetCocoaWindow(ctx->window);
  if (glfwWindowShouldClose(ctx->window)) {
    return false;
  }
  @autoreleasepool {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(ctx->window, &width, &height);

    layer.drawableSize = CGSizeMake(width, height);
    auto drawable = [layer nextDrawable];

    id<MTLCommandBuffer> commandBuffer = [ctx->commandQueue commandBuffer];
    ctx->renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
    ctx->renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    ctx->renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    ctx->renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:ctx->renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"ImGui demo"];

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(ctx->renderPassDescriptor);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    
    // This is where the magic actually happens...
    if (guiCallback) {
      guiCallback();
    }
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
  }
  return true;
}

void disposeGUI()
{
  ImGui_ImplMetal_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

} // namespace o2::framework
