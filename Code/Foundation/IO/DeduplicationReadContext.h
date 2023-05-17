
#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/SerializationContext.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

class wdStreamReader;

/// \brief Serialization Context that reads de-duplicated objects from a stream and restores the pointers.
class WD_FOUNDATION_DLL wdDeduplicationReadContext : public wdSerializationContext<wdDeduplicationReadContext>
{
  WD_DECLARE_SERIALIZATION_CONTEXT(wdDeduplicationReadContext);

public:
  wdDeduplicationReadContext();
  ~wdDeduplicationReadContext();

  /// \brief Reads a single object inplace.
  template <typename T>
  wdResult ReadObjectInplace(wdStreamReader& inout_stream, T& ref_obj); // [tested]

  /// \brief Reads a single object and sets the pointer to it. The given allocator is used to create the object if it doesn't exist yet.
  template <typename T>
  wdResult ReadObject(wdStreamReader& inout_stream, T*& ref_pObject,
    wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a single object and sets the shared pointer to it. The given allocator is used to create the object if it doesn't exist
  /// yet.
  template <typename T>
  wdResult ReadObject(wdStreamReader& inout_stream, wdSharedPtr<T>& ref_pObject,
    wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a single object and sets the unique pointer to it. The given allocator is used to create the object if it doesn't exist
  /// yet.
  template <typename T>
  wdResult ReadObject(wdStreamReader& inout_stream, wdUniquePtr<T>& ref_pObject,
    wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads an array of de-duplicated objects.
  template <typename ArrayType, typename ValueType>
  wdResult ReadArray(wdStreamReader& inout_stream, wdArrayBase<ValueType, ArrayType>& ref_array,
    wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Reads a set of de-duplicated objects.
  template <typename KeyType, typename Comparer>
  wdResult ReadSet(wdStreamReader& inout_stream, wdSetBase<KeyType, Comparer>& ref_set,
    wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  enum class ReadMapMode
  {
    DedupKey,
    DedupValue,
    DedupBoth
  };

  /// \brief Reads a map. Mode controls whether key or value or both should de-duplicated.
  template <typename KeyType, typename ValueType, typename Comparer>
  wdResult ReadMap(wdStreamReader& inout_stream, wdMapBase<KeyType, ValueType, Comparer>& ref_map, ReadMapMode mode,
    wdAllocatorBase* pKeyAllocator = wdFoundation::GetDefaultAllocator(),
    wdAllocatorBase* pValueAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

private:
  template <typename T>
  wdResult ReadObject(wdStreamReader& stream, T& obj, wdAllocatorBase* pAllocator); // [tested]

  wdDynamicArray<void*> m_Objects;
};

#include <Foundation/IO/Implementation/DeduplicationReadContext_inl.h>
