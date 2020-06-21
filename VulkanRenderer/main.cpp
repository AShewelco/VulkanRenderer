#include "main.h"

namespace vkr::api {
    Renderer::Renderer(api::RendererCreateInfo&& rendererCreateInfo) : part::LastPart(std::move(rendererCreateInfo)) {}

    auto Renderer::pushModel(const data::Model& model) -> void {
        getDevice().waitIdle();
        LastPart::pushModel(model);
    }

    auto Renderer::getVertexSpan() -> std::span<data::Vertex> {
        return LastPart::getVertexSpan();
    }
    
    auto Renderer::getCamera() -> data::Camera& {
        return LastPart::getCamera();
    }
    
    auto Renderer::getWindow() -> io::Window& {
        return getWindowHandle().getWindow();
    }
    
    auto Renderer::runLoop() -> void {
        LastPart::runLoop();
    }
}

namespace vkr::test {
    Application::Application() : api::Renderer(std::move(rendererCreateInfo())) {}

    auto Application::rendererCreateInfo() -> api::RendererCreateInfo {
        api::RendererCreateInfo info;
        info.debuggerMinimumLevel = api::DebuggerMinimunLevel::eWarning;
        info.deviceSelector = selectDevice;
        info.maxAntialiasing = vk::SampleCountFlagBits::e64;
        info.texture = data::Texture("textures/room.png");
        info.onUpdate = [&](float delta, float time) {
            onUpdate(delta, time);
        };
        info.windowCreateInfo.size = glm::ivec2(1980, 1080);
        return info;
    }

    auto Application::selectDevice(std::vector<vk::PhysicalDeviceProperties> deviceProperties) -> size_t {
        fmt::print("Please select a physical device:\n");
        for (size_t i = 0; i < deviceProperties.size(); ++i) {
            fmt::print("  {}. {}\n", i + 1, deviceProperties[i].deviceName);
        }
        while (true) {
            try {
                size_t index = std::stoull(io::console::read()) - 1;
                if (index >= deviceProperties.size()) {
                    throw std::out_of_range("");
                }
                return index;
            }
            catch (std::out_of_range&) {
                fmt::print("Out of range\n");
            }
            catch (std::invalid_argument&) {
                fmt::print("Not a number\n");
            }
        }
    }

    auto Application::onUpdate(float delta, float time) -> void {
        static float sum = 0.0f;
        sum += delta;
        if (sum >= 0.25f) {
            getWindow().setTitle(fmt::format("FPS: {}", static_cast<int>(1.0f / delta)));
            sum -= 0.25f;
        }
        
        getWindow().on<io::event::MousePress>([&](auto e, bool& handled) {
            if (getWindow().getMouse().getInputMode() != io::CursorInputMode::eInfinite) {
                getWindow().getMouse().setInputMode(io::CursorInputMode::eInfinite);
            }
            else {
                data::Model roomRelative = room;
                for (auto& vertex : roomRelative.vertices) {
                    vertex.position += (getCamera().position + getCamera().getDirection() * 2.0f) * glm::vec3(-1.0f, 1.0f, -1.0f);
                }
                pushModel(roomRelative);
                View view;
                view.start = getVertexSpan().size() - roomRelative.vertices.size();
                view.count = roomRelative.vertices.size();
                models.push_back(view);
            }
        });

        getWindow().on<io::event::KeyPress>([&](auto e, bool& handled) {
            if (e.key == io::Key::eF11) {
                getWindow().setFullscreen(!getWindow().getFullscreen());
            }
            else if (e.key == io::Key::eEscape) {
                if (getWindow().getFullscreen()) {
                    getWindow().setFullscreen(false);
                }
                else if (getWindow().getMouse().getInputMode() == io::CursorInputMode::eInfinite) {
                    getWindow().getMouse().setInputMode(io::CursorInputMode::eNormal);
                }
                else {
                    getWindow().setClosed(true);
                }
            }
        });
    
        getWindow().on<io::event::MouseOffset>([&](auto e, bool& handled) {
            if (getWindow().getMouse().getInputMode() == io::CursorInputMode::eInfinite) {
                getCamera().yaw += e.offset.x / 150.0f;
                getCamera().pitch += e.offset.y / 150.0f;
                getCamera().pitch = std::clamp(getCamera().pitch, -glm::pi<float>(), 0.0f);
            }
        });
    
        getCamera().position += getCamera().getForward() * getWindow().getKeyboard().getKeyScalar(io::Key::eW, io::Key::eS) * delta * 2.0f;
        getCamera().position += getCamera().getLeft() * getWindow().getKeyboard().getKeyScalar(io::Key::eA, io::Key::eD) * delta * 2.0f;
        getCamera().position.z += getWindow().getKeyboard().getKeyScalar(io::Key::eLeftShift, io::Key::eSpace) * delta * 2.0f;

        try {
            size_t index = 0;
            for (View model : models) {
                glm::vec3 center(0.0f);
                for (data::Vertex& vertex : getVertexSpan().subspan(model.start, model.count)) {
                    center += vertex.position;
                }
                center /= static_cast<float>(model.count);
                
                for (data::Vertex& vertex : getVertexSpan().subspan(model.start, model.count)) {
                    vertex.position -= center;
                    vertex.position = glm::rotateZ(vertex.position, delta);
                    vertex.position += center;
                }
            }
            index++;
        }
        catch (...) {}
    }
}

int main() {
    try {
        vkr::test::Application().runLoop();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
        std::cin.get();
    }
}