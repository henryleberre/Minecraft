#pragma once

#include "header.hpp"
#include "vertex.hpp"
#include "fileUtils.hpp"
#include "vertexBuffer.hpp"
#include "physicalDeviceSupport.hpp"

namespace mc {

    namespace details {
#ifndef NDEBUG
        static VKAPI_ATTR VkBool32 VKAPI_CALL RendererVulkanValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
            if ((vk::DebugUtilsMessageSeverityFlagsEXT)messageSeverity > vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
                std::cout << pCallbackData->pMessage << '\n';

            return VK_FALSE;
        }
#endif
    }; // details

    class Renderer {
    private:
        struct {
            vk::Instance instance;

#ifndef NDEBUG
            vk::DispatchLoaderDynamic dynamicLoader;
            vk::DebugUtilsMessengerEXT debugUtilsMessenger;
#endif

            vk::SurfaceKHR surface;

            vk::PhysicalDevice physical;
            mc::vk_utils::PhysicalDeviceSupport physicalSupport;

            vk::Device device;

            vk::SwapchainKHR swapChain;

            vk::SurfaceFormatKHR swapChainSurfaceFormat;
            vk::PresentModeKHR swapChainPresentMode;
            vk::Extent2D swapChainExtent;

            vk::RenderPass renderPass;

            std::vector<vk::Image> swapChainImages;
            std::vector<vk::ImageView> swapChainImageViews;
            std::vector<vk::Framebuffer> swapChainFrameBuffers;

            vk::PipelineLayout pipelineLayout;
            vk::Pipeline pipeline;

            mc::VertexBuffer vertexBuffer;

            vk::CommandPool commandPool;

            std::vector<vk::CommandBuffer> commandBuffers;

            vk::Semaphore imageAvailableSemaphore;
            vk::Semaphore renderFinishedSemaphore;

            u32 swapChainImageCount;
        } static s_;

    private:
        static void CreateInstance() {
            vk::ApplicationInfo appInfo{};
            appInfo.apiVersion = MC_VULKAN_VERSION;
            appInfo.applicationVersion = MC_APPLICATION_VERSION;
            appInfo.engineVersion = MC_APPLICATION_VERSION;
            appInfo.pApplicationName = "Minecraft";
            appInfo.pEngineName = "F. Weiss <3";

            vk::InstanceCreateInfo instanceCI{};
            instanceCI.enabledExtensionCount   = static_cast<u32>(MC_VULKAN_INSTANCE_EXTENSIONS.size());
            instanceCI.ppEnabledExtensionNames = MC_VULKAN_INSTANCE_EXTENSIONS.data();

            instanceCI.enabledLayerCount   = static_cast<u32>(MC_VULKAN_LAYERS.size());
            instanceCI.ppEnabledLayerNames = MC_VULKAN_LAYERS.data();

            instanceCI.flags = {};
            instanceCI.pApplicationInfo = &appInfo;

            s_.instance = vk::createInstance(instanceCI);

#ifndef NDEBUG
            s_.dynamicLoader.init(s_.instance, vkGetInstanceProcAddr);

            vk::DebugUtilsMessengerCreateInfoEXT dumci{};
            dumci.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            dumci.messageType     = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral     | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation  | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            dumci.pfnUserCallback = mc::details::RendererVulkanValidationCallback;

            s_.debugUtilsMessenger = s_.instance.createDebugUtilsMessengerEXT(dumci, nullptr, s_.dynamicLoader);
#endif // NDEBUG
        }

        static void CreateLogicalDeviceAndFetchQueues() {
            const std::vector<vk::DeviceQueueCreateInfo> dqcis = s_.physicalSupport.GenerateDeviceQueueCreateInfos();

            vk::PhysicalDeviceFeatures enabledDeviceFeatures{};

            vk::DeviceCreateInfo deviceCI{};
            deviceCI.enabledExtensionCount   = static_cast<u32>(MC_VULKAN_DEVICE_EXTENSIONS.size());
            deviceCI.ppEnabledExtensionNames = MC_VULKAN_DEVICE_EXTENSIONS.data();

            deviceCI.enabledLayerCount   = static_cast<u32>(MC_VULKAN_LAYERS.size());
            deviceCI.ppEnabledLayerNames = MC_VULKAN_LAYERS.data();

            deviceCI.queueCreateInfoCount = static_cast<u32>(dqcis.size());
            deviceCI.pQueueCreateInfos    = dqcis.data();

            deviceCI.flags = {};
            deviceCI.pEnabledFeatures = &enabledDeviceFeatures;

            s_.device = s_.physical.createDevice(deviceCI);

#ifndef NDEBUG
            s_.dynamicLoader.init(s_.device);
#endif

            s_.physicalSupport.FetchQueues(s_.device);
        }

