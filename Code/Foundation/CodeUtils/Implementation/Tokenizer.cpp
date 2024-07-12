#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Tokenizer.h>
#include <Foundation/Memory/CommonAllocators.h>

const char* nsTokenType::EnumNames[nsTokenType::ENUM_COUNT] = {
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
  "RawString1",
  "RawString1Prefix",
  "RawString1Postfix",
  "EndOfFile"};

namespace
{
  // This allocator is used to get rid of some of the memory allocation tracking
  // that would otherwise occur for allocations made by the tokenizer.
  thread_local nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::Nothing> s_ClassAllocator("nsTokenizer", nsFoundation::GetDefaultAllocator());
} // namespace


nsTokenizer::nsTokenizer(nsAllocator* pAllocator)
  : m_Tokens(pAllocator != nullptr ? pAllocator : &s_ClassAllocator)
  , m_Data(pAllocator != nullptr ? pAllocator : &s_ClassAllocator)
{
}

nsTokenizer::~nsTokenizer() = default;

void nsTokenizer::NextChar()
{
  m_uiCurChar = m_uiNextChar;
  m_szCurCharStart = m_szNextCharStart;
  ++m_uiCurColumn;

  if (m_uiCurChar == '\n')
  {
    ++m_uiCurLine;
    m_uiCurColumn = 0;
  }

  if (!m_sIterator.IsValid() || m_sIterator.IsEmpty())
  {
    m_szNextCharStart = m_sIterator.GetEndPointer();
    m_uiNextChar = '\0';
    return;
  }

  m_uiNextChar = m_sIterator.GetCharacter();
  m_szNextCharStart = m_sIterator.GetStartPointer();

  ++m_sIterator;
}

void nsTokenizer::AddToken()
{
  const char* szEnd = m_szCurCharStart;

  nsToken t;
  t.m_uiLine = m_uiLastLine;
  t.m_uiColumn = m_uiLastColumn;
  t.m_iType = m_CurMode;
  t.m_DataView = nsStringView(m_szTokenStart, szEnd);

  m_uiLastLine = m_uiCurLine;
  m_uiLastColumn = m_uiCurColumn;

  m_Tokens.PushBack(t);

  m_szTokenStart = szEnd;

  m_CurMode = nsTokenType::Unknown;
}

