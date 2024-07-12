#pragma once

#include <Foundation/Strings/StringUtils.h>

/// \brief STL forward iterator used by all string classes. Iterates over unicode characters.
///  The iterator starts at the first character of the string and ends at the address beyond the last character of the string.
struct nsStringIterator
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = nsUInt32;
  using difference_type = std::ptrdiff_t;
  using pointer = const char*;
  using reference = nsUInt32;

  NS_DECLARE_POD_TYPE();

  /// \brief Constructs an invalid iterator.
  NS_ALWAYS_INLINE nsStringIterator() = default; // [tested]

  /// \brief Constructs either a begin or end iterator for the given string.
  NS_FORCE_INLINE explicit nsStringIterator(const char* pStartPtr, const char* pEndPtr, const char* pCurPtr)
  {
    m_pStartPtr = pStartPtr;
    m_pEndPtr = pEndPtr;
    m_pCurPtr = pCurPtr;
  }

  /// \brief Checks whether this iterator points to a valid element. Invalid iterators either point to m_pEndPtr or were never initialized.
  NS_ALWAYS_INLINE bool IsValid() const { return m_pCurPtr != nullptr && m_pCurPtr != m_pEndPtr; } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  NS_ALWAYS_INLINE nsUInt32 GetCharacter() const { return IsValid() ? nsUnicodeUtils::ConvertUtf8ToUtf32(m_pCurPtr) : nsUInt32(0); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  NS_ALWAYS_INLINE nsUInt32 operator*() const { return GetCharacter(); } // [tested]

  /// \brief Returns the address the iterator currently points to.
  NS_ALWAYS_INLINE const char* GetData() const { return m_pCurPtr; } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  NS_ALWAYS_INLINE bool operator==(const nsStringIterator& it2) const { return (m_pCurPtr == it2.m_pCurPtr); } // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsStringIterator&);

  /// \brief Advances the iterated to the next character, same as operator++, but returns how many bytes were consumed in the source string.
  NS_ALWAYS_INLINE nsUInt32 Advance()
  {
    const char* pPrevElement = m_pCurPtr;

    if (m_pCurPtr < m_pEndPtr)
    {
      nsUnicodeUtils::MoveToNextUtf8(m_pCurPtr).AssertSuccess();
    }

    return static_cast<nsUInt32>(m_pCurPtr - pPrevElement);
  }

  /// \brief Move to the next Utf8 character
  NS_ALWAYS_INLINE nsStringIterator& operator++() // [tested]
  {
    if (m_pCurPtr < m_pEndPtr)
    {
      nsUnicodeUtils::MoveToNextUtf8(m_pCurPtr).AssertSuccess();
    }

    return *this;
  }

  /// \brief Move to the previous Utf8 character
  NS_ALWAYS_INLINE nsStringIterator& operator--() // [tested]
  {
    if (m_pStartPtr < m_pCurPtr)
    {
      nsUnicodeUtils::MoveToPriorUtf8(m_pCurPtr, m_pStartPtr).AssertSuccess();
    }

    return *this;
  }

  /// \brief Move to the next Utf8 character
  NS_ALWAYS_INLINE nsStringIterator operator++(int) // [tested]
  {
    nsStringIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// \brief Move to the previous Utf8 character
  NS_ALWAYS_INLINE nsStringIterator operator--(int) // [tested]
  {
    nsStringIterator tmp = *this;
    --(*this);
    return tmp;
  }

  /// \brief Advances the iterator forwards by d characters. Does not move it beyond the range's end.
  NS_FORCE_INLINE void operator+=(difference_type d) // [tested]
  {
    while (d > 0)
    {
      ++(*this);
      --d;
    }
    while (d < 0)
    {
      --(*this);
      ++d;
    }
  }

  /// \brief Moves the iterator backwards by d characters. Does not move it beyond the range's start.
  NS_FORCE_INLINE void operator-=(difference_type d) // [tested]
  {
    while (d > 0)
    {
      --(*this);
      --d;
    }
    while (d < 0)
    {
      ++(*this);
      ++d;
    }
  }

  /// \brief Returns an iterator that is advanced forwards by d characters.
  NS_ALWAYS_INLINE nsStringIterator operator+(difference_type d) const // [tested]
  {
    nsStringIterator it = *this;
    it += d;
    return it;
  }

  /// \brief Returns an iterator that is advanced backwards by d characters.
  NS_ALWAYS_INLINE nsStringIterator operator-(difference_type d) const // [tested]
  {
    nsStringIterator it = *this;
    it -= d;
    return it;
  }

  /// \brief Allows to set the 'current' iteration position to a different value.
  ///
  /// Must be between the iterators start and end range.
  void SetCurrentPosition(const char* szCurPos)
  {
    NS_ASSERT_DEV((szCurPos >= m_pStartPtr) && (szCurPos <= m_pEndPtr), "New position must still be inside the iterator's range.");

    m_pCurPtr = szCurPos;
  }

private:
  const char* m_pStartPtr = nullptr;
  const char* m_pEndPtr = nullptr;
  const char* m_pCurPtr = nullptr;
};


/// \brief STL reverse iterator used by all string classes. Iterates over unicode characters.
///  The iterator starts at the last character of the string and ends at the address before the first character of the string.
struct nsStringReverseIterator
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = nsUInt32;
  using difference_type = std::ptrdiff_t;
  using pointer = const char*;
  using reference = nsUInt32;

  NS_DECLARE_POD_TYPE();

  /// \brief Constructs an invalid iterator.
  NS_ALWAYS_INLINE nsStringReverseIterator() = default; // [tested]

  /// \brief Constructs either a rbegin or rend iterator for the given string.
  NS_FORCE_INLINE explicit nsStringReverseIterator(const char* pStartPtr, const char* pEndPtr, const char* pCurPtr) // [tested]
  {
    m_pStartPtr = pStartPtr;
    m_pEndPtr = pEndPtr;
    m_pCurPtr = pCurPtr;

    if (m_pStartPtr >= m_pEndPtr)
    {
      m_pCurPtr = nullptr;
    }
    else if (m_pCurPtr == m_pEndPtr)
    {
      nsUnicodeUtils::MoveToPriorUtf8(m_pCurPtr, m_pStartPtr).AssertSuccess();
    }
  }

  /// \brief Checks whether this iterator points to a valid element.
  NS_ALWAYS_INLINE bool IsValid() const { return (m_pCurPtr != nullptr); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  NS_ALWAYS_INLINE nsUInt32 GetCharacter() const { return IsValid() ? nsUnicodeUtils::ConvertUtf8ToUtf32(m_pCurPtr) : nsUInt32(0); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  NS_ALWAYS_INLINE nsUInt32 operator*() const { return GetCharacter(); } // [tested]

  /// \brief Returns the address the iterator currently points to.
  NS_ALWAYS_INLINE const char* GetData() const { return m_pCurPtr; } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  NS_ALWAYS_INLINE bool operator==(const nsStringReverseIterator& it2) const { return (m_pCurPtr == it2.m_pCurPtr); } // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsStringReverseIterator&);

  /// \brief Move to the next Utf8 character
  NS_FORCE_INLINE nsStringReverseIterator& operator++() // [tested]
  {
    if (m_pCurPtr != nullptr && m_pStartPtr < m_pCurPtr)
      nsUnicodeUtils::MoveToPriorUtf8(m_pCurPtr, m_pStartPtr).AssertSuccess();
    else
      m_pCurPtr = nullptr;

    return *this;
  }

  /// \brief Move to the previous Utf8 character
  NS_FORCE_INLINE nsStringReverseIterator& operator--() // [tested]
  {
    if (m_pCurPtr != nullptr)
    {
      const char* szOldPos = m_pCurPtr;
      nsUnicodeUtils::MoveToNextUtf8(m_pCurPtr).AssertSuccess();

      if (m_pCurPtr == m_pEndPtr)
        m_pCurPtr = szOldPos;
    }
    else
    {
      // Set back to the first character.
      m_pCurPtr = m_pStartPtr;
    }
    return *this;
  }

  /// \brief Move to the next Utf8 character
  NS_ALWAYS_INLINE nsStringReverseIterator operator++(int) // [tested]
  {
    nsStringReverseIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// \brief Move to the previous Utf8 character
  NS_ALWAYS_INLINE nsStringReverseIterator operator--(int) // [tested]
  {
    nsStringReverseIterator tmp = *this;
    --(*this);
    return tmp;
  }

  /// \brief Advances the iterator forwards by d characters. Does not move it beyond the range's end.
  NS_FORCE_INLINE void operator+=(difference_type d) // [tested]
  {
    while (d > 0)
    {
      ++(*this);
      --d;
    }
    while (d < 0)
    {
      --(*this);
      ++d;
    }
  }

  /// \brief Moves the iterator backwards by d characters. Does not move it beyond the range's start.
  NS_FORCE_INLINE void operator-=(difference_type d) // [tested]
  {
    while (d > 0)
    {
      --(*this);
      --d;
    }
    while (d < 0)
    {
      ++(*this);
      ++d;
    }
  }

  /// \brief Returns an iterator that is advanced forwards by d characters.
  NS_ALWAYS_INLINE nsStringReverseIterator operator+(difference_type d) const // [tested]
  {
    nsStringReverseIterator it = *this;
    it += d;
    return it;
  }

  /// \brief Returns an iterator that is advanced backwards by d characters.
  NS_ALWAYS_INLINE nsStringReverseIterator operator-(difference_type d) const // [tested]
  {
    nsStringReverseIterator it = *this;
    it -= d;
    return it;
  }

  /// \brief Allows to set the 'current' iteration position to a different value.
  ///
  /// Must be between the iterators start and end range.
  NS_FORCE_INLINE void SetCurrentPosition(const char* szCurPos)
  {
    NS_ASSERT_DEV((szCurPos == nullptr) || ((szCurPos >= m_pStartPtr) && (szCurPos < m_pEndPtr)), "New position must still be inside the iterator's range.");

    m_pCurPtr = szCurPos;
  }

private:
  const char* m_pStartPtr = nullptr;
  const char* m_pEndPtr = nullptr;
  const char* m_pCurPtr = nullptr;
};
