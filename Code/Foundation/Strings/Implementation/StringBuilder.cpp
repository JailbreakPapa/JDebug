#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/StringBuilder.h>

#include <stdarg.h>

wdStringBuilder::wdStringBuilder(wdStringView sData1, wdStringView sData2, wdStringView sData3, wdStringView sData4, wdStringView sData5, wdStringView sData6)
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  Append(sData1, sData2, sData3, sData4, sData5, sData6);
}

void wdStringBuilder::Set(wdStringView sData1, wdStringView sData2, wdStringView sData3, wdStringView sData4, wdStringView sData5, wdStringView sData6)
{
  Clear();
  Append(sData1, sData2, sData3, sData4, sData5, sData6);
}

void wdStringBuilder::SetSubString_FromTo(const char* pStart, const char* pEnd)
{
  WD_ASSERT_DEBUG(wdUnicodeUtils::IsValidUtf8(pStart, pEnd), "Invalid substring, the start does not point to a valid Utf-8 character");

  wdStringView view(pStart, pEnd);
  *this = view;
}

void wdStringBuilder::SetSubString_ElementCount(const char* pStart, wdUInt32 uiElementCount)
{
  WD_ASSERT_DEBUG(
    wdUnicodeUtils::IsValidUtf8(pStart, pStart + uiElementCount), "Invalid substring, the start does not point to a valid Utf-8 character");

  wdStringView view(pStart, pStart + uiElementCount);
  *this = view;
}

void wdStringBuilder::SetSubString_CharacterCount(const char* pStart, wdUInt32 uiCharacterCount)
{
  const char* pEnd = pStart;
  wdUnicodeUtils::MoveToNextUtf8(pEnd, uiCharacterCount);

  wdStringView view(pStart, pEnd);
  *this = view;
}

void wdStringBuilder::Append(wdStringView sData1, wdStringView sData2, wdStringView sData3, wdStringView sData4, wdStringView sData5, wdStringView sData6)
{
  // it is not possible to find out how many parameters were passed to a vararg function
  // with a fixed size of parameters we do not need to have a parameter that tells us how many strings will come

  const wdUInt32 uiMaxParams = 6;

  const wdStringView pStrings[uiMaxParams] = {sData1, sData2, sData3, sData4, sData5, sData6};
  wdUInt32 uiStrLen[uiMaxParams] = {0};

  wdUInt32 uiMoreBytes = 0;

  // first figure out how much the string has to grow
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (pStrings[i].IsEmpty())
      continue;

    WD_ASSERT_DEBUG(pStrings[i].GetStartPointer() < m_Data.GetData() || pStrings[i].GetStartPointer() >= m_Data.GetData() + m_Data.GetCapacity(),
      "Parameter {0} comes from the string builders own storage. This type assignment is not allowed.", i);

    wdUInt32 uiCharacters = 0;
    wdStringUtils::GetCharacterAndElementCount(pStrings[i].GetStartPointer(), uiCharacters, uiStrLen[i], pStrings[i].GetEndPointer());
    uiMoreBytes += uiStrLen[i];
    m_uiCharacterCount += uiCharacters;

    WD_ASSERT_DEBUG(wdUnicodeUtils::IsValidUtf8(pStrings[i].GetStartPointer(), pStrings[i].GetEndPointer()), "Parameter {0} is not a valid Utf8 sequence.", i + 1);
  }

  wdUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  WD_ASSERT_DEBUG(uiPrevCount > 0, "There should be a 0 terminator somewhere around here.");

  // now resize
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  // and then append all the strings
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (uiStrLen[i] == 0)
      continue;

    // make enough room to copy the entire string, including the T-800
    wdStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen[i] + 1, pStrings[i].GetStartPointer(), pStrings[i].GetStartPointer() + uiStrLen[i]);

    uiPrevCount += uiStrLen[i];
  }
}

