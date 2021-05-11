#pragma once

#include "header.hpp"

namespace mc {

    static std::optional<std::vector<char>> ReadBinaryFileToBuffer(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            return {};

        const auto fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

}; // namespace mc