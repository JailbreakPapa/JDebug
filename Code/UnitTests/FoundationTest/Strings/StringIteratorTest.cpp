#include <FoundationTest/FoundationTestPCH.h>

// NOTE: always save as Unicode UTF-8 with signature

#include <Foundation/Strings/String.h>

template <typename STRING>
void TestConstruction(const STRING& value, const char* szStart, const char* szEnd)
{
  nsStringUtf8 sUtf8(L"A単語F");
  NS_TEST_BOOL(value.IsEqual(sUtf8.GetData()));
  const bool bEqualForwardItTypes = nsConversionTest<typename STRING::iterator, typename STRING::const_iterator>::sameType == 1;
  NS_CHECK_AT_COMPILETIME_MSG(
    bEqualForwardItTypes, "As the string iterator is read-only, both const and non-const versions should be the same type.");
  const bool bEqualReverseItTypes = nsConversionTest<typename STRING::reverse_iterator, typename STRING::const_reverse_iterator>::sameType == 1;
  NS_CHECK_AT_COMPILETIME_MSG(
    bEqualReverseItTypes, "As the reverse string iterator is read-only, both const and non-const versions should be the same type.");

  typename STRING::iterator itInvalid;
  NS_TEST_BOOL(!itInvalid.IsValid());
  typename STRING::reverse_iterator itInvalidR;
  NS_TEST_BOOL(!itInvalidR.IsValid());

  // Begin
  const typename STRING::iterator itBegin = begin(value);
  NS_TEST_BOOL(itBegin == value.GetIteratorFront());
  NS_TEST_BOOL(itBegin.IsValid());
  NS_TEST_BOOL(itBegin == itBegin);
  NS_TEST_BOOL(itBegin.GetData() == szStart);
  NS_TEST_BOOL(itBegin.GetCharacter() == nsUnicodeUtils::ConvertUtf8ToUtf32("A"));
  NS_TEST_BOOL(*itBegin == nsUnicodeUtils::ConvertUtf8ToUtf32("A"));

  // End
  const typename STRING::iterator itEnd = end(value);
  NS_TEST_BOOL(!itEnd.IsValid());
  NS_TEST_BOOL(itEnd == itEnd);
  NS_TEST_BOOL(itBegin != itEnd);
  NS_TEST_BOOL(itEnd.GetData() == szEnd);
  NS_TEST_BOOL(itEnd.GetCharacter() == 0);
  NS_TEST_BOOL(*itEnd == 0);

  // RBegin
  const typename STRING::reverse_iterator itBeginR = rbegin(value);
  NS_TEST_BOOL(itBeginR == value.GetIteratorBack());
  NS_TEST_BOOL(itBeginR.IsValid());
  NS_TEST_BOOL(itBeginR == itBeginR);
  const char* szEndPrior = szEnd;
  nsUnicodeUtils::MoveToPriorUtf8(szEndPrior, szStart).AssertSuccess();
  NS_TEST_BOOL(itBeginR.GetData() == szEndPrior);
  NS_TEST_BOOL(itBeginR.GetCharacter() == nsUnicodeUtils::ConvertUtf8ToUtf32("F"));
  NS_TEST_BOOL(*itBeginR == nsUnicodeUtils::ConvertUtf8ToUtf32("F"));

  // REnd
  const typename STRING::reverse_iterator itEndR = rend(value);
  NS_TEST_BOOL(!itEndR.IsValid());
  NS_TEST_BOOL(itEndR == itEndR);
  NS_TEST_BOOL(itBeginR != itEndR);
  NS_TEST_BOOL(itEndR.GetData() == nullptr); // Position before first character is not a valid ptr, so it is set to nullptr.
  NS_TEST_BOOL(itEndR.GetCharacter() == 0);
  NS_TEST_BOOL(*itEndR == 0);
}

template <typename STRING, typename IT>
void TestIteratorBegin(const STRING& value, const IT& it)
{
  // It is safe to try to move beyond the iterator's range.
  IT itBegin = it;
  --itBegin;
  itBegin -= 4;
  NS_TEST_BOOL(itBegin == it);
  NS_TEST_BOOL(itBegin - 2 == it);

  // Prefix / Postfix
  NS_TEST_BOOL(itBegin + 2 != it);
  NS_TEST_BOOL(itBegin++ == it);
  NS_TEST_BOOL(itBegin-- != it);
  itBegin = it;
  NS_TEST_BOOL(++itBegin != it);
  NS_TEST_BOOL(--itBegin == it);

  // Misc
  itBegin = it;
  NS_TEST_BOOL(it + 2 == ++(++itBegin));
  itBegin -= 1;
  NS_TEST_BOOL(itBegin == it + 1);
  itBegin -= 0;
  NS_TEST_BOOL(itBegin == it + 1);
  itBegin += 0;
  NS_TEST_BOOL(itBegin == it + 1);
  itBegin += -1;
  NS_TEST_BOOL(itBegin == it);
}

