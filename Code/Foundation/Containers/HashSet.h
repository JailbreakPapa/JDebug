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
/// The hash function can be customized by providing a Hasher helper class like nsHashHelper.

/// \see nsHashHelper
template <typename KeyType, typename Hasher>
class nsHashSetBase
{
public:
  /// \brief Const iterator.
  class ConstIterator
  {
  public:
    /// \brief Checks whether this iterator points to a valid element.
    bool IsValid() const; // [tested]

    /// \brief Checks whether the two iterators point to the same element.
    bool operator==(const typename nsHashSetBase<KeyType, Hasher>::ConstIterator& rhs) const;

    NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const typename nsHashSetBase<KeyType, Hasher>::ConstIterator&);

    /// \brief Returns the 'key' of the element that this iterator points to.
    const KeyType& Key() const; // [tested]

    /// \brief Returns the 'key' of the element that this iterator points to.
    NS_ALWAYS_INLINE const KeyType& operator*() const { return Key(); } // [tested]

    /// \brief Advances the iterator to the next element in the map. The iterator will not be valid anymore, if the end is reached.
    void Next(); // [tested]

    /// \brief Shorthand for 'Next'
    void operator++(); // [tested]

  protected:
    friend class nsHashSetBase<KeyType, Hasher>;

    explicit ConstIterator(const nsHashSetBase<KeyType, Hasher>& hashSet);
    void SetToBegin();
    void SetToEnd();

    const nsHashSetBase<KeyType, Hasher>* m_pHashSet = nullptr;
    nsUInt32 m_uiCurrentIndex = 0; // current element index that this iterator points to.
    nsUInt32 m_uiCurrentCount = 0; // current number of valid elements that this iterator has found so far.
  };

