
#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

class nsStreamWriter;

/// \brief Serialization Context that de-duplicates objects when writing to a stream. Duplicated objects are identified by their address and
/// only the first occurrence is written to the stream while all subsequence occurrences are just written as an index.
class NS_FOUNDATION_DLL nsDeduplicationWriteContext : public nsSerializationContext<nsDeduplicationWriteContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsDeduplicationWriteContext);

public:
  nsDeduplicationWriteContext();
  ~nsDeduplicationWriteContext();

  /// \brief Writes a single object to the stream. Can be either a reference or a pointer to the object.
  template <typename T>
  nsResult WriteObject(nsStreamWriter& inout_stream, const T& obj); // [tested]

  /// \brief Writes a single object to the stream.
  template <typename T>
  nsResult WriteObject(nsStreamWriter& inout_stream, const nsSharedPtr<T>& pObject); // [tested]

  /// \brief Writes a single object to the stream.
  template <typename T>
  nsResult WriteObject(nsStreamWriter& inout_stream, const nsUniquePtr<T>& pObject); // [tested]

  /// \brief Writes an array of de-duplicated objects.
  template <typename ArrayType, typename ValueType>
  nsResult WriteArray(nsStreamWriter& inout_stream, const nsArrayBase<ValueType, ArrayType>& array); // [tested]

  /// \brief Writes a set of de-duplicated objects.
  template <typename KeyType, typename Comparer>
  nsResult WriteSet(nsStreamWriter& inout_stream, const nsSetBase<KeyType, Comparer>& set); // [tested]

  enum class WriteMapMode
  {
    DedupKey,
    DedupValue,
    DedupBoth
  };

  /// \brief Writes a map. Mode controls whether key or value or both should de-duplicated.
  template <typename KeyType, typename ValueType, typename Comparer>
  nsResult WriteMap(nsStreamWriter& inout_stream, const nsMapBase<KeyType, ValueType, Comparer>& map, WriteMapMode mode); // [tested]

private:
  template <typename T>
  nsResult WriteObjectInternal(nsStreamWriter& stream, const T* pObject);

  nsHashTable<const void*, nsUInt32> m_Objects;
};

#include <Foundation/IO/Implementation/DeduplicationWriteContext_inl.h>
