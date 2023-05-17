#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Tokenizer.h>
#include <Foundation/Memory/CommonAllocators.h>

const char* wdTokenType::EnumNames[wdTokenType::ENUM_COUNT] = {
  "Unknown",
  "Whitespace",
  "Identifier",
  "NonIdentifier",
  "Newline",
  "LineComment",
  "BlockComment",
  "String1",
  "String2",
  "Integer",
  "Float",
};

namespace
{
  // This allocator is used to get rid of some of the memory allocation tracking
  // that would otherwise occur for allocations made by the tokenizer.
  thread_local wdAllocator<wdMemoryPolicies::wdHeapAllocation, wdMemoryTrackingFlags::None> s_ClassAllocator("wdTokenizer", wdFoundation::GetDefaultAllocator());
} // namespace


wdTokenizer::wdTokenizer(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator != nullptr ? pAllocator : &s_ClassAllocator)
  , m_Tokens(pAllocator != nullptr ? pAllocator : &s_ClassAllocator)
{
}

wdTokenizer::~wdTokenizer() = default;

void wdTokenizer::NextChar()
{
  m_uiCurChar = m_uiNextChar;
  m_szCurCharStart = m_szNextCharStart;
  ++m_uiCurColumn;

  if (m_uiCurChar == '\n')
  {
    ++m_uiCurLine;
    m_uiCurColumn = 0;
  }

  if (!m_sIterator.IsValid())
  {
    m_szNextCharStart = m_sIterator.GetEndPointer();
    m_uiNextChar = '\0';
    return;
  }

  m_uiNextChar = m_sIterator.GetCharacter();
  m_szNextCharStart = m_sIterator.GetStartPointer();

  ++m_sIterator;
}

void wdTokenizer::AddToken()
{
  const char* szEnd = m_szCurCharStart;

  wdToken t;
  t.m_uiLine = m_uiLastLine;
  t.m_uiColumn = m_uiLastColumn;
  t.m_iType = m_CurMode;
  t.m_DataView = wdStringView(m_szTokenStart, szEnd);

  m_uiLastLine = m_uiCurLine;
  m_uiLastColumn = m_uiCurColumn;

  m_Tokens.PushBack(t);

  m_szTokenStart = szEnd;

  m_CurMode = wdTokenType::Unknown;
}

void wdTokenizer::Tokenize(wdArrayPtr<const wdUInt8> data, wdLogInterface* pLog)
{
  if (data.GetCount() >= 3)
  {
    const char* dataStart = reinterpret_cast<const char*>(data.GetPtr());

    if (wdUnicodeUtils::SkipUtf8Bom(dataStart))
    {
      wdLog::Error(pLog, "Data to tokenize contains a Utf-8 BOM.");

      // although the tokenizer should get data without a BOM, it's easy enough to work around that here
      // that's what the tokenizer does in other error cases as well - complain, but continue
      data = wdArrayPtr<const wdUInt8>((const wdUInt8*)dataStart, data.GetCount() - 3);
    }
  }

  m_Data.Clear();
  m_Data.Reserve(m_Data.GetCount() + 1);
  m_Data = data;

  if (m_Data.IsEmpty() || m_Data[m_Data.GetCount() - 1] != 0)
    m_Data.PushBack('\0'); // make sure the string is zero terminated

  m_Tokens.Clear();
  m_pLog = pLog;

  {
    m_CurMode = wdTokenType::Unknown;
    m_uiCurLine = 1;
    m_uiCurColumn = -1;
    m_uiCurChar = '\0';
    m_uiNextChar = '\0';
    m_uiLastLine = 1;
    m_uiLastColumn = 1;

    m_szCurCharStart = nullptr;
    m_szNextCharStart = nullptr;
    m_szTokenStart = nullptr;
  }

  m_sIterator = wdStringView((const char*)&m_Data[0], (const char*)&m_Data[0] + m_Data.GetCount() - 1);

  if (!m_sIterator.IsValid())
  {
    wdToken t;
    t.m_uiLine = 1;
    t.m_iType = wdTokenType::EndOfFile;
    m_Tokens.PushBack(t);
    return;
  }

  NextChar();
  NextChar();

  m_szTokenStart = m_szCurCharStart;

  while (m_szTokenStart != nullptr && *m_szTokenStart != '\0')
  {
    switch (m_CurMode)
    {
      case wdTokenType::Unknown:
        HandleUnknown();
        break;

      case wdTokenType::String1:
        HandleString('\"');
        break;

      case wdTokenType::String2:
        HandleString('\'');
        break;

      case wdTokenType::Integer:
      case wdTokenType::Float:
        HandleNumber();
        break;

      case wdTokenType::LineComment:
        HandleLineComment();
        break;

      case wdTokenType::BlockComment:
        HandleBlockComment();
        break;

      case wdTokenType::Whitespace:
        HandleWhitespace();
        break;

      case wdTokenType::Identifier:
        HandleIdentifier();
        break;

      case wdTokenType::NonIdentifier:
        HandleNonIdentifier();
        break;

      case wdTokenType::Newline:
      case wdTokenType::EndOfFile:
      case wdTokenType::ENUM_COUNT:
        break;
    }
  }

  wdToken t;
  t.m_uiLine = m_uiCurLine;
  t.m_iType = wdTokenType::EndOfFile;
  m_Tokens.PushBack(t);
}