template <typename STRING, typename IT>
void TestIteratorEnd(const STRING& value, const IT& it)
{
  // It is safe to try to move beyond the iterator's range.
  IT itEnd = it;
  ++itEnd;
  itEnd += 4;
  NS_TEST_BOOL(itEnd == it);
  NS_TEST_BOOL(itEnd + 2 == it);

  // Prefix / Postfix
  NS_TEST_BOOL(itEnd - 2 != it);
  NS_TEST_BOOL(itEnd-- == it);
  NS_TEST_BOOL(itEnd++ != it);
  itEnd = it;
  NS_TEST_BOOL(--itEnd != it);
  NS_TEST_BOOL(++itEnd == it);

  // Misc
  itEnd = it;
  NS_TEST_BOOL(it - 2 == --(--itEnd));
  itEnd += 1;
  NS_TEST_BOOL(itEnd == it - 1);
  itEnd += 0;
  NS_TEST_BOOL(itEnd == it - 1);
  itEnd -= 0;
  NS_TEST_BOOL(itEnd == it - 1);
  itEnd -= -1;
  NS_TEST_BOOL(itEnd == it);
}

template <typename STRING>
void TestOperators(const STRING& value, const char* szStart, const char* szEnd)
{
  nsStringUtf8 sUtf8(L"A単語F");
  NS_TEST_BOOL(value.IsEqual(sUtf8.GetData()));

  // Begin
  typename STRING::iterator itBegin = begin(value);
  TestIteratorBegin(value, itBegin);

  // End
  typename STRING::iterator itEnd = end(value);
  TestIteratorEnd(value, itEnd);

  // RBegin
  typename STRING::reverse_iterator itBeginR = rbegin(value);
  TestIteratorBegin(value, itBeginR);

  // REnd
  typename STRING::reverse_iterator itEndR = rend(value);
  TestIteratorEnd(value, itEndR);
}

template <typename STRING>
void TestLoops(const STRING& value, const char* szStart, const char* szEnd)
{
  nsStringUtf8 sUtf8(L"A単語F");
  nsUInt32 characters[] = {nsUnicodeUtils::ConvertUtf8ToUtf32(nsStringUtf8(L"A").GetData()),
    nsUnicodeUtils::ConvertUtf8ToUtf32(nsStringUtf8(L"単").GetData()), nsUnicodeUtils::ConvertUtf8ToUtf32(nsStringUtf8(L"語").GetData()),
    nsUnicodeUtils::ConvertUtf8ToUtf32(nsStringUtf8(L"F").GetData())};

  // Forward
  nsInt32 iIndex = 0;
  for (nsUInt32 character : value)
  {
    NS_TEST_INT(characters[iIndex], character);
    ++iIndex;
  }
  NS_TEST_INT(iIndex, 4);

  typename STRING::iterator itBegin = begin(value);
  typename STRING::iterator itEnd = end(value);
  iIndex = 0;
  for (auto it = itBegin; it != itEnd; ++it)
  {
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(characters[iIndex], it.GetCharacter());
    NS_TEST_INT(characters[iIndex], *it);
    NS_TEST_BOOL(it.GetData() >= szStart);
    NS_TEST_BOOL(it.GetData() < szEnd);
    ++iIndex;
  }
  NS_TEST_INT(iIndex, 4);

  // Reverse
  typename STRING::reverse_iterator itBeginR = rbegin(value);
  typename STRING::reverse_iterator itEndR = rend(value);
  iIndex = 3;
  for (auto it = itBeginR; it != itEndR; ++it)
  {
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(characters[iIndex], it.GetCharacter());
    NS_TEST_INT(characters[iIndex], *it);
    NS_TEST_BOOL(it.GetData() >= szStart);
    NS_TEST_BOOL(it.GetData() < szEnd);
    --iIndex;
  }
  NS_TEST_INT(iIndex, -1);
}

NS_CREATE_SIMPLE_TEST(Strings, StringIterator)
{
  nsStringUtf8 sUtf8(L"_A単語F_");
  nsStringBuilder sTestStringBuilder = sUtf8.GetData();
  sTestStringBuilder.Shrink(1, 1);
  nsString sTextString = sTestStringBuilder.GetData();

  nsStringView view(sUtf8.GetData());
  view.Shrink(1, 1);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construction")
  {
    TestConstruction<nsString>(sTextString, sTextString.GetData(), sTextString.GetData() + sTextString.GetElementCount());
    TestConstruction<nsStringBuilder>(
      sTestStringBuilder, sTestStringBuilder.GetData(), sTestStringBuilder.GetData() + sTestStringBuilder.GetElementCount());
    TestConstruction<nsStringView>(view, view.GetStartPointer(), view.GetEndPointer());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    TestOperators<nsString>(sTextString, sTextString.GetData(), sTextString.GetData() + sTextString.GetElementCount());
    TestOperators<nsStringBuilder>(
      sTestStringBuilder, sTestStringBuilder.GetData(), sTestStringBuilder.GetData() + sTestStringBuilder.GetElementCount());
    TestOperators<nsStringView>(view, view.GetStartPointer(), view.GetEndPointer());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Loops")
  {
    TestLoops<nsString>(sTextString, sTextString.GetData(), sTextString.GetData() + sTextString.GetElementCount());
    TestLoops<nsStringBuilder>(sTestStringBuilder, sTestStringBuilder.GetData(), sTestStringBuilder.GetData() + sTestStringBuilder.GetElementCount());
    TestLoops<nsStringView>(view, view.GetStartPointer(), view.GetEndPointer());
  }
}
