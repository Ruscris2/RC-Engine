/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: PipelineManager.cpp                                  |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "PipelineManager.h"
#include "LogManager.h"
#include "StdInc.h"

extern LogManager * gLogManager;

PipelineManager::PipelineManager()
{
	defaultShader = NULL;
	skinnedShader = NULL;
	deferredShader = NULL;
	wireframeShader = NULL;
	skydomeShader = NULL;
	canvasShader = NULL;

	defaultPipeline = NULL;
	skinnedPipeline = NULL;
	deferredPipeline = NULL;
	wireframePipeline = NULL;
	skydomePipeline = NULL;
	canvasPipeline = NULL;
}

bool PipelineManager::InitUIPipelines(VulkanInterface * vulkan)
{
	canvasShader = new CanvasShader();
	if (!canvasShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init canvas shader!");
		return false;
	}

	if (!BuildCanvasPipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init canvas pipeline!");
		return false;
	}

	return true;
}

bool PipelineManager::InitGamePipelines(VulkanInterface * vulkan)
{
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

	skydomeShader = new SkydomeShader();
	if (!skydomeShader->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init skydome shader!");
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

	if (!BuildSkydomePipeline(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init skydome pipeline!");
		return false;
	}

	return true;
}

void PipelineManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(canvasPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skydomePipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(wireframePipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(deferredPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skinnedPipeline, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultPipeline, vulkan->GetVulkanDevice());

	SAFE_UNLOAD(canvasShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skydomeShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(wireframeShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(deferredShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(skinnedShader, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(defaultShader, vulkan->GetVulkanDevice());
}

VulkanPipeline * PipelineManager::GetDefault()
{
	return defaultPipeline;
}

VulkanPipeline * PipelineManager::GetSkinned()
{
	return skinnedPipeline;
}

VulkanPipeline * PipelineManager::GetDeferred()
{
	return deferredPipeline;
}

VulkanPipeline * PipelineManager::GetWireframe()
{
	return wireframePipeline;
}

VulkanPipeline * PipelineManager::GetSkydome()
{
	return skydomePipeline;
}

VulkanPipeline * PipelineManager::GetCanvas()
{
	return canvasPipeline;
}

bool PipelineManager::BuildDefaultPipeline(VulkanInterface * vulkan)
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
	VkDescriptorSetLayoutBinding layoutBindingsDefault[7];

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
	layoutBindingsDefault[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsDefault[5].descriptorCount = 1;
	layoutBindingsDefault[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[5].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsDefault[6].binding = 6;
	layoutBindingsDefault[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsDefault[6].descriptorCount = 1;
	layoutBindingsDefault[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsDefault[6].pImmutableSamplers = VK_NULL_HANDLE;

	struct DefaultVertex {
		float x, y, z;
		float u, v;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.shader = defaultShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutDefault;
	pipelineCI.numVertexLayout = 2;
	pipelineCI.layoutBindings = layoutBindingsDefault;
	pipelineCI.numLayoutBindings = 7;
	pipelineCI.strideSize = sizeof(DefaultVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.backFaceCullingEnabled = true;
	pipelineCI.transparencyEnabled = false;

	defaultPipeline = new VulkanPipeline();
	if (!defaultPipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}

bool PipelineManager::BuildSkinnedPipeline(VulkanInterface * vulkan)
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
	pipelineCI.shader = skinnedShader;
	pipelineCI.vulkanRenderpass = vulkan->GetDeferredRenderpass();
	pipelineCI.vertexLayout = vertexLayoutSkinned;
	pipelineCI.numVertexLayout = 5;
	pipelineCI.layoutBindings = layoutBindingsSkinned;
	pipelineCI.numLayoutBindings = 4;
	pipelineCI.strideSize = sizeof(SkinnedVertex);
	pipelineCI.numColorAttachments = 4;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.backFaceCullingEnabled = true;
	pipelineCI.transparencyEnabled = false;

	skinnedPipeline = new VulkanPipeline();
	if (!skinnedPipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}

bool PipelineManager::BuildDeferredPipeline(VulkanInterface * vulkan)
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
	pipelineCI.shader = deferredShader;
	pipelineCI.vulkanRenderpass = vulkan->GetDeferredRenderpass();
	pipelineCI.vertexLayout = vertexLayoutDeferred;
	pipelineCI.numVertexLayout = 3;
	pipelineCI.layoutBindings = layoutBindingsDeferred;
	pipelineCI.numLayoutBindings = 4;
	pipelineCI.strideSize = sizeof(DeferredVertex);
	pipelineCI.numColorAttachments = 4;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.backFaceCullingEnabled = true;
	pipelineCI.transparencyEnabled = false;

	deferredPipeline = new VulkanPipeline();
	if (!deferredPipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}

bool PipelineManager::BuildWireframePipeline(VulkanInterface * vulkan)
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
	pipelineCI.shader = wireframeShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutWireframe;
	pipelineCI.numVertexLayout = 2;
	pipelineCI.layoutBindings = layoutBindingsWireframe;
	pipelineCI.numLayoutBindings = 1;
	pipelineCI.strideSize = sizeof(WireframeVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = true;
	pipelineCI.backFaceCullingEnabled = false;
	pipelineCI.transparencyEnabled = false;

	wireframePipeline = new VulkanPipeline();
	if (!wireframePipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}

bool PipelineManager::BuildSkydomePipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutSkydome[1];

	vertexLayoutSkydome[0].binding = 0;
	vertexLayoutSkydome[0].location = 0;
	vertexLayoutSkydome[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutSkydome[0].offset = 0;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsSkydome[2];

	layoutBindingsSkydome[0].binding = 0;
	layoutBindingsSkydome[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsSkydome[0].descriptorCount = 1;
	layoutBindingsSkydome[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsSkydome[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsSkydome[1].binding = 1;
	layoutBindingsSkydome[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsSkydome[1].descriptorCount = 1;
	layoutBindingsSkydome[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsSkydome[1].pImmutableSamplers = VK_NULL_HANDLE;

	struct SkydomeVertex {
		float x, y, z;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.shader = skydomeShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutSkydome;
	pipelineCI.numVertexLayout = 1;
	pipelineCI.layoutBindings = layoutBindingsSkydome;
	pipelineCI.numLayoutBindings = 2;
	pipelineCI.strideSize = sizeof(SkydomeVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.backFaceCullingEnabled = false;
	pipelineCI.transparencyEnabled = false;

	skydomePipeline = new VulkanPipeline();
	if (!skydomePipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}

bool PipelineManager::BuildCanvasPipeline(VulkanInterface * vulkan)
{
	// Vertex layout
	VkVertexInputAttributeDescription vertexLayoutCanvas[2];

	vertexLayoutCanvas[0].binding = 0;
	vertexLayoutCanvas[0].location = 0;
	vertexLayoutCanvas[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexLayoutCanvas[0].offset = 0;

	vertexLayoutCanvas[1].binding = 0;
	vertexLayoutCanvas[1].location = 1;
	vertexLayoutCanvas[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexLayoutCanvas[1].offset = sizeof(float) * 3;

	// Layout bindings
	VkDescriptorSetLayoutBinding layoutBindingsCanvas[2];

	layoutBindingsCanvas[0].binding = 0;
	layoutBindingsCanvas[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindingsCanvas[0].descriptorCount = 1;
	layoutBindingsCanvas[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBindingsCanvas[0].pImmutableSamplers = VK_NULL_HANDLE;

	layoutBindingsCanvas[1].binding = 1;
	layoutBindingsCanvas[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindingsCanvas[1].descriptorCount = 1;
	layoutBindingsCanvas[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBindingsCanvas[1].pImmutableSamplers = VK_NULL_HANDLE;

	struct CanvasVertex {
		float x, y, z;
		float u, v;
	};

	VulkanPipelineCI pipelineCI;
	pipelineCI.shader = canvasShader;
	pipelineCI.vulkanRenderpass = vulkan->GetForwardRenderpass();
	pipelineCI.vertexLayout = vertexLayoutCanvas;
	pipelineCI.numVertexLayout = 2;
	pipelineCI.layoutBindings = layoutBindingsCanvas;
	pipelineCI.numLayoutBindings = 2;
	pipelineCI.strideSize = sizeof(CanvasVertex);
	pipelineCI.numColorAttachments = 1;
	pipelineCI.wireframeEnabled = false;
	pipelineCI.backFaceCullingEnabled = true;
	pipelineCI.transparencyEnabled = true;

	canvasPipeline = new VulkanPipeline();
	if (!canvasPipeline->Init(vulkan, &pipelineCI))
		return false;

	return true;
}