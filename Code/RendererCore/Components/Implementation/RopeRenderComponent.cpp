#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Geometry.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Configuration/CVar.h>
#include <RendererCore/AnimationSystem/Declarations.h>
#include <RendererCore/Components/RopeRenderComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/SkinnedMeshComponent.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Types.h>

nsCVarBool cvar_FeatureRopesVisBones("Feature.Ropes.VisBones", false, nsCVarFlags::Default, "Enables debug visualization of rope bones");

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsRopeRenderComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Material", GetMaterialFile, SetMaterialFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),
    NS_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new nsDefaultValueAttribute(nsColor::White), new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("Thickness", GetThickness, SetThickness)->AddAttributes(new nsDefaultValueAttribute(0.05f), new nsClampValueAttribute(0.0f, nsVariant())),
    NS_ACCESSOR_PROPERTY("Detail", GetDetail, SetDetail)->AddAttributes(new nsDefaultValueAttribute(6), new nsClampValueAttribute(3, 16)),
    NS_ACCESSOR_PROPERTY("Subdivide", GetSubdivide, SetSubdivide),
    NS_ACCESSOR_PROPERTY("UScale", GetUScale, SetUScale)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgRopePoseUpdated, OnRopePoseUpdated),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
    NS_MESSAGE_HANDLER(nsMsgSetMeshMaterial, OnMsgSetMeshMaterial),      
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Effects/Ropes"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsRopeRenderComponent::nsRopeRenderComponent() = default;
nsRopeRenderComponent::~nsRopeRenderComponent() = default;

void nsRopeRenderComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_Color;
  s << m_hMaterial;
  s << m_fThickness;
  s << m_uiDetail;
  s << m_bSubdivide;
  s << m_fUScale;
}

void nsRopeRenderComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = inout_stream.GetStream();

  s >> m_Color;
  s >> m_hMaterial;
  s >> m_fThickness;
  s >> m_uiDetail;
  s >> m_bSubdivide;
  s >> m_fUScale;
}

void nsRopeRenderComponent::OnActivated()
{
  SUPER::OnActivated();

  m_LocalBounds = nsBoundingBoxSphere::MakeInvalid();
}

void nsRopeRenderComponent::OnDeactivated()
{
  m_SkinningState.Clear();

  SUPER::OnDeactivated();
}

nsResult nsRopeRenderComponent::GetLocalBounds(nsBoundingBoxSphere& bounds, bool& bAlwaysVisible, nsMsgUpdateLocalBounds& msg)
{
  bounds = m_LocalBounds;
  return NS_SUCCESS;
}

void nsRopeRenderComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (!m_hMesh.IsValid())
    return;

  const nsUInt32 uiFlipWinding = GetOwner()->GetGlobalTransformSimd().ContainsNegativeScale() ? 1 : 0;
  const nsUInt32 uiUniformScale = GetOwner()->GetGlobalTransformSimd().ContainsUniformScale() ? 1 : 0;

  nsResourceLock<nsMeshResource> pMesh(m_hMesh, nsResourceAcquireMode::AllowLoadingFallback);
  nsMaterialResourceHandle hMaterial = m_hMaterial.IsValid() ? m_hMaterial : pMesh->GetMaterials()[0];

  nsSkinnedMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsSkinnedMeshRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hMesh = m_hMesh;
    pRenderData->m_hMaterial = hMaterial;
    pRenderData->m_Color = m_Color;

    pRenderData->m_uiSubMeshIndex = 0;
    pRenderData->m_uiFlipWinding = uiFlipWinding;
    pRenderData->m_uiUniformScale = uiUniformScale;

    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();

    m_SkinningState.FillSkinnedMeshRenderData(*pRenderData);

    pRenderData->FillBatchIdAndSortingKey();
  }

  // Determine render data category.
  nsRenderData::Category category = nsDefaultRenderDataCategories::LitOpaque;

  if (hMaterial.IsValid())
  {
    nsResourceLock<nsMaterialResource> pMaterial(hMaterial, nsResourceAcquireMode::AllowLoadingFallback);
    category = pMaterial->GetRenderDataCategory();
  }

  msg.AddRenderData(pRenderData, category, nsRenderData::Caching::Never);

  if (cvar_FeatureRopesVisBones)
  {
    nsHybridArray<nsDebugRenderer::Line, 128> lines(nsFrameAllocator::GetCurrentAllocator());
    lines.Reserve(m_SkinningState.m_Transforms.GetCount() * 3);

    nsMat4 offsetMat;
    offsetMat.SetIdentity();

    for (nsUInt32 i = 0; i < m_SkinningState.m_Transforms.GetCount(); ++i)
    {
      offsetMat.SetTranslationVector(nsVec3(static_cast<float>(i), 0, 0));
      nsMat4 skinningMat = m_SkinningState.m_Transforms[i].GetAsMat4() * offsetMat;

      nsVec3 pos = skinningMat.GetTranslationVector();

      auto& x = lines.ExpandAndGetRef();
      x.m_start = pos;
      x.m_end = x.m_start + skinningMat.TransformDirection(nsVec3::MakeAxisX());
      x.m_startColor = nsColor::Red;
      x.m_endColor = nsColor::Red;

      auto& y = lines.ExpandAndGetRef();
      y.m_start = pos;
      y.m_end = y.m_start + skinningMat.TransformDirection(nsVec3::MakeAxisY() * 2.0f);
      y.m_startColor = nsColor::Green;
      y.m_endColor = nsColor::Green;

      auto& z = lines.ExpandAndGetRef();
      z.m_start = pos;
      z.m_end = z.m_start + skinningMat.TransformDirection(nsVec3::MakeAxisZ() * 2.0f);
      z.m_startColor = nsColor::Blue;
      z.m_endColor = nsColor::Blue;
    }

    nsDebugRenderer::DrawLines(msg.m_pView->GetHandle(), lines, nsColor::White, GetOwner()->GetGlobalTransform());
  }
}

void nsRopeRenderComponent::SetMaterialFile(const char* szFile)
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

const char* nsRopeRenderComponent::GetMaterialFile() const
{
  if (!m_hMaterial.IsValid())
    return "";

  return m_hMaterial.GetResourceID();
}

void nsRopeRenderComponent::SetThickness(float fThickness)
{
  if (m_fThickness != fThickness)
  {
    m_fThickness = fThickness;

    if (IsActiveAndInitialized() && !m_SkinningState.m_Transforms.IsEmpty())
    {
      nsHybridArray<nsTransform, 128> transforms;
      transforms.SetCountUninitialized(m_SkinningState.m_Transforms.GetCount());

      nsMat4 offsetMat;
      offsetMat.SetIdentity();

      for (nsUInt32 i = 0; i < m_SkinningState.m_Transforms.GetCount(); ++i)
      {
        offsetMat.SetTranslationVector(nsVec3(static_cast<float>(i), 0, 0));
        nsMat4 skinningMat = m_SkinningState.m_Transforms[i].GetAsMat4() * offsetMat;

        transforms[i] = nsTransform::MakeFromMat4(skinningMat);
      }

      UpdateSkinningTransformBuffer(transforms);
    }
  }
}

void nsRopeRenderComponent::SetDetail(nsUInt32 uiDetail)
{
  if (m_uiDetail != uiDetail)
  {
    m_uiDetail = uiDetail;

    if (IsActiveAndInitialized() && !m_SkinningState.m_Transforms.IsEmpty())
    {
      GenerateRenderMesh(m_SkinningState.m_Transforms.GetCount());
    }
  }
}

void nsRopeRenderComponent::SetSubdivide(bool bSubdivide)
{
  if (m_bSubdivide != bSubdivide)
  {
    m_bSubdivide = bSubdivide;

    if (IsActiveAndInitialized() && !m_SkinningState.m_Transforms.IsEmpty())
    {
      GenerateRenderMesh(m_SkinningState.m_Transforms.GetCount());
    }
  }
}

void nsRopeRenderComponent::SetUScale(float fUScale)
{
  if (m_fUScale != fUScale)
  {
    m_fUScale = fUScale;

    if (IsActiveAndInitialized() && !m_SkinningState.m_Transforms.IsEmpty())
    {
      GenerateRenderMesh(m_SkinningState.m_Transforms.GetCount());
    }
  }
}

void nsRopeRenderComponent::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_Color);
}

