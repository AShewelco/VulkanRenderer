#pragma once
#include "part.h"

namespace vkr::api {
    class Renderer : private part::LastPart {
    public:
        Renderer(api::RendererCreateInfo&& rendererCreateInfo);
        auto pushModel(const data::Model& model) -> void;
        auto getVertexSpan() -> std::span<data::Vertex>;
        auto getCamera() -> data::Camera&;
        auto getWindow() -> io::Window&;
        auto runLoop() -> void;
    };
}

namespace vkr::test {
    struct View {
        size_t start;
        size_t count;
    };

    class Application : public api::Renderer {
    public:
        Application();
    private:
        static auto selectDevice(std::vector<vk::PhysicalDeviceProperties> deviceProperties) -> size_t;
    private:
        data::Model room { "models/room.obj" };
        data::Model orange { "models/orange.obj" };
        std::vector<View> models;
    private:
        auto rendererCreateInfo() -> api::RendererCreateInfo;
        auto onUpdate(float delta, float time) -> void;
    };
}