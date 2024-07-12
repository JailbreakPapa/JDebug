#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/StringBuilder.h>

#include <stdarg.h>

nsStringBuilder::nsStringBuilder(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6)
{
  AppendTerminator();

  Append(sData1, sData2, sData3, sData4, sData5, sData6);
}

void nsStringBuilder::Set(nsStringView sData1)
{
  Clear();
  Append(sData1);
}

void nsStringBuilder::Set(nsStringView sData1, nsStringView sData2)
{
  Clear();
  Append(sData1, sData2);
}

void nsStringBuilder::Set(nsStringView sData1, nsStringView sData2, nsStringView sData3)
{
  Clear();
  Append(sData1, sData2, sData3);
}

void nsStringBuilder::Set(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4)
{
  Clear();
  Append(sData1, sData2, sData3, sData4);
}

void nsStringBuilder::Set(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6)
{
  Clear();
  Append(sData1, sData2, sData3, sData4, sData5, sData6);
}

void nsStringBuilder::SetPath(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4)
{
  Clear();
  AppendPath(sData1, sData2, sData3, sData4);
}

void nsStringBuilder::SetSubString_FromTo(const char* pStart, const char* pEnd)
{
  NS_ASSERT_DEBUG(nsUnicodeUtils::IsValidUtf8(pStart, pEnd), "Invalid substring, the start does not point to a valid Utf-8 character");

  nsStringView view(pStart, pEnd);
  *this = view;
}

void nsStringBuilder::SetSubString_ElementCount(const char* pStart, nsUInt32 uiElementCount)
{
  NS_ASSERT_DEBUG(
    nsUnicodeUtils::IsValidUtf8(pStart, pStart + uiElementCount), "Invalid substring, the start does not point to a valid Utf-8 character");

  nsStringView view(pStart, pStart + uiElementCount);
  *this = view;
}

void nsStringBuilder::SetSubString_CharacterCount(const char* pStart, nsUInt32 uiCharacterCount)
{
  const char* pEnd = pStart;
  nsUnicodeUtils::MoveToNextUtf8(pEnd, uiCharacterCount).IgnoreResult(); // fine to fail, will just copy as much as possible

  nsStringView view(pStart, pEnd);
  *this = view;
}

void nsStringBuilder::Append(nsStringView sData1)
{
  nsUInt32 uiMoreBytes = 0;
  uiMoreBytes += sData1.GetElementCount();

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  {
    const char* szStartPtr = sData1.GetStartPointer();
    const nsUInt32 uiStrLen = sData1.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }
}

void nsStringBuilder::Append(nsStringView sData1, nsStringView sData2)
{
  nsUInt32 uiMoreBytes = 0;
  uiMoreBytes += sData1.GetElementCount();
  uiMoreBytes += sData2.GetElementCount();

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  {
    const char* szStartPtr = sData1.GetStartPointer();
    const nsUInt32 uiStrLen = sData1.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData2.GetStartPointer();
    const nsUInt32 uiStrLen = sData2.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }
}

void nsStringBuilder::Append(nsStringView sData1, nsStringView sData2, nsStringView sData3)
{
  nsUInt32 uiMoreBytes = 0;
  uiMoreBytes += sData1.GetElementCount();
  uiMoreBytes += sData2.GetElementCount();
  uiMoreBytes += sData3.GetElementCount();

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  {
    const char* szStartPtr = sData1.GetStartPointer();
    const nsUInt32 uiStrLen = sData1.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData2.GetStartPointer();
    const nsUInt32 uiStrLen = sData2.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData3.GetStartPointer();
    const nsUInt32 uiStrLen = sData3.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }
}

void nsStringBuilder::Append(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4)
{
  nsUInt32 uiMoreBytes = 0;
  uiMoreBytes += sData1.GetElementCount();
  uiMoreBytes += sData2.GetElementCount();
  uiMoreBytes += sData3.GetElementCount();
  uiMoreBytes += sData4.GetElementCount();

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  {
    const char* szStartPtr = sData1.GetStartPointer();
    const nsUInt32 uiStrLen = sData1.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData2.GetStartPointer();
    const nsUInt32 uiStrLen = sData2.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData3.GetStartPointer();
    const nsUInt32 uiStrLen = sData3.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData4.GetStartPointer();
    const nsUInt32 uiStrLen = sData4.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }
}