void wdTokenizer::HandleUnknown()
{
  m_szTokenStart = m_szCurCharStart;

  if ((m_uiCurChar == '/') && (m_uiNextChar == '/'))
  {
    m_CurMode = wdTokenType::LineComment;
    NextChar();
    NextChar();
    return;
  }

  if (m_bHashSignIsLineComment && (m_uiCurChar == '#'))
  {
    m_CurMode = wdTokenType::LineComment;
    NextChar();
    return;
  }

  if ((m_uiCurChar == '/') && (m_uiNextChar == '*'))
  {
    m_CurMode = wdTokenType::BlockComment;
    NextChar();
    NextChar();
    return;
  }

  if (m_uiCurChar == '\"')
  {
    m_CurMode = wdTokenType::String1;
    NextChar();
    return;
  }

  if (m_uiCurChar == '\'')
  {
    m_CurMode = wdTokenType::String2;
    NextChar();
    return;
  }

  if ((m_uiCurChar == ' ') || (m_uiCurChar == '\t'))
  {
    m_CurMode = wdTokenType::Whitespace;
    NextChar();
    return;
  }

  if (wdStringUtils::IsDecimalDigit(m_uiCurChar) || (m_uiCurChar == '.' && wdStringUtils::IsDecimalDigit(m_uiNextChar)))
  {
    m_CurMode = m_uiCurChar == '.' ? wdTokenType::Float : wdTokenType::Integer;
    // Do not advance to next char here since we need the first character in HandleNumber
    return;
  }

  if (!wdStringUtils::IsIdentifierDelimiter_C_Code(m_uiCurChar))
  {
    m_CurMode = wdTokenType::Identifier;
    NextChar();
    return;
  }

  if (m_uiCurChar == '\n')
  {
    m_CurMode = wdTokenType::Newline;
    NextChar();
    AddToken();
    return;
  }

  if ((m_uiCurChar == '\r') && (m_uiNextChar == '\n'))
  {
    NextChar();
    NextChar();
    m_CurMode = wdTokenType::Newline;
    AddToken();
    return;
  }

  // else
  m_CurMode = wdTokenType::NonIdentifier;
  NextChar();
}

