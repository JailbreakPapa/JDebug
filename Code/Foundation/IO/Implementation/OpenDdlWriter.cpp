#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/ConversionUtils.h>

void wdOpenDdlWriter::OutputEscapedString(const wdStringView& string)
{
  m_sTemp = string;
  m_sTemp.ReplaceAll("\\", "\\\\");
  m_sTemp.ReplaceAll("\"", "\\\"");
  m_sTemp.ReplaceAll("\b", "\\b");
  m_sTemp.ReplaceAll("\r", "\\r");
  m_sTemp.ReplaceAll("\f", "\\f");
  m_sTemp.ReplaceAll("\n", "\\n");
  m_sTemp.ReplaceAll("\t", "\\t");

  OutputString("\"", 1);
  OutputString(m_sTemp.GetData());
  OutputString("\"", 1);
}

void wdOpenDdlWriter::OutputIndentation()
{
  if (m_bCompactMode)
    return;

  wdInt32 iIndentation = m_iIndentation;

  // I need my space!
  const char* szIndentation = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

  while (iIndentation >= 16)
  {
    OutputString(szIndentation, 16);
    iIndentation -= 16;
  }

  if (iIndentation > 0)
  {
    OutputString(szIndentation, iIndentation);
  }
}

void wdOpenDdlWriter::OutputPrimitiveTypeNameCompliant(wdOpenDdlPrimitiveType type)
{
  switch (type)
  {
    case wdOpenDdlPrimitiveType::Bool:
      OutputString("bool", 4);
      break;
    case wdOpenDdlPrimitiveType::Int8:
      OutputString("int8", 4);
      break;
    case wdOpenDdlPrimitiveType::Int16:
      OutputString("int16", 5);
      break;
    case wdOpenDdlPrimitiveType::Int32:
      OutputString("int32", 5);
      break;
    case wdOpenDdlPrimitiveType::Int64:
      OutputString("int64", 5);
      break;
    case wdOpenDdlPrimitiveType::UInt8:
      OutputString("unsigned_int8", 13);
      break;
    case wdOpenDdlPrimitiveType::UInt16:
      OutputString("unsigned_int16", 14);
      break;
    case wdOpenDdlPrimitiveType::UInt32:
      OutputString("unsigned_int32", 14);
      break;
    case wdOpenDdlPrimitiveType::UInt64:
      OutputString("unsigned_int64", 14);
      break;
    case wdOpenDdlPrimitiveType::Float:
      OutputString("float", 5);
      break;
    case wdOpenDdlPrimitiveType::Double:
      OutputString("double", 6);
      break;
    case wdOpenDdlPrimitiveType::String:
      OutputString("string", 6);
      break;

    default:
      WD_REPORT_FAILURE("Unknown DDL primitive type {0}", (wdUInt32)type);
      break;
  }
}
void wdOpenDdlWriter::OutputPrimitiveTypeNameShort(wdOpenDdlPrimitiveType type)
{
  // Change to OpenDDL: We write uint8 etc. instead of unsigned_int

  switch (type)
  {
    case wdOpenDdlPrimitiveType::Bool:
      OutputString("bool", 4);
      break;
    case wdOpenDdlPrimitiveType::Int8:
      OutputString("int8", 4);
      break;
    case wdOpenDdlPrimitiveType::Int16:
      OutputString("int16", 5);
      break;
    case wdOpenDdlPrimitiveType::Int32:
      OutputString("int32", 5);
      break;
    case wdOpenDdlPrimitiveType::Int64:
      OutputString("int64", 5);
      break;
    case wdOpenDdlPrimitiveType::UInt8:
      OutputString("uint8", 5);
      break;
    case wdOpenDdlPrimitiveType::UInt16:
      OutputString("uint16", 6);
      break;
    case wdOpenDdlPrimitiveType::UInt32:
      OutputString("uint32", 6);
      break;
    case wdOpenDdlPrimitiveType::UInt64:
      OutputString("uint64", 6);
      break;
    case wdOpenDdlPrimitiveType::Float:
      OutputString("float", 5);
      break;
    case wdOpenDdlPrimitiveType::Double:
      OutputString("double", 6);
      break;
    case wdOpenDdlPrimitiveType::String:
      OutputString("string", 6);
      break;

    default:
      WD_REPORT_FAILURE("Unknown DDL primitive type {0}", (wdUInt32)type);
      break;
  }
}

