#pragma once

#include "header.hpp"
#include "physicalDeviceHelper.hpp"

namespace mc {

    namespace details {
#ifndef NDEBUG
        static VKAPI_ATTR VkBool32 VKAPI_CALL RendererVulkanValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
            if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                std::cout << pCallbackData->pMessage << '\n';
            }

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

            std::vector<vk::Image> swapChainImages;
            std::vector<vk::ImageView> swapChainImageViews;
            std::vector<vk::Framebuffer> swapChainFrameBuffers;

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

            s_.instance = vk::createInstance(instanceCI).value;

#ifndef NDEBUG
            s_.dynamicLoader.init(s_.instance, vkGetInstanceProcAddr);

            vk::DebugUtilsMessengerCreateInfoEXT dumci{};
            dumci.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            dumci.messageType     = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral     | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation  | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            dumci.pfnUserCallback = mc::details::RendererVulkanValidationCallback;

            s_.debugUtilsMessenger = s_.instance.createDebugUtilsMessengerEXT(dumci, nullptr, s_.dynamicLoader).value;
#endif // NDEBUG
        }

        static void CreateSurface() noexcept {
#ifdef _WIN32
            vk::Win32SurfaceCreateInfoKHR win32SurfaceCIkhr{};
            win32SurfaceCIkhr.flags     = {};
            win32SurfaceCIkhr.hinstance = GetModuleHandleA(NULL);
            win32SurfaceCIkhr.hwnd      = AppSurface::GetNativeHandle();

            s_.surface = s_.instance.createWin32SurfaceKHR(win32SurfaceCIkhr).value;
#endif // _WIN32
        }

        static void PickPhysicalDevice() noexcept {
            const std::vector<vk::PhysicalDevice> physicalDevices = s_.instance.enumeratePhysicalDevices().value;

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

            s_.device = s_.physicalDevice.GetPhysicalDevice().createDevice(deviceCI).value;

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

            PickSwapChainSurfaceFormat(s_.physicalDevice.GetPhysicalDevice().getSurfaceFormatsKHR(s_.surface).value);
            PickSwapChainPresentMode(s_.physicalDevice.GetPhysicalDevice().getSurfacePresentModesKHR(s_.surface).value);
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
            sci.minImageCount = surfaceCapabilities.value.minImageCount + 1;
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

            s_.swapChain = s_.device.createSwapchainKHR(sci).value;
        }

        static void LoadShaders() noexcept {

        }

        static void CreateSwapChainImagesViewsFrameBuffers() noexcept {
            s_.swapChainImages = s_.device.getSwapchainImagesKHR(s_.swapChain).value;
            s_.swapChainImageCount = s_.swapChainImages.size();

            s_.swapChainImageViews.resize(s_.swapChainImageCount);
            s_.swapChainFrameBuffers.resize(s_.swapChainImageCount);

            for (int i = 0; i < s_.swapChainImages.size(); ++i) {
                vk::ImageViewCreateInfo ivci{};
                ivci.image    = s_.swapChainImages[i];
                ivci.viewType = vk::ImageViewType::e2D;
                ivci.format   = s_.swapChainSurfaceFormat.format;
                ivci.components.r = vk::ComponentSwizzle::eIdentity;
                ivci.components.g = vk::ComponentSwizzle::eIdentity;
                ivci.components.b = vk::ComponentSwizzle::eIdentity;
                ivci.components.a = vk::ComponentSwizzle::eIdentity;
                ivci.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
                ivci.subresourceRange.baseMipLevel   = 0;
                ivci.subresourceRange.levelCount     = 1;
                ivci.subresourceRange.baseArrayLayer = 0;
                ivci.subresourceRange.layerCount     = 1;

                s_.swapChainImageViews[i] = s_.device.createImageView(ivci).value;


                //vk::ImageView attachments[] = {
                //    s_.swapChainImageViews[i]
                //};
                //
                //vk::FramebufferCreateInfo fbci{};
                //fbci.renderPass = renderPass;
                //fbci.attachmentCount = 1;
                //fbci.pAttachments = attachments;
                //fbci.width  = s_.swapChainExtent.width;
                //fbci.height = s_.swapChainExtent.height;
                //fbci.layers = 1;
                //
                //s_.swapChainFrameBuffers[i] = s_.device.createFramebufferUnique(fbci).value;
            }
        }

    public:
        static void Startup() noexcept {
            CreateInstance();
            CreateSurface();
            PickPhysicalDevice();
            CreateLogicalDeviceAndFetchQueues();
            CreateSwapChain();
            CreateSwapChainImagesViewsFrameBuffers();
        }

        static void Shutdown() noexcept {
            for (const auto& frameBuffer : s_.swapChainFrameBuffers)
                s_.device.destroyFramebuffer(frameBuffer);
            s_.swapChainFrameBuffers.clear();

            for (const auto& imgView : s_.swapChainImageViews)
                s_.device.destroyImageView(imgView);
            s_.swapChainImageViews.clear();

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