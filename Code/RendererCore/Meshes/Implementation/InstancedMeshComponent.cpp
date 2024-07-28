#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Meshes/InstancedMeshComponent.h>
#include <RendererCore/Pipeline/InstanceDataProvider.h>
#include <RendererCore/Utils/WorldGeoExtractionUtil.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/../../../Data/Base/Shaders/Common/ObjectConstants.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsMeshInstanceData, nsNoBase, 1, nsRTTIDefaultAllocator<nsMeshInstanceData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("LocalPosition", GetLocalPosition, SetLocalPosition)->AddAttributes(new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("LocalRotation", GetLocalRotation, SetLocalRotation),
    NS_ACCESSOR_PROPERTY("LocalScaling", GetLocalScaling, SetLocalScaling)->AddAttributes(new nsDefaultValueAttribute(nsVec3(1.0f, 1.0f, 1.0f))),

    NS_MEMBER_PROPERTY("Color", m_color)
  }
  NS_END_PROPERTIES;

  NS_BEGIN_ATTRIBUTES
  {
    new nsTransformManipulatorAttribute("LocalPosition", "LocalRotation", "LocalScaling"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE
// clang-format on

void nsMeshInstanceData::SetLocalPosition(nsVec3 vPosition)
{
  m_transform.m_vPosition = vPosition;
}
nsVec3 nsMeshInstanceData::GetLocalPosition() const
{
  return m_transform.m_vPosition;
}

void nsMeshInstanceData::SetLocalRotation(nsQuat qRotation)
{
  m_transform.m_qRotation = qRotation;
}

nsQuat nsMeshInstanceData::GetLocalRotation() const
{
  return m_transform.m_qRotation;
}

void nsMeshInstanceData::SetLocalScaling(nsVec3 vScaling)
{
  m_transform.m_vScale = vScaling;
}

nsVec3 nsMeshInstanceData::GetLocalScaling() const
{
  return m_transform.m_vScale;
}

static constexpr nsTypeVersion s_MeshInstanceDataVersion = 1;
nsResult nsMeshInstanceData::Serialize(nsStreamWriter& ref_writer) const
{
  ref_writer.WriteVersion(s_MeshInstanceDataVersion);

  ref_writer << m_transform;
  ref_writer << m_color;

  return NS_SUCCESS;
}

nsResult nsMeshInstanceData::Deserialize(nsStreamReader& ref_reader)
{
  /*auto version = */ ref_reader.ReadVersion(s_MeshInstanceDataVersion);

  ref_reader >> m_transform;
  ref_reader >> m_color;

  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInstancedMeshRenderData, 1, nsRTTIDefaultAllocator<nsInstancedMeshRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsInstancedMeshRenderData::FillBatchIdAndSortingKey()
{
  FillBatchIdAndSortingKeyInternal(m_pExplicitInstanceData->m_hInstanceDataBuffer.GetInternalID().m_Data);
}

//////////////////////////////////////////////////////////////////////////////////////

nsInstancedMeshComponentManager::nsInstancedMeshComponentManager(nsWorld* pWorld)
  : SUPER(pWorld)
{
}

void nsInstancedMeshComponentManager::EnqueueUpdate(const nsInstancedMeshComponent* pComponent) const
{
  nsUInt64 uiCurrentFrame = nsRenderWorld::GetFrameCounter();
  if (pComponent->m_uiEnqueuedFrame == uiCurrentFrame)
    return;

  NS_LOCK(m_Mutex);
  if (pComponent->m_uiEnqueuedFrame == uiCurrentFrame)
    return;

  auto instanceData = pComponent->GetInstanceData();
  if (instanceData.IsEmpty())
    return;

  m_RequireUpdate.PushBack({pComponent->GetHandle(), instanceData});
  pComponent->m_uiEnqueuedFrame = uiCurrentFrame;
}

void nsInstancedMeshComponentManager::OnRenderEvent(const nsRenderWorldRenderEvent& e)
{
  if (e.m_Type != nsRenderWorldRenderEvent::Type::BeginRender)
    return;

  NS_LOCK(m_Mutex);

  if (m_RequireUpdate.IsEmpty())
    return;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsGALPass* pGALPass = pDevice->BeginPass("Update Instanced Mesh Data");

  nsRenderContext* pRenderContext = nsRenderContext::GetDefaultInstance();
  pRenderContext->BeginCompute(pGALPass);

  for (const auto& componentToUpdate : m_RequireUpdate)
  {
    nsInstancedMeshComponent* pComp = nullptr;
    if (!TryGetComponent(componentToUpdate.m_hComponent, pComp))
      continue;

    if (pComp->m_pExplicitInstanceData)
    {
      nsUInt32 uiOffset = 0;
      auto instanceData = pComp->m_pExplicitInstanceData->GetInstanceData(componentToUpdate.m_InstanceData.GetCount(), uiOffset);
      instanceData.CopyFrom(componentToUpdate.m_InstanceData);

      pComp->m_pExplicitInstanceData->UpdateInstanceData(pRenderContext, instanceData.GetCount());
    }
  }

  pRenderContext->EndCompute();
  pDevice->EndPass(pGALPass);

  m_RequireUpdate.Clear();
}

void nsInstancedMeshComponentManager::Initialize()
{
  SUPER::Initialize();

  nsRenderWorld::GetRenderEvent().AddEventHandler(nsMakeDelegate(&nsInstancedMeshComponentManager::OnRenderEvent, this));
}

void nsInstancedMeshComponentManager::Deinitialize()
{
  nsRenderWorld::GetRenderEvent().RemoveEventHandler(nsMakeDelegate(&nsInstancedMeshComponentManager::OnRenderEvent, this));

  SUPER::Deinitialize();
}

//////////////////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsInstancedMeshComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Mesh", GetMeshFile, SetMeshFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    NS_ACCESSOR_PROPERTY("MainColor", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ARRAY_ACCESSOR_PROPERTY("Materials", Materials_GetCount, Materials_GetValue, Materials_SetValue, Materials_Insert, Materials_Remove)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),

    NS_ARRAY_ACCESSOR_PROPERTY("InstanceData", Instances_GetCount, Instances_GetValue, Instances_SetValue, Instances_Insert, Instances_Remove),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractGeometry, OnMsgExtractGeometry),
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsInstancedMeshComponent::nsInstancedMeshComponent() = default;
nsInstancedMeshComponent::~nsInstancedMeshComponent() = default;

void nsInstancedMeshComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  inout_stream.GetStream().WriteArray(m_RawInstancedData).IgnoreResult();
}

void nsInstancedMeshComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);

  inout_stream.GetStream().ReadArray(m_RawInstancedData).IgnoreResult();
}

