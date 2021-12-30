#include "VulkanImage.h"

#include "VulkanRenderer.h"

#include <cimgui_impl.h>
#include <stb_image.h>

VkImageAspectFlags EuphorbeFormatToVulkanAspect(E_ImageFormat format)
{
    switch (format)
    {
    case E_ImageFormatD32_Float:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case E_ImageFormatRGBA8:
    case E_ImageFormatRGBA16:
    case E_ImageFormatRGBA32:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    return 0;
}

E_VulkanImage* E_Vk_MakeImage(i32 width, i32 height, E_ImageFormat format, E_ImageUsage usage)
{
    E_VulkanImage* result = malloc(sizeof(E_VulkanImage));

    // Only used to make the C6011 warning go away
    if (result)
    {
        result->format = (VkFormat)format;
        result->euphorbe_format = format;
        result->euphorbe_usage = usage;

        VkImageCreateInfo image_create_info = { 0 };
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = width;
        image_create_info.extent.height = height;
        image_create_info.format = result->format;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = usage;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        result->image_extent = image_create_info.extent;

        VmaAllocationCreateInfo allocation = { 0 };
        allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult res = vmaCreateImage(rhi.allocator, &image_create_info, &allocation, &result->image, &result->allocation, &result->allocation_info);
        assert(res == VK_SUCCESS);

        VkImageViewCreateInfo view_info = { 0 };
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = result->image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = result->format;
        view_info.subresourceRange.aspectMask = EuphorbeFormatToVulkanAspect(format);
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        res = vkCreateImageView(rhi.device.handle, &view_info, NULL, &result->image_view);
        assert(res == VK_SUCCESS);

        VkSamplerCreateInfo sampler_info = {0};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_TRUE;
        sampler_info.maxAnisotropy = rhi.physical_device.handle_props.limits.maxSamplerAnisotropy;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        res = vkCreateSampler(rhi.device.handle, &sampler_info, NULL, &result->sampler);
        assert(res == VK_SUCCESS);

        VkDescriptorSetLayout set_layout = ImGui_ImplVulkan_GetDescriptorSetLayout();

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorSetAllocateInfo alloc_info = { 0 };
            alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            alloc_info.descriptorSetCount = 1;
            alloc_info.pSetLayouts = &set_layout;
            alloc_info.descriptorPool = rhi.imgui.descriptor_pool;

            res = vkAllocateDescriptorSets(rhi.device.handle, &alloc_info, &result->gui_descriptor_set[i]);
            assert(res == VK_SUCCESS);
        }

        return result;
    }

    return NULL;
}