void nsTokenizer::Tokenize(nsArrayPtr<const nsUInt8> data, nsLogInterface* pLog, bool bCopyData)
{
  if (bCopyData)
  {
    m_Data = data;
    data = m_Data;
  }
  else
  {
    m_Data.Clear();
  }

  if (data.GetCount() >= 3)
  {
    const char* dataStart = reinterpret_cast<const char*>(data.GetPtr());

    if (nsUnicodeUtils::SkipUtf8Bom(dataStart))
    {
      nsLog::Error(pLog, "Data to tokenize contains a Utf-8 BOM.");

      // although the tokenizer should get data without a BOM, it's easy enough to work around that here
      // that's what the tokenizer does in other error cases as well - complain, but continue
      data = nsArrayPtr<const nsUInt8>((const nsUInt8*)dataStart, data.GetCount() - 3);
    }
  }

  m_Tokens.Clear();
  m_pLog = pLog;

  {
    m_CurMode = nsTokenType::Unknown;
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

  m_sIterator = {};
  if (!data.IsEmpty())
  {
    m_sIterator = nsStringView((const char*)&data[0], (const char*)&data[0] + data.GetCount());
  }

  if (!m_sIterator.IsValid() || m_sIterator.IsEmpty())
  {
    nsToken t;
    t.m_uiLine = 1;
    t.m_iType = nsTokenType::EndOfFile;
    m_Tokens.PushBack(t);
    return;
  }

  NextChar();
  NextChar();

  m_szTokenStart = m_szCurCharStart;

  while (m_szTokenStart != nullptr && m_szTokenStart != m_sIterator.GetEndPointer())
  {
    switch (m_CurMode)
    {
      case nsTokenType::Unknown:
        HandleUnknown();
        break;

      case nsTokenType::String1:
        HandleString('\"');
        break;

      case nsTokenType::RawString1:
        HandleRawString();
        break;

      case nsTokenType::String2:
        HandleString('\'');
        break;

      case nsTokenType::Integer:
      case nsTokenType::Float:
        HandleNumber();
        break;

      case nsTokenType::LineComment:
        HandleLineComment();
        break;

      case nsTokenType::BlockComment:
        HandleBlockComment();
        break;

      case nsTokenType::Whitespace:
        HandleWhitespace();
        break;

      case nsTokenType::Identifier:
        HandleIdentifier();
        break;

      case nsTokenType::NonIdentifier:
        HandleNonIdentifier();
        break;

      case nsTokenType::RawString1Prefix:
      case nsTokenType::RawString1Postfix:
      case nsTokenType::Newline:
      case nsTokenType::EndOfFile:
      case nsTokenType::ENUM_COUNT:
        break;
    }
  }

  nsToken t;
  t.m_uiLine = m_uiCurLine;
  t.m_iType = nsTokenType::EndOfFile;
  m_Tokens.PushBack(t);
}

void nsTokenizer::HandleUnknown()
{
  m_szTokenStart = m_szCurCharStart;

  if ((m_uiCurChar == '/') && (m_uiNextChar == '/'))
  {
    m_CurMode = nsTokenType::LineComment;
    NextChar();
    NextChar();
    return;
  }

  if (m_bHashSignIsLineComment && (m_uiCurChar == '#'))
  {
    m_CurMode = nsTokenType::LineComment;
    NextChar();
    return;
  }

  if ((m_uiCurChar == '/') && (m_uiNextChar == '*'))
  {
    m_CurMode = nsTokenType::BlockComment;
    NextChar();
    NextChar();
    return;
  }

  if (m_uiCurChar == '\"')
  {
    m_CurMode = nsTokenType::String1;
    NextChar();
    return;
  }

  if (m_uiCurChar == 'R' && m_uiNextChar == '\"')
  {
    m_CurMode = nsTokenType::RawString1;
    NextChar();
    NextChar();
    return;
  }

  if (m_uiCurChar == '\'')
  {
    m_CurMode = nsTokenType::String2;
    NextChar();
    return;
  }

  if ((m_uiCurChar == ' ') || (m_uiCurChar == '\t'))
  {
    m_CurMode = nsTokenType::Whitespace;
    NextChar();
    return;
  }

  if (nsStringUtils::IsDecimalDigit(m_uiCurChar) || (m_uiCurChar == '.' && nsStringUtils::IsDecimalDigit(m_uiNextChar)))
  {
    m_CurMode = m_uiCurChar == '.' ? nsTokenType::Float : nsTokenType::Integer;
    // Do not advance to next char here since we need the first character in HandleNumber
    return;
  }

  if (!nsStringUtils::IsIdentifierDelimiter_C_Code(m_uiCurChar))
  {
    m_CurMode = nsTokenType::Identifier;
    NextChar();
    return;
  }

  if (m_uiCurChar == '\n')
  {
    m_CurMode = nsTokenType::Newline;
    NextChar();
    AddToken();
    return;
  }

  if ((m_uiCurChar == '\r') && (m_uiNextChar == '\n'))
  {
    NextChar();
    NextChar();
    m_CurMode = nsTokenType::Newline;
    AddToken();
    return;
  }

  // else
  m_CurMode = nsTokenType::NonIdentifier;
  NextChar();
}

void nsTokenizer::HandleString(char terminator)
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

      m_CurMode = terminator == '\"' ? nsTokenType::String1 : nsTokenType::String2;
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

      m_CurMode = terminator == '\"' ? nsTokenType::String1 : nsTokenType::String2;
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
      nsLog::Error(m_pLog, "Unescaped Newline in string line {0} column {1}", m_uiCurLine, m_uiCurColumn);
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

  nsLog::Error(m_pLog, "String not closed at end of file");
  AddToken();
}

