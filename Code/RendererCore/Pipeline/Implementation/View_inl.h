
NS_ALWAYS_INLINE nsViewHandle nsView::GetHandle() const
{
  return nsViewHandle(m_InternalId);
}

NS_ALWAYS_INLINE nsStringView nsView::GetName() const
{
  return m_sName.GetView();
}

NS_ALWAYS_INLINE nsWorld* nsView::GetWorld()
{
  return m_pWorld;
}

NS_ALWAYS_INLINE const nsWorld* nsView::GetWorld() const
{
  return m_pWorld;
}

NS_ALWAYS_INLINE nsGALSwapChainHandle nsView::GetSwapChain() const
{
  return m_Data.m_hSwapChain;
}

NS_ALWAYS_INLINE const nsGALRenderTargets& nsView::GetRenderTargets() const
{
  return m_Data.m_renderTargets;
}

NS_ALWAYS_INLINE void nsView::SetCamera(nsCamera* pCamera)
{
  m_pCamera = pCamera;
}

NS_ALWAYS_INLINE nsCamera* nsView::GetCamera()
{
  return m_pCamera;
}

NS_ALWAYS_INLINE const nsCamera* nsView::GetCamera() const
{
  return m_pCamera;
}

NS_ALWAYS_INLINE void nsView::SetCullingCamera(const nsCamera* pCamera)
{
  m_pCullingCamera = pCamera;
}

NS_ALWAYS_INLINE const nsCamera* nsView::GetCullingCamera() const
{
  return m_pCullingCamera != nullptr ? m_pCullingCamera : m_pCamera;
}

NS_ALWAYS_INLINE void nsView::SetLodCamera(const nsCamera* pCamera)
{
  m_pLodCamera = pCamera;
}

NS_ALWAYS_INLINE const nsCamera* nsView::GetLodCamera() const
{
  return m_pLodCamera != nullptr ? m_pLodCamera : m_pCamera;
}

NS_ALWAYS_INLINE nsEnum<nsCameraUsageHint> nsView::GetCameraUsageHint() const
{
  return m_Data.m_CameraUsageHint;
}

NS_ALWAYS_INLINE nsEnum<nsViewRenderMode> nsView::GetViewRenderMode() const
{
  return m_Data.m_ViewRenderMode;
}

NS_ALWAYS_INLINE const nsRectFloat& nsView::GetViewport() const
{
  return m_Data.m_ViewPortRect;
}

NS_ALWAYS_INLINE const nsViewData& nsView::GetData() const
{
  UpdateCachedMatrices();
  return m_Data;
}

NS_FORCE_INLINE bool nsView::IsValid() const
{
  return m_pWorld != nullptr && m_pRenderPipeline != nullptr && m_pCamera != nullptr && m_Data.m_ViewPortRect.HasNonZeroArea();
}

NS_ALWAYS_INLINE const nsSharedPtr<nsTask>& nsView::GetExtractTask()
{
  return m_pExtractTask;
}

NS_FORCE_INLINE nsResult nsView::ComputePickingRay(float fScreenPosX, float fScreenPosY, nsVec3& out_vRayStartPos, nsVec3& out_vRayDir) const
{
  UpdateCachedMatrices();
  return m_Data.ComputePickingRay(fScreenPosX, fScreenPosY, out_vRayStartPos, out_vRayDir);
}

NS_FORCE_INLINE nsResult nsView::ComputeScreenSpacePos(const nsVec3& vPoint, nsVec3& out_vScreenPos) const
{
  UpdateCachedMatrices();
  return m_Data.ComputeScreenSpacePos(vPoint, out_vScreenPos);
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetProjectionMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_ProjectionMatrix[static_cast<int>(eye)];
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetInverseProjectionMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_InverseProjectionMatrix[static_cast<int>(eye)];
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetViewMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_ViewMatrix[static_cast<int>(eye)];
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetInverseViewMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_InverseViewMatrix[static_cast<int>(eye)];
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetViewProjectionMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_ViewProjectionMatrix[static_cast<int>(eye)];
}

NS_ALWAYS_INLINE const nsMat4& nsView::GetInverseViewProjectionMatrix(nsCameraEye eye) const
{
  UpdateCachedMatrices();
  return m_Data.m_InverseViewProjectionMatrix[static_cast<int>(eye)];
}
