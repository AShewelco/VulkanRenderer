#pragma once

namespace vkr::io {
    enum class Key {
        eUnknown = GLFW_KEY_UNKNOWN,
        eSpace = GLFW_KEY_SPACE,
        eApostrophe = GLFW_KEY_APOSTROPHE,
        eComma = GLFW_KEY_COMMA,
        eMinus = GLFW_KEY_MINUS,
        ePeriod = GLFW_KEY_PERIOD,
        eSlash = GLFW_KEY_SLASH,
        e0 = GLFW_KEY_0,
        e1 = GLFW_KEY_1,
        e2 = GLFW_KEY_2,
        e3 = GLFW_KEY_3,
        e4 = GLFW_KEY_4,
        e5 = GLFW_KEY_5,
        e6 = GLFW_KEY_6,
        e7 = GLFW_KEY_7,
        e8 = GLFW_KEY_8,
        e9 = GLFW_KEY_9,
        eSemicolon = GLFW_KEY_SEMICOLON,
        eEqual = GLFW_KEY_EQUAL,
        eA = GLFW_KEY_A,
        eB = GLFW_KEY_B,
        eC = GLFW_KEY_C,
        eD = GLFW_KEY_D,
        eE = GLFW_KEY_E,
        eF = GLFW_KEY_F,
        eG = GLFW_KEY_G,
        eH = GLFW_KEY_H,
        eI = GLFW_KEY_I,
        eJ = GLFW_KEY_J,
        eK = GLFW_KEY_K,
        eL = GLFW_KEY_L,
        eM = GLFW_KEY_M,
        eN = GLFW_KEY_N,
        eO = GLFW_KEY_O,
        eP = GLFW_KEY_P,
        eQ = GLFW_KEY_Q,
        eR = GLFW_KEY_R,
        eS = GLFW_KEY_S,
        eT = GLFW_KEY_T,
        eU = GLFW_KEY_U,
        eV = GLFW_KEY_V,
        eW = GLFW_KEY_W,
        eX = GLFW_KEY_X,
        eY = GLFW_KEY_Y,
        eZ = GLFW_KEY_Z,
        eLeftBracket = GLFW_KEY_LEFT_BRACKET,
        eBackslash = GLFW_KEY_BACKSLASH,
        eRightBracket = GLFW_KEY_RIGHT_BRACKET,
        eGraveAccent = GLFW_KEY_GRAVE_ACCENT,
        eWorld1 = GLFW_KEY_WORLD_1,
        eWorld2 = GLFW_KEY_WORLD_2,
        eEscape = GLFW_KEY_ESCAPE,
        eEnter = GLFW_KEY_ENTER,
        eTab = GLFW_KEY_TAB,
        eBackspace = GLFW_KEY_BACKSPACE,
        eInsert = GLFW_KEY_INSERT,
        eDelete = GLFW_KEY_DELETE,
        eRight = GLFW_KEY_RIGHT,
        eLeft = GLFW_KEY_LEFT,
        eDown = GLFW_KEY_DOWN,
        eUp = GLFW_KEY_UP,
        ePageUp = GLFW_KEY_PAGE_UP,
        ePageDown = GLFW_KEY_PAGE_DOWN,
        eHome = GLFW_KEY_HOME,
        eEnd = GLFW_KEY_END,
        eCapsLock = GLFW_KEY_CAPS_LOCK,
        eScrollLock = GLFW_KEY_SCROLL_LOCK,
        eNumLock = GLFW_KEY_NUM_LOCK,
        ePrintScreen = GLFW_KEY_PRINT_SCREEN,
        ePause = GLFW_KEY_PAUSE,
        eF1 = GLFW_KEY_F1,
        eF2 = GLFW_KEY_F2,
        eF3 = GLFW_KEY_F3,
        eF4 = GLFW_KEY_F4,
        eF5 = GLFW_KEY_F5,
        eF6 = GLFW_KEY_F6,
        eF7 = GLFW_KEY_F7,
        eF8 = GLFW_KEY_F8,
        eF9 = GLFW_KEY_F9,
        eF10 = GLFW_KEY_F10,
        eF11 = GLFW_KEY_F11,
        eF12 = GLFW_KEY_F12,
        eF13 = GLFW_KEY_F13,
        eF14 = GLFW_KEY_F14,
        eF15 = GLFW_KEY_F15,
        eF16 = GLFW_KEY_F16,
        eF17 = GLFW_KEY_F17,
        eF18 = GLFW_KEY_F18,
        eF19 = GLFW_KEY_F19,
        eF20 = GLFW_KEY_F20,
        eF21 = GLFW_KEY_F21,
        eF22 = GLFW_KEY_F22,
        eF23 = GLFW_KEY_F23,
        eF24 = GLFW_KEY_F24,
        eF25 = GLFW_KEY_F25,
        eNumpad0 = GLFW_KEY_KP_0,
        eNumpad1 = GLFW_KEY_KP_1,
        eNumpad2 = GLFW_KEY_KP_2,
        eNumpad3 = GLFW_KEY_KP_3,
        eNumpad4 = GLFW_KEY_KP_4,
        eNumpad5 = GLFW_KEY_KP_5,
        eNumpad6 = GLFW_KEY_KP_6,
        eNumpad7 = GLFW_KEY_KP_7,
        eNumpad8 = GLFW_KEY_KP_8,
        eNumpad9 = GLFW_KEY_KP_9,
        eNumpadDecimal = GLFW_KEY_KP_DECIMAL,
        eNumpadDivide = GLFW_KEY_KP_DIVIDE,
        eNumpadMultiply = GLFW_KEY_KP_MULTIPLY,
        eNumpadSubtract = GLFW_KEY_KP_SUBTRACT,
        eNumpadAdd = GLFW_KEY_KP_ADD,
        eNumpadEnter = GLFW_KEY_KP_ENTER,
        eNumpadEqual = GLFW_KEY_KP_EQUAL,
        eLeftShift = GLFW_KEY_LEFT_SHIFT,
        eLeftControl = GLFW_KEY_LEFT_CONTROL,
        eLeftAlt = GLFW_KEY_LEFT_ALT,
        eLeftSuper = GLFW_KEY_LEFT_SUPER,
        eRightShift = GLFW_KEY_RIGHT_SHIFT,
        eRightControl = GLFW_KEY_RIGHT_CONTROL,
        eRightAlt = GLFW_KEY_RIGHT_ALT,
        eRightSuper = GLFW_KEY_RIGHT_SUPER,
        eMenu = GLFW_KEY_MENU,
    };

