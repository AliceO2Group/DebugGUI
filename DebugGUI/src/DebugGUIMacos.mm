#include "icons_font_awesome.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include "implot.h"
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

static void default_error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

namespace o2::framework {

struct DebugGUIContext {
  GLFWwindow *window;
  id<MTLDevice> device;
  id<MTLCommandQueue> commandQueue;
  MTLRenderPassDescriptor *renderPassDescriptor;
};

CAMetalLayer *layer;
GLFWwindow *window;

// @return an object of kind GLFWwindow* as void* to avoid having a direct dependency
void *initGUI(const char *name, void (*error_callback)(int, char const *description)) {
  DebugGUIContext *context = nullptr;

  if (name) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
    // Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable
    // Gamepad Controls

    // Setup style
    ImGui::StyleColorsDark();

    // Setup window
    if (error_callback == nullptr) {
      glfwSetErrorCallback(default_error_callback);
    }
    if (!glfwInit()) {
      return nullptr;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    context = (DebugGUIContext *)(malloc(sizeof(DebugGUIContext)));
    window = glfwCreateWindow(1280, 720, name, nullptr, nullptr);

    if (window == nullptr) {
      return nullptr;
    }

    context->window = window;
    context->device = MTLCreateSystemDefaultDevice();
    context->commandQueue = [context->device newCommandQueue];

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(context->window, true);
    // Load Fonts
    // (there is a default font, this is only if you want to change it. see
    // extra_fonts/README.txt for more details)
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;
    icons_config.GlyphOffset = {0, 2};
    icons_config.GlyphMinAdvanceX = 17;
    io.Fonts->AddFontFromMemoryTTF((void *)s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf),
                                   13.0f, &icons_config, icons_ranges);

    if (io.Fonts->ConfigData.empty()) {
      io.Fonts->AddFontDefault();
    }
    //  io.Fonts->Build();
    io.DisplaySize = ImVec2(1280, 720);
    ImGui_ImplMetal_Init(context->device);

    NSWindow *nswin = glfwGetCocoaWindow(context->window);
    layer = [CAMetalLayer layer];
    layer.device = context->device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    context->renderPassDescriptor = [MTLRenderPassDescriptor new];
  } else {
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
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
    // Load Fonts
    // (there is a default font, this is only if you want to change it. see
    // extra_fonts/README.txt for more details)
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset = {0, 2};
    icons_config.GlyphMinAdvanceX = 17;
    icons_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void *)s_iconsFontAwesomeTtf, sizeof(s_iconsFontAwesomeTtf),
                                   13.0f, &icons_config, icons_ranges);

    if (io.Fonts->ConfigData.empty()) {
      io.Fonts->AddFontDefault();
    }
    //  io.Fonts->Build();
    io.DisplaySize = ImVec2(1280, 720);
    ImGui_ImplMetal_Init(device);
  }

  ImPlot::CreateContext();
  return context;
}

float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

bool pollGUIPreRender(void *context, float delta) {
  if (context == nullptr) {
    // Just initialize new frame
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = delta;
    ImGui::NewFrame();
    return true;
  }

  auto *ctx = reinterpret_cast<DebugGUIContext *>(context);
  NSWindow *nswin = glfwGetCocoaWindow(ctx->window);
  @autoreleasepool {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(ctx->window, &width, &height);

    layer.drawableSize = CGSizeMake(width, height);
    auto drawable = [layer nextDrawable];

    ctx->renderPassDescriptor.colorAttachments[0].clearColor =
        MTLClearColorMake(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
    ctx->renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
    ctx->renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    ctx->renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(ctx->renderPassDescriptor);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }
  return glfwWindowShouldClose(ctx->window) == false;
}

void pollGUIPostRender(void *context, void *drawData) {
  if (!context) {
    return;
  }
  auto *ctx = reinterpret_cast<DebugGUIContext *>(context);
  @autoreleasepool {
    auto drawable = [layer nextDrawable];

    id<MTLCommandBuffer> commandBuffer = [ctx->commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> renderEncoder =
        [commandBuffer renderCommandEncoderWithDescriptor:ctx->renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"ImGui demo"];

    ImGui_ImplMetal_RenderDrawData((ImDrawData *)drawData, commandBuffer, renderEncoder);

    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
  }
}

/// @return draw data as void* to avoid dependencies
void *pollGUIRender(std::function<void(void)> guiCallback) {
  // This is where the magic actually happens...
  if (guiCallback) {
    guiCallback();
  }
  ImGui::Render();

  return ImGui::GetDrawData();
}

/// @return true if we do not need to exit, false if we do.
bool pollGUI(void *context, std::function<void(void)> guiCallback) {
  if (!pollGUIPreRender(context, 1.0f / 60.0f)) {
    return false;
  }
  // This is where the magic actually happens...
  void *drawData = pollGUIRender(guiCallback);
  pollGUIPostRender(context, drawData);
  return true;
}

void disposeGUI() {
  ImGui_ImplMetal_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

}  // namespace o2::framework
