/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: RenderDummy.h                                        |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Sunlight.h"
#include "Camera.h"
#include "ShadowMaps.h"
#include "LightManager.h"

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
			glm::mat4 lightViewMatrix[SHADOW_CASCADE_COUNT];
			glm::vec3 lightDirection;
			int imageIndex;
			glm::vec3 cameraPosition;
			float lightStrength;
		};
		VertexUniformBuffer vertexUniformBuffer;
		FragmentUniformBuffer fragmentUniformBuffer;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;
		VulkanBuffer * vsUBO;
		VulkanBuffer * fsUBO;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	public:
		RenderDummy();
		~RenderDummy();

		bool Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VkImageView * positionView, VkImageView * normalView,
			VkImageView * albedoView, VkImageView * materialView, VkImageView * depthView, ShadowMaps * shadowMaps,
			LightManager * lightManager, VkImageView * cubemapView);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			glm::mat4 orthoMatrix, Sunlight * light, int imageIndex, Camera * camera, ShadowMaps * shadowMaps, int frameBufferId);
};