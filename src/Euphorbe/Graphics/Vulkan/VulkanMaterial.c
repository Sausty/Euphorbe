#include "VulkanMaterial.h"
#include "VulkanRenderer.h"

#include "VulkanImage.h"

#include <spirv_reflect.h>

u32 E_Vk_FormatSize(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_UNDEFINED: return 0;
        case VK_FORMAT_R4G4_UNORM_PACK8: return 1;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return 2;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return 2;
        case VK_FORMAT_R5G6B5_UNORM_PACK16: return 2;
        case VK_FORMAT_B5G6R5_UNORM_PACK16: return 2;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return 2;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return 2;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return 2;
        case VK_FORMAT_R8_UNORM: return 1;
        case VK_FORMAT_R8_SNORM: return 1;
        case VK_FORMAT_R8_USCALED: return 1;
        case VK_FORMAT_R8_SSCALED: return 1;
        case VK_FORMAT_R8_UINT: return 1;
        case VK_FORMAT_R8_SINT: return 1;
        case VK_FORMAT_R8_SRGB: return 1;
        case VK_FORMAT_R8G8_UNORM: return 2;
        case VK_FORMAT_R8G8_SNORM: return 2;
        case VK_FORMAT_R8G8_USCALED: return 2;
        case VK_FORMAT_R8G8_SSCALED: return 2;
        case VK_FORMAT_R8G8_UINT: return 2;
        case VK_FORMAT_R8G8_SINT: return 2;
        case VK_FORMAT_R8G8_SRGB: return 2;
        case VK_FORMAT_R8G8B8_UNORM: return 3;
        case VK_FORMAT_R8G8B8_SNORM: return 3;
        case VK_FORMAT_R8G8B8_USCALED: return 3;
        case VK_FORMAT_R8G8B8_SSCALED: return 3;
        case VK_FORMAT_R8G8B8_UINT: return 3;
        case VK_FORMAT_R8G8B8_SINT: return 3;
        case VK_FORMAT_R8G8B8_SRGB: return 3;
        case VK_FORMAT_B8G8R8_UNORM: return 3;
        case VK_FORMAT_B8G8R8_SNORM: return 3;
        case VK_FORMAT_B8G8R8_USCALED: return 3;
        case VK_FORMAT_B8G8R8_SSCALED: return 3;
        case VK_FORMAT_B8G8R8_UINT: return 3;
        case VK_FORMAT_B8G8R8_SINT: return 3;
        case VK_FORMAT_B8G8R8_SRGB: return 3;
        case VK_FORMAT_R8G8B8A8_UNORM: return 4;
        case VK_FORMAT_R8G8B8A8_SNORM: return 4;
        case VK_FORMAT_R8G8B8A8_USCALED: return 4;
        case VK_FORMAT_R8G8B8A8_SSCALED: return 4;
        case VK_FORMAT_R8G8B8A8_UINT: return 4;
        case VK_FORMAT_R8G8B8A8_SINT: return 4;
        case VK_FORMAT_R8G8B8A8_SRGB: return 4;
        case VK_FORMAT_B8G8R8A8_UNORM: return 4;
        case VK_FORMAT_B8G8R8A8_SNORM: return 4;
        case VK_FORMAT_B8G8R8A8_USCALED: return 4;
        case VK_FORMAT_B8G8R8A8_SSCALED: return 4;
        case VK_FORMAT_B8G8R8A8_UINT: return 4;
        case VK_FORMAT_B8G8R8A8_SINT: return 4;
        case VK_FORMAT_B8G8R8A8_SRGB: return 4;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: return 4;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: return 4;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: return 4;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: return 4;
        case VK_FORMAT_R16_UNORM: return 2;
        case VK_FORMAT_R16_SNORM: return 2;
        case VK_FORMAT_R16_USCALED: return 2;
        case VK_FORMAT_R16_SSCALED: return 2;
        case VK_FORMAT_R16_UINT: return 2;
        case VK_FORMAT_R16_SINT: return 2;
        case VK_FORMAT_R16_SFLOAT: return 2;
        case VK_FORMAT_R16G16_UNORM: return 4;
        case VK_FORMAT_R16G16_SNORM: return 4;
        case VK_FORMAT_R16G16_USCALED: return 4;
        case VK_FORMAT_R16G16_SSCALED: return 4;
        case VK_FORMAT_R16G16_UINT: return 4;
        case VK_FORMAT_R16G16_SINT: return 4;
        case VK_FORMAT_R16G16_SFLOAT: return 4;
        case VK_FORMAT_R16G16B16_UNORM: return 6;
        case VK_FORMAT_R16G16B16_SNORM: return 6;
        case VK_FORMAT_R16G16B16_USCALED: return 6;
        case VK_FORMAT_R16G16B16_SSCALED: return 6;
        case VK_FORMAT_R16G16B16_UINT: return 6;
        case VK_FORMAT_R16G16B16_SINT: return 6;
        case VK_FORMAT_R16G16B16_SFLOAT: return 6;
        case VK_FORMAT_R16G16B16A16_UNORM: return 8;
        case VK_FORMAT_R16G16B16A16_SNORM: return 8;
        case VK_FORMAT_R16G16B16A16_USCALED: return 8;
        case VK_FORMAT_R16G16B16A16_SSCALED: return 8;
        case VK_FORMAT_R16G16B16A16_UINT: return 8;
        case VK_FORMAT_R16G16B16A16_SINT: return 8;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;
        case VK_FORMAT_R32_UINT: return 4;
        case VK_FORMAT_R32_SINT: return 4;
        case VK_FORMAT_R32_SFLOAT: return 4;
        case VK_FORMAT_R32G32_UINT: return 8;
        case VK_FORMAT_R32G32_SINT: return 8;
        case VK_FORMAT_R32G32_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32_UINT: return 12;
        case VK_FORMAT_R32G32B32_SINT: return 12;
        case VK_FORMAT_R32G32B32_SFLOAT: return 12;
        case VK_FORMAT_R32G32B32A32_UINT: return 16;
        case VK_FORMAT_R32G32B32A32_SINT: return 16;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
        case VK_FORMAT_R64_UINT: return 8;
        case VK_FORMAT_R64_SINT: return 8;
        case VK_FORMAT_R64_SFLOAT: return 8;
        case VK_FORMAT_R64G64_UINT: return 16;
        case VK_FORMAT_R64G64_SINT: return 16;
        case VK_FORMAT_R64G64_SFLOAT: return 16;
        case VK_FORMAT_R64G64B64_UINT: return 24;
        case VK_FORMAT_R64G64B64_SINT: return 24;
        case VK_FORMAT_R64G64B64_SFLOAT: return 24;
        case VK_FORMAT_R64G64B64A64_UINT: return 32;
        case VK_FORMAT_R64G64B64A64_SINT: return 32;
        case VK_FORMAT_R64G64B64A64_SFLOAT: return 32;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return 4;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return 4;
    }

    return 0;
}

