/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Skydome.h                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanInterface.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "Camera.h"

#pragma once

class Skydome
{
	private:
		struct Vertex {
			float x, y, z;
		};

		unsigned int vertexCount;
		unsigned int indexCount;

		VulkanBuffer * vertexBuffer;
		VulkanBuffer * indexBuffer;

		glm::mat4 worldMatrix;
		glm::vec4 skyColor;
		glm::vec4 atmosphereColor;
		glm::vec4 groundColor;
		float atmosphereHeight;

		// Vertex shader uniform buffer
		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
		};
		VertexUniformBuffer vertexUniformBuffer;
		VulkanBuffer * vsUBO;

		// Fragment shader uniform buffer
		struct FragmentUniformBuffer
		{
			glm::vec4 skyColor;
			glm::vec4 atmosphereColor;
			glm::vec4 groundColor;
			float atmosphereHeight;
			glm::vec3 padding;
		};
		FragmentUniformBuffer fragmentUniformBuffer;
		VulkanBuffer * fsUBO;

		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
	public:
		Skydome();
		~Skydome();

		bool Init(VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * pipeline, Camera * camera, int framebufferId);
		void SetSkyColor(float r, float g, float b, float a);
		void SetAtmosphereColor(float r, float g, float b, float a);
		void SetGroundColor(float r, float g, float b, float a);
		void SetAtmosphereHeight(float height);
};