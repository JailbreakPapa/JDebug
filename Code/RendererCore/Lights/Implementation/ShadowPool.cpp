#include <RendererCore/RendererCorePCH.h>

#include <Core/GameApplication/GameApplicationBase.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Profiling/Profiling.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/DirectionalLightComponent.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/PointLightComponent.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Utils/CoreRenderProfile.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/Pass.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <Shaders/Common/LightData.h>


// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, ShadowPool)
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "RenderWorld"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsShadowPool::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsShadowPool::OnEngineShutdown();
  }
NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
nsCVarBool cvar_RenderingShadowsShowPoolStats("Rendering.Shadows.ShowPoolStats", false, nsCVarFlags::Default, "Display same stats of the shadow pool");
nsCVarBool cvar_RenderingShadowsVisCascadeBounds("Rendering.Shadows.VisCascadeBounds", false, nsCVarFlags::Default, "Visualizes the bounding volumes of shadow cascades");
#endif

/// NOTE: The default values for these are defined in nsCoreRenderProfileConfig
///       but they can also be overwritten in custom game states at startup.
NS_RENDERERCORE_DLL nsCVarInt cvar_RenderingShadowsAtlasSize("Rendering.Shadows.AtlasSize", 4096, nsCVarFlags::RequiresDelayedSync, "The size of the shadow atlas texture.");
NS_RENDERERCORE_DLL nsCVarInt cvar_RenderingShadowsMaxShadowMapSize("Rendering.Shadows.MaxShadowMapSize", 1024, nsCVarFlags::RequiresDelayedSync, "The max shadow map size used.");
NS_RENDERERCORE_DLL nsCVarInt cvar_RenderingShadowsMinShadowMapSize("Rendering.Shadows.MinShadowMapSize", 64, nsCVarFlags::RequiresDelayedSync, "The min shadow map size used.");

static nsUInt32 s_uiLastConfigModification = 0;
static float s_fFadeOutScaleStart = 0.0f;
static float s_fFadeOutScaleEnd = 0.0f;

struct ShadowView
{
  nsViewHandle m_hView;
  nsCamera m_Camera;
};

struct ShadowData
{
  nsHybridArray<nsViewHandle, 6> m_Views;
  nsUInt32 m_uiType;
  float m_fShadowMapScale;
  float m_fPenumbraSize;
  float m_fSlopeBias;
  float m_fConstantBias;
  float m_fFadeOutStart;
  float m_fMinRange;
  float m_fActualRange;
  nsUInt32 m_uiPackedDataOffset; // in 16 bytes steps
};

struct LightAndRefView
{
  NS_DECLARE_POD_TYPE();

  const nsLightComponent* m_pLight;
  const nsView* m_pReferenceView;
};

struct SortedShadowData
{
  NS_DECLARE_POD_TYPE();

  nsUInt32 m_uiIndex;
  float m_fShadowMapScale;

  NS_ALWAYS_INLINE bool operator<(const SortedShadowData& other) const
  {
    if (m_fShadowMapScale > other.m_fShadowMapScale) // we want to sort descending (higher scale first)
      return true;

    return m_uiIndex < other.m_uiIndex;
  }
};

static nsDynamicArray<SortedShadowData> s_SortedShadowData;

struct AtlasCell
{
  NS_DECLARE_POD_TYPE();

  NS_ALWAYS_INLINE AtlasCell()
    : m_Rect(0, 0, 0, 0)
  {
    m_uiChildIndices[0] = m_uiChildIndices[1] = m_uiChildIndices[2] = m_uiChildIndices[3] = 0xFFFF;
    m_uiDataIndex = nsInvalidIndex;
  }

  NS_ALWAYS_INLINE bool IsLeaf() const
  {
    return m_uiChildIndices[0] == 0xFFFF && m_uiChildIndices[1] == 0xFFFF && m_uiChildIndices[2] == 0xFFFF && m_uiChildIndices[3] == 0xFFFF;
  }

  nsRectU32 m_Rect;
  nsUInt16 m_uiChildIndices[4];
  nsUInt32 m_uiDataIndex;
};

static nsDeque<AtlasCell> s_AtlasCells;

static AtlasCell* Insert(AtlasCell* pCell, nsUInt32 uiShadowMapSize, nsUInt32 uiDataIndex)
{
  if (!pCell->IsLeaf())
  {
    for (nsUInt32 i = 0; i < 4; ++i)
    {
      AtlasCell* pChildCell = &s_AtlasCells[pCell->m_uiChildIndices[i]];
      if (AtlasCell* pNewCell = Insert(pChildCell, uiShadowMapSize, uiDataIndex))
      {
        return pNewCell;
      }
    }

    return nullptr;
  }
  else
  {
    if (pCell->m_uiDataIndex != nsInvalidIndex)
      return nullptr;

    if (pCell->m_Rect.width < uiShadowMapSize || pCell->m_Rect.height < uiShadowMapSize)
      return nullptr;

    if (pCell->m_Rect.width == uiShadowMapSize && pCell->m_Rect.height == uiShadowMapSize)
    {
      pCell->m_uiDataIndex = uiDataIndex;
      return pCell;
    }

    // Split
    nsUInt32 x = pCell->m_Rect.x;
    nsUInt32 y = pCell->m_Rect.y;
    nsUInt32 w = pCell->m_Rect.width / 2;
    nsUInt32 h = pCell->m_Rect.height / 2;

    nsUInt32 uiCellIndex = s_AtlasCells.GetCount();
    s_AtlasCells.ExpandAndGetRef().m_Rect = nsRectU32(x, y, w, h);
    s_AtlasCells.ExpandAndGetRef().m_Rect = nsRectU32(x + w, y, w, h);
    s_AtlasCells.ExpandAndGetRef().m_Rect = nsRectU32(x, y + h, w, h);
    s_AtlasCells.ExpandAndGetRef().m_Rect = nsRectU32(x + w, y + h, w, h);

    for (nsUInt32 i = 0; i < 4; ++i)
    {
      pCell->m_uiChildIndices[i] = static_cast<nsUInt16>(uiCellIndex + i);
    }

    AtlasCell* pChildCell = &s_AtlasCells[pCell->m_uiChildIndices[0]];
    return Insert(pChildCell, uiShadowMapSize, uiDataIndex);
  }
}

