#pragma once

#include "header.hpp"
#include "vulkanUtils.hpp"

namespace mc {

    namespace vk_utils {

        class PhysicalDeviceSupport {
        private:
            static constexpr mc::u32 _QF_COUNT       = 2u;
            static constexpr mc::f32 _fQueuePriority = 1.f;

        private:
            struct QF_IndicesData { // "QF" stands for "Queue Family"
                mc::u32 familyIndex;
                mc::u32 queueIndex;
            };

            struct QF_Data {
                std::optional<QF_IndicesData> indices;
                std::optional<vk::Queue>      queue;
            };

        private:
            vk::PhysicalDevice					   m_physical;
            std::vector<vk::QueueFamilyProperties> m_queueFamilyPropss;
            vk::PhysicalDeviceProperties		   m_deviceProperties;
            vk::PhysicalDeviceMemoryProperties     m_memoryProperties;

            mc::u32 m_score = 0;

            bool m_bExtensionsSupported = false;
            bool m_bIndicesComplete     = false;
            
            bool m_bDeviceSuitable = false;

            union {
                std::array<QF_Data, _QF_COUNT> m_families;

                struct {
                    QF_Data graphics;
                    QF_Data presentation;
                } m_namedQF;
            };

        public:
            inline const QF_Data& GetGraphicsQFData()     const { return m_namedQF.graphics;     }
            inline const QF_Data& GetPresentationQFData() const { return m_namedQF.presentation; }

        private:
            void CalculateScore() {
                switch (m_deviceProperties.deviceType) {
                case vk::PhysicalDeviceType::eDiscreteGpu:
                    m_score += 4; break;
                case vk::PhysicalDeviceType::eIntegratedGpu:
                    m_score += 3; break;
                case vk::PhysicalDeviceType::eCpu:
                    m_score += 2; break;
                case vk::PhysicalDeviceType::eOther:
                case vk::PhysicalDeviceType::eVirtualGpu:
                    m_score += 1; break;
                }
            }

            void FetchFamilyIndices(const vk::SurfaceKHR& surface) {
                mc::u32 i = 0;
                for (const vk::QueueFamilyProperties& qfps : m_queueFamilyPropss) {
                    if (qfps.queueCount > 0) {
                        if (qfps.queueFlags & vk::QueueFlagBits::eGraphics)
                            m_namedQF.graphics.indices = QF_IndicesData{ i, 0 };

                        const vk::Bool32 surfaceSupport = m_physical.getSurfaceSupportKHR(i, surface);
                        if (surfaceSupport == VK_TRUE)
                            m_namedQF.presentation.indices = QF_IndicesData{ i, 0 };
                    }
                    ++i;
                }
            }

            void CheckExtensionSupport() {
                const auto instanceLayerProperties = m_physical.enumerateDeviceExtensionProperties();

                for (const char* const requiredDeviceExt : mc::MC_VULKAN_DEVICE_EXTENSIONS) {
                    if (!mc::vk_utils::IsExtensionPresent(requiredDeviceExt, instanceLayerProperties)) {
                        m_bExtensionsSupported = false;
                        break;
                    }
                }

                m_bExtensionsSupported = true;
            }

            inline void CheckIndicesCompletion() {
                for (const QF_Data& family : m_families) {
                    if (!family.indices.has_value()) {
                        m_bIndicesComplete = false;
                        return;
                    }
                }
                
                m_bIndicesComplete = true;
            }

        public:
            inline PhysicalDeviceSupport() {
                for (QF_Data& family : m_families)
                    family = {  };
            }

            PhysicalDeviceSupport(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
                : m_physical(physicalDevice)
            {
                m_queueFamilyPropss = physicalDevice.getQueueFamilyProperties();
                m_deviceProperties  = m_physical.getProperties();
                m_memoryProperties  = m_physical.getMemoryProperties();

                for (QF_Data& family : m_families)
                    family = {  };

                this->CalculateScore();
                this->CheckExtensionSupport();
                this->FetchFamilyIndices(surface);
                this->CheckIndicesCompletion();

                m_bDeviceSuitable = m_bExtensionsSupported && m_bIndicesComplete;
            }

            inline mc::u32 GetScore()   const { return m_score;           }
            inline bool    IsSuitable() const { return m_bDeviceSuitable; }

            inline const vk::PhysicalDevice&                 GetPhysical()         const { return m_physical;         }
            inline const vk::PhysicalDeviceProperties&       GetProperties()       const { return m_deviceProperties; }
            inline const vk::PhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memoryProperties; }

            std::vector<vk::DeviceQueueCreateInfo> GenerateDeviceQueueCreateInfos() const {
                std::unordered_set<mc::u32> uniqueFamilyIndices;
                uniqueFamilyIndices.reserve(_QF_COUNT);

                for (const QF_Data& family : m_families)
                    uniqueFamilyIndices.insert(family.indices.value().familyIndex);

                const mc::u32 nUniqueQueues = static_cast<mc::u32>(uniqueFamilyIndices.size());

                std::vector<vk::DeviceQueueCreateInfo> createInfos(nUniqueQueues);

                mc::u32 i = 0;
                for (const mc::u32& queueIndex : uniqueFamilyIndices) {
                    vk::DeviceQueueCreateInfo& dqci = createInfos[i++];

                    dqci.flags = vk::DeviceQueueCreateFlagBits{};
                    dqci.queueCount = 1;
                    dqci.pQueuePriorities = &PhysicalDeviceSupport::_fQueuePriority;
                    dqci.queueFamilyIndex = queueIndex;
                }

                return createInfos;
            }

            inline void FetchQueues(const vk::Device& logicalDevice) {
                for (QF_Data& family : m_families) {
                    family.queue = logicalDevice.getQueue(family.indices.value().familyIndex, family.indices.value().queueIndex);
                }
            }
        }; // class PhysicalDeviceSupport

        PhysicalDeviceSupport PickPhysicalDevice(const vk::Instance& instance, const vk::SurfaceKHR& surface) {
            const std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

            std::vector<mc::vk_utils::PhysicalDeviceSupport> supports;
            supports.reserve(physicalDevices.size());

            for (const vk::PhysicalDevice& physicalDevice : physicalDevices) {
                PhysicalDeviceSupport currentSupport(physicalDevice, surface);

                if (currentSupport.IsSuitable())
                    supports.push_back(currentSupport);
            }

            if (supports.size() == 0) {
                throw std::runtime_error("No suitable Vulkan compatible devices were found");
            }

            PhysicalDeviceSupport* pBest = supports.data();
            for (PhysicalDeviceSupport* pCurrent = supports.data(); pCurrent < supports.data() + supports.size(); ++pCurrent)
                if (pCurrent->GetScore() > pBest->GetScore())
                    pBest = pCurrent;

            return *pBest;
        }

    }; // namespace vk_utils

}; // namespace mc