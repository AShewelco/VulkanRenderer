#pragma once
#include "io.h"
#include "data.h"

namespace vkr::api {
    enum class DebuggerMinimunLevel {
        eVerbose,
        eInfo,
        eWarning,
        eError,
        eDisabled
    };

    struct RendererCreateInfo {
        DebuggerMinimunLevel debuggerMinimumLevel = DebuggerMinimunLevel::eDisabled;
        vk::SampleCountFlagBits maxAntialiasing = vk::SampleCountFlagBits::e1;
        data::Texture texture;
        std::function<size_t(std::vector<vk::PhysicalDeviceProperties>)> deviceSelector = [](std::vector<vk::PhysicalDeviceProperties>) {
            return 0;
        };
        std::function<void(float delta, float time)> onUpdate = [](float, float) {};
        io::WindowCreateInfo windowCreateInfo;
    };
}