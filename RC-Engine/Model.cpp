/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: Model.cpp                                            |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>
#include <gtc/type_ptr.hpp>

#include "Model.h"
#include "StdInc.h"
#include "LogManager.h"

extern LogManager * gLogManager;

Model::Model()
{
	vsUniformBuffer = VK_NULL_HANDLE;
}

Model::~Model()
{
	vsUniformBuffer = VK_NULL_HANDLE;
}

bool Model::Init(std::string filename, VulkanInterface * vulkan, VulkanPipeline * vulkanPipeline, VulkanCommandBuffer * cmdBuffer,
	Physics * physics, float mass)
{
	this->physics = physics;

	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	VkResult result;
	
	VkMemoryAllocateInfo allocInfo{};
	uint8_t *pData;

	// Uniform buffer init
	vertexUniformBuffer.worldMatrix = glm::mat4(1.0f);
	vertexUniformBuffer.MVP = glm::mat4();

	// Vertex shader Uniform buffer
	VkBufferCreateInfo vsBufferCI{};
	vsBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vsBufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vsBufferCI.size = sizeof(vertexUniformBuffer);
	vsBufferCI.queueFamilyIndexCount = 0;
	vsBufferCI.pQueueFamilyIndices = VK_NULL_HANDLE;
	vsBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(vulkanDevice->GetDevice(), &vsBufferCI, VK_NULL_HANDLE, &vsUniformBuffer);
	if (result != VK_SUCCESS)
		return false;

	vkGetBufferMemoryRequirements(vulkanDevice->GetDevice(), vsUniformBuffer, &vsMemReq);

	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = vsMemReq.size;
	if (!vulkanDevice->MemoryTypeFromProperties(vsMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex))
		return false;

	result = vkAllocateMemory(vulkanDevice->GetDevice(), &allocInfo, VK_NULL_HANDLE, &vsUniformMemory);
	if (result != VK_SUCCESS)
		return false;

	result = vkMapMemory(vulkanDevice->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	if (result != VK_SUCCESS)
		return false;

	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));

	vkUnmapMemory(vulkanDevice->GetDevice(), vsUniformMemory);

	result = vkBindBufferMemory(vulkanDevice->GetDevice(), vsUniformBuffer, vsUniformMemory, 0);
	if (result != VK_SUCCESS)
		return false;

	vsUniformBufferInfo.buffer = vsUniformBuffer;
	vsUniformBufferInfo.offset = 0;
	vsUniformBufferInfo.range = sizeof(vertexUniformBuffer);

	// Open .rcm file
	FILE * file = fopen(filename.c_str(), "rb");
	if (file == NULL)
	{
		gLogManager->AddMessage("ERROR: Model file not found!");
		return false;
	}
	
	// Open .mat file
	size_t pos = filename.rfind('.');
	filename.replace(pos, 4, ".mat");

	std::ifstream matFile(filename.c_str());
	if (!matFile.is_open())
	{
		gLogManager->AddMessage("ERROR: Model .mat file not found! (" + filename + ")");
		return false;
	}

	unsigned int meshCount;
	fread(&meshCount, sizeof(unsigned int), 1, file);
	
	for (unsigned int i = 0; i < meshCount; i++)
	{
		// Create and read mesh data
		Mesh * mesh = new Mesh();
		if (!mesh->Init(vulkan, file, vulkanPipeline, vsUniformBufferInfo))
		{
			gLogManager->AddMessage("ERROR: Failed to init a mesh!");
			return false;
		}
		meshes.push_back(mesh);

		std::string texturePath;
		char diffuseTextureName[64];
		char specularTextureName[64];

		// Read diffuse texture
		fread(diffuseTextureName, sizeof(char), 64, file);
		if (strcmp(diffuseTextureName, "NONE") == 0)
			texturePath = "data/textures/test.rct";
		else
			texturePath = "data/textures/" + std::string(diffuseTextureName);
		
		Texture * diffuse = new Texture();
		if (!diffuse->Init(vulkan->GetVulkanDevice(), cmdBuffer, texturePath))
		{
			gLogManager->AddMessage("ERROR: Couldn't init texture!");
			return false;
		}
		textures.push_back(diffuse);

		// Read specular texture
		fread(specularTextureName, sizeof(char), 64, file);
		if (strcmp(specularTextureName, "NONE") == 0)
			texturePath = "data/textures/spec.rct";
		else
			texturePath = "data/textures/" + std::string(specularTextureName);

		Texture * specular = new Texture();
		if (!specular->Init(vulkan->GetVulkanDevice(), cmdBuffer, texturePath))
		{
			gLogManager->AddMessage("ERROR: Couldn't init a texture!");
			return false;
		}
		textures.push_back(specular);

		// Init mesh material
		Material * material = new Material();
		material->SetDiffuseTexture(diffuse);
		material->SetSpecularTexture(specular);
		
		std::string matName;
		float specularStrength, specularShininess;
		matFile >> matName >> specularShininess >> specularStrength;

		material->SetSpecularShininess(specularShininess);
		material->SetSpecularStrength(specularStrength);

		materials.push_back(material);
		meshes[i]->SetMaterial(material);

		// Write descriptor set for each mesh
		meshes[i]->WriteDescriptorSet(vulkan, vsUniformBufferInfo);

		// Init draw command buffers for each meash
		VulkanCommandBuffer * drawCmdBuffer = new VulkanCommandBuffer();
		if (!drawCmdBuffer->Init(vulkanDevice, vulkan->GetVulkanCommandPool(), false))
		{
			gLogManager->AddMessage("ERROR: Failed to create a draw command buffer!");
			return false;
		}
		drawCmdBuffers.push_back(drawCmdBuffer);
	}

	fclose(file);
	matFile.close();


	// Read collision file
	pos = filename.rfind('.');
	filename.replace(pos, 4, ".col");

	FILE * colFile = fopen(filename.c_str(), "rb");
	if (colFile == NULL)
		collisionMeshPresent = false;
	else
	{
		collisionMeshPresent = true;

		collisionMesh = new btTriangleMesh();

		struct ColVertex
		{
			float x, y, z;
		};

		unsigned int colVertexCount;

		fread(&colVertexCount, sizeof(unsigned int), 1, colFile);
		for (unsigned int i = 0; i < colVertexCount / 3; i++)
		{
			btVector3 v0, v1, v2;
			ColVertex colVertex;

			fread(&colVertex, sizeof(ColVertex), 1, colFile);
			v0 = btVector3(colVertex.x, colVertex.y, colVertex.z);
			fread(&colVertex, sizeof(ColVertex), 1, colFile);
			v1 = btVector3(colVertex.x, colVertex.y, colVertex.z);
			fread(&colVertex, sizeof(ColVertex), 1, colFile);
			v2 = btVector3(colVertex.x, colVertex.y, colVertex.z);

			collisionMesh->addTriangle(v0, v1, v2);
		}
	}

	if(colFile)
		fclose(colFile);

	// Setup physics object
	this->mass = (btScalar)mass;

	physicsStatic = false;
	if (mass == 0.0f)
		physicsStatic = true;

	if(collisionMeshPresent == false)
	{
		emptyCollisionShape = new btEmptyShape();
		physicsStatic = true;
	}
	else
	{
		collisionShape = new btGImpactMeshShape(collisionMesh);
		collisionShape->setLocalScaling(btVector3(1, 1, 1));
		collisionShape->setMargin(0.0f);
		collisionShape->updateBound();
	}

	btTransform transform;
	transform.setIdentity();
	
	btDefaultMotionState * motionState = new btDefaultMotionState(transform);
	btVector3 inertia(0, 0, 0);

	btCollisionShape * colShape;
	if (physicsStatic == true)
	{
		if (collisionMeshPresent)
		{
			collisionShape->calculateLocalInertia(mass, inertia);
			colShape = collisionShape;
		}
		else
		{
			emptyCollisionShape->calculateLocalInertia(mass, inertia);
			colShape = emptyCollisionShape;
		}
	}
	else
	{
		collisionShape->calculateLocalInertia(mass, inertia);
		colShape = collisionShape;
	}
	
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, colShape, inertia);
	rigidBody = new btRigidBody(rigidBodyCI);
	
	physics->GetDynamicsWorld()->addRigidBody(rigidBody);
	rigidBody->setFriction(1.0f);

	return true;
}

