#include "WebGPU_Util.h"

using namespace wgpu;

Instance instance;
Device device;
RenderPipeline pipeline;
SwapChain swapChain;
const uint32_t w = 512;
const uint32_t h = 512;
const char shaderCode[] = R"(
    @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
      @builtin(position) vec4f {
        const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
        return vec4f(pos[i], 0, 1);
    }
    @fragment fn fragmentMain() -> @location(0) vec4f {
        return vec4f(1, 0, 0, 1);
    }
)";

void Render() {
    RenderPassColorAttachment attachment{
        .view       = swapChain.GetCurrentTextureView(),
        .loadOp     = LoadOp::Clear,
        .storeOp    = StoreOp::Store,
        .clearValue = Color{0.0, 0.0, 0.0, 1.0}
    };
    RenderPassDescriptor renderpass{
        .colorAttachmentCount = 1,
        .colorAttachments     = &attachment
    };
    CommandEncoder encoder = device.CreateCommandEncoder();
    RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

void Start() {
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(w, h, "WebGPU", nullptr, nullptr);

    Surface surface = GetSurface(instance, window);

    SwapChainDescriptor scDesc{
        .usage       = TextureUsage::RenderAttachment,
        .format      = TextureFormat::BGRA8Unorm,
        .width       = w,
        .height      = h,
        .presentMode = PresentMode::Fifo
    };
    swapChain = device.CreateSwapChain(surface, &scDesc);

    ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = shaderCode;

    ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgslDesc };
    ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    ColorTargetState colorTargetState{.format = TextureFormat::BGRA8Unorm};

    FragmentState fragmentState{
        .module = shaderModule,
        .targetCount = 1,
        .targets = &colorTargetState
    };

    RenderPipelineDescriptor descriptor{
        .vertex = {.module = shaderModule},
        .fragment = &fragmentState
    };
    pipeline = device.CreateRenderPipeline(&descriptor);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Render, 0, false);
#else
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Render();
        swapChain.Present();
        instance.ProcessEvents();
    }
#endif
}

int main() {
    instance = CreateInstance();
    GetDevice(instance, [](Device dev) {
        device = dev;
        Start();
    });
}
