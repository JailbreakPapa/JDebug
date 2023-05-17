#pragma once

WD_ALWAYS_INLINE constexpr wdStringView::wdStringView() = default;

WD_ALWAYS_INLINE wdStringView::wdStringView(char* pStart)
  : m_pStart(pStart)
  , m_pEnd(pStart + wdStringUtils::GetStringElementCount(pStart))
{
}

template <typename T>
constexpr WD_ALWAYS_INLINE wdStringView::wdStringView(T pStart, typename std::enable_if<std::is_same<T, const char*>::value, int>::type*)
  : m_pStart(pStart)
  , m_pEnd(pStart + wdStringUtils::GetStringElementCount(pStart))
{
}

template <typename T>
WD_ALWAYS_INLINE wdStringView::wdStringView(const T&& str, typename std::enable_if<std::is_same<T, const char*>::value == false && std::is_convertible<T, const char*>::value, int>::type*)
{
  m_pStart = str;
  m_pEnd = m_pStart + wdStringUtils::GetStringElementCount(m_pStart);
}

WD_ALWAYS_INLINE wdStringView::wdStringView(const char* pStart, const char* pEnd)
{
  WD_ASSERT_DEV(pStart <= pEnd, "It should start BEFORE it ends.");

  m_pStart = pStart;
  m_pEnd = pEnd;
}

constexpr WD_ALWAYS_INLINE wdStringView::wdStringView(const char* pStart, wdUInt32 uiLength)
  : m_pStart(pStart)
  , m_pEnd(pStart + uiLength)
{
}

template <size_t N>
WD_ALWAYS_INLINE wdStringView::wdStringView(const char (&str)[N])
  : m_pStart(str)
  , m_pEnd(str + N - 1)
{
  static_assert(N > 0, "Not a string literal");
  WD_ASSERT_DEBUG(str[N - 1] == '\0', "Not a string literal. Manually cast to 'const char*' if you are trying to pass a const char fixed size array.");
}

template <size_t N>
WD_ALWAYS_INLINE wdStringView::wdStringView(char (&str)[N])
{
  m_pStart = str;
  m_pEnd = m_pStart + wdStringUtils::GetStringElementCount(str, str + N);
}

inline void wdStringView::operator++()
{
  if (!IsValid())
    return;

  wdUnicodeUtils::MoveToNextUtf8(m_pStart, m_pEnd);
}

inline void wdStringView::operator+=(wdUInt32 d)
{
  wdUnicodeUtils::MoveToNextUtf8(m_pStart, m_pEnd, d);
}
WD_ALWAYS_INLINE bool wdStringView::IsValid() const
{
  return (m_pStart != nullptr) && (m_pStart < m_pEnd);
}

WD_ALWAYS_INLINE void wdStringView::SetStartPosition(const char* szCurPos)
{
  WD_ASSERT_DEV((szCurPos >= m_pStart) && (szCurPos <= m_pEnd), "New start position must still be inside the view's range.");

  m_pStart = szCurPos;
}

WD_ALWAYS_INLINE bool wdStringView::IsEmpty() const
{
  return m_pStart == m_pEnd || wdStringUtils::IsNullOrEmpty(m_pStart);
}

WD_ALWAYS_INLINE bool wdStringView::IsEqual(wdStringView sOther) const
{
  return wdStringUtils::IsEqual(m_pStart, sOther.GetStartPointer(), m_pEnd, sOther.GetEndPointer());
}

WD_ALWAYS_INLINE bool wdStringView::IsEqual_NoCase(wdStringView sOther) const
{
  return wdStringUtils::IsEqual_NoCase(m_pStart, sOther.GetStartPointer(), m_pEnd, sOther.GetEndPointer());
}

WD_ALWAYS_INLINE bool wdStringView::StartsWith(wdStringView sStartsWith) const
{
  return wdStringUtils::StartsWith(m_pStart, sStartsWith.GetStartPointer(), m_pEnd, sStartsWith.GetEndPointer());
}

WD_ALWAYS_INLINE bool wdStringView::StartsWith_NoCase(wdStringView sStartsWith) const
{
  return wdStringUtils::StartsWith_NoCase(m_pStart, sStartsWith.GetStartPointer(), m_pEnd, sStartsWith.GetEndPointer());
}

