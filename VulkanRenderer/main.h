#pragma once
#include "math.h"
#include "algorithm.h"
#include "meta.h"
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
        data::Model model;
        data::Texture texture;
        std::function<size_t(std::vector<vk::PhysicalDeviceProperties>)> deviceSelector = [](std::vector<vk::PhysicalDeviceProperties>) {
            return 0;
        };
        std::function<void(float delta, float time)> onUpdate = [](float, float) {};
    };

    class Renderer;
}

namespace vkr::part {
    class BeginPart {
    public:
        BeginPart(api::RendererCreateInfo&& rendererCreateInfo);
        BeginPart(const BeginPart&) = delete;
        ~BeginPart();
        auto getCreateInfo() -> api::RendererCreateInfo&;
    private:
        api::RendererCreateInfo rendererCreateInfo;
    };

    class InstancePart : public BeginPart {
    public:
        using Base = BeginPart;
        InstancePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getInstance() -> vk::Instance;
        auto getInstanceExtentions() -> const std::vector<const char*>&;
        auto getInstanceLayers() -> const std::vector<const char*>&;
    private:
        vk::UniqueInstance instance;
        std::vector<const char*> layers;
        std::vector<const char*> extentions;
    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT typeFlags,
            const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
            void* userData
        );
        vk::UniqueDebugUtilsMessengerEXT debugger;
    };

    class SurfacePart : public InstancePart, public io::Window {
    public:
        using Base = InstancePart;
        SurfacePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getSurface() -> vk::SurfaceKHR;
    private:
        vk::UniqueSurfaceKHR surface;
    };

    class PhysicalDevicePart : public SurfacePart {
    public:
        using Base = SurfacePart;
        PhysicalDevicePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getPhysicalDevice() -> const vk::PhysicalDevice&;
        auto getPhysicalDeviceExtentions() -> const std::vector<const char*>&;
        auto getQueueFamilyIndices() -> const std::array<uint32_t, 2>&;
        auto getGraphicsQueueFamilyIndex() -> uint32_t;
        auto getPresentQueueFamilyIndex() -> uint32_t;
    private:
        vk::PhysicalDevice device;
        std::vector<const char*> extentions;
        std::array<uint32_t, 2> queueFamilyIndices = {};
    };

    class PhysicalDeviceDataPart : public PhysicalDevicePart {
    public:
        using Base = PhysicalDevicePart;
        PhysicalDeviceDataPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getSurfaceFormats() -> const std::vector<vk::SurfaceFormatKHR>&;
        auto getSurfacePresentModes() -> const std::vector<vk::PresentModeKHR>&;
        auto getDepthFormat() -> vk::Format;
        auto getMsaaSamples() -> vk::SampleCountFlagBits;
        auto getCurrentExtent() -> vk::Extent2D;
        auto getMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) -> uint32_t;
    private:
        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> surfacePresentModes;
        vk::Format depthFormat;
        vk::SampleCountFlagBits msaaSamples;
    };

    class DevicePart : public PhysicalDeviceDataPart {
    public:
        using Base = PhysicalDeviceDataPart;
        DevicePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getDevice() -> vk::Device;
        auto getGraphicsQueue() -> vk::Queue;
        auto getPresentQueue() -> vk::Queue;
        auto makeBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) -> std::tuple<vk::UniqueBuffer, vk::UniqueDeviceMemory>;
        auto makeImage(glm::uvec2 size, uint32_t mipLevels, vk::SampleCountFlagBits sampleCount, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties) -> std::tuple<vk::UniqueImage, vk::UniqueDeviceMemory>;
        auto makeImageView(vk::Image image, vk::Format format, vk::ImageAspectFlagBits aspectFlags, uint32_t mipLevels) -> vk::UniqueImageView;
    private:
        vk::UniqueDevice device;
    };

    class SwapchainPart : public DevicePart {
    public:
        using Base = DevicePart;
        SwapchainPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getSwapchain() -> vk::SwapchainKHR;
        auto getSwapchainFormat() -> vk::Format;
    public:
        auto buildSwapchain(vk::Extent2D extent) -> void;
    private:
        vk::UniqueSwapchainKHR swapchain;
        vk::Format format;
    };

    class ImagesPart : public SwapchainPart {
    public:
        using Base = SwapchainPart;
        ImagesPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getSwapchainImageViews() -> std::vector<vk::ImageView>;
        auto getSwapchainImageCount() -> size_t;
    public:
        auto buildImages(vk::Extent2D extent) -> void;
    private:
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> uniqueImageViews;
    };

    class RenderPassPart : public ImagesPart {
    public:
        using Base = ImagesPart;
        RenderPassPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getRenderPass() -> vk::RenderPass;
    private:
        vk::UniqueRenderPass renderPass;
    };

    class DescriptorSetLayoutPart : public RenderPassPart {
    public:
        using Base = RenderPassPart;
        DescriptorSetLayoutPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getDescriptorSetLayout() -> vk::DescriptorSetLayout;
    private:
        vk::UniqueDescriptorSetLayout descriptorSetLayout;
    };

    class GraphicsPipelinePart : public DescriptorSetLayoutPart {
    public:
        using Base = DescriptorSetLayoutPart;
        GraphicsPipelinePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getGraphicsPipeline() -> vk::Pipeline;
        auto getGraphicsPipelineLayout() -> vk::PipelineLayout;
    public:
        auto buildGraphicsPipeline(vk::Extent2D extent) -> void;
    private:
        vk::UniquePipelineLayout layout;
        vk::UniquePipeline pipeline;
    };

    class CommandPoolPart : public GraphicsPipelinePart {
    public:
        using Base = GraphicsPipelinePart;
        CommandPoolPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getCommandPool() -> vk::CommandPool;
        auto copyBuffer(vk::Buffer from, vk::Buffer to, vk::DeviceSize size) -> void;
        auto transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) -> void;
        auto copyBufferToImage(vk::Buffer buffer, vk::Image image, glm::uvec2 size) -> void;
        template <class Callback>
        auto executeSingleTimeCommands(Callback callback) -> void {
            vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
            commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
            commandBufferAllocateInfo.commandPool = *commandPool;
            commandBufferAllocateInfo.commandBufferCount = 1;

            vk::UniqueCommandBuffer commandBuffer = std::move(getDevice().allocateCommandBuffersUnique(commandBufferAllocateInfo)[0]);

            vk::CommandBufferBeginInfo commandBufferBeginInfo;
            commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

            commandBuffer->begin(commandBufferBeginInfo);
            callback(*commandBuffer);
            commandBuffer->end();

            vk::SubmitInfo submitInfo;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &*commandBuffer;

            getGraphicsQueue().submit(submitInfo, {});
            getGraphicsQueue().waitIdle();
        };
        auto generateMipmaps(vk::Image image, vk::Format imageFormat, glm::ivec2 dimentionsBeforeFullscreen, uint32_t mipLevels) -> void;
    private:
        vk::UniqueCommandPool commandPool;
    };

    class ColorResourcesPart : public CommandPoolPart {
    public:
        using Base = CommandPoolPart;
        ColorResourcesPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getColorImageView() -> vk::ImageView;
    public:
        auto buildColorResources(vk::Extent2D extent) -> void;
    private:
        vk::UniqueImage colorImage;
        vk::UniqueDeviceMemory colorImageMemory;
        vk::UniqueImageView colorImageView;
    };

    class DepthPart : public ColorResourcesPart {
    public:
        using Base = ColorResourcesPart;
        DepthPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getDepthImageView() -> vk::ImageView;
    public:
        auto buildDepthBuffer(vk::Extent2D extent) -> void;
    private:
        vk::UniqueImage depthImage;
        vk::UniqueDeviceMemory depthImageMemory;
        vk::UniqueImageView depthImageView;
    };

    class FramebufferPart : public DepthPart {
    public:
        using Base = DepthPart;
        FramebufferPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getFramebuffers() -> std::vector<vk::Framebuffer>;
    public:
        auto buildFramebuffers(vk::Extent2D extent) -> void;
    private:
        std::vector<vk::UniqueFramebuffer> framebuffers;
    };

    class TexturePart : public FramebufferPart {
    public:
        using Base = FramebufferPart;
        TexturePart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getTextureImageView() -> const vk::ImageView&;
        auto getTextureSampler() -> const vk::Sampler&;
    private:
        uint32_t mipLevels;
        vk::UniqueImage image;
        vk::UniqueDeviceMemory memory;
        vk::UniqueImageView textureImageView;
        vk::UniqueSampler sampler;
    };

    class ModelDataPart : public TexturePart {
    public:
        using Base = TexturePart;
        ModelDataPart(api::RendererCreateInfo&& rendererCreateInfo);
        ~ModelDataPart();
        auto setVertexData(const data::Model& data) -> void;
        auto getVertexBuffer() -> const vk::Buffer&;
        auto getIndexCount() -> size_t;
        auto getIndexBuffer() -> const vk::Buffer&;
    private:
        data::Model model;
        std::span<data::Vertex> vertexStagingBufferSpan;
        vk::UniqueBuffer vertexStagingBuffer;
        vk::UniqueDeviceMemory vertexStagingBufferMemory;
        vk::UniqueBuffer vertexBuffer;
        vk::UniqueDeviceMemory vertexBufferMemory;
        vk::UniqueBuffer indexBuffer;
        vk::UniqueDeviceMemory indexBufferMemory;
    };

    class UniformBuffersPart : public ModelDataPart {
    public:
        using Base = ModelDataPart;
        UniformBuffersPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getUniformBuffers() -> std::vector<vk::Buffer>;
        auto getUniformBuffersMemory() -> std::vector<vk::DeviceMemory>;
    public:
        auto buildUniformBuffers() -> void;
    private:
        std::vector<vk::UniqueBuffer> uniformBuffers;
        std::vector<vk::UniqueDeviceMemory> uniformBuffersMemory;
    };

    class DescriptorPoolPart : public UniformBuffersPart {
    public:
        using Base = UniformBuffersPart;
        DescriptorPoolPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getDescriptorPool() -> const vk::DescriptorPool&;
    public:
        auto buildDescriptorPool() -> void;
    private:
        vk::UniqueDescriptorPool descriptorPool;
    };

    class DescriptorSetsPart : public DescriptorPoolPart {
    public:
        using Base = DescriptorPoolPart;
        DescriptorSetsPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getDescriptorSets() -> const std::vector<vk::DescriptorSet>&;
    public:
        auto buildDescriptorSets() -> void;
    private:
        std::vector<vk::DescriptorSet> descriptorSets;
    };

    class CommandBufferPart : public DescriptorSetsPart {
    public:
        using Base = DescriptorSetsPart;
        CommandBufferPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto getCommandBuffers() -> std::vector<vk::CommandBuffer>;
    public:
        auto buildCommandBuffers(vk::Extent2D extent) -> void;
    private:
        std::vector<vk::UniqueCommandBuffer> commandBuffers;
    };

    class LoopPart : public CommandBufferPart {
    public:
        using Base = CommandBufferPart;
        LoopPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto update() -> void;
        auto rebuildForResize() -> void;
    public:
        auto getCamera() -> data::Camera&;
    private:
        bool rebuildIsNeeded = false;
        double lastRebuild = 0.0;
        uint32_t maxFramesInFlight = 2;
        uint32_t currentFrame = 0;
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::UniqueFence> fencesInFlight;
        std::vector<vk::Fence> imagesInFlight;
        data::Camera camera;
    };

    class LastPart : public LoopPart {
    public:
        using Base = LoopPart;
        LastPart(api::RendererCreateInfo&& rendererCreateInfo);
        auto runLoop() -> void;
    };
}

namespace vkr::api {
    class Renderer : private part::LastPart {
    public:
        Renderer(api::RendererCreateInfo&& rendererCreateInfo);
        auto getCamera() -> data::Camera&;
        auto getWindow() -> io::Window&;
        auto runLoop() -> void;
    };
}

namespace vkr::test {
    class Application : public api::Renderer {
    public:
        Application();
    private:
        auto rendererCreateInfo() -> api::RendererCreateInfo;
        static auto selectDevice(std::vector<vk::PhysicalDeviceProperties> deviceProperties) -> size_t;
        auto onUpdate(float delta, float time) -> void;
    };
}