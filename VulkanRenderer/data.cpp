#include "data.h"

namespace vkr::data {
    auto Vertex::operator==(const Vertex& other) const -> bool {
        return position == other.position && color == other.color && textureCoordinates == other.textureCoordinates;
    }

    auto Camera::getDirection() -> glm::vec3 {
        return glm::rotateZ(glm::rotateX(glm::vec3(0.0f, 0.0f, 1.0f), pitch), yaw);
    }

    auto Camera::getForward() -> glm::vec3 {
        return glm::rotateZ(glm::vec3(0.0f, 1.0f, 0.0f), yaw);
    }

    auto Camera::getLeft() -> glm::vec3 {
        return glm::rotateZ(glm::vec3(1.0f, 0.0f, 0.0f), yaw);
    }

    Texture::Texture() {
        data = new uint8_t[4];
        data[0] = 255;
        data[1] = 255;
        data[2] = 255;
        data[3] = 255;
        size = glm::ivec2(1, 1);
    }

    Texture::Texture(const char* file) {
        int temp;
        data = stb::stbi_load(file, &size.x, &size.y, &temp, stb::STBI_rgb_alpha);
        if (!data) {
            throw std::runtime_error(fmt::format("STB: Failed to load a texture {}", file));
        }
    }

    Texture::Texture(Texture&& other) {
        std::swap(size, other.size);
        std::swap(data, other.data);
    }

    auto Texture::operator=(Texture&& other) -> void {
        size = other.size;
        data = other.data;
        other.size = glm::ivec2(0, 0);
        other.data = nullptr;
    }

    Texture::~Texture() {
        if (data) {
            stb::stbi_image_free(data);
        }
    }

    auto Texture::getDimentions() const -> glm::ivec2 {
        return size;
    }

    auto Texture::getPixels() const -> const uint8_t* {
        return data;
    }

    auto Texture::getSize() const -> size_t {
        return static_cast<size_t>(size.x) * static_cast<size_t>(size.y) * sizeof(uint32_t);
    }

    Model::Model(const char* file) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file)) {
            throw std::runtime_error(fmt::format("TinyObj: {}; {};", warn, err));
        }

        std::unordered_map<data::Vertex, uint32_t> uniqueVertices;

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                data::Vertex vertex;

                vertex.position = {
                    attrib.vertices[static_cast<size_t>(index.vertex_index) * 3 + 0],
                    attrib.vertices[static_cast<size_t>(index.vertex_index) * 3 + 1],
                    attrib.vertices[static_cast<size_t>(index.vertex_index) * 3 + 2]
                };

                vertex.textureCoordinates = {
                    attrib.texcoords[static_cast<size_t>(index.texcoord_index) * 2 + 0],
                    1.0f - attrib.texcoords[static_cast<size_t>(index.texcoord_index) * 2 + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}