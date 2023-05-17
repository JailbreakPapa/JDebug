#pragma once

#include <Foundation/Strings/StringUtils.h>

/// \brief STL forward iterator used by all string classes. Iterates over unicode characters.
///  The iterator starts at the first character of the string and ends at the address beyond the last character of the string.
struct wdStringIterator
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = wdUInt32;
  using difference_type = ptrdiff_t;
  using pointer = const char*;
  using reference = wdUInt32;

  WD_DECLARE_POD_TYPE();

  /// \brief Constructs an invalid iterator.
  WD_ALWAYS_INLINE wdStringIterator() = default; // [tested]

  /// \brief Constructs either a begin or end iterator for the given string.
  WD_FORCE_INLINE explicit wdStringIterator(const char* pStartPtr, const char* pEndPtr, const char* pCurPtr)
  {
    m_pStartPtr = pStartPtr;
    m_pEndPtr = pEndPtr;
    m_pCurPtr = pCurPtr;
  }

  /// \brief Checks whether this iterator points to a valid element. Invalid iterators either point to m_pEndPtr or were never initialized.
  WD_ALWAYS_INLINE bool IsValid() const { return m_pCurPtr != nullptr && m_pCurPtr != m_pEndPtr; } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  WD_ALWAYS_INLINE wdUInt32 GetCharacter() const { return IsValid() ? wdUnicodeUtils::ConvertUtf8ToUtf32(m_pCurPtr) : wdUInt32(0); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  WD_ALWAYS_INLINE wdUInt32 operator*() const { return GetCharacter(); } // [tested]

  /// \brief Returns the address the iterator currently points to.
  WD_ALWAYS_INLINE const char* GetData() const { return m_pCurPtr; } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  WD_ALWAYS_INLINE bool operator==(const wdStringIterator& it2) const { return (m_pCurPtr == it2.m_pCurPtr); } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  WD_ALWAYS_INLINE bool operator!=(const wdStringIterator& it2) const { return (m_pCurPtr != it2.m_pCurPtr); } // [tested]

  /// \brief Advances the iterated to the next character, same as operator++, but returns how many bytes were consumed in the source string.
  WD_ALWAYS_INLINE wdUInt32 Advance()
  {
    const char* pPrevElement = m_pCurPtr;

    if (m_pCurPtr < m_pEndPtr)
    {
      wdUnicodeUtils::MoveToNextUtf8(m_pCurPtr);
    }

    return static_cast<wdUInt32>(m_pCurPtr - pPrevElement);
  }

  /// \brief Move to the next Utf8 character
  WD_ALWAYS_INLINE wdStringIterator& operator++() // [tested]
  {
    if (m_pCurPtr < m_pEndPtr)
    {
      wdUnicodeUtils::MoveToNextUtf8(m_pCurPtr);
    }

    return *this;
  }

  /// \brief Move to the previous Utf8 character
  WD_ALWAYS_INLINE wdStringIterator& operator--() // [tested]
  {
    if (m_pStartPtr < m_pCurPtr)
    {
      wdUnicodeUtils::MoveToPriorUtf8(m_pCurPtr);
    }

    return *this;
  }

  /// \brief Move to the next Utf8 character
  WD_ALWAYS_INLINE wdStringIterator operator++(int) // [tested]
  {
    wdStringIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// \brief Move to the previous Utf8 character
  WD_ALWAYS_INLINE wdStringIterator operator--(int) // [tested]
  {
    wdStringIterator tmp = *this;
    --(*this);
    return tmp;
  }

  /// \brief Advances the iterator forwards by d characters. Does not move it beyond the range's end.
  WD_FORCE_INLINE void operator+=(difference_type d) // [tested]
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
  WD_FORCE_INLINE void operator-=(difference_type d) // [tested]
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
  WD_ALWAYS_INLINE wdStringIterator operator+(difference_type d) const // [tested]
  {
    wdStringIterator it = *this;
    it += d;
    return it;
  }

  /// \brief Returns an iterator that is advanced backwards by d characters.
  WD_ALWAYS_INLINE wdStringIterator operator-(difference_type d) const // [tested]
  {
    wdStringIterator it = *this;
    it -= d;
    return it;
  }

  /// \brief Allows to set the 'current' iteration position to a different value.
  ///
  /// Must be between the iterators start and end range.
  void SetCurrentPosition(const char* szCurPos)
  {
    WD_ASSERT_DEV((szCurPos >= m_pStartPtr) && (szCurPos <= m_pEndPtr), "New position must still be inside the iterator's range.");

    m_pCurPtr = szCurPos;
  }

private:
  const char* m_pStartPtr = nullptr;
  const char* m_pEndPtr = nullptr;
  const char* m_pCurPtr = nullptr;
};


/// \brief STL reverse iterator used by all string classes. Iterates over unicode characters.
///  The iterator starts at the last character of the string and ends at the address before the first character of the string.
struct wdStringReverseIterator
{
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = wdUInt32;
  using difference_type = ptrdiff_t;
  using pointer = const char*;
  using reference = wdUInt32;

  WD_DECLARE_POD_TYPE();

  /// \brief Constructs an invalid iterator.
  WD_ALWAYS_INLINE wdStringReverseIterator() = default; // [tested]

  /// \brief Constructs either a rbegin or rend iterator for the given string.
  WD_FORCE_INLINE explicit wdStringReverseIterator(const char* pStartPtr, const char* pEndPtr, const char* pCurPtr) // [tested]
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
      wdUnicodeUtils::MoveToPriorUtf8(m_pCurPtr);
    }
  }

  /// \brief Checks whether this iterator points to a valid element.
  WD_ALWAYS_INLINE bool IsValid() const { return (m_pCurPtr != nullptr); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  WD_ALWAYS_INLINE wdUInt32 GetCharacter() const { return IsValid() ? wdUnicodeUtils::ConvertUtf8ToUtf32(m_pCurPtr) : wdUInt32(0); } // [tested]

  /// \brief Returns the currently pointed to character in Utf32 encoding.
  WD_ALWAYS_INLINE wdUInt32 operator*() const { return GetCharacter(); } // [tested]

  /// \brief Returns the address the iterator currently points to.
  WD_ALWAYS_INLINE const char* GetData() const { return m_pCurPtr; } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  WD_ALWAYS_INLINE bool operator==(const wdStringReverseIterator& it2) const { return (m_pCurPtr == it2.m_pCurPtr); } // [tested]

  /// \brief Checks whether the two iterators point to the same element.
  WD_ALWAYS_INLINE bool operator!=(const wdStringReverseIterator& it2) const { return (m_pCurPtr != it2.m_pCurPtr); } // [tested]

  /// \brief Move to the next Utf8 character
  WD_FORCE_INLINE wdStringReverseIterator& operator++() // [tested]
  {
    if (m_pCurPtr != nullptr && m_pStartPtr < m_pCurPtr)
      wdUnicodeUtils::MoveToPriorUtf8(m_pCurPtr);
    else
      m_pCurPtr = nullptr;

    return *this;
  }

  /// \brief Move to the previous Utf8 character
  WD_FORCE_INLINE wdStringReverseIterator& operator--() // [tested]
  {
    if (m_pCurPtr != nullptr)
    {
      const char* szOldPos = m_pCurPtr;
      wdUnicodeUtils::MoveToNextUtf8(m_pCurPtr);

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
  WD_ALWAYS_INLINE wdStringReverseIterator operator++(int) // [tested]
  {
    wdStringReverseIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  /// \brief Move to the previous Utf8 character
  WD_ALWAYS_INLINE wdStringReverseIterator operator--(int) // [tested]
  {
    wdStringReverseIterator tmp = *this;
    --(*this);
    return tmp;
  }

  /// \brief Advances the iterator forwards by d characters. Does not move it beyond the range's end.
  WD_FORCE_INLINE void operator+=(difference_type d) // [tested]
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
  WD_FORCE_INLINE void operator-=(difference_type d) // [tested]
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
  WD_ALWAYS_INLINE wdStringReverseIterator operator+(difference_type d) const // [tested]
  {
    wdStringReverseIterator it = *this;
    it += d;
    return it;
  }

  /// \brief Returns an iterator that is advanced backwards by d characters.
  WD_ALWAYS_INLINE wdStringReverseIterator operator-(difference_type d) const // [tested]
  {
    wdStringReverseIterator it = *this;
    it -= d;
    return it;
  }

  /// \brief Allows to set the 'current' iteration position to a different value.
  ///
  /// Must be between the iterators start and end range.
  WD_FORCE_INLINE void SetCurrentPosition(const char* szCurPos)
  {
    WD_ASSERT_DEV((szCurPos == nullptr) || ((szCurPos >= m_pStartPtr) && (szCurPos < m_pEndPtr)), "New position must still be inside the iterator's range.");

    m_pCurPtr = szCurPos;
  }

private:
  const char* m_pStartPtr = nullptr;
  const char* m_pEndPtr = nullptr;
  const char* m_pCurPtr = nullptr;
};
