/*
 * Project    : Minecraft
 * Author     : Henry LE BERRE (https://github.com/henryleberre)
 * Repository : https://github.com/henryleberre/Minecraft
 */

/*
 * It is here that we shall begin. Don't fret, I'll guide you.
 */

/*
 * System Library Includes
 */

#ifdef _WIN32
#   define NOMINMAX

#   include <Windows.h>
#   include <windowsx.h>

#   pragma comment(lib, "user32.lib")
#   pragma comment(lib, "kernel32.lib")

#   undef NOMINMAX
#endif // _WIN32

/*
 * STL <3: What would we do without std::chrono::time_point<std::chrono::high_resolution_clock> ?
 * Rust will never win... Please don't Rust. What's your deal with references ?
 * C is sheer clarity.
 */

#ifdef _WIN32
#   define _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

#include <set>
#include <map>
#include <list>
#include <array>
#include <queue>
#include <mutex>
#include <cmath>
#include <future>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <bitset>
#include <cassert>
#include <cstring>
#include <numeric>
#include <codecvt>
#include <variant>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <optional>
#include <exception>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#   undef _CRT_SECURE_NO_WARNINGS
#endif // _WIN32

/*
 * Vulkan Includes.
 * Vulkan, don't worry, I'll feed you pointers to structs.
 */

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

#define VULKAN_HPP_NO_EXCEPTIONS

#include <vulkan/vulkan.hpp>

#undef VULKAN_HPP_NO_EXCEPTIONS

#ifdef _WIN32
#   pragma comment(lib, "vulkan-1.lib")

#   undef VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32

/*
 * Nice Platform Detection Macros
 */

#ifdef _WIN32
#   define DO_WINDOWS(x) x
#   define DO_WINDOWS_COMMA(x) x,
#   define DO_LINUX(x)
#   define DO_LINUX_COMMA(x)
#   define DO_X11(x)
#   define DO_X11_COMMA(x)
#endif // _WIN32

#ifdef NDEBUG
#   define DO_RELEASE(x) x
#   define DO_RELEASE_COMMA(x) x,
#   define DO_DEBUG(x)
#   define DO_DEBUG_COMMA(x)
#else
#   define DO_RELEASE(x)
#   define DO_RELEASE_COMMA(x)
#   define DO_DEBUG(x) x
#   define DO_DEBUG_COMMA(x) x,
#endif

/*
 * Better basic typenames.
 */

namespace mc {
    using f32 = float;        using f64 = double;
    using i8  = std::int8_t;  using i16 = std::int16_t;  using i32 = std::int32_t;  using i64 = std::int64_t;
    using u8  = std::uint8_t; using u16 = std::uint16_t; using u32 = std::uint32_t; using u64 = std::uint64_t;
}; // namespace mc

/*
 * Constants...
 * They made life happen, let's be courteous with them, they don't like change
 */

namespace mc {
    constexpr const char* MC_APPLICATION_NAME    = "Minecraft - Client";
    constexpr u32         MC_APPLICATION_VERSION = VK_MAKE_API_VERSION(1, 0, 0, 0);

    constexpr const char* MC_ENGINE_NAME    = "F. Weiss <3";
    constexpr u32         MC_ENGINE_VERSION = MC_APPLICATION_VERSION;

    constexpr u32 MC_VULKAN_VERSION = VK_VERSION_1_0;

    constexpr inline std::array MC_VULKAN_INSTANCE_EXTENSIONS = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        DO_LINUX_COMMA(VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
        DO_WINDOWS_COMMA(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)
        DO_DEBUG_COMMA(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
    };

    constexpr inline std::array MC_VULKAN_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    constexpr inline std::array<const char*, 0> MC_VULKAN_LAYERS = { };
#else
    constexpr inline std::array MC_VULKAN_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif
}; // namespace mc

/*
 * A very simple Timer class.
 */

namespace mc {
    class Timer {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

    public:
        inline Timer() noexcept
            : m_start(std::chrono::high_resolution_clock::now())
        { }

        inline u32 GetElapsedMS() const noexcept {
            const auto end = std::chrono::high_resolution_clock::now();

            return std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
        }
    }; // class Timer
}; // namespace mc

namespace mc {
    namespace details {
        bool IsExtensionPresent(const char* extName, const std::vector<vk::ExtensionProperties>& extPropss) noexcept {
            for (const vk::ExtensionProperties& extProps : extPropss) {
                if (!std::strcmp(extName, extProps.extensionName))
                    return true;
            }

            return false;
        }

        bool IsLayerPresent(const char* lyrName, const std::vector<vk::LayerProperties>& lyrPropss) noexcept {
            for (const vk::LayerProperties& lyrProps : lyrPropss) {
                if (!std::strcmp(lyrName, lyrProps.layerName))
                    return true;
            }

            return false;
        }
    }; // namespace details
}; // namespace mc

namespace mc {
    class AppSurface {
    private:
        struct {
#ifdef _WIN32
            HWND handle;
#endif // _WIN32
            mc::u32 width, height;
        } static s_;

