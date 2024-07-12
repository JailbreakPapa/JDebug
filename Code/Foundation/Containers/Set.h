#pragma once

#include <Foundation/Containers/Deque.h>

/// \brief A set container that only stores whether an element resides in it or not. Similar to STL::set
///
/// Sets are similar to maps that do not store a value (or only a bool that is always true).
/// Sets can be used to reduce an unordered number of elements to only those that are unique.
/// Insertion/erasure/lookup in sets is quite fast (O (log n)).
/// This container is implemented with a red-black tree, so it will always be a balanced tree.
template <typename KeyType, typename Comparer>
class nsSetBase
{
private:
  struct Node;

  /// \brief Only used by the sentinel node.
  struct NilNode
  {
    nsUInt16 m_uiLevel = 0;
    Node* m_pParent = nullptr;
    Node* m_pLink[2] = {nullptr, nullptr};
  };

  /// \brief A node storing the key
  struct Node : public NilNode
  {
    KeyType m_Key;
  };

public:
  /// \brief Base class for all iterators.
  template <bool REVERSE>
  struct IteratorBase
  {
    using iterator_category = std::forward_iterator_tag;
    using value_type = IteratorBase<REVERSE>;
    using difference_type = std::ptrdiff_t;
    using pointer = IteratorBase<REVERSE>*;
    using reference = IteratorBase<REVERSE>&;

    NS_DECLARE_POD_TYPE();

    /// \brief Constructs an invalid iterator.
    NS_ALWAYS_INLINE IteratorBase()
      : m_pElement(nullptr)
    {
    } // [tested]

    /// \brief Checks whether this iterator points to a valid element.
    NS_ALWAYS_INLINE bool IsValid() const { return (m_pElement != nullptr); } // [tested]

    /// \brief Checks whether the two iterators point to the same element.
    NS_ALWAYS_INLINE bool operator==(const typename nsSetBase<KeyType, Comparer>::IteratorBase<REVERSE>& it2) const { return (m_pElement == it2.m_pElement); }
    NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const typename nsSetBase<KeyType, Comparer>::IteratorBase<REVERSE>&);

    /// \brief Returns the 'key' of the element that this iterator points to.
    NS_FORCE_INLINE const KeyType& Key() const
    {
      NS_ASSERT_DEBUG(IsValid(), "Cannot access the 'key' of an invalid iterator.");
      return m_pElement->m_Key;
    } // [tested]

    /// \brief Returns the 'key' of the element that this iterator points to.
    NS_ALWAYS_INLINE const KeyType& operator*() const { return Key(); }

    /// \brief Advances the iterator to the next element in the set. The iterator will not be valid anymore, if the end is reached.
    void Next(); // [tested]

    /// \brief Advances the iterator to the previous element in the set. The iterator will not be valid anymore, if the end is reached.
    void Prev(); // [tested]

    /// \brief Shorthand for 'Next'
    NS_ALWAYS_INLINE void operator++() { Next(); } // [tested]

    /// \brief Shorthand for 'Prev'
    NS_ALWAYS_INLINE void operator--() { Prev(); } // [tested]

  protected:
    void Advance(nsInt32 dir0, nsInt32 dir1);

    friend class nsSetBase<KeyType, Comparer>;

    NS_ALWAYS_INLINE explicit IteratorBase(Node* pInit)
      : m_pElement(pInit)
    {
    }

    Node* m_pElement;
  };

  using Iterator = IteratorBase<false>;
  using ReverseIterator = IteratorBase<true>;

protected:
  /// \brief Initializes the set to be empty.
  nsSetBase(const Comparer& comparer, nsAllocator* pAllocator); // [tested]

  /// \brief Copies all keys from the given set into this one.
  nsSetBase(const nsSetBase<KeyType, Comparer>& cc, nsAllocator* pAllocator); // [tested]

  /// \brief Destroys all elements in the set.
  ~nsSetBase(); // [tested]

  /// \brief Copies all keys from the given set into this one.
  void operator=(const nsSetBase<KeyType, Comparer>& rhs); // [tested]

