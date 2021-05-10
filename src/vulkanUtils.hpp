#pragma once

#include "header.hpp"

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