E_VulkanImage* E_Vk_MakeImageFromFile(const char* path)
{
    E_VulkanImage* result = malloc(sizeof(E_VulkanImage));

    stbi_set_flip_vertically_on_load(0);
    i32 width = 0, height = 0, channels = 0;
    u8* data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize image_size = width * height * 4; // Force alpha channel

    result->format = VK_FORMAT_R8G8B8A8_UNORM;
    result->euphorbe_format = E_ImageFormatRGBA8;
    result->image_extent.width = width;
    result->image_extent.height = height;

    VkImageCreateInfo image_create_info = { 0 };
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.format = result->format;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation = { 0 };
    allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult res = vmaCreateImage(rhi.allocator, &image_create_info, &allocation, &result->image, &result->allocation, &result->allocation_info);
    assert(res == VK_SUCCESS);

    VkSamplerCreateInfo sampler_info = { 0 };
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = rhi.physical_device.handle_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    res = vkCreateSampler(rhi.device.handle, &sampler_info, NULL, &result->sampler);
    assert(res == VK_SUCCESS);

    VkDescriptorSetLayout set_layout = ImGui_ImplVulkan_GetDescriptorSetLayout();
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSetAllocateInfo alloc_info = { 0 };
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &set_layout;
        alloc_info.descriptorPool = rhi.imgui.descriptor_pool;

        res = vkAllocateDescriptorSets(rhi.device.handle, &alloc_info, &result->gui_descriptor_set[i]);
        assert(res == VK_SUCCESS);
    }

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_buffer_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo staging_buffer_allocation_info = {0};

    VkBufferCreateInfo staging_buffer_info = { 0 };
    staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_info.size = image_size;

    VmaAllocationCreateInfo staging_buffer_alloc_info = { 0 };
    staging_buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    res = vmaCreateBuffer(rhi.allocator, &staging_buffer_info, &staging_buffer_alloc_info, &staging_buffer, &staging_buffer_allocation, NULL);
    assert(res == VK_SUCCESS);

    void* upload_data;
    vmaMapMemory(rhi.allocator, staging_buffer_allocation, &upload_data);
    memcpy(upload_data, data, image_size);
    vmaUnmapMemory(rhi.allocator, staging_buffer_allocation);

    VkBufferImageCopy image_copy_region = {0};
    image_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy_region.imageSubresource.mipLevel = 0;
    image_copy_region.imageSubresource.baseArrayLayer = 0;
    image_copy_region.imageSubresource.layerCount = 1;
    image_copy_region.imageExtent.width = result->image_extent.width;
    image_copy_region.imageExtent.height = result->image_extent.height;
    image_copy_region.imageExtent.depth = 1;

    VkImageSubresourceRange subresource_range = { 0 };
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseArrayLayer = 0;
    subresource_range.baseMipLevel = 0;
    subresource_range.layerCount = 1;
    subresource_range.levelCount = 1;

    //
    VkCommandBuffer stc = E_Vk_SingleTimeCommands();

    E_Vk_Image_Memory_Barrier(stc, result->image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, subresource_range);
    vkCmdCopyBufferToImage(stc, staging_buffer, result->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy_region);
    E_Vk_Image_Memory_Barrier(stc, result->image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresource_range);
    
    E_Vk_EndSingleTimeCommands(stc);
    //

    vmaDestroyBuffer(rhi.allocator, staging_buffer, staging_buffer_allocation);
    stbi_image_free(data);

    VkImageViewCreateInfo view_info = { 0 };
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = result->image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = result->format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    res = vkCreateImageView(rhi.device.handle, &view_info, NULL, &result->image_view);
    assert(res == VK_SUCCESS);

    return result;
}

E_VulkanImage* E_Vk_MakeHDRImageFromFile(const char* path)
{
    E_VulkanImage* result = malloc(sizeof(E_VulkanImage));

    stbi_set_flip_vertically_on_load(1);
    i32 width = 0, height = 0, channels = 0;
    f32* data = stbi_loadf(path, &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize image_size = width * height * 16; // Force alpha channel

    result->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    result->euphorbe_format = E_ImageFormatRGBA32;
    result->image_extent.width = width;
    result->image_extent.height = height;

    VkImageCreateInfo image_create_info = { 0 };
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.format = result->format;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation = { 0 };
    allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult res = vmaCreateImage(rhi.allocator, &image_create_info, &allocation, &result->image, &result->allocation, &result->allocation_info);
    assert(res == VK_SUCCESS);

    VkSamplerCreateInfo sampler_info = { 0 };
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = rhi.physical_device.handle_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    res = vkCreateSampler(rhi.device.handle, &sampler_info, NULL, &result->sampler);
    assert(res == VK_SUCCESS);

    VkDescriptorSetLayout set_layout = ImGui_ImplVulkan_GetDescriptorSetLayout();
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorSetAllocateInfo alloc_info = { 0 };
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &set_layout;
        alloc_info.descriptorPool = rhi.imgui.descriptor_pool;

        res = vkAllocateDescriptorSets(rhi.device.handle, &alloc_info, &result->gui_descriptor_set[i]);
        assert(res == VK_SUCCESS);
    }

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_buffer_allocation = VK_NULL_HANDLE;
    VmaAllocationInfo staging_buffer_allocation_info = { 0 };

    VkBufferCreateInfo staging_buffer_info = { 0 };
    staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_buffer_info.size = image_size;

    VmaAllocationCreateInfo staging_buffer_alloc_info = { 0 };
    staging_buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    res = vmaCreateBuffer(rhi.allocator, &staging_buffer_info, &staging_buffer_alloc_info, &staging_buffer, &staging_buffer_allocation, NULL);
    assert(res == VK_SUCCESS);

    void* upload_data;
    vmaMapMemory(rhi.allocator, staging_buffer_allocation, &upload_data);
    memcpy(upload_data, data, image_size);
    vmaUnmapMemory(rhi.allocator, staging_buffer_allocation);

    VkBufferImageCopy image_copy_region = { 0 };
    image_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_copy_region.imageSubresource.mipLevel = 0;
    image_copy_region.imageSubresource.baseArrayLayer = 0;
    image_copy_region.imageSubresource.layerCount = 1;
    image_copy_region.imageExtent.width = result->image_extent.width;
    image_copy_region.imageExtent.height = result->image_extent.height;
    image_copy_region.imageExtent.depth = 1;

    VkImageSubresourceRange subresource_range = { 0 };
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseArrayLayer = 0;
    subresource_range.baseMipLevel = 0;
    subresource_range.layerCount = 1;
    subresource_range.levelCount = 1;

    //
    VkCommandBuffer stc = E_Vk_SingleTimeCommands();

    E_Vk_Image_Memory_Barrier(stc, result->image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, subresource_range);
    vkCmdCopyBufferToImage(stc, staging_buffer, result->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy_region);
    E_Vk_Image_Memory_Barrier(stc, result->image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresource_range);

    E_Vk_EndSingleTimeCommands(stc);
    //

    vmaDestroyBuffer(rhi.allocator, staging_buffer, staging_buffer_allocation);
    stbi_image_free(data);

    VkImageViewCreateInfo view_info = { 0 };
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = result->image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = result->format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    res = vkCreateImageView(rhi.device.handle, &view_info, NULL, &result->image_view);
    assert(res == VK_SUCCESS);

    return result;
}

