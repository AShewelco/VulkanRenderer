#pragma once
#include "math.h"
#include "algorithm.h"
#include "meta.h"
#include "io.h"
#include "data.h"

namespace vkr::part {
    class LastPart;
}

namespace vkr::api {
    enum class DebuggerMinimunLevel {
        eVerbose,
        eInfo,
        eWarning,
        eError,
        eDisabled
    };

    class Renderer {
    public:
        auto start() -> void;
    public:
        virtual auto getDebuggerMinimumLevel() -> DebuggerMinimunLevel = 0;
        virtual auto selectPhysicalDevice(const std::vector<vk::PhysicalDeviceProperties>& physicalDeviceProperties) -> size_t = 0;
        virtual auto getTexture() -> data::Texture& = 0;
        virtual auto getCamera() -> const data::Camera& = 0;
        virtual auto onUpdate(float delta, float time) -> void = 0;
    protected:
        auto getWindow() -> io::Window&;
        auto updateVertices() -> void;
        auto setVertexData(const data::Model& data) -> void;
    private:
        part::LastPart* part;
    };
}

namespace vkr::part {
    class BeginPart {
    public:
        static api::Renderer* renderer;
    public:
        BeginPart();
        BeginPart(const BeginPart&) = delete;
        ~BeginPart();
        auto getRenderer() -> api::Renderer&;
    };

    class InstancePart : public BeginPart {
    public:
        InstancePart();
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
        SurfacePart();
        auto getSurface() -> vk::SurfaceKHR;
    private:
        vk::UniqueSurfaceKHR surface;
    };

    class PhysicalDevicePart : public SurfacePart {
    public:
        PhysicalDevicePart();
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
        PhysicalDeviceDataPart();
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
        DevicePart();
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
        SwapchainPart();
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
        ImagesPart();
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
        RenderPassPart();
        auto getRenderPass() -> vk::RenderPass;
    private:
        vk::UniqueRenderPass renderPass;
    };

    class DescriptorSetLayoutPart : public RenderPassPart {
    public:
        DescriptorSetLayoutPart();
        auto getDescriptorSetLayout() -> vk::DescriptorSetLayout;
    private:
        vk::UniqueDescriptorSetLayout descriptorSetLayout;
    };

    class GraphicsPipelinePart : public DescriptorSetLayoutPart {
    public:
        GraphicsPipelinePart();
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
        CommandPoolPart();
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
        ColorResourcesPart();
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
        DepthPart();
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
        FramebufferPart();
        auto getFramebuffers() -> std::vector<vk::Framebuffer>;
    public:
        auto buildFramebuffers(vk::Extent2D extent) -> void;
    private:
        std::vector<vk::UniqueFramebuffer> framebuffers;
    };

    class TexturePart : public FramebufferPart {
    public:
        TexturePart();
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
        ModelDataPart();
        ~ModelDataPart();
        auto setVertexData(const data::Model& data) -> void;
        auto getVertexBuffer() -> const vk::Buffer&;
        auto getIndexCount() -> size_t;
        auto getIndexBuffer() -> const vk::Buffer&;
        auto updateVertices() -> void;
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
        UniformBuffersPart();
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
        DescriptorPoolPart();
        auto getDescriptorPool() -> const vk::DescriptorPool&;
    public:
        auto buildDescriptorPool() -> void;
    private:
        vk::UniqueDescriptorPool descriptorPool;
    };

    class DescriptorSetsPart : public DescriptorPoolPart {
    public:
        DescriptorSetsPart();
        auto getDescriptorSets() -> const std::vector<vk::DescriptorSet>&;
    public:
        auto buildDescriptorSets() -> void;
    private:
        std::vector<vk::DescriptorSet> descriptorSets;
    };

    class CommandBufferPart : public DescriptorSetsPart {
    public:
        CommandBufferPart();
        auto getCommandBuffers() -> std::vector<vk::CommandBuffer>;
    public:
        auto buildCommandBuffers(vk::Extent2D extent) -> void;
    private:
        std::vector<vk::UniqueCommandBuffer> commandBuffers;
    };

    class LoopPart : public CommandBufferPart {
    public:
        LoopPart();
        auto update() -> void;
        auto rebuildForResize() -> void;
    private:
        bool rebuildIsNeeded = false;
        double lastRebuild = 0.0;
        uint32_t maxFramesInFlight = 2;
        uint32_t currentFrame = 0;
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::UniqueFence> fencesInFlight;
        std::vector<vk::Fence> imagesInFlight;
    };

    class LastPart : public LoopPart {
    public:
        auto start() -> void;
    };
}

namespace vkr::test {
    class Application : public api::Renderer {
    public:
        Application();
        virtual auto getDebuggerMinimumLevel() -> api::DebuggerMinimunLevel override;
        virtual auto getTexture() -> data::Texture& override;
        virtual auto getCamera() -> const data::Camera& override;
        virtual auto onUpdate(float delta, float time) -> void override;
        virtual auto selectPhysicalDevice(const std::vector<vk::PhysicalDeviceProperties>& physicalDeviceProperties) -> size_t override;
    private:
        data::Camera camera;
        std::vector<data::Vertex> vertices;
    };
}