#pragma once

NS_ALWAYS_INLINE constexpr nsStringView::nsStringView() = default;

NS_ALWAYS_INLINE nsStringView::nsStringView(char* pStart)
  : m_pStart(pStart)
  , m_uiElementCount(nsStringUtils::GetStringElementCount(pStart))
{
}

template <typename T>
constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(T pStart, typename std::enable_if<std::is_same<T, const char*>::value, int>::type*)
  : m_pStart(pStart)
  , m_uiElementCount(nsStringUtils::GetStringElementCount(pStart))
{
}

template <typename T>
constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(const T&& str, typename std::enable_if<std::is_same<T, const char*>::value == false && std::is_convertible<T, const char*>::value, int>::type*)
{
  m_pStart = str;
  m_uiElementCount = nsStringUtils::GetStringElementCount(m_pStart);
}

constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(const char* pStart, const char* pEnd)
{
  NS_ASSERT_DEBUG(pStart <= pEnd, "Invalid pointers to construct a string view from.");

  m_pStart = pStart;
  m_uiElementCount = static_cast<nsUInt32>(pEnd - pStart);
}

constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(const char* pStart, nsUInt32 uiLength)
  : m_pStart(pStart)
  , m_uiElementCount(uiLength)
{
}

template <size_t N>
constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(const char (&str)[N])
  : m_pStart(str)
  , m_uiElementCount(N - 1)
{
  static_assert(N > 0, "Not a string literal");
}

template <size_t N>
constexpr NS_ALWAYS_INLINE nsStringView::nsStringView(char (&str)[N])
{
  m_pStart = str;
  m_uiElementCount = nsStringUtils::GetStringElementCount(str, str + N);
}

inline void nsStringView::operator++()
{
  if (!IsValid())
    return;

  const char* pEnd = m_pStart + m_uiElementCount;
  nsUnicodeUtils::MoveToNextUtf8(m_pStart, pEnd).IgnoreResult(); // if it fails, the string is just empty
  m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
}

inline void nsStringView::operator+=(nsUInt32 d)
{
  const char* pEnd = m_pStart + m_uiElementCount;
  nsUnicodeUtils::MoveToNextUtf8(m_pStart, pEnd, d).IgnoreResult(); // if it fails, the string is just empty
  m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
}

NS_ALWAYS_INLINE bool nsStringView::IsValid() const
{
  return (m_pStart != nullptr) && (m_uiElementCount > 0);
}

NS_ALWAYS_INLINE void nsStringView::SetStartPosition(const char* szCurPos)
{
  NS_ASSERT_DEV((szCurPos >= m_pStart) && (szCurPos <= m_pStart + m_uiElementCount), "New start position must still be inside the view's range.");

  const char* pEnd = m_pStart + m_uiElementCount;
  m_pStart = szCurPos;
  m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
}

NS_ALWAYS_INLINE bool nsStringView::IsEmpty() const
{
  return m_uiElementCount == 0;
}

NS_ALWAYS_INLINE bool nsStringView::IsEqual(nsStringView sOther) const
{
  return nsStringUtils::IsEqual(m_pStart, sOther.GetStartPointer(), m_pStart + m_uiElementCount, sOther.GetEndPointer());
}

NS_ALWAYS_INLINE bool nsStringView::IsEqual_NoCase(nsStringView sOther) const
{
  return nsStringUtils::IsEqual_NoCase(m_pStart, sOther.GetStartPointer(), m_pStart + m_uiElementCount, sOther.GetEndPointer());
}

NS_ALWAYS_INLINE bool nsStringView::StartsWith(nsStringView sStartsWith) const
{
  return nsStringUtils::StartsWith(m_pStart, sStartsWith.GetStartPointer(), m_pStart + m_uiElementCount, sStartsWith.GetEndPointer());
}

NS_ALWAYS_INLINE bool nsStringView::StartsWith_NoCase(nsStringView sStartsWith) const
{
  return nsStringUtils::StartsWith_NoCase(m_pStart, sStartsWith.GetStartPointer(), m_pStart + m_uiElementCount, sStartsWith.GetEndPointer());
}

