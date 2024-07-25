#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/DynamicEnums.h>

nsMap<nsString, nsDynamicEnum> nsDynamicEnum::s_DynamicEnums;

void nsDynamicEnum::Clear()
{
  m_ValidValues.Clear();
}

void nsDynamicEnum::SetValueAndName(nsInt32 iValue, nsStringView sNewName)
{
  m_ValidValues[iValue] = sNewName;
}

void nsDynamicEnum::RemoveValue(nsInt32 iValue)
{
  m_ValidValues.Remove(iValue);
}

bool nsDynamicEnum::IsValueValid(nsInt32 iValue) const
{
  return m_ValidValues.Find(iValue).IsValid();
}

nsStringView nsDynamicEnum::GetValueName(nsInt32 iValue) const
{
  auto it = m_ValidValues.Find(iValue);

  if (!it.IsValid())
    return "<invalid value>";

  return it.Value();
}

nsDynamicEnum& nsDynamicEnum::GetDynamicEnum(const char* szEnumName)
{
  return s_DynamicEnums[szEnumName];
}