WD_ALWAYS_INLINE bool wdStringView::EndsWith(wdStringView sEndsWith) const
{
  return wdStringUtils::EndsWith(m_pStart, sEndsWith.GetStartPointer(), m_pEnd, sEndsWith.GetEndPointer());
}

WD_ALWAYS_INLINE bool wdStringView::EndsWith_NoCase(wdStringView sEndsWith) const
{
  return wdStringUtils::EndsWith_NoCase(m_pStart, sEndsWith.GetStartPointer(), m_pEnd, sEndsWith.GetEndPointer());
}

WD_ALWAYS_INLINE void wdStringView::Trim(const char* szTrimChars)
{
  return Trim(szTrimChars, szTrimChars);
}

WD_ALWAYS_INLINE void wdStringView::Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd)
{
  if (IsValid())
  {
    wdStringUtils::Trim(m_pStart, m_pEnd, szTrimCharsStart, szTrimCharsEnd);
  }
}

constexpr WD_ALWAYS_INLINE wdStringView operator"" _wdsv(const char* pString, size_t uiLen)
{
  return wdStringView(pString, static_cast<wdUInt32>(uiLen));
}

template <typename Container>
void wdStringView::Split(bool bReturnEmptyStrings, Container& ref_output, const char* szSeparator1, const char* szSeparator2 /*= nullptr*/, const char* szSeparator3 /*= nullptr*/, const char* szSeparator4 /*= nullptr*/, const char* szSeparator5 /*= nullptr*/, const char* szSeparator6 /*= nullptr*/) const
{
  ref_output.Clear();

  if (IsEmpty())
    return;

  const wdUInt32 uiParams = 6;

  const wdStringView seps[uiParams] = {szSeparator1, szSeparator2, szSeparator3, szSeparator4, szSeparator5, szSeparator6};

  const char* szReadPos = GetStartPointer();

  while (true)
  {
    const char* szFoundPos = wdUnicodeUtils::GetMaxStringEnd<char>();
    wdInt32 iFoundSeparator = 0;

    for (wdInt32 i = 0; i < uiParams; ++i)
    {
      const char* szFound = wdStringUtils::FindSubString(szReadPos, seps[i].GetStartPointer(), GetEndPointer(), seps[i].GetEndPointer());

      if ((szFound != nullptr) && (szFound < szFoundPos))
      {
        szFoundPos = szFound;
        iFoundSeparator = i;
      }
    }

    // nothing found
    if (szFoundPos == wdUnicodeUtils::GetMaxStringEnd<char>())
    {
      const wdUInt32 uiLen = wdStringUtils::GetStringElementCount(szReadPos, GetEndPointer());

      if (bReturnEmptyStrings || (uiLen > 0))
        ref_output.PushBack(wdStringView(szReadPos, szReadPos + uiLen));

      return;
    }

    if (bReturnEmptyStrings || (szFoundPos > szReadPos))
      ref_output.PushBack(wdStringView(szReadPos, szFoundPos));

    szReadPos = szFoundPos + seps[iFoundSeparator].GetElementCount();
  }
}

WD_ALWAYS_INLINE bool operator==(wdStringView lhs, wdStringView rhs)
{
  return lhs.IsEqual(rhs);
}

WD_ALWAYS_INLINE bool operator!=(wdStringView lhs, wdStringView rhs)
{
  return !lhs.IsEqual(rhs);
}

WD_ALWAYS_INLINE bool operator<(wdStringView lhs, wdStringView rhs)
{
  return lhs.Compare(rhs) < 0;
}

WD_ALWAYS_INLINE bool operator<=(wdStringView lhs, wdStringView rhs)
{
  return lhs.Compare(rhs) <= 0;
}

WD_ALWAYS_INLINE bool operator>(wdStringView lhs, wdStringView rhs)
{
  return lhs.Compare(rhs) > 0;
}

WD_ALWAYS_INLINE bool operator>=(wdStringView lhs, wdStringView rhs)
{
  return lhs.Compare(rhs) >= 0;
}
