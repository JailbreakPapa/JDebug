#include <Core/CorePCH.h>

#include <Core/World/SettingsComponent.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSettingsComponent, 1, nsRTTINoAllocator)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Settings"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSettingsComponent::nsSettingsComponent()
{
  SetModified();
}

nsSettingsComponent::~nsSettingsComponent() = default;


NS_STATICLINK_FILE(Core, Core_World_Implementation_SettingsComponent);
