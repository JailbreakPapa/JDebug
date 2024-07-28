#pragma once

#include <Core/World/SettingsComponent.h>
#include <Core/World/SettingsComponentManager.h>
#include <Foundation/Types/SharedPtr.h>
#include <RendererCore/BakedProbes/BakingInterface.h>
#include <RendererCore/Declarations.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct nsMsgUpdateLocalBounds;
struct nsMsgExtractRenderData;
struct nsRenderWorldRenderEvent;
class nsAbstractObjectNode;

class NS_RENDERERCORE_DLL nsBakedProbesComponentManager : public nsSettingsComponentManager<class nsBakedProbesComponent>
{
public:
  nsBakedProbesComponentManager(nsWorld* pWorld);
  ~nsBakedProbesComponentManager();

  virtual void Initialize() override;
  virtual void Deinitialize() override;

  nsMeshResourceHandle m_hDebugSphere;
  nsMaterialResourceHandle m_hDebugMaterial;

private:
  void RenderDebug(const nsWorldModule::UpdateContext& updateContext);
  void OnRenderEvent(const nsRenderWorldRenderEvent& e);
  void CreateDebugResources();
};

class NS_RENDERERCORE_DLL nsBakedProbesComponent : public nsSettingsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsBakedProbesComponent, nsSettingsComponent, nsBakedProbesComponentManager);

public:
  nsBakedProbesComponent();
  ~nsBakedProbesComponent();

  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  nsBakingSettings m_Settings;                                      // [ property ]

  void SetShowDebugOverlay(bool bShow);                             // [ property ]
  bool GetShowDebugOverlay() const { return m_bShowDebugOverlay; }  // [ property ]

  void SetShowDebugProbes(bool bShow);                              // [ property ]
  bool GetShowDebugProbes() const { return m_bShowDebugProbes; }    // [ property ]

  void SetUseTestPosition(bool bUse);                               // [ property ]
  bool GetUseTestPosition() const { return m_bUseTestPosition; }    // [ property ]

  void SetTestPosition(const nsVec3& vPos);                         // [ property ]
  const nsVec3& GetTestPosition() const { return m_vTestPosition; } // [ property ]

  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& ref_msg);
  void OnExtractRenderData(nsMsgExtractRenderData& ref_msg) const;

  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

private:
  void RenderDebugOverlay();
  void OnObjectCreated(const nsAbstractObjectNode& node);

  nsHashedString m_sProbeTreeResourcePrefix;

  bool m_bShowDebugOverlay = false;
  bool m_bShowDebugProbes = false;
  bool m_bUseTestPosition = false;
  nsVec3 m_vTestPosition = nsVec3::MakeZero();

  struct RenderDebugViewTask;
  nsSharedPtr<RenderDebugViewTask> m_pRenderDebugViewTask;

  nsGALTextureHandle m_hDebugViewTexture;
};
