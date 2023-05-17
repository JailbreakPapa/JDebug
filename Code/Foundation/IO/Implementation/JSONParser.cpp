#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONParser.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/ConversionUtils.h>

wdJSONParser::wdJSONParser()
{
  m_uiCurByte = '\0';
  m_uiNextByte = '\0';
  m_pInput = nullptr;
  m_bSkippingMode = false;
  m_pLogInterface = nullptr;
  m_uiCurLine = 1;
  m_uiCurColumn = 0;
}

void wdJSONParser::SetInputStream(wdStreamReader& stream, wdUInt32 uiFirstLineOffset)
{
  m_StateStack.Clear();
  m_uiCurByte = '\0';
  m_TempString.Clear();
  m_bSkippingMode = false;
  m_uiCurLine = 1 + uiFirstLineOffset;
  m_uiCurColumn = 0;

  m_pInput = &stream;

  m_uiNextByte = ' ';
  ReadCharacter(true);

  // go to the start of the document
  SkipWhitespace();

  // put the NotStarted state onto the stack
  {
    JSONState s;
    s.m_State = NotStarted;
    m_StateStack.PushBack(s);
  }
}

void wdJSONParser::StartParsing()
{
  // remove the NotStarted state
  m_StateStack.PopBack();

  // put the Finished state onto the stack
  {
    JSONState s;
    s.m_State = Finished;
    m_StateStack.PushBack(s);
  }

  switch (m_uiCurByte)
  {
    case '\0':
      // document is empty
      return;

    case '{':
    {
      JSONState s;
      s.m_State = ReadingObject;
      m_StateStack.PushBack(s);

      SkipWhitespace();

      if (!m_bSkippingMode)
        OnBeginObject();
    }
      return;

    default:
    {
      // document is malformed

      wdStringBuilder s;
      s.Format("Start of document: Expected a { or an empty document. Got '{0}' instead.", wdArgC(m_uiCurByte));
      ParsingError(s.GetData(), true);
    }
      return;
  }
}

void wdJSONParser::ParseAll()
{
  while (ContinueParsing())
  {
  }
}

void wdJSONParser::ParsingError(const char* szMessage, bool bFatal)
{
  if (bFatal)
  {
    // prevent further error messages
    m_uiCurByte = '\0';
    m_StateStack.Clear();
  }

  if (bFatal)
    wdLog::Error(m_pLogInterface, "Line {0} ({1}): {2}", m_uiCurLine, m_uiCurColumn, szMessage);
  else
    wdLog::Warning(m_pLogInterface, szMessage);

  OnParsingError(szMessage, bFatal, m_uiCurLine, m_uiCurColumn);
}

void wdJSONParser::SkipObject()
{
  SkipStack(ReadingObject);
}

void wdJSONParser::SkipArray()
{
  SkipStack(ReadingArray);
}

void wdJSONParser::SkipStack(State s)
{
  m_bSkippingMode = true;

  wdUInt32 iSkipToStackHeight = m_StateStack.GetCount();

  for (wdUInt32 top = m_StateStack.GetCount(); top > 1; --top)
  {
    if (m_StateStack[top - 1].m_State == s)
    {
      iSkipToStackHeight = top - 1;
      break;
    }
  }

  while (m_StateStack.GetCount() > iSkipToStackHeight)
    ContinueParsing();

  m_bSkippingMode = false;
}

bool wdJSONParser::ContinueParsing()
{
  if (m_uiCurByte == '\0')
  {
    // there's always the 'finished' state on the top of the stack when everything went fine
    if (m_StateStack.GetCount() > 1)
    {
      ParsingError("End of the document reached without closing all objects.", true);
    }

    return false;
  }

  switch (m_StateStack.PeekBack().m_State)
  {
    case NotStarted:
      StartParsing();
      break;

    case Finished:
      return false;

    case ReadingObject:
      ContinueObject();
      break;

    case ReadingArray:
      ContinueArray();
      break;

    case ReadingVariable:
      ContinueVariable();
      break;

    case ReadingValue:
      ContinueValue();
      break;

    case ExpectSeparator:
      ContinueSeparator();
      break;

    default:
      WD_REPORT_FAILURE("Unknown State in JSON parser state machine.");
      break;
  }

  return true;
}

