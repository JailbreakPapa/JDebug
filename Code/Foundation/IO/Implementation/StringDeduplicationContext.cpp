#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/StringDeduplicationContext.h>

static constexpr nsTypeVersion s_uiStringDeduplicationVersion = 1;

NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsStringDeduplicationWriteContext)

nsStringDeduplicationWriteContext::nsStringDeduplicationWriteContext(nsStreamWriter& ref_originalStream)
  : nsSerializationContext()
  , m_OriginalStream(ref_originalStream)
{
}

nsStringDeduplicationWriteContext::~nsStringDeduplicationWriteContext() = default;

nsStreamWriter& nsStringDeduplicationWriteContext::Begin()
{
  NS_ASSERT_DEV(m_TempStreamStorage.GetStorageSize64() == 0, "Begin() can only be called once on a string deduplication context.");

  m_TempStreamWriter.SetStorage(&m_TempStreamStorage);

  return m_TempStreamWriter;
}

nsResult nsStringDeduplicationWriteContext::End()
{
  // We set the context manual to null here since we need normal
  // string serialization to write the de-duplicated map
  SetContext(nullptr);

  m_OriginalStream.WriteVersion(s_uiStringDeduplicationVersion);

  const nsUInt64 uiNumEntries = m_DeduplicatedStrings.GetCount();
  m_OriginalStream << uiNumEntries;

  nsMap<nsUInt32, nsHybridString<64>> StringsSortedByIndex;

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
  NS_SUCCEED_OR_RETURN(m_TempStreamStorage.CopyToStream(m_OriginalStream));

  return NS_SUCCESS;
}

void nsStringDeduplicationWriteContext::SerializeString(const nsStringView& sString, nsStreamWriter& ref_writer)
{
  bool bAlreadDeduplicated = false;
  auto it = m_DeduplicatedStrings.FindOrAdd(sString, &bAlreadDeduplicated);

  if (!bAlreadDeduplicated)
  {
    it.Value() = m_DeduplicatedStrings.GetCount() - 1;
  }

  ref_writer << it.Value();
}

nsUInt32 nsStringDeduplicationWriteContext::GetUniqueStringCount() const
{
  return m_DeduplicatedStrings.GetCount();
}


NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsStringDeduplicationReadContext)

nsStringDeduplicationReadContext::nsStringDeduplicationReadContext(nsStreamReader& inout_stream)
  : nsSerializationContext()
{
  // We set the context manually to nullptr to get the original string table
  SetContext(nullptr);

  // Read the string table first
  /*auto version =*/inout_stream.ReadVersion(s_uiStringDeduplicationVersion);

  nsUInt64 uiNumEntries = 0;
  inout_stream >> uiNumEntries;

  m_DeduplicatedStrings.Reserve(static_cast<nsUInt32>(uiNumEntries));

  for (nsUInt64 i = 0; i < uiNumEntries; ++i)
  {
    nsStringBuilder s;
    inout_stream >> s;

    m_DeduplicatedStrings.PushBackUnchecked(std::move(s));
  }

  SetContext(this);
}

nsStringDeduplicationReadContext::~nsStringDeduplicationReadContext() = default;

nsStringView nsStringDeduplicationReadContext::DeserializeString(nsStreamReader& ref_reader)
{
  nsUInt32 uiIndex = nsInvalidIndex;
  ref_reader >> uiIndex;

  if (uiIndex >= m_DeduplicatedStrings.GetCount())
  {
    NS_ASSERT_DEBUG(uiIndex < m_DeduplicatedStrings.GetCount(), "Failed to read data from file.");
    return {};
  }

  return m_DeduplicatedStrings[uiIndex].GetView();
}