static nsRectU32 FindAtlasRect(nsUInt32 uiShadowMapSize, nsUInt32 uiDataIndex)
{
  NS_ASSERT_DEBUG(nsMath::IsPowerOf2(uiShadowMapSize), "Size must be power of 2");

  AtlasCell* pCell = Insert(&s_AtlasCells[0], uiShadowMapSize, uiDataIndex);
  if (pCell != nullptr)
  {
    NS_ASSERT_DEBUG(pCell->IsLeaf() && pCell->m_uiDataIndex == uiDataIndex, "Implementation error");
    return pCell->m_Rect;
  }

  nsLog::Warning("Shadow Pool is full. Not enough space for a {0}x{0} shadow map. The light will have no shadow.", uiShadowMapSize);
  return nsRectU32(0, 0, 0, 0);
}

static float AddSafeBorder(nsAngle fov, float fPenumbraSize)
{
  float fHalfHeight = nsMath::Tan(fov * 0.5f);
  float fNewFov = nsMath::ATan(fHalfHeight + fPenumbraSize).GetDegree() * 2.0f;
  return fNewFov;
}

nsTagSet s_ExcludeTagsWhiteList;

static void CopyExcludeTagsOnWhiteList(const nsTagSet& referenceTags, nsTagSet& out_targetTags)
{
  out_targetTags.Clear();
  out_targetTags.SetByName("EditorHidden");

  for (auto& tag : referenceTags)
  {
    if (s_ExcludeTagsWhiteList.IsSet(tag))
    {
      out_targetTags.Set(tag);
    }
  }
}

// must not be in anonymous namespace
template <>
struct nsHashHelper<LightAndRefView>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(LightAndRefView value) { return nsHashingUtils::xxHash32(&value.m_pLight, sizeof(LightAndRefView)); }

  NS_ALWAYS_INLINE static bool Equal(const LightAndRefView& a, const LightAndRefView& b)
  {
    return a.m_pLight == b.m_pLight && a.m_pReferenceView == b.m_pReferenceView;
  }
};

//////////////////////////////////////////////////////////////////////////

struct nsShadowPool::Data
{
  Data() { Clear(); }

  ~Data()
  {
    for (auto& shadowView : m_ShadowViews)
    {
      nsRenderWorld::DeleteView(shadowView.m_hView);
    }

    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
    if (!m_hShadowAtlasTexture.IsInvalidated())
    {
      pDevice->DestroyTexture(m_hShadowAtlasTexture);
      m_hShadowAtlasTexture.Invalidate();
    }

    if (!m_hShadowDataBuffer.IsInvalidated())
    {
      pDevice->DestroyBuffer(m_hShadowDataBuffer);
      m_hShadowDataBuffer.Invalidate();
    }
  }

  enum
  {
    MAX_SHADOW_DATA = 1024
  };

  void CreateShadowAtlasTexture()
  {
    if (m_hShadowAtlasTexture.IsInvalidated())
    {
      // use the current CVar values to initialize the values
      nsUInt32 uiAtlas = cvar_RenderingShadowsAtlasSize;
      nsUInt32 uiMax = cvar_RenderingShadowsMaxShadowMapSize;
      nsUInt32 uiMin = cvar_RenderingShadowsMinShadowMapSize;

      // if the platform profile has changed, use it to reset the defaults
      if (s_uiLastConfigModification != nsGameApplicationBase::GetGameApplicationBaseInstance()->GetPlatformProfile().GetLastModificationCounter())
      {
        s_uiLastConfigModification = nsGameApplicationBase::GetGameApplicationBaseInstance()->GetPlatformProfile().GetLastModificationCounter();

        const auto* pConfig = nsGameApplicationBase::GetGameApplicationBaseInstance()->GetPlatformProfile().GetTypeConfig<nsCoreRenderProfileConfig>();

        uiAtlas = pConfig->m_uiShadowAtlasTextureSize;
        uiMax = pConfig->m_uiMaxShadowMapSize;
        uiMin = pConfig->m_uiMinShadowMapSize;
      }

      // if the CVars were modified recently (e.g. during game startup), use those values to override the default
      if (cvar_RenderingShadowsAtlasSize.HasDelayedSyncValueChanged())
        uiAtlas = cvar_RenderingShadowsAtlasSize.GetValue(nsCVarValue::DelayedSync);

      if (cvar_RenderingShadowsMaxShadowMapSize.HasDelayedSyncValueChanged())
        uiMax = cvar_RenderingShadowsMaxShadowMapSize.GetValue(nsCVarValue::DelayedSync);

      if (cvar_RenderingShadowsMinShadowMapSize.HasDelayedSyncValueChanged())
        uiMin = cvar_RenderingShadowsMinShadowMapSize.GetValue(nsCVarValue::DelayedSync);

      // make sure the values are valid
      uiAtlas = nsMath::Clamp(nsMath::PowerOfTwo_Floor(uiAtlas), 512u, 8192u);
      uiMax = nsMath::Clamp(nsMath::PowerOfTwo_Floor(uiMax), 64u, 2048u);
      uiMin = nsMath::Clamp(nsMath::PowerOfTwo_Floor(uiMin), 8u, 512u);

      uiMax = nsMath::Min(uiMax, uiAtlas);
      uiMin = nsMath::Min(uiMin, uiMax);

      // write back the clamped values, so that everyone sees the valid values
      cvar_RenderingShadowsAtlasSize = uiAtlas;
      cvar_RenderingShadowsMaxShadowMapSize = uiMax;
      cvar_RenderingShadowsMinShadowMapSize = uiMin;

      // apply the new values
      cvar_RenderingShadowsAtlasSize.SetToDelayedSyncValue();
      cvar_RenderingShadowsMaxShadowMapSize.SetToDelayedSyncValue();
      cvar_RenderingShadowsMinShadowMapSize.SetToDelayedSyncValue();

      nsGALTextureCreationDescription desc;
      desc.SetAsRenderTarget(cvar_RenderingShadowsAtlasSize, cvar_RenderingShadowsAtlasSize, nsGALResourceFormat::D16);

      m_hShadowAtlasTexture = nsGALDevice::GetDefaultDevice()->CreateTexture(desc);

      s_fFadeOutScaleStart = (cvar_RenderingShadowsMinShadowMapSize + 1.0f) / cvar_RenderingShadowsMaxShadowMapSize;
      s_fFadeOutScaleEnd = s_fFadeOutScaleStart * 0.5f;
    }
  }

