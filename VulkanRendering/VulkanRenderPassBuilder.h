/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;
	class VulkanTexture;
	class RenderPassBuilder	{
	public:
		RenderPassBuilder(vk::Device sourceDevice);
		~RenderPassBuilder() {}

		RenderPassBuilder& WithColourAttachment(
			VulkanTexture* texture, 
			bool clear	= true,
			vk::ImageLayout startLayout = vk::ImageLayout::eColorAttachmentOptimal, 
			vk::ImageLayout useLayout	= vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout endLayout	= vk::ImageLayout::eColorAttachmentOptimal
		);

		RenderPassBuilder& WithDepthAttachment(
			VulkanTexture* texture, 
			bool clear	= true,
			vk::ImageLayout startLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			vk::ImageLayout useLayout	= vk::ImageLayout::eDepthAttachmentOptimal,
			vk::ImageLayout endLayout	= vk::ImageLayout::eDepthAttachmentOptimal
		);

		RenderPassBuilder& WithDepthStencilAttachment(
			VulkanTexture* texture,
			bool clear = true,
			vk::ImageLayout startLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
			vk::ImageLayout useLayout	= vk::ImageLayout::eDepthStencilAttachmentOptimal,
			vk::ImageLayout endLayout	= vk::ImageLayout::eDepthStencilAttachmentOptimal
		);

		vk::UniqueRenderPass Build(const std::string& name = "");

	protected:
		std::vector<vk::AttachmentDescription>	allDescriptions;
		std::vector<vk::AttachmentReference>		allReferences;
		vk::AttachmentReference depthReference;
		vk::SubpassDescription	subPass;
		vk::Device sourceDevice;
	};
}