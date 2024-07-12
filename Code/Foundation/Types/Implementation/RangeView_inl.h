
template <typename ValueType, typename IteratorType>
nsRangeView<ValueType, IteratorType>::nsRangeView(BeginCallback begin, EndCallback end, NextCallback next, ValueCallback value)
  : m_Begin(begin)
  , m_End(end)
  , m_Next(next)
  , m_Value(value)
{
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE void nsRangeView<ValueType, IteratorType>::ConstIterator::Next()
{
  this->m_pView->m_Next(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE ValueType nsRangeView<ValueType, IteratorType>::ConstIterator::Value() const
{
  return this->m_pView->m_Value(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE bool nsRangeView<ValueType, IteratorType>::ConstIterator::operator==(
  const typename nsRangeView<ValueType, IteratorType>::ConstIterator& it2) const
{
  return m_pView == it2.m_pView && m_Pos == it2.m_Pos;
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE bool nsRangeView<ValueType, IteratorType>::ConstIterator::operator!=(
  const typename nsRangeView<ValueType, IteratorType>::ConstIterator& it2) const
{
  return !(*this == it2);
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE nsRangeView<ValueType, IteratorType>::ConstIterator::ConstIterator(const nsRangeView<ValueType, IteratorType>* view, IteratorType pos)
{
  m_pView = view;
  m_Pos = pos;
}

template <typename ValueType, typename IteratorType>
NS_FORCE_INLINE ValueType nsRangeView<ValueType, IteratorType>::Iterator::Value()
{
  return this->m_View->m_value(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
nsRangeView<ValueType, IteratorType>::Iterator::Iterator(const nsRangeView<ValueType, IteratorType>* view, IteratorType pos)
  : nsRangeView<ValueType, IteratorType>::ConstIterator(view, pos)
{
}
