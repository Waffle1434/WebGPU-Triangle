#include "WebGPU_Util.h"

Instance       instance;
Device         device;
SwapChain      swapChain;
RenderPipeline pipeline;
BindGroup      bindGroup;
Buffer         uniformBuffer;
const uint32_t w = 512;
const uint32_t h = 512;
const char shaderCode[] = R"(
    @group(0) @binding(0) var<uniform> t: f32;

    struct V2F {
        @builtin(position) position : vec4f,
        @location(0) color : vec4f,
    };

    @vertex fn vertexMain(@builtin(vertex_index) i : u32) -> V2F {
        const verts = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
        const colors = array(vec3f(1,0,0), vec3f(0,1,0), vec3f(0,0,1));

        var out : V2F;
        out.position = vec4f(verts[i], 0, 1);
        out.color = vec4f(colors[i], 1);

        var i_f = f32(i);
        out.position = vec4f(sin(t + 1.241f*i_f), sin(t + 2.213f*i_f), 0, 1);

        return out;
    }
    @fragment fn fragmentMain(in : V2F) -> @location(0) vec4f {
        return in.color;
    }
)";

void Render() {
    Queue queue = device.GetQueue();

    float t = static_cast<float>(glfwGetTime());
    queue.WriteBuffer(uniformBuffer, 0, &t, sizeof(float));

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
    pass.SetBindGroup(0, bindGroup, 0, nullptr);
    pass.Draw(3);
    pass.End();
    CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
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

    BindGroupLayoutEntry bindingLayout; // = Default
    bindingLayout.binding = 0;
    bindingLayout.visibility = ShaderStage::Vertex;
    bindingLayout.buffer.type = BufferBindingType::Uniform;
    bindingLayout.buffer.minBindingSize = sizeof(float);

    BindGroupLayoutDescriptor bindGroupLayoutDesc {
        .entryCount = 1,
        .entries = &bindingLayout
    };
    BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&bindGroupLayoutDesc);

    PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (BindGroupLayout*)&bindGroupLayout;
    PipelineLayout layout = device.CreatePipelineLayout(&layoutDesc);

    BufferDescriptor bufferDesc {
        .usage = BufferUsage::CopyDst | BufferUsage::Uniform,
        .size  = sizeof(float),
        .mappedAtCreation = false
    };
    uniformBuffer = device.CreateBuffer(&bufferDesc);

    BindGroupEntry binding{
        .binding = 0,
        .buffer  = uniformBuffer,
        .offset  = 0,
        .size    = sizeof(float)
    };

    BindGroupDescriptor bindGroupDesc{
        .layout     = bindGroupLayout,
        .entryCount = bindGroupLayoutDesc.entryCount,
        .entries    = &binding
    };
    bindGroup = device.CreateBindGroup(&bindGroupDesc);


    RenderPipelineDescriptor pipeDesc{
        .layout = layout,
        .vertex = {.module = shaderModule},
        .fragment = &fragmentState
    };
    pipeline = device.CreateRenderPipeline(&pipeDesc);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Render, 0, false);
#else
    while (!glfwWindowShouldClose(window) && !error) {
        glfwPollEvents();
        Render();
        swapChain.Present();
        instance.ProcessEvents();
    }
#endif
}

int main() {
    std::cout << "std::cout test" << std::endl;
    instance = CreateInstance();
    GetDevice(instance, [](Device dev) {
        device = dev;
        Start();
    });
}
