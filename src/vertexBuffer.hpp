#pragma once

#include "header.hpp"
#include "vulkanUtils.hpp"

namespace mc {

	class VertexBuffer {
	private:
		vk::Buffer       m_buffer;
		vk::DeviceMemory m_memory;

		vk::Device m_device;

		std::vector<mc::u8> m_vertices;

	public:
		VertexBuffer() = default;

		VertexBuffer(VertexBuffer&& other) {
			m_buffer = other.m_buffer;
			m_memory = other.m_memory;
			m_device = other.m_device;
			m_vertices = std::move(other.m_vertices);

			other.m_buffer = vk::Buffer{};
			other.m_memory = vk::DeviceMemory{};
			other.m_device = vk::Device{};
		}

		VertexBuffer(const vk::Device& device, const vk::PhysicalDeviceMemoryProperties& deviceMemoryProperties, const std::size_t size)
			: m_device(device), m_vertices(size)
		{
			vk::BufferCreateInfo bci{};
			bci.size        = size;
			bci.usage       = vk::BufferUsageFlagBits::eVertexBuffer;
			bci.sharingMode = vk::SharingMode::eExclusive;

			m_buffer = m_device.createBuffer(bci);

			const vk::MemoryRequirements requirements = m_device.getBufferMemoryRequirements(m_buffer);

			vk::MemoryAllocateInfo allocationInfo{};
			allocationInfo.allocationSize  = requirements.size;
			allocationInfo.memoryTypeIndex = mc::vk_utils::FindMemoryType(deviceMemoryProperties, requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			m_memory = device.allocateMemory(allocationInfo);

			device.bindBufferMemory(m_buffer, m_memory, 0);
		}

		VertexBuffer& operator=(VertexBuffer&& other) {
			new (this) VertexBuffer(std::move(other));

			return *this;
		}

		void* GetData() const noexcept { return (void*)m_vertices.data(); }

		void Upload() const {
			void* const buff = m_device.mapMemory(m_memory, 0, m_vertices.size());

			std::memcpy(buff, m_vertices.data(), m_vertices.size());

			m_device.unmapMemory(m_memory);
		}

		void Bind(const vk::CommandBuffer& cmdBuff) const {
			vk::DeviceSize offset = 0;

			cmdBuff.bindVertexBuffers(0, 1, &m_buffer, &offset);
		}

		~VertexBuffer() {
			if ((VkDevice)m_device != VK_NULL_HANDLE) {
				m_device.freeMemory(m_memory);
				m_device.destroyBuffer(m_buffer);
			}
		}
	}; // class VertexBuffer

}; // namespace mc