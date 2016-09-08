/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: LightManager.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#pragma once

#include <vector>
#include "Light.h"
#include "VulkanBuffer.h"

#define MAX_LIGHTS 32

class LightManager
{
	private:
		std::vector<Light*> sceneLights;

		struct PointLight
		{
			glm::vec4 lightColor;
			glm::vec3 lightPosition;
			float radius;
		};

		struct LightBuffer
		{
			PointLight lights[MAX_LIGHTS];
			int lightCount;
			glm::vec3 padding;
		};
		LightBuffer lightBufferData;
		VulkanBuffer * lightUBO;
	private:
		void Update(VulkanDevice * device);
	public:
		bool Init(VulkanDevice * device);
		void Unload(VulkanDevice * device);
		void AddLightToScene(VulkanDevice * device, Light * light);
		void RemoveLightFromScene(VulkanDevice * device, Light * light);
		VkDescriptorBufferInfo * GetBufferInfo();
};