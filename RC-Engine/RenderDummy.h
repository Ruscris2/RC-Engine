/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: RenderDummy.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "Light.h"
#include "Camera.h"

class RenderDummy
{
	private:
		struct Vertex {
			float x, y, z;
			float u, v;
		};
		unsigned int vertexCount;
		unsigned int indexCount;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		struct FragmentUniformBuffer
		{
			glm::vec4 ambientColor;
			glm::vec4 diffuseColor;
			glm::vec4 specularColor;
			glm::vec3 lightDirection;
			int imageIndex;
			glm::vec3 cameraPosition;
			float padding;
		};
		VertexUniformBuffer vertexUniformBuffer;
		FragmentUniformBuffer fragmentUniformBuffer;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;

		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;

		VkBuffer fsUniformBuffer;
		VkDeviceMemory fsUniformMemory;
		VkDescriptorBufferInfo fsUniformBufferInfo;
		VkMemoryRequirements fsMemReq;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	public:
		RenderDummy();
		~RenderDummy();

		bool Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView * positionView, VkImageView * normalView,
			VkImageView * albedoView, VkImageView * materialView, VkImageView * depthView);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			glm::mat4 orthoMatrix, Light * light, int imageIndex, Camera * camera, int frameBufferId);
};