void nsStringBuilder::Append(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6)
{
  nsUInt32 uiMoreBytes = 0;
  uiMoreBytes += sData1.GetElementCount();
  uiMoreBytes += sData2.GetElementCount();
  uiMoreBytes += sData3.GetElementCount();
  uiMoreBytes += sData4.GetElementCount();
  uiMoreBytes += sData5.GetElementCount();
  uiMoreBytes += sData6.GetElementCount();

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  {
    const char* szStartPtr = sData1.GetStartPointer();
    const nsUInt32 uiStrLen = sData1.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData2.GetStartPointer();
    const nsUInt32 uiStrLen = sData2.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData3.GetStartPointer();
    const nsUInt32 uiStrLen = sData3.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData4.GetStartPointer();
    const nsUInt32 uiStrLen = sData4.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData5.GetStartPointer();
    const nsUInt32 uiStrLen = sData5.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }

  {
    const char* szStartPtr = sData6.GetStartPointer();
    const nsUInt32 uiStrLen = sData6.GetElementCount();
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen + 1, szStartPtr, szStartPtr + uiStrLen);
    uiPrevCount += uiStrLen;
  }
}

void nsStringBuilder::Prepend(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6)
{
  // it is not possible to find out how many parameters were passed to a vararg function
  // with a fixed size of parameters we do not need to have a parameter that tells us how many strings will come

  const nsUInt32 uiMaxParams = 6;

  const nsStringView pStrings[uiMaxParams] = {sData1, sData2, sData3, sData4, sData5, sData6};
  nsUInt32 uiStrLen[uiMaxParams] = {0};

  nsUInt32 uiMoreBytes = 0;

  // first figure out how much the string has to grow
  for (nsUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (pStrings[i].IsEmpty())
      continue;

    uiStrLen[i] = pStrings[i].GetElementCount();
    uiMoreBytes += uiStrLen[i];

    NS_ASSERT_DEBUG(nsUnicodeUtils::IsValidUtf8(pStrings[i].GetStartPointer(), pStrings[i].GetEndPointer()), "Parameter {0} is not a valid Utf8 sequence.", i + 1);
  }

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  NS_ASSERT_DEBUG(uiPrevCount > 0, "There should be a 0 terminator somewhere around here.");

  // now resize
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  // move the previous string data at the end
  nsMemoryUtils::CopyOverlapped(&m_Data[0] + uiMoreBytes, GetData(), uiPrevCount);

  nsUInt32 uiWritePos = 0;

  // and then prepend all the strings
  for (nsUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (uiStrLen[i] == 0)
      continue;

    // make enough room to copy the entire string, including the T-800
    nsMemoryUtils::Copy(&m_Data[uiWritePos], pStrings[i].GetStartPointer(), uiStrLen[i]);

    uiWritePos += uiStrLen[i];
  }
}

void nsStringBuilder::SetPrintfArgs(const char* szUtf8Format, va_list szArgs0)
{
  va_list args;
  va_copy(args, szArgs0);

  Clear();

  const nsUInt32 TempBuffer = 4096;

  char szTemp[TempBuffer];
  const nsInt32 iCount = nsStringUtils::vsnprintf(szTemp, TempBuffer - 1, szUtf8Format, args);

  NS_ASSERT_DEV(iCount != -1, "There was an error while formatting the string. Probably and unescaped usage of the %% sign.");

  if (iCount == -1)
  {
    va_end(args);
    return;
  }

  if (iCount > TempBuffer - 1)
  {
    nsDynamicArray<char> Temp;
    Temp.SetCountUninitialized(iCount + 1);

    nsStringUtils::vsnprintf(&Temp[0], iCount + 1, szUtf8Format, args);

    Append(&Temp[0]);
  }
  else
  {
    Append(&szTemp[0]);
  }

  va_end(args);
}

void nsStringBuilder::ChangeCharacterNonASCII(iterator& it, nsUInt32 uiCharacter)
{
  char* pPos = const_cast<char*>(it.GetData()); // yes, I know...

  const nsUInt32 uiOldCharLength = nsUnicodeUtils::GetUtf8SequenceLength(*pPos);
  const nsUInt32 uiNewCharLength = nsUnicodeUtils::GetSizeForCharacterInUtf8(uiCharacter);

  // if the old character and the new one are encoded with the same length, we can replace the character in-place
  if (uiNewCharLength == uiOldCharLength)
  {
    // just overwrite all characters at the given position with the new Utf8 string
    nsUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);

    // if the encoding length is identical, this will also handle all ASCII strings
    // if the string was pure ASCII before, this won't change, so no need to update that state
    return;
  }

  // in this case we can still update the string without reallocation, but the tail of the string has to be moved forwards
  if (uiNewCharLength < uiOldCharLength)
  {
    // just overwrite all characters at the given position with the new Utf8 string
    nsUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);

    // pPos will be changed (moved forwards) to the next character position

    // how much has changed
    const nsUInt32 uiDifference = uiOldCharLength - uiNewCharLength;
    const nsUInt32 uiTrailStringBytes = (nsUInt32)(GetData() + GetElementCount() - it.GetData() - uiOldCharLength + 1); // ???

    // move the trailing characters forwards
    nsMemoryUtils::CopyOverlapped(pPos, pPos + uiDifference, uiTrailStringBytes);

    // update the data array
    m_Data.PopBack(uiDifference);

    // 'It' references this already, no need to change anything.
  }
  else
  {
    // in this case we insert a character that is longer int Utf8 encoding than the character that already exists there *sigh*
    // so we must first move the trailing string backwards to make room, then we can write the new char in there

    // how much has changed
    const nsUInt32 uiDifference = uiNewCharLength - uiOldCharLength;
    const nsUInt32 uiTrailStringBytes = (nsUInt32)(GetData() + GetElementCount() - it.GetData() - uiOldCharLength + 1);
    auto iCurrentPos = (it.GetData() - GetData());
    // resize the array
    m_Data.SetCountUninitialized(m_Data.GetCount() + uiDifference);

    // these might have changed (array realloc)
    pPos = &m_Data[0] + iCurrentPos;
    it.SetCurrentPosition(pPos);

    // move the trailing string backwards
    nsMemoryUtils::CopyOverlapped(pPos + uiNewCharLength, pPos + uiOldCharLength, uiTrailStringBytes);

    // just overwrite all characters at the given position with the new Utf8 string
    nsUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);
  }
}

