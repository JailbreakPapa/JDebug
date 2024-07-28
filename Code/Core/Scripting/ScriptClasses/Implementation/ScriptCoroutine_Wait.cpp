#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptCoroutine_Wait.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptCoroutine_Wait, nsScriptCoroutine, 1, nsRTTIDefaultAllocator<nsScriptCoroutine_Wait>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(Start, In, "Timeout"),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Coroutine::Wait {Timeout}"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsScriptCoroutine_Wait::Start(nsTime timeout)
{
  m_TimeRemaing = timeout;
}

nsScriptCoroutine::Result nsScriptCoroutine_Wait::Update(nsTime deltaTimeSinceLastUpdate)
{
  m_TimeRemaing -= deltaTimeSinceLastUpdate;
  if (m_TimeRemaing.IsPositive())
  {
    // Don't wait for the full remaining time to prevent oversleeping due to scheduling precision.
    return Result::Running(m_TimeRemaing * 0.8);
  }

  return Result::Completed();
}


NS_STATICLINK_FILE(Core, Core_Scripting_ScriptClasses_Implementation_ScriptCoroutine_Wait);
