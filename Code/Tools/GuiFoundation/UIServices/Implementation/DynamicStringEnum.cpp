#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <GuiFoundation/UIServices/DynamicStringEnum.h>

wdMap<wdString, wdDynamicStringEnum> wdDynamicStringEnum::s_DynamicEnums;
wdDelegate<void(const char* szEnumName, wdDynamicStringEnum& e)> wdDynamicStringEnum::s_RequestUnknownCallback;

void wdDynamicStringEnum::RemoveEnum(const char* szEnumName)
{
  s_DynamicEnums.Remove(szEnumName);
}

void wdDynamicStringEnum::Clear()
{
  m_ValidValues.Clear();
}

void wdDynamicStringEnum::AddValidValue(const char* szNewName, bool bSortValues /*= false*/)
{
  wdString sName = szNewName;

  if (!m_ValidValues.Contains(sName))
    m_ValidValues.PushBack(sName);

  if (bSortValues)
    SortValues();
}

void wdDynamicStringEnum::RemoveValue(const char* szValue)
{
  m_ValidValues.RemoveAndCopy(szValue);
}

bool wdDynamicStringEnum::IsValueValid(const char* szValue) const
{
  return m_ValidValues.Contains(szValue);
}

void wdDynamicStringEnum::SortValues()
{
  m_ValidValues.Sort();
}

wdDynamicStringEnum& wdDynamicStringEnum::GetDynamicEnum(const char* szEnumName)
{
  bool bExisted = false;
  auto it = s_DynamicEnums.FindOrAdd(szEnumName, &bExisted);

  if (!bExisted && s_RequestUnknownCallback.IsValid())
  {
    s_RequestUnknownCallback(szEnumName, it.Value());
  }

  return it.Value();
}

wdDynamicStringEnum& wdDynamicStringEnum::CreateDynamicEnum(const char* szEnumName)
{
  bool bExisted = false;
  auto it = s_DynamicEnums.FindOrAdd(szEnumName, &bExisted);

  wdDynamicStringEnum& e = it.Value();
  e.Clear();
  e.SetStorageFile(nullptr);

  return e;
}

void wdDynamicStringEnum::ReadFromStorage()
{
  Clear();

  wdStringBuilder sFile, tmp;

  wdFileReader file;
  if (file.Open(m_sStorageFile).Failed())
    return;

  sFile.ReadAll(file);

  wdHybridArray<wdStringView, 32> values;

  sFile.Split(false, values, "\n", "\r");

  for (auto val : values)
  {
    AddValidValue(val.GetData(tmp));
  }
}

void wdDynamicStringEnum::SaveToStorage()
{
  if (m_sStorageFile.IsEmpty())
    return;

  wdFileWriter file;
  if (file.Open(m_sStorageFile).Failed())
    return;

  wdStringBuilder tmp;

  for (const auto& val : m_ValidValues)
  {
    tmp.Set(val, "\n");
    file.WriteBytes(tmp.GetData(), tmp.GetElementCount()).IgnoreResult();
  }
}
