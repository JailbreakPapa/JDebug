#pragma once

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/World/World.h>
#include <RendererCore/RendererCoreDLL.h>

/// \brief Base class for objects that should be rendered.
class NS_RENDERERCORE_DLL nsRenderComponent : public nsComponent
{
  NS_DECLARE_ABSTRACT_COMPONENT_TYPE(nsRenderComponent, nsComponent);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

protected:
  virtual void Deinitialize() override;
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  nsRenderComponent();
  ~nsRenderComponent();

  /// \brief Called by nsRenderComponent::OnUpdateLocalBounds().
  ///
  /// If NS_SUCCESS is returned, out_bounds and out_bAlwaysVisible will be integrated into the nsMsgUpdateLocalBounds ref_msg,
  /// otherwise the out values are simply ignored.
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) = 0;

  /// \brief Call this when some value was modified that affects the size of the local bounding box and it should be recomputed.
  void TriggerLocalBoundsUpdate();

  /// \brief Computes a unique ID for the given component, that is usually given to the renderer to distinguish objects.
  static nsUInt32 GetUniqueIdForRendering(const nsComponent& component, nsUInt32 uiInnerIndex = 0, nsUInt32 uiInnerIndexShift = 24);

  /// \brief Computes a unique ID for the given component, that is usually given to the renderer to distinguish objects.
  NS_ALWAYS_INLINE nsUInt32 GetUniqueIdForRendering(nsUInt32 uiInnerIndex = 0, nsUInt32 uiInnerIndexShift = 24) const
  {
    return GetUniqueIdForRendering(*this, uiInnerIndex, uiInnerIndexShift);
  }

protected:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void InvalidateCachedRenderData();
};
