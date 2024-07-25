#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/DynamicBitflags.h>

nsMap<nsString, nsDynamicBitflags> nsDynamicBitflags::s_DynamicBitflags;

void nsDynamicBitflags::Clear()
{
  m_ValidValues.Clear();
}

void nsDynamicBitflags::SetValueAndName(nsUInt32 uiBitPos, nsStringView sName)
{
  NS_ASSERT_DEV(uiBitPos < 64, "Only up to 64 bits is supported.");
  auto it = m_ValidValues.FindOrAdd(NS_BIT(uiBitPos));
  it.Value() = sName;
}

void nsDynamicBitflags::RemoveValue(nsUInt32 uiBitPos)
{
  NS_ASSERT_DEV(uiBitPos < 64, "Only up to 64 bits is supported.");
  m_ValidValues.Remove(NS_BIT(uiBitPos));
}

bool nsDynamicBitflags::IsValueValid(nsUInt32 uiBitPos) const
{
  return m_ValidValues.Find(NS_BIT(uiBitPos)).IsValid();
}

bool nsDynamicBitflags::TryGetValueName(nsUInt32 uiBitPos, nsStringView& out_sName) const
{
  auto it = m_ValidValues.Find(NS_BIT(uiBitPos));
  if (it.IsValid())
  {
    out_sName = it.Value();
    return true;
  }
  return false;
}

nsDynamicBitflags& nsDynamicBitflags::GetDynamicBitflags(nsStringView sName)
{
  return s_DynamicBitflags[sName];
}
