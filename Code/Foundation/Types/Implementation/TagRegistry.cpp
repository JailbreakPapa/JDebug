#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Tag.h>

static nsTagRegistry s_GlobalRegistry;

nsTagRegistry::nsTagRegistry() = default;

nsTagRegistry& nsTagRegistry::GetGlobalRegistry()
{
  return s_GlobalRegistry;
}

const nsTag& nsTagRegistry::RegisterTag(nsStringView sTagString)
{
  nsHashedString TagString;
  TagString.Assign(sTagString);

  return RegisterTag(TagString);
}

const nsTag& nsTagRegistry::RegisterTag(const nsHashedString& sTagString)
{
  NS_LOCK(m_TagRegistryMutex);

  // Early out if the tag is already registered
  const nsTag* pResult = GetTagByName(sTagString);

  if (pResult != nullptr)
    return *pResult;

  const nsUInt32 uiNextTagIndex = m_TagsByIndex.GetCount();

  // Build temp tag
  nsTag TempTag;
  TempTag.m_uiBlockIndex = uiNextTagIndex / (sizeof(nsTagSetBlockStorage) * 8);
  TempTag.m_uiBitIndex = uiNextTagIndex - (TempTag.m_uiBlockIndex * sizeof(nsTagSetBlockStorage) * 8);
  TempTag.m_sTagString = sTagString;

  // Store the tag
  auto it = m_RegisteredTags.Insert(sTagString, TempTag);

  m_TagsByIndex.PushBack(&it.Value());

  nsLog::Debug("Registered Tag '{0}'", sTagString);
  return *m_TagsByIndex.PeekBack();
}

const nsTag* nsTagRegistry::GetTagByName(const nsTempHashedString& sTagString) const
{
  NS_LOCK(m_TagRegistryMutex);

  auto It = m_RegisteredTags.Find(sTagString);
  if (It.IsValid())
  {
    return &It.Value();
  }

  return nullptr;
}

const nsTag* nsTagRegistry::GetTagByMurmurHash(nsUInt32 uiMurmurHash) const
{
  NS_LOCK(m_TagRegistryMutex);

  for (nsTag* pTag : m_TagsByIndex)
  {
    if (nsHashingUtils::MurmurHash32String(pTag->GetTagString()) == uiMurmurHash)
    {
      return pTag;
    }
  }

  return nullptr;
}

const nsTag* nsTagRegistry::GetTagByIndex(nsUInt32 uiIndex) const
{
  NS_LOCK(m_TagRegistryMutex);
  return m_TagsByIndex[uiIndex];
}

nsUInt32 nsTagRegistry::GetNumTags() const
{
  NS_LOCK(m_TagRegistryMutex);
  return m_TagsByIndex.GetCount();
}

nsResult nsTagRegistry::Load(nsStreamReader& inout_stream)
{
  NS_LOCK(m_TagRegistryMutex);

  nsUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  if (uiVersion != 1)
  {
    nsLog::Error("Invalid nsTagRegistry version {0}", uiVersion);
    return NS_FAILURE;
  }

  nsUInt32 uiNumTags = 0;
  inout_stream >> uiNumTags;

  if (uiNumTags > 16 * 1024)
  {
    nsLog::Error("nsTagRegistry::Load, unreasonable amount of tags {0}, cancelling load.", uiNumTags);
    return NS_FAILURE;
  }

  nsStringBuilder temp;
  for (nsUInt32 i = 0; i < uiNumTags; ++i)
  {
    inout_stream >> temp;

    RegisterTag(temp);
  }

  return NS_SUCCESS;
}