    private:
#ifdef _WIN32
        static LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
            if (hwnd == mc::AppSurface::s_.handle) {
                switch (msg) {
                case WM_DESTROY:
                    mc::AppSurface::Release();
                    return 0;
                case WM_SIZE:
                    mc::AppSurface::s_.width  = GET_X_LPARAM(lParam);
                    mc::AppSurface::s_.height = GET_Y_LPARAM(lParam);
                    return 0;
                }
            }

            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
#endif // _WIN32

    public:
        static void Acquire() noexcept {
#ifdef _WIN32
            WNDCLASSA wc     = { };
            wc.lpfnWndProc   = mc::AppSurface::Win32WindowProc;
            wc.hInstance     = GetModuleHandleA(NULL);
            wc.lpszClassName = "Minecraft's Window Class";

            RegisterClassA(&wc);

            s_.handle = CreateWindowExA(0, "Minecraft's Window Class", "Minecraft - Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandleA(NULL), NULL);

            Show();
#endif // _WIN32
        }

#ifdef _WIN32
        static inline bool Exists() noexcept { return s_.handle != NULL; }
        static inline void Show()   noexcept { ShowWindow(s_.handle, SW_SHOW); }
        static inline void Hide()   noexcept { ShowWindow(s_.handle, SW_HIDE); }
        static inline void Free()   noexcept { DestroyWindow(s_.handle); s_.handle = NULL; }
#endif // _WIN32

        static inline u32 GetWidth()  noexcept { return s_.width; }
        static inline u32 GetHeight() noexcept { return s_.height; }

#ifdef _WIN32
        static inline HWND GetNativeHandle() noexcept { return s_.handle; }
#endif // _WIN32

        static void Update() noexcept {
#ifdef _WIN32
            MSG msg = { };
            while (PeekMessageA(&msg, s_.handle, 0, 0, PM_REMOVE) > 0) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
#endif // _WIN32
        }

        static void Release() noexcept {
            DestroyWindow(s_.handle);
            s_.handle = NULL;

            UnregisterClassA("Minecraft's Window Class", GetModuleHandleA(NULL));
        }
    }; // class AppSurface

#ifdef _WIN32
    decltype(AppSurface::s_) AppSurface::s_;
#endif // _WIN32

}; // namespace mc


namespace mc {

    class PhysicalDeviceHelper {
    private:
        static constexpr mc::u32 _QF_COUNT = 2u;

    private:
        vk::PhysicalDevice					   physicalDevice;
        std::vector<vk::QueueFamilyProperties> queueFamilyPropss;
        vk::PhysicalDeviceProperties		   physicalDeviceProps;

        mc::u32 score = 0;

        bool bExtensionsSupported = false;

        static constexpr mc::f32 _fQueuePriority = 1.f;

        struct QF_IndicesData {
            mc::u32 familyIndex;
            mc::u32 queueIndex;
        };

        struct QF_Data { // "QF" stands for "Queue Family"
            std::optional<QF_IndicesData> indices;
            std::optional<vk::Queue>      queue;
        };

        union {
            std::array<QF_Data, _QF_COUNT> families;

            struct {
                QF_Data graphics;
                QF_Data presentation;
            } named;
        };

    public:
        inline QF_Data GetGraphicsQFData()     const noexcept { return this->named.graphics; }
        inline QF_Data GetPresentationQFData() const noexcept { return this->named.presentation; }

    private:
        void CalculateScore() noexcept {
            switch (this->physicalDeviceProps.deviceType) {
            case vk::PhysicalDeviceType::eDiscreteGpu:
                this->score += 4; break;
            case vk::PhysicalDeviceType::eIntegratedGpu:
                this->score += 3; break;
            case vk::PhysicalDeviceType::eCpu:
                this->score += 2; break;
            case vk::PhysicalDeviceType::eOther:
            case vk::PhysicalDeviceType::eVirtualGpu:
                this->score += 1; break;
            }
        }

        void FetchFamilyIndices(const vk::SurfaceKHR& surface) noexcept {
            mc::u32 i = 0;
            for (const vk::QueueFamilyProperties& qfps : this->queueFamilyPropss) {
                if (qfps.queueCount > 0) {
                    if (qfps.queueFlags & vk::QueueFlagBits::eGraphics)
                        this->named.graphics.indices = QF_IndicesData{ i, 0 };

                    const vk::Bool32 surfaceSupport = this->physicalDevice.getSurfaceSupportKHR(i, surface);
                    if (surfaceSupport == VK_TRUE)
                        this->named.presentation.indices = QF_IndicesData{ i, 0 };

                    if (this->AreIndicesComplete())
                        break;
                }
                ++i;
            }
        }

        void CheckExtensionSupport() noexcept {
            const auto instanceLayerProperties = this->physicalDevice.enumerateDeviceExtensionProperties();

            for (const char* const requiredDeviceExt : mc::MC_VULKAN_DEVICE_EXTENSIONS) {
                if (!mc::details::IsExtensionPresent(requiredDeviceExt, instanceLayerProperties)) {
                    this->bExtensionsSupported = false;
                    break;
                }
            }

            this->bExtensionsSupported = true;
        }

