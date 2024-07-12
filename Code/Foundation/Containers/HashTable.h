#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/AllocatorWrapper.h>

template <typename KeyType, typename ValueType, typename Hasher>
class nsHashTableBase;

/// \brief Const iterator.
template <typename KeyType, typename ValueType, typename Hasher>
struct nsHashTableBaseConstIterator
{
  using iterator_category = std::forward_iterator_tag;
  using value_type = nsHashTableBaseConstIterator;
  using difference_type = std::ptrdiff_t;
  using pointer = nsHashTableBaseConstIterator*;
  using reference = nsHashTableBaseConstIterator&;

  NS_DECLARE_POD_TYPE();

  /// \brief Checks whether this iterator points to a valid element.
  bool IsValid() const; // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  bool operator==(const nsHashTableBaseConstIterator& rhs) const;
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsHashTableBaseConstIterator&);

  /// \brief Returns the 'key' of the element that this iterator points to.
  const KeyType& Key() const; // [tested]

  /// \brief Returns the 'value' of the element that this iterator points to.
  const ValueType& Value() const; // [tested]

  /// \brief Advances the iterator to the next element in the map. The iterator will not be valid anymore, if the end is reached.
  void Next(); // [tested]

  /// \brief Shorthand for 'Next'
  void operator++(); // [tested]

  /// \brief Returns '*this' to enable foreach
  NS_ALWAYS_INLINE nsHashTableBaseConstIterator& operator*() { return *this; } // [tested]

protected:
  friend class nsHashTableBase<KeyType, ValueType, Hasher>;

  explicit nsHashTableBaseConstIterator(const nsHashTableBase<KeyType, ValueType, Hasher>& hashTable);
  void SetToBegin();
  void SetToEnd();

  const nsHashTableBase<KeyType, ValueType, Hasher>* m_pHashTable = nullptr;
  nsUInt32 m_uiCurrentIndex = 0; // current element index that this iterator points to.
  nsUInt32 m_uiCurrentCount = 0; // current number of valid elements that this iterator has found so far.

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)
public:
  struct Pointer
  {
    std::pair<const KeyType&, const ValueType&> value;
    const std::pair<const KeyType&, const ValueType&>* operator->() const { return &value; }
  };

  NS_ALWAYS_INLINE Pointer operator->() const
  {
    return Pointer{.value = {Key(), Value()}};
  }

  // These function is used to return the values for structured bindings.
  // The number and type of type of each slot are defined in the inl file.
  template <std::size_t Index>
  std::tuple_element_t<Index, nsHashTableBaseConstIterator>& get() const
  {
    if constexpr (Index == 0)
      return Key();
    if constexpr (Index == 1)
      return Value();
  }
#endif
};

/// \brief Iterator with write access.
template <typename KeyType, typename ValueType, typename Hasher>
struct nsHashTableBaseIterator : public nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>
{
  NS_DECLARE_POD_TYPE();

  /// \brief Creates a new iterator from another.
  NS_ALWAYS_INLINE nsHashTableBaseIterator(const nsHashTableBaseIterator& rhs); // [tested]

  /// \brief Assigns one iterator no another.
  NS_ALWAYS_INLINE void operator=(const nsHashTableBaseIterator& rhs); // [tested]

  // this is required to pull in the const version of this function
  using nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>::Value;

  /// \brief Returns the 'value' of the element that this iterator points to.
  NS_FORCE_INLINE ValueType& Value(); // [tested]

  /// \brief Returns the 'value' of the element that this iterator points to.
  NS_FORCE_INLINE ValueType& Value() const;

  /// \brief Returns '*this' to enable foreach
  NS_ALWAYS_INLINE nsHashTableBaseIterator& operator*() { return *this; } // [tested]

private:
  friend class nsHashTableBase<KeyType, ValueType, Hasher>;

  explicit nsHashTableBaseIterator(const nsHashTableBase<KeyType, ValueType, Hasher>& hashTable);

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)
public:
  // These functions are used to return the values for structured bindings.
  // The number and type of type of each slot are defined in the inl file.
  template <std::size_t Index>
  std::tuple_element_t<Index, nsHashTableBaseIterator>& get()
  {
    if constexpr (Index == 0)
      return nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>::Key();
    if constexpr (Index == 1)
      return Value();
  }

  template <std::size_t Index>
  std::tuple_element_t<Index, nsHashTableBaseIterator>& get() const
  {
    if constexpr (Index == 0)
      return nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>::Key();
    if constexpr (Index == 1)
      return Value();
  }
#endif
};