  void CreateShadowDataBuffer()
  {
    if (m_hShadowDataBuffer.IsInvalidated())
    {
      nsGALBufferCreationDescription desc;
      desc.m_uiStructSize = sizeof(nsVec4);
      desc.m_uiTotalSize = desc.m_uiStructSize * MAX_SHADOW_DATA;
      desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_ResourceAccess.m_bImmutable = false;

      m_hShadowDataBuffer = nsGALDevice::GetDefaultDevice()->CreateBuffer(desc);
    }
  }

  nsViewHandle CreateShadowView()
  {
    CreateShadowAtlasTexture();
    CreateShadowDataBuffer();

    nsView* pView = nullptr;
    nsViewHandle hView = nsRenderWorld::CreateView("Unknown", pView);

    pView->SetCameraUsageHint(nsCameraUsageHint::Shadow);

    nsGALRenderTargets renderTargets;
    renderTargets.m_hDSTarget = m_hShadowAtlasTexture;
    pView->SetRenderTargets(renderTargets);

    NS_ASSERT_DEV(m_ShadowViewsMutex.IsLocked(), "m_ShadowViewsMutex must be locked at this point.");
    m_ShadowViewsMutex.Unlock(); // if the resource gets loaded in the call below, his could lead to a deadlock

    // ShadowMapRenderPipeline.nsRenderPipelineAsset
    pView->SetRenderPipelineResource(nsResourceManager::LoadResource<nsRenderPipelineResource>("{ 4f4d9f16-3d47-4c67-b821-a778f11dcaf5 }"));

    m_ShadowViewsMutex.Lock();

    // Set viewport size to something valid, this will be changed to the proper location in the atlas texture in OnEndExtraction before
    // rendering.
    pView->SetViewport(nsRectFloat(0.0f, 0.0f, 1024.0f, 1024.0f));

    const nsTag& tagCastShadows = nsTagRegistry::GetGlobalRegistry().RegisterTag("CastShadow");
    pView->m_IncludeTags.Set(tagCastShadows);

    pView->m_ExcludeTags.SetByName("EditorHidden");

    return hView;
  }

  ShadowView& GetShadowView(nsView*& out_pView)
  {
    NS_LOCK(m_ShadowViewsMutex);

    if (m_uiUsedViews == m_ShadowViews.GetCount())
    {
      m_ShadowViews.ExpandAndGetRef().m_hView = CreateShadowView();
    }

    auto& shadowView = m_ShadowViews[m_uiUsedViews];
    if (nsRenderWorld::TryGetView(shadowView.m_hView, out_pView))
    {
      out_pView->SetCamera(&shadowView.m_Camera);
      out_pView->SetLodCamera(nullptr);
    }

    m_uiUsedViews++;
    return shadowView;
  }

  bool GetDataForExtraction(const nsLightComponent* pLight, const nsView* pReferenceView, float fShadowMapScale, nsUInt32 uiPackedDataSizeInBytes, ShadowData*& out_pData)
  {
    NS_LOCK(m_ShadowDataMutex);

    LightAndRefView key = {pLight, pReferenceView};

    nsUInt32 uiDataIndex = nsInvalidIndex;
    if (m_LightToShadowDataTable.TryGetValue(key, uiDataIndex))
    {
      out_pData = &m_ShadowData[uiDataIndex];
      out_pData->m_fShadowMapScale = nsMath::Max(out_pData->m_fShadowMapScale, fShadowMapScale);
      return true;
    }

    m_ShadowData.EnsureCount(m_uiUsedShadowData + 1);

    out_pData = &m_ShadowData[m_uiUsedShadowData];
    out_pData->m_fShadowMapScale = fShadowMapScale;
    out_pData->m_fPenumbraSize = pLight->GetPenumbraSize();
    out_pData->m_fSlopeBias = pLight->GetSlopeBias() * 100.0f;       // map from user friendly range to real range
    out_pData->m_fConstantBias = pLight->GetConstantBias() / 100.0f; // map from user friendly range to real range
    out_pData->m_fFadeOutStart = 1.0f;
    out_pData->m_fMinRange = 1.0f;
    out_pData->m_fActualRange = 1.0f;
    out_pData->m_uiPackedDataOffset = m_uiUsedPackedShadowData;

    m_LightToShadowDataTable.Insert(key, m_uiUsedShadowData);

    ++m_uiUsedShadowData;
    m_uiUsedPackedShadowData += uiPackedDataSizeInBytes / sizeof(nsVec4);

    return false;
  }

  void Clear()
  {
    m_uiUsedViews = 0;
    m_uiUsedShadowData = 0;

    m_LightToShadowDataTable.Clear();

    m_uiUsedPackedShadowData = 0;
  }

  nsMutex m_ShadowViewsMutex;
  nsDeque<ShadowView> m_ShadowViews;
  nsUInt32 m_uiUsedViews = 0;

  nsMutex m_ShadowDataMutex;
  nsDeque<ShadowData> m_ShadowData;
  nsUInt32 m_uiUsedShadowData = 0;
  nsHashTable<LightAndRefView, nsUInt32> m_LightToShadowDataTable;

  nsDynamicArray<nsVec4, nsAlignedAllocatorWrapper> m_PackedShadowData[2];
  nsUInt32 m_uiUsedPackedShadowData = 0; // in 16 bytes steps (sizeof(nsVec4))

  nsGALTextureHandle m_hShadowAtlasTexture;
  nsGALBufferHandle m_hShadowDataBuffer;
};

//////////////////////////////////////////////////////////////////////////

nsShadowPool::Data* nsShadowPool::s_pData = nullptr;