        inline bool AreRequiredExtensionsSupported() const noexcept { return this->bExtensionsSupported; }

        inline bool AreIndicesComplete() const noexcept {
            for (const QF_Data& family : this->families)
                if (!family.indices.has_value())
                    return false;

            return true;
        }

    public:
        inline PhysicalDeviceHelper() noexcept {
            for (QF_Data& family : this->families)
                family = {  };
        }

        PhysicalDeviceHelper(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface) noexcept
            : physicalDevice(physicalDevice), queueFamilyPropss(physicalDevice.getQueueFamilyProperties()), physicalDeviceProps(physicalDevice.getProperties())
        {
            for (QF_Data& family : this->families)
                family = {  };

            this->CheckExtensionSupport();
            this->FetchFamilyIndices(surface);
            this->CalculateScore();
        }

        std::vector<vk::DeviceQueueCreateInfo> GenerateDeviceQueueCreateInfos() const noexcept {
            std::unordered_set<mc::u32> uniqueFamilyIndices;
            uniqueFamilyIndices.reserve(_QF_COUNT);

            for (const QF_Data& family : this->families)
                uniqueFamilyIndices.insert(family.indices.value().familyIndex);

            const mc::u32 nUniqueQueues = static_cast<mc::u32>(uniqueFamilyIndices.size());

            std::vector<vk::DeviceQueueCreateInfo> createInfos(nUniqueQueues);

            mc::u32 i = 0;
            for (const mc::u32& queueIndex : uniqueFamilyIndices) {
                vk::DeviceQueueCreateInfo& dqci = createInfos[i++];

                dqci.flags = vk::DeviceQueueCreateFlagBits{};
                dqci.queueCount = 1;
                dqci.pQueuePriorities = &PhysicalDeviceHelper::_fQueuePriority;
            }

            return createInfos;
        }

        void    FetchQueues(const vk::Device& logicalDevice) noexcept {
            for (QF_Data& family : this->families) {
                family.queue = logicalDevice.getQueue(family.indices.value().familyIndex, family.indices.value().queueIndex);
            }
        }

        inline vk::PhysicalDevice GetPhysicalDevice() const noexcept { return this->physicalDevice; }

        inline mc::u32 GetScore()   const noexcept { return this->score; }

        inline bool    IsSuitable() const noexcept {
            return this->AreIndicesComplete() && this->AreRequiredExtensionsSupported();
        }

        inline operator vk::PhysicalDevice() const noexcept { return this->physicalDevice; }

        inline vk::PhysicalDevice& operator->() noexcept { return this->physicalDevice; }
    }; // struct PhysicalDeviceWrapper

}; // namespace mc

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
            vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debugUtilsMessenger;
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

            s_.debugUtilsMessenger = s_.instance.createDebugUtilsMessengerEXTUnique(dumci, nullptr, s_.dynamicLoader);
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

        static void CreateSwapChainImagesViewsFrameBuffers() noexcept {
            s_.swapChainImages = s_.device.getSwapchainImagesKHR(s_.swapChain).value;
            s_.swapChainImageViews.resize(s_.swapChainImages.size());
            s_.swapChainFrameBuffers.resize(s_.swapChainImages.size());

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


                vk::ImageView attachments[] = {
                    s_.swapChainImageViews[i]
                };

                vk::FramebufferCreateInfo fbci{};
                fbci.renderPass = renderPass;
                fbci.attachmentCount = 1;
                fbci.pAttachments = attachments;
                fbci.width  = s_.swapChainExtent.width;
                fbci.height = s_.swapChainExtent.height;
                fbci.layers = 1;

                s_.swapChainFrameBuffers[i] = s_.device.createFramebufferUnique(fbci).value;
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
            for (const auto& imgView : s_.swapChainImageViews)
                s_.device.destroyImageView(imgView);
            s_.swapChainImageViews.clear();

            s_.device.destroySwapchainKHR(s_.swapChain);
            s_.device.destroy();
            s_.instance.destroySurfaceKHR(s_.surface);
            s_.instance.destroyDebugUtilsMessengerEXT(s_.debugUtilsMessenger);
            s_.instance.destroy();
        }
    }; // class Renderer

    decltype(Renderer::s_) Renderer::s_;

}; // namespace mc

namespace mc {

    class Minecraft {
    public:
        Minecraft(int argc, char** argv) noexcept {
            AppSurface::Acquire();
            Renderer::Startup();
        }

        void Run() noexcept {
            while (AppSurface::Exists()) {
                AppSurface::Update();
                std::this_thread::yield();
            }
        }

        ~Minecraft() {
            AppSurface::Release();
            Renderer::Shutdown();
        }
    }; // class Minecraft

}; // namespace mc

int main(int argc, char** argv) {
    mc::Minecraft minecraft(argc, argv);
    minecraft.Run();
}