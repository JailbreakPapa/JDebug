#pragma once

#include <Core/World/World.h>
#include <RendererCore/RendererCoreDLL.h>

struct nsMsgUpdateLocalBounds;

using nsBakedProbesVolumeComponentManager = nsComponentManager<class nsBakedProbesVolumeComponent, nsBlockStorageType::Compact>;

class NS_RENDERERCORE_DLL nsBakedProbesVolumeComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsBakedProbesVolumeComponent, nsComponent, nsBakedProbesVolumeComponentManager);

public:
  nsBakedProbesVolumeComponent();
  ~nsBakedProbesVolumeComponent();

  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  const nsVec3& GetExtents() const { return m_vExtents; }
  void SetExtents(const nsVec3& vExtents);

  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& ref_msg) const;

private:
  nsVec3 m_vExtents = nsVec3(10.0f);
};
