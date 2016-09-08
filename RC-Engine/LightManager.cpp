/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: LightManager.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "LightManager.h"
#include "StdInc.h"
#include "LogManager.h"

extern LogManager * gLogManager;

bool LightManager::Init(VulkanDevice * device)
{
	lightBufferData.lightCount = 0;
	lightBufferData.padding = glm::vec3();
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		lightBufferData.lights[i].lightColor = glm::vec4();
		lightBufferData.lights[i].lightPosition = glm::vec3();
		lightBufferData.lights[i].radius = 0.0f;
	}

	lightUBO = new VulkanBuffer();
	if (!lightUBO->Init(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &lightBufferData,
		sizeof(lightBufferData), false))
	{
		gLogManager->AddMessage("ERROR: Failed to create uniform buffer object!");
		return false;
	}
	
	return true;
}

void LightManager::Unload(VulkanDevice * device)
{
	SAFE_UNLOAD(lightUBO, device);
}

void LightManager::AddLightToScene(VulkanDevice * device, Light * light)
{
	if (sceneLights.size() != MAX_LIGHTS)
	{
		sceneLights.push_back(light);
		Update(device);
	}
	else
		gLogManager->AddMessage("WARNING: Scene light limit reached!");
}

void LightManager::RemoveLightFromScene(VulkanDevice * device, Light * light)
{
	for(unsigned int i = 0; i < sceneLights.size(); i++)
		if (sceneLights[i] == light)
		{
			sceneLights.erase(sceneLights.begin() + i);
			Update(device);
			break;
		}
}

VkDescriptorBufferInfo * LightManager::GetBufferInfo()
{
	return lightUBO->GetBufferInfo();
}

void LightManager::Update(VulkanDevice * device)
{
	lightBufferData.lightCount = (int)sceneLights.size();
	for (unsigned int i = 0; i < sceneLights.size(); i++)
	{
		lightBufferData.lights[i].lightColor = sceneLights[i]->GetLightColor();
		lightBufferData.lights[i].lightPosition = sceneLights[i]->GetLightPosition();
		lightBufferData.lights[i].radius = sceneLights[i]->GetLightRadius();
	}
	
	lightUBO->Update(device, &lightBufferData, sizeof(lightBufferData));
}