#pragma once

#include <Core/Scripting/ScriptCoroutine.h>
#include <Core/World/Declarations.h>
#include <Foundation/Math/CurveFunctions.h>

class NS_CORE_DLL nsScriptCoroutine_MoveTo : public nsTypedScriptCoroutine<nsScriptCoroutine_MoveTo, nsGameObjectHandle, nsVec3, nsTime, nsEnum<nsCurveFunction>>
{
public:
  void Start(nsGameObjectHandle hObject, const nsVec3& vTargetPos, nsTime duration, nsEnum<nsCurveFunction> easing);
  virtual Result Update(nsTime deltaTimeSinceLastUpdate) override;

private:
  nsGameObjectHandle m_hObject;
  nsVec3 m_vSourcePos;
  nsVec3 m_vTargetPos;
  nsEnum<nsCurveFunction> m_Easing;

  nsTime m_Duration;
  nsTime m_TimePassed;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutine_MoveTo);
