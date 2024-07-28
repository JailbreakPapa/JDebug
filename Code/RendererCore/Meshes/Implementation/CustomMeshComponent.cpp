#include <RendererCore/RendererCorePCH.h>

#include <../../Data/Base/Shaders/Common/ObjectConstants.h>
#include <Core/Graphics/Geometry.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Meshes/CustomMeshComponent.h>
#include <RendererCore/Meshes/DynamicMeshBufferResource.h>
#include <RendererCore/Pipeline/InstanceDataProvider.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsCustomMeshComponent, 3, nsComponentMode::Static)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("CustomData", GetCustomData, SetCustomData)->AddAttributes(new nsDefaultValueAttribute(nsVec4(0, 1, 0, 1))),
    NS_ACCESSOR_PROPERTY("Material", GetMaterialFile, SetMaterialFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgSetMeshMaterial, OnMsgSetMeshMaterial),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
    NS_MESSAGE_HANDLER(nsMsgSetCustomData, OnMsgSetCustomData),
  } NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsAtomicInteger32 s_iCustomMeshResources;

nsCustomMeshComponent::nsCustomMeshComponent()
{
  m_Bounds = nsBoundingBoxSphere::MakeInvalid();
}

nsCustomMeshComponent::~nsCustomMeshComponent() = default;

void nsCustomMeshComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_Color;
  s << m_hMaterial;

  s << m_vCustomData;
}

void nsCustomMeshComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_Color;
  s >> m_hMaterial;

  if (uiVersion < 2)
  {
    nsUInt32 uiCategory = 0;
    s >> uiCategory;
  }

  if (uiVersion >= 3)
  {
    s >> m_vCustomData;
  }
}

nsResult nsCustomMeshComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  if (m_Bounds.IsValid())
  {
    ref_bounds = m_Bounds;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsDynamicMeshBufferResourceHandle nsCustomMeshComponent::CreateMeshResource(nsGALPrimitiveTopology::Enum topology, nsUInt32 uiMaxVertices, nsUInt32 uiMaxPrimitives, nsGALIndexType::Enum indexType)
{
  nsDynamicMeshBufferResourceDescriptor desc;
  desc.m_Topology = topology;
  desc.m_uiMaxVertices = uiMaxVertices;
  desc.m_uiMaxPrimitives = uiMaxPrimitives;
  desc.m_IndexType = indexType;
  desc.m_bColorStream = true;

  nsStringBuilder sGuid;
  sGuid.SetFormat("CustomMesh_{}", s_iCustomMeshResources.Increment());

  m_hDynamicMesh = nsResourceManager::CreateResource<nsDynamicMeshBufferResource>(sGuid, std::move(desc));

  InvalidateCachedRenderData();

  return m_hDynamicMesh;
}

void nsCustomMeshComponent::SetMeshResource(const nsDynamicMeshBufferResourceHandle& hMesh)
{
  m_hDynamicMesh = hMesh;
  InvalidateCachedRenderData();
}

void nsCustomMeshComponent::SetBounds(const nsBoundingBoxSphere& bounds)
{
  m_Bounds = bounds;
  TriggerLocalBoundsUpdate();
}

void nsCustomMeshComponent::SetMaterial(const nsMaterialResourceHandle& hMaterial)
{
  m_hMaterial = hMaterial;
  InvalidateCachedRenderData();
}

nsMaterialResourceHandle nsCustomMeshComponent::GetMaterial() const
{
  return m_hMaterial;
}

void nsCustomMeshComponent::SetMaterialFile(const char* szMaterial)
{
  nsMaterialResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szMaterial))
  {
    hResource = nsResourceManager::LoadResource<nsMaterialResource>(szMaterial);
  }

  m_hMaterial = hResource;
}

const char* nsCustomMeshComponent::GetMaterialFile() const
{
  if (!m_hMaterial.IsValid())
    return "";

  return m_hMaterial.GetResourceID();
}

void nsCustomMeshComponent::SetColor(const nsColor& color)
{
  m_Color = color;

  InvalidateCachedRenderData();
}

const nsColor& nsCustomMeshComponent::GetColor() const
{
  return m_Color;
}

void nsCustomMeshComponent::SetCustomData(const nsVec4& vData)
{
  m_vCustomData = vData;

  InvalidateCachedRenderData();
}

const nsVec4& nsCustomMeshComponent::GetCustomData() const
{
  return m_vCustomData;
}

