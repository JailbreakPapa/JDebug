#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/AllocatorWrapper.h>

/// \brief Implementation of a hashset.
///
/// The hashset stores values by using the hash as an index into the table.
/// This implementation uses linear-probing to resolve hash collisions which means all values are stored
/// in a linear array.
/// All insertion/erasure/lookup functions take O(1) time if the table does not need to be expanded,
/// which happens when the load gets greater than 60%.
/// The hash function can be customized by providing a Hasher helper class like wdHashHelper.

/// \see wdHashHelper
template <typename KeyType, typename Hasher>
class wdHashSetBase
{
public:
  /// \brief Const iterator.
  class ConstIterator
  {
  public:
    /// \brief Checks whether this iterator points to a valid element.
    bool IsValid() const; // [tested]

    /// \brief Checks whether the two iterators point to the same element.
    bool operator==(const typename wdHashSetBase<KeyType, Hasher>::ConstIterator& rhs) const;

    /// \brief Checks whether the two iterators point to the same element.
    bool operator!=(const typename wdHashSetBase<KeyType, Hasher>::ConstIterator& rhs) const;

    /// \brief Returns the 'key' of the element that this iterator points to.
    const KeyType& Key() const; // [tested]

    /// \brief Returns the 'key' of the element that this iterator points to.
    WD_ALWAYS_INLINE const KeyType& operator*() { return Key(); } // [tested]

    /// \brief Advances the iterator to the next element in the map. The iterator will not be valid anymore, if the end is reached.
    void Next(); // [tested]

    /// \brief Shorthand for 'Next'
    void operator++(); // [tested]

  protected:
    friend class wdHashSetBase<KeyType, Hasher>;

    explicit ConstIterator(const wdHashSetBase<KeyType, Hasher>& hashSet);
    void SetToBegin();
    void SetToEnd();