void nsInstancedMeshComponent::OnActivated()
{
  SUPER::OnActivated();

  NS_ASSERT_DEV(m_pExplicitInstanceData == nullptr, "Instance data must not be initialized at this point");
  m_pExplicitInstanceData = NS_DEFAULT_NEW(nsInstanceData);
}

void nsInstancedMeshComponent::OnDeactivated()
{
  NS_DEFAULT_DELETE(m_pExplicitInstanceData);
  m_pExplicitInstanceData = nullptr;

  SUPER::OnDeactivated();
}

void nsInstancedMeshComponent::OnMsgExtractGeometry(nsMsgExtractGeometry& ref_msg) {}

nsResult nsInstancedMeshComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  nsBoundingBoxSphere singleBounds;
  if (m_hMesh.IsValid())
  {
    nsResourceLock<nsMeshResource> pMesh(m_hMesh, nsResourceAcquireMode::AllowLoadingFallback);
    singleBounds = pMesh->GetBounds();

    for (const auto& instance : m_RawInstancedData)
    {
      auto instanceBounds = singleBounds;
      instanceBounds.Transform(instance.m_transform.GetAsMat4());

      ref_bounds.ExpandToInclude(instanceBounds);
    }

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsInstancedMeshComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (!m_hMesh.IsValid())
    return;

  SUPER::OnMsgExtractRenderData(msg);

  static_cast<const nsInstancedMeshComponentManager*>(GetOwningManager())->EnqueueUpdate(this);
}

