#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Components/AtmosphericScatteringComponent.h>

#include <Core/Graphics/Geometry.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Pipeline/View.h>

#include <RendererCore/Lights/DirectionalLightComponent.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsAtmosphericScatteringComponent, 1, nsComponentMode::Static)
{

  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE;
// clang-format on

nsAtmosphericScatteringComponent::nsAtmosphericScatteringComponent() = default;
nsAtmosphericScatteringComponent::~nsAtmosphericScatteringComponent() = default;

void nsAtmosphericScatteringComponent::Initialize()
{
  SUPER::Initialize();

  const char* szBufferResourceName = "SkyBoxBuffer";
  nsMeshBufferResourceHandle hMeshBuffer = nsResourceManager::GetExistingResource<nsMeshBufferResource>(szBufferResourceName);
  if (!hMeshBuffer.IsValid())
  {
    nsGeometry geom;
    geom.AddRect(nsVec2(2.0f));

    nsMeshBufferResourceDescriptor desc;
    desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
    desc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

    hMeshBuffer = nsResourceManager::GetOrCreateResource<nsMeshBufferResource>(szBufferResourceName, std::move(desc), szBufferResourceName);
  }

  const char* szMeshResourceName = "AtmosphericScatteringMesh";
  m_hMesh = nsResourceManager::GetExistingResource<nsMeshResource>(szMeshResourceName);
  if (!m_hMesh.IsValid())
  {
    nsMeshResourceDescriptor desc;
    desc.UseExistingMeshBuffer(hMeshBuffer);
    desc.AddSubMesh(2, 0, 0);
    desc.ComputeBounds();

    m_hMesh = nsResourceManager::GetOrCreateResource<nsMeshResource>(szMeshResourceName, std::move(desc), szMeshResourceName);
  }

  nsStringBuilder cubeMapMaterialName = "AtmosphericScatteringMaterial";
  cubeMapMaterialName.AppendFormat("_{0}", nsArgP(GetWorld())); // make the resource unique for each world

  m_hMaterial = nsResourceManager::GetExistingResource<nsMaterialResource>(cubeMapMaterialName);
  if (!m_hMaterial.IsValid())
  {
    nsMaterialResourceDescriptor desc;
    desc.m_hBaseMaterial = nsResourceManager::LoadResource<nsMaterialResource>("AtmosphericScattering.nsMaterialAsset"); // AtmosphericScattering.nsMaterialAsset

    m_hMaterial = nsResourceManager::CreateResource<nsMaterialResource>(cubeMapMaterialName, std::move(desc), cubeMapMaterialName);
  }

  UpdateMaterials();
}

nsResult nsAtmosphericScatteringComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  ref_bAlwaysVisible = true;
  return NS_SUCCESS;
}

void nsAtmosphericScatteringComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract sky render data for selection or in orthographic views.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory || msg.m_pView->GetCamera()->IsOrthographic())
    return;


  nsVec3 sunDirection = GetOwner()->GetGlobalTransform().m_qRotation * nsVec3(-1, 0, 0);

  nsMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsMeshRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalTransform.m_vPosition.SetZero(); // skybox should always be at the origin
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hMesh = m_hMesh;
    pRenderData->m_hMaterial = m_hMaterial;
    pRenderData->m_uiSubMeshIndex = 0;
    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();

    pRenderData->FillBatchIdAndSortingKey();
  }

  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Sky, nsRenderData::Caching::Never);
}

void nsAtmosphericScatteringComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();
}

void nsAtmosphericScatteringComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();
}

void nsAtmosphericScatteringComponent::OnActivated()
{
  SUPER::OnActivated();

  UpdateMaterials();
}

void nsAtmosphericScatteringComponent::UpdateMaterials()
{
  if (m_hMaterial.IsValid())
  {
    nsResourceLock<nsMaterialResource> pMaterial(m_hMaterial, nsResourceAcquireMode::AllowLoadingFallback);

    pMaterial->PreserveCurrentDesc();
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_nsAtmosphericScatteringComponent);
