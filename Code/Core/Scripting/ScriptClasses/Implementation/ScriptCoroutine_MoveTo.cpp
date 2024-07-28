#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptClasses/ScriptCoroutine_MoveTo.h>
#include <Core/World/World.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsScriptCoroutine_MoveTo, nsScriptCoroutine, 1, nsRTTIDefaultAllocator<nsScriptCoroutine_MoveTo>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(Start, In, "Object", In, "TargetPos", In, "Duration", In, "Easing"),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Coroutine::MoveTo {TargetPos}"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsScriptCoroutine_MoveTo::Start(nsGameObjectHandle hObject, const nsVec3& vTargetPos, nsTime duration, nsEnum<nsCurveFunction> easing)
{
  nsGameObject* pObject = nullptr;
  if (nsWorld::GetWorld(hObject)->TryGetObject(hObject, pObject) == false)
  {
    nsLog::Error("MoveTo: The given game object was not found.");
    return;
  }

  m_hObject = hObject;
  m_vSourcePos = pObject->GetLocalPosition();
  m_vTargetPos = vTargetPos;
  m_Easing = easing;

  m_Duration = duration;
  m_TimePassed = nsTime::MakeZero();
}

nsScriptCoroutine::Result nsScriptCoroutine_MoveTo::Update(nsTime deltaTimeSinceLastUpdate)
{
  if (deltaTimeSinceLastUpdate.IsPositive())
  {
    nsGameObject* pObject = nullptr;
    if (nsWorld::GetWorld(m_hObject)->TryGetObject(m_hObject, pObject) == false)
    {
      return Result::Failed();
    }

    m_TimePassed += deltaTimeSinceLastUpdate;

    const double fDuration = m_Duration.GetSeconds();
    double fCurrentX = nsMath::Min(fDuration > 0 ? m_TimePassed.GetSeconds() / fDuration : 1.0, 1.0);
    fCurrentX = nsCurveFunction::GetValue(m_Easing, fCurrentX);

    nsVec3 vCurrentPos = nsMath::Lerp(m_vSourcePos, m_vTargetPos, static_cast<float>(fCurrentX));
    pObject->SetLocalPosition(vCurrentPos);
  }

  if (m_TimePassed < m_Duration)
  {
    return Result::Running();
  }

  return Result::Completed();
}


NS_STATICLINK_FILE(Core, Core_Scripting_ScriptClasses_Implementation_ScriptCoroutine_MoveTo);
