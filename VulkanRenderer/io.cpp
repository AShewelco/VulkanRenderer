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
    auto Window::framebufferResizeCallback(glfw::GLFWwindow* window, int width, int height) -> void {
        self(window).rebuildForResize();
    }

    auto Window::cursorPosCallback(glfw::GLFWwindow* window, double x, double y) -> void {
        static glm::dvec2 last(x, y);
        glm::dvec2 current(x, y);
        if (last == current) {
            return;
        }
        if (!self(window).cursorPosSuppress) {
            self(window).fire(event::MouseOffset { current - last });
        }
        else {
            self(window).cursorPosSuppress = false;
        }
        last = current;
    }

    auto Window::mouseButtonCallback(glfw::GLFWwindow* window, int button, int action, int mods) -> void {
        switch (action) {
            case GLFW_PRESS:
                self(window).fire(event::MousePress { static_cast<io::Button>(button) });
                break;
            case GLFW_RELEASE:
                self(window).fire(event::MouseRelease { static_cast<io::Button>(button) });
                break;
        }
    }

    auto Window::keyCallback(glfw::GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
        switch (action) {
            case GLFW_PRESS:
                self(window).fire(event::KeyPress { static_cast<io::Key>(key) });
                break;
            case GLFW_RELEASE:
                self(window).fire(event::KeyRelease { static_cast<io::Key>(key) });
                break;
        }
    }

    auto Window::windowRefreshCallback(glfw::GLFWwindow* window) -> void {
        self(window).onRefresh();
    }

    auto Window::windowSizeCallback(glfw::GLFWwindow* window, int width, int height) -> void {
        self(window).cursorPosSuppress = true;
        self(window).fire(event::WindowResize { glm::ivec2(width, height) });
    }

    Window::Window(WindowCreateInfo info) {
        glfw::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfw::glfwWindowHint(GLFW_VISIBLE, info.visible);
        window = glfw::glfwCreateWindow(info.size.x, info.size.y, info.title.c_str(), nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("GLFW: Couldn't create window");
        }
        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetCursorPosCallback(window, cursorPosCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetWindowRefreshCallback(window, windowRefreshCallback);
        glfwSetWindowSizeCallback(window, windowSizeCallback);
    }

    auto Window::getKeyboardScalar(io::Key positive, io::Key negative) -> float {
        float result = 0;
        if (getKeyPressed(positive)) {
            result += 1;
        }
        if (getKeyPressed(negative)) {
            result -= 1;
        }
        return result;
    }

    auto Window::getKeyboardVector(io::Key positiveX, io::Key negativeX, io::Key positiveY, io::Key negativeY) -> glm::vec2 {
        return glm::vec2(getKeyboardScalar(positiveX, negativeX), getKeyboardScalar(positiveY, negativeY));
    }

    Window::~Window() {
        glfwDestroyWindow(window);
    }

    auto Window::getClosed() const -> bool {
        return glfwWindowShouldClose(window);
    }

    auto Window::setClosed(bool closed) -> void {
        glfwSetWindowShouldClose(window, closed);
    }

    auto Window::getWindowPointer() -> glfw::GLFWwindow* {
        return window;
    }

    auto Window::setRefreshCallback(std::function<void()> callback) -> void {
        onRefresh = callback;
    }

    auto Window::setFramebufferResizeCallback(std::function<void()> callback) -> void {
        rebuildForResize = callback;
    }

    auto Window::getKeyPressed(Key key) -> bool {
        return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
    }

    auto Window::setCursorPos(glm::dvec2 position) -> void {
        glfwSetCursorPos(window, position.x, position.y);
    }

    auto Window::getCursorPos() -> glm::dvec2 {
        glm::dvec2 result;
        glfwGetCursorPos(window, &result.x, &result.y);
        return result;
    }

    auto Window::setCursorInputMode(CursorInputMode cursorMode) -> void {
        glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(cursorMode));
    }

    auto Window::getCursorInputMode() -> CursorInputMode {
        return static_cast<CursorInputMode>(glfwGetInputMode(window, GLFW_CURSOR));
    }

    auto Window::getMouseButtonPressed(io::Button button) -> bool {
        return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
    }

    auto Window::setFullscreen(bool fullscreen) -> void {
        if (fullscreen == getFullscreen()) {
            return;
        }

        cursorPosSuppress = true;

        glfw::GLFWmonitor* monitor = glfw::glfwGetPrimaryMonitor();
        const glfw::GLFWvidmode* mode = glfw::glfwGetVideoMode(monitor);

        static glm::ivec2 position = getPos();
        static glm::ivec2 dimentions = getSize();

        if (fullscreen) {
            position = getPos();
            dimentions = getSize();
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            glfwSetWindowMonitor(window, nullptr, position.x, position.y, dimentions.x, dimentions.y, mode->refreshRate);
        }
    }

    auto Window::getFullscreen() -> bool {
        return glfwGetWindowMonitor(window);
    }

    auto Window::getSize() -> glm::ivec2 {
        glm::ivec2 result;
        glfwGetWindowSize(window, &result.x, &result.y);
        return result;
    }

    auto Window::setSize(glm::ivec2 dimentions) -> void {
        glfwSetWindowSize(window, dimentions.x, dimentions.y);
    }

    auto Window::getPos() -> glm::ivec2 {
        glm::ivec2 result;
        glfwGetWindowPos(window, &result.x, &result.y);
        return result;
    }

    auto Window::setPos(glm::ivec2 position) -> void {
        if (getPos() == position) {
            return;
        }
        cursorPosSuppress = true;
        glfwSetWindowPos(window, position.x, position.y);
    }

    auto Window::setVisible(bool visible) -> void {
        if (visible) {
            glfwShowWindow(window);
        }
        else {
            glfwHideWindow(window);
        }
    }

    auto Window::setTitle(const std::string& title) -> void {
        glfwSetWindowTitle(window, title.c_str());
    }

    auto Window::poll() -> void {
        events.clear();
        glfw::glfwPollEvents();
    }

    auto Window::self(glfw::GLFWwindow* window) -> Window& {
        return *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    }
}