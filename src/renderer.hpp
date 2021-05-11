#pragma once

#include "header.hpp"
#include "fileUtils.hpp"
#include "physicalDeviceHelper.hpp"

namespace mc {

    namespace details {
#ifndef NDEBUG
        static VKAPI_ATTR VkBool32 VKAPI_CALL RendererVulkanValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
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
            mc::PhysicalDeviceHelper physicalDevice;
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

            vk::CommandPool commandPool;

            std::vector<vk::CommandBuffer> commandBuffers;

            vk::Semaphore imageAvailableSemaphore;
            vk::Semaphore renderFinishedSemaphore;

            u32 swapChainImageCount;
        } static s_;

    private:
        static void CreateInstance() noexcept {
            vk::ApplicationInfo appInfo{};
            appInfo.apiVersion = MC_VULKAN_VERSION;
            appInfo.applicationVersion = MC_APPLICATION_VERSION;
            appInfo.engineVersion = MC_APPLICATION_VERSION;
            appInfo.pApplicationName = "Minecraft";
            appInfo.pEngineName = "F. Weiss <3";

            vk::InstanceCreateInfo instanceCI{};
            instanceCI.enabledExtensionCount   = static_cast<u32>(MC_VULKAN_INSTANCE_EXTENSIONS.size());
            instanceCI.ppEnabledExtensionNames = MC_VULKAN_INSTANCE_EXTENSIONS.data();

            instanceCI.enabledLayerCount   = 0;
            instanceCI.ppEnabledLayerNames = nullptr;

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

        static void CreateSurface() noexcept {
#ifdef _WIN32
            vk::Win32SurfaceCreateInfoKHR win32SurfaceCIkhr{};
            win32SurfaceCIkhr.flags     = {};
            win32SurfaceCIkhr.hinstance = GetModuleHandleA(NULL);
            win32SurfaceCIkhr.hwnd      = AppSurface::GetNativeHandle();

            s_.surface = s_.instance.createWin32SurfaceKHR(win32SurfaceCIkhr);
#endif // _WIN32
        }

        static void PickPhysicalDevice() noexcept {
            const std::vector<vk::PhysicalDevice> physicalDevices = s_.instance.enumeratePhysicalDevices();

            std::vector<PhysicalDeviceHelper> physicalDeviceWrappers;
            physicalDeviceWrappers.reserve(physicalDevices.size());
            for (const vk::PhysicalDevice& physicalDevice : physicalDevices) {
                PhysicalDeviceHelper physicalDeviceWrapper(physicalDevice, s_.surface);

                if (physicalDeviceWrapper.IsSuitable())
                    physicalDeviceWrappers.push_back(physicalDeviceWrapper);
            }

            PhysicalDeviceHelper* pBest = physicalDeviceWrappers.data();
            for (PhysicalDeviceHelper* pCurrent = pBest; pCurrent < physicalDeviceWrappers.data() + physicalDeviceWrappers.size(); ++pCurrent)
                if (pCurrent->GetScore() > pBest->GetScore())
                    pBest = pCurrent;

            s_.physicalDevice = *pBest;

            std::cout << "[RENDERER] - Selected \"" << pBest->GetPhysicalDevice().getProperties().deviceName << "\" for rendering \n";
        }

        static void CreateLogicalDeviceAndFetchQueues() noexcept {
            const std::vector<vk::DeviceQueueCreateInfo> dqcis = s_.physicalDevice.GenerateDeviceQueueCreateInfos();

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

            s_.device = s_.physicalDevice.GetPhysicalDevice().createDevice(deviceCI);

#ifndef NDEBUG
            s_.dynamicLoader.init(s_.device);
#endif

            s_.physicalDevice.FetchQueues(s_.device);
        }

        static void PickSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) noexcept {
            for (const auto& format : formats) {
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    s_.swapChainSurfaceFormat = format;
                    return;
                }
            }