void wdJSONParser::ContinueObject()
{
  switch (m_uiCurByte)
  {
    case '\"':
    {
      JSONState s;
      s.m_State = ReadingVariable;
      m_StateStack.PushBack(s);
    }
      return;

    case '}':
      SkipWhitespace();

      m_StateStack.PopBack();

      if (!m_bSkippingMode)
        OnEndObject();
      return;

    case ',': // ignore superfluous commas
      SkipWhitespace();
      return;

    default:
    {
      wdStringBuilder s;
      s.Format("While parsing object: Expected \" to begin a new variable, or } to close the object. Got '{0}' instead.", wdArgC(m_uiCurByte));
      ParsingError(s.GetData(), true);
    }
      return;
  }
}

void wdJSONParser::ContinueArray()
{
  switch (m_uiCurByte)
  {
    case ']':
    {
      SkipWhitespace();

      m_StateStack.PopBack();

      if (!m_bSkippingMode)
        OnEndArray();
    }
      return;

    default:
    {
      JSONState s;

      s.m_State = ExpectSeparator;
      m_StateStack.PushBack(s);

      s.m_State = ReadingValue;
      m_StateStack.PushBack(s);
    }
      return;
  }
}

void wdJSONParser::ContinueVariable()
{
  if (!m_bSkippingMode)
    ReadString();
  else
    SkipString();

  SkipWhitespace();

  if (m_uiCurByte != ':')
  {
    wdStringBuilder s;
    s.Format("After parsing variable name: Expected : to separate variable and value, Got '{0}' instead.", wdArgC(m_uiCurByte));
    ParsingError(s.GetData(), false);
  }
  else
    SkipWhitespace();

  // remove ReadingVariable from the stack
  m_StateStack.PopBack();

  JSONState s;

  s.m_State = ExpectSeparator;
  m_StateStack.PushBack(s);

  s.m_State = ReadingValue;
  m_StateStack.PushBack(s);

  if (!m_bSkippingMode)
  {
    if (!OnVariable((const char*)&m_TempString[0]))
      SkipStack(ExpectSeparator);
  }
}


void wdJSONParser::ContinueValue()
{
  switch (m_uiCurByte)
  {
    case '\0':
      return;

    case '\"':
    {
      if (!m_bSkippingMode)
        ReadString();
      else
        SkipString();

      SkipWhitespace();

      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      if (!m_bSkippingMode)
        OnReadValue((const char*)&m_TempString[0]);
    }
      return;

    case '+':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.':
    {
      const double fValue = ReadNumber();

      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      if (!m_bSkippingMode)
        OnReadValue(fValue);
    }
      return;

    case 't':
    case 'f':
    {
      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      ReadWord();

      bool bRes = false;
      if (wdConversionUtils::StringToBool((const char*)&m_TempString[0], bRes) == WD_FAILURE)
      {
        wdStringBuilder s;
        s.Format("Parsing value: Expected 'true' or 'false', Got '{0}' instead.", (const char*)&m_TempString[0]);
        ParsingError(s.GetData(), false);
      }

      if (!m_bSkippingMode)
        OnReadValue(bRes);
    }
      return;

    case 'n':
    case 'N':
    {
      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      ReadWord();

      bool bIsNull = false;

      // if it's 'null' but with the wrong casing, output an error, but it is not fatal
      if (wdStringUtils::IsEqual_NoCase((const char*)&m_TempString[0], "null"))
        bIsNull = true;

      if (!wdStringUtils::IsEqual((const char*)&m_TempString[0], "null"))
      {
        wdStringBuilder s;
        s.Format("Parsing value: Expected 'null', Got '{0}' instead.", (const char*)&m_TempString[0]);
        ParsingError(s.GetData(), !bIsNull);
      }

      if (!m_bSkippingMode)
        OnReadValueNULL();
    }
      return;

    case '[':
    {
      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      JSONState s;
      s.m_State = ReadingArray;
      m_StateStack.PushBack(s);

      SkipWhitespace();

      if (!m_bSkippingMode)
        OnBeginArray();
    }
      return;

    case '{':
    {
      // remove ReadingValue from the stack
      m_StateStack.PopBack();

      JSONState s;
      s.m_State = ReadingObject;
      m_StateStack.PushBack(s);

      SkipWhitespace();

      if (!m_bSkippingMode)
        OnBeginObject();
    }
      return;

    default:
    {
      wdStringBuilder s;
      s.Format("Parsing value: Expected [, {, f, t, \", 0-1, ., +, -, or even 'e'. Got '{0}' instead", wdArgC(m_uiCurByte));
      ParsingError(s.GetData(), true);
    }
      return;
  }
}