NS_ALWAYS_INLINE bool nsStringView::EndsWith(nsStringView sEndsWith) const
{
  return nsStringUtils::EndsWith(m_pStart, sEndsWith.GetStartPointer(), m_pStart + m_uiElementCount, sEndsWith.GetEndPointer());
}

NS_ALWAYS_INLINE bool nsStringView::EndsWith_NoCase(nsStringView sEndsWith) const
{
  return nsStringUtils::EndsWith_NoCase(m_pStart, sEndsWith.GetStartPointer(), m_pStart + m_uiElementCount, sEndsWith.GetEndPointer());
}

NS_ALWAYS_INLINE void nsStringView::Trim(const char* szTrimChars)
{
  return Trim(szTrimChars, szTrimChars);
}

NS_ALWAYS_INLINE void nsStringView::Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd)
{
  if (IsValid())
  {
    const char* pEnd = m_pStart + m_uiElementCount;
    nsStringUtils::Trim(m_pStart, pEnd, szTrimCharsStart, szTrimCharsEnd);
    m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
  }
}

constexpr NS_ALWAYS_INLINE nsStringView operator"" _nssv(const char* pString, size_t uiLen)
{
  return nsStringView(pString, static_cast<nsUInt32>(uiLen));
}

template <typename Container>
void nsStringView::Split(bool bReturnEmptyStrings, Container& ref_output, const char* szSeparator1, const char* szSeparator2 /*= nullptr*/, const char* szSeparator3 /*= nullptr*/, const char* szSeparator4 /*= nullptr*/, const char* szSeparator5 /*= nullptr*/, const char* szSeparator6 /*= nullptr*/) const
{
  ref_output.Clear();

  if (IsEmpty())
    return;

  const nsUInt32 uiParams = 6;

  const nsStringView seps[uiParams] = {szSeparator1, szSeparator2, szSeparator3, szSeparator4, szSeparator5, szSeparator6};

  const char* szReadPos = GetStartPointer();

  while (true)
  {
    const char* szFoundPos = nsUnicodeUtils::GetMaxStringEnd<char>();
    nsUInt32 uiFoundSeparator = 0;

    for (nsUInt32 i = 0; i < uiParams; ++i)
    {
      const char* szFound = nsStringUtils::FindSubString(szReadPos, seps[i].GetStartPointer(), GetEndPointer(), seps[i].GetEndPointer());

      if ((szFound != nullptr) && (szFound < szFoundPos))
      {
        szFoundPos = szFound;
        uiFoundSeparator = i;
      }
    }

    // nothing found
    if (szFoundPos == nsUnicodeUtils::GetMaxStringEnd<char>())
    {
      const nsUInt32 uiLen = nsStringUtils::GetStringElementCount(szReadPos, GetEndPointer());

      if (bReturnEmptyStrings || (uiLen > 0))
        ref_output.PushBack(nsStringView(szReadPos, szReadPos + uiLen));

      return;
    }

    if (bReturnEmptyStrings || (szFoundPos > szReadPos))
      ref_output.PushBack(nsStringView(szReadPos, szFoundPos));

    szReadPos = szFoundPos + seps[uiFoundSeparator].GetElementCount();
  }
}

NS_ALWAYS_INLINE bool operator==(nsStringView lhs, nsStringView rhs)
{
  return lhs.IsEqual(rhs);
}

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)

NS_ALWAYS_INLINE bool operator!=(nsStringView lhs, nsStringView rhs)
{
  return !lhs.IsEqual(rhs);
}

#endif

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)

NS_ALWAYS_INLINE std::strong_ordering operator<=>(nsStringView lhs, nsStringView rhs)
{
  return lhs.Compare(rhs) <=> 0;
}

#else

NS_ALWAYS_INLINE bool operator<(nsStringView lhs, nsStringView rhs)
{
  return lhs.Compare(rhs) < 0;
}

NS_ALWAYS_INLINE bool operator<=(nsStringView lhs, nsStringView rhs)
{
  return lhs.Compare(rhs) <= 0;
}

NS_ALWAYS_INLINE bool operator>(nsStringView lhs, nsStringView rhs)
{
  return lhs.Compare(rhs) > 0;
}

NS_ALWAYS_INLINE bool operator>=(nsStringView lhs, nsStringView rhs)
{
  return lhs.Compare(rhs) >= 0;
}

#endif
