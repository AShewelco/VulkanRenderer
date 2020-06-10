#include "io.h"

namespace vkr::io::console {
    auto read() -> std::string {
        std::string result;
        std::getline(std::cin, result);
        return result;
    }
}

namespace vkr::io::file {
    template <>
    auto read<uint8_t>(const char* file) -> std::vector<uint8_t> {
        std::ifstream stream(file, std::ios::ate | std::ios::binary);

        if (!file) {
            throw std::runtime_error(fmt::format("Failed to open {}", file));
        }

        size_t size = static_cast<size_t>(stream.tellg());
        std::vector<uint8_t> buffer(size);

        stream.seekg(0);
        stream.read(reinterpret_cast<char*>(buffer.data()), size);

        stream.close();

        return buffer;
    }

    template<>
    auto read<uint32_t>(const char* file) -> std::vector<uint32_t> {
        std::vector<uint8_t> data = read<uint8_t>(file);
        std::vector<uint32_t> aligned;
        aligned.resize((data.size() + sizeof(uint32_t) - 1) / sizeof(uint32_t));
        memcpy(aligned.data(), data.data(), data.size());
        return aligned;
    }
}

namespace vkr::io {
    auto Keyboard::getKeyPressed(Key key) -> bool {
        return glfw::glfwGetKey(windowGLFW, static_cast<int>(key)) == GLFW_PRESS;
    }

    auto Keyboard::getKeyScalar(Key positive, Key negative) -> float {
        float result = 0;
        if (getKeyPressed(positive)) {
            result += 1;
        }
        if (getKeyPressed(negative)) {
            result -= 1;
        }
        return result;
    }

    auto Keyboard::getKeyVector(Key positiveX, Key negativeX, Key positiveY, Key negativeY) -> glm::vec2 {
        return glm::vec2(getKeyScalar(positiveX, negativeX), getKeyScalar(positiveY, negativeY));
    }

    auto Mouse::getPosition() -> glm::dvec2 {
        glm::dvec2 result;
        glfw::glfwGetCursorPos(windowGLFW, &result.x, &result.y);
        return result;
    }

    auto Mouse::setPosition(glm::dvec2 position) -> void {
        glfw::glfwSetCursorPos(windowGLFW, position.x, position.y);
    }

    auto Mouse::getInputMode() -> CursorInputMode {
        return static_cast<CursorInputMode>(glfw::glfwGetInputMode(windowGLFW, GLFW_CURSOR));
    }

    auto Mouse::setInputMode(CursorInputMode mode) -> void {
        glfw::glfwSetInputMode(windowGLFW, GLFW_CURSOR, static_cast<int>(mode));
    }

    auto Mouse::getButtonPressed(Button button) -> bool {
        return glfw::glfwGetMouseButton(windowGLFW, static_cast<int>(button)) == GLFW_PRESS;
    }

    auto Window::getKeyboard() -> Keyboard {
        Keyboard keyboard;
        keyboard.windowGLFW = windowGLFW;
        return keyboard;
    }

    auto Window::getMouse() -> Mouse {
        Mouse mouse;
        mouse.windowGLFW = windowGLFW;
        return mouse;
    }

    auto Window::getClosed() -> bool {
        return glfw::glfwWindowShouldClose(windowGLFW);
    }

    auto Window::setClosed(bool closed) -> void {
        glfwSetWindowShouldClose(windowGLFW, closed);
    }

    auto Window::getDimentions() -> glm::ivec2 {
        glm::ivec2 result;
        glfw::glfwGetWindowSize(windowGLFW, &result.x, &result.y);
        return result;
    }

    auto Window::setDimentions(glm::ivec2 dimentions) -> void {
        if (dimentions == getDimentions()) {
            return;
        }
        glfw::glfwSetWindowSize(windowGLFW, dimentions.x, dimentions.y);
    }

    auto Window::getPosition() -> glm::ivec2 {
        glm::ivec2 result;
        glfw::glfwGetWindowPos(windowGLFW, &result.x, &result.y);
        return result;
    }

    auto Window::setPosition(glm::ivec2 position) -> void {
        if (position == getPosition()) {
            return;
        }
        glfw::glfwSetWindowPos(windowGLFW, position.x, position.y);
    }