void E_Vk_FreeImage(E_VulkanImage* image)
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        vkFreeDescriptorSets(rhi.device.handle, rhi.imgui.descriptor_pool, 1, &image->gui_descriptor_set[i]);
    vkDestroySampler(rhi.device.handle, image->sampler, NULL);
    vkDestroyImageView(rhi.device.handle, image->image_view, NULL);
    vmaDestroyImage(rhi.allocator, image->image, image->allocation);
    free(image);
}

void E_Vk_ResizeImage(E_VulkanImage* image, i32 width, i32 height)
{
    vkDestroyImageView(rhi.device.handle, image->image_view, NULL);
    vmaDestroyImage(rhi.allocator, image->image, image->allocation);

    VkImageCreateInfo image_create_info = { 0 };
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.format = image->format;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = image->euphorbe_usage;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    image->image_extent = image_create_info.extent;

    VmaAllocationCreateInfo allocation = { 0 };
    allocation.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult res = vmaCreateImage(rhi.allocator, &image_create_info, &allocation, &image->image, &image->allocation, &image->allocation_info);
    assert(res == VK_SUCCESS);

    VkImageViewCreateInfo view_info = { 0 };
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image->image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = image->format;
    view_info.subresourceRange.aspectMask = EuphorbeFormatToVulkanAspect(image->euphorbe_format);
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    res = vkCreateImageView(rhi.device.handle, &view_info, NULL, &image->image_view);
    assert(res == VK_SUCCESS);
}

void E_Vk_BlitImage(E_VulkanImage* src, E_VulkanImage* dst, E_ImageLayout src_layout, E_ImageLayout dst_layout)
{
    VkImageBlit region = {0};
    region.srcOffsets[1].x = src->image_extent.width;
    region.srcOffsets[1].y = src->image_extent.height;
    region.srcOffsets[1].z = 1;
    region.srcSubresource.layerCount = 1;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstOffsets[1].x = dst->image_extent.width;
    region.dstOffsets[1].y = dst->image_extent.height;
    region.dstOffsets[1].z = 1;
    region.dstSubresource.layerCount = 1;
    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdBlitImage(rhi.command.command_buffers[rhi.sync.image_index], src->image, src_layout, dst->image, dst_layout, 1, &region, VK_FILTER_NEAREST);
}

void E_Vk_DrawImageToGUI(E_VulkanImage* image, i32 width, i32 height)
{
    ImVec2 size = { (f32)width, (f32)height };

    ImVec2 uv0 = { 0, 0 };
    ImVec2 uv1 = { 1, 1 };
    ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    ImVec4 border = { 0.0f, 0.0f, 0.0f, 0.0f };

    ImGui_ImplVulkan_AddTexture(image->image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, image->sampler, image->gui_descriptor_set[rhi.sync.image_index]);
    igImage(image->gui_descriptor_set[rhi.sync.image_index], size, uv0, uv1, color, border);
}