void wdStringBuilder::Prepend(wdStringView sData1, wdStringView sData2, wdStringView sData3, wdStringView sData4, wdStringView sData5, wdStringView sData6)
{
  // it is not possible to find out how many parameters were passed to a vararg function
  // with a fixed size of parameters we do not need to have a parameter that tells us how many strings will come

  const wdUInt32 uiMaxParams = 6;

  const wdStringView pStrings[uiMaxParams] = {sData1, sData2, sData3, sData4, sData5, sData6};
  wdUInt32 uiStrLen[uiMaxParams] = {0};

  wdUInt32 uiMoreBytes = 0;

  // first figure out how much the string has to grow
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (pStrings[i].IsEmpty())
      continue;

    wdUInt32 uiCharacters = 0;
    wdStringUtils::GetCharacterAndElementCount(pStrings[i].GetStartPointer(), uiCharacters, uiStrLen[i], pStrings[i].GetEndPointer());
    uiMoreBytes += uiStrLen[i];
    m_uiCharacterCount += uiCharacters;

    WD_ASSERT_DEBUG(wdUnicodeUtils::IsValidUtf8(pStrings[i].GetStartPointer(), pStrings[i].GetEndPointer()), "Parameter {0} is not a valid Utf8 sequence.", i + 1);
  }

  wdUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  WD_ASSERT_DEBUG(uiPrevCount > 0, "There should be a 0 terminator somewhere around here.");

  // now resize
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  // move the previous string data at the end
  wdMemoryUtils::CopyOverlapped(&m_Data[0] + uiMoreBytes, GetData(), uiPrevCount);

  wdUInt32 uiWritePos = 0;

  // and then prepend all the strings
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (uiStrLen[i] == 0)
      continue;

    // make enough room to copy the entire string, including the T-800
    wdMemoryUtils::Copy(&m_Data[uiWritePos], pStrings[i].GetStartPointer(), uiStrLen[i]);

    uiWritePos += uiStrLen[i];
  }
}

void wdStringBuilder::PrintfArgs(const char* szUtf8Format, va_list szArgs0)
{
  va_list args;
  va_copy(args, szArgs0);

  Clear();

  const wdUInt32 TempBuffer = 4096;

  char szTemp[TempBuffer];
  const wdInt32 iCount = wdStringUtils::vsnprintf(szTemp, TempBuffer - 1, szUtf8Format, args);

  WD_ASSERT_DEV(iCount != -1, "There was an error while formatting the string. Probably and unescaped usage of the %% sign.");

  if (iCount == -1)
  {
    va_end(args);
    return;
  }

  if (iCount > TempBuffer - 1)
  {
    wdDynamicArray<char> Temp;
    Temp.SetCountUninitialized(iCount + 1);

    wdStringUtils::vsnprintf(&Temp[0], iCount + 1, szUtf8Format, args);

    Append(&Temp[0]);
  }
  else
  {
    Append(&szTemp[0]);
  }

  va_end(args);
}

void wdStringBuilder::ChangeCharacterNonASCII(iterator& it, wdUInt32 uiCharacter)
{
  char* pPos = const_cast<char*>(it.GetData()); // yes, I know...

  const wdUInt32 uiOldCharLength = wdUnicodeUtils::GetUtf8SequenceLength(*pPos);
  const wdUInt32 uiNewCharLength = wdUnicodeUtils::GetSizeForCharacterInUtf8(uiCharacter);

  // if the old character and the new one are encoded with the same length, we can replace the character in-place
  if (uiNewCharLength == uiOldCharLength)
  {
    // just overwrite all characters at the given position with the new Utf8 string
    wdUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);

    // if the encoding length is identical, this will also handle all ASCII strings
    // if the string was pure ASCII before, this won't change, so no need to update that state
    return;
  }

  // in this case we can still update the string without reallocation, but the tail of the string has to be moved forwards
  if (uiNewCharLength < uiOldCharLength)
  {
    // just overwrite all characters at the given position with the new Utf8 string
    wdUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);

    // pPos will be changed (moved forwards) to the next character position

    // how much has changed
    const wdUInt32 uiDifference = uiOldCharLength - uiNewCharLength;
    const wdUInt32 uiTrailStringBytes = (wdUInt32)(GetData() + GetElementCount() - it.GetData() - uiOldCharLength + 1); // ???

    // move the trailing characters forwards
    wdMemoryUtils::CopyOverlapped(pPos, pPos + uiDifference, uiTrailStringBytes);

    // update the data array
    m_Data.PopBack(uiDifference);

    // 'It' references this already, no need to change anything.
  }
  else
  {
    // in this case we insert a character that is longer int Utf8 encoding than the character that already exists there *sigh*
    // so we must first move the trailing string backwards to make room, then we can write the new char in there

    // how much has changed
    const wdUInt32 uiDifference = uiNewCharLength - uiOldCharLength;
    const wdUInt32 uiTrailStringBytes = (wdUInt32)(GetData() + GetElementCount() - it.GetData() - uiOldCharLength + 1);
    auto iCurrentPos = (it.GetData() - GetData());
    // resize the array
    m_Data.SetCountUninitialized(m_Data.GetCount() + uiDifference);

    // these might have changed (array realloc)
    pPos = &m_Data[0] + iCurrentPos;
    it.SetCurrentPosition(pPos);

    // move the trailing string backwards
    wdMemoryUtils::CopyOverlapped(pPos + uiNewCharLength, pPos + uiOldCharLength, uiTrailStringBytes);

    // just overwrite all characters at the given position with the new Utf8 string
    wdUnicodeUtils::EncodeUtf32ToUtf8(uiCharacter, pPos);
  }
}

