#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Core/Messages/CollisionMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Reflection/Implementation/PropertyAttributes.h>
#include <RendererCore/Components/BeamComponent.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererFoundation/Device/Device.h>


// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsBeamComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("TargetObject", DummyGetter, SetTargetObject)->AddAttributes(new nsGameObjectReferenceAttribute()),
    NS_ACCESSOR_PROPERTY("Material", GetMaterialFile, SetMaterialFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),
    NS_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new nsDefaultValueAttribute(nsColor::White)),
    NS_ACCESSOR_PROPERTY("Width", GetWidth, SetWidth)->AddAttributes(new nsDefaultValueAttribute(0.1f), new nsClampValueAttribute(0.001f, nsVariant()), new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("UVUnitsPerWorldUnit", GetUVUnitsPerWorldUnit, SetUVUnitsPerWorldUnit)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.01f, nsVariant())),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Effects"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBeamComponent::nsBeamComponent() = default;
nsBeamComponent::~nsBeamComponent() = default;

void nsBeamComponent::Update()
{
  nsGameObject* pTargetObject = nullptr;
  if (GetWorld()->TryGetObject(m_hTargetObject, pTargetObject))
  {
    nsVec3 currentOwnerPosition = GetOwner()->GetGlobalPosition();
    nsVec3 currentTargetPosition = pTargetObject->GetGlobalPosition();

    if (!pTargetObject->IsActive())
    {
      currentTargetPosition = currentOwnerPosition;
    }

    bool updateMesh = false;

    if ((currentOwnerPosition - m_vLastOwnerPosition).GetLengthSquared() > m_fDistanceUpdateEpsilon)
    {
      updateMesh = true;
      m_vLastOwnerPosition = currentOwnerPosition;
    }

    if ((currentTargetPosition - m_vLastTargetPosition).GetLengthSquared() > m_fDistanceUpdateEpsilon)
    {
      updateMesh = true;
      m_vLastTargetPosition = currentTargetPosition;
    }

    if (updateMesh)
    {
      ReinitMeshes();
    }
  }
  else
  {
    m_hMesh.Invalidate();
  }
}

void nsBeamComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();
  inout_stream.WriteGameObjectHandle(m_hTargetObject);

  s << m_fWidth;
  s << m_fUVUnitsPerWorldUnit;
  s << m_hMaterial;
  s << m_Color;
}

void nsBeamComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();
  m_hTargetObject = inout_stream.ReadGameObjectHandle();

  s >> m_fWidth;
  s >> m_fUVUnitsPerWorldUnit;
  s >> m_hMaterial;
  s >> m_Color;
}

nsResult nsBeamComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  nsGameObject* pTargetObject = nullptr;
  if (GetWorld()->TryGetObject(m_hTargetObject, pTargetObject))
  {
    const nsVec3 currentTargetPosition = pTargetObject->GetGlobalPosition();
    const nsVec3 targetPositionInOwnerSpace = GetOwner()->GetGlobalTransform().GetInverse().TransformPosition(currentTargetPosition);

    nsVec3 pts[] = {nsVec3::MakeZero(), targetPositionInOwnerSpace};

    nsBoundingBox box = nsBoundingBox::MakeFromPoints(pts, 2);
    const float fHalfWidth = m_fWidth * 0.5f;
    box.m_vMin -= nsVec3(0, fHalfWidth, fHalfWidth);
    box.m_vMax += nsVec3(0, fHalfWidth, fHalfWidth);
    ref_bounds = nsBoundingBoxSphere::MakeFromBox(box);

    return NS_SUCCESS;
  }

  return NS_FAILURE;
}


void nsBeamComponent::OnActivated()
{
  SUPER::OnActivated();

  ReinitMeshes();
}

void nsBeamComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  Cleanup();
}

void nsBeamComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (!m_hMesh.IsValid() || !m_hMaterial.IsValid())
    return;

  nsMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsMeshRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hMesh = m_hMesh;
    pRenderData->m_hMaterial = m_hMaterial;
    pRenderData->m_Color = m_Color;
    pRenderData->m_uiSubMeshIndex = 0;
    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();

    pRenderData->FillBatchIdAndSortingKey();
  }

  // Determine render data category.
  nsResourceLock<nsMaterialResource> pMaterial(m_hMaterial, nsResourceAcquireMode::AllowLoadingFallback);
  nsRenderData::Category category = pMaterial->GetRenderDataCategory();

  msg.AddRenderData(pRenderData, category, nsRenderData::Caching::Never);
}

void nsBeamComponent::SetTargetObject(const char* szReference)
{
  auto resolver = GetWorld()->GetGameObjectReferenceResolver();

  if (!resolver.IsValid())
    return;

  m_hTargetObject = resolver(szReference, GetHandle(), "TargetObject");

  ReinitMeshes();
}

void nsBeamComponent::SetWidth(float fWidth)
{
  if (fWidth <= 0.0f)
    return;

  m_fWidth = fWidth;

  ReinitMeshes();
}

float nsBeamComponent::GetWidth() const
{
  return m_fWidth;
}

void nsBeamComponent::SetUVUnitsPerWorldUnit(float fUVUnitsPerWorldUnit)
{
  if (fUVUnitsPerWorldUnit <= 0.0f)
    return;

  m_fUVUnitsPerWorldUnit = fUVUnitsPerWorldUnit;

  ReinitMeshes();
}

float nsBeamComponent::GetUVUnitsPerWorldUnit() const
{
  return m_fUVUnitsPerWorldUnit;
}

void nsBeamComponent::SetMaterialFile(const char* szFile)
{
  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    m_hMaterial = nsResourceManager::LoadResource<nsMaterialResource>(szFile);
  }
  else
  {
    m_hMaterial.Invalidate();
  }
}

