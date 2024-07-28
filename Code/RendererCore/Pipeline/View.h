#pragma once

#include <Foundation/Strings/HashedString.h>
#include <Foundation/Threading/DelegateTask.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/TagSet.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelineNode.h>
#include <RendererCore/Pipeline/RenderPipelineResource.h>
#include <RendererCore/Pipeline/ViewData.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>

class nsFrustum;
class nsWorld;
class nsRenderPipeline;

/// \brief Encapsulates a view on the given world through the given camera
/// and rendered with the specified RenderPipeline into the given render target setup.
class NS_RENDERERCORE_DLL nsView : public nsRenderPipelineNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsView, nsRenderPipelineNode);

private:
  /// \brief Use nsRenderLoop::CreateView to create a view.
  nsView();
  ~nsView();

public:
  nsViewHandle GetHandle() const;

  void SetName(nsStringView sName);
  nsStringView GetName() const;

  void SetWorld(nsWorld* pWorld);
  nsWorld* GetWorld();
  const nsWorld* GetWorld() const;

  /// \brief Sets the swapchain that this view will be rendering into. Can be invalid in case the render target is an off-screen buffer in which case SetRenderTargets needs to be called.
  /// Setting the swap-chain is necessary in order to acquire and present the image to the window.
  /// SetSwapChain and SetRenderTargets are mutually exclusive. Calling this function will reset the render targets.
  void SetSwapChain(nsGALSwapChainHandle hSwapChain);
  nsGALSwapChainHandle GetSwapChain() const;

  /// \brief Sets the off-screen render targets. Use SetSwapChain if rendering to a window.
  /// SetSwapChain and SetRenderTargets are mutually exclusive. Calling this function will reset the swap chain.
  void SetRenderTargets(const nsGALRenderTargets& renderTargets);
  const nsGALRenderTargets& GetRenderTargets() const;

  /// \brief Returns the render targets that were either set via the swapchain or via the manually set render targets.
  const nsGALRenderTargets& GetActiveRenderTargets() const;

  void SetRenderPipelineResource(nsRenderPipelineResourceHandle hPipeline);
  nsRenderPipelineResourceHandle GetRenderPipelineResource() const;

  void SetCamera(nsCamera* pCamera);
  nsCamera* GetCamera();
  const nsCamera* GetCamera() const;

  void SetCullingCamera(const nsCamera* pCamera);
  const nsCamera* GetCullingCamera() const;

  void SetLodCamera(const nsCamera* pCamera);
  const nsCamera* GetLodCamera() const;

  /// \brief Returns the camera usage hint for the view.
  nsEnum<nsCameraUsageHint> GetCameraUsageHint() const;
  /// \brief Sets the camera usage hint for the view. If not 'None', the camera component of the same usage will be auto-connected
  ///   to this view.
  void SetCameraUsageHint(nsEnum<nsCameraUsageHint> val);

  void SetViewRenderMode(nsEnum<nsViewRenderMode> value);
  nsEnum<nsViewRenderMode> GetViewRenderMode() const;

  void SetViewport(const nsRectFloat& viewport);
  const nsRectFloat& GetViewport() const;

  /// \brief Forces the render pipeline to be rebuilt.
  void ForceUpdate();

  const nsViewData& GetData() const;

  bool IsValid() const;

  /// \brief Extracts all relevant data from the world to render the view.
  void ExtractData();

  /// \brief Returns a task implementation that calls ExtractData on this view.
  const nsSharedPtr<nsTask>& GetExtractTask();


  /// \brief Returns the start position and direction (in world space) of the picking ray through the screen position in this view.
  ///
  /// fScreenPosX and fScreenPosY are expected to be in [0; 1] range (normalized pixel coordinates).
  /// If no ray can be computed, NS_FAILURE is returned.
  nsResult ComputePickingRay(float fScreenPosX, float fScreenPosY, nsVec3& out_vRayStartPos, nsVec3& out_vRayDir) const;

  nsResult ComputeScreenSpacePos(const nsVec3& vPoint, nsVec3& out_vScreenPos) const;

  /// \brief Returns the current projection matrix.
  const nsMat4& GetProjectionMatrix(nsCameraEye eye) const;

  /// \brief Returns the current inverse projection matrix.
  const nsMat4& GetInverseProjectionMatrix(nsCameraEye eye) const;

  /// \brief Returns the current view matrix (camera orientation).
  const nsMat4& GetViewMatrix(nsCameraEye eye) const;

  /// \brief Returns the current inverse view matrix (inverse camera orientation).
  const nsMat4& GetInverseViewMatrix(nsCameraEye eye) const;

  /// \brief Returns the current view-projection matrix.
  const nsMat4& GetViewProjectionMatrix(nsCameraEye eye) const;

  /// \brief Returns the current inverse view-projection matrix.
  const nsMat4& GetInverseViewProjectionMatrix(nsCameraEye eye) const;

  /// \brief Returns the frustum that should be used for determine visible objects for this view.
  void ComputeCullingFrustum(nsFrustum& out_frustum) const;

  void SetShaderPermutationVariable(const char* szName, const char* szValue);

  void SetRenderPassProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value);
  void SetExtractorProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value);

  void ResetRenderPassProperties();
  void ResetExtractorProperties();

  void SetRenderPassReadBackProperty(const char* szPassName, const char* szPropertyName, const nsVariant& value);
  nsVariant GetRenderPassReadBackProperty(const char* szPassName, const char* szPropertyName);
  bool IsRenderPassReadBackPropertyExisting(const char* szPassName, const char* szPropertyName) const;

  /// \brief Pushes the view and camera data into the extracted data of the pipeline.
  ///
  /// Use nsRenderWorld::GetDataIndexForExtraction() to update the data from the extraction thread. Can't be used if this view is currently extracted.
  /// Use nsRenderWorld::GetDataIndexForRendering() to update the data from the render thread.
  void UpdateViewData(nsUInt32 uiDataIndex);

  nsTagSet m_IncludeTags;
  nsTagSet m_ExcludeTags;

