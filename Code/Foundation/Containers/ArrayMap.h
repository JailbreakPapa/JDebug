#pragma once

#include <Foundation/Containers/DynamicArray.h>

/// \brief An associative container, similar to wdMap, but all data is stored in a sorted contiguous array, which makes frequent lookups more
/// efficient.
///
/// Prefer this container over wdMap when you modify the container less often than you look things up (which is in most cases), and when
/// you do not need to store iterators to elements and require them to stay valid when the container is modified.
///
/// wdArrayMapBase also allows to store multiple values under the same key (like a multi-map).
template <typename KEY, typename VALUE>
class wdArrayMapBase
{
  /// \todo Custom comparer

public:
  struct Pair
  {
    KEY key;
    VALUE value;

    WD_DETECT_TYPE_CLASS(KEY, VALUE);

    WD_ALWAYS_INLINE bool operator<(const Pair& rhs) const { return key < rhs.key; }

    WD_ALWAYS_INLINE bool operator==(const Pair& rhs) const { return key == rhs.key; }
  };

  /// \brief Constructor.
  explicit wdArrayMapBase(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copy-Constructor.
  wdArrayMapBase(const wdArrayMapBase& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copy assignment operator.
  void operator=(const wdArrayMapBase& rhs); // [tested]

  /// \brief Returns the number of elements stored in the map.
  wdUInt32 GetCount() const; // [tested]

  /// \brief True if the map contains no elements.
  bool IsEmpty() const; // [tested]

  /// \brief Purges all elements from the map.
  void Clear(); // [tested]

  /// \brief Always inserts a new value under the given key. Duplicates are allowed.
  /// Returns the index of the newly added element.
  template <typename CompatibleKeyType, typename CompatibleValueType>
  wdUInt32 Insert(CompatibleKeyType&& key, CompatibleValueType&& value); // [tested]

  /// \brief Ensures the internal data structure is sorted. This is done automatically every time a lookup needs to be made.
  void Sort() const; // [tested]

  /// \brief Returns an index to one element with the given key. If the key is inserted multiple times, there is no guarantee which one is returned.
  /// Returns wdInvalidIndex when no such element exists.
  template <typename CompatibleKeyType>
  wdUInt32 Find(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns the index to the first element with a key equal or larger than the given key.
  /// Returns wdInvalidIndex when no such element exists.
  /// If there are multiple keys with the same value, the one at the smallest index is returned.
  template <typename CompatibleKeyType>
  wdUInt32 LowerBound(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns the index to the first element with a key that is LARGER than the given key.
  /// Returns wdInvalidIndex when no such element exists.
  /// If there are multiple keys with the same value, the one at the smallest index is returned.
  template <typename CompatibleKeyType>
  wdUInt32 UpperBound(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns the key that is stored at the given index.
  const KEY& GetKey(wdUInt32 uiIndex) const; // [tested]

  /// \brief Returns the value that is stored at the given index.
  const VALUE& GetValue(wdUInt32 uiIndex) const; // [tested]

  /// \brief Returns the value that is stored at the given index.
  VALUE& GetValue(wdUInt32 uiIndex); // [tested]

  /// \brief Returns a reference to the map data array.
  wdDynamicArray<Pair>& GetData();

  /// \brief Returns a constant reference to the map data array.
  const wdDynamicArray<Pair>& GetData() const;

  /// \brief Returns the value stored at the given key. If none exists, one is created. \a bExisted indicates whether an element needed to be created.
  template <typename CompatibleKeyType>
  VALUE& FindOrAdd(const CompatibleKeyType& key, bool* out_pExisted = nullptr); // [tested]

  /// \brief Same as FindOrAdd.
  template <typename CompatibleKeyType>
  VALUE& operator[](const CompatibleKeyType& key); // [tested]

  /// \brief Returns the key/value pair at the given index.
  const Pair& GetPair(wdUInt32 uiIndex) const; // [tested]

  /// \brief Removes the element at the given index.
  ///
  /// If the map is sorted and bKeepSorted is true, the element will be removed such that the map stays sorted.
  /// This is only useful, if only a single (or very few) elements are removed before the next lookup. If multiple values
  /// are removed, or new values are going to be inserted, as well, \a bKeepSorted should be left to false.
  void RemoveAtAndCopy(wdUInt32 uiIndex, bool bKeepSorted = false);

  /// \brief Removes one element with the given key. Returns true, if one was found and removed. If the same key exists multiple times, you need to
  /// call this function multiple times to remove them all.
  ///
  /// If the map is sorted and bKeepSorted is true, the element will be removed such that the map stays sorted.
  /// This is only useful, if only a single (or very few) elements are removed before the next lookup. If multiple values
  /// are removed, or new values are going to be inserted, as well, \a bKeepSorted should be left to false.
  template <typename CompatibleKeyType>
  bool RemoveAndCopy(const CompatibleKeyType& key, bool bKeepSorted = false); // [tested]

  /// \brief Returns whether an element with the given key exists.
  template <typename CompatibleKeyType>
  bool Contains(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns whether an element with the given key and value already exists.
  template <typename CompatibleKeyType>
  bool Contains(const CompatibleKeyType& key, const VALUE& value) const; // [tested]

  /// \brief Reserves enough memory to store \a size elements.
  void Reserve(wdUInt32 uiSize); // [tested]

  /// \brief Compacts the internal memory to not waste any space.
  void Compact(); // [tested]

  /// \brief Compares the two containers for equality.
  bool operator==(const wdArrayMapBase<KEY, VALUE>& rhs) const; // [tested]

  /// \brief Compares the two containers for equality.
  bool operator!=(const wdArrayMapBase<KEY, VALUE>& rhs) const; // [tested]

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  wdUInt64 GetHeapMemoryUsage() const { return m_Data.GetHeapMemoryUsage(); } // [tested]

  using const_iterator = typename wdDynamicArray<Pair>::const_iterator;
  using const_reverse_iterator = typename wdDynamicArray<Pair>::const_reverse_iterator;
  using iterator = typename wdDynamicArray<Pair>::iterator;
  using reverse_iterator = typename wdDynamicArray<Pair>::reverse_iterator;

private:
  mutable bool m_bSorted;
  mutable wdDynamicArray<Pair> m_Data;
};

/// \brief See wdArrayMapBase for details.
template <typename KEY, typename VALUE, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdArrayMap : public wdArrayMapBase<KEY, VALUE>
{
  WD_DECLARE_MEM_RELOCATABLE_TYPE();

public:
  wdArrayMap();
  explicit wdArrayMap(wdAllocatorBase* pAllocator);

  wdArrayMap(const wdArrayMap<KEY, VALUE, AllocatorWrapper>& rhs);
  wdArrayMap(const wdArrayMapBase<KEY, VALUE>& rhs);

  void operator=(const wdArrayMap<KEY, VALUE, AllocatorWrapper>& rhs);
  void operator=(const wdArrayMapBase<KEY, VALUE>& rhs);
};


template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::iterator begin(wdArrayMapBase<KEY, VALUE>& ref_container)
{
  return begin(ref_container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_iterator begin(const wdArrayMapBase<KEY, VALUE>& container)
{
  return begin(container.GetData());
}
template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_iterator cbegin(const wdArrayMapBase<KEY, VALUE>& container)
{
  return cbegin(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::reverse_iterator rbegin(wdArrayMapBase<KEY, VALUE>& ref_container)
{
  return rbegin(ref_container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_reverse_iterator rbegin(const wdArrayMapBase<KEY, VALUE>& container)
{
  return rbegin(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_reverse_iterator crbegin(const wdArrayMapBase<KEY, VALUE>& container)
{
  return crbegin(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::iterator end(wdArrayMapBase<KEY, VALUE>& ref_container)
{
  return end(ref_container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_iterator end(const wdArrayMapBase<KEY, VALUE>& container)
{
  return end(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_iterator cend(const wdArrayMapBase<KEY, VALUE>& container)
{
  return cend(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::reverse_iterator rend(wdArrayMapBase<KEY, VALUE>& ref_container)
{
  return rend(ref_container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_reverse_iterator rend(const wdArrayMapBase<KEY, VALUE>& container)
{
  return rend(container.GetData());
}

template <typename KEY, typename VALUE>
typename wdArrayMapBase<KEY, VALUE>::const_reverse_iterator crend(const wdArrayMapBase<KEY, VALUE>& container)
{
  return crend(container.GetData());
}


#include <Foundation/Containers/Implementation/ArrayMap_inl.h>
