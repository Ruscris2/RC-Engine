/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.h                                              |
|                             Author: Ruscris2                                           |
==========================================================================================*/
#pragma once

#include <BulletCollision/Gimpact/btGimpactShape.h>

#include "VulkanPipeline.h"
#include "Mesh.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "Physics.h"
#include "ShadowMaps.h"

class Model
{
	private:
		std::vector<Mesh*> meshes;
		std::vector<Texture*> textures;
		std::vector<Material*> materials;
		std::vector<VulkanCommandBuffer*> drawCmdBuffers;

		float posX, posY, posZ;
		float rotX, rotY, rotZ;

		struct VertexUniformBuffer
		{
			glm::mat4 MVP;
			glm::mat4 worldMatrix;
		};
		VertexUniformBuffer vertexUniformBuffer;

		VkBuffer vsUniformBuffer;
		VkDeviceMemory vsUniformMemory;
		VkDescriptorBufferInfo vsUniformBufferInfo;
		VkMemoryRequirements vsMemReq;

		Physics * physics;
		bool collisionMeshPresent;
		bool physicsStatic;
		btCollisionShape * emptyCollisionShape;
		btGImpactMeshShape * collisionShape;
		btTriangleMesh * collisionMesh;
		btRigidBody * rigidBody;
		btScalar mass;
	private:
		bool InitVertexUniformBuffer(VulkanDevice * vulkanDevice);
		bool ReadRCMFile(VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer, std::string filename);
		void ReadCollisionFile(std::string filename);
		void SetupPhysicsObject(float mass);
		void UpdateDescriptorSet(VulkanInterface * vulkan, VulkanPipeline * pipeline, Mesh * mesh);
	public:
		Model();
		~Model();

		bool Init(std::string filename, VulkanInterface * vulkan, VulkanCommandBuffer * cmdBuffer,
			Physics * physics, float mass);
		void Unload(VulkanInterface * vulkan);
		void Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline,
			Camera * camera, ShadowMaps * shadowMaps);
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetVelocity(float x, float y, float z);
		unsigned int GetMeshCount();
		Mesh * GetMesh(int meshId);
		Material * GetMaterial(int materialId);
};