void Model::Unload(VulkanInterface * vulkan)
{
	VulkanDevice * vulkanDevice = vulkan->GetVulkanDevice();
	
	physics->GetDynamicsWorld()->removeRigidBody(rigidBody);
	delete rigidBody->getMotionState();
	delete rigidBody;

	if (physicsStatic == false)
		delete collisionShape;
	if (collisionMeshPresent == true)
		delete collisionMesh;
	else
		delete emptyCollisionShape;

	vkFreeMemory(vulkanDevice->GetDevice(), vsUniformMemory, VK_NULL_HANDLE);
	vkDestroyBuffer(vulkanDevice->GetDevice(), vsUniformBuffer, VK_NULL_HANDLE);

	for(unsigned int i = 0; i < textures.size(); i++)
		SAFE_UNLOAD(textures[i], vulkanDevice);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		SAFE_UNLOAD(drawCmdBuffers[i], vulkanDevice, vulkan->GetVulkanCommandPool());
		SAFE_DELETE(materials[i]);
		SAFE_UNLOAD(meshes[i], vulkan);
	}
}

void Model::Render(VulkanInterface * vulkan, VulkanCommandBuffer * commandBuffer, VulkanPipeline * vulkanPipeline, Camera * camera)
{
	btTransform transform;

	rigidBody->getMotionState()->getWorldTransform(transform);

	transform.getOpenGLMatrix((btScalar*)&vertexUniformBuffer.worldMatrix);
	
	uint8_t *pData;

	// Update vertex uniform buffer
	vertexUniformBuffer.MVP = vulkan->GetProjectionMatrix() * camera->GetViewMatrix() * vertexUniformBuffer.worldMatrix;

	vkMapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory, 0, vsMemReq.size, 0, (void**)&pData);
	memcpy(pData, &vertexUniformBuffer, sizeof(vertexUniformBuffer));
	vkUnmapMemory(vulkan->GetVulkanDevice()->GetDevice(), vsUniformMemory);

	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->UpdateUniformBuffer(vulkan);

		// Record draw command
		drawCmdBuffers[i]->BeginRecordingSecondary(vulkan->GetDeferredRenderpass()->GetRenderpass(), vulkan->GetDeferredFramebuffer());

		vulkan->InitViewportAndScissors(drawCmdBuffers[i]);
		vulkanPipeline->SetActive(drawCmdBuffers[i]);
		vkCmdBindDescriptorSets(drawCmdBuffers[i]->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipelineLayout(), 0, 1, meshes[i]->GetDescriptorSet(), 0, NULL);
		meshes[i]->Render(vulkan, drawCmdBuffers[i]);

		drawCmdBuffers[i]->EndRecording();
		drawCmdBuffers[i]->ExecuteSecondary(commandBuffer);
	}
}

void Model::SetPosition(float x, float y, float z)
{
	physics->GetDynamicsWorld()->removeRigidBody(rigidBody);
	
	if (physicsStatic == false)
	{
		rigidBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
		rigidBody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(x, y, z));

		rigidBody->setCenterOfMassTransform(transform);
		rigidBody->setWorldTransform(transform);
	}
	else
	{
		btTransform transform;
		rigidBody->getMotionState()->getWorldTransform(transform);
		transform.setOrigin(btVector3(x, y, z));
		rigidBody->getMotionState()->setWorldTransform(transform);
		rigidBody->setCenterOfMassTransform(transform);
	}

	rigidBody->activate();
	physics->GetDynamicsWorld()->addRigidBody(rigidBody);
}

void Model::SetRotation(float x, float y, float z)
{
	rotX = x;
	rotY = y;
	rotZ = z;
}

void Model::SetVelocity(float x, float y, float z)
{
	rigidBody->setLinearVelocity(btVector3(x, y, z));
}

unsigned int Model::GetMeshCount()
{
	return (unsigned int)meshes.size();
}

Material * Model::GetMaterial(int materialId)
{
	return materials[materialId];
}