void wdStringBuilder::Shrink(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack)
{
  if (uiShrinkCharsFront + uiShrinkCharsBack >= m_uiCharacterCount)
  {
    Clear();
    return;
  }

  const char* szNewStart = &m_Data[0];

  if (IsPureASCII())
  {
    if (uiShrinkCharsBack > 0)
    {
      m_Data.PopBack(uiShrinkCharsBack + 1);
      AppendTerminator();
    }

    szNewStart = &m_Data[uiShrinkCharsFront];
  }
  else
  {
    if (uiShrinkCharsBack > 0)
    {
      const char* szEnd = GetData() + GetElementCount();
      const char* szNewEnd = szEnd;
      wdUnicodeUtils::MoveToPriorUtf8(szNewEnd, uiShrinkCharsBack);

      const wdUInt32 uiLessBytes = (wdUInt32)(szEnd - szNewEnd);

      m_Data.PopBack(uiLessBytes + 1);
      AppendTerminator();
    }

    wdUnicodeUtils::MoveToNextUtf8(szNewStart, uiShrinkCharsFront);
  }

  if (szNewStart > &m_Data[0])
  {
    const wdUInt32 uiLessBytes = (wdUInt32)(szNewStart - &m_Data[0]);

    wdMemoryUtils::CopyOverlapped(&m_Data[0], szNewStart, m_Data.GetCount() - uiLessBytes);
    m_Data.PopBack(uiLessBytes);
  }

  m_uiCharacterCount -= uiShrinkCharsFront;
  m_uiCharacterCount -= uiShrinkCharsBack;
}

void wdStringBuilder::ReplaceSubString(const char* szStartPos, const char* szEndPos, wdStringView sReplaceWith)
{
  WD_ASSERT_DEV(wdMath::IsInRange(szStartPos, GetData(), GetData() + m_Data.GetCount()), "szStartPos is not inside this string.");
  WD_ASSERT_DEV(wdMath::IsInRange(szEndPos, GetData(), GetData() + m_Data.GetCount()), "szEndPos is not inside this string.");
  WD_ASSERT_DEV(szStartPos <= szEndPos, "wdStartPos must be before wdEndPos");

  wdUInt32 uiWordChars = 0;
  wdUInt32 uiWordBytes = 0;
  wdStringUtils::GetCharacterAndElementCount(sReplaceWith.GetStartPointer(), uiWordChars, uiWordBytes, sReplaceWith.GetEndPointer());

  const wdUInt32 uiSubStringBytes = (wdUInt32)(szEndPos - szStartPos);

  char* szWritePos = const_cast<char*>(szStartPos); // szStartPos points into our own data anyway
  const char* szReadPos = sReplaceWith.GetStartPointer();

  // most simple case, just replace characters
  if (uiSubStringBytes == uiWordBytes)
  {
    while (szWritePos < szEndPos)
    {
      if (!wdUnicodeUtils::IsUtf8ContinuationByte(*szWritePos))
        --m_uiCharacterCount;

      *szWritePos = *szReadPos;
      ++szWritePos;
      ++szReadPos;
    }

    // the number of bytes might be identical, but that does not mean that the number of characters is also identical
    // therefore we subtract the number of characters that were found in the old substring
    // and add the number of characters for the new substring
    m_uiCharacterCount += uiWordChars;
    return;
  }

  // the replacement is shorter than the existing stuff -> move characters to the left, no reallocation needed
  if (uiWordBytes < uiSubStringBytes)
  {
    m_uiCharacterCount -= wdStringUtils::GetCharacterCount(szStartPos, szEndPos);
    m_uiCharacterCount += uiWordChars;

    // first copy the replacement to the correct position
    wdMemoryUtils::Copy(szWritePos, sReplaceWith.GetStartPointer(), uiWordBytes);

    const wdUInt32 uiDifference = uiSubStringBytes - uiWordBytes;

    const char* szStringEnd = GetData() + m_Data.GetCount();

    // now move all the characters from behind the replaced string to the correct position
    wdMemoryUtils::CopyOverlapped(szWritePos + uiWordBytes, szWritePos + uiSubStringBytes, szStringEnd - (szWritePos + uiSubStringBytes));

    m_Data.PopBack(uiDifference);

    return;
  }

  // else the replacement is longer than the existing word
  {
    m_uiCharacterCount -= wdStringUtils::GetCharacterCount(szStartPos, szEndPos);
    m_uiCharacterCount += uiWordChars;

    const wdUInt32 uiDifference = uiWordBytes - uiSubStringBytes;
    const wdUInt64 uiRelativeWritePosition = szWritePos - GetData();
    const wdUInt64 uiDataByteCountBefore = m_Data.GetCount();

    m_Data.SetCountUninitialized(m_Data.GetCount() + uiDifference);

    // all pointer are now possibly invalid since the data may be reallocated!
    szWritePos = const_cast<char*>(GetData()) + uiRelativeWritePosition;
    const char* szStringEnd = GetData() + uiDataByteCountBefore;

    // first move the characters to the proper position from back to front
    wdMemoryUtils::CopyOverlapped(szWritePos + uiWordBytes, szWritePos + uiSubStringBytes, szStringEnd - (szWritePos + uiSubStringBytes));

    // now copy the replacement to the correct position
    wdMemoryUtils::Copy(szWritePos, sReplaceWith.GetStartPointer(), uiWordBytes);
  }
}