// static
nsUInt32 nsShadowPool::AddDirectionalLight(const nsDirectionalLightComponent* pDirLight, const nsView* pReferenceView)
{
  NS_ASSERT_DEBUG(pDirLight->GetCastShadows(), "Implementation error");

  // No shadows in orthographic views
  if (pReferenceView->GetCullingCamera()->IsOrthographic())
  {
    return nsInvalidIndex;
  }

  float fMaxReferenceSize = nsMath::Max(pReferenceView->GetViewport().width, pReferenceView->GetViewport().height);
  float fShadowMapScale = fMaxReferenceSize / cvar_RenderingShadowsMaxShadowMapSize;

  ShadowData* pData = nullptr;
  if (s_pData->GetDataForExtraction(pDirLight, pReferenceView, fShadowMapScale, sizeof(nsDirShadowData), pData))
  {
    return pData->m_uiPackedDataOffset;
  }

  nsUInt32 uiNumCascades = nsMath::Min(pDirLight->GetNumCascades(), 4u);
  const nsCamera* pReferenceCamera = pReferenceView->GetCullingCamera();

  pData->m_uiType = LIGHT_TYPE_DIR;
  pData->m_fFadeOutStart = pDirLight->GetFadeOutStart();
  pData->m_fMinRange = pDirLight->GetMinShadowRange();
  pData->m_Views.SetCount(uiNumCascades);

  // determine cascade ranges
  float fNearPlane = pReferenceCamera->GetNearPlane();
  float fShadowRange = pDirLight->GetMinShadowRange();
  float fSplitModeWeight = pDirLight->GetSplitModeWeight();

  float fCascadeRanges[4];
  for (nsUInt32 i = 0; i < uiNumCascades; ++i)
  {
    float f = float(i + 1) / uiNumCascades;
    float logDistance = fNearPlane * nsMath::Pow(fShadowRange / fNearPlane, f);
    float linearDistance = fNearPlane + (fShadowRange - fNearPlane) * f;
    fCascadeRanges[i] = nsMath::Lerp(linearDistance, logDistance, fSplitModeWeight);
  }

  const char* viewNames[4] = {"DirLightViewC0", "DirLightViewC1", "DirLightViewC2", "DirLightViewC3"};

  const nsGameObject* pOwner = pDirLight->GetOwner();
  const nsVec3 vLightDirForwards = pOwner->GetGlobalDirForwards();
  const nsVec3 vLightDirUp = pOwner->GetGlobalDirUp();

  float fAspectRatio = pReferenceView->GetViewport().width / pReferenceView->GetViewport().height;

  float fCascadeStart = 0.0f;
  float fCascadeEnd = 0.0f;
  float fTanFovX = nsMath::Tan(pReferenceCamera->GetFovX(fAspectRatio) * 0.5f);
  float fTanFovY = nsMath::Tan(pReferenceCamera->GetFovY(fAspectRatio) * 0.5f);
  nsVec3 corner = nsVec3(fTanFovX, fTanFovY, 1.0f);

  float fNearPlaneOffset = pDirLight->GetNearPlaneOffset();

  for (nsUInt32 i = 0; i < uiNumCascades; ++i)
  {
    nsView* pView = nullptr;
    ShadowView& shadowView = s_pData->GetShadowView(pView);
    pData->m_Views[i] = shadowView.m_hView;

    // Setup view
    {
      pView->SetName(viewNames[i]);
      pView->SetWorld(const_cast<nsWorld*>(pDirLight->GetWorld()));
      pView->SetLodCamera(pReferenceCamera);
      CopyExcludeTagsOnWhiteList(pReferenceView->m_ExcludeTags, pView->m_ExcludeTags);
    }

    // Setup camera
    {
      fCascadeStart = fCascadeEnd;

      // Make sure that the last cascade always covers the whole range so effects like e.g. particles can get away with
      // sampling only the last cascade.
      if (i == uiNumCascades - 1)
      {
        fCascadeStart = 0.0f;
      }

      fCascadeEnd = fCascadeRanges[i];

      nsVec3 startCorner = corner * fCascadeStart;
      nsVec3 endCorner = corner * fCascadeEnd;
      pData->m_fActualRange = endCorner.GetLength();

      // Find the enclosing sphere for the frustum:
      // The sphere center must be on the view's center ray and should be equally far away from the corner points.
      // x = distance from camera origin to sphere center
      // d1^2 = sc.x^2 + sc.y^2 + (x - sc.z)^2
      // d2^2 = ec.x^2 + ec.y^2 + (x - ec.z)^2
      // d1 == d2 and solve for x:
      float x = (endCorner.Dot(endCorner) - startCorner.Dot(startCorner)) / (2.0f * (endCorner.z - startCorner.z));
      x = nsMath::Min(x, fCascadeEnd);

      nsVec3 center = pReferenceCamera->GetPosition() + pReferenceCamera->GetDirForwards() * x;

      // prevent too large values
      // sometimes this can happen when imported data is badly scaled and thus way too large
      // then adding dirForwards result in no change and we run into other asserts later
      center.x = nsMath::Clamp(center.x, -1000000.0f, +1000000.0f);
      center.y = nsMath::Clamp(center.y, -1000000.0f, +1000000.0f);
      center.z = nsMath::Clamp(center.z, -1000000.0f, +1000000.0f);

      endCorner.z -= x;
      const float radius = endCorner.GetLength();

      const float fCameraToCenterDistance = radius + fNearPlaneOffset;
      const nsVec3 shadowCameraPos = center - vLightDirForwards * fCameraToCenterDistance;
      const float fFarPlane = radius + fCameraToCenterDistance;

      nsCamera& camera = shadowView.m_Camera;
      camera.LookAt(shadowCameraPos, center, vLightDirUp);
      camera.SetCameraMode(nsCameraMode::OrthoFixedWidth, radius * 2.0f, 0.0f, fFarPlane);

      // stabilize
      const nsMat4 worldToLightMatrix = pView->GetViewMatrix(nsCameraEye::Left);
      const float texelInWorld = (2.0f * radius) / cvar_RenderingShadowsMaxShadowMapSize;
      nsVec3 offset = worldToLightMatrix.TransformPosition(nsVec3::MakeZero());
      offset.x -= nsMath::Floor(offset.x / texelInWorld) * texelInWorld;
      offset.y -= nsMath::Floor(offset.y / texelInWorld) * texelInWorld;

      camera.MoveLocally(0.0f, offset.x, offset.y);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
      if (cvar_RenderingShadowsVisCascadeBounds)
      {
        nsDebugRenderer::DrawLineSphere(pReferenceView->GetHandle(), nsBoundingSphere::MakeFromCenterAndRadius(center, radius), nsColorScheme::LightUI(nsColorScheme::Orange));

        nsDebugRenderer::DrawLineSphere(pReferenceView->GetHandle(), nsBoundingSphere::MakeFromCenterAndRadius(nsVec3(0, 0, fCameraToCenterDistance), radius), nsColorScheme::LightUI(nsColorScheme::Red), nsTransform::MakeFromMat4(camera.GetViewMatrix().GetInverse()));
      }
#endif
    }

    nsRenderWorld::AddViewToRender(shadowView.m_hView);
  }

  return pData->m_uiPackedDataOffset;
}

