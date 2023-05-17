
#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

class wdStreamWriter;

/// \brief Serialization Context that de-duplicates objects when writing to a stream. Duplicated objects are identified by their address and
/// only the first occurrence is written to the stream while all subsequence occurrences are just written as an index.
class WD_FOUNDATION_DLL wdDeduplicationWriteContext : public wdSerializationContext<wdDeduplicationWriteContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdDeduplicationWriteContext);

public:
  wdDeduplicationWriteContext();
  ~wdDeduplicationWriteContext();

  /// \brief Writes a single object to the stream. Can be either a reference or a pointer to the object.
  template <typename T>
  wdResult WriteObject(wdStreamWriter& inout_stream, const T& obj); // [tested]

  /// \brief Writes a single object to the stream.
  template <typename T>
  wdResult WriteObject(wdStreamWriter& inout_stream, const wdSharedPtr<T>& pObject); // [tested]

  /// \brief Writes a single object to the stream.
  template <typename T>
  wdResult WriteObject(wdStreamWriter& inout_stream, const wdUniquePtr<T>& pObject); // [tested]

  /// \brief Writes an array of de-duplicated objects.
  template <typename ArrayType, typename ValueType>
  wdResult WriteArray(wdStreamWriter& inout_stream, const wdArrayBase<ValueType, ArrayType>& array); // [tested]

  /// \brief Writes a set of de-duplicated objects.
  template <typename KeyType, typename Comparer>
  wdResult WriteSet(wdStreamWriter& inout_stream, const wdSetBase<KeyType, Comparer>& set); // [tested]

  enum class WriteMapMode
  {
    DedupKey,
    DedupValue,
    DedupBoth
  };

  /// \brief Writes a map. Mode controls whether key or value or both should de-duplicated.
  template <typename KeyType, typename ValueType, typename Comparer>
  wdResult WriteMap(wdStreamWriter& inout_stream, const wdMapBase<KeyType, ValueType, Comparer>& map, WriteMapMode mode); // [tested]

private:
  template <typename T>
  wdResult WriteObjectInternal(wdStreamWriter& stream, const T* pObject);

  wdHashTable<const void*, wdUInt32> m_Objects;
};

#include <Foundation/IO/Implementation/DeduplicationWriteContext_inl.h>