        static void CreateSwapChain() {
            const auto surfaceCapabilities = s_.physical.getSurfaceCapabilitiesKHR(s_.surface);

            s_.swapChainSurfaceFormat = mc::vk_utils::PickSwapChainSurfaceFormat(s_.physical.getSurfaceFormatsKHR(s_.surface));
            s_.swapChainPresentMode = mc::vk_utils::PickSwapChainPresentMode(s_.physical.getSurfacePresentModesKHR(s_.surface));
            s_.swapChainExtent = mc::vk_utils::PickSwapChainExtent(surfaceCapabilities);

            vk::SwapchainCreateInfoKHR sci{};
            sci.clipped = VK_TRUE;
            sci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            sci.flags = vk::SwapchainCreateFlagsKHR{};
            sci.imageArrayLayers = 1;
            sci.imageExtent = s_.swapChainExtent;
            sci.imageColorSpace = s_.swapChainSurfaceFormat.colorSpace;
            sci.imageFormat = s_.swapChainSurfaceFormat.format;
            sci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
            sci.minImageCount = surfaceCapabilities.minImageCount + 1;
            sci.oldSwapchain = vk::SwapchainKHR();
            sci.presentMode = s_.swapChainPresentMode;
            sci.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
            sci.surface = s_.surface;

            std::array pQueueFamilyIndices = {
                s_.physicalSupport.GetGraphicsQFData().indices.value().familyIndex,
                s_.physicalSupport.GetPresentationQFData().indices.value().familyIndex
            };

            if (s_.physicalSupport.GetGraphicsQFData().indices.value().familyIndex != s_.physicalSupport.GetPresentationQFData().indices.value().familyIndex) {
                sci.imageSharingMode      = vk::SharingMode::eConcurrent;
                sci.pQueueFamilyIndices   = pQueueFamilyIndices.data();
                sci.queueFamilyIndexCount = static_cast<u32>(pQueueFamilyIndices.size());
            } else {
                sci.imageSharingMode      = vk::SharingMode::eExclusive;
                sci.pQueueFamilyIndices   = nullptr;
                sci.queueFamilyIndexCount = 0;
            }

            s_.swapChain = s_.device.createSwapchainKHR(sci);
        }

        static void CreateRenderPass() {
            vk::AttachmentDescription colorAttachment{};
            colorAttachment.format = s_.swapChainSurfaceFormat.format;// swapChainImageFormat;
            colorAttachment.samples = vk::SampleCountFlagBits::e1;// VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;// VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;// VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;// VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;// VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = vk::ImageLayout::eUndefined;// VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

            vk::AttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

            vk::SubpassDescription subpass{};
            subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            vk::RenderPassCreateInfo rpci{};
            rpci.attachmentCount = 1;
            rpci.pAttachments = &colorAttachment;
            rpci.subpassCount = 1;
            rpci.pSubpasses = &subpass;

            s_.renderPass = s_.device.createRenderPass(rpci);
        }

        static void CreateSwapChainImagesViewsFrameBuffers() {
            s_.swapChainImages = s_.device.getSwapchainImagesKHR(s_.swapChain);
            s_.swapChainImageCount = s_.swapChainImages.size();

            s_.swapChainImageViews.resize(s_.swapChainImageCount);
            s_.swapChainFrameBuffers.resize(s_.swapChainImageCount);

            for (int i = 0; i < s_.swapChainImages.size(); ++i) {
                vk::ImageViewCreateInfo ivci{};
                ivci.image = s_.swapChainImages[i];
                ivci.viewType = vk::ImageViewType::e2D;
                ivci.format = s_.swapChainSurfaceFormat.format;
                ivci.components.r = vk::ComponentSwizzle::eIdentity;
                ivci.components.g = vk::ComponentSwizzle::eIdentity;
                ivci.components.b = vk::ComponentSwizzle::eIdentity;
                ivci.components.a = vk::ComponentSwizzle::eIdentity;
                ivci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                ivci.subresourceRange.baseMipLevel = 0;
                ivci.subresourceRange.levelCount = 1;
                ivci.subresourceRange.baseArrayLayer = 0;
                ivci.subresourceRange.layerCount = 1;

                s_.swapChainImageViews[i] = s_.device.createImageView(ivci);

                vk::ImageView attachments[] = {
                    s_.swapChainImageViews[i]
                };

                vk::FramebufferCreateInfo fbci{};
                fbci.renderPass = s_.renderPass;
                fbci.attachmentCount = 1;
                fbci.pAttachments = attachments;
                fbci.width = s_.swapChainExtent.width;
                fbci.height = s_.swapChainExtent.height;
                fbci.layers = 1;

                s_.swapChainFrameBuffers[i] = s_.device.createFramebuffer(fbci);
            }
        }