const char* nsBeamComponent::GetMaterialFile() const
{
  if (!m_hMaterial.IsValid())
    return "";

  return m_hMaterial.GetResourceID();
}

nsMaterialResourceHandle nsBeamComponent::GetMaterial() const
{
  return m_hMaterial;
}

void nsBeamComponent::CreateMeshes()
{
  nsVec3 targetPositionInOwnerSpace = GetOwner()->GetGlobalTransform().GetInverse().TransformPosition(m_vLastTargetPosition);

  if (targetPositionInOwnerSpace.IsZero(0.01f))
    return;

  // Create the beam mesh name, it expresses the beam in local space with it's width
  // this way multiple beams in a corridor can share the same mesh for example.
  nsStringBuilder meshName;
  meshName.SetFormat("nsBeamComponent_{0}_{1}_{2}_{3}.createdAtRuntime.nsMesh", m_fWidth, nsArgF(targetPositionInOwnerSpace.x, 2), nsArgF(targetPositionInOwnerSpace.y, 2), nsArgF(targetPositionInOwnerSpace.z, 2));

  m_hMesh = nsResourceManager::GetExistingResource<nsMeshResource>(meshName);

  // We build a cross mesh, thus we need the following vectors, x is the origin and we need to construct
  // the star points.
  //
  //  3        1
  //
  //      x
  //
  //  4        2
  nsVec3 crossVector1 = (0.5f * nsVec3::MakeAxisY() + 0.5f * nsVec3::MakeAxisZ());
  crossVector1.SetLength(m_fWidth * 0.5f).IgnoreResult();

  nsVec3 crossVector2 = (0.5f * nsVec3::MakeAxisY() - 0.5f * nsVec3::MakeAxisZ());
  crossVector2.SetLength(m_fWidth * 0.5f).IgnoreResult();

  nsVec3 crossVector3 = (-0.5f * nsVec3::MakeAxisY() + 0.5f * nsVec3::MakeAxisZ());
  crossVector3.SetLength(m_fWidth * 0.5f).IgnoreResult();

  nsVec3 crossVector4 = (-0.5f * nsVec3::MakeAxisY() - 0.5f * nsVec3::MakeAxisZ());
  crossVector4.SetLength(m_fWidth * 0.5f).IgnoreResult();

  const float fDistance = (m_vLastOwnerPosition - m_vLastTargetPosition).GetLength();



  // Build mesh if no existing one is found
  if (!m_hMesh.IsValid())
  {
    nsGeometry g;

    // Quad 1
    {
      nsUInt32 index0 = g.AddVertex(nsVec3::MakeZero() + crossVector1, nsVec3::MakeAxisX(), nsVec2(0, 0), nsColor::White);
      nsUInt32 index1 = g.AddVertex(nsVec3::MakeZero() + crossVector4, nsVec3::MakeAxisX(), nsVec2(0, 1), nsColor::White);
      nsUInt32 index2 = g.AddVertex(targetPositionInOwnerSpace + crossVector1, nsVec3::MakeAxisX(), nsVec2(fDistance * m_fUVUnitsPerWorldUnit, 0), nsColor::White);
      nsUInt32 index3 = g.AddVertex(targetPositionInOwnerSpace + crossVector4, nsVec3::MakeAxisX(), nsVec2(fDistance * m_fUVUnitsPerWorldUnit, 1), nsColor::White);

      nsUInt32 indices[] = {index0, index2, index3, index1};
      g.AddPolygon(nsArrayPtr(indices), false);
      g.AddPolygon(nsArrayPtr(indices), true);
    }

    // Quad 2
    {
      nsUInt32 index0 = g.AddVertex(nsVec3::MakeZero() + crossVector2, nsVec3::MakeAxisX(), nsVec2(0, 0), nsColor::White);
      nsUInt32 index1 = g.AddVertex(nsVec3::MakeZero() + crossVector3, nsVec3::MakeAxisX(), nsVec2(0, 1), nsColor::White);
      nsUInt32 index2 = g.AddVertex(targetPositionInOwnerSpace + crossVector2, nsVec3::MakeAxisX(), nsVec2(fDistance * m_fUVUnitsPerWorldUnit, 0), nsColor::White);
      nsUInt32 index3 = g.AddVertex(targetPositionInOwnerSpace + crossVector3, nsVec3::MakeAxisX(), nsVec2(fDistance * m_fUVUnitsPerWorldUnit, 1), nsColor::White);

      nsUInt32 indices[] = {index0, index2, index3, index1};
      g.AddPolygon(nsArrayPtr(indices), false);
      g.AddPolygon(nsArrayPtr(indices), true);
    }

    g.ComputeTangents();

    nsMeshResourceDescriptor desc;
    BuildMeshResourceFromGeometry(g, desc);

    m_hMesh = nsResourceManager::CreateResource<nsMeshResource>(meshName, std::move(desc));
  }
}

void nsBeamComponent::BuildMeshResourceFromGeometry(nsGeometry& Geometry, nsMeshResourceDescriptor& MeshDesc) const
{
  auto& MeshBufferDesc = MeshDesc.MeshBufferDesc();

  MeshBufferDesc.AddCommonStreams();
  MeshBufferDesc.AllocateStreamsFromGeometry(Geometry, nsGALPrimitiveTopology::Triangles);

  MeshDesc.AddSubMesh(MeshBufferDesc.GetPrimitiveCount(), 0, 0);

  MeshDesc.ComputeBounds();
}

void nsBeamComponent::ReinitMeshes()
{
  Cleanup();

  if (IsActiveAndInitialized())
  {
    CreateMeshes();
    GetOwner()->UpdateLocalBounds();
  }
}

void nsBeamComponent::Cleanup()
{
  m_hMesh.Invalidate();
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_BeamComponent);
