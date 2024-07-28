#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/SkyBoxComponent.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/Textures/TextureCubeResource.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsSkyBoxComponent, 4, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("CubeMap", GetCubeMapFile, SetCubeMapFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_Cube")),
    NS_ACCESSOR_PROPERTY("ExposureBias", GetExposureBias, SetExposureBias)->AddAttributes(new nsClampValueAttribute(-32.0f, 32.0f)),
    NS_ACCESSOR_PROPERTY("InverseTonemap", GetInverseTonemap, SetInverseTonemap),
    NS_ACCESSOR_PROPERTY("UseFog", GetUseFog, SetUseFog)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ACCESSOR_PROPERTY("VirtualDistance", GetVirtualDistance, SetVirtualDistance)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1000.0f)),
  }
  NS_END_PROPERTIES;
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

nsSkyBoxComponent::nsSkyBoxComponent() = default;
nsSkyBoxComponent::~nsSkyBoxComponent() = default;

void nsSkyBoxComponent::Initialize()
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

  const char* szMeshResourceName = "SkyBoxMesh";
  m_hMesh = nsResourceManager::GetExistingResource<nsMeshResource>(szMeshResourceName);
  if (!m_hMesh.IsValid())
  {
    nsMeshResourceDescriptor desc;
    desc.UseExistingMeshBuffer(hMeshBuffer);
    desc.AddSubMesh(2, 0, 0);
    desc.ComputeBounds();

    m_hMesh = nsResourceManager::GetOrCreateResource<nsMeshResource>(szMeshResourceName, std::move(desc), szMeshResourceName);
  }

  nsStringBuilder cubeMapMaterialName = "SkyBoxMaterial_CubeMap";
  cubeMapMaterialName.AppendFormat("_{0}", nsArgP(GetWorld())); // make the resource unique for each world

  m_hCubeMapMaterial = nsResourceManager::GetExistingResource<nsMaterialResource>(cubeMapMaterialName);
  if (!m_hCubeMapMaterial.IsValid())
  {
    nsMaterialResourceDescriptor desc;
    desc.m_hBaseMaterial = nsResourceManager::LoadResource<nsMaterialResource>("{ b4b75b1c-c2c8-4a0e-8076-780bdd46d18b }"); // Sky.nsMaterialAsset

    m_hCubeMapMaterial = nsResourceManager::CreateResource<nsMaterialResource>(cubeMapMaterialName, std::move(desc), cubeMapMaterialName);
  }

  UpdateMaterials();
}

nsResult nsSkyBoxComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  ref_bAlwaysVisible = true;
  return NS_SUCCESS;
}

void nsSkyBoxComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract sky render data for selection or in orthographic views.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory || msg.m_pView->GetCamera()->IsOrthographic())
    return;

  nsMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsMeshRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalTransform.m_vPosition.SetZero(); // skybox should always be at the origin
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hMesh = m_hMesh;
    pRenderData->m_hMaterial = m_hCubeMapMaterial;
    pRenderData->m_uiSubMeshIndex = 0;
    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();

    pRenderData->FillBatchIdAndSortingKey();
  }

  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Sky, nsRenderData::Caching::Never);
}

void nsSkyBoxComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_fExposureBias;
  s << m_bInverseTonemap;
  s << m_bUseFog;
  s << m_fVirtualDistance;
  s << m_hCubeMap;
}

void nsSkyBoxComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_fExposureBias;
  s >> m_bInverseTonemap;

  if (uiVersion >= 4)
  {
    s >> m_bUseFog;
    s >> m_fVirtualDistance;
  }

  if (uiVersion >= 3)
  {
    s >> m_hCubeMap;
  }
  else
  {
    nsTexture2DResourceHandle dummyHandle;
    for (int i = 0; i < 6; i++)
    {
      s >> dummyHandle;
    }
  }
}

void nsSkyBoxComponent::SetExposureBias(float fExposureBias)
{
  m_fExposureBias = fExposureBias;

  UpdateMaterials();
}

void nsSkyBoxComponent::SetInverseTonemap(bool bInverseTonemap)
{
  m_bInverseTonemap = bInverseTonemap;

  UpdateMaterials();
}

void nsSkyBoxComponent::SetUseFog(bool bUseFog)
{
  m_bUseFog = bUseFog;

  UpdateMaterials();
}

void nsSkyBoxComponent::SetVirtualDistance(float fVirtualDistance)
{
  m_fVirtualDistance = fVirtualDistance;

  UpdateMaterials();
}

void nsSkyBoxComponent::SetCubeMapFile(const char* szFile)
{
  nsTextureCubeResourceHandle hCubeMap;
  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hCubeMap = nsResourceManager::LoadResource<nsTextureCubeResource>(szFile);
  }

  SetCubeMap(hCubeMap);
}

const char* nsSkyBoxComponent::GetCubeMapFile() const
{
  return m_hCubeMap.IsValid() ? m_hCubeMap.GetResourceID().GetData() : "";
}

void nsSkyBoxComponent::SetCubeMap(const nsTextureCubeResourceHandle& hCubeMap)
{
  m_hCubeMap = hCubeMap;
  UpdateMaterials();
}

const nsTextureCubeResourceHandle& nsSkyBoxComponent::GetCubeMap() const
{
  return m_hCubeMap;
}

void nsSkyBoxComponent::OnActivated()
{
  SUPER::OnActivated();

  UpdateMaterials();
}

void nsSkyBoxComponent::UpdateMaterials()
{
  if (m_hCubeMapMaterial.IsValid())
  {
    nsResourceLock<nsMaterialResource> pMaterial(m_hCubeMapMaterial, nsResourceAcquireMode::AllowLoadingFallback);

    pMaterial->SetParameter("ExposureBias", m_fExposureBias);
    pMaterial->SetParameter("InverseTonemap", m_bInverseTonemap);
    pMaterial->SetParameter("UseFog", m_bUseFog);
    pMaterial->SetParameter("VirtualDistance", m_fVirtualDistance);
    pMaterial->SetTextureCubeBinding("CubeMap", m_hCubeMap);

    pMaterial->PreserveCurrentDesc();
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsSkyBoxComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsSkyBoxComponentPatch_1_2()
    : nsGraphPatch("nsSkyBoxComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Exposure Bias", "ExposureBias");
    pNode->RenameProperty("Inverse Tonemap", "InverseTonemap");
    pNode->RenameProperty("Left Texture", "LeftTexture");
    pNode->RenameProperty("Front Texture", "FrontTexture");
    pNode->RenameProperty("Right Texture", "RightTexture");
    pNode->RenameProperty("Back Texture", "BackTexture");
    pNode->RenameProperty("Up Texture", "UpTexture");
    pNode->RenameProperty("Down Texture", "DownTexture");
  }
};

nsSkyBoxComponentPatch_1_2 g_nsSkyBoxComponentPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_SkyBoxComponent);
