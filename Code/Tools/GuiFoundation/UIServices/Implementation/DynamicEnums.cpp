#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/DynamicEnums.h>

wdMap<wdString, wdDynamicEnum> wdDynamicEnum::s_DynamicEnums;

void wdDynamicEnum::Clear()
{
  m_ValidValues.Clear();
}

void wdDynamicEnum::SetValueAndName(wdInt32 iValue, const char* szNewName)
{
  m_ValidValues[iValue] = szNewName;
}

void wdDynamicEnum::RemoveValue(wdInt32 iValue)
{
  m_ValidValues.Remove(iValue);
}

bool wdDynamicEnum::IsValueValid(wdInt32 iValue) const
{
  return m_ValidValues.Find(iValue).IsValid();
}

const char* wdDynamicEnum::GetValueName(wdInt32 iValue) const
{
  auto it = m_ValidValues.Find(iValue);

  if (!it.IsValid())
    return "<invalid value>";

  return it.Value();
}

wdDynamicEnum& wdDynamicEnum::GetDynamicEnum(const char* szEnumName)
{
  return s_DynamicEnums[szEnumName];
}
