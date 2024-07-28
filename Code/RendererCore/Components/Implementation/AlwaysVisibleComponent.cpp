#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/AlwaysVisibleComponent.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsAlwaysVisibleComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE;
// clang-format on

nsAlwaysVisibleComponent::nsAlwaysVisibleComponent() = default;
nsAlwaysVisibleComponent::~nsAlwaysVisibleComponent() = default;

nsResult nsAlwaysVisibleComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  ref_bAlwaysVisible = true;
  return NS_SUCCESS;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_AlwaysVisibleComponent);