protected:
  /// \brief Creates an empty hashset. Does not allocate any data yet.
  explicit nsHashSetBase(nsAllocator* pAllocator); // [tested]

  /// \brief Creates a copy of the given hashset.
  nsHashSetBase(const nsHashSetBase<KeyType, Hasher>& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Moves data from an existing hashtable into this one.
  nsHashSetBase(nsHashSetBase<KeyType, Hasher>&& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Destructor.
  ~nsHashSetBase(); // [tested]

  /// \brief Copies the data from another hashset into this one.
  void operator=(const nsHashSetBase<KeyType, Hasher>& rhs); // [tested]

  /// \brief Moves data from an existing hashset into this one.
  void operator=(nsHashSetBase<KeyType, Hasher>&& rhs); // [tested]

public:
  /// \brief Compares this table to another table.
  bool operator==(const nsHashSetBase<KeyType, Hasher>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsHashSetBase<KeyType, Hasher>&);

  /// \brief Expands the hashset by over-allocating the internal storage so that the load factor is lower or equal to 60% when inserting the
  /// given number of entries.
  void Reserve(nsUInt32 uiCapacity); // [tested]

  /// \brief Tries to compact the hashset to avoid wasting memory.
  ///
  /// The resulting capacity is at least 'GetCount' (no elements get removed).
  /// Will deallocate all data, if the hashset is empty.
  void Compact(); // [tested]

  /// \brief Returns the number of active entries in the table.
  nsUInt32 GetCount() const; // [tested]

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
  bool ContainsSet(const nsHashSetBase<KeyType, Hasher>& operand) const; // [tested]

  /// \brief Makes this set the union of itself and the operand.
  void Union(const nsHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Makes this set the difference of itself and the operand, i.e. subtracts operand.
  void Difference(const nsHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Makes this set the intersection of itself and the operand.
  void Intersection(const nsHashSetBase<KeyType, Hasher>& operand); // [tested]

  /// \brief Returns a constant Iterator to the very first element.
  ConstIterator GetIterator() const; // [tested]

  /// \brief Returns a constant Iterator to the first element that is not part of the hashset. Needed to implement range based for loop
  /// support.
  ConstIterator GetEndIterator() const;

  /// \brief Returns the allocator that is used by this instance.
  nsAllocator* GetAllocator() const;

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const; // [tested]

  /// \brief Swaps this map with the other one.
  void Swap(nsHashSetBase<KeyType, Hasher>& other); // [tested]

  /// \brief Searches for key, returns a ConstIterator to it or an invalid iterator, if no such key is found. O(1) operation.
  template <typename CompatibleKeyType>
  ConstIterator Find(const CompatibleKeyType& key) const;

private:
  KeyType* m_pEntries;
  nsUInt32* m_pEntryFlags;

  nsUInt32 m_uiCount;
  nsUInt32 m_uiCapacity;

  nsAllocator* m_pAllocator;

  enum
  {
    FREE_ENTRY = 0,
    VALID_ENTRY = 1,
    DELETED_ENTRY = 2,
    FLAGS_MASK = 3,
    CAPACITY_ALIGNMENT = 32
  };

  void SetCapacity(nsUInt32 uiCapacity);

  void RemoveInternal(nsUInt32 uiIndex);

  template <typename CompatibleKeyType>
  nsUInt32 FindEntry(const CompatibleKeyType& key) const;

  template <typename CompatibleKeyType>
  nsUInt32 FindEntry(nsUInt32 uiHash, const CompatibleKeyType& key) const;

  nsUInt32 GetFlagsCapacity() const;
  nsUInt32 GetFlags(nsUInt32* pFlags, nsUInt32 uiEntryIndex) const;
  void SetFlags(nsUInt32 uiEntryIndex, nsUInt32 uiFlags);

  bool IsFreeEntry(nsUInt32 uiEntryIndex) const;
  bool IsValidEntry(nsUInt32 uiEntryIndex) const;
  bool IsDeletedEntry(nsUInt32 uiEntryIndex) const;

  void MarkEntryAsFree(nsUInt32 uiEntryIndex);
  void MarkEntryAsValid(nsUInt32 uiEntryIndex);
  void MarkEntryAsDeleted(nsUInt32 uiEntryIndex);
};

/// \brief \see nsHashSetBase
template <typename KeyType, typename Hasher = nsHashHelper<KeyType>, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
class nsHashSet : public nsHashSetBase<KeyType, Hasher>
{
public:
  nsHashSet();
  explicit nsHashSet(nsAllocator* pAllocator);

  nsHashSet(const nsHashSet<KeyType, Hasher, AllocatorWrapper>& other);
  nsHashSet(const nsHashSetBase<KeyType, Hasher>& other);

  nsHashSet(nsHashSet<KeyType, Hasher, AllocatorWrapper>&& other);
  nsHashSet(nsHashSetBase<KeyType, Hasher>&& other);

  void operator=(const nsHashSet<KeyType, Hasher, AllocatorWrapper>& rhs);
  void operator=(const nsHashSetBase<KeyType, Hasher>& rhs);

  void operator=(nsHashSet<KeyType, Hasher, AllocatorWrapper>&& rhs);
  void operator=(nsHashSetBase<KeyType, Hasher>&& rhs);
};

template <typename KeyType, typename Hasher>
typename nsHashSetBase<KeyType, Hasher>::ConstIterator begin(const nsHashSetBase<KeyType, Hasher>& set)
{
  return set.GetIterator();
}

template <typename KeyType, typename Hasher>
typename nsHashSetBase<KeyType, Hasher>::ConstIterator cbegin(const nsHashSetBase<KeyType, Hasher>& set)
{
  return set.GetIterator();
}

template <typename KeyType, typename Hasher>
typename nsHashSetBase<KeyType, Hasher>::ConstIterator end(const nsHashSetBase<KeyType, Hasher>& set)
{
  return set.GetEndIterator();
}

template <typename KeyType, typename Hasher>
typename nsHashSetBase<KeyType, Hasher>::ConstIterator cend(const nsHashSetBase<KeyType, Hasher>& set)
{
  return set.GetEndIterator();
}

#include <Foundation/Containers/Implementation/HashSet_inl.h>
