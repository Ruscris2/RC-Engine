/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Player.h                                             |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "SkinnedModel.h"

#pragma once

class Player
{
	private:
		SkinnedModel * playerModel;
	public:
		Player();
		~Player();

		void Init(SkinnedModel * model);
		void SetPosition(float x, float y, float z);
		float SetDirection(float angle);
		void Update(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera);
};