void nsTokenizer::HandleRawString()
{
  const char* markerStart = m_szCurCharStart;
  while (m_uiCurChar != '\0')
  {
    if (m_uiCurChar == '(')
    {
      m_sRawStringMarker = nsStringView(markerStart, m_szCurCharStart);
      NextChar(); // consume '('
      break;
    }
    NextChar();
  }
  if (m_uiCurChar == '\0')
  {
    nsLog::Error(m_pLog, "Failed to find '(' for raw string before end of file");
    AddToken();
    return;
  }

  m_CurMode = nsTokenType::RawString1Prefix;
  AddToken();

  m_CurMode = nsTokenType::RawString1;

  while (m_uiCurChar != '\0')
  {
    if (m_uiCurChar == ')')
    {
      if (m_sRawStringMarker.GetElementCount() == 0 && m_uiNextChar == '\"')
      {
        AddToken();
        NextChar();
        NextChar();
        m_CurMode = nsTokenType::RawString1Postfix;
        AddToken();
        return;
      }
      else if (m_szCurCharStart + m_sRawStringMarker.GetElementCount() + 2 <= m_sIterator.GetEndPointer())
      {
        if (nsStringUtils::CompareN(m_szCurCharStart + 1, m_sRawStringMarker.GetStartPointer(), m_sRawStringMarker.GetElementCount()) == 0 &&
            m_szCurCharStart[m_sRawStringMarker.GetElementCount() + 1] == '\"')
        {
          AddToken();
          for (nsUInt32 i = 0; i < m_sRawStringMarker.GetElementCount() + 2; ++i) // consume )marker"
          {
            NextChar();
          }
          m_CurMode = nsTokenType::RawString1Postfix;
          AddToken();
          return;
        }
      }
      NextChar();
    }
    else
    {
      NextChar();
    }
  }

  nsLog::Error(m_pLog, "Raw string not closed at end of file");
  AddToken();
}

