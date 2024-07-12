#pragma once

#include <Foundation/Types/Delegate.h>

/// \brief This class uses delegates to define a range of values that can be enumerated using a forward iterator.
///
/// Can be used to create a contiguous view to elements of a certain type without the need for them to actually
/// exist in the same space or format. Think of IEnumerable in c# using composition via nsDelegate instead of derivation.
/// ValueType defines the value type we are iterating over and IteratorType is the internal key to identify an element.
/// An example that creates a RangeView of strings that are stored in a linear array of structs.
/// \code{.cpp}
/// auto range = nsRangeView<const char*, nsUInt32>(
///   [this]()-> nsUInt32 { return 0; },
///   [this]()-> nsUInt32 { return array.GetCount(); },
///   [this](nsUInt32& it) { ++it; },
///   [this](const nsUInt32& it)-> const char* { return array[it].m_String; });
///
/// for (const char* szValue : range)
/// {
/// }
/// \endcode
template <typename ValueType, typename IteratorType>
class nsRangeView
{
public:
  using BeginCallback = nsDelegate<IteratorType()>;
  using EndCallback = nsDelegate<IteratorType()>;
  using NextCallback = nsDelegate<void(IteratorType&)>;
  using ValueCallback = nsDelegate<ValueType(const IteratorType&)>;

  /// \brief Initializes the nsRangeView with the delegates used to enumerate the range.
  NS_ALWAYS_INLINE nsRangeView(BeginCallback begin, EndCallback end, NextCallback next, ValueCallback value);

  /// \brief Const iterator, don't use directly, use ranged based for loops or call begin() end().
  struct ConstIterator
  {
    NS_DECLARE_POD_TYPE();

    using iterator_category = std::forward_iterator_tag;
    using value_type = ConstIterator;
    using pointer = ConstIterator*;
    using reference = ConstIterator&;

    NS_ALWAYS_INLINE ConstIterator(const ConstIterator& rhs) = default;
    NS_FORCE_INLINE void Next();
    NS_FORCE_INLINE ValueType Value() const;
    NS_ALWAYS_INLINE ValueType operator*() const { return Value(); }
    NS_ALWAYS_INLINE void operator++() { Next(); }
    NS_FORCE_INLINE bool operator==(const typename nsRangeView<ValueType, IteratorType>::ConstIterator& it2) const;
    NS_FORCE_INLINE bool operator!=(const typename nsRangeView<ValueType, IteratorType>::ConstIterator& it2) const;

  protected:
    NS_FORCE_INLINE explicit ConstIterator(const nsRangeView<ValueType, IteratorType>* view, IteratorType pos);

    friend class nsRangeView<ValueType, IteratorType>;
    const nsRangeView<ValueType, IteratorType>* m_pView = nullptr;
    IteratorType m_Pos;
  };

  /// \brief Iterator, don't use directly, use ranged based for loops or call begin() end().
  struct Iterator : public ConstIterator
  {
    NS_DECLARE_POD_TYPE();

    using iterator_category = std::forward_iterator_tag;
    using value_type = Iterator;
    using pointer = Iterator*;
    using reference = Iterator&;

    using ConstIterator::Value;
    NS_ALWAYS_INLINE Iterator(const Iterator& rhs) = default;
    NS_FORCE_INLINE ValueType Value();
    NS_ALWAYS_INLINE ValueType operator*() { return Value(); }

  protected:
    NS_FORCE_INLINE explicit Iterator(const nsRangeView<ValueType, IteratorType>* view, IteratorType pos);
  };

  Iterator begin() { return Iterator(this, m_Begin()); }
  Iterator end() { return Iterator(this, m_End()); }
  ConstIterator begin() const { return ConstIterator(this, m_Begin()); }
  ConstIterator end() const { return ConstIterator(this, m_End()); }
  ConstIterator cbegin() const { return ConstIterator(this, m_Begin()); }
  ConstIterator cend() const { return ConstIterator(this, m_End()); }

private:
  friend struct Iterator;
  friend struct ConstIterator;

  BeginCallback m_Begin;
  EndCallback m_End;
  NextCallback m_Next;
  ValueCallback m_Value;
};

template <typename V, typename I>
typename nsRangeView<V, I>::Iterator begin(nsRangeView<V, I>& in_container)
{
  return in_container.begin();
}

template <typename V, typename I>
typename nsRangeView<V, I>::ConstIterator begin(const nsRangeView<V, I>& container)
{
  return container.cbegin();
}

template <typename V, typename I>
typename nsRangeView<V, I>::ConstIterator cbegin(const nsRangeView<V, I>& container)
{
  return container.cbegin();
}

template <typename V, typename I>
typename nsRangeView<V, I>::Iterator end(nsRangeView<V, I>& in_container)
{
  return in_container.end();
}

template <typename V, typename I>
typename nsRangeView<V, I>::ConstIterator end(const nsRangeView<V, I>& container)
{
  return container.cend();
}

template <typename V, typename I>
typename nsRangeView<V, I>::ConstIterator cend(const nsRangeView<V, I>& container)
{
  return container.cend();
}

#include <Foundation/Types/Implementation/RangeView_inl.h>