            s_.swapChainSurfaceFormat = formats[0];
        }

        static void PickSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& modes) noexcept {
            for (const auto& mode : modes) {
                if (mode == vk::PresentModeKHR::eMailbox) {
                    s_.swapChainPresentMode = mode;
                    return;
                }
            }

            s_.swapChainPresentMode = vk::PresentModeKHR::eFifo;
        }

        static void PickSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) noexcept {
            if (capabilities.currentExtent.width == UINT32_MAX) {
                s_.swapChainExtent = capabilities.currentExtent;
                return;
            }

            s_.swapChainExtent.width  = std::clamp(AppSurface::GetWidth(),  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
            s_.swapChainExtent.height = std::clamp(AppSurface::GetHeight(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        static void CreateSwapChain() noexcept {
            const auto surfaceCapabilities = s_.physicalDevice.GetPhysicalDevice().getSurfaceCapabilitiesKHR(s_.surface);

            PickSwapChainSurfaceFormat(s_.physicalDevice.GetPhysicalDevice().getSurfaceFormatsKHR(s_.surface));
            PickSwapChainPresentMode(s_.physicalDevice.GetPhysicalDevice().getSurfacePresentModesKHR(s_.surface));
            PickSwapChainExtent(surfaceCapabilities);

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

            mc::u32 pQueueFamilyIndices[2] = {
                s_.physicalDevice.GetGraphicsQFData().indices.value().familyIndex,
                s_.physicalDevice.GetPresentationQFData().indices.value().familyIndex
            };

            if (s_.physicalDevice.GetGraphicsQFData().indices.value().familyIndex == s_.physicalDevice.GetPresentationQFData().indices.value().familyIndex) {
                sci.imageSharingMode      = vk::SharingMode::eConcurrent;
                sci.pQueueFamilyIndices   = pQueueFamilyIndices;
                sci.queueFamilyIndexCount = 2;
            } else {
                sci.imageSharingMode      = vk::SharingMode::eExclusive;
                sci.pQueueFamilyIndices   = nullptr;
                sci.queueFamilyIndexCount = 0;
            }

            s_.swapChain = s_.device.createSwapchainKHR(sci);
        }

        static void CreateRenderPass() noexcept {
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

        static void CreateSwapChainImagesViewsFrameBuffers() noexcept {
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

        static void CreateGraphicsPipeline() noexcept {
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

            vk::PipelineVertexInputStateCreateInfo pvisci{};
            pvisci.vertexBindingDescriptionCount = 0;
            pvisci.pVertexBindingDescriptions = nullptr; // Optional
            pvisci.vertexAttributeDescriptionCount = 0;
            pvisci.pVertexAttributeDescriptions = nullptr; // Optional

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

        static void CreateCommandPool() noexcept {
            vk::CommandPoolCreateInfo cpci{};
            cpci.queueFamilyIndex = s_.physicalDevice.GetGraphicsQFData().indices.value().queueIndex;// .graphicsFamily.value();
            cpci.flags = {}; // Optional

            s_.commandPool = s_.device.createCommandPool(cpci);
        }

        static void CreateCommandBuffers() noexcept {
            vk::CommandBufferAllocateInfo cbai{};
            cbai.commandPool = s_.commandPool;
            cbai.level = vk::CommandBufferLevel::ePrimary;// VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cbai.commandBufferCount = s_.swapChainImageCount;

            s_.commandBuffers = s_.device.allocateCommandBuffers(cbai);
        }

        static void CreateSemaphores() noexcept {
            vk::SemaphoreCreateInfo sci{};

            s_.imageAvailableSemaphore = s_.device.createSemaphore(sci);
            s_.renderFinishedSemaphore = s_.device.createSemaphore(sci);
        }

    public:
        static void Startup() noexcept {
            CreateInstance();
            CreateSurface();
            PickPhysicalDevice();
            CreateLogicalDeviceAndFetchQueues();
            CreateSwapChain();
            CreateRenderPass();
            CreateSwapChainImagesViewsFrameBuffers();
            CreateGraphicsPipeline();
            CreateCommandPool();
            CreateCommandBuffers();
            CreateSemaphores();
        }

        static void Render() noexcept {
            uint32_t imageIndex = s_.device.acquireNextImageKHR(s_.swapChain, UINT64_MAX, s_.imageAvailableSemaphore);

            vk::RenderPassBeginInfo renderPassInfo{};
            renderPassInfo.renderPass  = s_.renderPass;
            renderPassInfo.framebuffer = s_.swapChainFrameBuffers[imageIndex];// swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
            renderPassInfo.renderArea.extent = s_.swapChainExtent;

            vk::ClearValue clearColor;
            clearColor.color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });

            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues    = &clearColor;

            vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = {}; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            s_.commandBuffers[imageIndex].begin(beginInfo);
            s_.commandBuffers[imageIndex].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            s_.commandBuffers[imageIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, s_.pipeline);
            s_.commandBuffers[imageIndex].draw(3, 1, 0, 0);
            s_.commandBuffers[imageIndex].endRenderPass();
            s_.commandBuffers[imageIndex].end();

            vk::SubmitInfo submitInfo{};

            std::array waitSemaphores = { s_.imageAvailableSemaphore };
            std::array waitStages = { (vk::PipelineStageFlags)vk::PipelineStageFlagBits::eColorAttachmentOutput };
            std::array signalSemaphores = { s_.renderFinishedSemaphore };

            submitInfo.waitSemaphoreCount   = static_cast<u32>(waitSemaphores.size());
            submitInfo.pWaitSemaphores      = waitSemaphores.data();
            submitInfo.pWaitDstStageMask    = waitStages.data();
            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = &s_.commandBuffers[imageIndex];
            submitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            submitInfo.pSignalSemaphores    = signalSemaphores.data();

            s_.physicalDevice.GetPresentationQFData().queue.value().submit(submitInfo);

            vk::PresentInfoKHR presentInfo{};
            presentInfo.waitSemaphoreCount = static_cast<u32>(signalSemaphores.size());
            presentInfo.pWaitSemaphores    = signalSemaphores.data();

            std::array swapChains = { s_.swapChain };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains    = swapChains.data();
            presentInfo.pImageIndices  = &imageIndex;
            presentInfo.pResults = nullptr; // Optional

            s_.physicalDevice.GetPresentationQFData().queue.value().presentKHR(presentInfo);
            s_.physicalDevice.GetPresentationQFData().queue.value().waitIdle();
        }

        static void Shutdown() noexcept {
            s_.device.destroySemaphore(s_.imageAvailableSemaphore);
            s_.device.destroySemaphore(s_.renderFinishedSemaphore);

            s_.device.destroyCommandPool(s_.commandPool);

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