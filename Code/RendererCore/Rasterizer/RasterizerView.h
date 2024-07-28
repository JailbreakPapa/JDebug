#pragma once

#include <Foundation/Containers/Deque.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Types/ArrayPtr.h>
#include <RendererCore/RendererCoreDLL.h>

class Rasterizer;
class nsRasterizerObject;
class nsColorLinearUB;
class nsCamera;
class nsSimdBBox;

class NS_RENDERERCORE_DLL nsRasterizerView final
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsRasterizerView);

public:
  nsRasterizerView();
  ~nsRasterizerView();

  /// \brief Changes the resolution of the view. Has to be called at least once before starting to render anything.
  void SetResolution(nsUInt32 uiWidth, nsUInt32 uiHeight, float fAspectRatio);

  nsUInt32 GetResolutionX() const { return m_uiResolutionX; }
  nsUInt32 GetResolutionY() const { return m_uiResolutionY; }

  /// \brief Prepares the view to rasterize a new scene.
  void BeginScene();

  /// \brief Finishes rasterizing the scene. Visibility queries only work after this.
  void EndScene();

  /// \brief Writes an RGBA8 representation of the depth values to targetBuffer.
  ///
  /// The buffer must be large enough for the chosen resolution.
  void ReadBackFrame(nsArrayPtr<nsColorLinearUB> targetBuffer) const;

  /// \brief Sets the camera from which to extract the rendering position, direction and field-of-view.
  void SetCamera(const nsCamera* pCamera)
  {
    m_pCamera = pCamera;
  }

  /// \brief Adds an object as an occluder to the scene. Once all occluders have been rasterized, visibility queries can be done.
  void AddObject(const nsRasterizerObject* pObject, const nsTransform& transform)
  {
    auto& inst = m_Instances.ExpandAndGetRef();
    inst.m_pObject = pObject;
    inst.m_Transform = transform;
  }

  /// \brief Checks whether a box would be visible, or is fully occluded by the existing scene geometry.
  ///
  /// Note: This only works after EndScene().
  bool IsVisible(const nsSimdBBox& aabb) const;

  /// \brief Wether any occluder was actually added and also rasterized. If not, no need to do any visibility checks.
  bool HasRasterizedAnyOccluders() const
  {
    return m_bAnyOccludersRasterized;
  }

private:
  void SortObjectsFrontToBack();
  void RasterizeObjects(nsUInt32 uiMaxObjects);
  void UpdateViewProjectionMatrix();
  void ApplyModelViewProjectionMatrix(const nsTransform& modelTransform);

  bool m_bAnyOccludersRasterized = false;
  const nsCamera* m_pCamera = nullptr;
  nsUInt32 m_uiResolutionX = 0;
  nsUInt32 m_uiResolutionY = 0;
  float m_fAspectRation = 1.0f;
  nsUniquePtr<Rasterizer> m_pRasterizer;

  struct Instance
  {
    nsTransform m_Transform;
    const nsRasterizerObject* m_pObject;
  };

  nsDeque<Instance> m_Instances;
  nsMat4 m_mViewProjection;
};

class nsRasterizerViewPool
{
public:
  nsRasterizerView* GetRasterizerView(nsUInt32 uiWidth, nsUInt32 uiHeight, float fAspectRatio);
  void ReturnRasterizerView(nsRasterizerView* pView);

private:
  struct PoolEntry
  {
    bool m_bInUse = false;
    nsRasterizerView m_RasterizerView;
  };

  nsMutex m_Mutex;
  nsDeque<PoolEntry> m_Entries;
};