void wdJSONParser::ContinueSeparator()
{
  if (wdStringUtils::IsWhiteSpace(m_uiCurByte))
    SkipWhitespace();

  // remove ExpectSeparator from the stack
  m_StateStack.PopBack();

  switch (m_uiCurByte)
  {
    case '\0':
      return;

    case ',':
      SkipWhitespace();
      return;

    case ']':
    case '}':
      return;

    default:
    {
      wdStringBuilder s;
      s.Format("After parsing value: Expected a comma or closing brackets/braces (], }). Got '{0}' instead.", wdArgC(m_uiCurByte));
      ParsingError(s.GetData(), true);
    }
      return;
  }
}

void wdJSONParser::ReadNextByte()
{
  m_pInput->ReadBytes(&m_uiNextByte, sizeof(wdUInt8));

  if (m_uiNextByte == '\n')
  {
    ++m_uiCurLine;
    m_uiCurColumn = 0;
  }
  else
    ++m_uiCurColumn;
}

bool wdJSONParser::ReadCharacter(bool bSkipComments)
{
  m_uiCurByte = m_uiNextByte;

  m_uiNextByte = '\0';
  ReadNextByte();

  // skip comments
  if (m_uiCurByte == '/' && bSkipComments)
  {
    // line comment, read till line break
    if (m_uiNextByte == '/')
    {
      while (m_uiNextByte != '\0' && m_uiNextByte != '\n')
      {
        m_uiNextByte = '\0';
        ReadNextByte();
      }

      ReadCharacter(true);
    }
    else if (m_uiNextByte == '*') // block comment, read till */
    {
      m_uiNextByte = ' ';

      while (m_uiNextByte != '\0' && (m_uiCurByte != '*' || m_uiNextByte != '/'))
      {
        m_uiCurByte = m_uiNextByte;

        m_uiNextByte = '\0';
        ReadNextByte();
      }

      // replace the current end-comment by whitespace
      m_uiCurByte = ' ';
      m_uiNextByte = ' ';

      ReadCharacter(true);
      ReadCharacter(true); // might trigger another comment skipping
    }
  }

  return m_uiCurByte != '\0';
}

void wdJSONParser::SkipWhitespace()
{
  WD_ASSERT_DEBUG(m_pInput != nullptr, "Input Stream is not set up.");

  do
  {
    m_uiCurByte = '\0';

    if (!ReadCharacter(true))
      return; // stop when end of stream is encountered
  } while (wdStringUtils::IsWhiteSpace(m_uiCurByte));
}

void wdJSONParser::SkipString()
{
  WD_ASSERT_DEBUG(m_pInput != nullptr, "Input Stream is not set up.");

  m_TempString.Clear();
  m_TempString.PushBack('\0');

  bool bEscapeSequence = false;

  do
  {
    bEscapeSequence = (m_uiCurByte == '\\');

    m_uiCurByte = '\0';

    if (!ReadCharacter(false))
    {
      ParsingError("While skipping string: Reached end of document before end of string was found.", true);

      return; // stop when end of stream is encountered
    }
  } while (bEscapeSequence || m_uiCurByte != '\"');
}