    const wdHashSetBase<KeyType, Hasher>* m_pHashSet = nullptr;
    wdUInt32 m_uiCurrentIndex = 0; // current element index that this iterator points to.
    wdUInt32 m_uiCurrentCount = 0; // current number of valid elements that this iterator has found so far.
  };

protected:
  /// \brief Creates an empty hashset. Does not allocate any data yet.
  explicit wdHashSetBase(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Creates a copy of the given hashset.
  wdHashSetBase(const wdHashSetBase<KeyType, Hasher>& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Moves data from an existing hashtable into this one.
  wdHashSetBase(wdHashSetBase<KeyType, Hasher>&& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Destructor.
  ~wdHashSetBase(); // [tested]

  /// \brief Copies the data from another hashset into this one.
  void operator=(const wdHashSetBase<KeyType, Hasher>& rhs); // [tested]

  /// \brief Moves data from an existing hashset into this one.
  void operator=(wdHashSetBase<KeyType, Hasher>&& rhs); // [tested]

public:
  /// \brief Compares this table to another table.
  bool operator==(const wdHashSetBase<KeyType, Hasher>& rhs) const; // [tested]

  /// \brief Compares this table to another table.
  bool operator!=(const wdHashSetBase<KeyType, Hasher>& rhs) const; // [tested]

  /// \brief Expands the hashset by over-allocating the internal storage so that the load factor is lower or equal to 60% when inserting the
  /// given number of entries.
  void Reserve(wdUInt32 uiCapacity); // [tested]

  /// \brief Tries to compact the hashset to avoid wasting memory.
  ///
  /// The resulting capacity is at least 'GetCount' (no elements get removed).
  /// Will deallocate all data, if the hashset is empty.
  void Compact(); // [tested]

  /// \brief Returns the number of active entries in the table.
  wdUInt32 GetCount() const; // [tested]

  /// \brief Returns true, if the hashset does not contain any elements.
  bool IsEmpty() const; // [tested]

  /// \brief Clears the table.
  void Clear(); // [tested]

  /// \brief Inserts the key. Returns whether the key was already existing.
  template <typename CompatibleKeyType>
  bool Insert(CompatibleKeyType&& key); // [tested]

  /// \brief Removes the entry with the given key. Returns if an entry was removed.
  template <typename CompatibleKeyType>
  bool Remove(const CompatibleKeyType& key); // [tested]

  /// \brief Erases the key at the given Iterator. Returns an iterator to the element after the given iterator.
  ConstIterator Remove(const ConstIterator& pos); // [tested]

  /// \brief Returns if an entry with given key exists in the table.
  template <typename CompatibleKeyType>
  bool Contains(const CompatibleKeyType& key) const; // [tested]

  /// \brief Checks whether all keys of the given set are in the container.
  bool ContainsSet(const wdHashSetBase<KeyType, Hasher>& operand) const; // [tested]

  /// \brief Makes this set the union of itself and the operand.
  void Union(const wdHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Makes this set the difference of itself and the operand, i.e. subtracts operand.
  void Difference(const wdHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Makes this set the intersection of itself and the operand.
  void Intersection(const wdHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Returns a constant Iterator to the very first element.
  ConstIterator GetIterator() const; // [tested]

  /// \brief Returns a constant Iterator to the first element that is not part of the hashset. Needed to implement range based for loop
  /// support.
  ConstIterator GetEndIterator() const;

  /// \brief Returns the allocator that is used by this instance.
  wdAllocatorBase* GetAllocator() const;

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  wdUInt64 GetHeapMemoryUsage() const; // [tested]

  /// \brief Swaps this map with the other one.
  void Swap(wdHashSetBase<KeyType, Hasher>& other); // [tested]

private:
  KeyType* m_pEntries;
  wdUInt32* m_pEntryFlags;

  wdUInt32 m_uiCount;
  wdUInt32 m_uiCapacity;

  wdAllocatorBase* m_pAllocator;

  enum
  {
    FREE_ENTRY = 0,
    VALID_ENTRY = 1,
    DELETED_ENTRY = 2,
    FLAGS_MASK = 3,
    CAPACITY_ALIGNMENT = 32
  };

  void SetCapacity(wdUInt32 uiCapacity);

  void RemoveInternal(wdUInt32 uiIndex);

  template <typename CompatibleKeyType>
  wdUInt32 FindEntry(const CompatibleKeyType& key) const;

  template <typename CompatibleKeyType>
  wdUInt32 FindEntry(wdUInt32 uiHash, const CompatibleKeyType& key) const;

  wdUInt32 GetFlagsCapacity() const;
  wdUInt32 GetFlags(wdUInt32* pFlags, wdUInt32 uiEntryIndex) const;
  void SetFlags(wdUInt32 uiEntryIndex, wdUInt32 uiFlags);

  bool IsFreeEntry(wdUInt32 uiEntryIndex) const;
  bool IsValidEntry(wdUInt32 uiEntryIndex) const;
  bool IsDeletedEntry(wdUInt32 uiEntryIndex) const;

  void MarkEntryAsFree(wdUInt32 uiEntryIndex);
  void MarkEntryAsValid(wdUInt32 uiEntryIndex);
  void MarkEntryAsDeleted(wdUInt32 uiEntryIndex);
};

/// \brief \see wdHashSetBase
template <typename KeyType, typename Hasher = wdHashHelper<KeyType>, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdHashSet : public wdHashSetBase<KeyType, Hasher>
{
public:
  wdHashSet();
  explicit wdHashSet(wdAllocatorBase* pAllocator);

  wdHashSet(const wdHashSet<KeyType, Hasher, AllocatorWrapper>& other);
  wdHashSet(const wdHashSetBase<KeyType, Hasher>& other);

  wdHashSet(wdHashSet<KeyType, Hasher, AllocatorWrapper>&& other);
  wdHashSet(wdHashSetBase<KeyType, Hasher>&& other);

  void operator=(const wdHashSet<KeyType, Hasher, AllocatorWrapper>& rhs);
  void operator=(const wdHashSetBase<KeyType, Hasher>& rhs);

  void operator=(wdHashSet<KeyType, Hasher, AllocatorWrapper>&& rhs);
  void operator=(wdHashSetBase<KeyType, Hasher>&& rhs);
};

template <typename KeyType, typename Hasher>
typename wdHashSetBase<KeyType, Hasher>::ConstIterator begin(const wdHashSetBase<KeyType, Hasher>& set)
{
  return set.GetIterator();
}

template <typename KeyType, typename Hasher>
typename wdHashSetBase<KeyType, Hasher>::ConstIterator cbegin(const wdHashSetBase<KeyType, Hasher>& set)
{
  return set.GetIterator();
}

template <typename KeyType, typename Hasher>
typename wdHashSetBase<KeyType, Hasher>::ConstIterator end(const wdHashSetBase<KeyType, Hasher>& set)
{
  return set.GetEndIterator();
}

template <typename KeyType, typename Hasher>
typename wdHashSetBase<KeyType, Hasher>::ConstIterator cend(const wdHashSetBase<KeyType, Hasher>& set)
{
  return set.GetEndIterator();
}

#include <Foundation/Containers/Implementation/HashSet_inl.h>
