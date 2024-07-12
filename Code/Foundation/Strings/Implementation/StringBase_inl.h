#pragma once

template <typename Derived>
NS_ALWAYS_INLINE const char* nsStringBase<Derived>::InternalGetData() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetData();
}

template <typename Derived>
NS_ALWAYS_INLINE const char* nsStringBase<Derived>::InternalGetDataEnd() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetData() + pDerived->GetElementCount();
}

template <typename Derived>
NS_ALWAYS_INLINE nsUInt32 nsStringBase<Derived>::InternalGetElementCount() const
{
  const Derived* pDerived = static_cast<const Derived*>(this);
  return pDerived->GetElementCount();
}

template <typename Derived>
NS_ALWAYS_INLINE bool nsStringBase<Derived>::IsEmpty() const
{
  return nsStringUtils::IsNullOrEmpty(InternalGetData()) || (InternalGetData() == InternalGetDataEnd());
}

template <typename Derived>
bool nsStringBase<Derived>::StartsWith(nsStringView sStartsWith) const
{
  return nsStringUtils::StartsWith(InternalGetData(), sStartsWith.GetStartPointer(), InternalGetDataEnd(), sStartsWith.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::StartsWith_NoCase(nsStringView sStartsWith) const
{
  return nsStringUtils::StartsWith_NoCase(InternalGetData(), sStartsWith.GetStartPointer(), InternalGetDataEnd(), sStartsWith.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::EndsWith(nsStringView sEndsWith) const
{
  return nsStringUtils::EndsWith(InternalGetData(), sEndsWith.GetStartPointer(), InternalGetDataEnd(), sEndsWith.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::EndsWith_NoCase(nsStringView sEndsWith) const
{
  return nsStringUtils::EndsWith_NoCase(InternalGetData(), sEndsWith.GetStartPointer(), InternalGetDataEnd(), sEndsWith.GetEndPointer());
}

template <typename Derived>
const char* nsStringBase<Derived>::FindSubString(nsStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindSubString(szStartSearchAt, sStringToFind.GetStartPointer(), InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
const char* nsStringBase<Derived>::FindSubString_NoCase(nsStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindSubString_NoCase(szStartSearchAt, sStringToFind.GetStartPointer(), InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* nsStringBase<Derived>::FindLastSubString(nsStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetDataEnd();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()),
    "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindLastSubString(InternalGetData(), sStringToFind.GetStartPointer(), szStartSearchAt, InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* nsStringBase<Derived>::FindLastSubString_NoCase(nsStringView sStringToFind, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetDataEnd();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()),
    "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindLastSubString_NoCase(InternalGetData(), sStringToFind.GetStartPointer(), szStartSearchAt, InternalGetDataEnd(), sStringToFind.GetEndPointer());
}

template <typename Derived>
inline const char* nsStringBase<Derived>::FindWholeWord(const char* szSearchFor, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindWholeWord(szStartSearchAt, szSearchFor, isDelimiterCB, InternalGetDataEnd());
}

template <typename Derived>
inline const char* nsStringBase<Derived>::FindWholeWord_NoCase(const char* szSearchFor, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /* = nullptr */) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = InternalGetData();

  NS_ASSERT_DEV((szStartSearchAt >= InternalGetData()) && (szStartSearchAt <= InternalGetDataEnd()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindWholeWord_NoCase(szStartSearchAt, szSearchFor, isDelimiterCB, InternalGetDataEnd());
}

template <typename Derived>
nsInt32 nsStringBase<Derived>::Compare(nsStringView sOther) const
{
  return nsStringUtils::Compare(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
nsInt32 nsStringBase<Derived>::CompareN(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::CompareN(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
nsInt32 nsStringBase<Derived>::Compare_NoCase(nsStringView sOther) const
{
  return nsStringUtils::Compare_NoCase(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
nsInt32 nsStringBase<Derived>::CompareN_NoCase(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::CompareN_NoCase(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::IsEqual(nsStringView sOther) const
{
  return nsStringUtils::IsEqual(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::IsEqualN(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::IsEqualN(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::IsEqual_NoCase(nsStringView sOther) const
{
  return nsStringUtils::IsEqual_NoCase(InternalGetData(), sOther.GetStartPointer(), InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
bool nsStringBase<Derived>::IsEqualN_NoCase(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::IsEqualN_NoCase(InternalGetData(), sOther.GetStartPointer(), uiCharsToCompare, InternalGetDataEnd(), sOther.GetEndPointer());
}

template <typename Derived>
const char* nsStringBase<Derived>::ComputeCharacterPosition(nsUInt32 uiCharacterIndex) const
{
  const char* pos = InternalGetData();
  if (nsUnicodeUtils::MoveToNextUtf8(pos, InternalGetDataEnd(), uiCharacterIndex).Failed())
    return nullptr;

  return pos;
}

template <typename Derived>
typename nsStringBase<Derived>::iterator nsStringBase<Derived>::GetIteratorFront() const
{
  return begin(*this);
}

template <typename Derived>
typename nsStringBase<Derived>::reverse_iterator nsStringBase<Derived>::GetIteratorBack() const
{
  return rbegin(*this);
}

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator==(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.IsEqual(rhs.GetView());
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator==(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.IsEqual(lhs);
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator==(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.IsEqual(rhs);
}

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator!=(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return !lhs.IsEqual(rhs);
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator!=(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return !rhs.IsEqual(lhs);
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator!=(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return !lhs.IsEqual(rhs);
}

#endif

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE std::strong_ordering operator<=>(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs)
{
  return lhs.Compare(rhs) <=> 0;
}

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE std::strong_ordering operator<=>(const nsStringBase<DerivedLhs>& lhs, const char* rhs)
{
  return lhs.Compare(rhs) <=> 0;
}

#else

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator<(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.Compare(rhs) < 0;
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator<(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) > 0;
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator<(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) < 0;
}

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator>(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return lhs.Compare(rhs) > 0;
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator>(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) < 0;
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator>(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) > 0;
}

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator<=(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return nsStringUtils::Compare(lhs.InternalGetData(), rhs.InternalGetData(), lhs.InternalGetDataEnd(), rhs.InternalGetDataEnd()) <= 0;
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator<=(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) >= 0;
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator<=(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) <= 0;
}

template <typename DerivedLhs, typename DerivedRhs>
NS_ALWAYS_INLINE bool operator>=(const nsStringBase<DerivedLhs>& lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return nsStringUtils::Compare(lhs.InternalGetData(), rhs.InternalGetData(), lhs.InternalGetDataEnd(), rhs.InternalGetDataEnd()) >= 0;
}

template <typename DerivedRhs>
NS_ALWAYS_INLINE bool operator>=(const char* lhs, const nsStringBase<DerivedRhs>& rhs) // [tested]
{
  return rhs.Compare(lhs) <= 0;
}

template <typename DerivedLhs>
NS_ALWAYS_INLINE bool operator>=(const nsStringBase<DerivedLhs>& lhs, const char* rhs) // [tested]
{
  return lhs.Compare(rhs) >= 0;
}

#endif

template <typename DerivedLhs>
NS_ALWAYS_INLINE nsStringBase<DerivedLhs>::operator nsStringView() const
{
  return nsStringView(InternalGetData(), InternalGetElementCount());
}

template <typename Derived>
NS_ALWAYS_INLINE nsStringView nsStringBase<Derived>::GetView() const
{
  return nsStringView(InternalGetData(), InternalGetElementCount());
}

template <typename Derived>
template <typename Container>
void nsStringBase<Derived>::Split(bool bReturnEmptyStrings, Container& ref_output, const char* szSeparator1, const char* szSeparator2 /*= nullptr*/, const char* szSeparator3 /*= nullptr*/, const char* szSeparator4 /*= nullptr*/, const char* szSeparator5 /*= nullptr*/, const char* szSeparator6 /*= nullptr*/) const
{
  GetView().Split(bReturnEmptyStrings, ref_output, szSeparator1, szSeparator2, szSeparator3, szSeparator4, szSeparator5, szSeparator6);
}

template <typename Derived>
nsStringView nsStringBase<Derived>::GetRootedPathRootName() const
{
  return GetView().GetRootedPathRootName();
}

template <typename Derived>
bool nsStringBase<Derived>::IsRootedPath() const
{
  return GetView().IsRootedPath();
}

template <typename Derived>
bool nsStringBase<Derived>::IsRelativePath() const
{
  return GetView().IsRelativePath();
}

template <typename Derived>
bool nsStringBase<Derived>::IsAbsolutePath() const
{
  return GetView().IsAbsolutePath();
}

template <typename Derived>
nsStringView nsStringBase<Derived>::GetFileDirectory() const
{
  return GetView().GetFileDirectory();
}

template <typename Derived>
nsStringView nsStringBase<Derived>::GetFileNameAndExtension() const
{
  return GetView().GetFileNameAndExtension();
}

template <typename Derived>
nsStringView nsStringBase<Derived>::GetFileName() const
{
  return GetView().GetFileName();
}

template <typename Derived>
nsStringView nsStringBase<Derived>::GetFileExtension() const
{
  return GetView().GetFileExtension();
}

template <typename Derived>
bool nsStringBase<Derived>::HasExtension(nsStringView sExtension) const
{
  return GetView().HasExtension(sExtension);
}

template <typename Derived>
bool nsStringBase<Derived>::HasAnyExtension() const
{
  return GetView().HasAnyExtension();
}