void E_Vk_MakeShaderModule(VkShaderModule* module, u8* code, i64 size)
{
    VkShaderModuleCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = size;
    info.pCode = (u32*)code;

    VkResult result = vkCreateShaderModule(rhi.device.handle, &info, NULL, module);
    assert(result == VK_SUCCESS);
}

E_VulkanMaterial* E_Vk_CreateMaterial(E_MaterialCreateInfo* create_info)
{
    E_VulkanMaterial* result = malloc(sizeof(E_VulkanMaterial));

    // Make descriptor set layout
    i32 descriptor_desc_count = 0;
    VkDescriptorSetLayoutBinding bindings[EUPHORBE_MAX_DESCRIPTORS] = { 0 };
    for (i32 i = 0; i < create_info->descriptor_count; i++)
    {
        bindings[i].binding = create_info->descriptors[i].binding;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = (VkDescriptorType)create_info->descriptors[i].type;
        bindings[i].pImmutableSamplers = NULL;
        bindings[i].stageFlags = VK_SHADER_STAGE_ALL;
    }

    VkDescriptorSetLayoutCreateInfo layout_info = { 0 };
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = create_info->descriptor_count;
    layout_info.pBindings = bindings;

    VkResult res = vkCreateDescriptorSetLayout(rhi.device.handle, &layout_info, NULL, &result->set_layout);
    assert(res == VK_SUCCESS);

    VkShaderModule vertex_module;
    VkShaderModule fragment_module;

    E_Shader* vertex_shader = create_info->vertex_shader->as.shader;
    E_Shader* fragment_shader = create_info->fragment_shader->as.shader;

    E_Vk_MakeShaderModule(&vertex_module, vertex_shader->code, vertex_shader->code_size);
    E_Vk_MakeShaderModule(&fragment_module, fragment_shader->code, fragment_shader->code_size);

    SpvReflectShaderModule vs_reflect, fs_reflect;

    SpvReflectResult reflect_result = spvReflectCreateShaderModule(vertex_shader->code_size, vertex_shader->code, &vs_reflect);
    assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);
    
    reflect_result = spvReflectCreateShaderModule(fragment_shader->code_size, fragment_shader->code, &fs_reflect);
    assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {0};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vertex_module;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {0};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = fragment_module;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

    VkVertexInputBindingDescription vertex_input_binding_desc = {0};
    vertex_input_binding_desc.binding = 0;
    vertex_input_binding_desc.stride = 0; 
    vertex_input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    u32 vs_input_var_count = 0;
    spvReflectEnumerateInputVariables(&vs_reflect, &vs_input_var_count, 0);
    SpvReflectInterfaceVariable** vs_input_vars = calloc(vs_input_var_count, sizeof(SpvReflectInterfaceVariable*));
    spvReflectEnumerateInputVariables(&vs_reflect, &vs_input_var_count, vs_input_vars);

    VkVertexInputAttributeDescription* attribute_descriptions = malloc(sizeof(VkVertexInputAttributeDescription) * vs_input_var_count);

    for (u32 i = 0; i < vs_input_var_count; i++)
    {
        SpvReflectInterfaceVariable* var = vs_input_vars[i];

        VkVertexInputAttributeDescription attrib = { 0 };

        attrib.location = var->location;
        attrib.binding = 0;
        attrib.format = (VkFormat)var->format;
        attrib.offset = vertex_input_binding_desc.stride;

        attribute_descriptions[i] = attrib;

        vertex_input_binding_desc.stride += E_Vk_FormatSize(attrib.format);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {0};
    vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_info.vertexBindingDescriptionCount = vertex_input_binding_desc.stride == 0 ? 0 : 1;
    vertex_input_state_info.pVertexBindingDescriptions = &vertex_input_binding_desc;
    vertex_input_state_info.vertexAttributeDescriptionCount = vs_input_var_count;
    vertex_input_state_info.pVertexAttributeDescriptions = attribute_descriptions;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = (VkPrimitiveTopology)create_info->primitive_topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = states;

    VkViewport viewport = {0};
    viewport.width = (f32)rhi.window->width;
    viewport.height = (f32)rhi.window->height;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = rhi.swapchain.extent;

    VkPipelineViewportStateCreateInfo viewport_state = {0};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = (VkPolygonMode)create_info->polygon_mode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = (VkCullModeFlagBits)create_info->cull_mode;
    rasterizer.frontFace = (VkFrontFace)create_info->front_face;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {0};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = (VkCompareOp)create_info->depth_op;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; // Optional
    depth_stencil.maxDepthBounds = 1.0f; // Optional
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {0};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &result->set_layout;

    res = vkCreatePipelineLayout(rhi.device.handle, &pipeline_layout_info, NULL, &result->pipeline_layout);
    assert(res == VK_SUCCESS);

    VkPipelineRenderingCreateInfoKHR rendering_create_info = {0};
    rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    rendering_create_info.colorAttachmentCount = create_info->render_info.color_attachment_count;
    rendering_create_info.depthAttachmentFormat = create_info->render_info.depth_format;
    rendering_create_info.pColorAttachmentFormats = (VkFormat*)create_info->render_info.color_formats;
    rendering_create_info.pNext = VK_NULL_HANDLE;

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = result->pipeline_layout;
    pipeline_info.renderPass = VK_NULL_HANDLE;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pNext = &rendering_create_info;

    res = vkCreateGraphicsPipelines(rhi.device.handle, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &result->pipeline);
    assert(res == VK_SUCCESS);

    free(attribute_descriptions);
    free(vs_input_vars);

    spvReflectDestroyShaderModule(&vs_reflect);
    spvReflectDestroyShaderModule(&fs_reflect);
    vkDestroyShaderModule(rhi.device.handle, vertex_module, NULL);
    vkDestroyShaderModule(rhi.device.handle, fragment_module, NULL);

    return result;
}