/// \brief Implementation of a hashtable which stores key/value pairs.
///
/// The hashtable maps keys to values by using the hash of the key as an index into the table.
/// This implementation uses linear-probing to resolve hash collisions which means all key/value pairs are stored
/// in a linear array.
/// All insertion/erasure/lookup functions take O(1) time if the table does not need to be expanded,
/// which happens when the load gets greater than 60%.
/// The hash function can be customized by providing a Hasher helper class like nsHashHelper.

/// \see nsHashHelper
template <typename KeyType, typename ValueType, typename Hasher>
class nsHashTableBase
{
public:
  using Iterator = nsHashTableBaseIterator<KeyType, ValueType, Hasher>;
  using ConstIterator = nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>;

protected:
  /// \brief Creates an empty hashtable. Does not allocate any data yet.
  explicit nsHashTableBase(nsAllocator* pAllocator); // [tested]

  /// \brief Creates a copy of the given hashtable.
  nsHashTableBase(const nsHashTableBase<KeyType, ValueType, Hasher>& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Moves data from an existing hashtable into this one.
  nsHashTableBase(nsHashTableBase<KeyType, ValueType, Hasher>&& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Destructor.
  ~nsHashTableBase(); // [tested]

  /// \brief Copies the data from another hashtable into this one.
  void operator=(const nsHashTableBase<KeyType, ValueType, Hasher>& rhs); // [tested]

  /// \brief Moves data from an existing hashtable into this one.
  void operator=(nsHashTableBase<KeyType, ValueType, Hasher>&& rhs); // [tested]

public:
  /// \brief Compares this table to another table.
  bool operator==(const nsHashTableBase<KeyType, ValueType, Hasher>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsHashTableBase<KeyType, ValueType, Hasher>&);

  /// \brief Expands the hashtable by over-allocating the internal storage so that the load factor is lower or equal to 60% when inserting the given
  /// number of entries.
  void Reserve(nsUInt32 uiCapacity); // [tested]

  /// \brief Tries to compact the hashtable to avoid wasting memory.
  ///
  /// The resulting capacity is at least 'GetCount' (no elements get removed).
  /// Will deallocate all data, if the hashtable is empty.
  void Compact(); // [tested]

  /// \brief Returns the number of active entries in the table.
  nsUInt32 GetCount() const; // [tested]

  /// \brief Returns true, if the hashtable does not contain any elements.
  bool IsEmpty() const; // [tested]

  /// \brief Clears the table.
  void Clear(); // [tested]

  /// \brief Inserts the key value pair or replaces value if an entry with the given key already exists.
  ///
  /// Returns true if an existing value was replaced and optionally writes out the old value to out_oldValue.
  template <typename CompatibleKeyType, typename CompatibleValueType>
  bool Insert(CompatibleKeyType&& key, CompatibleValueType&& value, ValueType* out_pOldValue = nullptr); // [tested]

  /// \brief Removes the entry with the given key. Returns whether an entry was removed and optionally writes out the old value to out_oldValue.
  template <typename CompatibleKeyType>
  bool Remove(const CompatibleKeyType& key, ValueType* out_pOldValue = nullptr); // [tested]

  /// \brief Erases the key/value pair at the given Iterator. Returns an iterator to the element after the given iterator.
  Iterator Remove(const Iterator& pos); // [tested]

  /// \brief Cannot remove an element with just a nsHashTableBaseConstIterator
  void Remove(const ConstIterator& pos) = delete;

  /// \brief Returns whether an entry with the given key was found and if found writes out the corresponding value to out_value.
  template <typename CompatibleKeyType>
  bool TryGetValue(const CompatibleKeyType& key, ValueType& out_value) const; // [tested]

  /// \brief Returns whether an entry with the given key was found and if found writes out the pointer to the corresponding value to out_pValue.
  template <typename CompatibleKeyType>
  bool TryGetValue(const CompatibleKeyType& key, const ValueType*& out_pValue) const; // [tested]

  /// \brief Returns whether an entry with the given key was found and if found writes out the pointer to the corresponding value to out_pValue.
  template <typename CompatibleKeyType>
  bool TryGetValue(const CompatibleKeyType& key, ValueType*& out_pValue) const; // [tested]

  /// \brief Searches for key, returns a nsHashTableBaseConstIterator to it or an invalid iterator, if no such key is found. O(1) operation.
  template <typename CompatibleKeyType>
  ConstIterator Find(const CompatibleKeyType& key) const;

  /// \brief Searches for key, returns an Iterator to it or an invalid iterator, if no such key is found. O(1) operation.
  template <typename CompatibleKeyType>
  Iterator Find(const CompatibleKeyType& key);

  /// \brief Returns a pointer to the value of the entry with the given key if found, otherwise returns nullptr.
  template <typename CompatibleKeyType>
  const ValueType* GetValue(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns a pointer to the value of the entry with the given key if found, otherwise returns nullptr.
  template <typename CompatibleKeyType>
  ValueType* GetValue(const CompatibleKeyType& key); // [tested]

  /// \brief Returns the value to the given key if found or creates a new entry with the given key and a default constructed value.
  ValueType& operator[](const KeyType& key); // [tested]

  /// \brief Returns the value stored at the given key. If none exists, one is created. \a bExisted indicates whether an element needed to be created.
  ValueType& FindOrAdd(const KeyType& key, bool* out_pExisted); // [tested]

  /// \brief Returns if an entry with given key exists in the table.
  template <typename CompatibleKeyType>
  bool Contains(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns an Iterator to the very first element.
  Iterator GetIterator(); // [tested]

  /// \brief Returns an Iterator to the first element that is not part of the hash-table. Needed to support range based for loops.
  Iterator GetEndIterator(); // [tested]

  /// \brief Returns a constant Iterator to the very first element.
  ConstIterator GetIterator() const; // [tested]

  /// \brief Returns a nsHashTableBaseConstIterator to the first element that is not part of the hash-table. Needed to support range based for loops.
  ConstIterator GetEndIterator() const; // [tested]

  /// \brief Returns the allocator that is used by this instance.
  nsAllocator* GetAllocator() const;

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const; // [tested]

  /// \brief Swaps this map with the other one.
  void Swap(nsHashTableBase<KeyType, ValueType, Hasher>& other); // [tested]

private:
  friend struct nsHashTableBaseConstIterator<KeyType, ValueType, Hasher>;
  friend struct nsHashTableBaseIterator<KeyType, ValueType, Hasher>;

  struct Entry
  {
    KeyType key;
    ValueType value;
  };

  Entry* m_pEntries = nullptr;
  nsUInt32* m_pEntryFlags = nullptr;

  nsUInt32 m_uiCount = 0;
  nsUInt32 m_uiCapacity = 0;

  nsAllocator* m_pAllocator = nullptr;

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

/// \brief \see nsHashTableBase
template <typename KeyType, typename ValueType, typename Hasher = nsHashHelper<KeyType>, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
class nsHashTable : public nsHashTableBase<KeyType, ValueType, Hasher>
{
public:
  nsHashTable();
  explicit nsHashTable(nsAllocator* pAllocator);

  nsHashTable(const nsHashTable<KeyType, ValueType, Hasher, AllocatorWrapper>& other);
  nsHashTable(const nsHashTableBase<KeyType, ValueType, Hasher>& other);

  nsHashTable(nsHashTable<KeyType, ValueType, Hasher, AllocatorWrapper>&& other);
  nsHashTable(nsHashTableBase<KeyType, ValueType, Hasher>&& other);


  void operator=(const nsHashTable<KeyType, ValueType, Hasher, AllocatorWrapper>& rhs);
  void operator=(const nsHashTableBase<KeyType, ValueType, Hasher>& rhs);

  void operator=(nsHashTable<KeyType, ValueType, Hasher, AllocatorWrapper>&& rhs);
  void operator=(nsHashTableBase<KeyType, ValueType, Hasher>&& rhs);
};

//////////////////////////////////////////////////////////////////////////
// begin() /end() for range-based for-loop support

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::Iterator begin(nsHashTableBase<KeyType, ValueType, Hasher>& ref_container)
{
  return ref_container.GetIterator();
}

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::ConstIterator begin(const nsHashTableBase<KeyType, ValueType, Hasher>& container)
{
  return container.GetIterator();
}

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::ConstIterator cbegin(const nsHashTableBase<KeyType, ValueType, Hasher>& container)
{
  return container.GetIterator();
}

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::Iterator end(nsHashTableBase<KeyType, ValueType, Hasher>& ref_container)
{
  return ref_container.GetEndIterator();
}

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::ConstIterator end(const nsHashTableBase<KeyType, ValueType, Hasher>& container)
{
  return container.GetEndIterator();
}

template <typename KeyType, typename ValueType, typename Hasher>
typename nsHashTableBase<KeyType, ValueType, Hasher>::ConstIterator cend(const nsHashTableBase<KeyType, ValueType, Hasher>& container)
{
  return container.GetEndIterator();
}

#include <Foundation/Containers/Implementation/HashTable_inl.h>
