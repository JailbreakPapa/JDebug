#pragma once

#include <Core/Scripting/ScriptCoroutine.h>

class NS_CORE_DLL nsScriptCoroutine_Wait : public nsTypedScriptCoroutine<nsScriptCoroutine_Wait, nsTime>
{
public:
  void Start(nsTime timeout);
  virtual Result Update(nsTime deltaTimeSinceLastUpdate) override;

private:
  nsTime m_TimeRemaing;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsScriptCoroutine_Wait);