void wdTokenizer::HandleString(char terminator)
{
  while (m_uiCurChar != '\0')
  {
    // Escaped quote \"
    if ((m_uiCurChar == '\\') && (m_uiNextChar == terminator))
    {
      // skip this one
      NextChar();
      NextChar();
    }
    // escaped line break in string
    else if ((m_uiCurChar == '\\') && (m_uiNextChar == '\n'))
    {
      AddToken();

      // skip this one entirely
      NextChar();
      NextChar();

      m_CurMode = terminator == '\"' ? wdTokenType::String1 : wdTokenType::String2;
      m_szTokenStart = m_szCurCharStart;
    }
    // escaped line break in string
    else if ((m_uiCurChar == '\\') && (m_uiNextChar == '\r'))
    {
      // this might be a 3 character sequence of \\ \r \n -> skip them all
      AddToken();

      // skip \\ and \r
      NextChar();
      NextChar();

      // skip \n
      if (m_uiCurChar == '\n')
        NextChar();

      m_CurMode = terminator == '\"' ? wdTokenType::String1 : wdTokenType::String2;
      m_szTokenStart = m_szCurCharStart;
    }
    // escaped backslash
    else if ((m_uiCurChar == '\\') && (m_uiNextChar == '\\'))
    {
      // Skip
      NextChar();
      NextChar();
    }
    // not-escaped line break in string
    else if (m_uiCurChar == '\n')
    {
      wdLog::Error(m_pLog, "Unescaped Newline in string line {0} column {1}", m_uiCurLine, m_uiCurColumn);
      // NextChar(); // not sure whether to include the newline in the string or not
      AddToken();
      return;
    }
    // end of string
    else if (m_uiCurChar == terminator)
    {
      NextChar();
      AddToken();
      return;
    }
    else
    {
      NextChar();
    }
  }

  wdLog::Error(m_pLog, "String not closed at end of file");
  AddToken();
}

void wdTokenizer::HandleNumber()
{
  if (m_uiCurChar == '0' && (m_uiNextChar == 'x' || m_uiNextChar == 'X'))
  {
    NextChar();
    NextChar();

    wdUInt32 uiDigitsRead = 0;
    while (wdStringUtils::IsHexDigit(m_uiCurChar))
    {
      NextChar();
      ++uiDigitsRead;
    }

    if (uiDigitsRead < 1)
    {
      wdLog::Error(m_pLog, "Invalid hex literal");
    }
  }
  else
  {
    NextChar();

    while (wdStringUtils::IsDecimalDigit(m_uiCurChar))
    {
      NextChar();
    }

    if (m_CurMode != wdTokenType::Float && (m_uiCurChar == '.' || m_uiCurChar == 'e' || m_uiCurChar == 'E'))
    {
      m_CurMode = wdTokenType::Float;
      bool bAllowExponent = true;

      if (m_uiCurChar == '.')
      {
        NextChar();

        wdUInt32 uiDigitsRead = 0;
        while (wdStringUtils::IsDecimalDigit(m_uiCurChar))
        {
          NextChar();
          ++uiDigitsRead;
        }

        bAllowExponent = uiDigitsRead > 0;
      }

      if ((m_uiCurChar == 'e' || m_uiCurChar == 'E') && bAllowExponent)
      {
        NextChar();
        if (m_uiCurChar == '+' || m_uiCurChar == '-')
        {
          NextChar();
        }

        wdUInt32 uiDigitsRead = 0;
        while (wdStringUtils::IsDecimalDigit(m_uiCurChar))
        {
          NextChar();
          ++uiDigitsRead;
        }

        if (uiDigitsRead < 1)
        {
          wdLog::Error(m_pLog, "Invalid float literal");
        }
      }

      if (m_uiCurChar == 'f') // skip float suffix
      {
        NextChar();
      }
    }
  }

  AddToken();
}

void wdTokenizer::HandleLineComment()
{
  while (m_uiCurChar != '\0')
  {
    if ((m_uiCurChar == '\r') || (m_uiCurChar == '\n'))
    {
      AddToken();
      return;
    }

    NextChar();
  }

  // comment at end of file
  AddToken();
}

void wdTokenizer::HandleBlockComment()
{
  while (m_uiCurChar != '\0')
  {
    if ((m_uiCurChar == '*') && (m_uiNextChar == '/'))
    {
      NextChar();
      NextChar();
      AddToken();
      return;
    }

    NextChar();
  }

  wdLog::Error(m_pLog, "Block comment not closed at end of file.");
  AddToken();
}

