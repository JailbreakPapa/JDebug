
template <typename Type>
wdProcessingStreamIterator<Type>::wdProcessingStreamIterator(const wdProcessingStream* pStream, wdUInt64 uiNumElements, wdUInt64 uiStartIndex)
  : m_pCurrentPtr(nullptr)
  , m_pEndPtr(nullptr)
  , m_uiElementStride(0)
{
  WD_ASSERT_DEV(pStream != nullptr, "Stream pointer may not be null!");
  WD_ASSERT_DEV(pStream->GetElementSize() == sizeof(Type), "Data size missmatch");

  m_uiElementStride = pStream->GetElementStride();

  m_pCurrentPtr = wdMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<ptrdiff_t>(uiStartIndex * m_uiElementStride));
  m_pEndPtr = wdMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<ptrdiff_t>((uiStartIndex + uiNumElements) * m_uiElementStride));
}

template <typename Type>
WD_ALWAYS_INLINE Type& wdProcessingStreamIterator<Type>::Current() const
{
  return *static_cast<Type*>(m_pCurrentPtr);
}

template <typename Type>
WD_ALWAYS_INLINE bool wdProcessingStreamIterator<Type>::HasReachedEnd() const
{
  return m_pCurrentPtr >= m_pEndPtr;
}

template <typename Type>
WD_ALWAYS_INLINE void wdProcessingStreamIterator<Type>::Advance()
{
  m_pCurrentPtr = wdMemoryUtils::AddByteOffset(m_pCurrentPtr, static_cast<ptrdiff_t>(m_uiElementStride));
}

template <typename Type>
WD_ALWAYS_INLINE void wdProcessingStreamIterator<Type>::Advance(wdUInt32 uiNumElements)
{
  m_pCurrentPtr = wdMemoryUtils::AddByteOffset(m_pCurrentPtr, static_cast<ptrdiff_t>(m_uiElementStride * uiNumElements));
}
