#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <GuiFoundation/UIServices/DynamicStringEnum.h>

nsMap<nsString, nsDynamicStringEnum> nsDynamicStringEnum::s_DynamicEnums;
nsDelegate<void(nsStringView sEnumName, nsDynamicStringEnum& e)> nsDynamicStringEnum::s_RequestUnknownCallback;

// static
nsDynamicStringEnum& nsDynamicStringEnum::GetDynamicEnum(nsStringView sEnumName)
{
  bool bExisted = false;
  auto it = s_DynamicEnums.FindOrAdd(sEnumName, &bExisted);

  if (!bExisted && s_RequestUnknownCallback.IsValid())
  {
    s_RequestUnknownCallback(sEnumName, it.Value());
  }

  return it.Value();
}

// static
nsDynamicStringEnum& nsDynamicStringEnum::CreateDynamicEnum(nsStringView sEnumName)
{
  bool bExisted = false;
  auto it = s_DynamicEnums.FindOrAdd(sEnumName, &bExisted);

  nsDynamicStringEnum& e = it.Value();
  e.Clear();
  e.SetStorageFile(nullptr);

  return e;
}

// static
void nsDynamicStringEnum::RemoveEnum(nsStringView sEnumName)
{
  s_DynamicEnums.Remove(sEnumName);
}

void nsDynamicStringEnum::Clear()
{
  m_ValidValues.Clear();
}

void nsDynamicStringEnum::AddValidValue(nsStringView sValue, bool bSortValues /*= false*/)
{
  nsString sNewValue = sValue;

  if (!m_ValidValues.Contains(sNewValue))
    m_ValidValues.PushBack(sNewValue);

  if (bSortValues)
    SortValues();
}

void nsDynamicStringEnum::RemoveValue(nsStringView sValue)
{
  m_ValidValues.RemoveAndCopy(sValue);
}

bool nsDynamicStringEnum::IsValueValid(nsStringView sValue) const
{
  return m_ValidValues.Contains(sValue);
}

void nsDynamicStringEnum::SortValues()
{
  m_ValidValues.Sort();
}

void nsDynamicStringEnum::ReadFromStorage()
{
  Clear();

  nsStringBuilder sFile, tmp;

  nsFileReader file;
  if (file.Open(m_sStorageFile).Failed())
    return;

  sFile.ReadAll(file);

  nsHybridArray<nsStringView, 32> values;

  sFile.Split(false, values, "\n", "\r");

  for (auto val : values)
  {
    AddValidValue(val.GetData(tmp));
  }
}

void nsDynamicStringEnum::SaveToStorage()
{
  if (m_sStorageFile.IsEmpty())
    return;

  nsFileWriter file;
  if (file.Open(m_sStorageFile).Failed())
    return;

  nsStringBuilder tmp;

  for (const auto& val : m_ValidValues)
  {
    tmp.Set(val, "\n");
    file.WriteBytes(tmp.GetData(), tmp.GetElementCount()).IgnoreResult();
  }
}
