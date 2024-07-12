
template <typename Type>
nsProcessingStreamIterator<Type>::nsProcessingStreamIterator(const nsProcessingStream* pStream, nsUInt64 uiNumElements, nsUInt64 uiStartIndex)

{
  NS_ASSERT_DEV(pStream != nullptr, "Stream pointer may not be null!");
  NS_ASSERT_DEV(pStream->GetElementSize() == sizeof(Type), "Data size missmatch");

  m_uiElementStride = pStream->GetElementStride();

  m_pCurrentPtr = nsMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<std::ptrdiff_t>(uiStartIndex * m_uiElementStride));
  m_pEndPtr = nsMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<std::ptrdiff_t>((uiStartIndex + uiNumElements) * m_uiElementStride));
}

template <typename Type>
NS_ALWAYS_INLINE Type& nsProcessingStreamIterator<Type>::Current() const
{
  return *static_cast<Type*>(m_pCurrentPtr);
}

template <typename Type>
NS_ALWAYS_INLINE bool nsProcessingStreamIterator<Type>::HasReachedEnd() const
{
  return m_pCurrentPtr >= m_pEndPtr;
}

template <typename Type>
NS_ALWAYS_INLINE void nsProcessingStreamIterator<Type>::Advance()
{
  m_pCurrentPtr = nsMemoryUtils::AddByteOffset(m_pCurrentPtr, static_cast<std::ptrdiff_t>(m_uiElementStride));
}

template <typename Type>
NS_ALWAYS_INLINE void nsProcessingStreamIterator<Type>::Advance(nsUInt32 uiNumElements)
{
  m_pCurrentPtr = nsMemoryUtils::AddByteOffset(m_pCurrentPtr, static_cast<std::ptrdiff_t>(m_uiElementStride * uiNumElements));
}
