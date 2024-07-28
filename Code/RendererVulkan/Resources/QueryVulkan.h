#pragma once

#include <RendererFoundation/Resources/Query.h>

class nsGALQueryVulkan : public nsGALQuery
{
public:
  NS_ALWAYS_INLINE nsUInt32 GetID() const;
  NS_ALWAYS_INLINE vk::QueryPool GetPool() const { return nullptr; } // TODO

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALQueryVulkan(const nsGALQueryCreationDescription& Description);
  ~nsGALQueryVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  virtual void SetDebugNamePlatform(const char* szName) const override;

  nsUInt32 m_uiID;
};

#include <RendererVulkan/Resources/Implementation/QueryVulkan_inl.h>
