
#pragma once

#include <RendererFoundation/Resources/RenderTargetView.h>

struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11UnorderedAccessView;

class nsGALRenderTargetViewVulkan : public nsGALRenderTargetView
{
public:
  vk::ImageView GetImageView() const;
  bool IsFullRange() const;
  vk::ImageSubresourceRange GetRange() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALRenderTargetViewVulkan(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& Description);
  virtual ~nsGALRenderTargetViewVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::ImageView m_imageView;
  bool m_bfullRange = false;
  vk::ImageSubresourceRange m_range;
};

#include <RendererVulkan/Resources/Implementation/RenderTargetViewVulkan_inl.h>
