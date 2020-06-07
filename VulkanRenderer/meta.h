#pragma once

namespace vkr::meta {
    namespace detail {
        template <size_t I, class V>
        constexpr auto getFieldSize() -> size_t {
            auto type = boost::pfr::get<I, V>(V {});
            return sizeof(decltype(type));
        }

        template <size_t I, class V>
        struct FieldOffset {
            constexpr static auto get() -> size_t {
                return FieldOffset<I - 1, V>::get() + getFieldSize<I - 1, V>();
            }
        };

        template <class V>
        struct FieldOffset<0, V> {
            constexpr static auto get() -> size_t {
                return 0;
            }
        };

        template <size_t I, class V>
        constexpr auto getFieldOffset() -> size_t {
            return FieldOffset<I, V>::get();
        }

        template <class F>
        constexpr auto getFieldFormat() -> vk::Format {
            if (std::is_same_v<F, glm::f32vec1>) {
                return vk::Format::eR32Sfloat;
            }
            if (std::is_same_v<F, glm::f32vec2>) {
                return vk::Format::eR32G32Sfloat;
            }
            if (std::is_same_v<F, glm::f32vec3>) {
                return vk::Format::eR32G32B32Sfloat;
            }
            if (std::is_same_v<F, glm::f32vec4>) {
                return vk::Format::eR32G32B32A32Sfloat;
            }
            if (std::is_same_v<F, glm::f64vec1>) {
                return vk::Format::eR64Sfloat;
            }
            if (std::is_same_v<F, glm::f64vec2>) {
                return vk::Format::eR64G64Sfloat;
            }
            if (std::is_same_v<F, glm::f64vec3>) {
                return vk::Format::eR64G64B64Sfloat;
            }
            if (std::is_same_v<F, glm::f64vec4>) {
                return vk::Format::eR64G64B64A64Sfloat;
            }
            if (std::is_same_v<F, glm::i32vec1>) {
                return vk::Format::eR32Sint;
            }
            if (std::is_same_v<F, glm::i32vec2>) {
                return vk::Format::eR32G32Sint;
            }
            if (std::is_same_v<F, glm::i32vec3>) {
                return vk::Format::eR32G32B32Sint;
            }
            if (std::is_same_v<F, glm::i32vec4>) {
                return vk::Format::eR32G32B32A32Sint;
            }
            if (std::is_same_v<F, glm::i64vec1>) {
                return vk::Format::eR64Sint;
            }
            if (std::is_same_v<F, glm::i64vec2>) {
                return vk::Format::eR64G64Sint;
            }
            if (std::is_same_v<F, glm::i64vec3>) {
                return vk::Format::eR64G64B64Sint;
            }
            if (std::is_same_v<F, glm::i64vec4>) {
                return vk::Format::eR64G64B64A64Sint;
            }
            if (std::is_same_v<F, glm::u32vec1>) {
                return vk::Format::eR32Uint;
            }
            if (std::is_same_v<F, glm::u32vec2>) {
                return vk::Format::eR32G32Uint;
            }
            if (std::is_same_v<F, glm::u32vec3>) {
                return vk::Format::eR32G32B32Uint;
            }
            if (std::is_same_v<F, glm::u32vec4>) {
                return vk::Format::eR32G32B32A32Uint;
            }
            if (std::is_same_v<F, glm::u64vec1>) {
                return vk::Format::eR64Uint;
            }
            if (std::is_same_v<F, glm::u64vec2>) {
                return vk::Format::eR64G64Uint;
            }
            if (std::is_same_v<F, glm::u64vec3>) {
                return vk::Format::eR64G64B64Uint;
            }
            if (std::is_same_v<F, glm::u64vec4>) {
                return vk::Format::eR64G64B64A64Uint;
            }
            assert(!"Format is not supported");
            return vk::Format::eUndefined;
        }

        template <size_t I, class V>
        constexpr auto getAttributeDescription() -> vk::VertexInputAttributeDescription {
            vk::VertexInputAttributeDescription result;
            auto type = boost::pfr::get<I, V>(V {});
            result.location = static_cast<uint32_t>(I);
            result.format = getFieldFormat<decltype(type)>();
            result.offset = static_cast<uint32_t>(getFieldOffset<I, V>());
            return result;
        }

        template <size_t S, size_t I, class V>
        struct AttributeDescriptions {
            static constexpr auto get() {
                auto result = AttributeDescriptions<S, I - 1, V>::get();
                result[I] = getAttributeDescription<I, V>();
                return result;
            }
        };

        template <size_t S, class V>
        struct AttributeDescriptions<S, 0, V> {
            static constexpr auto get() {
                std::array<vk::VertexInputAttributeDescription, S> result;
                result[0] = getAttributeDescription<0, V>();
                return result;
            }
        };
    }

    template <class V>
    constexpr auto getAttributeDescriptions() {
        return detail::AttributeDescriptions<boost::pfr::detail::fields_count<V>(), boost::pfr::detail::fields_count<V>() - 1, V>::get();
    }
}