// static
nsUInt32 nsShadowPool::AddPointLight(const nsPointLightComponent* pPointLight, float fScreenSpaceSize, const nsView* pReferenceView)
{
  NS_ASSERT_DEBUG(pPointLight->GetCastShadows(), "Implementation error");

  if (fScreenSpaceSize < s_fFadeOutScaleEnd * 2.0f)
  {
    return nsInvalidIndex;
  }

  ShadowData* pData = nullptr;
  if (s_pData->GetDataForExtraction(pPointLight, nullptr, fScreenSpaceSize, sizeof(nsPointShadowData), pData))
  {
    return pData->m_uiPackedDataOffset;
  }

  pData->m_uiType = LIGHT_TYPE_POINT;
  pData->m_Views.SetCount(6);

  nsVec3 faceDirs[6] = {
    nsVec3(1.0f, 0.0f, 0.0f),
    nsVec3(-1.0f, 0.0f, 0.0f),
    nsVec3(0.0f, 1.0f, 0.0f),
    nsVec3(0.0f, -1.0f, 0.0f),
    nsVec3(0.0f, 0.0f, 1.0f),
    nsVec3(0.0f, 0.0f, -1.0f),
  };

  const char* viewNames[6] = {
    "PointLightView+X",
    "PointLightView-X",
    "PointLightView+Y",
    "PointLightView-Y",
    "PointLightView+Z",
    "PointLightView-Z",
  };

  const nsGameObject* pOwner = pPointLight->GetOwner();
  nsVec3 vPosition = pOwner->GetGlobalPosition();
  nsVec3 vUp = nsVec3(0.0f, 0.0f, 1.0f);

  float fPenumbraSize = nsMath::Max(pPointLight->GetPenumbraSize(), (0.5f / cvar_RenderingShadowsMinShadowMapSize)); // at least one texel for hardware pcf
  float fFov = AddSafeBorder(nsAngle::MakeFromDegree(90.0f), fPenumbraSize);

  float fNearPlane = 0.1f;                                                                                           ///\todo expose somewhere
  float fFarPlane = pPointLight->GetEffectiveRange();

  for (nsUInt32 i = 0; i < 6; ++i)
  {
    nsView* pView = nullptr;
    ShadowView& shadowView = s_pData->GetShadowView(pView);
    pData->m_Views[i] = shadowView.m_hView;

    // Setup view
    {
      pView->SetName(viewNames[i]);
      pView->SetWorld(const_cast<nsWorld*>(pPointLight->GetWorld()));
      CopyExcludeTagsOnWhiteList(pReferenceView->m_ExcludeTags, pView->m_ExcludeTags);
    }

    // Setup camera
    {
      nsVec3 vForward = faceDirs[i];

      nsCamera& camera = shadowView.m_Camera;
      camera.LookAt(vPosition, vPosition + vForward, vUp);
      camera.SetCameraMode(nsCameraMode::PerspectiveFixedFovX, fFov, fNearPlane, fFarPlane);
    }

    nsRenderWorld::AddViewToRender(shadowView.m_hView);
  }

  return pData->m_uiPackedDataOffset;
}

// static
nsUInt32 nsShadowPool::AddSpotLight(const nsSpotLightComponent* pSpotLight, float fScreenSpaceSize, const nsView* pReferenceView)
{
  NS_ASSERT_DEBUG(pSpotLight->GetCastShadows(), "Implementation error");

  if (fScreenSpaceSize < s_fFadeOutScaleEnd)
  {
    return nsInvalidIndex;
  }

  ShadowData* pData = nullptr;
  if (s_pData->GetDataForExtraction(pSpotLight, nullptr, fScreenSpaceSize, sizeof(nsSpotShadowData), pData))
  {
    return pData->m_uiPackedDataOffset;
  }

  pData->m_uiType = LIGHT_TYPE_SPOT;
  pData->m_Views.SetCount(1);

  nsView* pView = nullptr;
  ShadowView& shadowView = s_pData->GetShadowView(pView);
  pData->m_Views[0] = shadowView.m_hView;

  // Setup view
  {
    pView->SetName("SpotLightView");
    pView->SetWorld(const_cast<nsWorld*>(pSpotLight->GetWorld()));
    CopyExcludeTagsOnWhiteList(pReferenceView->m_ExcludeTags, pView->m_ExcludeTags);
  }

  // Setup camera
  {
    const nsGameObject* pOwner = pSpotLight->GetOwner();
    nsVec3 vPosition = pOwner->GetGlobalPosition();
    nsVec3 vForward = pOwner->GetGlobalDirForwards();
    nsVec3 vUp = pOwner->GetGlobalDirUp();

    float fFov = AddSafeBorder(pSpotLight->GetOuterSpotAngle(), pSpotLight->GetPenumbraSize());
    float fNearPlane = 0.1f; ///\todo expose somewhere
    float fFarPlane = pSpotLight->GetEffectiveRange();

    nsCamera& camera = shadowView.m_Camera;
    camera.LookAt(vPosition, vPosition + vForward, vUp);
    camera.SetCameraMode(nsCameraMode::PerspectiveFixedFovX, fFov, fNearPlane, fFarPlane);
  }

  nsRenderWorld::AddViewToRender(shadowView.m_hView);

  return pData->m_uiPackedDataOffset;
}

// static
nsGALTextureHandle nsShadowPool::GetShadowAtlasTexture()
{
  return s_pData->m_hShadowAtlasTexture;
}

