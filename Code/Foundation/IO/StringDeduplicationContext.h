
#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Strings/String.h>

class wdStreamWriter;
class wdStreamReader;

/// \brief This class allows for automatic deduplication of strings written to a stream.
/// To use, create an object of this type on the stack, call Begin() and use the returned
/// wdStreamWriter for subsequent serialization operations. Call End() once you want to finish writing
/// deduplicated strings. For a sample see StreamOperationsTest.cpp
class WD_FOUNDATION_DLL wdStringDeduplicationWriteContext : public wdSerializationContext<wdStringDeduplicationWriteContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdStringDeduplicationWriteContext);

public:
  /// \brief Setup the write context to perform string deduplication.
  wdStringDeduplicationWriteContext(wdStreamWriter& ref_originalStream);
  ~wdStringDeduplicationWriteContext();

  /// \brief Call this method to begin string deduplicaton. You need to use the returned stream writer for subsequent serialization operations until
  /// End() is called.
  wdStreamWriter& Begin();

  /// \brief Ends the string deduplication and writes the string table to the original stream
  wdResult End();

  /// \brief Internal method to serialize a string.
  void SerializeString(const wdStringView& sString, wdStreamWriter& ref_writer);

  /// \brief Returns the number of unique strings which were serialized with this instance.
  wdUInt32 GetUniqueStringCount() const;

  /// \brief Returns the original stream that was passed to the constructor.
  wdStreamWriter& GetOriginalStream() { return m_OriginalStream; }

protected:
  wdStreamWriter& m_OriginalStream;

  wdDefaultMemoryStreamStorage m_TempStreamStorage;
  wdMemoryStreamWriter m_TempStreamWriter;

  wdMap<wdHybridString<64>, wdUInt32> m_DeduplicatedStrings;
};

/// \brief This class to restore strings written to a stream using a wdStringDeduplicationWriteContext.
class WD_FOUNDATION_DLL wdStringDeduplicationReadContext : public wdSerializationContext<wdStringDeduplicationReadContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdStringDeduplicationReadContext);

public:
  /// \brief Setup the string table used internally.
  wdStringDeduplicationReadContext(wdStreamReader& inout_stream);
  ~wdStringDeduplicationReadContext();

  /// \brief Internal method to deserialize a string.
  wdStringView DeserializeString(wdStreamReader& ref_reader);

protected:
  wdDynamicArray<wdHybridString<64>> m_DeduplicatedStrings;
};