void E_Vk_FreeMaterial(E_VulkanMaterial* material)
{
    vkDestroyDescriptorSetLayout(rhi.device.handle, material->set_layout, NULL);
    vkDestroyPipeline(rhi.device.handle, material->pipeline, NULL);
    vkDestroyPipelineLayout(rhi.device.handle, material->pipeline_layout, NULL);
    free(material);
}

E_VulkanMaterialInstance* E_Vk_CreateMaterialInstance(E_VulkanMaterial* material)
{
    E_VulkanMaterialInstance* material_instance = malloc(sizeof(E_VulkanMaterialInstance));

    VkDescriptorSetAllocateInfo allocate_info = { 0 };
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = rhi.imgui.descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &material->set_layout;

    VkResult res = vkAllocateDescriptorSets(rhi.device.handle, &allocate_info, &material_instance->set);
    assert(res == VK_SUCCESS);

    return material_instance;
}

void E_Vk_MaterialInstanceWriteBuffer(E_VulkanMaterialInstance* instance, E_DescriptorInstance* buffer, i32 buffer_size)
{
    E_VulkanBuffer* vk_buffer = (E_VulkanBuffer*)buffer->buffer.buffer->rhi_handle;

    VkDescriptorBufferInfo buffer_info = { 0 };
    buffer_info.buffer = vk_buffer->buffer;
    buffer_info.offset = 0;
    buffer_info.range = buffer_size;

    VkWriteDescriptorSet write = { 0 };
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.dstBinding = buffer->descriptor->binding;
    write.dstArrayElement = 0;
    write.dstSet = instance->set;
    write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(rhi.device.handle, 1, &write, 0, NULL);
}

void E_Vk_MaterialInstanceWriteImage(E_VulkanMaterialInstance* instance, E_DescriptorInstance* image)
{
    E_VulkanImage* vk_image = (E_VulkanImage*)image->image.image;

    VkDescriptorImageInfo image_info = { 0 };
    image_info.imageLayout = (VkImageLayout)image->image.layout;
    image_info.sampler = vk_image->sampler;
    image_info.imageView = vk_image->image_view;

    VkWriteDescriptorSet write = { 0 };
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.dstBinding = image->descriptor->binding;
    write.dstArrayElement = 0;
    write.dstSet = instance->set;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(rhi.device.handle, 1, &write, 0, NULL);
}

void E_Vk_FreeMaterialInstance(E_VulkanMaterialInstance* instance)
{
    vkFreeDescriptorSets(rhi.device.handle, rhi.imgui.descriptor_pool, 1, &instance->set);
    free(instance);
}