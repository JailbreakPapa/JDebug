#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/StringDeduplicationContext.h>

static const wdTypeVersion s_uiStringDeduplicationVersion = 1;

WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdStringDeduplicationWriteContext)

wdStringDeduplicationWriteContext::wdStringDeduplicationWriteContext(wdStreamWriter& ref_originalStream)
  : wdSerializationContext()
  , m_OriginalStream(ref_originalStream)
{
}

wdStringDeduplicationWriteContext::~wdStringDeduplicationWriteContext() = default;

wdStreamWriter& wdStringDeduplicationWriteContext::Begin()
{
  WD_ASSERT_DEV(m_TempStreamStorage.GetStorageSize64() == 0, "Begin() can only be called once on a string deduplication context.");

  m_TempStreamWriter.SetStorage(&m_TempStreamStorage);

  return m_TempStreamWriter;
}

wdResult wdStringDeduplicationWriteContext::End()
{
  // We set the context manual to null here since we need normal
  // string serialization to write the de-duplicated map
  SetContext(nullptr);

  m_OriginalStream.WriteVersion(s_uiStringDeduplicationVersion);

  const wdUInt64 uiNumEntries = m_DeduplicatedStrings.GetCount();
  m_OriginalStream << uiNumEntries;

  wdMap<wdUInt32, wdHybridString<64>> StringsSortedByIndex;

  // Build a new map from index to string so we can use a plain
  // array for serialization and lookup purposes
  for (const auto& it : m_DeduplicatedStrings)
  {
    StringsSortedByIndex.Insert(it.Value(), std::move(it.Key()));
  }

  // Write the new map entries, but just the strings since the indices are linear ascending
  for (const auto& it : StringsSortedByIndex)
  {
    m_OriginalStream << it.Value();
  }

  // Now append the original stream
  WD_SUCCEED_OR_RETURN(m_TempStreamStorage.CopyToStream(m_OriginalStream));

  return WD_SUCCESS;
}

void wdStringDeduplicationWriteContext::SerializeString(const wdStringView& sString, wdStreamWriter& ref_writer)
{
  bool bAlreadDeduplicated = false;
  auto it = m_DeduplicatedStrings.FindOrAdd(sString, &bAlreadDeduplicated);

  if (!bAlreadDeduplicated)
  {
    it.Value() = m_DeduplicatedStrings.GetCount() - 1;
  }

  ref_writer << it.Value();
}

wdUInt32 wdStringDeduplicationWriteContext::GetUniqueStringCount() const
{
  return m_DeduplicatedStrings.GetCount();
}


WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdStringDeduplicationReadContext)

wdStringDeduplicationReadContext::wdStringDeduplicationReadContext(wdStreamReader& inout_stream)
  : wdSerializationContext()
{
  // We set the context manually to nullptr to get the original string table
  SetContext(nullptr);

  // Read the string table first
  /*auto version =*/inout_stream.ReadVersion(s_uiStringDeduplicationVersion);

  wdUInt64 uiNumEntries = 0;
  inout_stream >> uiNumEntries;

  for (wdUInt64 i = 0; i < uiNumEntries; ++i)
  {
    wdStringBuilder Builder;
    inout_stream >> Builder;

    m_DeduplicatedStrings.ExpandAndGetRef() = std::move(Builder);
  }

  SetContext(this);
}

wdStringDeduplicationReadContext::~wdStringDeduplicationReadContext() = default;

wdStringView wdStringDeduplicationReadContext::DeserializeString(wdStreamReader& ref_reader)
{
  wdUInt32 uiIndex;
  ref_reader >> uiIndex;

  return m_DeduplicatedStrings[uiIndex].GetView();
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_StringDeduplicationContext);
