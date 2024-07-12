
#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Strings/String.h>

class nsStreamWriter;
class nsStreamReader;

/// \brief This class allows for automatic deduplication of strings written to a stream.
/// To use, create an object of this type on the stack, call Begin() and use the returned
/// nsStreamWriter for subsequent serialization operations. Call End() once you want to finish writing
/// deduplicated strings. For a sample see StreamOperationsTest.cpp
class NS_FOUNDATION_DLL nsStringDeduplicationWriteContext : public nsSerializationContext<nsStringDeduplicationWriteContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsStringDeduplicationWriteContext);

public:
  /// \brief Setup the write context to perform string deduplication.
  nsStringDeduplicationWriteContext(nsStreamWriter& ref_originalStream);
  ~nsStringDeduplicationWriteContext();

  /// \brief Call this method to begin string deduplicaton. You need to use the returned stream writer for subsequent serialization operations until
  /// End() is called.
  nsStreamWriter& Begin();

  /// \brief Ends the string deduplication and writes the string table to the original stream
  nsResult End();

  /// \brief Internal method to serialize a string.
  void SerializeString(const nsStringView& sString, nsStreamWriter& ref_writer);

  /// \brief Returns the number of unique strings which were serialized with this instance.
  nsUInt32 GetUniqueStringCount() const;

  /// \brief Returns the original stream that was passed to the constructor.
  nsStreamWriter& GetOriginalStream() { return m_OriginalStream; }

protected:
  nsStreamWriter& m_OriginalStream;

  nsDefaultMemoryStreamStorage m_TempStreamStorage;
  nsMemoryStreamWriter m_TempStreamWriter;

  nsMap<nsHybridString<64>, nsUInt32> m_DeduplicatedStrings;
};

/// \brief This class to restore strings written to a stream using a nsStringDeduplicationWriteContext.
class NS_FOUNDATION_DLL nsStringDeduplicationReadContext : public nsSerializationContext<nsStringDeduplicationReadContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsStringDeduplicationReadContext);

public:
  /// \brief Setup the string table used internally.
  nsStringDeduplicationReadContext(nsStreamReader& inout_stream);
  ~nsStringDeduplicationReadContext();

  /// \brief Internal method to deserialize a string.
  nsStringView DeserializeString(nsStreamReader& ref_reader);

protected:
  nsDynamicArray<nsHybridString<64>> m_DeduplicatedStrings;
};
