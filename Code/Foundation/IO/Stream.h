
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/ArrayBase.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/EndianHelper.h>

typedef wdUInt16 wdTypeVersion;

template <wdUInt16 Size, typename AllocatorWrapper>
struct wdHybridString;

using wdString = wdHybridString<32, wdDefaultAllocatorWrapper>;

/// \brief Interface for binary in (read) streams.
class WD_FOUNDATION_DLL wdStreamReader
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdStreamReader);

public:
  /// \brief Constructor
  wdStreamReader();

  /// \brief Virtual destructor to ensure correct cleanup
  virtual ~wdStreamReader();

  /// \brief Reads a raw number of bytes into the read buffer, this is the only method which has to be implemented to fully implement the
  /// interface.
  virtual wdUInt64 ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead) = 0; // [tested]

  /// \brief Helper method to read a word value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult ReadWordValue(T* pWordValue); // [tested]

  /// \brief Helper method to read a dword value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult ReadDWordValue(T* pDWordValue); // [tested]

  /// \brief Helper method to read a qword value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult ReadQWordValue(T* pQWordValue); // [tested]

  /// \brief Reads an array of elements from the stream
  template <typename ArrayType, typename ValueType>
  wdResult ReadArray(wdArrayBase<ValueType, ArrayType>& inout_array); // [tested]

  /// \brief Reads a small array of elements from the stream
  template <typename ValueType, wdUInt16 uiSize, typename AllocatorWrapper>
  wdResult ReadArray(wdSmallArray<ValueType, uiSize, AllocatorWrapper>& ref_array);

  /// \brief Writes a C style fixed array
  template <typename ValueType, wdUInt32 uiSize>
  wdResult ReadArray(ValueType (&array)[uiSize]);

  /// \brief Reads a set
  template <typename KeyType, typename Comparer>
  wdResult ReadSet(wdSetBase<KeyType, Comparer>& inout_set); // [tested]

  /// \brief Reads a map
  template <typename KeyType, typename ValueType, typename Comparer>
  wdResult ReadMap(wdMapBase<KeyType, ValueType, Comparer>& inout_map); // [tested]

  /// \brief Read a hash table (note that the entry order is not stable)
  template <typename KeyType, typename ValueType, typename Hasher>
  wdResult ReadHashTable(wdHashTableBase<KeyType, ValueType, Hasher>& inout_hashTable); // [tested]

  /// \brief Reads a string into an wdStringBuilder
  wdResult ReadString(wdStringBuilder& ref_sBuilder); // [tested]

  /// \brief Reads a string into an wdString
  wdResult ReadString(wdString& ref_sString);


  /// \brief Helper method to skip a number of bytes (implementations of the stream reader may implement this more efficiently for example)
  virtual wdUInt64 SkipBytes(wdUInt64 uiBytesToSkip)
  {
    wdUInt8 uiTempBuffer[1024];

    wdUInt64 uiBytesSkipped = 0;

    while (uiBytesSkipped < uiBytesToSkip)
    {
      wdUInt64 uiBytesToRead = wdMath::Min<wdUInt64>(uiBytesToSkip - uiBytesSkipped, 1024);

      wdUInt64 uiBytesRead = ReadBytes(uiTempBuffer, uiBytesToRead);

      uiBytesSkipped += uiBytesRead;

      // Terminate early if the stream didn't read as many bytes as we requested (EOF for example)
      if (uiBytesRead < uiBytesToRead)
        break;
    }

    return uiBytesSkipped;
  }

  WD_ALWAYS_INLINE wdTypeVersion ReadVersion(wdTypeVersion expectedMaxVersion);
};

/// \brief Interface for binary out (write) streams.
class WD_FOUNDATION_DLL wdStreamWriter
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdStreamWriter);

public:
  /// \brief Constructor
  wdStreamWriter();

  /// \brief Virtual destructor to ensure correct cleanup
  virtual ~wdStreamWriter();

  /// \brief Writes a raw number of bytes from the buffer, this is the only method which has to be implemented to fully implement the
  /// interface.
  virtual wdResult WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite) = 0; // [tested]

  /// \brief Flushes the stream, may be implemented (not necessary to implement the interface correctly) so that user code can ensure that
  /// content is written
  virtual wdResult Flush() // [tested]
  {
    return WD_SUCCESS;
  }

  /// \brief Helper method to write a word value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult WriteWordValue(const T* pWordValue); // [tested]

  /// \brief Helper method to write a dword value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult WriteDWordValue(const T* pDWordValue); // [tested]

  /// \brief Helper method to write a qword value correctly (copes with potentially different endianess)
  template <typename T>
  wdResult WriteQWordValue(const T* pQWordValue); // [tested]

  /// \brief Writes a type version to the stream
  WD_ALWAYS_INLINE void WriteVersion(wdTypeVersion version);

  /// \brief Writes an array of elements to the stream
  template <typename ArrayType, typename ValueType>
  wdResult WriteArray(const wdArrayBase<ValueType, ArrayType>& array); // [tested]

  /// \brief Writes a small array of elements to the stream
  template <typename ValueType, wdUInt16 uiSize>
  wdResult WriteArray(const wdSmallArrayBase<ValueType, uiSize>& array);

  /// \brief Writes a C style fixed array
  template <typename ValueType, wdUInt32 uiSize>
  wdResult WriteArray(const ValueType (&array)[uiSize]);

  /// \brief Writes a set
  template <typename KeyType, typename Comparer>
  wdResult WriteSet(const wdSetBase<KeyType, Comparer>& set); // [tested]

  /// \brief Writes a map
  template <typename KeyType, typename ValueType, typename Comparer>
  wdResult WriteMap(const wdMapBase<KeyType, ValueType, Comparer>& map); // [tested]

  /// \brief Writes a hash table (note that the entry order might change on read)
  template <typename KeyType, typename ValueType, typename Hasher>
  wdResult WriteHashTable(const wdHashTableBase<KeyType, ValueType, Hasher>& hashTable); // [tested]

  /// \brief Writes a string
  wdResult WriteString(const wdStringView sStringView); // [tested]
};

// Contains the helper methods of both interfaces
#include <Foundation/IO/Implementation/Stream_inl.h>

// Standard operators for overloads of common data types
#include <Foundation/IO/Implementation/StreamOperations_inl.h>

#include <Foundation/IO/Implementation/StreamOperationsMath_inl.h>

#include <Foundation/IO/Implementation/StreamOperationsOther_inl.h>
