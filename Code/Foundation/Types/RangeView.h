#pragma once

#include <Foundation/Types/Delegate.h>

/// \brief This class uses delegates to define a range of values that can be enumerated using a forward iterator.
///
/// Can be used to create a contiguous view to elements of a certain type without the need for them to actually
/// exist in the same space or format. Think of IEnumerable in c# using composition via wdDelegate instead of derivation.
/// ValueType defines the value type we are iterating over and IteratorType is the internal key to identify an element.
/// An example that creates a RangeView of strings that are stored in a linear array of structs.
/// \code{.cpp}
/// auto range = wdRangeView<const char*, wdUInt32>(
///   [this]()-> wdUInt32 { return 0; },
///   [this]()-> wdUInt32 { return array.GetCount(); },
///   [this](wdUInt32& it) { ++it; },
///   [this](const wdUInt32& it)-> const char* { return array[it].m_String; });
///
/// for (const char* szValue : range)
/// {
/// }
/// \endcode
template <typename ValueType, typename IteratorType>
class wdRangeView
{
public:
  typedef wdDelegate<IteratorType()> BeginCallback;
  typedef wdDelegate<IteratorType()> EndCallback;
  typedef wdDelegate<void(IteratorType&)> NextCallback;
  typedef wdDelegate<ValueType(const IteratorType&)> ValueCallback;

  /// \brief Initializes the wdRangeView with the delegates used to enumerate the range.
  WD_ALWAYS_INLINE wdRangeView(BeginCallback begin, EndCallback end, NextCallback next, ValueCallback value);

  /// \brief Const iterator, don't use directly, use ranged based for loops or call begin() end().
  struct ConstIterator
  {
    WD_DECLARE_POD_TYPE();

    typedef std::forward_iterator_tag iterator_category;
    typedef ConstIterator value_type;
    typedef ConstIterator* pointer;
    typedef ConstIterator& reference;

    WD_ALWAYS_INLINE ConstIterator(const ConstIterator& rhs) = default;
    WD_FORCE_INLINE void Next();
    WD_FORCE_INLINE ValueType Value() const;
    WD_ALWAYS_INLINE ValueType operator*() const { return Value(); }
    WD_ALWAYS_INLINE void operator++() { Next(); }
    WD_FORCE_INLINE bool operator==(const typename wdRangeView<ValueType, IteratorType>::ConstIterator& it2) const;
    WD_FORCE_INLINE bool operator!=(const typename wdRangeView<ValueType, IteratorType>::ConstIterator& it2) const;

  protected:
    WD_FORCE_INLINE explicit ConstIterator(const wdRangeView<ValueType, IteratorType>* view, IteratorType pos);

    friend class wdRangeView<ValueType, IteratorType>;
    const wdRangeView<ValueType, IteratorType>* m_pView = nullptr;
    IteratorType m_Pos;
  };

  /// \brief Iterator, don't use directly, use ranged based for loops or call begin() end().
  struct Iterator : public ConstIterator
  {
    WD_DECLARE_POD_TYPE();

    typedef std::forward_iterator_tag iterator_category;
    typedef Iterator value_type;
    typedef Iterator* pointer;
    typedef Iterator& reference;

    using ConstIterator::Value;
    WD_ALWAYS_INLINE Iterator(const Iterator& rhs) = default;
    WD_FORCE_INLINE ValueType Value();
    WD_ALWAYS_INLINE ValueType operator*() { return Value(); }

  protected:
    WD_FORCE_INLINE explicit Iterator(const wdRangeView<ValueType, IteratorType>* view, IteratorType pos);
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
typename wdRangeView<V, I>::Iterator begin(wdRangeView<V, I>& in_container)
{
  return in_container.begin();
}

template <typename V, typename I>
typename wdRangeView<V, I>::ConstIterator begin(const wdRangeView<V, I>& container)
{
  return container.cbegin();
}

template <typename V, typename I>
typename wdRangeView<V, I>::ConstIterator cbegin(const wdRangeView<V, I>& container)
{
  return container.cbegin();
}

template <typename V, typename I>
typename wdRangeView<V, I>::Iterator end(wdRangeView<V, I>& in_container)
{
  return in_container.end();
}

template <typename V, typename I>
typename wdRangeView<V, I>::ConstIterator end(const wdRangeView<V, I>& container)
{
  return container.cend();
}

template <typename V, typename I>
typename wdRangeView<V, I>::ConstIterator cend(const wdRangeView<V, I>& container)
{
  return container.cend();
}

#include <Foundation/Types/Implementation/RangeView_inl.h>
