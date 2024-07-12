
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/ArrayBase.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/EndianHelper.h>

using nsTypeVersion = nsUInt16;

template <nsUInt16 Size, typename AllocatorWrapper>
struct nsHybridString;

using nsString = nsHybridString<32, nsDefaultAllocatorWrapper>;

/// \brief Interface for binary in (read) streams.
class NS_FOUNDATION_DLL nsStreamReader
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsStreamReader);

public:
  /// \brief Constructor
  nsStreamReader();

  /// \brief Virtual destructor to ensure correct cleanup
  virtual ~nsStreamReader();

  /// \brief Reads a raw number of bytes into the read buffer, this is the only method which has to be implemented to fully implement the
  /// interface.
  virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead) = 0; // [tested]

  /// \brief Helper method to read a word value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult ReadWordValue(T* pWordValue); // [tested]

  /// \brief Helper method to read a dword value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult ReadDWordValue(T* pDWordValue); // [tested]

  /// \brief Helper method to read a qword value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult ReadQWordValue(T* pQWordValue); // [tested]

  /// \brief Reads an array of elements from the stream
  template <typename ArrayType, typename ValueType>
  nsResult ReadArray(nsArrayBase<ValueType, ArrayType>& inout_array); // [tested]

  /// \brief Reads a small array of elements from the stream
  template <typename ValueType, nsUInt16 uiSize, typename AllocatorWrapper>
  nsResult ReadArray(nsSmallArray<ValueType, uiSize, AllocatorWrapper>& ref_array);

  /// \brief Writes a C style fixed array
  template <typename ValueType, nsUInt32 uiSize>
  nsResult ReadArray(ValueType (&array)[uiSize]);

  /// \brief Reads a set
  template <typename KeyType, typename Comparer>
  nsResult ReadSet(nsSetBase<KeyType, Comparer>& inout_set); // [tested]

  /// \brief Reads a map
  template <typename KeyType, typename ValueType, typename Comparer>
  nsResult ReadMap(nsMapBase<KeyType, ValueType, Comparer>& inout_map); // [tested]

  /// \brief Read a hash table (note that the entry order is not stable)
  template <typename KeyType, typename ValueType, typename Hasher>
  nsResult ReadHashTable(nsHashTableBase<KeyType, ValueType, Hasher>& inout_hashTable); // [tested]

  /// \brief Reads a string into an nsStringBuilder
  nsResult ReadString(nsStringBuilder& ref_sBuilder); // [tested]

  /// \brief Reads a string into an nsString
  nsResult ReadString(nsString& ref_sString);


  /// \brief Helper method to skip a number of bytes (implementations of the stream reader may implement this more efficiently for example)
  virtual nsUInt64 SkipBytes(nsUInt64 uiBytesToSkip)
  {
    nsUInt8 uiTempBuffer[1024];

    nsUInt64 uiBytesSkipped = 0;

    while (uiBytesSkipped < uiBytesToSkip)
    {
      nsUInt64 uiBytesToRead = nsMath::Min<nsUInt64>(uiBytesToSkip - uiBytesSkipped, 1024);

      nsUInt64 uiBytesRead = ReadBytes(uiTempBuffer, uiBytesToRead);

      uiBytesSkipped += uiBytesRead;

      // Terminate early if the stream didn't read as many bytes as we requested (EOF for example)
      if (uiBytesRead < uiBytesToRead)
        break;
    }

    return uiBytesSkipped;
  }

  NS_ALWAYS_INLINE nsTypeVersion ReadVersion(nsTypeVersion expectedMaxVersion);
};

/// \brief Interface for binary out (write) streams.
class NS_FOUNDATION_DLL nsStreamWriter
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsStreamWriter);

public:
  /// \brief Constructor
  nsStreamWriter();

  /// \brief Virtual destructor to ensure correct cleanup
  virtual ~nsStreamWriter();

  /// \brief Writes a raw number of bytes from the buffer, this is the only method which has to be implemented to fully implement the
  /// interface.
  virtual nsResult WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite) = 0; // [tested]

  /// \brief Flushes the stream, may be implemented (not necessary to implement the interface correctly) so that user code can ensure that
  /// content is written
  virtual nsResult Flush() // [tested]
  {
    return NS_SUCCESS;
  }

  /// \brief Helper method to write a word value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult WriteWordValue(const T* pWordValue); // [tested]

  /// \brief Helper method to write a dword value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult WriteDWordValue(const T* pDWordValue); // [tested]

  /// \brief Helper method to write a qword value correctly (copes with potentially different endianess)
  template <typename T>
  nsResult WriteQWordValue(const T* pQWordValue); // [tested]

  /// \brief Writes a type version to the stream
  NS_ALWAYS_INLINE void WriteVersion(nsTypeVersion version);

  /// \brief Writes an array of elements to the stream
  template <typename ArrayType, typename ValueType>
  nsResult WriteArray(const nsArrayBase<ValueType, ArrayType>& array); // [tested]

  /// \brief Writes a small array of elements to the stream
  template <typename ValueType, nsUInt16 uiSize>
  nsResult WriteArray(const nsSmallArrayBase<ValueType, uiSize>& array);

  /// \brief Writes a C style fixed array
  template <typename ValueType, nsUInt32 uiSize>
  nsResult WriteArray(const ValueType (&array)[uiSize]);

  /// \brief Writes a set
  template <typename KeyType, typename Comparer>
  nsResult WriteSet(const nsSetBase<KeyType, Comparer>& set); // [tested]

  /// \brief Writes a map
  template <typename KeyType, typename ValueType, typename Comparer>
  nsResult WriteMap(const nsMapBase<KeyType, ValueType, Comparer>& map); // [tested]

  /// \brief Writes a hash table (note that the entry order might change on read)
  template <typename KeyType, typename ValueType, typename Hasher>
  nsResult WriteHashTable(const nsHashTableBase<KeyType, ValueType, Hasher>& hashTable); // [tested]

  /// \brief Writes a string
  nsResult WriteString(const nsStringView sStringView); // [tested]
};

// Contains the helper methods of both interfaces
#include <Foundation/IO/Implementation/Stream_inl.h>

// Standard operators for overloads of common data types
#include <Foundation/IO/Implementation/StreamOperations_inl.h>

#include <Foundation/IO/Implementation/StreamOperationsMath_inl.h>

#include <Foundation/IO/Implementation/StreamOperationsOther_inl.h>
