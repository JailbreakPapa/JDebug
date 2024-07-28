#pragma once

#include <RendererCore/Meshes/MeshComponentBase.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/ObjectConstants.h>

namespace nsInternal
{
  NS_FORCE_INLINE void FillPerInstanceData(nsPerInstanceData& ref_perInstanceData, const nsMeshRenderData* pRenderData)
  {
    nsMat4 objectToWorld = pRenderData->m_GlobalTransform.GetAsMat4();

    ref_perInstanceData.ObjectToWorld = objectToWorld;

    if (pRenderData->m_uiUniformScale)
    {
      ref_perInstanceData.ObjectToWorldNormal = objectToWorld;
    }
    else
    {
      nsMat3 mInverse = objectToWorld.GetRotationalPart();
      mInverse.Invert(0.0f).IgnoreResult();
      // we explicitly ignore the return value here (success / failure)
      // because when we have a scale of 0 (which happens temporarily during editing) that would be annoying

      nsShaderTransform shaderT;
      shaderT = mInverse.GetTranspose();
      ref_perInstanceData.ObjectToWorldNormal = shaderT;
    }

    ref_perInstanceData.BoundingSphereRadius = pRenderData->m_GlobalBounds.m_fSphereRadius;
    ref_perInstanceData.GameObjectID = pRenderData->m_uiUniqueID;
    ref_perInstanceData.VertexColorAccessData = 0;
    ref_perInstanceData.Color = pRenderData->m_Color;
    ref_perInstanceData.CustomData = pRenderData->m_vCustomData;
  }
} // namespace nsInternal