        static void CreateGraphicsPipeline() {
            auto CreateShaderModule = [](const vk::Device& device, const std::vector<char>& byteCode) -> vk::ShaderModule {
                vk::ShaderModuleCreateInfo smci{};
                smci.codeSize = byteCode.size();
                smci.pCode    = reinterpret_cast<const mc::u32*>(byteCode.data());
            
                return device.createShaderModule(smci);
            };

            const vk::ShaderModule vertShaderModule = CreateShaderModule(s_.device, mc::ReadBinaryFileToBuffer("res/shaders/vert.spv").value());
            const vk::ShaderModule fragShaderModule = CreateShaderModule(s_.device, mc::ReadBinaryFileToBuffer("res/shaders/frag.spv").value());
            
            vk::PipelineShaderStageCreateInfo vssci{};
            vssci.stage  = vk::ShaderStageFlagBits::eVertex;// VK_SHADER_STAGE_VERTEX_BIT;
            vssci.module = vertShaderModule;
            vssci.pName  = "main";

            vk::PipelineShaderStageCreateInfo fssci{};
            fssci.stage  = vk::ShaderStageFlagBits::eFragment;
            fssci.module = fragShaderModule;
            fssci.pName  = "main";

            std::array shaderStages = { vssci, fssci };

            const auto bindingDescription    = mc::Vertex::GetBindingDescription();
            const auto attributeDescriptions = mc::Vertex::GetAttributeDescriptions();

            vk::PipelineVertexInputStateCreateInfo pvisci{};
            pvisci.vertexBindingDescriptionCount   = 1;
            pvisci.pVertexBindingDescriptions      = &bindingDescription; // Optional
            pvisci.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
            pvisci.pVertexAttributeDescriptions    = attributeDescriptions.data(); // Optional

            vk::PipelineInputAssemblyStateCreateInfo piasci{};
            piasci.topology = vk::PrimitiveTopology::eTriangleList;// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            piasci.primitiveRestartEnable = VK_FALSE;

            vk::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width  = (float)s_.swapChainExtent.width;
            viewport.height = (float)s_.swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vk::Rect2D scissor{};
            scissor.offset = vk::Offset2D{ 0, 0 };
            scissor.extent = s_.swapChainExtent;

            vk::PipelineViewportStateCreateInfo pvsci{};
            pvsci.viewportCount = 1;
            pvsci.pViewports    = &viewport;
            pvsci.scissorCount  = 1;
            pvsci.pScissors     = &scissor;

            vk::PipelineRasterizationStateCreateInfo prsci{};
            prsci.depthClampEnable = VK_FALSE;
            prsci.rasterizerDiscardEnable = VK_FALSE;
            prsci.polygonMode = vk::PolygonMode::eFill;// VK_POLYGON_MODE_FILL;
            prsci.lineWidth = 1.0f;
            prsci.cullMode = vk::CullModeFlagBits::eBack;// VK_CULL_MODE_BACK_BIT;
            prsci.frontFace = vk::FrontFace::eClockwise;// VK_FRONT_FACE_CLOCKWISE;
            prsci.depthBiasEnable = VK_FALSE;
            prsci.depthBiasConstantFactor = 0.0f; // Optional
            prsci.depthBiasClamp = 0.0f; // Optional
            prsci.depthBiasSlopeFactor = 0.0f; // Optional

            vk::PipelineMultisampleStateCreateInfo pmsci{};
            pmsci.sampleShadingEnable  = VK_FALSE;
            pmsci.rasterizationSamples = vk::SampleCountFlagBits::e1;// VK_SAMPLE_COUNT_1_BIT;
            pmsci.minSampleShading = 1.0f; // Optional
            pmsci.pSampleMask = nullptr; // Optional
            pmsci.alphaToCoverageEnable = VK_FALSE; // Optional
            pmsci.alphaToOneEnable = VK_FALSE; // Optional

            vk::PipelineColorBlendAttachmentState pcbas{};
            pcbas.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
            pcbas.blendEnable = VK_FALSE;
            pcbas.srcColorBlendFactor = vk::BlendFactor::eOne; // VK_BLEND_FACTOR_ONE; // Optional
            pcbas.dstColorBlendFactor = vk::BlendFactor::eZero;// VK_BLEND_FACTOR_ZERO; // Optional
            pcbas.colorBlendOp = vk::BlendOp::eAdd;// VK_BLEND_OP_ADD; // Optional
            pcbas.srcAlphaBlendFactor = vk::BlendFactor::eOne;// VK_BLEND_FACTOR_ONE; // Optional
            pcbas.dstAlphaBlendFactor = vk::BlendFactor::eZero;// VK_BLEND_FACTOR_ZERO; // Optional
            pcbas.alphaBlendOp = vk::BlendOp::eAdd;// VK_BLEND_OP_ADD; // Optional

            vk::PipelineColorBlendStateCreateInfo pcbsci{};
            pcbsci.logicOpEnable = VK_FALSE;
            pcbsci.logicOp = vk::LogicOp::eCopy;// VK_LOGIC_OP_COPY; // Optional
            pcbsci.attachmentCount = 1;
            pcbsci.pAttachments = &pcbas;
            pcbsci.blendConstants[0] = 0.0f; // Optional
            pcbsci.blendConstants[1] = 0.0f; // Optional
            pcbsci.blendConstants[2] = 0.0f; // Optional
            pcbsci.blendConstants[3] = 0.0f; // Optional

            std::array dynamicStates = {
                vk::DynamicState::eViewport,// VK_DYNAMIC_STATE_VIEWPORT,
                vk::DynamicState::eLineWidth // VK_DYNAMIC_STATE_LINE_WIDTH
            };

            vk::PipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
            dynamicState.pDynamicStates    = dynamicStates.data();

            vk::PipelineLayoutCreateInfo plci{};
            plci.setLayoutCount = 0; // Optional
            plci.pSetLayouts = nullptr; // Optional
            plci.pushConstantRangeCount = 0; // Optional
            plci.pPushConstantRanges = nullptr; // Optional

            s_.pipelineLayout = s_.device.createPipelineLayout(plci);

            vk::GraphicsPipelineCreateInfo gpci{};
            gpci.stageCount = static_cast<u32>(shaderStages.size());
            gpci.pStages    = shaderStages.data();
            gpci.pVertexInputState = &pvisci;
            gpci.pInputAssemblyState = &piasci;
            gpci.pViewportState = &pvsci;
            gpci.pRasterizationState = &prsci;
            gpci.pMultisampleState = &pmsci;
            gpci.pDepthStencilState = nullptr; // Optional
            gpci.pColorBlendState = &pcbsci;
            gpci.pDynamicState = nullptr; // Optional
            gpci.layout = s_.pipelineLayout;
            gpci.renderPass = s_.renderPass;
            gpci.subpass = 0;
            gpci.basePipelineHandle = vk::Pipeline{}; // Optional
            gpci.basePipelineIndex  = -1; // Optional

            s_.pipeline = s_.device.createGraphicsPipeline({}, gpci).value;

            s_.device.destroyShaderModule(vertShaderModule);
            s_.device.destroyShaderModule(fragShaderModule);
        }

