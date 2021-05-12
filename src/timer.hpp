#pragma once

#include "header.hpp"

/*
 * A very simple Timer class.
 */

namespace mc {

    class Timer {
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

    public:
        inline Timer()
            : m_start(std::chrono::high_resolution_clock::now())
        { }

        inline u32 GetElapsedMS() const {
            const auto end = std::chrono::high_resolution_clock::now();

            return std::chrono::duration_cast<std::chrono::milliseconds>(end - m_start).count();
        }
    }; // class Timer

}; // namespace mc