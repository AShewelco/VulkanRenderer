#pragma once
#include "part.h"

namespace vkr::api {
    class Renderer : private part::LastPart {
    public:
        Renderer(api::RendererCreateInfo&& rendererCreateInfo);
        auto getCamera() -> data::Camera&;
        auto getWindow() -> io::Window&;
        auto runLoop() -> void;
    };
}

namespace vkr::test {
    class Application : public api::Renderer {
    public:
        Application();
    private:
        auto rendererCreateInfo() -> api::RendererCreateInfo;
        static auto selectDevice(std::vector<vk::PhysicalDeviceProperties> deviceProperties) -> size_t;
        auto onUpdate(float delta, float time) -> void;
    };
}