void wdTokenizer::HandleWhitespace()
{
  while (m_uiCurChar != '\0')
  {
    if (m_uiCurChar != ' ' && m_uiCurChar != '\t')
    {
      AddToken();
      return;
    }

    NextChar();
  }

  // whitespace at end of file
  AddToken();
}

void wdTokenizer::HandleIdentifier()
{
  while (m_uiCurChar != '\0')
  {
    if (wdStringUtils::IsIdentifierDelimiter_C_Code(m_uiCurChar))
    {
      AddToken();
      return;
    }

    NextChar();
  }

  // identifier at end of file
  AddToken();
}

void wdTokenizer::HandleNonIdentifier()
{
  AddToken();
}

void wdTokenizer::GetAllLines(wdHybridArray<const wdToken*, 32>& ref_tokens) const
{
  ref_tokens.Clear();
  ref_tokens.Reserve(m_Tokens.GetCount());

  for (const wdToken& curToken : m_Tokens)
  {
    if (curToken.m_iType != wdTokenType::Newline)
    {
      ref_tokens.PushBack(&curToken);
    }
  }
}

wdResult wdTokenizer::GetNextLine(wdUInt32& ref_uiFirstToken, wdHybridArray<wdToken*, 32>& ref_tokens)
{
  ref_tokens.Clear();

  wdHybridArray<const wdToken*, 32> Tokens0;
  wdResult r = GetNextLine(ref_uiFirstToken, Tokens0);

  ref_tokens.SetCountUninitialized(Tokens0.GetCount());
  for (wdUInt32 i = 0; i < Tokens0.GetCount(); ++i)
    ref_tokens[i] = const_cast<wdToken*>(Tokens0[i]); // soo evil !

  return r;
}

wdResult wdTokenizer::GetNextLine(wdUInt32& ref_uiFirstToken, wdHybridArray<const wdToken*, 32>& ref_tokens) const
{
  ref_tokens.Clear();

  const wdUInt32 uiMaxTokens = m_Tokens.GetCount() - 1;

  while (ref_uiFirstToken < uiMaxTokens)
  {
    const wdToken& tCur = m_Tokens[ref_uiFirstToken];

    // found a backslash
    if (tCur.m_iType == wdTokenType::NonIdentifier && tCur.m_DataView == "\\")
    {
      const wdToken& tNext = m_Tokens[ref_uiFirstToken + 1];

      // and a newline!
      if (tNext.m_iType == wdTokenType::Newline)
      {
        /// \todo Theoretically, if the line ends with an identifier, and the next directly starts with one again,
        // we would need to merge the two into one identifier name, because the \ \n combo means it is not a
        // real line break
        // for now we ignore this and assume there is a 'whitespace' between such identifiers

        // we could maybe at least output a warning, if we detect it
        if (ref_uiFirstToken > 0 && m_Tokens[ref_uiFirstToken - 1].m_iType == wdTokenType::Identifier && ref_uiFirstToken + 2 < uiMaxTokens && m_Tokens[ref_uiFirstToken + 2].m_iType == wdTokenType::Identifier)
        {
          wdStringBuilder s1 = m_Tokens[ref_uiFirstToken - 1].m_DataView;
          wdStringBuilder s2 = m_Tokens[ref_uiFirstToken + 2].m_DataView;
          wdLog::Warning("Line {0}: The \\ at the line end is in the middle of an identifier name ('{1}' and '{2}'). However, merging identifier "
                         "names is currently not supported.",
            m_Tokens[ref_uiFirstToken].m_uiLine, s1, s2);
        }

        // ignore this
        ref_uiFirstToken += 2;
        continue;
      }
    }

    ref_tokens.PushBack(&tCur);

    if (m_Tokens[ref_uiFirstToken].m_iType == wdTokenType::Newline)
    {
      ++ref_uiFirstToken;
      return WD_SUCCESS;
    }

    ++ref_uiFirstToken;
  }

  if (ref_tokens.IsEmpty())
    return WD_FAILURE;

  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_Tokenizer);
