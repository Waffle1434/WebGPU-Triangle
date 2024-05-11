#include "WebGPU_Util.h"

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
        out.color = vec4f(colors[i % 3], 1);

        var i_f = f32(i);
        out.position = vec4f(sin(t + 1.241f*i_f), sin(t + 2.213f*i_f), 0, 1);

        return out;
    }
    @fragment fn fragmentMain(in : V2F) -> @location(0) vec4f {
        return in.color;
    }
)";

Instance       instance;
Device         device;
Surface        surface;
SwapChain      swapchain;
RenderPipeline pipeline;
BindGroup      bindgrp;
Buffer         uniform_buffer;
RenderPassColorAttachment color_attach;
RenderPassDescriptor      pass_d;

void Render() {
    Queue queue = device.GetQueue();

    float t = static_cast<float>(glfwGetTime());
    queue.WriteBuffer(uniform_buffer, 0, &t, sizeof(float));

    CommandEncoder encoder = device.CreateCommandEncoder();
    color_attach.view = swapchain.GetCurrentTextureView();
    RenderPassEncoder pass = encoder.BeginRenderPass(&pass_d);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindgrp, 0, nullptr);
    pass.Draw(1*3);
    pass.End();
    CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}


void InitGraphics(GLFWwindow* window) {
    surface = GetSurface(instance, window);

    SwapChainDescriptor swp_d {
        .usage       = TextureUsage::RenderAttachment,
        .format      = TextureFormat::BGRA8Unorm,
        .width       = w,
        .height      = h,
        .presentMode = PresentMode::Fifo
    };
    swapchain = device.CreateSwapChain(surface, &swp_d);

    ShaderModuleWGSLDescriptor wgsl_d{}; wgsl_d.code = shaderCode;
    ShaderModuleDescriptor shader_mod_d { .nextInChain = &wgsl_d };
    ShaderModule shader_mod = device.CreateShaderModule(&shader_mod_d);

    ColorTargetState color_t_state { .format = TextureFormat::BGRA8Unorm };
    FragmentState frag_state { .module = shader_mod, .targetCount = 1, .targets = &color_t_state };

    BindGroupLayoutEntry bind_layout{}; // = Default
    bind_layout.binding = 0;
    bind_layout.visibility = ShaderStage::Vertex;
    bind_layout.buffer.type = BufferBindingType::Uniform;
    bind_layout.buffer.minBindingSize = sizeof(float);
    BindGroupLayoutDescriptor bindgrp_layout_d { .entryCount = 1, .entries = &bind_layout };
    BindGroupLayout bindgrp_layout = device.CreateBindGroupLayout(&bindgrp_layout_d);
    PipelineLayoutDescriptor layout_d { .bindGroupLayoutCount = 1, .bindGroupLayouts = &bindgrp_layout };
    BufferDescriptor buffer_d {
        .usage = BufferUsage::CopyDst | BufferUsage::Uniform,
        .size  = sizeof(float)
    };
    uniform_buffer = device.CreateBuffer(&buffer_d);
    BindGroupEntry binding {
        .binding = 0,
        .buffer  = uniform_buffer,
        .offset  = 0,
        .size    = sizeof(float)
    };
    BindGroupDescriptor bindgrp_d {
        .layout     = bindgrp_layout,
        .entryCount = bindgrp_layout_d.entryCount,
        .entries    = &binding
    };
    bindgrp = device.CreateBindGroup(&bindgrp_d);

    RenderPipelineDescriptor pipe_d {
        .layout = device.CreatePipelineLayout(&layout_d),
        .vertex = { .module = shader_mod },
        .fragment = &frag_state
    };
    pipeline = device.CreateRenderPipeline(&pipe_d);

    color_attach = {
        .loadOp     = LoadOp::Clear,
        .storeOp    = StoreOp::Store,
        .clearValue = Color{0.0, 0.0, 0.0, 1.0}
    };
    pass_d = {
        .colorAttachmentCount = 1,
        .colorAttachments     = &color_attach
    };
}

void Start() {
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(w, h, "WebGPU", nullptr, nullptr);

    InitGraphics(window);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Render, 0, false);
#else
    while (!glfwWindowShouldClose(window) && !error) {
        glfwPollEvents();
        Render();
        swapchain.Present();
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
