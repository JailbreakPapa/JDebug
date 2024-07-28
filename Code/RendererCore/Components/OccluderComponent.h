#pragma once

#include <Core/World/Component.h>
#include <Core/World/World.h>
#include <RendererCore/Rasterizer/RasterizerObject.h>
#include <RendererCore/RendererCoreDLL.h>

struct nsMsgTransformChanged;
struct nsMsgUpdateLocalBounds;
struct nsMsgExtractOccluderData;

class NS_RENDERERCORE_DLL nsOccluderComponentManager final : public nsComponentManager<class nsOccluderComponent, nsBlockStorageType::FreeList>
{
public:
  nsOccluderComponentManager(nsWorld* pWorld);
};

/// \brief Adds invisible geometry to a scene that is used for occlusion culling.
///
/// The component adds a box occluder to the scene. The renderer uses this geometry
/// to cull other objects which are behind occluder geometry. Use occluder components to optimize levels.
/// Make the shapes conservative, meaning that they shouldn't be bigger than the actual shapes, otherwise
/// they may incorrectly occlude other objects and lead to incorrectly culled objects.
///
/// The nsGreyBoxComponent can also create occluder geometry in different shapes.
///
/// Contrary to nsGreyBoxComponent, occluder components can be moved around dynamically and thus can be attached to
/// doors and other objects that may dynamically change the visible areas of a level.
class NS_RENDERERCORE_DLL nsOccluderComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsOccluderComponent, nsComponent, nsOccluderComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // nsOccluderComponent

public:
  nsOccluderComponent();
  ~nsOccluderComponent();

  /// \brief Sets the size of the box occluder.
  void SetExtents(const nsVec3& vExtents);                // [ property ]
  const nsVec3& GetExtents() const { return m_vExtents; } // [ property ]

private:
  nsVec3 m_vExtents = nsVec3(5.0f);

  mutable nsSharedPtr<const nsRasterizerObject> m_pOccluderObject;

  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void OnMsgExtractOccluderData(nsMsgExtractOccluderData& msg) const;
};