const char* wdStringBuilder::ReplaceFirst(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData();
  else
  {
    WD_ASSERT_DEV(wdMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = wdStringUtils::FindSubString(szStartSearchAt, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = (wdUInt32)(szFoundAt - GetData());

  const wdUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

const char* wdStringBuilder::ReplaceLast(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData() + m_Data.GetCount() - 1;
  else
  {
    WD_ASSERT_DEV(wdMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = wdStringUtils::FindLastSubString(GetData(), sSearchFor.GetStartPointer(), szStartSearchAt, GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = (wdUInt32)(szFoundAt - GetData());

  const wdUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

wdUInt32 wdStringBuilder::ReplaceAll(wdStringView sSearchFor, wdStringView sReplacement)
{
  const wdUInt32 uiSearchBytes = sSearchFor.GetElementCount();
  const wdUInt32 uiWordBytes = sReplacement.GetElementCount();

  wdUInt32 uiReplacements = 0;
  wdUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = wdStringUtils::FindSubString(GetData() + uiOffset, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<wdUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplacement);

    ++uiReplacements;
  }

  return uiReplacements;
}


const char* wdStringBuilder::ReplaceFirst_NoCase(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData();
  else
  {
    WD_ASSERT_DEV(wdMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = wdStringUtils::FindSubString_NoCase(szStartSearchAt, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = (wdUInt32)(szFoundAt - GetData());

  const wdUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

const char* wdStringBuilder::ReplaceLast_NoCase(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt)
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetData() + m_Data.GetCount() - 1;
  else
  {
    WD_ASSERT_DEV(wdMath::IsInRange(szStartSearchAt, GetData(), GetData() + m_Data.GetCount() - 1), "szStartSearchAt is not inside the string range.");
  }

  const char* szFoundAt = wdStringUtils::FindLastSubString_NoCase(GetData(), sSearchFor.GetStartPointer(), szStartSearchAt, GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

  if (szFoundAt == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = (wdUInt32)(szFoundAt - GetData());

  const wdUInt32 uiSearchStrLength = sSearchFor.GetElementCount();

  ReplaceSubString(szFoundAt, szFoundAt + uiSearchStrLength, sReplacement);

  return GetData() + uiOffset; // memory might have been reallocated
}

wdUInt32 wdStringBuilder::ReplaceAll_NoCase(wdStringView sSearchFor, wdStringView sReplacement)
{
  const wdUInt32 uiSearchBytes = sSearchFor.GetElementCount();
  const wdUInt32 uiWordBytes = sReplacement.GetElementCount();

  wdUInt32 uiReplacements = 0;
  wdUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = wdStringUtils::FindSubString_NoCase(GetData() + uiOffset, sSearchFor.GetStartPointer(), GetData() + m_Data.GetCount() - 1, sSearchFor.GetEndPointer());

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<wdUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplacement);

    ++uiReplacements;
  }

  return uiReplacements;
}

const char* wdStringBuilder::ReplaceWholeWord(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB)
{
  const char* szPos = FindWholeWord(szSearchFor, isDelimiterCB);

  if (szPos == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = static_cast<wdUInt32>(szPos - GetData());

  ReplaceSubString(szPos, szPos + wdStringUtils::GetStringElementCount(szSearchFor), sReplaceWith);
  return GetData() + uiOffset;
}

const char* wdStringBuilder::ReplaceWholeWord_NoCase(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB)
{
  const char* szPos = FindWholeWord_NoCase(szSearchFor, isDelimiterCB);

  if (szPos == nullptr)
    return nullptr;

  const wdUInt32 uiOffset = static_cast<wdUInt32>(szPos - GetData());

  ReplaceSubString(szPos, szPos + wdStringUtils::GetStringElementCount(szSearchFor), sReplaceWith);
  return GetData() + uiOffset;
}


wdUInt32 wdStringBuilder::ReplaceWholeWordAll(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB)
{
  const wdUInt32 uiSearchBytes = wdStringUtils::GetStringElementCount(szSearchFor);
  const wdUInt32 uiWordBytes = wdStringUtils::GetStringElementCount(sReplaceWith.GetStartPointer(), sReplaceWith.GetEndPointer());

  wdUInt32 uiReplacements = 0;
  wdUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = wdStringUtils::FindWholeWord(GetData() + uiOffset, szSearchFor, isDelimiterCB, GetData() + m_Data.GetCount() - 1);

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<wdUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplaceWith);

    ++uiReplacements;
  }

  return uiReplacements;
}

wdUInt32 wdStringBuilder::ReplaceWholeWordAll_NoCase(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB)
{
  const wdUInt32 uiSearchBytes = wdStringUtils::GetStringElementCount(szSearchFor);
  const wdUInt32 uiWordBytes = wdStringUtils::GetStringElementCount(sReplaceWith.GetStartPointer(), sReplaceWith.GetEndPointer());

  wdUInt32 uiReplacements = 0;
  wdUInt32 uiOffset = 0;

  while (true)
  {
    // during ReplaceSubString the string data might get reallocated and the memory addresses do not stay valid
    // so we need to work with offsets and recompute the pointers every time
    const char* szFoundAt = wdStringUtils::FindWholeWord_NoCase(GetData() + uiOffset, szSearchFor, isDelimiterCB, GetData() + m_Data.GetCount() - 1);

    if (szFoundAt == nullptr)
      return uiReplacements;

    // do not search withing the replaced part, otherwise we get recursive replacement which will not end
    uiOffset = static_cast<wdUInt32>(szFoundAt - GetData()) + uiWordBytes;

    ReplaceSubString(szFoundAt, szFoundAt + uiSearchBytes, sReplaceWith);

    ++uiReplacements;
  }

  return uiReplacements;
}

void wdStringBuilder::operator=(wdStringView rhs)
{
  wdUInt32 uiBytes;
  wdUInt32 uiCharacters;

  wdStringUtils::GetCharacterAndElementCount(rhs.GetStartPointer(), uiCharacters, uiBytes, rhs.GetEndPointer());

  // if we need more room, allocate up front (rhs cannot use our own data in this case)
  if (uiBytes + 1 > m_Data.GetCount())
    m_Data.SetCountUninitialized(uiBytes + 1);

  // the data might actually come from our very own string, so we 'move' the memory in there, just to be safe
  // if it comes from our own array, the data will always be a sub-set -> smaller than this array
  // in this case we defer the SetCount till later, to ensure that the data is not corrupted (destructed) before we copy it
  // however, when the new data is larger than the old, it cannot be from our own data, so we can (and must) reallocate before copying
  wdMemoryUtils::CopyOverlapped(&m_Data[0], rhs.GetStartPointer(), uiBytes);

  m_Data.SetCountUninitialized(uiBytes + 1);
  m_Data[uiBytes] = '\0';

  m_uiCharacterCount = uiCharacters;
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

void wdStringBuilder::MakeCleanPath()
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

  wdInt32 iLevelsDown = 0;
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
    else if (wdPathUtils::IsPathSeparator(CurChar))
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
        WD_ASSERT_DEBUG(writeOffset >= 3, "invalid write offset");
        writeOffset -= 3; // go back, skip two dots, one slash

        while ((writeOffset > 0) && (szCurWritePos[writeOffset - 1] != '/'))
        {
          WD_ASSERT_DEBUG(writeOffset > 0, "invalid write offset");
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
      WD_ASSERT_DEBUG(writeOffset > 0, "invalid write offset");
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

  const wdUInt32 uiPrevByteCount = m_Data.GetCount();
  const wdUInt32 uiNewByteCount = (wdUInt32)(writeOffset) + 1;

  WD_ASSERT_DEBUG(uiPrevByteCount >= uiNewByteCount, "It should not be possible that a path grows during cleanup. Old: {0} Bytes, New: {1} Bytes",
    uiPrevByteCount, uiNewByteCount);

  // we will only remove characters and only ASCII ones (slash, backslash, dot)
  // so the number of characters shrinks equally to the number of bytes
  m_uiCharacterCount -= (uiPrevByteCount - uiNewByteCount);

  // make sure to write the terminating \0 and reset the count
  szCurWritePos[writeOffset] = '\0';
  m_Data.SetCountUninitialized(uiNewByteCount);
}

void wdStringBuilder::PathParentDirectory(wdUInt32 uiLevelsUp)
{
  WD_ASSERT_DEV(uiLevelsUp > 0, "We have to do something!");

  for (wdUInt32 i = 0; i < uiLevelsUp; ++i)
    AppendPath("../");

  MakeCleanPath();
}

void wdStringBuilder::AppendPath(wdStringView sPath1, wdStringView sPath2, wdStringView sPath3, wdStringView sPath4)
{
  const wdStringView sPaths[4] = {sPath1, sPath2, sPath3, sPath4};

  for (wdUInt32 i = 0; i < 4; ++i)
  {
    wdStringView sThisPath = sPaths[i];

    if (!sThisPath.IsEmpty())
    {
      if ((IsEmpty() && wdPathUtils::IsAbsolutePath(sPaths[i])))
      {
        // this is for Linux systems where absolute paths start with a slash, wouldn't want to remove that
      }
      else
      {
        // prevent creating multiple path separators through concatenation
        while (wdPathUtils::IsPathSeparator(*sThisPath.GetStartPointer()))
          sThisPath.ChopAwayFirstCharacterAscii();
      }

      if (IsEmpty() || wdPathUtils::IsPathSeparator(GetIteratorBack().GetCharacter()))
        Append(sThisPath);
      else
        Append("/", sThisPath);
    }
  }
}

void wdStringBuilder::AppendWithSeparator(wdStringView sOptional, wdStringView sText1, wdStringView sText2 /*= wdStringView()*/,
  wdStringView sText3 /*= wdStringView()*/, wdStringView sText4 /*= wdStringView()*/, wdStringView sText5 /*= wdStringView()*/,
  wdStringView sText6 /*= wdStringView()*/)
{
  // if this string already ends with the optional string, reset it to be empty
  if (IsEmpty() || wdStringUtils::EndsWith(GetData(), sOptional.GetStartPointer(), GetData() + GetElementCount(), sOptional.GetEndPointer()))
  {
    sOptional = wdStringView();
  }

  const wdUInt32 uiMaxParams = 7;

  const wdStringView pStrings[uiMaxParams] = {sOptional, sText1, sText2, sText3, sText4, sText5, sText6};
  wdUInt32 uiStrLen[uiMaxParams] = {0};
  wdUInt32 uiMoreBytes = 0;

  // first figure out how much the string has to grow
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (pStrings[i].IsEmpty())
      continue;

    WD_ASSERT_DEBUG(pStrings[i].GetStartPointer() < m_Data.GetData() || pStrings[i].GetStartPointer() >= m_Data.GetData() + m_Data.GetCapacity(),
      "Parameter {0} comes from the string builders own storage. This type assignment is not allowed.", i);

    wdUInt32 uiCharacters = 0;
    wdStringUtils::GetCharacterAndElementCount(pStrings[i].GetStartPointer(), uiCharacters, uiStrLen[i], pStrings[i].GetEndPointer());
    uiMoreBytes += uiStrLen[i];
    m_uiCharacterCount += uiCharacters;

    WD_ASSERT_DEV(wdUnicodeUtils::IsValidUtf8(pStrings[i].GetStartPointer(), pStrings[i].GetEndPointer()), "Parameter {0} is not a valid Utf8 sequence.", i + 1);
  }

  wdUInt32 uiPrevCount = m_Data.GetCount(); // already contains a 0 terminator
  WD_ASSERT_DEBUG(uiPrevCount > 0, "There should be a 0 terminator somewhere around here.");

  // now resize
  m_Data.SetCountUninitialized(uiPrevCount + uiMoreBytes);

  // and then append all the strings
  for (wdUInt32 i = 0; i < uiMaxParams; ++i)
  {
    if (uiStrLen[i] == 0)
      continue;

    // make enough room to copy the entire string, including the T-800
    wdStringUtils::Copy(&m_Data[uiPrevCount - 1], uiStrLen[i] + 1, pStrings[i].GetStartPointer(), pStrings[i].GetStartPointer() + uiStrLen[i]);

    uiPrevCount += uiStrLen[i];
  }
}

void wdStringBuilder::ChangeFileName(wdStringView sNewFileName)
{
  wdStringView it = wdPathUtils::GetFileName(GetView());

  ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewFileName);
}

void wdStringBuilder::ChangeFileNameAndExtension(wdStringView sNewFileNameWithExtension)
{
  wdStringView it = wdPathUtils::GetFileNameAndExtension(GetView());

  ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewFileNameWithExtension);
}

void wdStringBuilder::ChangeFileExtension(wdStringView sNewExtension)
{
  while (sNewExtension.StartsWith("."))
  {
    sNewExtension.ChopAwayFirstCharacterAscii();
  }

  const wdStringView it = wdPathUtils::GetFileExtension(GetView());

  if (it.IsEmpty() && !EndsWith("."))
    Append(".", sNewExtension);
  else
    ReplaceSubString(it.GetStartPointer(), it.GetEndPointer(), sNewExtension);
}

void wdStringBuilder::RemoveFileExtension()
{
  if (HasAnyExtension())
  {
    ChangeFileExtension("");
    Shrink(0, 1); // remove the dot
  }
}

wdResult wdStringBuilder::MakeRelativeTo(wdStringView sAbsolutePathToMakeThisRelativeTo)
{
  wdStringBuilder sAbsBase = sAbsolutePathToMakeThisRelativeTo;
  sAbsBase.MakeCleanPath();
  wdStringBuilder sAbsThis = *this;
  sAbsThis.MakeCleanPath();

  if (sAbsBase.IsEqual_NoCase(sAbsThis.GetData()))
  {
    Clear();
    return WD_SUCCESS;
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

      return WD_SUCCESS;
    }
    else
      sAbsThis.Shrink(0, 1);
  }

  const wdUInt32 uiMinLen = wdMath::Min(sAbsBase.GetElementCount(), sAbsThis.GetElementCount());

  wdInt32 iSame = uiMinLen - 1;
  for (; iSame >= 0; --iSame)
  {
    if (sAbsBase.GetData()[iSame] != '/')
      continue;

    // We need to check here if sAbsThis starts with sAbsBase in the range[0, iSame + 1]. However, we can't compare the first N bytes because those might not be a valid utf8 substring in absBase.
    // Thus we can't use IsEqualN_NoCase as N would need to be the number of characters, not bytes. Computing the number of characters in absBase would mean iterating the string twice.
    // As an alternative, as we know [0, iSame + 1] is a valid utf8 string in sAbsBase we can ask whether absThis starts with that substring.
    if (wdStringUtils::StartsWith_NoCase(sAbsThis.GetData(), sAbsBase.GetData(), sAbsThis.GetData() + sAbsThis.GetElementCount(), sAbsBase.GetData() + iSame + 1))
      break;
  }

  if (iSame < 0)
  {
    return WD_FAILURE;
  }

  Clear();

  for (wdUInt32 ui = iSame + 1; ui < sAbsBase.GetElementCount(); ++ui)
  {
    if (sAbsBase.GetData()[ui] == '/')
      Append("../");
  }

  if (sAbsThis.GetData()[iSame] == '/')
    ++iSame;

  Append(&(sAbsThis.GetData()[iSame]));

  return WD_SUCCESS;
}

/// An empty folder (zero length) does not contain ANY files.\n
/// A non-existing file-name (zero length) is never in any folder.\n
/// Example:\n
/// IsFileBelowFolder ("", "XYZ") -> always false\n
/// IsFileBelowFolder ("XYZ", "") -> always false\n
/// IsFileBelowFolder ("", "") -> always false\n
bool wdStringBuilder::IsPathBelowFolder(const char* szPathToFolder)
{
  WD_ASSERT_DEV(!wdStringUtils::IsNullOrEmpty(szPathToFolder), "The given path must not be empty. Because is 'nothing' under the empty path, or 'everything' ?");

  // a non-existing file is never in any folder
  if (IsEmpty())
    return false;

  MakeCleanPath();

  wdStringBuilder sBasePath(szPathToFolder);
  sBasePath.MakeCleanPath();

  if (IsEqual_NoCase(sBasePath.GetData()))
    return true;

  if (!sBasePath.EndsWith("/"))
    sBasePath.Append("/");

  return StartsWith_NoCase(sBasePath.GetData());
}

void wdStringBuilder::MakePathSeparatorsNative()
{
  const char sep = wdPathUtils::OsSpecificPathSeparator;

  MakeCleanPath();
  ReplaceAll("/", wdStringView(&sep, 1));
}

void wdStringBuilder::RemoveDoubleSlashesInPath()
{
  if (IsEmpty())
    return;

  const char* szReadPos = &m_Data[0];
  char* szCurWritePos = &m_Data[0];

  wdInt32 iAllowedSlashes = 2;

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


  const wdUInt32 uiPrevByteCount = m_Data.GetCount();
  const wdUInt32 uiNewByteCount = (wdUInt32)(szCurWritePos - &m_Data[0]) + 1;

  WD_ASSERT_DEBUG(uiPrevByteCount >= uiNewByteCount, "It should not be possible that a path grows during cleanup. Old: {0} Bytes, New: {1} Bytes",
    uiPrevByteCount, uiNewByteCount);

  // we will only remove characters and only ASCII ones (slash, backslash)
  // so the number of characters shrinks equally to the number of bytes
  m_uiCharacterCount -= (uiPrevByteCount - uiNewByteCount);

  // make sure to write the terminating \0 and reset the count
  *szCurWritePos = '\0';
  m_Data.SetCountUninitialized(uiNewByteCount);
}


void wdStringBuilder::ReadAll(wdStreamReader& inout_stream)
{
  Clear();

  wdHybridArray<wdUInt8, 1024 * 4> Bytes(m_Data.GetAllocator());
  wdUInt8 Temp[1024];

  while (true)
  {
    const wdUInt32 uiRead = (wdUInt32)inout_stream.ReadBytes(Temp, 1024);

    if (uiRead == 0)
      break;

    Bytes.PushBackRange(wdArrayPtr<wdUInt8>(Temp, uiRead));
  }

  Bytes.PushBack('\0');

  *this = (const char*)&Bytes[0];
}

void wdStringBuilder::Trim(const char* szTrimChars)
{
  return Trim(szTrimChars, szTrimChars);
}

void wdStringBuilder::Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd)
{
  const char* szNewStart = GetData();
  const char* szNewEnd = GetData() + GetElementCount();
  wdStringUtils::Trim(szNewStart, szNewEnd, szTrimCharsStart, szTrimCharsEnd);
  Shrink(wdStringUtils::GetCharacterCount(GetData(), szNewStart), wdStringUtils::GetCharacterCount(szNewEnd, GetData() + GetElementCount()));
}

bool wdStringBuilder::TrimWordStart(const char* szWord1, const char* szWord2 /*= nullptr*/, const char* szWord3 /*= nullptr*/,
  const char* szWord4 /*= nullptr*/, const char* szWord5 /*= nullptr*/)
{
  /// \test TrimWordStart
  bool trimmed = false;

  while (true)
  {
    if (!wdStringUtils::IsNullOrEmpty(szWord1) && StartsWith_NoCase(szWord1))
    {
      Shrink(wdStringUtils::GetCharacterCount(szWord1), 0);
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord2) && StartsWith_NoCase(szWord2))
    {
      Shrink(wdStringUtils::GetCharacterCount(szWord2), 0);
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord3) && StartsWith_NoCase(szWord3))
    {
      Shrink(wdStringUtils::GetCharacterCount(szWord3), 0);
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord4) && StartsWith_NoCase(szWord4))
    {
      Shrink(wdStringUtils::GetCharacterCount(szWord4), 0);
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord5) && StartsWith_NoCase(szWord5))
    {
      Shrink(wdStringUtils::GetCharacterCount(szWord5), 0);
      trimmed = true;
      continue;
    }

    return trimmed;
  }
}

bool wdStringBuilder::TrimWordEnd(const char* szWord1, const char* szWord2 /*= nullptr*/, const char* szWord3 /*= nullptr*/,
  const char* szWord4 /*= nullptr*/, const char* szWord5 /*= nullptr*/)
{
  /// \test TrimWordEnd

  bool trimmed = false;

  while (true)
  {

    if (!wdStringUtils::IsNullOrEmpty(szWord1) && EndsWith_NoCase(szWord1))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(szWord1));
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord2) && EndsWith_NoCase(szWord2))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(szWord2));
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord3) && EndsWith_NoCase(szWord3))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(szWord3));
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord4) && EndsWith_NoCase(szWord4))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(szWord4));
      trimmed = true;
      continue;
    }

    if (!wdStringUtils::IsNullOrEmpty(szWord5) && EndsWith_NoCase(szWord5))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(szWord5));
      trimmed = true;
      continue;
    }

    return trimmed;
  }
}

void wdStringBuilder::Format(const wdFormatString& string)
{
  Clear();
  const char* szText = string.GetText(*this);

  // this is for the case that GetText does not use the wdStringBuilder as temp storage
  if (szText != GetData())
    *this = szText;
}

void wdStringBuilder::AppendFormat(const wdFormatString& string)
{
  wdStringBuilder tmp;
  wdStringView view = string.GetText(tmp);

  Append(view);
}

void wdStringBuilder::PrependFormat(const wdFormatString& string)
{
  wdStringBuilder tmp;

  Prepend(string.GetText(tmp));
}

void wdStringBuilder::Printf(const char* szUtf8Format, ...)
{
  va_list args;
  va_start(args, szUtf8Format);

  PrintfArgs(szUtf8Format, args);

  va_end(args);
}

WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_StringBuilder);