void nsCustomMeshComponent::OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg)
{
  SetMaterial(ref_msg.m_hMaterial);
}

void nsCustomMeshComponent::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_Color);

  InvalidateCachedRenderData();
}

void nsCustomMeshComponent::OnMsgSetCustomData(nsMsgSetCustomData& ref_msg)
{
  m_vCustomData = ref_msg.m_vData;

  InvalidateCachedRenderData();
}

void nsCustomMeshComponent::SetUsePrimitiveRange(nsUInt32 uiFirstPrimitive /*= 0*/, nsUInt32 uiNumPrimitives /*= nsMath::MaxValue<nsUInt32>()*/)
{
  m_uiFirstPrimitive = uiFirstPrimitive;
  m_uiNumPrimitives = uiNumPrimitives;
}

void nsCustomMeshComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (!m_hDynamicMesh.IsValid() || !m_hMaterial.IsValid())
    return;

  nsResourceLock<nsDynamicMeshBufferResource> pMesh(m_hDynamicMesh, nsResourceAcquireMode::BlockTillLoaded);

  nsCustomMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsCustomMeshRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hMesh = m_hDynamicMesh;
    pRenderData->m_hMaterial = m_hMaterial;
    pRenderData->m_Color = m_Color;
    pRenderData->m_vCustomData = m_vCustomData;
    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();
    pRenderData->m_uiFirstPrimitive = nsMath::Min(m_uiFirstPrimitive, pMesh->GetDescriptor().m_uiMaxPrimitives);
    pRenderData->m_uiNumPrimitives = nsMath::Min(m_uiNumPrimitives, pMesh->GetDescriptor().m_uiMaxPrimitives - pRenderData->m_uiFirstPrimitive);

    pRenderData->FillBatchIdAndSortingKey();
  }

  nsResourceLock<nsMaterialResource> pMaterial(m_hMaterial, nsResourceAcquireMode::AllowLoadingFallback);
  nsRenderData::Category category = pMaterial->GetRenderDataCategory();
  bool bDontCacheYet = pMaterial.GetAcquireResult() == nsResourceAcquireResult::LoadingFallback;

  msg.AddRenderData(pRenderData, category, bDontCacheYet ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic);
}

void nsCustomMeshComponent::OnActivated()
{
  SUPER::OnActivated();

  if (false)
  {
    nsGeometry geo;
    geo.AddTorus(1.0f, 1.5f, 32, 16, false);
    geo.TriangulatePolygons();
    geo.ComputeTangents();

    auto hMesh = CreateMeshResource(nsGALPrimitiveTopology::Triangles, geo.GetVertices().GetCount(), geo.GetPolygons().GetCount(), nsGALIndexType::UInt);

    nsResourceLock<nsDynamicMeshBufferResource> pMesh(hMesh, nsResourceAcquireMode::BlockTillLoaded);

    auto verts = pMesh->AccessVertexData();
    auto cols = pMesh->AccessColorData();

    for (nsUInt32 v = 0; v < verts.GetCount(); ++v)
    {
      verts[v].m_vPosition = geo.GetVertices()[v].m_vPosition;
      verts[v].m_vTexCoord.SetZero();
      verts[v].EncodeNormal(geo.GetVertices()[v].m_vNormal);
      verts[v].EncodeTangent(geo.GetVertices()[v].m_vTangent, 1.0f);

      cols[v] = nsColor::CornflowerBlue;
    }

    auto ind = pMesh->AccessIndex32Data();

    for (nsUInt32 i = 0; i < geo.GetPolygons().GetCount(); ++i)
    {
      ind[i * 3 + 0] = geo.GetPolygons()[i].m_Vertices[0];
      ind[i * 3 + 1] = geo.GetPolygons()[i].m_Vertices[1];
      ind[i * 3 + 2] = geo.GetPolygons()[i].m_Vertices[2];
    }

    SetBounds(nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), 1.5f));
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCustomMeshRenderData, 1, nsRTTIDefaultAllocator<nsCustomMeshRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


