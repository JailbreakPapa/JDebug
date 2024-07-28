#pragma once

#include <RendererCore/Components/RenderComponent.h>

using nsAlwaysVisibleComponentManager = nsComponentManager<class nsAlwaysVisibleComponent, nsBlockStorageType::Compact>;

/// \brief Attaching this component to a game object makes the renderer consider it always visible, ie. disables culling
class NS_RENDERERCORE_DLL nsAlwaysVisibleComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsAlwaysVisibleComponent, nsRenderComponent, nsAlwaysVisibleComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // nsAlwaysVisibleComponent

public:
  nsAlwaysVisibleComponent();
  ~nsAlwaysVisibleComponent();
};
