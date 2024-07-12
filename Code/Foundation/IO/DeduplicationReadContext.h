
#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

class nsStreamReader;

/// \brief Serialization Context that reads de-duplicated objects from a stream and restores the pointers.
class NS_FOUNDATION_DLL nsDeduplicationReadContext : public nsSerializationContext<nsDeduplicationReadContext>
{
  NS_DECLARE_SERIALIZATION_CONTEXT(nsDeduplicationReadContext);

public:
  nsDeduplicationReadContext();
  ~nsDeduplicationReadContext();

  /// \brief Reads a single object inplace.
  template <typename T>
  nsResult ReadObjectInplace(nsStreamReader& inout_stream, T& ref_obj); // [tested]

  /// \brief Reads a single object and sets the pointer to it. The given allocator is used to create the object if it doesn't exist yet.
  template <typename T>
  nsResult ReadObject(nsStreamReader& inout_stream, T*& ref_pObject,
    nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a single object and sets the shared pointer to it. The given allocator is used to create the object if it doesn't exist
  /// yet.
  template <typename T>
  nsResult ReadObject(nsStreamReader& inout_stream, nsSharedPtr<T>& ref_pObject,
    nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a single object and sets the unique pointer to it. The given allocator is used to create the object if it doesn't exist
  /// yet.
  template <typename T>
  nsResult ReadObject(nsStreamReader& inout_stream, nsUniquePtr<T>& ref_pObject,
    nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads an array of de-duplicated objects.
  template <typename ArrayType, typename ValueType>
  nsResult ReadArray(nsStreamReader& inout_stream, nsArrayBase<ValueType, ArrayType>& ref_array,
    nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a set of de-duplicated objects.
  template <typename KeyType, typename Comparer>
  nsResult ReadSet(nsStreamReader& inout_stream, nsSetBase<KeyType, Comparer>& ref_set,
    nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  enum class ReadMapMode
  {
    DedupKey,
    DedupValue,
    DedupBoth
  };

  /// \brief Reads a map. Mode controls whether key or value or both should de-duplicated.
  template <typename KeyType, typename ValueType, typename Comparer>
  nsResult ReadMap(nsStreamReader& inout_stream, nsMapBase<KeyType, ValueType, Comparer>& ref_map, ReadMapMode mode,
    nsAllocator* pKeyAllocator = nsFoundation::GetDefaultAllocator(),
    nsAllocator* pValueAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

private:
  template <typename T>
  nsResult ReadObject(nsStreamReader& stream, T& obj, nsAllocator* pAllocator); // [tested]

  nsDynamicArray<void*> m_Objects;
};

#include <Foundation/IO/Implementation/DeduplicationReadContext_inl.h>
