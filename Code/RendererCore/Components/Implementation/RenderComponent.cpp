#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
NS_BEGIN_ABSTRACT_COMPONENT_TYPE(nsRenderComponent, 1)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds)
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

nsRenderComponent::nsRenderComponent() = default;
nsRenderComponent::~nsRenderComponent() = default;

void nsRenderComponent::Deinitialize()
{
  nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());

  SUPER::Deinitialize();
}

void nsRenderComponent::OnActivated()
{
  TriggerLocalBoundsUpdate();
}

void nsRenderComponent::OnDeactivated()
{
  // Can't call TriggerLocalBoundsUpdate because it checks whether we are active, which is not the case anymore.
  GetOwner()->UpdateLocalBounds();
}

void nsRenderComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  nsBoundingBoxSphere bounds = nsBoundingBoxSphere::MakeInvalid();

  bool bAlwaysVisible = false;

  if (GetLocalBounds(bounds, bAlwaysVisible, msg).Succeeded())
  {
    nsSpatialData::Category category = GetOwner()->IsDynamic() ? nsDefaultSpatialDataCategories::RenderDynamic : nsDefaultSpatialDataCategories::RenderStatic;

    if (bounds.IsValid())
    {
      msg.AddBounds(bounds, category);
    }

    if (bAlwaysVisible)
    {
      msg.SetAlwaysVisible(category);
    }
  }
}

void nsRenderComponent::InvalidateCachedRenderData()
{
  if (IsActiveAndInitialized())
  {
    nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());
  }
}

void nsRenderComponent::TriggerLocalBoundsUpdate()
{
  if (IsActiveAndInitialized())
  {
    GetOwner()->UpdateLocalBounds();
  }
}

// static
nsUInt32 nsRenderComponent::GetUniqueIdForRendering(const nsComponent& component, nsUInt32 uiInnerIndex /*= 0*/, nsUInt32 uiInnerIndexShift /*= 24*/)
{
  nsUInt32 uniqueId = component.GetUniqueID();
  if (uniqueId == nsInvalidIndex)
  {
    uniqueId = component.GetOwner()->GetHandle().GetInternalID().m_InstanceIndex;
  }
  else
  {
    uniqueId |= (uiInnerIndex << uiInnerIndexShift);
  }

  const nsUInt32 dynamicBit = (1 << 31);
  const nsUInt32 dynamicBitMask = ~dynamicBit;
  return (uniqueId & dynamicBitMask) | (component.GetOwner()->IsDynamic() ? dynamicBit : 0);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_RenderComponent);
