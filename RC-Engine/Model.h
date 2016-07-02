/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

class Model
{
	private:
		struct Vertex {
			float x, y, z, w;
			float r, g, b, a;
		};
		VkBuffer vertexBuffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo bufferInfo;
	public:
		Model();
		~Model();

		bool Init(VulkanDevice * vulkanDevice);
		void Unload(VulkanDevice * vulkanDevice);
		void Render(VulkanCommandBuffer * commandBuffer);
};