void nsCustomMeshRenderData::FillBatchIdAndSortingKey()
{
  const nsUInt32 uiAdditionalBatchData = 0;

  m_uiFlipWinding = m_GlobalTransform.ContainsNegativeScale() ? 1 : 0;
  m_uiUniformScale = m_GlobalTransform.ContainsUniformScale() ? 1 : 0;

  const nsUInt32 uiMeshIDHash = nsHashingUtils::StringHashTo32(m_hMesh.GetResourceIDHash());
  const nsUInt32 uiMaterialIDHash = m_hMaterial.IsValid() ? nsHashingUtils::StringHashTo32(m_hMaterial.GetResourceIDHash()) : 0;

  // Generate batch id from mesh, material and part index.
  nsUInt32 data[] = {uiMeshIDHash, uiMaterialIDHash, 0 /*m_uiSubMeshIndex*/, m_uiFlipWinding, uiAdditionalBatchData};
  m_uiBatchId = nsHashingUtils::xxHash32(data, sizeof(data));

  // Sort by material and then by mesh
  m_uiSortingKey = (uiMaterialIDHash << 16) | ((uiMeshIDHash + 0 /*m_uiSubMeshIndex*/) & 0xFFFE) | m_uiFlipWinding;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCustomMeshRenderer, 1, nsRTTIDefaultAllocator<nsCustomMeshRenderer>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCustomMeshRenderer::nsCustomMeshRenderer() = default;
nsCustomMeshRenderer::~nsCustomMeshRenderer() = default;

void nsCustomMeshRenderer::GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const
{
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitOpaque);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitMasked);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitTransparent);
  ref_categories.PushBack(nsDefaultRenderDataCategories::Selection);
}

void nsCustomMeshRenderer::GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const
{
  ref_types.PushBack(nsGetStaticRTTI<nsCustomMeshRenderData>());
}

void nsCustomMeshRenderer::RenderBatch(const nsRenderViewContext& renderViewContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const
{
  nsRenderContext* pRenderContext = renderViewContext.m_pRenderContext;
  nsGALCommandEncoder* pGALCommandEncoder = pRenderContext->GetCommandEncoder();

  nsInstanceData* pInstanceData = pPass->GetPipeline()->GetFrameDataProvider<nsInstanceDataProvider>()->GetData(renderViewContext);
  pInstanceData->BindResources(pRenderContext);

  const nsCustomMeshRenderData* pRenderData1st = batch.GetFirstData<nsCustomMeshRenderData>();

  if (pRenderData1st->m_uiFlipWinding)
  {
    pRenderContext->SetShaderPermutationVariable("FLIP_WINDING", "TRUE");
  }
  else
  {
    pRenderContext->SetShaderPermutationVariable("FLIP_WINDING", "FALSE");
  }

  pRenderContext->SetShaderPermutationVariable("VERTEX_SKINNING", "FALSE");

  for (auto it = batch.GetIterator<nsCustomMeshRenderData>(0, batch.GetCount()); it.IsValid(); ++it)
  {
    const nsCustomMeshRenderData* pRenderData = it;

    nsResourceLock<nsDynamicMeshBufferResource> pBuffer(pRenderData->m_hMesh, nsResourceAcquireMode::BlockTillLoaded);

    pRenderContext->BindMaterial(pRenderData->m_hMaterial);

    nsUInt32 uiInstanceDataOffset = 0;
    nsArrayPtr<nsPerInstanceData> instanceData = pInstanceData->GetInstanceData(1, uiInstanceDataOffset);

    instanceData[0].GameObjectID = pRenderData->m_uiUniqueID;
    instanceData[0].Color = pRenderData->m_Color;
    instanceData[0].CustomData = pRenderData->m_vCustomData;
    instanceData[0].ObjectToWorld = pRenderData->m_GlobalTransform;

    if (pRenderData->m_uiUniformScale)
    {
      instanceData[0].ObjectToWorldNormal = instanceData[0].ObjectToWorld;
    }
    else
    {
      nsMat4 objectToWorld = pRenderData->m_GlobalTransform.GetAsMat4();

      nsMat3 mInverse = objectToWorld.GetRotationalPart();
      mInverse.Invert(0.0f).IgnoreResult();
      // we explicitly ignore the return value here (success / failure)
      // because when we have a scale of 0 (which happens temporarily during editing) that would be annoying
      instanceData[0].ObjectToWorldNormal = mInverse.GetTranspose();
    }

    pInstanceData->UpdateInstanceData(pRenderContext, 1);

    const auto& desc = pBuffer->GetDescriptor();
    pBuffer->UpdateGpuBuffer(pGALCommandEncoder);

    // redo this after the primitive count has changed
    pRenderContext->BindMeshBuffer(pRenderData->m_hMesh);

    renderViewContext.m_pRenderContext->DrawMeshBuffer(pRenderData->m_uiNumPrimitives, pRenderData->m_uiFirstPrimitive).IgnoreResult();
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_CustomMeshComponent);
