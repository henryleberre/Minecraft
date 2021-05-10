#pragma once

#include "header.hpp"
#include "vulkanUtils.hpp"

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