void nsTokenizer::HandleNumber()
{
  if (m_uiCurChar == '0' && (m_uiNextChar == 'x' || m_uiNextChar == 'X'))
  {
    NextChar();
    NextChar();

    nsUInt32 uiDigitsRead = 0;
    while (nsStringUtils::IsHexDigit(m_uiCurChar))
    {
      NextChar();
      ++uiDigitsRead;
    }

    if (uiDigitsRead < 1)
    {
      nsLog::Error(m_pLog, "Invalid hex literal");
    }
  }
  else
  {
    NextChar();

    while (nsStringUtils::IsDecimalDigit(m_uiCurChar) || m_uiCurChar == '\'') // integer literal: 100'000
    {
      NextChar();
    }

    if (m_CurMode != nsTokenType::Float && (m_uiCurChar == '.' || m_uiCurChar == 'e' || m_uiCurChar == 'E'))
    {
      m_CurMode = nsTokenType::Float;
      bool bAllowExponent = true;

      if (m_uiCurChar == '.')
      {
        NextChar();

        nsUInt32 uiDigitsRead = 0;
        while (nsStringUtils::IsDecimalDigit(m_uiCurChar))
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

        nsUInt32 uiDigitsRead = 0;
        while (nsStringUtils::IsDecimalDigit(m_uiCurChar))
        {
          NextChar();
          ++uiDigitsRead;
        }

        if (uiDigitsRead < 1)
        {
          nsLog::Error(m_pLog, "Invalid float literal");
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

void nsTokenizer::HandleLineComment()
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

void nsTokenizer::HandleBlockComment()
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

  nsLog::Error(m_pLog, "Block comment not closed at end of file.");
  AddToken();
}

void nsTokenizer::HandleWhitespace()
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

void nsTokenizer::HandleIdentifier()
{
  while (m_uiCurChar != '\0')
  {
    if (nsStringUtils::IsIdentifierDelimiter_C_Code(m_uiCurChar))
    {
      AddToken();
      return;
    }

    NextChar();
  }

  // identifier at end of file
  AddToken();
}

void nsTokenizer::HandleNonIdentifier()
{
  AddToken();
}

void nsTokenizer::GetAllTokens(nsDynamicArray<const nsToken*>& ref_tokens) const
{
  ref_tokens.Clear();
  ref_tokens.Reserve(m_Tokens.GetCount());

  for (const nsToken& curToken : m_Tokens)
  {
    ref_tokens.PushBack(&curToken);
  }
}

void nsTokenizer::GetAllLines(nsDynamicArray<const nsToken*>& ref_tokens) const
{
  ref_tokens.Clear();
  ref_tokens.Reserve(m_Tokens.GetCount());

  for (const nsToken& curToken : m_Tokens)
  {
    if (curToken.m_iType != nsTokenType::Newline)
    {
      ref_tokens.PushBack(&curToken);
    }
  }
}

nsResult nsTokenizer::GetNextLine(nsUInt32& ref_uiFirstToken, nsHybridArray<nsToken*, 32>& ref_tokens)
{
  ref_tokens.Clear();

  nsHybridArray<const nsToken*, 32> Tokens0;
  nsResult r = GetNextLine(ref_uiFirstToken, Tokens0);

  ref_tokens.SetCountUninitialized(Tokens0.GetCount());
  for (nsUInt32 i = 0; i < Tokens0.GetCount(); ++i)
    ref_tokens[i] = const_cast<nsToken*>(Tokens0[i]); // soo evil !

  return r;
}

nsResult nsTokenizer::GetNextLine(nsUInt32& ref_uiFirstToken, nsHybridArray<const nsToken*, 32>& ref_tokens) const
{
  ref_tokens.Clear();

  const nsUInt32 uiMaxTokens = m_Tokens.GetCount() - 1;

  while (ref_uiFirstToken < uiMaxTokens)
  {
    const nsToken& tCur = m_Tokens[ref_uiFirstToken];

    // found a backslash
    if (tCur.m_iType == nsTokenType::NonIdentifier && tCur.m_DataView == "\\")
    {
      const nsToken& tNext = m_Tokens[ref_uiFirstToken + 1];

      // and a newline!
      if (tNext.m_iType == nsTokenType::Newline)
      {
        /// \todo Theoretically, if the line ends with an identifier, and the next directly starts with one again,
        // we would need to merge the two into one identifier name, because the \ \n combo means it is not a
        // real line break
        // for now we ignore this and assume there is a 'whitespace' between such identifiers

        // we could maybe at least output a warning, if we detect it
        if (ref_uiFirstToken > 0 && m_Tokens[ref_uiFirstToken - 1].m_iType == nsTokenType::Identifier && ref_uiFirstToken + 2 < uiMaxTokens && m_Tokens[ref_uiFirstToken + 2].m_iType == nsTokenType::Identifier)
        {
          nsStringBuilder s1 = m_Tokens[ref_uiFirstToken - 1].m_DataView;
          nsStringBuilder s2 = m_Tokens[ref_uiFirstToken + 2].m_DataView;
          nsLog::Warning("Line {0}: The \\ at the line end is in the middle of an identifier name ('{1}' and '{2}'). However, merging identifier "
                         "names is currently not supported.",
            m_Tokens[ref_uiFirstToken].m_uiLine, s1, s2);
        }

        // ignore this
        ref_uiFirstToken += 2;
        continue;
      }
    }

    ref_tokens.PushBack(&tCur);

    if (m_Tokens[ref_uiFirstToken].m_iType == nsTokenType::Newline)
    {
      ++ref_uiFirstToken;
      return NS_SUCCESS;
    }

    ++ref_uiFirstToken;
  }

  if (ref_tokens.IsEmpty())
    return NS_FAILURE;

  return NS_SUCCESS;
}