void wdOpenDdlWriter::OutputPrimitiveTypeNameShortest(wdOpenDdlPrimitiveType type)
{
  // Change to OpenDDL: We write super short type strings

  switch (type)
  {
    case wdOpenDdlPrimitiveType::Bool:
      OutputString("b", 1);
      break;
    case wdOpenDdlPrimitiveType::Int8:
      OutputString("i1", 2);
      break;
    case wdOpenDdlPrimitiveType::Int16:
      OutputString("i2", 2);
      break;
    case wdOpenDdlPrimitiveType::Int32:
      OutputString("i3", 2);
      break;
    case wdOpenDdlPrimitiveType::Int64:
      OutputString("i4", 2);
      break;
    case wdOpenDdlPrimitiveType::UInt8:
      OutputString("u1", 2);
      break;
    case wdOpenDdlPrimitiveType::UInt16:
      OutputString("u2", 2);
      break;
    case wdOpenDdlPrimitiveType::UInt32:
      OutputString("u3", 2);
      break;
    case wdOpenDdlPrimitiveType::UInt64:
      OutputString("u4", 2);
      break;
    case wdOpenDdlPrimitiveType::Float:
      OutputString("f", 1);
      break;
    case wdOpenDdlPrimitiveType::Double:
      OutputString("d", 1);
      break;
    case wdOpenDdlPrimitiveType::String:
      OutputString("s", 1);
      break;

    default:
      WD_REPORT_FAILURE("Unknown DDL primitive type {0}", (wdUInt32)type);
      break;
  }
}

wdOpenDdlWriter::wdOpenDdlWriter()
{
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesBool == (int)wdOpenDdlPrimitiveType::Bool);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesInt8 == (int)wdOpenDdlPrimitiveType::Int8);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesInt16 == (int)wdOpenDdlPrimitiveType::Int16);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesInt32 == (int)wdOpenDdlPrimitiveType::Int32);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesInt64 == (int)wdOpenDdlPrimitiveType::Int64);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesUInt8 == (int)wdOpenDdlPrimitiveType::UInt8);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesUInt16 == (int)wdOpenDdlPrimitiveType::UInt16);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesUInt32 == (int)wdOpenDdlPrimitiveType::UInt32);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesUInt64 == (int)wdOpenDdlPrimitiveType::UInt64);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesFloat == (int)wdOpenDdlPrimitiveType::Float);
  WD_CHECK_AT_COMPILETIME((int)wdOpenDdlWriter::State::PrimitivesString == (int)wdOpenDdlPrimitiveType::String);

  m_bCompactMode = false;
  m_TypeStringMode = TypeStringMode::ShortenedUnsignedInt;
  m_FloatPrecisionMode = FloatPrecisionMode::Exact;
  m_iIndentation = 0;

  m_StateStack.ExpandAndGetRef().m_State = State::Invalid;
  m_StateStack.ExpandAndGetRef().m_State = State::Empty;
}

// All,              ///< All whitespace is output. This is the default, it should be used for files that are read by humans.
// LessIndentation,  ///< Saves some space by using less space for indentation
// NoIndentation,    ///< Saves even more space by dropping all indentation from the output. The result will be noticeably less readable.
// NewlinesOnly,     ///< All unnecessary whitespace, except for newlines, is not output.
// None,             ///< No whitespace, not even newlines, is output. This should be used when DDL is used for data exchange, but probably not read
// by humans.

void wdOpenDdlWriter::BeginObject(const char* szType, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/, bool bSingleLine /*= false*/)
{
  {
    const auto state = m_StateStack.PeekBack().m_State;
    WD_IGNORE_UNUSED(state);
    WD_ASSERT_DEBUG(state == State::Empty || state == State::ObjectMultiLine || state == State::ObjectStart,
      "DDL Writer is in a state where no further objects may be created");
  }

  OutputObjectBeginning();

  {
    const auto state = m_StateStack.PeekBack().m_State;
    WD_IGNORE_UNUSED(state);
    WD_ASSERT_DEBUG(state != State::ObjectSingleLine, "Cannot put an object into another single-line object");
    WD_ASSERT_DEBUG(state != State::ObjectStart, "Object beginning should have been written");
  }

  OutputIndentation();
  OutputString(szType);

  OutputObjectName(szName, bGlobalName);

  if (bSingleLine)
  {
    m_StateStack.ExpandAndGetRef().m_State = State::ObjectSingleLine;
  }
  else
  {
    m_StateStack.ExpandAndGetRef().m_State = State::ObjectMultiLine;
  }

  m_StateStack.ExpandAndGetRef().m_State = State::ObjectStart;
}


