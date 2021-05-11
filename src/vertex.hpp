#pragma once

#include "header.hpp"
#include "vector.hpp"

namespace mc {

    struct Vertex {
        vec3f32 position;
        vec3f32 color;

        static inline vk::VertexInputBindingDescription GetBindingDescription() noexcept {
            vk::VertexInputBindingDescription bindingDescription{};
            bindingDescription.binding   = 0;
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;
            bindingDescription.stride    = sizeof(mc::Vertex);

            return bindingDescription;
        }

        static inline auto GetAttributeDescriptions() noexcept {
            std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding  = 0;
            attributeDescriptions[0].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].offset   = offsetof(mc::Vertex, position);

            attributeDescriptions[1].binding  = 0;
            attributeDescriptions[1].format   = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].offset   = offsetof(mc::Vertex, color);

            return attributeDescriptions;
        }
    }; // struct Vertex

}; // namespace mc