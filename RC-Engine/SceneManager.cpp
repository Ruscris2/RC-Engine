/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: SceneManager.cpp                                     |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include <fstream>

#include "SceneManager.h"
#include "StdInc.h"
#include "LogManager.h"
#include "Input.h"
#include "Timer.h"
#include "StdInc.h"

extern LogManager * gLogManager;
extern Input * gInput;
extern Timer * gTimer;

SceneManager::SceneManager()
{
	physics = NULL;
	camera = NULL;
	light = NULL;
	initCommandBuffer = NULL;
	deferredCommandBuffer = NULL;
	defaultShader = NULL;
	skinnedShader = NULL;
	deferredShader = NULL;
	defaultPipeline = NULL;
	skinnedPipeline = NULL;
	deferredPipeline = NULL;
	wireframePipeline = NULL;
	defaultShaderCanvas = NULL;
	idleAnim = NULL;
	player = NULL;
}

SceneManager::~SceneManager()
{
	SAFE_DELETE(player);
	SAFE_DELETE(idleAnim);
	SAFE_DELETE(light);
	SAFE_DELETE(camera);
	SAFE_DELETE(physics);
}

bool SceneManager::Init(VulkanInterface * vulkan)
{
	// Physics init
	physics = new Physics();
	physics->Init();

	// Camera setup
	camera = new Camera();
	camera->Init();
	camera->SetPosition(0.0f, 5.0f, -10.0f);
	camera->SetDirection(0.0f, 0.0f, 1.0f);
	camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);
	
	// Light setup
	light = new Light();
	light->SetAmbientColor(0.3f, 0.3f, 0.3f, 1.0f);
	light->SetDiffuseColor(0.6f, 0.6f, 0.6f, 1.0f);
	light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	light->SetLightDirection(-0.5f, -0.5f, 1.0f);

	// Init command buffers
	initCommandBuffer = new VulkanCommandBuffer();
	if (!initCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (initCommandBuffer)");
		return false;
	}

	deferredCommandBuffer = new VulkanCommandBuffer();
	if (!deferredCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (deferredCommandBuffer)");
		return false;
	}

	for(size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
		if (!cmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
		{
			gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffers)");
			return false;
		}
		renderCommandBuffers.push_back(cmdBuffer);

		VulkanCommandBuffer * secondaryCmdBuffer = new VulkanCommandBuffer();
		if (!secondaryCmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), false))
		{
			gLogManager->AddMessage("ERROR: Failed to craete a command buffer! (screenQuadDrawCmdBuffer)");
			return false;
		}
		screenQuadDrawCmdBuffers.push_back(secondaryCmdBuffer);
	}

	// Init shaders
	defaultShader = new DefaultShader();
	if (!defaultShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init default shader!");
		return false;
	}

	skinnedShader = new SkinnedShader();
	if (!skinnedShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init skinned shader!");
		return false;
	}

	deferredShader = new DeferredShader();
	if (!deferredShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred shader!");
		return false;
	}

	wireframeShader = new WireframeShader();
	if (!wireframeShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init wireframe shader!");
		return false;
	}

	// Build pipelines
	if (!BuildDefaultPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init default pipeline!");
		return false;
	}

	if (!BuildSkinnedPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init skinned pipeline!");
		return false;
	}

	if (!BuildDeferredPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init deferred pipeline!");
		return false;
	}

	if (!BuildWireframePipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init wireframe pipeline!");
		return false;
	}

	// Init screen quad
	defaultShaderCanvas = new Canvas();
	if (!defaultShaderCanvas->Init(vulkan, defaultPipeline, vulkan->GetPositionAttachment()->GetImageView(), vulkan->GetNormalAttachment()->GetImageView(),
		vulkan->GetAlbedoAttachment()->GetImageView(), vulkan->GetMaterialAttachment()->GetImageView()))
	{
		gLogManager->AddMessage("ERROR: Failed to init default shader canvas!");
		return false;
	}

	// Load map files
	if (!LoadMapFile("data/testmap.map", vulkan))
		return false;

	// Skinned models and animations
	male = new SkinnedModel();
	if (!male->Init("data/models/male.rcs", vulkan, skinnedPipeline, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init male model!");
		return false;
	}

	idleAnim = new Animation();
	if (!idleAnim->Init("data/anims/idle.fbx", 52))
		return false;
	idleAnim->SetAnimationSpeed(0.001f);

	male->SetAnimation(idleAnim);

	player = new Player();
	player->Init(male);
	player->SetPosition(0.0f, 0.0f, 0.0f);

	GEOMETRY_GENERATE_INFO geometryInfo{};
	geometryInfo.type = GEOMETRY_TYPE_BOX;
	geometryInfo.width = 3.0f;
	geometryInfo.height = 1.0f;
	geometryInfo.length = 1.0f;

	testModel = new WireframeModel();
	if (!testModel->Init(vulkan, wireframePipeline, geometryInfo, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)))
	{
		gLogManager->AddMessage("ERROR: Failed to init test model!");
		return false;
	}
	
	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(testModel, vulkan);

	SAFE_UNLOAD(male, vulkan);
	for (unsigned int i = 0; i < modelList.size(); i++)
		SAFE_UNLOAD(modelList[i], vulkan, physics);

	SAFE_UNLOAD(defaultShaderCanvas, vulkan);
	SAFE_UNLOAD(wireframePipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(deferredPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skinnedPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(wireframeShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(deferredShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skinnedShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultShader, vulkan->GetVulkanDevice());

	for (unsigned int i = 0; i < renderCommandBuffers.size(); i++)
	{
		SAFE_UNLOAD(renderCommandBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
		SAFE_UNLOAD(screenQuadDrawCmdBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	}
	SAFE_UNLOAD(deferredCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(initCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

int imageIndex = 5;
float angle = 0.0f;

void SceneManager::Render(VulkanInterface * vulkan)
{
	physics->Update();

	if (gInput->WasKeyPressed(KEYBOARD_KEY_6))
	{
		Model * model = new Model();
		model->Init("data/models/box.rcm", vulkan, deferredPipeline, initCommandBuffer, physics, 20.0f);
		
		glm::vec3 pos = camera->GetPosition();
		glm::vec3 dir = camera->GetDirection();
		float power = 5.0f;
		model->SetPosition(pos.x, pos.y, pos.z);
		model->SetVelocity(dir.x * power, dir.y * power, dir.z * power);

		modelList.push_back(model);
	}
	if (gInput->WasKeyPressed(KEYBOARD_KEY_7))
		camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);
	if (gInput->WasKeyPressed(KEYBOARD_KEY_8))
		camera->SetCameraState(CAMERA_STATE_FLY);

	angle += 0.001f * gTimer->GetDelta();
	if (angle > 90.0f)
		angle = 0.0f;
		
	camera->HandleInput();
	light->SetLightDirection(glm::sin(angle), -0.5f, 1.0f);

	// Debug deferred shading
	if (gInput->WasKeyPressed(KEYBOARD_KEY_1))
		imageIndex = 1;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_2))
		imageIndex = 2;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_3))
		imageIndex = 3;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_4))
		imageIndex = 4;
	if (gInput->WasKeyPressed(KEYBOARD_KEY_5))
		imageIndex = 5;
	
	// Deferred rendering
	vulkan->BeginSceneDeferred(deferredCommandBuffer);

	for (unsigned int i = 0; i < modelList.size(); i++)
		modelList[i]->Render(vulkan, deferredCommandBuffer, deferredPipeline, camera);
	
	player->Update(vulkan, deferredCommandBuffer, skinnedPipeline, camera);
	
	vulkan->EndSceneDeferred(deferredCommandBuffer);

	// Forward rendering
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		vulkan->BeginSceneForward(renderCommandBuffers[i], defaultPipeline, (int)i);

		screenQuadDrawCmdBuffers[i]->BeginRecordingSecondary(vulkan->GetForwardRenderpass()->GetRenderpass(), vulkan->GetVulkanSwapchain()->GetFramebuffer((int)i));
		vulkan->InitViewportAndScissors(screenQuadDrawCmdBuffers[i]);

		defaultPipeline->SetActive(screenQuadDrawCmdBuffers[i]);
		defaultShaderCanvas->Render(vulkan, screenQuadDrawCmdBuffers[i], defaultPipeline, vulkan->GetOrthoMatrix(), light, imageIndex, camera);

		screenQuadDrawCmdBuffers[i]->EndRecording();
		screenQuadDrawCmdBuffers[i]->ExecuteSecondary(renderCommandBuffers[i]);

		testModel->Render(vulkan, renderCommandBuffers[i], wireframePipeline, camera, (int)i);

		vulkan->EndSceneForward(renderCommandBuffers[i]);
	}

	// Present to screen
	vulkan->Present(renderCommandBuffers);
}