nsMeshRenderData* nsInstancedMeshComponent::CreateRenderData() const
{
  auto pRenderData = nsCreateRenderDataForThisFrame<nsInstancedMeshRenderData>(GetOwner());

  if (m_pExplicitInstanceData)
  {
    pRenderData->m_pExplicitInstanceData = m_pExplicitInstanceData;
    pRenderData->m_uiExplicitInstanceCount = m_RawInstancedData.GetCount();
  }

  return pRenderData;
}

nsUInt32 nsInstancedMeshComponent::Instances_GetCount() const
{
  return m_RawInstancedData.GetCount();
}

nsMeshInstanceData nsInstancedMeshComponent::Instances_GetValue(nsUInt32 uiIndex) const
{
  return m_RawInstancedData[uiIndex];
}

void nsInstancedMeshComponent::Instances_SetValue(nsUInt32 uiIndex, nsMeshInstanceData value)
{
  m_RawInstancedData[uiIndex] = value;

  TriggerLocalBoundsUpdate();
}

void nsInstancedMeshComponent::Instances_Insert(nsUInt32 uiIndex, nsMeshInstanceData value)
{
  m_RawInstancedData.InsertAt(uiIndex, value);

  TriggerLocalBoundsUpdate();
}

void nsInstancedMeshComponent::Instances_Remove(nsUInt32 uiIndex)
{
  m_RawInstancedData.RemoveAtAndCopy(uiIndex);

  TriggerLocalBoundsUpdate();
}

nsArrayPtr<nsPerInstanceData> nsInstancedMeshComponent::GetInstanceData() const
{
  if (!m_pExplicitInstanceData || m_RawInstancedData.IsEmpty())
    return nsArrayPtr<nsPerInstanceData>();

  auto instanceData = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsPerInstanceData, m_RawInstancedData.GetCount());

  const nsTransform ownerTransform = GetOwner()->GetGlobalTransform();

  float fBoundingSphereRadius = 1.0f;

  if (m_hMesh.IsValid())
  {
    nsResourceLock<nsMeshResource> pMesh(m_hMesh, nsResourceAcquireMode::AllowLoadingFallback);
    fBoundingSphereRadius = pMesh->GetBounds().GetSphere().m_fRadius;
  }

  for (nsUInt32 i = 0; i < m_RawInstancedData.GetCount(); ++i)
  {
    const nsTransform globalTransform = ownerTransform * m_RawInstancedData[i].m_transform;
    const nsMat4 objectToWorld = globalTransform.GetAsMat4();

    instanceData[i].ObjectToWorld = objectToWorld;

    if (m_RawInstancedData[i].m_transform.ContainsUniformScale())
    {
      instanceData[i].ObjectToWorldNormal = objectToWorld;
    }
    else
    {
      nsMat3 mInverse = objectToWorld.GetRotationalPart();
      mInverse.Invert(0.0f).IgnoreResult();
      // we explicitly ignore the return value here (success / failure)
      // because when we have a scale of 0 (which happens temporarily during editing) that would be annoying

      nsShaderTransform shaderT;
      shaderT = mInverse.GetTranspose();
      instanceData[i].ObjectToWorldNormal = shaderT;
    }

    instanceData[i].GameObjectID = GetUniqueIdForRendering();
    instanceData[i].BoundingSphereRadius = fBoundingSphereRadius * m_RawInstancedData[i].m_transform.GetMaxScale();

    instanceData[i].Color = m_Color * m_RawInstancedData[i].m_color;
    instanceData[i].CustomData.SetZero(); // unused
  }

  return instanceData;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_InstancedMeshComponent);
