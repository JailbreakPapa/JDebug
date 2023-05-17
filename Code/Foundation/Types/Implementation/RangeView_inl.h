
template <typename ValueType, typename IteratorType>
wdRangeView<ValueType, IteratorType>::wdRangeView(BeginCallback begin, EndCallback end, NextCallback next, ValueCallback value)
  : m_Begin(begin)
  , m_End(end)
  , m_Next(next)
  , m_Value(value)
{
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE void wdRangeView<ValueType, IteratorType>::ConstIterator::Next()
{
  this->m_pView->m_Next(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE ValueType wdRangeView<ValueType, IteratorType>::ConstIterator::Value() const
{
  return this->m_pView->m_Value(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE bool wdRangeView<ValueType, IteratorType>::ConstIterator::operator==(
  const typename wdRangeView<ValueType, IteratorType>::ConstIterator& it2) const
{
  return m_pView == it2.m_pView && m_Pos == it2.m_Pos;
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE bool wdRangeView<ValueType, IteratorType>::ConstIterator::operator!=(
  const typename wdRangeView<ValueType, IteratorType>::ConstIterator& it2) const
{
  return !(*this == it2);
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE wdRangeView<ValueType, IteratorType>::ConstIterator::ConstIterator(const wdRangeView<ValueType, IteratorType>* view, IteratorType pos)
{
  m_pView = view;
  m_Pos = pos;
}

template <typename ValueType, typename IteratorType>
WD_FORCE_INLINE ValueType wdRangeView<ValueType, IteratorType>::Iterator::Value()
{
  return this->m_View->m_value(this->m_Pos);
}

template <typename ValueType, typename IteratorType>
wdRangeView<ValueType, IteratorType>::Iterator::Iterator(const wdRangeView<ValueType, IteratorType>* view, IteratorType pos)
  : wdRangeView<ValueType, IteratorType>::ConstIterator(view, pos)
{
}
