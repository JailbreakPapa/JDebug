#pragma once

#include <Core/Graphics/Camera.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Pipeline/ViewRenderMode.h>
#include <RendererFoundation/Device/SwapChain.h>

/// \brief Holds view data like the viewport, view and projection matrices
struct NS_RENDERERCORE_DLL nsViewData
{
  nsViewData()
  {
    m_ViewPortRect = nsRectFloat(0.0f, 0.0f);
    m_ViewRenderMode = nsViewRenderMode::None;

    for (int i = 0; i < 2; ++i)
    {
      m_ViewMatrix[i].SetIdentity();
      m_InverseViewMatrix[i].SetIdentity();
      m_ProjectionMatrix[i].SetIdentity();
      m_InverseProjectionMatrix[i].SetIdentity();
      m_ViewProjectionMatrix[i].SetIdentity();
      m_InverseViewProjectionMatrix[i].SetIdentity();
    }
  }

  nsGALRenderTargets m_renderTargets;
  nsGALSwapChainHandle m_hSwapChain;
  nsRectFloat m_ViewPortRect;
  nsEnum<nsViewRenderMode> m_ViewRenderMode;
  nsEnum<nsCameraUsageHint> m_CameraUsageHint;

  // Each matrix is there for both left and right camera lens.
  nsMat4 m_ViewMatrix[2];
  nsMat4 m_InverseViewMatrix[2];
  nsMat4 m_ProjectionMatrix[2];
  nsMat4 m_InverseProjectionMatrix[2];
  nsMat4 m_ViewProjectionMatrix[2];
  nsMat4 m_InverseViewProjectionMatrix[2];

  /// \brief Returns the start position and direction (in world space) of the picking ray through the screen position in this view.
  ///
  /// fScreenPosX and fScreenPosY are expected to be in [0; 1] range (normalized pixel coordinates).
  /// If no ray can be computed, NS_FAILURE is returned.
  nsResult ComputePickingRay(
    float fScreenPosX, float fScreenPosY, nsVec3& out_vRayStartPos, nsVec3& out_vRayDir, nsCameraEye eye = nsCameraEye::Left) const
  {
    nsVec3 vScreenPos;
    vScreenPos.x = fScreenPosX;
    vScreenPos.y = 1.0f - fScreenPosY;
    vScreenPos.z = 0.0f;

    return nsGraphicsUtils::ConvertScreenPosToWorldPos(
      m_InverseViewProjectionMatrix[static_cast<int>(eye)], 0, 0, 1, 1, vScreenPos, out_vRayStartPos, &out_vRayDir);
  }

  nsResult ComputeScreenSpacePos(const nsVec3& vPoint, nsVec3& out_vScreenPos, nsCameraEye eye = nsCameraEye::Left) const
  {
    nsUInt32 x = (nsUInt32)m_ViewPortRect.x;
    nsUInt32 y = (nsUInt32)m_ViewPortRect.y;
    nsUInt32 w = (nsUInt32)m_ViewPortRect.width;
    nsUInt32 h = (nsUInt32)m_ViewPortRect.height;

    if (nsGraphicsUtils::ConvertWorldPosToScreenPos(m_ViewProjectionMatrix[static_cast<int>(eye)], x, y, w, h, vPoint, out_vScreenPos).Succeeded())
    {
      out_vScreenPos.y = m_ViewPortRect.height - out_vScreenPos.y;

      return NS_SUCCESS;
    }

    return NS_FAILURE;
  }
};
