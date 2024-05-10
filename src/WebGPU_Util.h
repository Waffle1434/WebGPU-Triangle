#include <iostream>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#else
#include <webgpu/webgpu_glfw.h>
#endif

using namespace wgpu;

void GetDevice(Instance instance, void (*callback)(Device)) {
    instance.RequestAdapter(
        nullptr,
        // TODO(https://bugs.chromium.org/p/dawn/issues/detail?id=1892): Use
        // RequestAdapterStatus, Adapter, and Device.
        [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter,
           const char* message, void* userdata) {
            if (status != WGPURequestAdapterStatus_Success) exit(0);

            Adapter adapter = Adapter::Acquire(cAdapter);
            adapter.RequestDevice(
                nullptr,
                [](WGPURequestDeviceStatus status, WGPUDevice cDevice,
                   const char* message, void* userdata) {
                    Device device = Device::Acquire(cDevice);
                    device.SetUncapturedErrorCallback(
                        [](WGPUErrorType type, const char* message, void* userdata) {
                            std::cout << "Error: " << type << " - message: " << message;
                        },
                        nullptr);
                    reinterpret_cast<void (*)(Device)>(userdata)(device);
                },
                userdata);
        },
        reinterpret_cast<void*>(callback)
    );
}

Surface GetSurface(Instance instance, GLFWwindow* window) {
#if defined(__EMSCRIPTEN__)
    SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    SurfaceDescriptor surfaceDesc{.nextInChain = &canvasDesc};
    return instance.CreateSurface(&surfaceDesc);
#else
    return glfw::CreateSurfaceForWindow(instance, window);
#endif
}
