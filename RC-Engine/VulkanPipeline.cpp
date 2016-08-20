/*========================================================================================
|                                   RC-Engine (c) 2016                                   |
|                             Project: RC-Engine                                         |
|                             File: VulkanPipeline.cpp                                   |
|                             Author: Ruscris2                                           |
==========================================================================================*/

#include "VulkanPipeline.h"

VulkanPipeline::VulkanPipeline()
{
	descriptorLayout = VK_NULL_HANDLE;
	pipelineLayout = VK_NULL_HANDLE;
	pipeline = VK_NULL_HANDLE;
	descriptorPool = VK_NULL_HANDLE;
}

VulkanPipeline::~VulkanPipeline()
{
	descriptorPool = VK_NULL_HANDLE;
	descriptorLayout = VK_NULL_HANDLE;
	pipelineLayout = VK_NULL_HANDLE;
	pipeline = VK_NULL_HANDLE;
}

bool VulkanPipeline::Init(VulkanInterface * vulkan, VulkanPipelineCI * pipelineCI)
{
	VkResult result;
	
	pipelineName = pipelineCI->pipelineName;

	// Vertex layout
	vertexBinding.binding = 0;
	vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexBinding.stride = (uint32_t)pipelineCI->strideSize;

	// Pipeline layout
	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = pipelineCI->numLayoutBindings;
	descriptorLayoutCI.pBindings = pipelineCI->layoutBindings;

	result = vkCreateDescriptorSetLayout(vulkan->GetVulkanDevice()->GetDevice(), &descriptorLayoutCI, VK_NULL_HANDLE, &descriptorLayout);
	if (result != VK_SUCCESS)
		return false;

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &descriptorLayout;

	result = vkCreatePipelineLayout(vulkan->GetVulkanDevice()->GetDevice(), &pipelineLayoutCI, VK_NULL_HANDLE, &pipelineLayout);
	if (result != VK_SUCCESS)
		return false;

	
	// Descriptor pool
	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.maxSets = 1;
	descriptorPoolCI.poolSizeCount = pipelineCI->numLayoutBindings;
	descriptorPoolCI.pPoolSizes = pipelineCI->typeCounts;

	result = vkCreateDescriptorPool(vulkan->GetVulkanDevice()->GetDevice(), &descriptorPoolCI, VK_NULL_HANDLE, &descriptorPool);
	if (result != VK_SUCCESS)
		return false;

	// Descriptor set
	VkDescriptorSetAllocateInfo descSetAllocInfo[1];
	descSetAllocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo[0].pNext = NULL;
	descSetAllocInfo[0].descriptorPool = descriptorPool;
	descSetAllocInfo[0].descriptorSetCount = 1;
	descSetAllocInfo[0].pSetLayouts = &descriptorLayout;
	result = vkAllocateDescriptorSets(vulkan->GetVulkanDevice()->GetDevice(), descSetAllocInfo, &descriptorSet);
	if (result != VK_SUCCESS)
		return false;

	// Pipeline
	VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pNext = NULL;
	dynamicStateCI.pDynamicStates = dynamicStateEnables;
	dynamicStateCI.dynamicStateCount = 0;

	VkPipelineVertexInputStateCreateInfo vi{};
	memset(&vi, 0, sizeof(vi));
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.pNext = NULL;
	vi.flags = 0;
	vi.vertexBindingDescriptionCount = 1;
	vi.pVertexBindingDescriptions = &vertexBinding;
	vi.vertexAttributeDescriptionCount = pipelineCI->numVertexLayout;
	vi.pVertexAttributeDescriptions = pipelineCI->vertexLayout;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
	inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCI.pNext = NULL;
	inputAssemblyCI.primitiveRestartEnable = VK_FALSE;
	inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rs{};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.pNext = NULL;
	rs.polygonMode = (pipelineCI->wireframeEnabled ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL);
	rs.cullMode = pipelineCI->cullMode;
	rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rs.depthClampEnable = VK_FALSE;
	rs.rasterizerDiscardEnable = VK_FALSE;
	rs.depthBiasEnable = (pipelineCI->depthBiasEnabled ? VK_TRUE : VK_FALSE);
	rs.depthBiasConstantFactor = 0;
	rs.depthBiasClamp = 0;
	rs.depthBiasSlopeFactor = 0;
	rs.lineWidth = 1.0f;

	VkPipelineColorBlendStateCreateInfo cb{};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.flags = 0;
	cb.pNext = NULL;

	std::vector<VkPipelineColorBlendAttachmentState> blendAttachState;
	blendAttachState.resize(pipelineCI->numColorAttachments);

	for (unsigned int i = 0; i < blendAttachState.size(); i++)
	{
		if (pipelineCI->transparencyEnabled)
		{
			blendAttachState[i] = {};
			blendAttachState[i].colorWriteMask = 0x0f;
			blendAttachState[i].blendEnable = VK_TRUE;
			blendAttachState[i].alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachState[i].colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		}
		else
		{
			blendAttachState[i] = {};
			blendAttachState[i].colorWriteMask = 0xf;
			blendAttachState[i].blendEnable = VK_FALSE;
			blendAttachState[i].alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachState[i].colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		}
	}

	cb.attachmentCount = (uint32_t)blendAttachState.size();
	cb.pAttachments = blendAttachState.data();
	cb.logicOpEnable = VK_FALSE;
	cb.logicOp = VK_LOGIC_OP_NO_OP;
	cb.blendConstants[0] = 1.0f;
	cb.blendConstants[1] = 1.0f;
	cb.blendConstants[2] = 1.0f;
	cb.blendConstants[3] = 1.0f;

	VkPipelineViewportStateCreateInfo vp{};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.pNext = NULL;
	vp.viewportCount = 1;
	vp.scissorCount = 1;
	vp.pScissors = NULL;
	vp.pViewports = NULL;
	dynamicStateEnables[dynamicStateCI.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStateEnables[dynamicStateCI.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	
	if (pipelineCI->depthBiasEnabled)
		dynamicStateEnables[dynamicStateCI.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;

	VkPipelineDepthStencilStateCreateInfo ds{};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.pNext = NULL;
	ds.depthTestEnable = VK_TRUE;
	ds.depthWriteEnable = VK_TRUE;
	ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.stencilTestEnable = VK_FALSE;
	ds.back.failOp = VK_STENCIL_OP_KEEP;
	ds.back.passOp = VK_STENCIL_OP_KEEP;
	ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
	ds.back.compareMask = 0;
	ds.back.reference = 0;
	ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
	ds.back.writeMask = 0;
	ds.minDepthBounds = 0;
	ds.maxDepthBounds = 0;
	ds.stencilTestEnable = VK_FALSE;
	ds.front = ds.back;

	VkPipelineMultisampleStateCreateInfo ms;
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.pNext = NULL;
	ms.flags = 0;
	ms.pSampleMask = NULL;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ms.sampleShadingEnable = VK_FALSE;
	ms.alphaToCoverageEnable = VK_FALSE;
	ms.alphaToOneEnable = VK_FALSE;
	ms.minSampleShading = 0.0;

	VkGraphicsPipelineCreateInfo graphicsPipelineCI{};
	graphicsPipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCI.pNext = NULL;
	graphicsPipelineCI.layout = pipelineLayout;
	graphicsPipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCI.basePipelineIndex = 0;
	graphicsPipelineCI.flags = 0;
	graphicsPipelineCI.pVertexInputState = &vi;
	graphicsPipelineCI.pInputAssemblyState = &inputAssemblyCI;
	graphicsPipelineCI.pRasterizationState = &rs;
	graphicsPipelineCI.pColorBlendState = &cb;
	graphicsPipelineCI.pTessellationState = VK_NULL_HANDLE;
	graphicsPipelineCI.pMultisampleState = &ms;
	graphicsPipelineCI.pDynamicState = &dynamicStateCI;
	graphicsPipelineCI.pViewportState = &vp;
	graphicsPipelineCI.pDepthStencilState = &ds;
	graphicsPipelineCI.pStages = pipelineCI->shader->GetShaderStages();
	graphicsPipelineCI.stageCount = pipelineCI->shader->GetStageCount();
	graphicsPipelineCI.renderPass = pipelineCI->vulkanRenderpass->GetRenderpass();
	graphicsPipelineCI.subpass = 0;

	result = vkCreateGraphicsPipelines(vulkan->GetVulkanDevice()->GetDevice(), vulkan->GetPipelineCache(), 1,
		&graphicsPipelineCI, VK_NULL_HANDLE, &pipeline);
	if (result != VK_SUCCESS)
		return false;
	
	return true;
}

void VulkanPipeline::Unload(VulkanDevice * vulkanDevice)
{
	vkDestroyDescriptorPool(vulkanDevice->GetDevice(), descriptorPool, VK_NULL_HANDLE);
	vkDestroyPipelineLayout(vulkanDevice->GetDevice(), pipelineLayout, VK_NULL_HANDLE);
	vkDestroyDescriptorSetLayout(vulkanDevice->GetDevice(), descriptorLayout, VK_NULL_HANDLE);
	vkDestroyPipeline(vulkanDevice->GetDevice(), pipeline, VK_NULL_HANDLE);
}

void VulkanPipeline::SetActive(VulkanCommandBuffer * commandBuffer)
{
	vkCmdBindPipeline(commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	vkCmdBindDescriptorSets(commandBuffer->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
}

VkDescriptorSet VulkanPipeline::GetDescriptorSet()
{
	return descriptorSet;
}

VkDescriptorSetLayout * VulkanPipeline::GetDescriptorLayout()
{
	return &descriptorLayout;
}

VkPipelineLayout VulkanPipeline::GetPipelineLayout()
{
	return pipelineLayout;
}

std::string VulkanPipeline::GetPipelineName()
{
	return pipelineName;
}
