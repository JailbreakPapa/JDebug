#pragma once

#include <Foundation/Configuration/CVar.h>

template <typename Type, wdCVarType::Enum CVarType>
wdTypedCVar<Type, CVarType>::wdTypedCVar(wdStringView sName, const Type& value, wdBitflags<wdCVarFlags> flags, wdStringView sDescription)
  : wdCVar(sName, flags, sDescription)
{
  WD_ASSERT_DEBUG(sName.FindSubString(" ") == nullptr, "CVar names must not contain whitespace");

  for (wdUInt32 i = 0; i < wdCVarValue::ENUM_COUNT; ++i)
    m_Values[i] = value;
}

template <typename Type, wdCVarType::Enum CVarType>
wdTypedCVar<Type, CVarType>::operator const Type&() const
{
  return (m_Values[wdCVarValue::Current]);
}

template <typename Type, wdCVarType::Enum CVarType>
wdCVarType::Enum wdTypedCVar<Type, CVarType>::GetType() const
{
  return CVarType;
}

template <typename Type, wdCVarType::Enum CVarType>
void wdTypedCVar<Type, CVarType>::SetToRestartValue()
{
  if (m_Values[wdCVarValue::Current] == m_Values[wdCVarValue::Restart])
    return;

  // this will NOT trigger a 'restart value changed' event
  m_Values[wdCVarValue::Current] = m_Values[wdCVarValue::Restart];

  wdCVarEvent e(this);
  e.m_EventType = wdCVarEvent::ValueChanged;
  m_CVarEvents.Broadcast(e);

  // broadcast the same to the 'all cvars' event handlers
  s_AllCVarEvents.Broadcast(e);
}

template <typename Type, wdCVarType::Enum CVarType>
const Type& wdTypedCVar<Type, CVarType>::GetValue(wdCVarValue::Enum val) const
{
  return (m_Values[val]);
}

template <typename Type, wdCVarType::Enum CVarType>
void wdTypedCVar<Type, CVarType>::operator=(const Type& value)
{
  wdCVarEvent e(this);

  if (GetFlags().IsAnySet(wdCVarFlags::RequiresRestart))
  {
    if (value == m_Values[wdCVarValue::Restart]) // no change
      return;

    e.m_EventType = wdCVarEvent::RestartValueChanged;
  }
  else
  {
    if (m_Values[wdCVarValue::Current] == value) // no change
      return;

    m_Values[wdCVarValue::Current] = value;
    e.m_EventType = wdCVarEvent::ValueChanged;
  }

  m_Values[wdCVarValue::Restart] = value;

  m_CVarEvents.Broadcast(e);

  // broadcast the same to the 'all cvars' event handlers
  s_AllCVarEvents.Broadcast(e);
}
