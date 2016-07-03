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
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;
	public:
		Model();
		~Model();

		bool Init(VulkanDevice * vulkanDevice, VulkanCommandPool * cmdPool);
		void Unload(VulkanDevice * vulkanDevice);
		void Render(VulkanCommandBuffer * commandBuffer);
};