void nsStringBuilder::Shrink(nsUInt32 uiShrinkCharsFront, nsUInt32 uiShrinkCharsBack)
{
  if (uiShrinkCharsBack > 0)
  {
    const char* szEnd = GetData() + GetElementCount();
    const char* szNewEnd = szEnd;
    if (nsUnicodeUtils::MoveToPriorUtf8(szNewEnd, GetData(), uiShrinkCharsBack).Failed())
    {
      Clear();
      return;
    }

    const nsUInt32 uiLessBytes = (nsUInt32)(szEnd - szNewEnd);

    m_Data.PopBack(uiLessBytes + 1);
    AppendTerminator();
  }

  const char* szNewStart = &m_Data[0];
  if (nsUnicodeUtils::MoveToNextUtf8(szNewStart, uiShrinkCharsFront).Failed())
  {
    Clear();
    return;
  }

  if (szNewStart > &m_Data[0])
  {
    const nsUInt32 uiLessBytes = (nsUInt32)(szNewStart - &m_Data[0]);

    nsMemoryUtils::CopyOverlapped(&m_Data[0], szNewStart, m_Data.GetCount() - uiLessBytes);
    m_Data.PopBack(uiLessBytes);
  }
}

void nsStringBuilder::ReplaceSubString(const char* szStartPos, const char* szEndPos, nsStringView sReplaceWith)
{
  NS_ASSERT_DEV(nsMath::IsInRange(szStartPos, GetData(), GetData() + m_Data.GetCount()), "szStartPos is not inside this string.");
  NS_ASSERT_DEV(nsMath::IsInRange(szEndPos, GetData(), GetData() + m_Data.GetCount()), "szEndPos is not inside this string.");
  NS_ASSERT_DEV(szStartPos <= szEndPos, "nsStartPos must be before nsEndPos");

  const nsUInt32 uiWordBytes = sReplaceWith.GetElementCount();

  const nsUInt32 uiSubStringBytes = (nsUInt32)(szEndPos - szStartPos);

  char* szWritePos = const_cast<char*>(szStartPos); // szStartPos points into our own data anyway
  const char* szReadPos = sReplaceWith.GetStartPointer();

  // most simple case, just replace characters
  if (uiSubStringBytes == uiWordBytes)
  {
    while (szWritePos < szEndPos)
    {
      *szWritePos = *szReadPos;
      ++szWritePos;
      ++szReadPos;
    }

    return;
  }

  // the replacement is shorter than the existing stuff -> move characters to the left, no reallocation needed
  if (uiWordBytes < uiSubStringBytes)
  {
    // first copy the replacement to the correct position
    nsMemoryUtils::Copy(szWritePos, sReplaceWith.GetStartPointer(), uiWordBytes);

    const nsUInt32 uiDifference = uiSubStringBytes - uiWordBytes;

    const char* szStringEnd = GetData() + m_Data.GetCount();

    // now move all the characters from behind the replaced string to the correct position
    nsMemoryUtils::CopyOverlapped(szWritePos + uiWordBytes, szWritePos + uiSubStringBytes, szStringEnd - (szWritePos + uiSubStringBytes));

    m_Data.PopBack(uiDifference);

    return;
  }

  // else the replacement is longer than the existing word
  {
    const nsUInt32 uiDifference = uiWordBytes - uiSubStringBytes;
    const nsUInt64 uiRelativeWritePosition = szWritePos - GetData();
    const nsUInt64 uiDataByteCountBefore = m_Data.GetCount();

    m_Data.SetCountUninitialized(m_Data.GetCount() + uiDifference);

    // all pointer are now possibly invalid since the data may be reallocated!
    szWritePos = const_cast<char*>(GetData()) + uiRelativeWritePosition;
    const char* szStringEnd = GetData() + uiDataByteCountBefore;

    // first move the characters to the proper position from back to front
    nsMemoryUtils::CopyOverlapped(szWritePos + uiWordBytes, szWritePos + uiSubStringBytes, szStringEnd - (szWritePos + uiSubStringBytes));

    // now copy the replacement to the correct position
    nsMemoryUtils::Copy(szWritePos, sReplaceWith.GetStartPointer(), uiWordBytes);
  }
}

