#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Utilities/Progress.h>
#include <RendererCore/BakedProbes/BakedProbesComponent.h>
#include <RendererCore/BakedProbes/BakedProbesWorldModule.h>
#include <RendererCore/BakedProbes/ProbeTreeSectorResource.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/Pass.h>
#include <RendererFoundation/Resources/Texture.h>

struct nsBakedProbesComponent::RenderDebugViewTask : public nsTask
{
  RenderDebugViewTask()
  {
    ConfigureTask("BakingDebugView", nsTaskNesting::Never);
  }

  virtual void Execute() override
  {
    NS_ASSERT_DEV(m_PixelData.GetCount() == m_uiWidth * m_uiHeight, "Pixel data must be pre-allocated");

    nsProgress progress;
    progress.m_Events.AddEventHandler([this](const nsProgressEvent& e)
      {
      if (e.m_Type != nsProgressEvent::Type::CancelClicked)
      {
        if (HasBeenCanceled())
        {
          e.m_pProgressbar->UserClickedCancel();
        }
        m_bHasNewData = true;
      } });

    if (m_pBakingInterface->RenderDebugView(*m_pWorld, m_InverseViewProjection, m_uiWidth, m_uiHeight, m_PixelData, progress).Succeeded())
    {
      m_bHasNewData = true;
    }
  }

  nsBakingInterface* m_pBakingInterface = nullptr;

  const nsWorld* m_pWorld = nullptr;
  nsMat4 m_InverseViewProjection = nsMat4::MakeIdentity();
  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;
  nsDynamicArray<nsColorGammaUB> m_PixelData;

  bool m_bHasNewData = false;
};

//////////////////////////////////////////////////////////////////////////


nsBakedProbesComponentManager::nsBakedProbesComponentManager(nsWorld* pWorld)
  : nsSettingsComponentManager<nsBakedProbesComponent>(pWorld)
{
}

nsBakedProbesComponentManager::~nsBakedProbesComponentManager() = default;

void nsBakedProbesComponentManager::Initialize()
{
  {
    auto desc = NS_CREATE_MODULE_UPDATE_FUNCTION_DESC(nsBakedProbesComponentManager::RenderDebug, this);

    this->RegisterUpdateFunction(desc);
  }

  nsRenderWorld::GetRenderEvent().AddEventHandler(nsMakeDelegate(&nsBakedProbesComponentManager::OnRenderEvent, this));

  CreateDebugResources();
}

void nsBakedProbesComponentManager::Deinitialize()
{
  nsRenderWorld::GetRenderEvent().RemoveEventHandler(nsMakeDelegate(&nsBakedProbesComponentManager::OnRenderEvent, this));
}

void nsBakedProbesComponentManager::RenderDebug(const nsWorldModule::UpdateContext& updateContext)
{
  if (nsBakedProbesComponent* pComponent = GetSingletonComponent())
  {
    if (pComponent->GetShowDebugOverlay())
    {
      pComponent->RenderDebugOverlay();
    }
  }
}

void nsBakedProbesComponentManager::OnRenderEvent(const nsRenderWorldRenderEvent& e)
{
  if (e.m_Type != nsRenderWorldRenderEvent::Type::BeginRender)
    return;

  if (nsBakedProbesComponent* pComponent = GetSingletonComponent())
  {
    auto& task = pComponent->m_pRenderDebugViewTask;
    if (task != nullptr && task->m_bHasNewData)
    {
      task->m_bHasNewData = false;

      nsGALDevice* pGALDevice = nsGALDevice::GetDefaultDevice();
      nsGALPass* pGALPass = pGALDevice->BeginPass("BakingDebugViewUpdate");
      auto pCommandEncoder = pGALPass->BeginCompute();

      nsBoundingBoxu32 destBox;
      destBox.m_vMin.SetZero();
      destBox.m_vMax = nsVec3U32(task->m_uiWidth, task->m_uiHeight, 1);

      nsGALSystemMemoryDescription sourceData;
      sourceData.m_pData = task->m_PixelData.GetData();
      sourceData.m_uiRowPitch = task->m_uiWidth * sizeof(nsColorGammaUB);

      pCommandEncoder->UpdateTexture(pComponent->m_hDebugViewTexture, nsGALTextureSubresource(), destBox, sourceData);

      pGALPass->EndCompute(pCommandEncoder);
      pGALDevice->EndPass(pGALPass);
    }
  }
}

