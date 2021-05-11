#pragma once

#include "header.hpp"

namespace mc {

    namespace vk_utils {

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

        vk::SurfaceFormatKHR PickSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) noexcept {
            for (const auto& format : formats)
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                    return format;

            return formats[0];
        }

        vk::PresentModeKHR PickSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& modes) noexcept {
            for (const auto& mode : modes)
                if (mode == vk::PresentModeKHR::eMailbox)
                    return mode;

            return vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D PickSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) noexcept {
            if (capabilities.currentExtent.width == UINT32_MAX)
                return capabilities.currentExtent;

            return vk::Extent2D{
                std::clamp(AppSurface::GetWidth(),  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
                std::clamp(AppSurface::GetHeight(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
            };
        }

        mc::u32 FindMemoryType(const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties, const mc::u32 typeFilter, const vk::MemoryPropertyFlags properties) noexcept {
            for (mc::u32 i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
                if ((typeFilter & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;

            //TODO: Optional
            throw std::runtime_error("FindMemoryType failed");
        }

    }; // namespace details

}; // namespace mc