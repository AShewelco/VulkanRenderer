#pragma once

namespace vkr::data {
    struct Vertex {
        glm::vec3 position = {};
        glm::vec3 color = {};
        glm::vec2 textureCoordinates = {};
        auto operator==(const Vertex& other) const -> bool;
    };

    struct UBO {
        alignas(16) glm::mat4 model = glm::mat4(1.0f);
        alignas(16) glm::mat4 view = glm::mat4(1.0f);
        alignas(16) glm::mat4 projection = glm::mat4(1.0f);
    };

    struct Camera {
        auto getDirection() -> glm::vec3;
        auto getForward() -> glm::vec3;
        auto getLeft() -> glm::vec3;

        float fov = glm::radians(90.0f);
        glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f);
        float pitch = 0.0f;
        float yaw = 0.0f;
    };

    class Texture {
    public:
        Texture();
        Texture(const char* file);
        Texture(const Texture&) = delete;
        Texture(Texture&& other);
        auto operator=(Texture&& other) -> void;
        ~Texture();
        auto getDimentions() const->glm::ivec2;
        auto getPixels() const -> const uint8_t*;
        auto getSize() const->size_t;
    private:
        glm::ivec2 size = glm::ivec2(0, 0);
        stb::stbi_uc* data = nullptr;
    };

    class Model {
    public:
        Model() = default;
        Model(const char* file);
        std::vector<data::Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}

namespace std {
    template<> struct hash<vkr::data::Vertex> {
        size_t operator()(vkr::data::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.textureCoordinates) << 1);
        }
    };
}