bool SceneManager::LoadMapFile(std::string filename, VulkanInterface * vulkan)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		gLogManager->AddMessage("ERROR: Couldn't find map file: " + filename);
		return false;
	}

	while (!file.eof())
	{
		std::string modelPath;
		std::string modelName;
		float posX, posY, posZ;
		float rotX, rotY, rotZ;
		float mass;

		file >> modelName >> posX >> posY >> posZ >> rotX >> rotY >> rotZ >> mass;

		modelName.append(".rcm");

		modelPath = "data/models/" + modelName;

		Model * model = new Model();
		if (!model->Init(modelPath, vulkan, deferredPipeline, initCommandBuffer, physics, mass))
		{
			gLogManager->AddMessage("ERROR: Failed to init model: " + modelName);
			return false;
		}

		model->SetPosition(posX, posY, posZ);
		model->SetRotation(rotX, rotY, rotZ);

		modelList.push_back(model);
	}

	file.close();
	return true;
}

bool SceneManager::BuildDefaultPipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutDefault[2];

	vertexLayoutDefault[0].binding = 0;
	vertexLayoutDefault[0].location = 0;
	vertexLayoutDefault[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDefault[0].offset = 0;

	vertexLayoutDefault[1].binding = 0;
	vertexLayoutDefault[1].location = 1;
	vertexLayoutDefault[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutDefault[1].offset = sizeof(float) * 3;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsDefault[6];

	layoutBindingsDefault[0].binding = 0;
	layoutBindingsDefault[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDefault[0].descriptorCount = 1;
	layoutBindingsDefault[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsDefault[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[1].binding = 1;
	layoutBindingsDefault[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[1].descriptorCount = 1;
	layoutBindingsDefault[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[1].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[2].binding = 2;
	layoutBindingsDefault[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[2].descriptorCount = 1;
	layoutBindingsDefault[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[2].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[3].binding = 3;
	layoutBindingsDefault[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[3].descriptorCount = 1;
	layoutBindingsDefault[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[3].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[4].binding = 4;
	layoutBindingsDefault[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[4].descriptorCount = 1;
	layoutBindingsDefault[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[4].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[5].binding = 5;
	layoutBindingsDefault[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDefault[5].descriptorCount = 1;
	layoutBindingsDefault[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[5].pImmutableSamplers = VK_NULL_HANDLE;

	struct DefaultVertex {
		float x, y, z;
		float u, v;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.vulkanDevice = vulkan->GetVulkanDevice();
	pipelineCI.shader = defaultShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutDefault;
	pipelineCI.numVertexLayout = 2;
	pipelineCI.layoutBindings = layoutBindingsDefault;
	pipelineCI.numLayoutBindings = 6;
	pipelineCI.strideSize = sizeof(DefaultVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.zbufferEnabled = false;

	defaultPipeline = new VulkanPipeline();
	if (!defaultPipeline->Init(&pipelineCI))
		return false;

	return true;
}

bool SceneManager::BuildSkinnedPipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutSkinned[5];

	// Position
	vertexLayoutSkinned[0].binding = 0;
	vertexLayoutSkinned[0].location = 0;
	vertexLayoutSkinned[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutSkinned[0].offset = 0;

	// Texture coords
	vertexLayoutSkinned[1].binding = 0;
	vertexLayoutSkinned[1].location = 1;
	vertexLayoutSkinned[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutSkinned[1].offset = sizeof(float) * 3;

	// Normals
	vertexLayoutSkinned[2].binding = 0;
	vertexLayoutSkinned[2].location = 2;
	vertexLayoutSkinned[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutSkinned[2].offset = sizeof(float) * 5;

	// Bone weights
	vertexLayoutSkinned[3].binding = 0;
	vertexLayoutSkinned[3].location = 3;
	vertexLayoutSkinned[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexLayoutSkinned[3].offset = sizeof(float) * 8;

	// Bone IDs
	vertexLayoutSkinned[4].binding = 0;
	vertexLayoutSkinned[4].location = 4;
	vertexLayoutSkinned[4].format = VK_FORMAT_R32G32B32A32_SINT;
	vertexLayoutSkinned[4].offset = sizeof(float) * 12;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsSkinned[4];

	layoutBindingsSkinned[0].binding = 0;
	layoutBindingsSkinned[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsSkinned[0].descriptorCount = 1;
	layoutBindingsSkinned[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsSkinned[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsSkinned[1].binding = 1;
	layoutBindingsSkinned[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsSkinned[1].descriptorCount = 1;
	layoutBindingsSkinned[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsSkinned[1].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsSkinned[2].binding = 2;
	layoutBindingsSkinned[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsSkinned[2].descriptorCount = 1;
	layoutBindingsSkinned[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsSkinned[2].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsSkinned[3].binding = 3;
	layoutBindingsSkinned[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsSkinned[3].descriptorCount = 1;
	layoutBindingsSkinned[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsSkinned[3].pImmutableSamplers = VK_NULL_HANDLE;

	struct SkinnedVertex {
		float x, y, z;
		float u, v;
		float nx, ny, nz;
		float boneWeights[4];
		uint32_t boneIDs[4];
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.vulkanDevice = vulkan->GetVulkanDevice();
	pipelineCI.shader = skinnedShader;
	pipelineCI.vulkanRenderpass = vulkan->GetDeferredRenderpass();
	pipelineCI.vertexLayout = vertexLayoutSkinned;
	pipelineCI.numVertexLayout = 5;
	pipelineCI.layoutBindings = layoutBindingsSkinned;
	pipelineCI.numLayoutBindings = 4;
	pipelineCI.strideSize = sizeof(SkinnedVertex);
	pipelineCI.numColorAttachments = 4;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.zbufferEnabled = true;

	skinnedPipeline = new VulkanPipeline();
	if (!skinnedPipeline->Init(&pipelineCI))
		return false;

	return true;
}

bool SceneManager::BuildDeferredPipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutDeferred[3];

	vertexLayoutDeferred[0].binding = 0;
	vertexLayoutDeferred[0].location = 0;
	vertexLayoutDeferred[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDeferred[0].offset = 0;

	vertexLayoutDeferred[1].binding = 0;
	vertexLayoutDeferred[1].location = 1;
	vertexLayoutDeferred[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutDeferred[1].offset = sizeof(float) * 3;

	vertexLayoutDeferred[2].binding = 0;
	vertexLayoutDeferred[2].location = 2;
	vertexLayoutDeferred[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutDeferred[2].offset = sizeof(float) * 5;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsDeferred[4];

	layoutBindingsDeferred[0].binding = 0;
	layoutBindingsDeferred[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDeferred[0].descriptorCount = 1;
	layoutBindingsDeferred[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsDeferred[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDeferred[1].binding = 1;
	layoutBindingsDeferred[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDeferred[1].descriptorCount = 1;
	layoutBindingsDeferred[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDeferred[1].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDeferred[2].binding = 2;
	layoutBindingsDeferred[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDeferred[2].descriptorCount = 1;
	layoutBindingsDeferred[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDeferred[2].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDeferred[3].binding = 3;
	layoutBindingsDeferred[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDeferred[3].descriptorCount = 1;
	layoutBindingsDeferred[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDeferred[3].pImmutableSamplers = VK_NULL_HANDLE;

	struct DeferredVertex {
		float x, y, z;
		float u, v;
		float nx, ny, nz;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.vulkanDevice = vulkan->GetVulkanDevice();
	pipelineCI.shader = deferredShader;
	pipelineCI.vulkanRenderpass = vulkan->GetDeferredRenderpass();
	pipelineCI.vertexLayout = vertexLayoutDeferred;
	pipelineCI.numVertexLayout = 3;
	pipelineCI.layoutBindings = layoutBindingsDeferred;
	pipelineCI.numLayoutBindings = 4;
	pipelineCI.strideSize = sizeof(DeferredVertex);
	pipelineCI.numColorAttachments = 4;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.zbufferEnabled = true;

	deferredPipeline = new VulkanPipeline();
	if (!deferredPipeline->Init(&pipelineCI))
		return false;

	return true;
}

bool SceneManager::BuildWireframePipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutWireframe[2];

	vertexLayoutWireframe[0].binding = 0;
	vertexLayoutWireframe[0].location = 0;
	vertexLayoutWireframe[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutWireframe[0].offset = 0;

	vertexLayoutWireframe[1].binding = 0;
	vertexLayoutWireframe[1].location = 1;
	vertexLayoutWireframe[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexLayoutWireframe[1].offset = sizeof(float) * 3;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsWireframe[1];

	layoutBindingsWireframe[0].binding = 0;
	layoutBindingsWireframe[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsWireframe[0].descriptorCount = 1;
	layoutBindingsWireframe[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsWireframe[0].pImmutableSamplers = VK_NULL_HANDLE;

	struct WireframeVertex {
		float x, y, z;
		float r, g, b, a;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.vulkanDevice = vulkan->GetVulkanDevice();
	pipelineCI.shader = wireframeShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutWireframe;
	pipelineCI.numVertexLayout = 2;
	pipelineCI.layoutBindings = layoutBindingsWireframe;
	pipelineCI.numLayoutBindings = 1;
	pipelineCI.strideSize = sizeof(WireframeVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = true;
	pipelineCI.zbufferEnabled = false;

	wireframePipeline = new VulkanPipeline();
	if (!wireframePipeline->Init(&pipelineCI))
		return false;

	return true;
}