private:
  friend class nsRenderWorld;
  friend class nsMemoryUtils;

  nsViewId m_InternalId;
  nsHashedString m_sName;

  nsSharedPtr<nsTask> m_pExtractTask;

  nsWorld* m_pWorld = nullptr;

  nsRenderPipelineResourceHandle m_hRenderPipeline;
  nsUInt32 m_uiRenderPipelineResourceDescriptionCounter = 0;
  nsSharedPtr<nsRenderPipeline> m_pRenderPipeline;
  nsCamera* m_pCamera = nullptr;
  const nsCamera* m_pCullingCamera = nullptr;
  const nsCamera* m_pLodCamera = nullptr;


private:
  nsRenderPipelineNodeInputPin m_PinRenderTarget0;
  nsRenderPipelineNodeInputPin m_PinRenderTarget1;
  nsRenderPipelineNodeInputPin m_PinRenderTarget2;
  nsRenderPipelineNodeInputPin m_PinRenderTarget3;
  nsRenderPipelineNodeInputPin m_PinDepthStencil;

private:
  void UpdateCachedMatrices() const;

  /// \brief Rebuilds pipeline if necessary and pushes double-buffered settings into the pipeline.
  void EnsureUpToDate();

  mutable nsUInt32 m_uiLastCameraSettingsModification = 0;
  mutable nsUInt32 m_uiLastCameraOrientationModification = 0;
  mutable float m_fLastViewportAspectRatio = 1.0f;

  mutable nsViewData m_Data;

  nsInternal::RenderDataCache* m_pRenderDataCache = nullptr;

  nsDynamicArray<nsPermutationVar> m_PermutationVars;
  bool m_bPermutationVarsDirty = false;

  void ApplyPermutationVars();

  struct PropertyValue
  {
    nsString m_sObjectName;
    nsString m_sPropertyName;
    nsVariant m_DefaultValue;
    nsVariant m_CurrentValue;
    bool m_bIsValid;
    bool m_bIsDirty;
  };

  void SetProperty(nsMap<nsString, PropertyValue>& map, const char* szPassName, const char* szPropertyName, const nsVariant& value);
  void SetReadBackProperty(nsMap<nsString, PropertyValue>& map, const char* szPassName, const char* szPropertyName, const nsVariant& value);

  void ReadBackPassProperties();

  void ResetAllPropertyStates(nsMap<nsString, PropertyValue>& map);

  void ApplyRenderPassProperties();
  void ApplyExtractorProperties();

  void ApplyProperty(nsReflectedClass* pObject, PropertyValue& data, const char* szTypeName);

  nsMap<nsString, PropertyValue> m_PassProperties;
  nsMap<nsString, PropertyValue> m_PassReadBackProperties;
  nsMap<nsString, PropertyValue> m_ExtractorProperties;
};

#include <RendererCore/Pipeline/Implementation/View_inl.h>