const char* nsStringBuilder::ReplaceFirst(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData();
  else
  {
    NS_ASSERT_DEV(nsMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = nsStringUtils::FindSubString(szStartSearchAt, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = (nsUInt32)(szFoundAt - GetData());

  const nsUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

const char* nsStringBuilder::ReplaceLast(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData() + m_Data.GetCount() - 1;
  else
  {
    NS_ASSERT_DEV(nsMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = nsStringUtils::FindLastSubString(GetData(), sSearchFor.GetStartPointer(), szStartSearchAt, GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = (nsUInt32)(szFoundAt - GetData());

  const nsUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

nsUInt32 nsStringBuilder::ReplaceAll(nsStringView sSearchFor, nsStringView sReplacement)
{
  const nsUInt32 uiSearchBytes = sSearchFor.GetElementCount();
  const nsUInt32 uiWordBytes = sReplacement.GetElementCount();

  nsUInt32 uiReplacements = 0;
  nsUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = nsStringUtils::FindSubString(GetData() + uiOffset, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<nsUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplacement);

    ++uiReplacements;
  }

  return uiReplacements;
}


const char* nsStringBuilder::ReplaceFirst_NoCase(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData();
  else
  {
    NS_ASSERT_DEV(nsMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = nsStringUtils::FindSubString_NoCase(szStartSearchAt, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = (nsUInt32)(szFoundAt - GetData());

  const nsUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

const char* nsStringBuilder::ReplaceLast_NoCase(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData() + m_Data.GetCount() - 1;
  else
  {
    NS_ASSERT_DEV(nsMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = nsStringUtils::FindLastSubString_NoCase(GetData(), sSearchFor.GetStartPointer(), szStartSearchAt, GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = (nsUInt32)(szFoundAt - GetData());

  const nsUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

nsUInt32 nsStringBuilder::ReplaceAll_NoCase(nsStringView sSearchFor, nsStringView sReplacement)
{
  const nsUInt32 uiSearchBytes = sSearchFor.GetElementCount();
  const nsUInt32 uiWordBytes = sReplacement.GetElementCount();

  nsUInt32 uiReplacements = 0;
  nsUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = nsStringUtils::FindSubString_NoCase(GetData() + uiOffset, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<nsUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplacement);

    ++uiReplacements;
  }

  return uiReplacements;
}

const char* nsStringBuilder::ReplaceWholeWord(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB)
{
  const char* szPos = FindWholeWord(szSearchFor, isDelimiterCB);

  if (szPos == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = static_cast<nsUInt32>(szPos - GetData());

  ReplaceSubString(szPos, szPos + nsStringUtils::GetStringElementCount(szSearchFor), sReplaceWith);
  return GetData() + uiOffset;
}

const char* nsStringBuilder::ReplaceWholeWord_NoCase(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB)
{
  const char* szPos = FindWholeWord_NoCase(szSearchFor, isDelimiterCB);

  if (szPos == nullptr)
    return nullptr;

  const nsUInt32 uiOffset = static_cast<nsUInt32>(szPos - GetData());

  ReplaceSubString(szPos, szPos + nsStringUtils::GetStringElementCount(szSearchFor), sReplaceWith);
  return GetData() + uiOffset;
}


nsUInt32 nsStringBuilder::ReplaceWholeWordAll(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB)
{
  const nsUInt32 uiSearchBytes = nsStringUtils::GetStringElementCount(szSearchFor);
  const nsUInt32 uiWordBytes = nsStringUtils::GetStringElementCount(sReplaceWith.GetStartPointer(), sReplaceWith.GetEndPointer());

  nsUInt32 uiReplacements = 0;
  nsUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = nsStringUtils::FindWholeWord(GetData() + uiOffset, szSearchFor, isDelimiterCB, GetData() + m_Data.GetCount() - 1);

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<nsUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplaceWith);

    ++uiReplacements;
  }

  return uiReplacements;
}

nsUInt32 nsStringBuilder::ReplaceWholeWordAll_NoCase(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB)
{
  const nsUInt32 uiSearchBytes = nsStringUtils::GetStringElementCount(szSearchFor);
  const nsUInt32 uiWordBytes = nsStringUtils::GetStringElementCount(sReplaceWith.GetStartPointer(), sReplaceWith.GetEndPointer());

  nsUInt32 uiReplacements = 0;
  nsUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = nsStringUtils::FindWholeWord_NoCase(GetData() + uiOffset, szSearchFor, isDelimiterCB, GetData() + m_Data.GetCount() - 1);

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<nsUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplaceWith);

    ++uiReplacements;
  }

  return uiReplacements;
}

void nsStringBuilder::operator=(nsStringView rhs)
{
  nsUInt32 uiBytes = rhs.GetElementCount();

  // if we need more room, allocate up front (rhs cannot use our own data in this case)
  if (uiBytes + 1 > m_Data.GetCount())
    m_Data.SetCountUninitialized(uiBytes + 1);

  // the data might actually come from our very own string, so we 'move' the memory in there, just to be safe
  // if it comes from our own array, the data will always be a sub-set -> smaller than this array
  // in this case we defer the SetCount till later, to ensure that the data is not corrupted (destructed) before we copy it
  // however, when the new data is larger than the old, it cannot be from our own data, so we can (and must) reallocate before copying
  nsMemoryUtils::CopyOverlapped(&m_Data[0], rhs.GetStartPointer(), uiBytes);

  m_Data.SetCountUninitialized(uiBytes + 1);
  m_Data[uiBytes] = '\0';
}

enum PathUpState
{
  NotStarted,
  OneDot,
  TwoDots,
  FoundDotSlash,
  FoundDotDotSlash,
  Invalid,
};

void nsStringBuilder::MakeCleanPath()
{
  if (IsEmpty())
    return;

  Trim(" \t\r\n");

  RemoveDoubleSlashesInPath();

  // remove Windows specific DOS device path indicators from the start
  TrimWordStart("//?/");
  TrimWordStart("//./");

  const char* const szEndPos = &m_Data[m_Data.GetCount() - 1];
  const char* szCurReadPos = &m_Data[0];
  char* const szCurWritePos = &m_Data[0];
  int writeOffset = 0;

  nsInt32 iLevelsDown = 0;
  PathUpState FoundPathUp = NotStarted;

  while (szCurReadPos < szEndPos)
  {
    char CurChar = *szCurReadPos;

    if (CurChar == '.')
    {
      if (FoundPathUp == NotStarted)
        FoundPathUp = OneDot;
      else if (FoundPathUp == OneDot)
        FoundPathUp = TwoDots;
      else
        FoundPathUp = Invalid;
    }
    else if (nsPathUtils::IsPathSeparator(CurChar))
    {
      CurChar = '/';

      if (FoundPathUp == OneDot)
      {
        FoundPathUp = FoundDotSlash;
      }
      else if (FoundPathUp == TwoDots)
      {
        FoundPathUp = FoundDotDotSlash;
      }
      else
      {
        ++iLevelsDown;
        FoundPathUp = NotStarted;
      }
    }
    else
      FoundPathUp = NotStarted;

    if (FoundPathUp == FoundDotDotSlash)
    {
      if (iLevelsDown > 0)
      {
        --iLevelsDown;
        NS_ASSERT_DEBUG(writeOffset >= 3, "invalid write offset");
        writeOffset -= 3; // go back, skip two dots, one slash

        while ((writeOffset > 0) && (szCurWritePos[writeOffset - 1] != '/'))
        {
          NS_ASSERT_DEBUG(writeOffset > 0, "invalid write offset");
          --writeOffset;
        }
      }
      else
      {
        szCurWritePos[writeOffset] = '/';
        ++writeOffset;
      }

      FoundPathUp = NotStarted;
    }
    else if (FoundPathUp == FoundDotSlash)
    {
      NS_ASSERT_DEBUG(writeOffset > 0, "invalid write offset");
      writeOffset -= 1; // go back to where we wrote the dot

      FoundPathUp = NotStarted;
    }
    else
    {
      szCurWritePos[writeOffset] = CurChar;
      ++writeOffset;
    }

    ++szCurReadPos;
  }

  const nsUInt32 uiPrevByteCount = m_Data.GetCount();
  const nsUInt32 uiNewByteCount = (nsUInt32)(writeOffset) + 1;

  NS_IGNORE_UNUSED(uiPrevByteCount);
  NS_ASSERT_DEBUG(uiPrevByteCount >= uiNewByteCount, "It should not be possible that a path grows during cleanup. Old: {0} Bytes, New: {1} Bytes",
    uiPrevByteCount, uiNewByteCount);

  // make sure to write the terminating \0 and reset the count
  szCurWritePos[writeOffset] = '\0';
  m_Data.SetCountUninitialized(uiNewByteCount);
}

void nsStringBuilder::PathParentDirectory(nsUInt32 uiLevelsUp)
{
  NS_ASSERT_DEV(uiLevelsUp > 0, "We have to do something!");

  for (nsUInt32 i = 0; i < uiLevelsUp; ++i)
    AppendPath("../");

  MakeCleanPath();
}

void nsStringBuilder::AppendPath(nsStringView sPath1, nsStringView sPath2, nsStringView sPath3, nsStringView sPath4)
{
  const nsStringView sPaths[4] = {sPath1, sPath2, sPath3, sPath4};

  for (nsUInt32 i = 0; i < 4; ++i)
  {
    nsStringView sThisPath = sPaths[i];

    if (!sThisPath.IsEmpty())
    {
      if ((IsEmpty() && nsPathUtils::IsAbsolutePath(sPaths[i])))
      {
        // this is for Linux systems where absolute paths start with a slash, wouldn't want to remove that
      }
      else
      {
        // prevent creating multiple path separators through concatenation
        while (nsPathUtils::IsPathSeparator(*sThisPath.GetStartPointer()))
          sThisPath.ChopAwayFirstCharacterAscii();
      }

      if (IsEmpty() || nsPathUtils::IsPathSeparator(GetIteratorBack().GetCharacter()))
        Append(sThisPath);
      else
        Append("/", sThisPath);
    }
  }
}

void nsStringBuilder::AppendWithSeparator(nsStringView sOptional, nsStringView sText1, nsStringView sText2 /*= nsStringView()*/,
  nsStringView sText3 /*= nsStringView()*/, nsStringView sText4 /*= nsStringView()*/, nsStringView sText5 /*= nsStringView()*/,
  nsStringView sText6 /*= nsStringView()*/)
{
  // if this string already ends with the optional string, reset it to be empty
  if (IsEmpty() || nsStringUtils::EndsWith(GetData(), sOptional.GetStartPointer(), GetData() + GetElementCount(), sOptional.GetEndPointer()))
  {
    sOptional = nsStringView();
  }

  const nsUInt32 uiMaxParams = 7;

  const nsStringView pStrings[uiMaxParams] = {sOptional, sText1, sText2, sText3, sText4, sText5, sText6};
  nsUInt32 uiStrLen[uiMaxParams] = {0};
  nsUInt32 uiMoreBytes = 0;

  // first figure out how much the string has to grow
  for (nsUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (pStrings[i].IsEmpty())
      continue;

    NS_ASSERT_DEBUG(pStrings[i].GetStartPointer() < m_Data.GetData() || pStrings[i].GetStartPointer() >= m_Data.GetData() + m_Data.GetCapacity(),
      "Parameter {0} comes from the string builders own storage. This type assignment is not allowed.", i);

    uiStrLen[i] = pStrings[i].GetElementCount();
    uiMoreBytes += uiStrLen[i];

    NS_ASSERT_DEV(nsUnicodeUtils::IsValidUtf8(pStrings[i].GetStartPointer(), pStrings[i].GetEndPointer()), "Parameter {0} is not a valid Utf8 sequence.", i + 1);
  }

  if (uiMoreBytes == uiStrLen[0])
  {
    // if all other strings (than the separator) are empty, don't append anything
    return;
  }

  nsUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  NS_ASSERT_DEBUG(uiPrevCount > 0, "There should be a 0 terminator somewhere around here.");

  // now resize
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  // and then append all the strings
  for (nsUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (uiStrLen[i] == 0)
      continue;

    // make enough room to copy the entire string, including the T-800
    nsStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen[i] + 1, pStrings[i].GetStartPointer(), pStrings[i].GetStartPointer() + uiStrLen[i]);

    uiPrevCount += uiStrLen[i];
  }
}

void nsStringBuilder::ChangeFileName(nsStringView sNewFileName)
{
  nsStringView it = nsPathUtils::GetFileName(GetView());

  ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewFileName);
}

void nsStringBuilder::ChangeFileNameAndExtension(nsStringView sNewFileNameWithExtension)
{
  nsStringView it = nsPathUtils::GetFileNameAndExtension(GetView());

  ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewFileNameWithExtension);
}

void nsStringBuilder::ChangeFileExtension(nsStringView sNewExtension, bool bFullExtension /*= false*/)
{
  while (sNewExtension.StartsWith("."))
  {
    sNewExtension.ChopAwayFirstCharacterAscii();
  }

  const nsStringView it = nsPathUtils::GetFileExtension(GetView(), bFullExtension);

  if (it.IsEmpty())
  {
    if (!EndsWith("."))
    {
      Append(".", sNewExtension);
    }
    else
    {
      Append(sNewExtension);
    }
  }
  else
  {
    ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewExtension);
  }
}

void nsStringBuilder::RemoveFileExtension(bool bFullExtension /*= false*/)
{
  if (HasAnyExtension())
  {
    ChangeFileExtension("", bFullExtension);
    Shrink(0, 1); // remove the dot
  }
}

nsResult nsStringBuilder::MakeRelativeTo(nsStringView sAbsolutePathToMakeThisRelativeTo)
{
  nsStringBuilder sAbsBase = sAbsolutePathToMakeThisRelativeTo;
  sAbsBase.MakeCleanPath();
  nsStringBuilder sAbsThis = *this;
  sAbsThis.MakeCleanPath();

  if (sAbsBase.IsEqual_NoCase(sAbsThis.GetData()))
  {
    Clear();
    return NS_SUCCESS;
  }

  if (!sAbsBase.EndsWith("/"))
    sAbsBase.Append("/");

  if (!sAbsThis.EndsWith("/"))
  {
    sAbsThis.Append("/");

    if (sAbsBase.StartsWith(sAbsThis.GetData()))
    {
      Clear();
      const char* szStart = &sAbsBase.GetData()[sAbsThis.GetElementCount()];

      while (*szStart != '\0')
      {
        if (*szStart == '/')
          Append("../");

        ++szStart;
      }

      return NS_SUCCESS;
    }
    else
      sAbsThis.Shrink(0, 1);
  }

  const nsUInt32 uiMinLen = nsMath::Min(sAbsBase.GetElementCount(), sAbsThis.GetElementCount());

  nsInt32 iSame = uiMinLen - 1;
  for (; iSame >= 0; --iSame)
  {
    if (sAbsBase.GetData()[iSame] != '/')
      continue;

    // We need to check here if sAbsThis starts with sAbsBase in the range[0, iSame + 1]. However, we can't compare the first N bytes because those might not be a valid utf8 substring in absBase.
    // Thus we can't use IsEqualN_NoCase as N would need to be the number of characters, not bytes. Computing the number of characters in absBase would mean iterating the string twice.
    // As an alternative, as we know [0, iSame + 1] is a valid utf8 string in sAbsBase we can ask whether absThis starts with that substring.
    if (nsStringUtils::StartsWith_NoCase(sAbsThis.GetData(), sAbsBase.GetData(), sAbsThis.GetData() + sAbsThis.GetElementCount(), sAbsBase.GetData() + iSame + 1))
      break;
  }

  if (iSame < 0)
  {
    return NS_FAILURE;
  }

  Clear();

  for (nsUInt32 ui = iSame + 1; ui < sAbsBase.GetElementCount(); ++ui)
  {
    if (sAbsBase.GetData()[ui] == '/')
      Append("../");
  }

  if (sAbsThis.GetData()[iSame] == '/')
    ++iSame;

  Append(&(sAbsThis.GetData()[iSame]));

  return NS_SUCCESS;
}

/// An empty folder (zero length) does not contain ANY files.\n
/// A non-existing file-name (zero length) is never in any folder.\n
/// Example:\n
/// IsFileBelowFolder ("", "XYZ") -> always false\n
/// IsFileBelowFolder ("XYZ", "") -> always false\n
/// IsFileBelowFolder ("", "") -> always false\n
bool nsStringBuilder::IsPathBelowFolder(const char* szPathToFolder)
{
  NS_ASSERT_DEV(!nsStringUtils::IsNullOrEmpty(szPathToFolder), "The given path must not be empty. Because is 'nothing' under the empty path, or 'everything' ?");

  // a non-existing file is never in any folder
  if (IsEmpty())
    return false;

  MakeCleanPath();

  nsStringBuilder sBasePath(szPathToFolder);
  sBasePath.MakeCleanPath();

  if (IsEqual_NoCase(sBasePath.GetData()))
    return true;

  if (!sBasePath.EndsWith("/"))
    sBasePath.Append("/");

  return StartsWith_NoCase(sBasePath.GetData());
}

void nsStringBuilder::MakePathSeparatorsNative()
{
  const char sep = nsPathUtils::OsSpecificPathSeparator;

  MakeCleanPath();
  ReplaceAll("/", nsStringView(&sep, 1));
}

void nsStringBuilder::RemoveDoubleSlashesInPath()
{
  if (IsEmpty())
    return;

  const char* szReadPos = &m_Data[0];
  char* szCurWritePos = &m_Data[0];

  nsInt32 iAllowedSlashes = 2;

  while (*szReadPos != '\0')
  {
    char CurChar = *szReadPos;
    ++szReadPos;

    if (CurChar == '\\')
      CurChar = '/';

    if (CurChar != '/')
      iAllowedSlashes = 1;
    else
    {
      if (iAllowedSlashes > 0)
        --iAllowedSlashes;
      else
        continue;
    }

    *szCurWritePos = CurChar;
    ++szCurWritePos;
  }


  const nsUInt32 uiPrevByteCount = m_Data.GetCount();
  const nsUInt32 uiNewByteCount = (nsUInt32)(szCurWritePos - &m_Data[0]) + 1;

  NS_IGNORE_UNUSED(uiPrevByteCount);
  NS_ASSERT_DEBUG(uiPrevByteCount >= uiNewByteCount, "It should not be possible that a path grows during cleanup. Old: {0} Bytes, New: {1} Bytes",
    uiPrevByteCount, uiNewByteCount);

  // make sure to write the terminating \0 and reset the count
  *szCurWritePos = '\0';
  m_Data.SetCountUninitialized(uiNewByteCount);
}


void nsStringBuilder::ReadAll(nsStreamReader& inout_stream)
{
  Clear();

  nsHybridArray<nsUInt8, 1024 * 4> Bytes(m_Data.GetAllocator());
  nsUInt8 Temp[1024];

  while (true)
  {
    const nsUInt32 uiRead = (nsUInt32)inout_stream.ReadBytes(Temp, 1024);

    if (uiRead == 0)
      break;

    Bytes.PushBackRange(nsArrayPtr<nsUInt8>(Temp, uiRead));
  }

  Bytes.PushBack('\0');

  *this = (const char*)&Bytes[0];
}

void nsStringBuilder::Trim(const char* szTrimChars)
{
  Trim(szTrimChars, szTrimChars);
}

void nsStringBuilder::Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd)
{
  const char* szNewStart = GetData();
  const char* szNewEnd = GetData() + GetElementCount();
  nsStringUtils::Trim(szNewStart, szNewEnd, szTrimCharsStart, szTrimCharsEnd);
  Shrink(nsStringUtils::GetCharacterCount(GetData(), szNewStart), nsStringUtils::GetCharacterCount(szNewEnd, GetData() + GetElementCount()));
}

void nsStringBuilder::TrimLeft(const char* szTrimChars /*= " \f\n\r\t\v"*/)
{
  Trim(szTrimChars, "");
}

void nsStringBuilder::TrimRight(const char* szTrimChars /*= " \f\n\r\t\v"*/)
{
  Trim("", szTrimChars);
}

bool nsStringBuilder::TrimWordStart(nsStringView sWord)
{
  const bool bTrimAll = false;

  bool trimmed = false;

  do
  {
    if (!sWord.IsEmpty() && StartsWith_NoCase(sWord))
    {
      Shrink(nsStringUtils::GetCharacterCount(sWord.GetStartPointer(), sWord.GetEndPointer()), 0);
      trimmed = true;
    }

  } while (bTrimAll);

  return trimmed;
}

bool nsStringBuilder::TrimWordEnd(nsStringView sWord)
{
  const bool bTrimAll = false;

  bool trimmed = false;

  do
  {

    if (!sWord.IsEmpty() && EndsWith_NoCase(sWord))
    {
      Shrink(0, nsStringUtils::GetCharacterCount(sWord.GetStartPointer(), sWord.GetEndPointer()));
      trimmed = true;
    }

  } while (bTrimAll);

  return trimmed;
}

void nsStringBuilder::SetFormat(const nsFormatString& string)
{
  Clear();
  nsStringView sText = string.GetText(*this);

  // this is for the case that GetText does not use the nsStringBuilder as temp storage
  if (sText.GetStartPointer() != GetData())
    *this = sText;
}

void nsStringBuilder::AppendFormat(const nsFormatString& string)
{
  nsStringBuilder tmp;
  nsStringView view = string.GetText(tmp);

  Append(view);
}

void nsStringBuilder::PrependFormat(const nsFormatString& string)
{
  nsStringBuilder tmp;

  Prepend(string.GetText(tmp));
}

void nsStringBuilder::SetPrintf(const char* szUtf8Format, ...)
{
  va_list args;
  va_start(args, szUtf8Format);

  SetPrintfArgs(szUtf8Format, args);

  va_end(args);
}

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)
nsStringBuilder::nsStringBuilder(const std::string_view& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();

  *this = rhs;
}

nsStringBuilder::nsStringBuilder(const std::string& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();

  *this = rhs;
}

void nsStringBuilder::operator=(const std::string_view& rhs)
{
  if (rhs.empty())
  {
    Clear();
  }
  else
  {
    *this = nsStringView(rhs.data(), rhs.data() + rhs.size());
  }
}

void nsStringBuilder::operator=(const std::string& rhs)
{
  if (rhs.empty())
  {
    Clear();
  }
  else
  {
    *this = nsStringView(rhs.data(), rhs.data() + rhs.size());
  }
}

#endif