void nsRopeRenderComponent::OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg)
{
  SetMaterial(ref_msg.m_hMaterial);
}

void nsRopeRenderComponent::OnRopePoseUpdated(nsMsgRopePoseUpdated& msg)
{
  if (msg.m_LinkTransforms.IsEmpty())
    return;

  if (m_SkinningState.m_Transforms.GetCount() != msg.m_LinkTransforms.GetCount())
  {
    m_SkinningState.Clear();

    GenerateRenderMesh(msg.m_LinkTransforms.GetCount());
  }

  UpdateSkinningTransformBuffer(msg.m_LinkTransforms);

  nsBoundingBox newBounds = nsBoundingBox::MakeFromPoints(&msg.m_LinkTransforms[0].m_vPosition, msg.m_LinkTransforms.GetCount(), sizeof(nsTransform));

  // if the existing bounds are big enough, don't update them
  if (!m_LocalBounds.IsValid() || !m_LocalBounds.GetBox().Contains(newBounds))
  {
    m_LocalBounds.ExpandToInclude(nsBoundingBoxSphere::MakeFromBox(newBounds));

    TriggerLocalBoundsUpdate();
  }
}

void nsRopeRenderComponent::GenerateRenderMesh(nsUInt32 uiNumRopePieces)
{
  nsStringBuilder sResourceName;
  sResourceName.SetFormat("Rope-Mesh:{}{}-d{}-u{}", uiNumRopePieces, m_bSubdivide ? "Sub" : "", m_uiDetail, m_fUScale);

  m_hMesh = nsResourceManager::GetExistingResource<nsMeshResource>(sResourceName);
  if (m_hMesh.IsValid())
    return;

  nsGeometry geom;

  const nsAngle fDegStep = nsAngle::MakeFromDegree(360.0f / m_uiDetail);
  const float fVStep = 1.0f / m_uiDetail;

  auto addCap = [&](float x, const nsVec3& vNormal, nsUInt16 uiBoneIndex, bool bFlipWinding)
  {
    nsVec4U16 boneIndices(uiBoneIndex, 0, 0, 0);

    nsUInt32 centerIndex = geom.AddVertex(nsVec3(x, 0, 0), vNormal, nsVec2(0.5f, 0.5f), nsColor::White, boneIndices);

    nsAngle deg = nsAngle::MakeFromRadian(0);
    for (nsUInt32 s = 0; s < m_uiDetail; ++s)
    {
      const float fY = nsMath::Cos(deg);
      const float fZ = nsMath::Sin(deg);

      geom.AddVertex(nsVec3(x, fY, fZ), vNormal, nsVec2(fY, fZ), nsColor::White, boneIndices);

      deg += fDegStep;
    }

    nsUInt32 triangle[3];
    triangle[0] = centerIndex;
    for (nsUInt32 s = 0; s < m_uiDetail; ++s)
    {
      triangle[1] = s + triangle[0] + 1;
      triangle[2] = ((s + 1) % m_uiDetail) + triangle[0] + 1;

      geom.AddPolygon(triangle, bFlipWinding);
    }
  };

  auto addPiece = [&](float x, const nsVec4U16& vBoneIndices, const nsColorLinearUB& boneWeights, bool bCreatePolygons)
  {
    nsAngle deg = nsAngle::MakeFromRadian(0);
    float fU = x * m_fUScale;
    float fV = 0;

    for (nsUInt32 s = 0; s <= m_uiDetail; ++s)
    {
      const float fY = nsMath::Cos(deg);
      const float fZ = nsMath::Sin(deg);

      const nsVec3 pos(x, fY, fZ);
      const nsVec3 normal(0, fY, fZ);

      geom.AddVertex(pos, normal, nsVec2(fU, fV), nsColor::White, vBoneIndices, boneWeights);

      deg += fDegStep;
      fV += fVStep;
    }

    if (bCreatePolygons)
    {
      nsUInt32 endIndex = geom.GetVertices().GetCount() - (m_uiDetail + 1);
      nsUInt32 startIndex = endIndex - (m_uiDetail + 1);

      nsUInt32 triangle[3];
      for (nsUInt32 s = 0; s < m_uiDetail; ++s)
      {
        triangle[0] = startIndex + s;
        triangle[1] = startIndex + s + 1;
        triangle[2] = endIndex + s + 1;
        geom.AddPolygon(triangle, false);

        triangle[0] = startIndex + s;
        triangle[1] = endIndex + s + 1;
        triangle[2] = endIndex + s;
        geom.AddPolygon(triangle, false);
      }
    }
  };

  // cap
  {
    const nsVec3 normal = nsVec3(-1, 0, 0);
    addCap(0.0f, normal, 0, true);
  }

  // pieces
  {
    // first ring full weight to first bone
    addPiece(0.0f, nsVec4U16(0, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), false);

    nsUInt16 p = 1;

    if (m_bSubdivide)
    {
      addPiece(0.75f, nsVec4U16(0, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), true);

      for (; p < uiNumRopePieces - 2; ++p)
      {
        addPiece(static_cast<float>(p) + 0.25f, nsVec4U16(p, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), true);
        addPiece(static_cast<float>(p) + 0.75f, nsVec4U16(p, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), true);
      }

      addPiece(static_cast<float>(p) + 0.25f, nsVec4U16(p, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), true);
      ++p;
    }
    else
    {
      for (; p < uiNumRopePieces - 1; ++p)
      {
        // Middle rings half weight between bones. To ensure that weights sum up to 1 we weight one bone with 128 and the other with 127,
        // since "ubyte normalized" can't represent 0.5 perfectly.
        addPiece(static_cast<float>(p), nsVec4U16(p - 1, p, 0, 0), nsColorLinearUB(128, 127, 0, 0), true);
      }
    }

    // last ring full weight to last bone
    addPiece(static_cast<float>(p), nsVec4U16(p, 0, 0, 0), nsColorLinearUB(255, 0, 0, 0), true);
  }

  // cap
  {
    const nsVec3 normal = nsVec3(1, 0, 0);
    addCap(static_cast<float>(uiNumRopePieces - 1), normal, static_cast<nsUInt16>(uiNumRopePieces - 1), false);
  }

  geom.ComputeTangents();

  nsMeshResourceDescriptor desc;

  // Data/Base/Materials/Prototyping/PrototypeBlack.nsMaterialAsset
  desc.SetMaterial(0, "{ d615cd66-0904-00ca-81f9-768ff4fc24ee }");

  auto& meshBufferDesc = desc.MeshBufferDesc();
  meshBufferDesc.AddCommonStreams();
  meshBufferDesc.AddStream(nsGALVertexAttributeSemantic::BoneIndices0, nsGALResourceFormat::RGBAUByte);
  meshBufferDesc.AddStream(nsGALVertexAttributeSemantic::BoneWeights0, nsGALResourceFormat::RGBAUByteNormalized);
  meshBufferDesc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

  desc.AddSubMesh(meshBufferDesc.GetPrimitiveCount(), 0, 0);

  desc.ComputeBounds();

  m_hMesh = nsResourceManager::CreateResource<nsMeshResource>(sResourceName, std::move(desc), sResourceName);
}

void nsRopeRenderComponent::UpdateSkinningTransformBuffer(nsArrayPtr<const nsTransform> skinningTransforms)
{
  nsMat4 bindPoseMat;
  bindPoseMat.SetIdentity();
  m_SkinningState.m_Transforms.SetCountUninitialized(skinningTransforms.GetCount());

  const nsVec3 newScale = nsVec3(1.0f, m_fThickness * 0.5f, m_fThickness * 0.5f);
  for (nsUInt32 i = 0; i < skinningTransforms.GetCount(); ++i)
  {
    nsTransform t = skinningTransforms[i];
    t.m_vScale = newScale;

    // scale x axis to match the distance between this bone and the next bone
    if (i < skinningTransforms.GetCount() - 1)
    {
      t.m_vScale.x = (skinningTransforms[i + 1].m_vPosition - skinningTransforms[i].m_vPosition).GetLength();
    }

    bindPoseMat.SetTranslationVector(nsVec3(-static_cast<float>(i), 0, 0));

    m_SkinningState.m_Transforms[i] = t.GetAsMat4() * bindPoseMat;
  }

  m_SkinningState.TransformsChanged();
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_RopeRenderComponent);