void nsBakedProbesComponentManager::CreateDebugResources()
{
  if (!m_hDebugSphere.IsValid())
  {
    nsGeometry geom;
    geom.AddStackedSphere(0.3f, 32, 16);

    const char* szBufferResourceName = "IrradianceProbeDebugSphereBuffer";
    nsMeshBufferResourceHandle hMeshBuffer = nsResourceManager::GetExistingResource<nsMeshBufferResource>(szBufferResourceName);
    if (!hMeshBuffer.IsValid())
    {
      nsMeshBufferResourceDescriptor desc;
      desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
      desc.AddStream(nsGALVertexAttributeSemantic::Normal, nsGALResourceFormat::XYZFloat);
      desc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

      hMeshBuffer = nsResourceManager::GetOrCreateResource<nsMeshBufferResource>(szBufferResourceName, std::move(desc), szBufferResourceName);
    }

    const char* szMeshResourceName = "IrradianceProbeDebugSphere";
    m_hDebugSphere = nsResourceManager::GetExistingResource<nsMeshResource>(szMeshResourceName);
    if (!m_hDebugSphere.IsValid())
    {
      nsMeshResourceDescriptor desc;
      desc.UseExistingMeshBuffer(hMeshBuffer);
      desc.AddSubMesh(geom.CalculateTriangleCount(), 0, 0);
      desc.ComputeBounds();

      m_hDebugSphere = nsResourceManager::GetOrCreateResource<nsMeshResource>(szMeshResourceName, std::move(desc), szMeshResourceName);
    }
  }

  if (!m_hDebugMaterial.IsValid())
  {
    m_hDebugMaterial = nsResourceManager::LoadResource<nsMaterialResource>(
      "{ 4d15c716-a8e9-43d4-9424-43174403fb94 }"); // IrradianceProbeVisualization.nsMaterialAsset
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsBakedProbesComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Settings", m_Settings),
    NS_ACCESSOR_PROPERTY("ShowDebugOverlay", GetShowDebugOverlay, SetShowDebugOverlay)->AddAttributes(new nsGroupAttribute("Debug")),
    NS_ACCESSOR_PROPERTY("ShowDebugProbes", GetShowDebugProbes, SetShowDebugProbes),
    NS_ACCESSOR_PROPERTY("UseTestPosition", GetUseTestPosition, SetUseTestPosition),
    NS_ACCESSOR_PROPERTY("TestPosition", GetTestPosition, SetTestPosition)
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_FUNCTIONS
  {
    NS_FUNCTION_PROPERTY(OnObjectCreated),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Lighting/Baking"),
    new nsLongOpAttribute("nsLongOpProxy_BakeScene"),
    new nsTransformManipulatorAttribute("TestPosition"),
    new nsInDevelopmentAttribute(nsInDevelopmentAttribute::Phase::Beta),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsBakedProbesComponent::nsBakedProbesComponent() = default;
nsBakedProbesComponent::~nsBakedProbesComponent() = default;

void nsBakedProbesComponent::OnActivated()
{
  auto pModule = GetWorld()->GetOrCreateModule<nsBakedProbesWorldModule>();
  pModule->SetProbeTreeResourcePrefix(m_sProbeTreeResourcePrefix);

  GetOwner()->UpdateLocalBounds();

  SUPER::OnActivated();
}

void nsBakedProbesComponent::OnDeactivated()
{
  if (m_pRenderDebugViewTask != nullptr)
  {
    nsTaskSystem::CancelTask(m_pRenderDebugViewTask).IgnoreResult();
  }

  GetOwner()->UpdateLocalBounds();

  SUPER::OnDeactivated();
}

void nsBakedProbesComponent::SetShowDebugOverlay(bool bShow)
{
  m_bShowDebugOverlay = bShow;

  if (bShow && m_pRenderDebugViewTask == nullptr)
  {
    m_pRenderDebugViewTask = NS_DEFAULT_NEW(RenderDebugViewTask);
  }
}

void nsBakedProbesComponent::SetShowDebugProbes(bool bShow)
{
  if (m_bShowDebugProbes != bShow)
  {
    m_bShowDebugProbes = bShow;

    if (IsActiveAndInitialized())
    {
      nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
    }
  }
}

void nsBakedProbesComponent::SetUseTestPosition(bool bUse)
{
  if (m_bUseTestPosition != bUse)
  {
    m_bUseTestPosition = bUse;

    if (IsActiveAndInitialized())
    {
      nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
    }
  }
}

void nsBakedProbesComponent::SetTestPosition(const nsVec3& vPos)
{
  m_vTestPosition = vPos;

  if (IsActiveAndInitialized())
  {
    nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
  }
}

void nsBakedProbesComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& ref_msg)
{
  ref_msg.SetAlwaysVisible(GetOwner()->IsDynamic() ? nsDefaultSpatialDataCategories::RenderDynamic : nsDefaultSpatialDataCategories::RenderStatic);
}

void nsBakedProbesComponent::OnExtractRenderData(nsMsgExtractRenderData& ref_msg) const
{
  if (!m_bShowDebugProbes)
    return;

  // Don't trigger probe rendering in shadow or reflection views.
  if (ref_msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow ||
      ref_msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Reflection)
    return;

  auto pModule = GetWorld()->GetModule<nsBakedProbesWorldModule>();
  if (!pModule->HasProbeData())
    return;

  const nsGameObject* pOwner = GetOwner();
  auto pManager = static_cast<const nsBakedProbesComponentManager*>(GetOwningManager());

  auto addProbeRenderData = [&](const nsVec3& vPosition, nsCompressedSkyVisibility skyVisibility, nsRenderData::Caching::Enum caching)
  {
    nsTransform transform = nsTransform::MakeIdentity();
    transform.m_vPosition = vPosition;

    nsColor encodedSkyVisibility = nsColor::Black;
    encodedSkyVisibility.r = *reinterpret_cast<const float*>(&skyVisibility);

    nsMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsMeshRenderData>(pOwner);
    {
      pRenderData->m_GlobalTransform = transform;
      pRenderData->m_GlobalBounds = nsBoundingBoxSphere::MakeInvalid();
      pRenderData->m_hMesh = pManager->m_hDebugSphere;
      pRenderData->m_hMaterial = pManager->m_hDebugMaterial;
      pRenderData->m_Color = encodedSkyVisibility;
      pRenderData->m_uiSubMeshIndex = 0;
      pRenderData->m_uiUniqueID = nsRenderComponent::GetUniqueIdForRendering(*this, 0);

      pRenderData->FillBatchIdAndSortingKey();
    }

    ref_msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::SimpleOpaque, caching);
  };

  if (m_bUseTestPosition)
  {
    nsBakedProbesWorldModule::ProbeIndexData indexData;
    if (pModule->GetProbeIndexData(m_vTestPosition, nsVec3::MakeAxisZ(), indexData).Failed())
      return;

    if (true)
    {
      nsResourceLock<nsProbeTreeSectorResource> pProbeTree(pModule->m_hProbeTree, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pProbeTree.GetAcquireResult() != nsResourceAcquireResult::Final)
        return;

      for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(indexData.m_probeIndices); ++i)
      {
        nsVec3 pos = pProbeTree->GetProbePositions()[indexData.m_probeIndices[i]];
        nsDebugRenderer::DrawCross(ref_msg.m_pView->GetHandle(), pos, 0.5f, nsColor::Yellow);

        pos.z += 0.5f;
        nsDebugRenderer::Draw3DText(ref_msg.m_pView->GetHandle(), nsFmt("Weight: {}", indexData.m_probeWeights[i]), pos, nsColor::Yellow);
      }
    }

    nsCompressedSkyVisibility skyVisibility = nsBakingUtils::CompressSkyVisibility(pModule->GetSkyVisibility(indexData));

    addProbeRenderData(m_vTestPosition, skyVisibility, nsRenderData::Caching::Never);
  }
  else
  {
    nsResourceLock<nsProbeTreeSectorResource> pProbeTree(pModule->m_hProbeTree, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (pProbeTree.GetAcquireResult() != nsResourceAcquireResult::Final)
      return;

    auto probePositions = pProbeTree->GetProbePositions();
    auto skyVisibility = pProbeTree->GetSkyVisibility();

    for (nsUInt32 uiProbeIndex = 0; uiProbeIndex < probePositions.GetCount(); ++uiProbeIndex)
    {
      addProbeRenderData(probePositions[uiProbeIndex], skyVisibility[uiProbeIndex], nsRenderData::Caching::IfStatic);
    }
  }
}

void nsBakedProbesComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  if (m_Settings.Serialize(s).Failed())
    return;

  s << m_sProbeTreeResourcePrefix;
  s << m_bShowDebugOverlay;
  s << m_bShowDebugProbes;
  s << m_bUseTestPosition;
  s << m_vTestPosition;
}

void nsBakedProbesComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  if (m_Settings.Deserialize(s).Failed())
    return;

  s >> m_sProbeTreeResourcePrefix;
  s >> m_bShowDebugOverlay;
  s >> m_bShowDebugProbes;
  s >> m_bUseTestPosition;
  s >> m_vTestPosition;
}

void nsBakedProbesComponent::RenderDebugOverlay()
{
  nsView* pView = nsRenderWorld::GetViewByUsageHint(nsCameraUsageHint::MainView, nsCameraUsageHint::EditorView);
  if (pView == nullptr)
    return;

  nsBakingInterface* pBakingInterface = nsSingletonRegistry::GetSingletonInstance<nsBakingInterface>();
  if (pBakingInterface == nullptr)
  {
    nsDebugRenderer::Draw2DText(pView->GetHandle(), "Baking Plugin not loaded", nsVec2I32(10, 10), nsColor::OrangeRed);
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsRectFloat viewport = pView->GetViewport();
  nsUInt32 uiWidth = static_cast<nsUInt32>(nsMath::Ceil(viewport.width / 3.0f));
  nsUInt32 uiHeight = static_cast<nsUInt32>(nsMath::Ceil(viewport.height / 3.0f));

  nsMat4 inverseViewProjection = pView->GetInverseViewProjectionMatrix(nsCameraEye::Left);

  if (m_pRenderDebugViewTask->m_InverseViewProjection != inverseViewProjection ||
      m_pRenderDebugViewTask->m_uiWidth != uiWidth || m_pRenderDebugViewTask->m_uiHeight != uiHeight)
  {
    nsTaskSystem::CancelTask(m_pRenderDebugViewTask).IgnoreResult();

    m_pRenderDebugViewTask->m_pBakingInterface = pBakingInterface;
    m_pRenderDebugViewTask->m_pWorld = GetWorld();
    m_pRenderDebugViewTask->m_InverseViewProjection = inverseViewProjection;
    m_pRenderDebugViewTask->m_uiWidth = uiWidth;
    m_pRenderDebugViewTask->m_uiHeight = uiHeight;
    m_pRenderDebugViewTask->m_PixelData.SetCount(uiWidth * uiHeight, nsColor::Red);
    m_pRenderDebugViewTask->m_bHasNewData = false;

    nsTaskSystem::StartSingleTask(m_pRenderDebugViewTask, nsTaskPriority::LongRunning);
  }

  nsUInt32 uiTextureWidth = 0;
  nsUInt32 uiTextureHeight = 0;
  if (const nsGALTexture* pTexture = pDevice->GetTexture(m_hDebugViewTexture))
  {
    uiTextureWidth = pTexture->GetDescription().m_uiWidth;
    uiTextureHeight = pTexture->GetDescription().m_uiHeight;
  }

  if (uiTextureWidth != uiWidth || uiTextureHeight != uiHeight)
  {
    if (!m_hDebugViewTexture.IsInvalidated())
    {
      pDevice->DestroyTexture(m_hDebugViewTexture);
    }

    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = uiWidth;
    desc.m_uiHeight = uiHeight;
    desc.m_Format = nsGALResourceFormat::RGBAUByteNormalizedsRGB;
    desc.m_ResourceAccess.m_bImmutable = false;

    m_hDebugViewTexture = pDevice->CreateTexture(desc);
  }

  nsRectFloat rectInPixel = nsRectFloat(10.0f, 10.0f, static_cast<float>(uiWidth), static_cast<float>(uiHeight));

  nsDebugRenderer::Draw2DRectangle(pView->GetHandle(), rectInPixel, 0.0f, nsColor::White, pDevice->GetDefaultResourceView(m_hDebugViewTexture));
}

void nsBakedProbesComponent::OnObjectCreated(const nsAbstractObjectNode& node)
{
  nsStringBuilder sPrefix;
  sPrefix.SetFormat(":project/AssetCache/Generated/{0}", node.GetGuid());

  m_sProbeTreeResourcePrefix.Assign(sPrefix);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_BakedProbes_Implementation_BakedProbesComponent);
