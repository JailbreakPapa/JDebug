#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Tag.h>

static wdTagRegistry s_GlobalRegistry;

wdTagRegistry::wdTagRegistry() {}

wdTagRegistry& wdTagRegistry::GetGlobalRegistry()
{
  return s_GlobalRegistry;
}

const wdTag& wdTagRegistry::RegisterTag(wdStringView sTagString)
{
  wdHashedString TagString;
  TagString.Assign(sTagString);

  return RegisterTag(TagString);
}

const wdTag& wdTagRegistry::RegisterTag(const wdHashedString& sTagString)
{
  WD_LOCK(m_TagRegistryMutex);

  // Early out if the tag is already registered
  const wdTag* pResult = GetTagByName(sTagString);

  if (pResult != nullptr)
    return *pResult;

  const wdUInt32 uiNextTagIndex = m_TagsByIndex.GetCount();

  // Build temp tag
  wdTag TempTag;
  TempTag.m_uiBlockIndex = uiNextTagIndex / (sizeof(wdTagSetBlockStorage) * 8);
  TempTag.m_uiBitIndex = uiNextTagIndex - (TempTag.m_uiBlockIndex * sizeof(wdTagSetBlockStorage) * 8);
  TempTag.m_sTagString = sTagString;

  // Store the tag
  auto it = m_RegisteredTags.Insert(sTagString, TempTag);

  m_TagsByIndex.PushBack(&it.Value());

  wdLog::Debug("Registered Tag '{0}'", sTagString);
  return *m_TagsByIndex.PeekBack();
}

const wdTag* wdTagRegistry::GetTagByName(const wdTempHashedString& sTagString) const
{
  WD_LOCK(m_TagRegistryMutex);

  auto It = m_RegisteredTags.Find(sTagString);
  if (It.IsValid())
  {
    return &It.Value();
  }

  return nullptr;
}

const wdTag* wdTagRegistry::GetTagByMurmurHash(wdUInt32 uiMurmurHash) const
{
  WD_LOCK(m_TagRegistryMutex);

  for (wdTag* pTag : m_TagsByIndex)
  {
    if (wdHashingUtils::MurmurHash32String(pTag->GetTagString()) == uiMurmurHash)
    {
      return pTag;
    }
  }

  return nullptr;
}

const wdTag* wdTagRegistry::GetTagByIndex(wdUInt32 uiIndex) const
{
  WD_LOCK(m_TagRegistryMutex);
  return m_TagsByIndex[uiIndex];
}

wdUInt32 wdTagRegistry::GetNumTags() const
{
  WD_LOCK(m_TagRegistryMutex);
  return m_TagsByIndex.GetCount();
}

wdResult wdTagRegistry::Load(wdStreamReader& inout_stream)
{
  WD_LOCK(m_TagRegistryMutex);

  wdUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != 1)
  {
    wdLog::Error("Invalid wdTagRegistry version {0}", uiVersion);
    return WD_FAILURE;
  }

  wdUInt32 uiNumTags = 0;
  inout_stream >> uiNumTags;

  if (uiNumTags > 16 * 1024)
  {
    wdLog::Error("wdTagRegistry::Load, unreasonable amount of tags {0}, cancelling load.", uiNumTags);
    return WD_FAILURE;
  }

  wdStringBuilder temp;
  for (wdUInt32 i = 0; i < uiNumTags; ++i)
  {
    inout_stream >> temp;

    RegisterTag(temp);
  }

  return WD_SUCCESS;
}

WD_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_TagRegistry);
