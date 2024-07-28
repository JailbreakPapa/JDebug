#pragma once

#include <Core/World/World.h>
#include <RendererCore/Meshes/MeshComponent.h>

typedef nsComponentManager<class nsAtmosphericScatteringComponent, nsBlockStorageType::Compact> nsAtmosphericScatteringComponentManager;

class NS_RENDERERCORE_DLL nsAtmosphericScatteringComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsAtmosphericScatteringComponent, nsRenderComponent, nsAtmosphericScatteringComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void Initialize() override;
  virtual void OnActivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;


  //////////////////////////////////////////////////////////////////////////
  // nsAtmosphericScatteringComponent

public:
  nsAtmosphericScatteringComponent();
  ~nsAtmosphericScatteringComponent();

private:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  void UpdateMaterials();

  nsMeshResourceHandle m_hMesh;
  nsMaterialResourceHandle m_hMaterial;
};