    auto Window::getFullscreen() -> bool {
        return glfw::glfwGetWindowMonitor(windowGLFW);
    }

    auto Window::setFullscreen(bool fullscreen) -> void {
        if (fullscreen == getFullscreen()) {
            return;
        }

        glfw::GLFWmonitor* monitor = glfw::glfwGetPrimaryMonitor();
        const glfw::GLFWvidmode* mode = glfw::glfwGetVideoMode(monitor);

        static glm::ivec2 position = getPosition();
        static glm::ivec2 dimentions = getDimentions();

        if (fullscreen) {
            position = getPosition();
            dimentions = getDimentions();
            glfw::glfwSetWindowMonitor(windowGLFW, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            glfw::glfwSetWindowMonitor(windowGLFW, nullptr, position.x, position.y, dimentions.x, dimentions.y, mode->refreshRate);
        }
    }

    auto Window::setTitle(std::string title) -> void {
        glfw::glfwSetWindowTitle(windowGLFW, title.c_str());
    }

    auto Window::hide() -> void {
        glfw::glfwHideWindow(windowGLFW);
    }

    auto Window::show() -> void {
        glfw::glfwShowWindow(windowGLFW);
    }

    auto WindowHandle::getWindowHandle(glfw::GLFWwindow* window) -> WindowHandle& {
        void* user = glfw::glfwGetWindowUserPointer(window);
        return *reinterpret_cast<WindowHandle*>(user);
    }
    
    WindowHandle::WindowHandle(WindowCreateInfo info) {
        glfw::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfw::glfwWindowHint(GLFW_VISIBLE, false);

        window.windowGLFW = glfw::glfwCreateWindow(info.size.x, info.size.y, info.title.c_str(), nullptr, nullptr);
        if (!window.windowGLFW) {
            throw std::runtime_error("GLFW: Couldn't create window");
        }
        glfwSetWindowUserPointer(window.windowGLFW, this);

        glfw::glfwSetFramebufferSizeCallback(window.windowGLFW, [](glfw::GLFWwindow* window, int, int) {
            getWindowHandle(window).onFramebufferResize();
        });

        glfw::glfwSetWindowRefreshCallback(window.windowGLFW, [](glfw::GLFWwindow* window) {
            getWindowHandle(window).onRefresh();
        });

        glfw::glfwSetCursorPosCallback(window.windowGLFW, [](glfw::GLFWwindow* window, double x, double y) {
            static glm::dvec2 last(x, y);
            glm::dvec2 current(x, y);
           
            getWindowHandle(window).fireEvent(event::MouseOffset { current - last });
            
            last = current;
        });

        glfw::glfwSetMouseButtonCallback(window.windowGLFW, [](glfw::GLFWwindow* window, int button, int action, int mods) {
            switch (action) {
                case GLFW_PRESS:
                    getWindowHandle(window).fireEvent(event::MousePress { static_cast<io::Button>(button) });
                    break;
                case GLFW_RELEASE:
                    getWindowHandle(window).fireEvent(event::MouseRelease { static_cast<io::Button>(button) });
                    break;
            }
        });

        glfw::glfwSetKeyCallback(window.windowGLFW, [](glfw::GLFWwindow * window, int key, int scancode, int action, int mods) {
            switch (action) {
                case GLFW_PRESS:
                    getWindowHandle(window).fireEvent(event::KeyPress { static_cast<io::Key>(key) });
                    break;
                case GLFW_RELEASE:
                    getWindowHandle(window).fireEvent(event::KeyRelease { static_cast<io::Key>(key) });
                    break;
            }
        });

        glfw::glfwSetWindowSizeCallback(window.windowGLFW, [](glfw::GLFWwindow* window, int width, int height) -> void {
            getWindowHandle(window).fireEvent(event::WindowResize { glm::ivec2(width, height) });
        });
    }

    WindowHandle::~WindowHandle() {
        glfw::glfwDestroyWindow(window.windowGLFW);
    }

    auto WindowHandle::getWindow() -> Window& {
        return window;
    }

    auto WindowHandle::getWindowGLFW() -> glfw::GLFWwindow* {
        return window.windowGLFW;
    }

    auto WindowHandle::poll() -> void {
        window.eventArena.clear();
        glfw::glfwPollEvents();
    }
}