void wdOpenDdlWriter::OutputObjectBeginning()
{
  if (m_StateStack.PeekBack().m_State != State::ObjectStart)
    return;

  m_StateStack.PopBack();

  const auto state = m_StateStack.PeekBack().m_State;

  if (state == State::ObjectSingleLine)
  {
    // if (m_bCompactMode)
    OutputString("{", 1); // more compact
    // else
    // OutputString(" { ", 3);
  }
  else if (state == State::ObjectMultiLine)
  {
    if (m_bCompactMode)
    {
      OutputString("{", 1);
    }
    else
    {
      OutputString("\n", 1);
      OutputIndentation();
      OutputString("{\n", 2);
    }
  }

  m_iIndentation++;
}

bool IsDdlIdentifierCharacter(wdUInt8 uiByte);

void wdOpenDdlWriter::OutputObjectName(const char* szName, bool bGlobalName)
{
  if (!wdStringUtils::IsNullOrEmpty(szName))
  {
    // WD_ASSERT_DEBUG(wdStringUtils::FindSubString(szName, " ") == nullptr, "Spaces are not allowed in DDL object names: '{0}'", szName);


    /// \test This code path is untested
    bool bEscape = false;
    for (const char* szNameCpy = szName; *szNameCpy != '\0'; ++szNameCpy)
    {
      if (!IsDdlIdentifierCharacter(*szNameCpy))
      {
        bEscape = true;
        break;
      }
    }

    if (m_bCompactMode)
    {
      // even remove the whitespace between type and name

      if (bGlobalName)
        OutputString("$", 1);
      else
        OutputString("%", 1);
    }
    else
    {
      if (bGlobalName)
        OutputString(" $", 2);
      else
        OutputString(" %", 2);
    }

    if (bEscape)
      OutputString("\'", 1);

    OutputString(szName);

    if (bEscape)
      OutputString("\'", 1);
  }
}

void wdOpenDdlWriter::EndObject()
{
  const auto state = m_StateStack.PeekBack().m_State;
  WD_ASSERT_DEBUG(state == State::ObjectSingleLine || state == State::ObjectMultiLine || state == State::ObjectStart, "No object is open");

  if (state == State::ObjectStart)
  {
    // object is empty

    OutputString("{}\n", 3);
    m_StateStack.PopBack();

    const auto newState = m_StateStack.PeekBack().m_State;
    WD_IGNORE_UNUSED(newState);
    WD_ASSERT_DEBUG(newState == State::ObjectSingleLine || newState == State::ObjectMultiLine, "No object is open");
  }
  else
  {
    m_iIndentation--;

    if (m_bCompactMode)
      OutputString("}", 1);
    else
    {
      if (state == State::ObjectMultiLine)
      {
        OutputIndentation();
        OutputString("}\n", 2);
      }
      else
      {
        // OutputString(" }\n", 3);
        OutputString("}\n", 2); // more compact
      }
    }
  }

  m_StateStack.PopBack();
}

void wdOpenDdlWriter::BeginPrimitiveList(wdOpenDdlPrimitiveType type, const char* szName /*= nullptr*/, bool bGlobalName /*= false*/)
{
  OutputObjectBeginning();

  const auto state = m_StateStack.PeekBack().m_State;
  WD_ASSERT_DEBUG(state == State::Empty || state == State::ObjectSingleLine || state == State::ObjectMultiLine,
    "DDL Writer is in a state where no primitive list may be created");

  if (state == State::ObjectMultiLine)
  {
    OutputIndentation();
  }

  if (m_TypeStringMode == TypeStringMode::Shortest)
    OutputPrimitiveTypeNameShortest(type);
  else if (m_TypeStringMode == TypeStringMode::ShortenedUnsignedInt)
    OutputPrimitiveTypeNameShort(type);
  else
    OutputPrimitiveTypeNameCompliant(type);

  OutputObjectName(szName, bGlobalName);

  // more compact
  // if (m_bCompactMode)
  OutputString("{", 1);
  // else
  // OutputString(" {", 2);

  m_StateStack.ExpandAndGetRef().m_State = static_cast<State>(type);
}

void wdOpenDdlWriter::EndPrimitiveList()
{
  const auto state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEBUG(state >= State::PrimitivesBool && state <= State::PrimitivesString, "No primitive list is open");

  m_StateStack.PopBack();

  if (m_bCompactMode)
    OutputString("}", 1);
  else
  {
    if (m_StateStack.PeekBack().m_State == State::ObjectSingleLine)
      OutputString("}", 1);
    else
      OutputString("}\n", 2);
  }
}

