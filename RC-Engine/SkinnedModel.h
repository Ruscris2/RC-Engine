/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SkinnedModel.h                                       |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include "SkinnedMesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Material.h"
#include "Animation.h"
#include "ShadowMaps.h"

class SkinnedModel
{
	private:
		std::vector<SkinnedMesh*> meshes;
		std::vector<Texture*> textures;
		std::vector<Material*> materials;
		std::vector<VulkanCommandBuffer*> drawCmdBuffers;
		
		Animation * currentAnim;
		unsigned int numBones;
		std::vector <aiMatrix4x4> boneOffsets;
		std::map <std::string, uint32_t> boneMapping;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
			glm::mat4 worldMatrix;
		};
		VertexUniformBuffer vertexUniformBuffer;

		struct BoneUniformBuffer
		{
			glm::mat4 bones[MAX_BONES];
		};
		BoneUniformBuffer boneUniformBufferData;

		VulkanBuffer * skinnedVS_UBO;
		VulkanBuffer * skinnedVS_bone_UBO;
	private:
		void UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline, SkinnedMesh * mesh, ShadowMaps * shadowMaps);
	public:
		SkinnedModel();
		~SkinnedModel();

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			Camera * camera, ShadowMaps * shadowMaps);
		void UpdateAnimation(VulkanInterface * vulkan);
		void SetWorldMatrix(glm::mat4 &worldMatrix);
		void SetAnimation(Animation * anim);
};