#include "main.h"

namespace vkr::api {
    auto Renderer::start() -> void {
        part::BeginPart::renderer = this;
        part::LastPart part;
        this->part = &part;
        part.start();
    }
    
    auto Renderer::getWindow() -> io::Window& {
        return *static_cast<io::Window*>(part);
    }
    
    auto Renderer::updateVertices() -> void {
        part->updateVertices();
    }
    
    auto Renderer::setVertexData(const data::Model& data) -> void {
        part->setVertexData(data);
    }
}

namespace vkr::part {
    api::Renderer* BeginPart::renderer = nullptr;

    BeginPart::BeginPart() {
        assert(renderer != nullptr);
        glfw::glfwSetErrorCallback([](int code, const char* message) {
            throw std::runtime_error(fmt::format("GLFW: {}", message));
        });

        if (!glfw::glfwInit()) {
            throw std::runtime_error("GLFW: Couldn't initialize");
        }

        if (!glfw::glfwVulkanSupported()) {
            throw std::runtime_error("GLFW: Vulkan is not supported");
        }
    }

    BeginPart::~BeginPart() {
        glfw::glfwTerminate();
    }

    auto BeginPart::getRenderer() -> api::Renderer& {
        return *renderer;
    }

    InstancePart::InstancePart() {
        using namespace std::string_literals;

        static vk::DynamicLoader l;
        VULKAN_HPP_DEFAULT_DISPATCHER.init(l.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        bool useDebugger = getRenderer().getDebuggerMinimumLevel() != api::DebuggerMinimunLevel::eDisabled;

        extentions = []() {
            std::vector<const char*> result;
            uint32_t count = 0;
            const char** result_ = glfw::glfwGetRequiredInstanceExtensions(&count);
            result.insert(result.end(), result_, result_ + static_cast<size_t>(count));
            return result;
        }();

        if (useDebugger) {
            using namespace algorithm;
            useDebugger = [&]() {
                if (!contains(vk::enumerateInstanceLayerProperties(), [](auto& p) { return "VK_LAYER_KHRONOS_validation"s == p.layerName; })) {
                    spdlog::warn("VK_LAYER_KHRONOS_validation is not available");
                    return false;
                }

                if (!contains(vk::enumerateInstanceExtensionProperties(), [](auto& p) { return "VK_EXT_debug_utils"s == p.extensionName; })) {
                    spdlog::warn("VK_EXT_debug_utils is not available");
                    return false;
                }

                layers.push_back("VK_LAYER_KHRONOS_validation");
                extentions.push_back("VK_EXT_debug_utils");

                return true;
            }();
        }
        
        vk::ApplicationInfo applicationInfo;
        applicationInfo.apiVersion = VK_API_VERSION_1_2;

        vk::InstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instanceCreateInfo.ppEnabledLayerNames = layers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extentions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extentions.data();

        instance = vk::createInstanceUnique(instanceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        if (useDebugger) {
            vk::DebugUtilsMessengerCreateInfoEXT info;

            if (getRenderer().getDebuggerMinimumLevel() == api::DebuggerMinimunLevel::eVerbose) {
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            }
            else if (getRenderer().getDebuggerMinimumLevel() == api::DebuggerMinimunLevel::eInfo) {
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            }
            else if (getRenderer().getDebuggerMinimumLevel() == api::DebuggerMinimunLevel::eWarning) {
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            }
            else if (getRenderer().getDebuggerMinimumLevel() == api::DebuggerMinimunLevel::eError) {
                info.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            }

            info.messageType |= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral;
            info.messageType |= vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
            info.messageType |= vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

            info.pfnUserCallback = debugCallback;

            debugger = getInstance().createDebugUtilsMessengerEXTUnique(info);
        }
    }

    auto InstancePart::getInstance() -> vk::Instance {
        return *instance;
    }

    auto InstancePart::getInstanceExtentions() -> const std::vector<const char*>& {
        return extentions;
    }

    auto InstancePart::getInstanceLayers() -> const std::vector<const char*>& {
        return layers;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL InstancePart::debugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT typeFlags,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData
    ) {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            spdlog::error(callbackData->pMessage);
            __debugbreak();
        }

        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::warn(callbackData->pMessage);
        }

        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            spdlog::info(callbackData->pMessage);
        }

        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            spdlog::info(callbackData->pMessage);
        }

        return VK_TRUE;
    }

    SurfacePart::SurfacePart() : io::Window("", glm::ivec2(640, 480), false) {
        VkSurfaceKHR surface_;
        glfw::glfwCreateWindowSurface((VkInstance)(getInstance()), getWindowPointer(), nullptr, &surface_);
        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> deleter(getInstance());
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(surface_), deleter);
    }

    auto SurfacePart::getSurface() -> vk::SurfaceKHR {
        return *surface;
    }

    PhysicalDevicePart::PhysicalDevicePart() {
        using namespace std::string_literals;

        {
            std::vector<vk::PhysicalDevice> devices = getInstance().enumeratePhysicalDevices();
            if (devices.size() == 0) {
                throw std::runtime_error("No physical devices with Vulkan support");
            }
            else if (devices.size() == 1) {
                device = devices.front();
            }
            else {
                using namespace algorithm;
                std::vector<vk::PhysicalDeviceProperties> properties = select(devices, [](auto d) { return d.getProperties(); });
                device = devices.at(getRenderer().selectPhysicalDevice(properties));
            }
        }

        bool found = false;
        for (auto& properties : device.enumerateDeviceExtensionProperties()) {
            if ("VK_KHR_swapchain"s == properties.extensionName) {
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("Device doesn't support swapchain extension");
        }

        extentions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        std::vector<vk::QueueFamilyProperties> properties = device.getQueueFamilyProperties();
        bool foundGraphics = false;
        bool foundPresent = false;

        for (uint32_t i = 0; i < static_cast<uint32_t>(properties.size()); ++i) {
            if (!foundGraphics && (properties[static_cast<size_t>(i)].queueFlags & vk::QueueFlagBits::eGraphics)) {
                queueFamilyIndices[0] = i;
                foundGraphics = true;
            }
            if (!foundPresent && device.getSurfaceSupportKHR(i, getSurface())) {
                queueFamilyIndices[1] = i;
                foundPresent = true;
            }
        }

        if (!foundGraphics) {
            throw std::runtime_error("Couldn't find graphics family queue index");
        }

        if (!foundPresent) {
            throw std::runtime_error("Couldn't find present family queue index");
        }
    }

    auto PhysicalDevicePart::getPhysicalDevice() -> const vk::PhysicalDevice& {
        return device;
    }

    auto PhysicalDevicePart::getPhysicalDeviceExtentions() -> const std::vector<const char*>& {
        return extentions;
    }

    auto PhysicalDevicePart::getGraphicsQueueFamilyIndex() -> uint32_t {
        return queueFamilyIndices[0];
    }

    auto PhysicalDevicePart::getPresentQueueFamilyIndex() -> uint32_t {
        return queueFamilyIndices[1];
    }

    auto PhysicalDevicePart::getQueueFamilyIndices() -> const std::array<uint32_t, 2>& {
        return queueFamilyIndices;
    }

    PhysicalDeviceDataPart::PhysicalDeviceDataPart() {
        surfaceFormats = getPhysicalDevice().getSurfaceFormatsKHR(getSurface());

        if (surfaceFormats.empty()) {
            throw std::runtime_error("No surface formats are available");
        }

        surfacePresentModes = getPhysicalDevice().getSurfacePresentModesKHR(getSurface());

        if (surfacePresentModes.empty()) {
            throw std::runtime_error("No present modes are available");
        }

        vk::PhysicalDeviceFeatures features = getPhysicalDevice().getFeatures();
        if (!features.samplerAnisotropy) {
            throw std::runtime_error("Physical device doesn't support anisotropic filtering");
        }

        depthFormat = [&]() {
            for (vk::Format format : { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }) {
                vk::FormatProperties properties = getPhysicalDevice().getFormatProperties(format);
                if ((properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) == vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
                    return format;
                }
            }

            throw std::runtime_error("Failed to find supported format");
        }();

        msaaSamples = [&]() {
            vk::PhysicalDeviceProperties physicalDeviceProperties = getPhysicalDevice().getProperties();

            vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
            if (counts & vk::SampleCountFlagBits::e64) {
                return vk::SampleCountFlagBits::e64;
            }
            if (counts & vk::SampleCountFlagBits::e32) {
                return vk::SampleCountFlagBits::e32;
            }
            if (counts & vk::SampleCountFlagBits::e16) {
                return vk::SampleCountFlagBits::e16;
            }
            if (counts & vk::SampleCountFlagBits::e8) {
                return vk::SampleCountFlagBits::e8;
            }
            if (counts & vk::SampleCountFlagBits::e4) {
                return vk::SampleCountFlagBits::e4;
            }
            if (counts & vk::SampleCountFlagBits::e2) {
                return vk::SampleCountFlagBits::e2;
            }

            return vk::SampleCountFlagBits::e1;
        }();
    }

    auto PhysicalDeviceDataPart::getSurfaceFormats() -> const std::vector<vk::SurfaceFormatKHR>& {
        return surfaceFormats;
    }

    auto PhysicalDeviceDataPart::getSurfacePresentModes() -> const std::vector<vk::PresentModeKHR>& {
        return surfacePresentModes;
    }

    auto PhysicalDeviceDataPart::getMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) -> uint32_t {
        vk::PhysicalDeviceMemoryProperties memoryProperties = getPhysicalDevice().getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Couldn't find suitable memory type");
    }

    auto PhysicalDeviceDataPart::getDepthFormat() -> vk::Format {
        return depthFormat;
    }

    auto PhysicalDeviceDataPart::getMsaaSamples() -> vk::SampleCountFlagBits {
        return msaaSamples;
    }

    auto PhysicalDeviceDataPart::getCurrentExtent() -> vk::Extent2D {
        return getPhysicalDevice().getSurfaceCapabilitiesKHR(getSurface()).currentExtent;
    }

    DevicePart::DevicePart() {
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::unordered_set<uint32_t> uniqueQueueFamilyIndices;
        uniqueQueueFamilyIndices.insert(getGraphicsQueueFamilyIndex());
        uniqueQueueFamilyIndices.insert(getPresentQueueFamilyIndex());

        float queuePriority = 1.0f;

        for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices) {
            vk::DeviceQueueCreateInfo info;
            info.pQueuePriorities = &queuePriority;
            info.queueCount = 1;
            info.queueFamilyIndex = queueFamilyIndex;
            queueCreateInfos.push_back(info);
        }

        vk::PhysicalDeviceFeatures features;
        features.samplerAnisotropy = true;

        vk::DeviceCreateInfo info;
        info.queueCreateInfoCount = (uint32_t)(queueCreateInfos.size());
        info.pQueueCreateInfos = queueCreateInfos.data();
        info.enabledLayerCount = (uint32_t)(getInstanceLayers().size());
        info.ppEnabledLayerNames = getInstanceLayers().data();
        info.enabledExtensionCount = (uint32_t)(getPhysicalDeviceExtentions().size());
        info.ppEnabledExtensionNames = getPhysicalDeviceExtentions().data();
        info.pEnabledFeatures = &features;

        device = getPhysicalDevice().createDeviceUnique(info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
    }

    auto DevicePart::getDevice() -> vk::Device {
        return *device;
    }

    auto DevicePart::getGraphicsQueue() -> vk::Queue {
        return device->getQueue(getGraphicsQueueFamilyIndex(), 0);
    }

    auto DevicePart::getPresentQueue() -> vk::Queue {
        return device->getQueue(getPresentQueueFamilyIndex(), 0);
    }

    auto DevicePart::makeBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) -> std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory> {
        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::UniqueBuffer buffer = device->createBufferUnique(bufferCreateInfo);

        vk::MemoryRequirements memoryRequirements = device->getBufferMemoryRequirements(*buffer);

        vk::MemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements.memoryTypeBits, properties);

        vk::UniqueDeviceMemory memory = device->allocateMemoryUnique(memoryAllocateInfo);

        getDevice().bindBufferMemory(*buffer, *memory, 0);

        return { std::move(buffer), std::move(memory) };
    }

    auto DevicePart::makeImage(glm::uvec2 size, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties) -> std::tuple<vk::UniqueImage, vk::UniqueDeviceMemory> {
        vk::ImageCreateInfo imageCreateInfo;
        imageCreateInfo.imageType = vk::ImageType::e2D;
        imageCreateInfo.extent.width = size.x;
        imageCreateInfo.extent.height = size.y;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageCreateInfo.usage = usage;
        imageCreateInfo.samples = sampleCount;
        imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;

        vk::UniqueImage image = device->createImageUnique(imageCreateInfo);
        vk::MemoryRequirements memoryRequirements = device->getImageMemoryRequirements(*image);

        vk::MemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirements.memoryTypeBits, properties);

        vk::UniqueDeviceMemory memory = device->allocateMemoryUnique(memoryAllocateInfo);

        device->bindImageMemory(*image, *memory, 0);

        return { std::move(image), std::move(memory) };
    }

    auto DevicePart::makeImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) -> vk::UniqueImageView {
        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        return device->createImageViewUnique(imageViewCreateInfo);
    }

    SwapchainPart::SwapchainPart() {
        buildSwapchain(getCurrentExtent());
    }

    auto SwapchainPart::getSwapchain() -> vk::SwapchainKHR {
        return *swapchain;
    }

    auto SwapchainPart::getSwapchainFormat() -> vk::Format {
        return format;
    }

    auto SwapchainPart::buildSwapchain(vk::Extent2D extent) -> void {
        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.surface = getSurface(); // surface

        [&]() { // imageFormat, imageColorSpace
            for (auto& format : getSurfaceFormats()) {
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    createInfo.imageFormat = format.format;
                    createInfo.imageColorSpace = format.colorSpace;
                    return;
                }
            }
            createInfo.imageFormat = getSurfaceFormats()[0].format;
            createInfo.imageColorSpace = getSurfaceFormats()[0].colorSpace;
        }();

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = getPhysicalDevice().getSurfaceCapabilitiesKHR(getSurface());
        createInfo.imageExtent = extent;
        createInfo.preTransform = surfaceCapabilities.currentTransform; // preTransform

        format = createInfo.imageFormat;

        // minImageCount, maxImageCount
        createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount != 0) { // if there is maximum
            if (createInfo.minImageCount > surfaceCapabilities.maxImageCount) {
                createInfo.minImageCount = surfaceCapabilities.maxImageCount;
            }
        }

        // presentMode
        [&]() {
            for (auto& mode : getSurfacePresentModes()) {
                if (mode == vk::PresentModeKHR::eMailbox) {
                    createInfo.presentMode = vk::PresentModeKHR::eMailbox;
                    return;
                }
            }
            createInfo.presentMode = vk::PresentModeKHR::eFifo;
        }();

        createInfo.imageArrayLayers = 1;                                    // imageArrayParts
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;   // imageUsage
        createInfo.clipped = true;                                          // clipped
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; // compositeAlpha

        if (getGraphicsQueueFamilyIndex() != getPresentQueueFamilyIndex()) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;                    // imageSharingMode
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(getQueueFamilyIndices().size()); // queueFamilyIndexCount
            createInfo.pQueueFamilyIndices = getQueueFamilyIndices().data();               // pQueueFamilyIndices
        }
        else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;                     // imageSharingMode
        }

        swapchain = getDevice().createSwapchainKHRUnique(createInfo); // swapchain
    }

    ImagesPart::ImagesPart() {
        buildImages(getCurrentExtent());
    }

    auto ImagesPart::getSwapchainImageViews() -> std::vector<vk::ImageView> {
        return vk::uniqueToRaw(uniqueImageViews);
    }

    auto ImagesPart::getSwapchainImageCount() -> size_t {
        return images.size();
    }

    auto ImagesPart::buildImages(vk::Extent2D extent) -> void {
        uniqueImageViews.clear();

        images = getDevice().getSwapchainImagesKHR(getSwapchain());

        for (const auto& image : images) {
            uniqueImageViews.push_back(makeImageView(image, getSwapchainFormat(), vk::ImageAspectFlagBits::eColor, 1));
        }
    }

    RenderPassPart::RenderPassPart() {
        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = getSwapchainFormat();
        colorAttachment.samples = getMsaaSamples();
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentDescription depthAttachment;
        depthAttachment.format = getDepthFormat();
        depthAttachment.samples = getMsaaSamples();
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentDescription colorAttachmentResolve;
        colorAttachmentResolve.format = getSwapchainFormat();
        colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
        colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentReference;
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depthAttachmentReference;
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference colorAttachmentResolveReference;
        colorAttachmentResolveReference.attachment = 2;
        colorAttachmentResolveReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentReference;
        subpass.pDepthStencilAttachment = &depthAttachmentReference;
        subpass.pResolveAttachments = &colorAttachmentResolveReference;

        vk::SubpassDependency dependency;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = {};
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        std::array attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

        vk::RenderPassCreateInfo info;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        renderPass = getDevice().createRenderPassUnique(info);
    }

    auto RenderPassPart::getRenderPass() -> vk::RenderPass {
        return *renderPass;
    }

    DescriptorSetLayoutPart::DescriptorSetLayoutPart() {
        vk::DescriptorSetLayoutBinding uboLayoutBinding;
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

        vk::DescriptorSetLayoutBinding samplerLayoutBinding;
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        std::array bindings = { uboLayoutBinding, samplerLayoutBinding };

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
        layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutCreateInfo.pBindings = bindings.data();

        descriptorSetLayout = getDevice().createDescriptorSetLayoutUnique(layoutCreateInfo);
    }

    auto DescriptorSetLayoutPart::getDescriptorSetLayout() -> vk::DescriptorSetLayout {
        return *descriptorSetLayout;
    }

    GraphicsPipelinePart::GraphicsPipelinePart() {
        buildGraphicsPipeline(getCurrentExtent());
    }

    auto GraphicsPipelinePart::getGraphicsPipeline() -> vk::Pipeline {
        return *pipeline;
    }

    auto GraphicsPipelinePart::getGraphicsPipelineLayout() -> vk::PipelineLayout {
        return *layout;
    }

    auto GraphicsPipelinePart::buildGraphicsPipeline(vk::Extent2D extent) -> void {
        vk::DescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        layout = getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);

        auto makeShaderModule = [&](const std::vector<uint32_t>& code) {
            vk::ShaderModuleCreateInfo info;
            info.codeSize = code.size() * sizeof(uint32_t);
            info.pCode = code.data();

            return getDevice().createShaderModuleUnique(info);
        };

        vk::UniqueShaderModule vertexShaderModule = makeShaderModule(io::file::read<uint32_t>("shaders/default.vert.spv"));
        vk::UniqueShaderModule fragmentShaderModule = makeShaderModule(io::file::read<uint32_t>("shaders/default.frag.spv"));
        
        std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
        stageCreateInfos[0].stage = vk::ShaderStageFlagBits::eVertex;
        stageCreateInfos[0].setModule(*vertexShaderModule);
        stageCreateInfos[0].pName = "main";

        stageCreateInfos[1].stage = vk::ShaderStageFlagBits::eFragment;
        stageCreateInfos[1].setModule(*fragmentShaderModule);
        stageCreateInfos[1].pName = "main";

        vk::VertexInputBindingDescription vertexBindingDescription;
        vertexBindingDescription.binding = 0;
        vertexBindingDescription.stride = sizeof(data::Vertex);
        vertexBindingDescription.inputRate = vk::VertexInputRate::eVertex;
        
        std::array vertexAttributeDescriptions = meta::getAttributeDescriptions<data::Vertex>();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

        vk::PipelineInputAssemblyStateCreateInfo assemblyStateInfo;
        assemblyStateInfo.topology = vk::PrimitiveTopology::eTriangleList;
        assemblyStateInfo.primitiveRestartEnable = false;

        vk::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(extent.height);
        viewport.width = static_cast<float>(extent.width);
        viewport.height = -static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = extent;

        vk::PipelineViewportStateCreateInfo viewportStateInfo;
        viewportStateInfo.viewportCount = 1;
        viewportStateInfo.pViewports = &viewport;
        viewportStateInfo.scissorCount = 1;
        viewportStateInfo.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
        rasterizerInfo.depthClampEnable = false;
        rasterizerInfo.rasterizerDiscardEnable = false;
        rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizerInfo.lineWidth = 1.0f;
        rasterizerInfo.cullMode = vk::CullModeFlagBits::eNone;
        rasterizerInfo.frontFace = vk::FrontFace::eCounterClockwise;
        rasterizerInfo.depthBiasEnable = false;
        rasterizerInfo.depthBiasConstantFactor = 0.0f;
        rasterizerInfo.depthBiasClamp = 0.0f;
        rasterizerInfo.depthBiasSlopeFactor = 0.0f;

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.sampleShadingEnable = false;
        multisampling.rasterizationSamples = getMsaaSamples();
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = false;
        multisampling.alphaToOneEnable = false;

        vk::PipelineDepthStencilStateCreateInfo depthStencil;
        depthStencil.depthTestEnable = true;
        depthStencil.depthWriteEnable = true;
        depthStencil.depthCompareOp = vk::CompareOp::eLess;
        depthStencil.depthBoundsTestEnable = false;
        depthStencil.stencilTestEnable = false;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask |= vk::ColorComponentFlagBits::eR;
        colorBlendAttachment.colorWriteMask |= vk::ColorComponentFlagBits::eG;
        colorBlendAttachment.colorWriteMask |= vk::ColorComponentFlagBits::eB;
        colorBlendAttachment.colorWriteMask |= vk::ColorComponentFlagBits::eA;

        colorBlendAttachment.blendEnable = false;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        vk::PipelineColorBlendStateCreateInfo colorBlendingInfo;
        colorBlendingInfo.logicOpEnable = false;
        colorBlendingInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendingInfo.attachmentCount = 1;
        colorBlendingInfo.pAttachments = &colorBlendAttachment;
        colorBlendingInfo.blendConstants[0] = 0.0f;
        colorBlendingInfo.blendConstants[1] = 0.0f;
        colorBlendingInfo.blendConstants[2] = 0.0f;
        colorBlendingInfo.blendConstants[3] = 0.0f;

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = stageCreateInfos.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
        pipelineCreateInfo.pInputAssemblyState = &assemblyStateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizerInfo;
        pipelineCreateInfo.pMultisampleState = &multisampling;
        pipelineCreateInfo.pDepthStencilState = &depthStencil;
        pipelineCreateInfo.pColorBlendState = &colorBlendingInfo;
        pipelineCreateInfo.pDynamicState = nullptr;
        pipelineCreateInfo.layout = *layout;
        pipelineCreateInfo.renderPass = getRenderPass();
        pipelineCreateInfo.subpass = 0;

        pipeline = getDevice().createGraphicsPipelineUnique({}, pipelineCreateInfo);
    }

    CommandPoolPart::CommandPoolPart() {
        vk::CommandPoolCreateInfo info;
        info.queueFamilyIndex = getGraphicsQueueFamilyIndex();
        commandPool = getDevice().createCommandPoolUnique(info);
    }

    auto CommandPoolPart::getCommandPool() -> vk::CommandPool {
        return *commandPool;
    }

    auto CommandPoolPart::copyBuffer(vk::Buffer from, vk::Buffer to, vk::DeviceSize size) -> void {
        executeSingleTimeCommands([&](const vk::CommandBuffer& commandBuffer) {
            vk::BufferCopy copyRegion;
            copyRegion.size = size;
            commandBuffer.copyBuffer(from, to, copyRegion);
            });
    }

    auto CommandPoolPart::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) -> void {
        executeSingleTimeCommands([&](const vk::CommandBuffer& commandBuffer) {
            vk::ImageMemoryBarrier imageMemoryBarrier;
            imageMemoryBarrier.oldLayout = oldLayout;
            imageMemoryBarrier.newLayout = newLayout;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;

            vk::PipelineStageFlags sourceStage;
            vk::PipelineStageFlags destinationStage;

            if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
                imageMemoryBarrier.srcAccessMask = {};
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            }
            else {
                throw std::invalid_argument("Unsupported layout transition");
            }

            commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, imageMemoryBarrier);
            });
    }

    auto CommandPoolPart::copyBufferToImage(vk::Buffer buffer, vk::Image image, glm::uvec2 size) -> void {
        executeSingleTimeCommands([&](const vk::CommandBuffer& commandBuffer) {
            vk::BufferImageCopy region;
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset.x = 0;
            region.imageOffset.y = 0;
            region.imageOffset.z = 0;
            region.imageExtent.width = size.x;
            region.imageExtent.height = size.y;
            region.imageExtent.depth = 1;

            commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
            });
    }

    auto CommandPoolPart::generateMipmaps(vk::Image image, vk::Format imageFormat, glm::ivec2 dimentionsBeforeFullscreen, uint32_t mipLevels) -> void {
        vk::FormatProperties formatProperties = getPhysicalDevice().getFormatProperties(imageFormat);

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
            throw std::runtime_error("Texture image format does not support linear blitting");
        }

        executeSingleTimeCommands([&](vk::CommandBuffer commandBuffer) {
            vk::ImageMemoryBarrier barrier;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            glm::ivec2 mipDimentions = dimentionsBeforeFullscreen;

            for (uint32_t i = 1; i < mipLevels; ++i) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);

                vk::ImageBlit blit;
                blit.srcOffsets[0].setX(0).setY(0).setZ(0);
                blit.srcOffsets[1].setX(mipDimentions.x).setY(mipDimentions.y).setZ(1);
                blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0].setX(0).setY(0).setZ(0);
                blit.dstOffsets[1].setX(mipDimentions.x > 1 ? mipDimentions.x / 2 : 1).setY(mipDimentions.y > 1 ? mipDimentions.y / 2 : 1).setZ(1);
                blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageBlit(blit), vk::Filter::eLinear);

                barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

                if (mipDimentions.x > 1) {
                    mipDimentions.x /= 2;
                }

                if (mipDimentions.y > 1) {
                    mipDimentions.y /= 2;
                }
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
            });


    }

    ColorResourcesPart::ColorResourcesPart() {
        buildColorResources(getCurrentExtent());
    }

    auto ColorResourcesPart::getColorImageView() -> vk::ImageView {
        return *colorImageView;
    }

    auto ColorResourcesPart::buildColorResources(vk::Extent2D extent) -> void {
        {
            std::tie(colorImage, colorImageMemory) = makeImage({ extent.width , extent.height },
                1,
                getMsaaSamples(),
                getSwapchainFormat(),
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
                vk::MemoryPropertyFlagBits::eDeviceLocal);
        }

        colorImageView = makeImageView(*colorImage, getSwapchainFormat(), vk::ImageAspectFlagBits::eColor, 1);
    }

    DepthPart::DepthPart() {
        buildDepthBuffer(getCurrentExtent());
    }

    auto DepthPart::getDepthImageView() -> vk::ImageView {
        return *depthImageView;
    }

    auto DepthPart::buildDepthBuffer(vk::Extent2D extent) -> void {
        {
            std::tie(depthImage, depthImageMemory) = makeImage({ extent.width, extent.height }, 1, getMsaaSamples(), getDepthFormat(), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
        }
        depthImageView = makeImageView(*depthImage, getDepthFormat(), vk::ImageAspectFlagBits::eDepth, 1);
    }

    FramebufferPart::FramebufferPart() {
        buildFramebuffers(getCurrentExtent());
    }

    auto FramebufferPart::getFramebuffers() -> std::vector<vk::Framebuffer> {
        return vk::uniqueToRaw(framebuffers);
    }

    auto FramebufferPart::buildFramebuffers(vk::Extent2D extent) -> void {
        framebuffers.clear();

        for (vk::ImageView view : getSwapchainImageViews()) {
            std::array attachments = { getColorImageView(), getDepthImageView(), view };

            vk::FramebufferCreateInfo info;
            info.renderPass = getRenderPass();
            info.attachmentCount = static_cast<uint32_t>(attachments.size());
            info.pAttachments = attachments.data();
            info.width = extent.width;
            info.height = extent.height;
            info.layers = 1;

            framebuffers.push_back(getDevice().createFramebufferUnique(info));
        }
    }

    TexturePart::TexturePart() {
        data::Texture& texture = getRenderer().getTexture();
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.getDimentions().x, texture.getDimentions().y)))) + 1;


        vk::DeviceSize imageSize = texture.getSize();

        auto [stagingBuffer, stagingBufferMemory] = makeBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void* data = getDevice().mapMemory(*stagingBufferMemory, 0, imageSize);
        memcpy(data, texture.getPixels(), static_cast<size_t>(imageSize));
        getDevice().unmapMemory(*stagingBufferMemory);

        std::tie(image, memory) = makeImage(texture.getDimentions(), mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

        transitionImageLayout(*image, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(*stagingBuffer, *image, texture.getDimentions());

        generateMipmaps(*image, vk::Format::eR8G8B8A8Srgb, texture.getDimentions(), mipLevels);

        textureImageView = makeImageView(*image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels);

        vk::SamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.magFilter = vk::Filter::eLinear;
        samplerCreateInfo.minFilter = vk::Filter::eLinear;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerCreateInfo.anisotropyEnable = true;
        samplerCreateInfo.maxAnisotropy = 16.0f;
        samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
        samplerCreateInfo.unnormalizedCoordinates = false;
        samplerCreateInfo.compareEnable = false;
        samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
        samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = static_cast<float>(mipLevels);
        samplerCreateInfo.mipLodBias = 0.0f;

        sampler = getDevice().createSamplerUnique(samplerCreateInfo);
    }

    auto TexturePart::getTextureImageView() -> const vk::ImageView& {
        return *textureImageView;
    }

    auto TexturePart::getTextureSampler() -> const vk::Sampler& {
        return *sampler;
    }

    ModelDataPart::ModelDataPart() {
        setVertexData(data::Model("models/room.obj"));
    }

    auto ModelDataPart::updateVertices() -> void {
        copyBuffer(*vertexStagingBuffer, *vertexBuffer, vertexStagingBufferSpan.size_bytes());
    }

    ModelDataPart::~ModelDataPart() { 
        getDevice().unmapMemory(*vertexStagingBufferMemory);
    }

    auto ModelDataPart::setVertexData(const data::Model& data) -> void {
        {
            model = data;

            vk::DeviceSize bufferSize = model.vertices.size() * sizeof(model.vertices[0]);

            std::tie(vertexStagingBuffer, vertexStagingBufferMemory) = makeBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void* data = getDevice().mapMemory(*vertexStagingBufferMemory, 0, bufferSize);
            memcpy(data, model.vertices.data(), static_cast<size_t>(bufferSize));
            vertexStagingBufferSpan = std::span<data::Vertex>(static_cast<data::Vertex*>(data), model.vertices.size());

            std::tie(vertexBuffer, vertexBufferMemory) = makeBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

            updateVertices();
        }
        {
            vk::DeviceSize bufferSize = model.indices.size() * sizeof(model.indices[0]);
            auto [stagingBuffer, stagingBufferMemory] = makeBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            void* data = getDevice().mapMemory(*stagingBufferMemory, 0, bufferSize);
            memcpy(data, model.indices.data(), static_cast<size_t>(bufferSize));
            getDevice().unmapMemory(*stagingBufferMemory);

            std::tie(indexBuffer, indexBufferMemory) = makeBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

            copyBuffer(*stagingBuffer, *indexBuffer, bufferSize);
        }
    }

    auto ModelDataPart::getVertexBuffer() -> const vk::Buffer& {
        return *vertexBuffer;
    }

    auto ModelDataPart::getIndexCount() -> size_t {
        return model.indices.size();
    }

    auto ModelDataPart::getIndexBuffer() -> const vk::Buffer& {
        return *indexBuffer;
    }

    UniformBuffersPart::UniformBuffersPart() {
        buildUniformBuffers();
    }

    auto UniformBuffersPart::buildUniformBuffers() -> void {
        uniformBuffers.clear();
        uniformBuffersMemory.clear();

        for (size_t i = 0; i < getSwapchainImageCount(); ++i) {
            auto [buffer, memory] = makeBuffer(sizeof(data::UBO), vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            uniformBuffers.push_back(std::move(buffer));
            uniformBuffersMemory.push_back(std::move(memory));
        }
    }

    auto UniformBuffersPart::getUniformBuffers() -> std::vector<vk::Buffer> {
        return vk::uniqueToRaw(uniformBuffers);
    }

    auto UniformBuffersPart::getUniformBuffersMemory() -> std::vector<vk::DeviceMemory> {
        return vk::uniqueToRaw(uniformBuffersMemory);
    }

    DescriptorPoolPart::DescriptorPoolPart() {
        buildDescriptorPool();
    }

    auto DescriptorPoolPart::getDescriptorPool() -> const vk::DescriptorPool& {
        return *descriptorPool;
    }

    auto DescriptorPoolPart::buildDescriptorPool() -> void {
        std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes;
        descriptorPoolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        descriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(getSwapchainImageCount());
        descriptorPoolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
        descriptorPoolSizes[1].descriptorCount = static_cast<uint32_t>(getSwapchainImageCount());

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(getSwapchainImageCount());

        descriptorPool = getDevice().createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    DescriptorSetsPart::DescriptorSetsPart() {
        buildDescriptorSets();
    }

    auto DescriptorSetsPart::buildDescriptorSets() -> void {
        std::vector<vk::DescriptorSetLayout> layouts(getSwapchainImageCount(), getDescriptorSetLayout());

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
        descriptorSetAllocateInfo.descriptorPool = getDescriptorPool();
        descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(getSwapchainImageCount());
        descriptorSetAllocateInfo.pSetLayouts = layouts.data();

        descriptorSets = getDevice().allocateDescriptorSets(descriptorSetAllocateInfo);

        std::vector<vk::Buffer> uniformBuffers = getUniformBuffers();

        for (size_t i = 0; i < getSwapchainImageCount(); ++i) {
            vk::DescriptorBufferInfo descriptorBufferInfo;
            descriptorBufferInfo.buffer = uniformBuffers[i];
            descriptorBufferInfo.offset = 0;
            descriptorBufferInfo.range = sizeof(data::UBO);

            vk::DescriptorImageInfo imageInfo;
            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo.imageView = getTextureImageView();
            imageInfo.sampler = getTextureSampler();

            std::array<vk::WriteDescriptorSet, 2> descriptorWrites;

            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &descriptorBufferInfo;

            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            getDevice().updateDescriptorSets(descriptorWrites, {});
        }
    }

    auto DescriptorSetsPart::getDescriptorSets() -> const std::vector<vk::DescriptorSet>& {
        return descriptorSets;
    }

    CommandBufferPart::CommandBufferPart() {
        buildCommandBuffers(getCurrentExtent());
    }

    auto CommandBufferPart::getCommandBuffers() -> std::vector<vk::CommandBuffer> {
        return vk::uniqueToRaw(commandBuffers);
    }

    auto CommandBufferPart::buildCommandBuffers(vk::Extent2D extent) -> void {
        commandBuffers.clear();

        std::vector<vk::Framebuffer> framebuffers = getFramebuffers();
        
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.commandPool = getCommandPool();
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandBufferCount = (uint32_t)(framebuffers.size());

        commandBuffers = getDevice().allocateCommandBuffersUnique(allocateInfo);

        size_t index = 0;
        for (const vk::UniqueCommandBuffer& commandBuffer : commandBuffers) {
            vk::CommandBufferBeginInfo beginInfo;
            commandBuffer->begin(beginInfo);

            vk::RenderPassBeginInfo renderPassInfo;
            renderPassInfo.renderPass = getRenderPass();
            renderPassInfo.framebuffer = framebuffers[index];
            renderPassInfo.renderArea.offset.x = 0;
            renderPassInfo.renderArea.offset.y = 0;
            renderPassInfo.renderArea.extent = extent;
            std::array<vk::ClearValue, 2> clearValues;
            clearValues[0].color.setFloat32({ 0.01f, 0.01f, 0.01f, 1.0f });
            clearValues[1].depthStencil.setDepth(1.0f).setStencil(0);

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            {
                commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, getGraphicsPipeline());

                std::array vertexBuffers = { getVertexBuffer() };
                std::array offsets = { (vk::DeviceSize)(0) };
                commandBuffer->bindVertexBuffers(0, vertexBuffers, offsets);

                commandBuffer->bindIndexBuffer(getIndexBuffer(), 0, vk::IndexType::eUint32);

                commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, getGraphicsPipelineLayout(), 0, 1, &getDescriptorSets()[index], 0, nullptr);

                commandBuffer->drawIndexed(static_cast<uint32_t>(getIndexCount()), 1, 0, 0, 0);
            }
            commandBuffer->endRenderPass();

            commandBuffer->end();

            ++index;
        }
    }

    LoopPart::LoopPart() {
        imagesInFlight.resize(getSwapchainImageCount());

        vk::SemaphoreCreateInfo semaphoreInfo;
        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (uint32_t i = 0; i < maxFramesInFlight; ++i) {
            imageAvailableSemaphores.push_back(getDevice().createSemaphoreUnique(semaphoreInfo));
            renderFinishedSemaphores.push_back(getDevice().createSemaphoreUnique(semaphoreInfo));
            fencesInFlight.push_back(getDevice().createFenceUnique(fenceInfo));
        }

        setRefreshCallback([&]() {
            update();
        });

        setFramebufferResizeCallback([&]() {
            rebuildIsNeeded = true;
        });
    }

    auto LoopPart::update() -> void {
        vk::Extent2D extent = getCurrentExtent();
        if (extent.width == 0 || extent.height == 0) {
            return;
        }
        getDevice().waitForFences(*fencesInFlight[static_cast<size_t>(currentFrame)], true, std::numeric_limits<uint64_t>::max());
        
        if (rebuildIsNeeded) {
            double now = glfw::glfwGetTime();
            if (now - lastRebuild > 1.0) {
                rebuildForResize();
                lastRebuild = now;
            }
        }

        uint32_t imageIndex = 0;
        try {
            auto [result, index] = getDevice().acquireNextImageKHR(getSwapchain(), std::numeric_limits<uint64_t>::max(), *imageAvailableSemaphores[static_cast<size_t>(currentFrame)], {});
            if (result != vk::Result::eSuccess) {
                rebuildForResize();
                return;
            }
            imageIndex = index;
        }
        catch (...) {
            rebuildForResize();
            return;
        }

        {
            static auto last = static_cast<float>(glfw::glfwGetTime());
            float now = static_cast<float>(glfw::glfwGetTime());
            getRenderer().onUpdate(now - last, now);
            last = now;

            const data::Camera& camera = getRenderer().getCamera();
            data::UBO ubo;

            glm::mat4 rotation = glm::eulerAngleXZ(camera.pitch, camera.yaw);

            ubo.view = rotation * glm::translate(glm::mat4(1.0f), camera.position * glm::vec3(1.0f, -1.0f, 1.0f));
            ubo.projection = glm::perspective(camera.fov, static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.01f, 1000.0f);

            void* data = getDevice().mapMemory(getUniformBuffersMemory()[static_cast<size_t>(imageIndex)], 0, sizeof(data::UBO));
            memcpy(data, &ubo, sizeof(data::UBO));
            getDevice().unmapMemory(getUniformBuffersMemory()[static_cast<size_t>(imageIndex)]);
        }

        if (imagesInFlight[static_cast<size_t>(imageIndex)] != VK_NULL_HANDLE) {
            getDevice().waitForFences(imagesInFlight[static_cast<size_t>(imageIndex)], true, std::numeric_limits<uint64_t>::max());
        }

        imagesInFlight[static_cast<size_t>(imageIndex)] = *fencesInFlight[static_cast<size_t>(currentFrame)];

        vk::SubmitInfo submitInfo;

        std::array waitSemaphores = { *imageAvailableSemaphores[static_cast<size_t>(currentFrame)] };
        std::array waitStages = { vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) };

        submitInfo.waitSemaphoreCount = (uint32_t)(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        
        vk::CommandBuffer commandBuffer = getCommandBuffers()[static_cast<size_t>(imageIndex)];

        submitInfo.pCommandBuffers = &commandBuffer;

        std::array signalSemaphores = { *renderFinishedSemaphores[static_cast<size_t>(currentFrame)] };
        submitInfo.signalSemaphoreCount = (uint32_t)(signalSemaphores.size());

        submitInfo.pSignalSemaphores = signalSemaphores.data();

        getDevice().resetFences(*fencesInFlight[static_cast<size_t>(currentFrame)]);

        getGraphicsQueue().submit(submitInfo, *fencesInFlight[static_cast<size_t>(currentFrame)]);

        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = (uint32_t)(signalSemaphores.size());
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        std::array swapchains = { getSwapchain() };
        presentInfo.swapchainCount = (uint32_t)(swapchains.size());
        presentInfo.pSwapchains = swapchains.data();

        presentInfo.pImageIndices = &imageIndex;

        try {
            if (getPresentQueue().presentKHR(presentInfo) != vk::Result::eSuccess) {
                rebuildForResize();
                return;
            }
        }
        catch (...) {
            rebuildForResize();
            return;
        }

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    auto LoopPart::rebuildForResize() -> void {
        getDevice().waitIdle();
        vk::Extent2D extent = getCurrentExtent();
        if (extent.width == 0 || extent.height == 0) {
            return;
        }
        buildSwapchain(extent);
        buildImages(extent);
        buildGraphicsPipeline(extent);
        buildColorResources(extent);
        buildDepthBuffer(extent);
        buildFramebuffers(extent);
        buildUniformBuffers();
        buildDescriptorPool();
        buildDescriptorSets();
        buildCommandBuffers(extent);
        rebuildIsNeeded = false;
    }

    auto LastPart::start() -> void {
        setVisible(true);
        while (!getClosed()) {
            poll();
            update();
        }
        getDevice().waitIdle();
    }
}

namespace vkr::test {
    Application::Application() {}

    auto Application::getDebuggerMinimumLevel() -> api::DebuggerMinimunLevel {
        return api::DebuggerMinimunLevel::eWarning;
    }

    auto Application::getTexture() -> data::Texture& {
        static data::Texture texture("textures/room.png");
        return texture;
    }

    auto Application::getCamera() -> const data::Camera& {
        return camera;
    }
    
    auto Application::selectPhysicalDevice(const std::vector<vk::PhysicalDeviceProperties>& physicalDeviceProperties) -> size_t {
        fmt::print("Please select a physical device:\n");
        for (size_t i = 0; i < physicalDeviceProperties.size(); ++i) {
            fmt::print("  {}. {}\n", i + 1, physicalDeviceProperties[i].deviceName);
        }
        while (true) {
            try {
                size_t index = std::stoull(io::console::read()) - 1;
                if (index >= physicalDeviceProperties.size()) {
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

        return 0;
    }
    
    auto Application::onUpdate(float delta, float time) -> void {
        updateVertices();

        static float sum = 0.0f;
        sum += delta;
        if (sum >= 0.25f) {
            getWindow().setTitle(fmt::format("FPS: {}", static_cast<int>(1.0f / delta)));
            sum -= 0.25f;
        }

        getWindow().on<io::event::MousePress>([&](auto e, bool& handled) {
            getWindow().setCursorInputMode(io::CursorInputMode::eInfinite);
            if (e.button == io::Button::eRight) {
                setVertexData(data::Model("models/room.obj"));
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
                else if (getWindow().getCursorInputMode() == io::CursorInputMode::eInfinite) {
                    getWindow().setCursorInputMode(io::CursorInputMode::eNormal);
                }
                else {
                    getWindow().setClosed(true);
                }
            }
        });

        getWindow().on<io::event::MouseOffset>([&](auto e, bool& handled) {
            if (getWindow().getCursorInputMode() == io::CursorInputMode::eInfinite) {
                camera.yaw += e.offset.x / 150.0f;
                camera.pitch += e.offset.y / 150.0f;
                camera.pitch = std::clamp(camera.pitch, -glm::pi<float>(), 0.0f);
            }
        });

        camera.position += camera.getForward() * getWindow().getKeyboardScalar(io::Key::eW, io::Key::eS) * delta * 2.0f;
        camera.position += camera.getLeft() * getWindow().getKeyboardScalar(io::Key::eA, io::Key::eD) * delta * 2.0f;
        camera.position.z += getWindow().getKeyboardScalar(io::Key::eLeftShift, io::Key::eSpace) * delta * 2.0f;
    }
}

int main() {
    try {
        vkr::test::Application().start();

    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
        std::cin.get();
    }
}