    enum class Button {
        eLeft = GLFW_MOUSE_BUTTON_LEFT,
        eRight = GLFW_MOUSE_BUTTON_RIGHT,
        eMiddle = GLFW_MOUSE_BUTTON_MIDDLE
    };

    enum class CursorInputMode {
        eNormal = GLFW_CURSOR_NORMAL,
        eInfinite = GLFW_CURSOR_DISABLED
    };
}

namespace vkr::io::event {
    struct MouseOffset {
        glm::vec2 offset;
    };

    struct MousePress {
        io::Button button;
    };

    struct MouseRelease {
        io::Button button;
    };

    struct KeyPress {
        io::Key key;
    };

    struct KeyRelease {
        io::Key key;
    };

    struct WindowResize {
        glm::ivec2 size;
    };
}

namespace vkr::io::console {
    auto read() -> std::string;
}

namespace vkr::io::file {
    template <class T>
    auto read(const char* file)->std::vector<T>;
}

namespace vkr::io {
    class Window {
    private:
        static auto framebufferResizeCallback(glfw::GLFWwindow* window, int width, int height) -> void;
        static auto cursorPosCallback(glfw::GLFWwindow* window, double x, double y) -> void;
        bool cursorPosSuppress = false;
        static auto mouseButtonCallback(glfw::GLFWwindow* window, int button, int action, int mods) -> void;
        static auto keyCallback(glfw::GLFWwindow* window, int key, int scancode, int action, int mods) -> void;
        static auto windowRefreshCallback(glfw::GLFWwindow* window) -> void;
        static auto windowSizeCallback(glfw::GLFWwindow* window, int width, int height) -> void;
    protected:
        Window(const char* title, glm::ivec2 size, bool visible);
        Window(const Window&) = delete;
    public:
        auto getKeyboardScalar(io::Key positive, io::Key negative) -> float;
        auto getKeyboardVector(io::Key positiveX, io::Key negativeX, io::Key positiveY, io::Key negativeY) -> glm::vec2;
    public:
        ~Window();
        auto setClosed(bool closed) -> void;
        auto getKeyPressed(Key key) -> bool;
        auto setCursorPos(glm::dvec2 position) -> void;
        auto getCursorPos() -> glm::dvec2;
        auto setCursorInputMode(CursorInputMode cursorMode) -> void;
        auto getCursorInputMode() -> CursorInputMode;
        auto getMouseButtonPressed(io::Button button) -> bool;
        auto setFullscreen(bool fullscreen) -> void;
        auto getFullscreen() -> bool;
        auto getSize() -> glm::ivec2;
        auto setSize(glm::ivec2 dimentions) -> void;
        auto getPos() -> glm::ivec2;
        auto setPos(glm::ivec2 position) -> void;
        auto setVisible(bool visible) -> void;
        auto setTitle(const std::string& title) -> void;
    public:
        template <class E>
        auto on(std::function<void(E e, bool& handled)> callback) {
            if (events.count(typeid(E))) {
                bool handled = false;
                callback(std::any_cast<E>(events[typeid(E)]), handled);
                if (handled) {
                    events.erase(typeid(E));
                }
            }
        }
    protected:
        template <class E>
        auto fire(E e) -> void {
            events[typeid(E)] = e;
        }
    protected:
        auto poll() -> void;
        auto getWindowPointer() -> glfw::GLFWwindow*;
        auto setRefreshCallback(std::function<void()> callback) -> void;
        auto setFramebufferResizeCallback(std::function<void()> callback) -> void;
        auto getClosed() const -> bool;
    private:
        static auto self(glfw::GLFWwindow* window) -> Window&;
    private:
        glfw::GLFWwindow* window;
    private:
        std::unordered_map<std::type_index, std::any> events;
    private:
        std::function<void()> onRefresh = []() {};
        std::function<void()> rebuildForResize = []() {};
    };
}