// static
nsGALBufferHandle nsShadowPool::GetShadowDataBuffer()
{
  return s_pData->m_hShadowDataBuffer;
}

// static
void nsShadowPool::AddExcludeTagToWhiteList(const nsTag& tag)
{
  s_ExcludeTagsWhiteList.Set(tag);
}

// static
void nsShadowPool::OnEngineStartup()
{
  s_pData = NS_DEFAULT_NEW(nsShadowPool::Data);

  nsRenderWorld::GetExtractionEvent().AddEventHandler(OnExtractionEvent);
  nsRenderWorld::GetRenderEvent().AddEventHandler(OnRenderEvent);
}

// static
void nsShadowPool::OnEngineShutdown()
{
  nsRenderWorld::GetExtractionEvent().RemoveEventHandler(OnExtractionEvent);
  nsRenderWorld::GetRenderEvent().RemoveEventHandler(OnRenderEvent);

  NS_DEFAULT_DELETE(s_pData);
}

// static
void nsShadowPool::OnExtractionEvent(const nsRenderWorldExtractionEvent& e)
{
  if (e.m_Type != nsRenderWorldExtractionEvent::Type::EndExtraction)
    return;

  NS_PROFILE_SCOPE("Shadow Pool Update");

  nsUInt32 uiDataIndex = nsRenderWorld::GetDataIndexForExtraction();
  auto& packedShadowData = s_pData->m_PackedShadowData[uiDataIndex];
  packedShadowData.SetCountUninitialized(s_pData->m_uiUsedPackedShadowData);

  if (s_pData->m_uiUsedShadowData == 0)
    return;

  // Sort by shadow map scale
  s_SortedShadowData.Clear();

  for (nsUInt32 uiShadowDataIndex = 0; uiShadowDataIndex < s_pData->m_uiUsedShadowData; ++uiShadowDataIndex)
  {
    auto& shadowData = s_pData->m_ShadowData[uiShadowDataIndex];

    auto& sorted = s_SortedShadowData.ExpandAndGetRef();
    sorted.m_uiIndex = uiShadowDataIndex;
    sorted.m_fShadowMapScale = shadowData.m_uiType == LIGHT_TYPE_DIR ? 100.0f : nsMath::Min(shadowData.m_fShadowMapScale, 10.0f);
  }

  s_SortedShadowData.Sort();

  // Prepare atlas
  s_AtlasCells.Clear();
  s_AtlasCells.ExpandAndGetRef().m_Rect = nsRectU32(0, 0, cvar_RenderingShadowsAtlasSize, cvar_RenderingShadowsAtlasSize);

  float fAtlasInvWidth = 1.0f / cvar_RenderingShadowsAtlasSize;
  float fAtlasInvHeight = 1.0f / cvar_RenderingShadowsAtlasSize;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  nsUInt32 uiTotalAtlasSize = cvar_RenderingShadowsAtlasSize * cvar_RenderingShadowsAtlasSize;
  nsUInt32 uiUsedAtlasSize = 0;

  nsDebugRendererContext debugContext(nsWorld::GetWorld(0));
  if (const nsView* pView = nsRenderWorld::GetViewByUsageHint(nsCameraUsageHint::MainView, nsCameraUsageHint::EditorView))
  {
    debugContext = nsDebugRendererContext(pView->GetHandle());
  }

  if (cvar_RenderingShadowsShowPoolStats)
  {
    nsDebugRenderer::DrawInfoText(debugContext, nsDebugTextPlacement::TopLeft, "ShadowPoolStats", "Shadow Pool Stats:", nsColor::LightSteelBlue);
    nsDebugRenderer::DrawInfoText(debugContext, nsDebugTextPlacement::TopLeft, "ShadowPoolStats", "Details (Name: Size - Atlas Offset)", nsColor::LightSteelBlue);
  }

#endif

  for (auto& sorted : s_SortedShadowData)
  {
    nsUInt32 uiShadowDataIndex = sorted.m_uiIndex;
    auto& shadowData = s_pData->m_ShadowData[uiShadowDataIndex];

    nsUInt32 uiShadowMapSize = cvar_RenderingShadowsMaxShadowMapSize;
    float fadeOutStart = s_fFadeOutScaleStart;
    float fadeOutEnd = s_fFadeOutScaleEnd;

    // point lights use a lot of atlas space thus we cut the shadow map size in half
    if (shadowData.m_uiType == LIGHT_TYPE_POINT)
    {
      uiShadowMapSize /= 2;
      fadeOutStart *= 2.0f;
      fadeOutEnd *= 2.0f;
    }

    uiShadowMapSize = nsMath::PowerOfTwo_Ceil((nsUInt32)(uiShadowMapSize * nsMath::Clamp(shadowData.m_fShadowMapScale, fadeOutStart, 1.0f)));

    nsHybridArray<nsView*, 8> shadowViews;
    nsHybridArray<nsRectU32, 8> atlasRects;

    // Fill atlas
    for (nsUInt32 uiViewIndex = 0; uiViewIndex < shadowData.m_Views.GetCount(); ++uiViewIndex)
    {
      nsView* pShadowView = nullptr;
      nsRenderWorld::TryGetView(shadowData.m_Views[uiViewIndex], pShadowView);
      shadowViews.PushBack(pShadowView);

      NS_ASSERT_DEV(pShadowView != nullptr, "Implementation error");

      nsRectU32 atlasRect = FindAtlasRect(uiShadowMapSize, uiShadowDataIndex);
      atlasRects.PushBack(atlasRect);

      pShadowView->SetViewport(nsRectFloat((float)atlasRect.x, (float)atlasRect.y, (float)atlasRect.width, (float)atlasRect.height));

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
      if (cvar_RenderingShadowsShowPoolStats)
      {
        nsDebugRenderer::DrawInfoText(debugContext, nsDebugTextPlacement::TopLeft, "ShadowPoolStats", nsFmt("{0}: {1} - {2}x{3}", pShadowView->GetName(), atlasRect.width, atlasRect.x, atlasRect.y), nsColor::LightSteelBlue);

        uiUsedAtlasSize += atlasRect.width * atlasRect.height;
      }
#endif
    }

    // Fill shadow data
    if (shadowData.m_uiType == LIGHT_TYPE_DIR)
    {
      nsUInt32 uiNumCascades = shadowData.m_Views.GetCount();

      nsUInt32 uiMatrixIndex = GET_WORLD_TO_LIGHT_MATRIX_INDEX(shadowData.m_uiPackedDataOffset, 0);
      nsMat4& worldToLightMatrix = *reinterpret_cast<nsMat4*>(&packedShadowData[uiMatrixIndex]);

      worldToLightMatrix = shadowViews[0]->GetViewProjectionMatrix(nsCameraEye::Left);

      for (nsUInt32 uiViewIndex = 0; uiViewIndex < uiNumCascades; ++uiViewIndex)
      {
        if (uiViewIndex >= 1)
        {
          nsMat4 cascadeToWorldMatrix = shadowViews[uiViewIndex]->GetInverseViewProjectionMatrix(nsCameraEye::Left);
          nsVec3 cascadeCorner = cascadeToWorldMatrix.TransformPosition(nsVec3(0.0f));
          cascadeCorner = worldToLightMatrix.TransformPosition(cascadeCorner);

          nsVec3 otherCorner = cascadeToWorldMatrix.TransformPosition(nsVec3(1.0f));
          otherCorner = worldToLightMatrix.TransformPosition(otherCorner);

          nsUInt32 uiCascadeScaleIndex = GET_CASCADE_SCALE_INDEX(shadowData.m_uiPackedDataOffset, uiViewIndex - 1);
          nsUInt32 uiCascadeOffsetIndex = GET_CASCADE_OFFSET_INDEX(shadowData.m_uiPackedDataOffset, uiViewIndex - 1);

          nsVec4& cascadeScale = packedShadowData[uiCascadeScaleIndex];
          nsVec4& cascadeOffset = packedShadowData[uiCascadeOffsetIndex];

          cascadeScale = nsVec3(1.0f).CompDiv(otherCorner - cascadeCorner).GetAsVec4(1.0f);
          cascadeOffset = cascadeCorner.GetAsVec4(0.0f).CompMul(-cascadeScale);
        }

        nsUInt32 uiAtlasScaleOffsetIndex = GET_ATLAS_SCALE_OFFSET_INDEX(shadowData.m_uiPackedDataOffset, uiViewIndex);
        nsVec4& atlasScaleOffset = packedShadowData[uiAtlasScaleOffsetIndex];

        nsRectU32 atlasRect = atlasRects[uiViewIndex];
        if (atlasRect.HasNonZeroArea())
        {
          nsVec2 scale = nsVec2(atlasRect.width * fAtlasInvWidth, atlasRect.height * fAtlasInvHeight);
          nsVec2 offset = nsVec2(atlasRect.x * fAtlasInvWidth, atlasRect.y * fAtlasInvHeight);

          // combine with tex scale offset
          atlasScaleOffset.x = scale.x * 0.5f;
          atlasScaleOffset.y = scale.y * -0.5f;
          atlasScaleOffset.z = offset.x + scale.x * 0.5f;
          atlasScaleOffset.w = offset.y + scale.y * 0.5f;
        }
        else
        {
          atlasScaleOffset.Set(1.0f, 1.0f, 0.0f, 0.0f);
        }
      }

      const nsCamera* pFirstCascadeCamera = shadowViews[0]->GetCamera();
      const nsCamera* pLastCascadeCamera = shadowViews[uiNumCascades - 1]->GetCamera();

      float cascadeSize = pFirstCascadeCamera->GetFovOrDim();
      float texelSize = 1.0f / uiShadowMapSize;
      float penumbraSize = nsMath::Max(shadowData.m_fPenumbraSize / cascadeSize, texelSize);
      float goodPenumbraSize = 8.0f / uiShadowMapSize;
      float relativeShadowSize = uiShadowMapSize * fAtlasInvHeight;

      // params
      {
        // tweak values to keep the default values consistent with spot and point lights
        float slopeBias = shadowData.m_fSlopeBias * nsMath::Max(penumbraSize, goodPenumbraSize);
        float constantBias = shadowData.m_fConstantBias * 0.2f;
        nsUInt32 uilastCascadeIndex = uiNumCascades - 1;

        nsUInt32 uiParamsIndex = GET_SHADOW_PARAMS_INDEX(shadowData.m_uiPackedDataOffset);
        nsVec4& shadowParams = packedShadowData[uiParamsIndex];
        shadowParams.x = slopeBias;
        shadowParams.y = constantBias;
        shadowParams.z = penumbraSize * relativeShadowSize;
        shadowParams.w = *reinterpret_cast<float*>(&uilastCascadeIndex);
      }

      // params2
      {
        float ditherMultiplier = 0.2f / cascadeSize;
        float zRange = cascadeSize / pFirstCascadeCamera->GetFarPlane();

        float actualPenumbraSize = shadowData.m_fPenumbraSize / pLastCascadeCamera->GetFovOrDim();
        float penumbraSizeIncrement = nsMath::Max(goodPenumbraSize - actualPenumbraSize, 0.0f) / shadowData.m_fMinRange;

        nsUInt32 uiParams2Index = GET_SHADOW_PARAMS2_INDEX(shadowData.m_uiPackedDataOffset);
        nsVec4& shadowParams2 = packedShadowData[uiParams2Index];
        shadowParams2.x = 1.0f - nsMath::Max(penumbraSize, goodPenumbraSize);
        shadowParams2.y = ditherMultiplier;
        shadowParams2.z = ditherMultiplier * zRange;
        shadowParams2.w = penumbraSizeIncrement * relativeShadowSize;
      }

      // fadeout
      {
        float fadeOutRange = 1.0f - shadowData.m_fFadeOutStart;
        float xyScale = -1.0f / fadeOutRange;
        float xyOffset = -xyScale;
        nsUInt32 xyScaleOffset = nsShaderUtils::PackFloat16intoUint(xyScale, xyOffset);

        float zFadeOutRange = fadeOutRange * pLastCascadeCamera->GetFovOrDim() / pLastCascadeCamera->GetFarPlane();
        float zScale = -1.0f / zFadeOutRange;
        float zOffset = -zScale;
        nsUInt32 zScaleOffset = nsShaderUtils::PackFloat16intoUint(zScale, zOffset);

        float distanceFadeOutRange = fadeOutRange * shadowData.m_fActualRange;
        float distanceScale = -1.0f / distanceFadeOutRange;
        float distanceOffset = -distanceScale * shadowData.m_fActualRange;

        nsUInt32 uiFadeOutIndex = GET_FADE_OUT_PARAMS_INDEX(shadowData.m_uiPackedDataOffset);
        nsVec4& fadeOutParams = packedShadowData[uiFadeOutIndex];
        fadeOutParams.x = *reinterpret_cast<float*>(&xyScaleOffset);
        fadeOutParams.y = *reinterpret_cast<float*>(&zScaleOffset);
        fadeOutParams.z = distanceScale;
        fadeOutParams.w = distanceOffset;
      }
    }
    else // spot or point light
    {
      nsMat4 texMatrix;
      texMatrix.SetIdentity();
      texMatrix.SetDiagonal(nsVec4(0.5f, -0.5f, 1.0f, 1.0f));
      texMatrix.SetTranslationVector(nsVec3(0.5f, 0.5f, 0.0f));

      nsAngle fov;

      for (nsUInt32 uiViewIndex = 0; uiViewIndex < shadowData.m_Views.GetCount(); ++uiViewIndex)
      {
        nsView* pShadowView = shadowViews[uiViewIndex];
        NS_ASSERT_DEV(pShadowView != nullptr, "Implementation error");

        nsUInt32 uiMatrixIndex = GET_WORLD_TO_LIGHT_MATRIX_INDEX(shadowData.m_uiPackedDataOffset, uiViewIndex);
        nsMat4& worldToLightMatrix = *reinterpret_cast<nsMat4*>(&packedShadowData[uiMatrixIndex]);

        nsRectU32 atlasRect = atlasRects[uiViewIndex];
        if (atlasRect.HasNonZeroArea())
        {
          nsVec2 scale = nsVec2(atlasRect.width * fAtlasInvWidth, atlasRect.height * fAtlasInvHeight);
          nsVec2 offset = nsVec2(atlasRect.x * fAtlasInvWidth, atlasRect.y * fAtlasInvHeight);

          nsMat4 atlasMatrix;
          atlasMatrix.SetIdentity();
          atlasMatrix.SetDiagonal(nsVec4(scale.x, scale.y, 1.0f, 1.0f));
          atlasMatrix.SetTranslationVector(offset.GetAsVec3(0.0f));

          fov = pShadowView->GetCamera()->GetFovY(1.0f);
          const nsMat4& viewProjection = pShadowView->GetViewProjectionMatrix(nsCameraEye::Left);

          worldToLightMatrix = atlasMatrix * texMatrix * viewProjection;
        }
        else
        {
          worldToLightMatrix.SetIdentity();
        }
      }

      float screenHeight = nsMath::Tan(fov * 0.5f) * 20.0f; // screen height in worldspace at 10m distance
      float texelSize = 1.0f / uiShadowMapSize;
      float penumbraSize = nsMath::Max(shadowData.m_fPenumbraSize / screenHeight, texelSize);
      float relativeShadowSize = uiShadowMapSize * fAtlasInvHeight;

      float slopeBias = shadowData.m_fSlopeBias * penumbraSize * nsMath::Tan(fov * 0.5f);
      float constantBias = shadowData.m_fConstantBias * cvar_RenderingShadowsMaxShadowMapSize / uiShadowMapSize;
      float fadeOut = nsMath::Clamp((shadowData.m_fShadowMapScale - fadeOutEnd) / (fadeOutStart - fadeOutEnd), 0.0f, 1.0f);

      nsUInt32 uiParamsIndex = GET_SHADOW_PARAMS_INDEX(shadowData.m_uiPackedDataOffset);
      nsVec4& shadowParams = packedShadowData[uiParamsIndex];
      shadowParams.x = slopeBias;
      shadowParams.y = constantBias;
      shadowParams.z = penumbraSize * relativeShadowSize;
      shadowParams.w = nsMath::Sqrt(fadeOut);
    }
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (cvar_RenderingShadowsShowPoolStats)
  {
    nsDebugRenderer::DrawInfoText(debugContext, nsDebugTextPlacement::TopLeft, "ShadowPoolStats", nsFmt("Atlas Utilization: {0}%%", nsArgF(100.0 * (double)uiUsedAtlasSize / uiTotalAtlasSize, 2)), nsColor::LightSteelBlue);
  }
#endif

  s_pData->Clear();
}

// static
void nsShadowPool::OnRenderEvent(const nsRenderWorldRenderEvent& e)
{
  if (e.m_Type != nsRenderWorldRenderEvent::Type::BeginRender)
    return;

  if (s_pData->m_hShadowAtlasTexture.IsInvalidated() || s_pData->m_hShadowDataBuffer.IsInvalidated())
    return;

  if (cvar_RenderingShadowsAtlasSize.HasDelayedSyncValueChanged() || cvar_RenderingShadowsMinShadowMapSize.HasDelayedSyncValueChanged() || cvar_RenderingShadowsMaxShadowMapSize.HasDelayedSyncValueChanged() || s_uiLastConfigModification != nsGameApplicationBase::GetGameApplicationBaseInstance()->GetPlatformProfile().GetLastModificationCounter())
  {
    OnEngineShutdown();
    OnEngineStartup();
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsGALPass* pGALPass = pDevice->BeginPass("Shadow Atlas");

  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(s_pData->m_hShadowAtlasTexture));
  renderingSetup.m_bClearDepth = true;

  auto pCommandEncoder = pGALPass->BeginRendering(renderingSetup);

  nsUInt32 uiDataIndex = nsRenderWorld::GetDataIndexForRendering();
  auto& packedShadowData = s_pData->m_PackedShadowData[uiDataIndex];
  if (!packedShadowData.IsEmpty())
  {
    NS_PROFILE_SCOPE("Shadow Data Buffer Update");

    pCommandEncoder->UpdateBuffer(s_pData->m_hShadowDataBuffer, 0, packedShadowData.GetByteArrayPtr());
  }

  pGALPass->EndRendering(pCommandEncoder);
  pDevice->EndPass(pGALPass);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ShadowPool);
