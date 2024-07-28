#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Meshes/CpuMeshResource.h>
#include <RendererCore/Meshes/MeshComponent.h>
#include <RendererCore/Utils/WorldGeoExtractionUtil.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsMeshComponent, 3, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Mesh", GetMeshFile, SetMeshFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("CustomData", GetCustomData, SetCustomData)->AddAttributes(new nsDefaultValueAttribute(nsVec4(0, 1, 0, 1))),
    NS_ARRAY_ACCESSOR_PROPERTY("Materials", Materials_GetCount, Materials_GetValue, Materials_SetValue, Materials_Insert, Materials_Remove)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),
    NS_ACCESSOR_PROPERTY("SortingDepthOffset", GetSortingDepthOffset, SetSortingDepthOffset),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractGeometry, OnMsgExtractGeometry)
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsMeshComponent::nsMeshComponent() = default;
nsMeshComponent::~nsMeshComponent() = default;

void nsMeshComponent::OnMsgExtractGeometry(nsMsgExtractGeometry& ref_msg) const
{
  if (ref_msg.m_Mode != nsWorldGeoExtractionUtil::ExtractionMode::RenderMesh)
    return;

  // ignore invalid and created resources
  {
    nsMeshResourceHandle hRenderMesh = GetMesh();
    if (!hRenderMesh.IsValid())
      return;

    nsResourceLock<nsMeshResource> pRenderMesh(hRenderMesh, nsResourceAcquireMode::PointerOnly);
    if (pRenderMesh->GetBaseResourceFlags().IsAnySet(nsResourceFlags::IsCreatedResource))
      return;
  }

  ref_msg.AddMeshObject(GetOwner()->GetGlobalTransform(), nsResourceManager::LoadResource<nsCpuMeshResource>(GetMeshFile()));
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshComponent);