public:
  /// \brief Returns whether there are no elements in the set. O(1) operation.
  bool IsEmpty() const; // [tested]

  /// \brief Returns the number of elements currently stored in the set. O(1) operation.
  nsUInt32 GetCount() const; // [tested]

  /// \brief Destroys all elements in the set and resets its size to zero.
  void Clear(); // [tested]

  /// \brief Returns a constant Iterator to the very first element.
  Iterator GetIterator() const; // [tested]

  /// \brief Returns a constant ReverseIterator to the very last element.
  ReverseIterator GetReverseIterator() const; // [tested]

  /// \brief Inserts the key into the tree and returns an Iterator to it. O(log n) operation.
  template <typename CompatibleKeyType>
  Iterator Insert(CompatibleKeyType&& key); // [tested]

  /// \brief Erases the element with the given key, if it exists. O(log n) operation.
  template <typename CompatibleKeyType>
  bool Remove(const CompatibleKeyType& key); // [tested]

  /// \brief Erases the element at the given Iterator. O(log n) operation.
  Iterator Remove(const Iterator& pos); // [tested]

  /// \brief Searches for key, returns an Iterator to it or an invalid iterator, if no such key is found. O(log n) operation.
  template <typename CompatibleKeyType>
  Iterator Find(const CompatibleKeyType& key) const; // [tested]

  /// \brief Checks whether the given key is in the container.
  template <typename CompatibleKeyType>
  bool Contains(const CompatibleKeyType& key) const; // [tested]

  /// \brief Checks whether all keys of the given set are in the container.
  bool ContainsSet(const nsSetBase<KeyType, Comparer>& operand) const; // [tested]

  /// \brief Returns an Iterator to the element with a key equal or larger than the given key. Returns an invalid iterator, if there is no such
  /// element.
  template <typename CompatibleKeyType>
  Iterator LowerBound(const CompatibleKeyType& key) const; // [tested]

  /// \brief Returns an Iterator to the element with a key that is LARGER than the given key. Returns an invalid iterator, if there is no such
  /// element.
  template <typename CompatibleKeyType>
  Iterator UpperBound(const CompatibleKeyType& key) const; // [tested]

  /// \brief Makes this set the union of itself and the operand.
  void Union(const nsSetBase<KeyType, Comparer>& operand); // [tested]

  /// \brief Makes this set the difference of itself and the operand, i.e. subtracts operand.
  void Difference(const nsSetBase<KeyType, Comparer>& operand); // [tested]

  /// \brief Makes this set the intersection of itself and the operand.
  void Intersection(const nsSetBase<KeyType, Comparer>& operand); // [tested]

  /// \brief Returns the allocator that is used by this instance.
  nsAllocator* GetAllocator() const { return m_Elements.GetAllocator(); }

  /// \brief Comparison operator
  bool operator==(const nsSetBase<KeyType, Comparer>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsSetBase<KeyType, Comparer>&);

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const { return m_Elements.GetHeapMemoryUsage(); } // [tested]

  /// \brief Swaps this map with the other one.
  void Swap(nsSetBase<KeyType, Comparer>& other); // [tested]

private:
  template <typename CompatibleKeyType>
  Node* Internal_Find(const CompatibleKeyType& key) const;
  template <typename CompatibleKeyType>
  Node* Internal_LowerBound(const CompatibleKeyType& key) const;
  template <typename CompatibleKeyType>
  Node* Internal_UpperBound(const CompatibleKeyType& key) const;

private:
  void Constructor();

  /// \brief Creates one new node and initializes it.
  template <typename CompatibleKeyType>
  Node* AcquireNode(CompatibleKeyType&& key, nsUInt16 uiLevel, Node* pParent);

  /// \brief Destroys the given node.
  void ReleaseNode(Node* pNode);

  /// \brief Red-Black Tree stuff(Anderson Tree to be exact).
  ///
  /// Code taken from here: http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_andersson.aspx
  Node* SkewNode(Node* root);
  Node* SplitNode(Node* root);

  template <typename CompatibleKeyType>
  Node* Insert(Node* root, CompatibleKeyType&& key, Node*& pInsertedNode);
  template <typename CompatibleKeyType>
  Node* Remove(Node* root, const CompatibleKeyType& key, bool& bRemoved);

  /// \brief Returns the left-most node of the tree(smallest key).
  Node* GetLeftMost() const;

  /// \brief Returns the right-most node of the tree(largest key).
  Node* GetRightMost() const;

  /// \brief Needed during Swap() to fix up the NilNode pointers from one container to the other
  void SwapNilNode(Node*& pCurNode, NilNode* pOld, NilNode* pNew);

  /// \brief Root node of the tree.
  Node* m_pRoot;

  /// \brief Sentinel node.
  NilNode m_NilNode;

  /// \brief Number of active nodes in the tree.
  nsUInt32 m_uiCount;

  /// \brief Data store. Keeps all the nodes.
  nsDeque<Node, nsNullAllocatorWrapper, false> m_Elements;

  /// \brief Stack of recently discarded nodes to quickly acquire new nodes.
  Node* m_pFreeElementStack;

  /// \brief Comparer object
  Comparer m_Comparer;
};

/// \brief \see nsSetBase
template <typename KeyType, typename Comparer = nsCompareHelper<KeyType>, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
class nsSet : public nsSetBase<KeyType, Comparer>
{
public:
  nsSet();
  explicit nsSet(nsAllocator* pAllocator);
  nsSet(const Comparer& comparer, nsAllocator* pAllocator);

  nsSet(const nsSet<KeyType, Comparer, AllocatorWrapper>& other);
  nsSet(const nsSetBase<KeyType, Comparer>& other);

  void operator=(const nsSet<KeyType, Comparer, AllocatorWrapper>& rhs);
  void operator=(const nsSetBase<KeyType, Comparer>& rhs);
};


template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator begin(nsSetBase<KeyType, Comparer>& ref_container)
{
  return ref_container.GetIterator();
}

template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator begin(const nsSetBase<KeyType, Comparer>& container)
{
  return container.GetIterator();
}

template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator cbegin(const nsSetBase<KeyType, Comparer>& container)
{
  return container.GetIterator();
}

template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator end(nsSetBase<KeyType, Comparer>& ref_container)
{
  return typename nsSetBase<KeyType, Comparer>::Iterator();
}

template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator end(const nsSetBase<KeyType, Comparer>& container)
{
  return typename nsSetBase<KeyType, Comparer>::Iterator();
}

template <typename KeyType, typename Comparer>
typename nsSetBase<KeyType, Comparer>::Iterator cend(const nsSetBase<KeyType, Comparer>& container)
{
  return typename nsSetBase<KeyType, Comparer>::Iterator();
}


#include <Foundation/Containers/Implementation/Set_inl.h>
