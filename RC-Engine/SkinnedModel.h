/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedModel.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "VulkanPipeline.h"
#include "SkinnedMesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "Animation.h"

class SkinnedModel
{
	private:
		std::vector<SkinnedMesh*> meshes;
		std::vector<Texture*> textures;
		std::vector<Material*> materials;
		std::vector<VulkanCommandBuffer*> drawCmdBuffers;

		float posX, posY, posZ;
		float rotX, rotY, rotZ;
		
		Animation * currentAnim;
		unsigned int numBones;
		std::vector <aiMatrix4x4> boneOffsets;
		std::map <std::string, uint32_t> boneMapping;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
			glm::mat4 worldMatrix;
			glm::mat4 bones[MAX_BONES];
		};
		VertexUniformBuffer vertexUniformBuffer;

		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;
	private:
		void UpdateWorldMatrix();
	public:
		SkinnedModel();
		~SkinnedModel();

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetAnimation(Animation * anim);
};