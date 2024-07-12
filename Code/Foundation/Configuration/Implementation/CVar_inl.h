#pragma once

#include <Foundation/Configuration/CVar.h>

template <typename Type, nsCVarType::Enum CVarType>
nsTypedCVar<Type, CVarType>::nsTypedCVar(nsStringView sName, const Type& value, nsBitflags<nsCVarFlags> flags, nsStringView sDescription)
  : nsCVar(sName, flags, sDescription)
{
  NS_ASSERT_DEBUG(sName.FindSubString(" ") == nullptr, "CVar names must not contain whitespace");

  for (nsUInt32 i = 0; i < nsCVarValue::ENUM_COUNT; ++i)
    m_Values[i] = value;
}

template <typename Type, nsCVarType::Enum CVarType>
nsTypedCVar<Type, CVarType>::operator const Type&() const
{
  return (m_Values[nsCVarValue::Current]);
}

template <typename Type, nsCVarType::Enum CVarType>
nsCVarType::Enum nsTypedCVar<Type, CVarType>::GetType() const
{
  return CVarType;
}

template <typename Type, nsCVarType::Enum CVarType>
void nsTypedCVar<Type, CVarType>::SetToDelayedSyncValue()
{
  if (m_Values[nsCVarValue::Current] == m_Values[nsCVarValue::DelayedSync])
    return;

  // this will NOT trigger a 'restart value changed' event
  m_Values[nsCVarValue::Current] = m_Values[nsCVarValue::DelayedSync];

  nsCVarEvent e(this);
  e.m_EventType = nsCVarEvent::ValueChanged;
  m_CVarEvents.Broadcast(e);

  // broadcast the same to the 'all CVars' event handlers
  s_AllCVarEvents.Broadcast(e);
}

template <typename Type, nsCVarType::Enum CVarType>
const Type& nsTypedCVar<Type, CVarType>::GetValue(nsCVarValue::Enum val) const
{
  return (m_Values[val]);
}

template <typename Type, nsCVarType::Enum CVarType>
void nsTypedCVar<Type, CVarType>::operator=(const Type& value)
{
  nsCVarEvent e(this);

  if (GetFlags().IsAnySet(nsCVarFlags::RequiresDelayedSync))
  {
    if (value == m_Values[nsCVarValue::DelayedSync]) // no change
      return;

    e.m_EventType = nsCVarEvent::DelayedSyncValueChanged;
  }
  else
  {
    if (m_Values[nsCVarValue::Current] == value) // no change
      return;

    m_Values[nsCVarValue::Current] = value;
    e.m_EventType = nsCVarEvent::ValueChanged;
  }

  m_Values[nsCVarValue::DelayedSync] = value;

  m_CVarEvents.Broadcast(e);

  // broadcast the same to the 'all cvars' event handlers
  s_AllCVarEvents.Broadcast(e);
}