void wdJSONParser::ReadString()
{
  WD_ASSERT_DEBUG(m_pInput != nullptr, "Input Stream is not set up.");

  m_TempString.Clear();

  bool bEscapeSequence = false;

  while (true)
  {
    bEscapeSequence = (m_uiCurByte == '\\');

    m_uiCurByte = '\0';

    if (!ReadCharacter(false))
    {
      ParsingError("While reading string: Reached end of document before end of string was found.", true);

      break; // stop when end of stream is encountered
    }

    if (!bEscapeSequence && m_uiCurByte == '\"')
      break;

    if (bEscapeSequence)
    {
      switch (m_uiCurByte)
      {
        case '\"':
          m_TempString.PushBack('\"');
          break;
        case '\\':
          m_TempString.PushBack('\\');
          m_uiCurByte = '\0'; // make sure the next character isn't interpreted as an escape sequence
          break;
        case '/':
          m_TempString.PushBack('/');
          break;
        case 'b':
          m_TempString.PushBack('\b');
          break;
        case 'f':
          m_TempString.PushBack('\f');
          break;
        case 'n':
          m_TempString.PushBack('\n');
          break;
        case 'r':
          m_TempString.PushBack('\r');
          break;
        case 't':
          m_TempString.PushBack('\t');
          break;
        case 'u':
        {
          wdUInt16 cpt[2];
          auto ReadUtf16CodePoint = [&](wdUInt16& ref_uiCodePoint) -> bool {
            ref_uiCodePoint = 0;

            // Unicode literal are utf16 in the format \uFFFF. The hex number FFFF can be upper or lower case but must be 4 characters long.
            wdUInt8 unicodeLiteral[5] = {0, 0, 0, 0, 0};
            wdUInt32 i = 0;
            for (; i < 4; i++)
            {
              if (m_uiNextByte == '\0' || m_uiNextByte == '\"')
              {
                ParsingError("Unicode literal is too short, must be 4 HEX characters.", false);
                return false;
              }
              if ((m_uiNextByte < '0' || m_uiNextByte > '9') && (m_uiNextByte < 'A' || m_uiNextByte > 'F') && (m_uiNextByte < 'a' || m_uiNextByte > 'f'))
              {
                ParsingError("Unicode literal contains an invalid character.", false);
                return false;
              }
              ReadCharacter(false);

              unicodeLiteral[i] = m_uiCurByte;
            }

            wdUInt32 uiHexValue = 0;
            if (wdConversionUtils::ConvertHexStringToUInt32((const char*)&unicodeLiteral[0], uiHexValue).Succeeded())
            {
              ref_uiCodePoint = static_cast<wdUInt16>(uiHexValue);
            }
            else
            {
              ParsingError("Unicode HEX literal is malformed.", false);
            }

            return true;
          };
          if (ReadUtf16CodePoint(cpt[0]))
          {
            wdUInt16* start = &cpt[0];
            if (wdUnicodeUtils::IsUtf16Surrogate(start))
            {
              if (m_uiNextByte != '\\' || !ReadCharacter(false))
              {
                ParsingError("Unicode surrogate must be followed by another unicode escape sequence", false);
                break;
              }
              if (m_uiNextByte != 'u' || !ReadCharacter(false))
              {
                ParsingError("Unicode surrogate must be followed by another unicode escape sequence", false);
                break;
              }
              if (!ReadUtf16CodePoint(cpt[1]))
              {
                break;
              }
            }
            wdUInt32 uiCodePoint = wdUnicodeUtils::DecodeUtf16ToUtf32(start);
            wdUnicodeUtils::UtfInserter<char, wdHybridArray<wdUInt8, 4096>> tempInserter(&m_TempString);
            wdUnicodeUtils::EncodeUtf32ToUtf8(uiCodePoint, tempInserter);
          }
          break;
        }
        default:
        {
          wdStringBuilder s;
          s.Format("Unknown escape-sequence '\\{0}'", wdArgC(m_uiCurByte));
          ParsingError(s, false);
        }
        break;
      }
    }
    else if (m_uiCurByte != '\\')
    {
      m_TempString.PushBack(m_uiCurByte);
    }
  }

  m_TempString.PushBack('\0');
}

void wdJSONParser::ReadWord()
{
  WD_ASSERT_DEBUG(m_pInput != nullptr, "Input Stream is not set up.");

  m_TempString.Clear();

  do
  {
    m_TempString.PushBack(m_uiCurByte);

    m_uiCurByte = '\0';

    if (!ReadCharacter(true))
      break; // stop when end of stream is encountered
  } while (!wdStringUtils::IsWhiteSpace(m_uiCurByte) && m_uiCurByte != ',' && m_uiCurByte != ']' && m_uiCurByte != '}');

  m_TempString.PushBack('\0');
}

double wdJSONParser::ReadNumber()
{
  WD_ASSERT_DEBUG(m_pInput != nullptr, "Input Stream is not set up.");

  m_TempString.Clear();

  do
  {
    m_TempString.PushBack(m_uiCurByte);

    m_uiCurByte = '\0';

    if (!ReadCharacter(true))
      break; // stop when end of stream is encountered
  } while ((m_uiCurByte >= '0' && m_uiCurByte <= '9') || m_uiCurByte == '.' || m_uiCurByte == 'e' || m_uiCurByte == 'E' || m_uiCurByte == '-' ||
           m_uiCurByte == '+');

  m_TempString.PushBack('\0');

  double fResult = 0;
  if (wdConversionUtils::StringToFloat((const char*)&m_TempString[0], fResult) == WD_FAILURE)
  {
    wdStringBuilder s;
    s.Format("Reading number failed: Could not convert '{0}' to a floating point value.", (const char*)&m_TempString[0]);
    ParsingError(s.GetData(), true);
  }

  return fResult;
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_JSONParser);
