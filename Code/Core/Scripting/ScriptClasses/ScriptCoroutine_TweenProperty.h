#pragma once

#include <Core/Scripting/ScriptCoroutine.h>
#include <Core/World/Declarations.h>
#include <Foundation/Math/CurveFunctions.h>

class NS_CORE_DLL nsScriptCoroutine_TweenProperty : public nsTypedScriptCoroutine<nsScriptCoroutine_TweenProperty, nsComponentHandle, nsStringView, nsVariant, nsTime, nsEnum<nsCurveFunction>>
{
public:
  void Start(nsComponentHandle hComponent, nsStringView sPropertyName, nsVariant targetValue, nsTime duration, nsEnum<nsCurveFunction> easing);
  virtual Result Update(nsTime deltaTimeSinceLastUpdate) override;

private:
  const nsAbstractMemberProperty* m_pProperty = nullptr;
  nsComponentHandle m_hComponent;
  nsVariant m_SourceValue;
  nsVariant m_TargetValue;
  nsEnum<nsCurveFunction> m_Easing;

  nsTime m_Duration;
  nsTime m_TimePassed;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutine_TweenProperty);
