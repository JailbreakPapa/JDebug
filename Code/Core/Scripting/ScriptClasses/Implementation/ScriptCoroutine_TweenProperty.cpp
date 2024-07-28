#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptCoroutine_TweenProperty.h>
#include <Core/World/World.h>
#include <Foundation/Reflection/ReflectionUtils.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptCoroutine_TweenProperty, nsScriptCoroutine, 1, nsRTTIDefaultAllocator<nsScriptCoroutine_TweenProperty>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(Start, In, "Component", In, "PropertyName", In, "TargetValue", In, "Duration", In, "Easing"),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Coroutine::TweenProperty {PropertyName}"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsScriptCoroutine_TweenProperty::Start(nsComponentHandle hComponent, nsStringView sPropertyName, nsVariant targetValue, nsTime duration, nsEnum<nsCurveFunction> easing)
{
  nsComponent* pComponent = nullptr;
  if (nsWorld::GetWorld(hComponent)->TryGetComponent(hComponent, pComponent) == false)
  {
    nsLog::Error("TweenProperty: The given component was not found.");
    return;
  }

  auto pType = pComponent->GetDynamicRTTI();
  auto pProp = pType->FindPropertyByName(sPropertyName);
  if (pProp == nullptr || pProp->GetCategory() != nsPropertyCategory::Member)
  {
    nsLog::Error("TweenProperty: The given component of type '{}' does not have a member property named '{}'.", pType->GetTypeName(), sPropertyName);
    return;
  }

  nsVariantType::Enum variantType = pProp->GetSpecificType()->GetVariantType();
  if (variantType == nsVariantType::Invalid)
  {
    nsLog::Error("TweenProperty: Can't tween property '{}' of type '{}'.", sPropertyName, pProp->GetSpecificType()->GetTypeName());
    return;
  }

  nsResult conversionStatus = NS_SUCCESS;
  m_TargetValue = targetValue.ConvertTo(variantType, &conversionStatus);
  if (conversionStatus.Failed())
  {
    nsLog::Error("TweenProperty: Can't convert given target value to '{}'.", pProp->GetSpecificType()->GetTypeName());
    return;
  }

  m_pProperty = static_cast<const nsAbstractMemberProperty*>(pProp);
  m_hComponent = hComponent;
  m_SourceValue = nsReflectionUtils::GetMemberPropertyValue(m_pProperty, pComponent);
  m_Easing = easing;

  m_Duration = duration;
  m_TimePassed = nsTime::MakeZero();
}

nsScriptCoroutine::Result nsScriptCoroutine_TweenProperty::Update(nsTime deltaTimeSinceLastUpdate)
{
  if (m_pProperty == nullptr)
  {
    return Result::Failed();
  }

  if (deltaTimeSinceLastUpdate.IsPositive())
  {
    nsComponent* pComponent = nullptr;
    if (nsWorld::GetWorld(m_hComponent)->TryGetComponent(m_hComponent, pComponent) == false)
    {
      return Result::Failed();
    }

    m_TimePassed += deltaTimeSinceLastUpdate;

    const double fDuration = m_Duration.GetSeconds();
    double fCurrentX = nsMath::Min(fDuration > 0 ? m_TimePassed.GetSeconds() / fDuration : 1.0, 1.0);
    fCurrentX = nsCurveFunction::GetValue(m_Easing, fCurrentX);
    nsVariant currentValue = nsMath::Lerp(m_SourceValue, m_TargetValue, fCurrentX);

    nsReflectionUtils::SetMemberPropertyValue(m_pProperty, pComponent, currentValue);
  }

  if (m_TimePassed < m_Duration)
  {
    return Result::Running();
  }

  return Result::Completed();
}


NS_STATICLINK_FILE(Core, Core_Scripting_ScriptClasses_Implementation_ScriptCoroutine_TweenProperty);