void wdOpenDdlWriter::WritePrimitiveType(wdOpenDdlWriter::State exp)
{
  auto& state = m_StateStack.PeekBack();
  WD_ASSERT_DEBUG(state.m_State == exp, "Cannot write thie primitive type without have the correct primitive list open");

  if (state.m_bPrimitivesWritten)
  {
    // already wrote some primitives, so append a comma
    OutputString(",", 1);
  }

  state.m_bPrimitivesWritten = true;
}


void wdOpenDdlWriter::WriteBinaryAsHex(const void* pData, wdUInt32 uiBytes)
{
  char tmp[4];

  wdUInt8* pBytes = (wdUInt8*)pData;

  for (wdUInt32 i = 0; i < uiBytes; ++i)
  {
    wdStringUtils::snprintf(tmp, 4, "%02X", (wdUInt32)*pBytes);
    ++pBytes;

    OutputString(tmp, 2);
  }
}

void wdOpenDdlWriter::WriteBool(const bool* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesBool);

  if (m_bCompactMode || m_TypeStringMode == TypeStringMode::Shortest)
  {
    // Extension to OpenDDL: We write only '1' or '0' in compact mode

    if (pValues[0])
      OutputString("1", 1);
    else
      OutputString("0", 1);

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      if (pValues[i])
        OutputString(",1", 2);
      else
        OutputString(",0", 2);
    }
  }
  else
  {
    if (pValues[0])
      OutputString("true", 4);
    else
      OutputString("false", 5);

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      if (pValues[i])
        OutputString(",true", 5);
      else
        OutputString(",false", 6);
    }
  }
}

void wdOpenDdlWriter::WriteInt8(const wdInt8* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt8);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteInt16(const wdInt16* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt16);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteInt32(const wdInt32* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt32);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteInt64(const wdInt64* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt64);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}


void wdOpenDdlWriter::WriteUInt8(const wdUInt8* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt8);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteUInt16(const wdUInt16* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt16);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteUInt32(const wdUInt32* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt32);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteUInt64(const wdUInt64* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt64);

  m_sTemp.Format("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (wdUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.Format(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void wdOpenDdlWriter::WriteFloat(const float* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesFloat);

  if (m_FloatPrecisionMode == FloatPrecisionMode::Readable)
  {
    m_sTemp.Format("{0}", pValues[0]);
    OutputString(m_sTemp.GetData());

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      m_sTemp.Format(",{0}", pValues[i]);
      OutputString(m_sTemp.GetData());
    }
  }
  else
  {
    // zeros are so common that writing them in HEX blows up file size, so write them as decimals

    if (pValues[0] == 0)
    {
      OutputString("0", 1);
    }
    else
    {
      OutputString("0x", 2);
      WriteBinaryAsHex(&pValues[0], 4);
    }

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      if (pValues[i] == 0)
      {
        OutputString(",0", 2);
      }
      else
      {
        OutputString(",0x", 3);
        WriteBinaryAsHex(&pValues[i], 4);
      }
    }
  }
}

void wdOpenDdlWriter::WriteDouble(const double* pValues, wdUInt32 uiCount /*= 1*/)
{
  WD_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  WD_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesDouble);

  if (m_FloatPrecisionMode == FloatPrecisionMode::Readable)
  {
    m_sTemp.Format("{0}", pValues[0]);
    OutputString(m_sTemp.GetData());

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      m_sTemp.Format(",{0}", pValues[i]);
      OutputString(m_sTemp.GetData());
    }
  }
  else
  {
    // zeros are so common that writing them in HEX blows up file size, so write them as decimals

    if (pValues[0] == 0)
    {
      OutputString("0", 1);
    }
    else
    {
      OutputString("0x", 2);
      WriteBinaryAsHex(&pValues[0], 8);
    }

    for (wdUInt32 i = 1; i < uiCount; ++i)
    {
      if (pValues[i] == 0)
      {
        OutputString(",0", 2);
      }
      else
      {
        OutputString(",0x", 3);
        WriteBinaryAsHex(&pValues[i], 8);
      }
    }
  }
}

void wdOpenDdlWriter::WriteString(const wdStringView& sString)
{
  WritePrimitiveType(State::PrimitivesString);

  OutputEscapedString(sString);
}

void wdOpenDdlWriter::WriteBinaryAsString(const void* pData, wdUInt32 uiBytes)
{
  /// \test wdOpenDdlWriter::WriteBinaryAsString

  WritePrimitiveType(State::PrimitivesString);

  OutputString("\"", 1);
  WriteBinaryAsHex(pData, uiBytes);
  OutputString("\"", 1);
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_OpenDdlWriter);
