#pragma once

template <typename Derived>
WD_ALWAYS_INLINE const char* wdStringBase<Derived>::InternalGetData() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetData();
}

template <typename Derived>
WD_ALWAYS_INLINE const char* wdStringBase<Derived>::InternalGetDataEnd() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetData() + pDerived->GetElementCount();
}

template <typename Derived>
WD_ALWAYS_INLINE wdUInt32 wdStringBase<Derived>::InternalGetElementCount() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetElementCount();
}

template <typename Derived>
WD_ALWAYS_INLINE bool wdStringBase<Derived>::IsEmpty() const
{
  return wdStringUtils::IsNullOrEmpty(InternalGetData()) || (InternalGetData() == InternalGetDataEnd());
}

template <typename Derived>
bool wdStringBase<Derived>::StartsWith(wdStringView sStartsWith) const
{
  return wdStringUtils::StartsWith(InternalGetData(), sStartsWith.GetStartPointer(), InternalGetDataEnd(), sStartsWith.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::StartsWith_NoCase(wdStringView sStartsWith) const
{
  return wdStringUtils::StartsWith_NoCase(InternalGetData(), sStartsWith.GetStartPointer(), InternalGetDataEnd(), sStartsWith.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::EndsWith(wdStringView sEndsWith) const
{
  return wdStringUtils::EndsWith(InternalGetData(), sEndsWith.GetStartPointer(), InternalGetDataEnd(), sEndsWith.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::EndsWith_NoCase(wdStringView sEndsWith) const
{
  return wdStringUtils::EndsWith_NoCase(InternalGetData(), sEndsWith.GetStartPointer(), InternalGetDataEnd(), sEndsWith.GetEndPointer());
}

template <typename Derived>
const char* wdStringBase<Derived>::FindSubString(wdStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindSubString(szStartSearchAt, sStringToFind.GetStartPointer(), InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
const char* wdStringBase<Derived>::FindSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindSubString_NoCase(szStartSearchAt, sStringToFind.GetStartPointer(), InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* wdStringBase<Derived>::FindLastSubString(wdStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetDataEnd();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()),
    "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindLastSubString(InternalGetData(), sStringToFind.GetStartPointer(), szStartSearchAt, InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* wdStringBase<Derived>::FindLastSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetDataEnd();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()),
    "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindLastSubString_NoCase(InternalGetData(), sStringToFind.GetStartPointer(), szStartSearchAt, InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* wdStringBase<Derived>::FindWholeWord(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindWholeWord(szStartSearchAt, szSearchFor, isDelimiterCB, InternalGetDataEnd());
}

template <typename Derived>
inline const char* wdStringBase<Derived>::FindWholeWord_NoCase(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  WD_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindWholeWord_NoCase(szStartSearchAt, szSearchFor, isDelimiterCB, InternalGetDataEnd());
}

template <typename Derived>
wdInt32 wdStringBase<Derived>::Compare(wdStringView sOther) const
{
  return wdStringUtils::Compare(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
wdInt32 wdStringBase<Derived>::CompareN(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::CompareN(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
wdInt32 wdStringBase<Derived>::Compare_NoCase(wdStringView sOther) const
{
  return wdStringUtils::Compare_NoCase(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
wdInt32 wdStringBase<Derived>::CompareN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::CompareN_NoCase(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::IsEqual(wdStringView sOther) const
{
  return wdStringUtils::IsEqual(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::IsEqualN(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::IsEqualN(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::IsEqual_NoCase(wdStringView sOther) const
{
  return wdStringUtils::IsEqual_NoCase(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool wdStringBase<Derived>::IsEqualN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::IsEqualN_NoCase(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
const char* wdStringBase<Derived>::ComputeCharacterPosition(wdUInt32 uiCharacterIndex) const
{
  const char* pos = InternalGetData();
  wdUnicodeUtils::MoveToNextUtf8(pos, InternalGetDataEnd(), uiCharacterIndex);
  return pos;
}

template <typename Derived>
typename wdStringBase<Derived>::iterator wdStringBase<Derived>::GetIteratorFront() const
{
  return begin(*this);
}

template <typename Derived>
typename wdStringBase<Derived>::reverse_iterator wdStringBase<Derived>::GetIteratorBack() const
{
  return rbegin(*this);
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator==(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.IsEqual(rhs.GetView());
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator==(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.IsEqual(lhs);
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator==(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.IsEqual(rhs);
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator!=(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return !lhs.IsEqual(rhs);
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator!=(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return !rhs.IsEqual(lhs);
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator!=(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return !lhs.IsEqual(rhs);
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator<(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.Compare(rhs) < 0;
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator<(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) > 0;
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator<(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) < 0;
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator>(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.Compare(rhs) > 0;
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator>(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) < 0;
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator>(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) > 0;
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator<=(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return wdStringUtils::Compare(lhs.InternalGetData(), rhs.InternalGetData(), lhs.InternalGetDataEnd(), rhs.InternalGetDataEnd()) <= 0;
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator<=(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) >= 0;
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator<=(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) <= 0;
}

template <typename DerivedLhs, typename DerivedRhs>
WD_ALWAYS_INLINE bool operator>=(const wdStringBase<DerivedLhs>& lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return wdStringUtils::Compare(lhs.InternalGetData(), rhs.InternalGetData(), lhs.InternalGetDataEnd(), rhs.InternalGetDataEnd()) >= 0;
}

template <typename DerivedRhs>
WD_ALWAYS_INLINE bool operator>=(const char* lhs, const wdStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) <= 0;
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE bool operator>=(const wdStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) >= 0;
}

template <typename DerivedLhs>
WD_ALWAYS_INLINE wdStringBase<DerivedLhs>::operator wdStringView() const
{
  return wdStringView(InternalGetData(), InternalGetElementCount());
}

template <typename Derived>
WD_ALWAYS_INLINE wdStringView wdStringBase<Derived>::GetView() const
{
  return wdStringView(InternalGetData(), InternalGetElementCount());
}

template <typename Derived>
template <typename Container>
void wdStringBase<Derived>::Split(bool bReturnEmptyStrings, Container& ref_output, const char* szSeparator1, const char* szSeparator2 /*= nullptr*/, const char* szSeparator3 /*= nullptr*/, const char* szSeparator4 /*= nullptr*/, const char* szSeparator5 /*= nullptr*/, const char* szSeparator6 /*= nullptr*/) const
{
  GetView().Split(bReturnEmptyStrings, ref_output, szSeparator1, szSeparator2, szSeparator3, szSeparator4, szSeparator5, szSeparator6);
}

template <typename Derived>
wdStringView wdStringBase<Derived>::GetRootedPathRootName() const
{
  return GetView().GetRootedPathRootName();
}

template <typename Derived>
bool wdStringBase<Derived>::IsRootedPath() const
{
  return GetView().IsRootedPath();
}

template <typename Derived>
bool wdStringBase<Derived>::IsRelativePath() const
{
  return GetView().IsRelativePath();
}

template <typename Derived>
bool wdStringBase<Derived>::IsAbsolutePath() const
{
  return GetView().IsAbsolutePath();
}

template <typename Derived>
wdStringView wdStringBase<Derived>::GetFileDirectory() const
{
  return GetView().GetFileDirectory();
}

template <typename Derived>
wdStringView wdStringBase<Derived>::GetFileNameAndExtension() const
{
  return GetView().GetFileNameAndExtension();
}

template <typename Derived>
wdStringView wdStringBase<Derived>::GetFileName() const
{
  return GetView().GetFileName();
}

template <typename Derived>
wdStringView wdStringBase<Derived>::GetFileExtension() const
{
  return GetView().GetFileExtension();
}

template <typename Derived>
bool wdStringBase<Derived>::HasExtension(wdStringView sExtension) const
{
  return GetView().HasExtension(sExtension);
}

template <typename Derived>
bool wdStringBase<Derived>::HasAnyExtension() const
{
  return GetView().HasAnyExtension();
}