        static void CreateVertexBuffers() {
            std::array vertices = {
                Vertex{{0.0f, -0.5f}, {1.0f, 0.0f, 1.0f}},
                Vertex{{0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
                Vertex{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
            };

            s_.vertexBuffer = mc::VertexBuffer(s_.device, s_.physicalSupport.GetMemoryProperties(), sizeof(vertices));
            std::memcpy(s_.vertexBuffer.GetData(), vertices.data(), sizeof(vertices));
            s_.vertexBuffer.Upload();
        }

        static void CreateCommandPool() {
            vk::CommandPoolCreateInfo cpci{};
            cpci.queueFamilyIndex = s_.physicalSupport.GetGraphicsQFData().indices.value().familyIndex;// .graphicsFamily.value();
            cpci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // Optional

            s_.commandPool = s_.device.createCommandPool(cpci);
        }

        static void CreateCommandBuffers() {
            vk::CommandBufferAllocateInfo cbai{};
            cbai.commandPool = s_.commandPool;
            cbai.level = vk::CommandBufferLevel::ePrimary;// VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cbai.commandBufferCount = s_.swapChainImageCount;

            s_.commandBuffers = s_.device.allocateCommandBuffers(cbai);
        }

        static void CreateSemaphores() {
            vk::SemaphoreCreateInfo sci{};

            s_.imageAvailableSemaphore = s_.device.createSemaphore(sci);
            s_.renderFinishedSemaphore = s_.device.createSemaphore(sci);
        }

    public:
        static void Startup() {
            CreateInstance();
            
            s_.surface = mc::AppSurface::CreateVulkanSurface(s_.instance);
            
            s_.physicalSupport = mc::vk_utils::PickPhysicalDevice(s_.instance, s_.surface);
            s_.physical        = s_.physicalSupport.GetPhysical();
            std::cout << "[RENDERER] Selected " << s_.physicalSupport.GetProperties().deviceName << " for rendering\n" << std::flush;

            CreateLogicalDeviceAndFetchQueues();
            CreateSwapChain();
            CreateRenderPass();
            CreateSwapChainImagesViewsFrameBuffers();
            CreateGraphicsPipeline();
            CreateVertexBuffers();
            CreateCommandPool();
            CreateCommandBuffers();
            CreateSemaphores();
        }

        static void Render() {
            const u32 imgIdx = s_.device.acquireNextImageKHR(s_.swapChain, UINT64_MAX, s_.imageAvailableSemaphore);

            //
            //
            // Fetch Useful Handles
            //
            //

            const vk::CommandBuffer cmdBuff      = s_.commandBuffers[imgIdx];
            const vk::Queue         gfxQueue     = s_.physicalSupport.GetGraphicsQFData().queue.value();
            const vk::Queue         presentQueue = s_.physicalSupport.GetPresentationQFData().queue.value();
            const vk::Framebuffer   frameBuffer  = s_.swapChainFrameBuffers[imgIdx];

            //
            // 
            // Reccord Command Buffers
            //
            //

            vk::RenderPassBeginInfo renderPassInfo{};
            renderPassInfo.renderPass  = s_.renderPass;
            renderPassInfo.framebuffer = frameBuffer;// swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
            renderPassInfo.renderArea.extent = s_.swapChainExtent;

            vk::ClearValue clearColor;
            clearColor.color.setFloat32({ 0.3f, 0.3f, 1.0f, 1.0f });

            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues    = &clearColor;

            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = {}; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            cmdBuff.begin(beginInfo);
            cmdBuff.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            cmdBuff.bindPipeline(vk::PipelineBindPoint::eGraphics, s_.pipeline);
            s_.vertexBuffer.Bind(cmdBuff);
            cmdBuff.draw(3, 1, 0, 0);
            cmdBuff.endRenderPass();
            cmdBuff.end();

            // 
            //
            // Submit Command Buffers
            //
            //

            vk::SubmitInfo submitInfo{};

            std::array waitSemaphores = { s_.imageAvailableSemaphore };
            std::array waitStages = { (vk::PipelineStageFlags)vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::array signalSemaphores = { s_.renderFinishedSemaphore };

            submitInfo.waitSemaphoreCount   = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphores      = waitSemaphores.data();
            submitInfo.pWaitDstStageMask    = waitStages.data();
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &cmdBuff;
            submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphores    = signalSemaphores.data();

            gfxQueue.submit(submitInfo);

            //
            //
            // Present Frames 
            //
            //

            vk::PresentInfoKHR presentInfo{};
            presentInfo.waitSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            presentInfo.pWaitSemaphores    = signalSemaphores.data();
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = &s_.swapChain;
            presentInfo.pImageIndices      = &imgIdx;
            presentInfo.pResults           = nullptr; // Optional

            presentQueue.presentKHR(presentInfo);

            //TODO: Upgrade to frames in flight
            presentQueue.waitIdle();
        }

        static void Shutdown() {
            s_.device.destroySemaphore(s_.imageAvailableSemaphore);
            s_.device.destroySemaphore(s_.renderFinishedSemaphore);

            s_.device.destroyCommandPool(s_.commandPool);

            s_.vertexBuffer.~VertexBuffer();

            s_.device.destroyPipeline(s_.pipeline);
            s_.device.destroyPipelineLayout(s_.pipelineLayout);

            for (const auto& frameBuffer : s_.swapChainFrameBuffers)
                s_.device.destroyFramebuffer(frameBuffer);
            s_.swapChainFrameBuffers.clear();

            for (const auto& imgView : s_.swapChainImageViews)
                s_.device.destroyImageView(imgView);
            s_.swapChainImageViews.clear();

            s_.device.destroyRenderPass(s_.renderPass);
            s_.device.destroySwapchainKHR(s_.swapChain);
            s_.device.destroy();

            s_.instance.destroySurfaceKHR(s_.surface);
#ifndef NDEBUG
            s_.instance.destroyDebugUtilsMessengerEXT(s_.debugUtilsMessenger, nullptr, s_.dynamicLoader);
#endif
            s_.instance.destroy();
        }
    }; // class Renderer

    decltype(Renderer::s_) Renderer::s_;

}; // namespace mc