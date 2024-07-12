#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/ConversionUtils.h>

void nsOpenDdlWriter::OutputEscapedString(const nsStringView& string)
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

void nsOpenDdlWriter::OutputIndentation()
{
  if (m_bCompactMode)
    return;

  nsInt32 iIndentation = m_iIndentation;

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

void nsOpenDdlWriter::OutputPrimitiveTypeNameCompliant(nsOpenDdlPrimitiveType type)
{
  switch (type)
  {
    case nsOpenDdlPrimitiveType::Bool:
      OutputString("bool", 4);
      break;
    case nsOpenDdlPrimitiveType::Int8:
      OutputString("int8", 4);
      break;
    case nsOpenDdlPrimitiveType::Int16:
      OutputString("int16", 5);
      break;
    case nsOpenDdlPrimitiveType::Int32:
      OutputString("int32", 5);
      break;
    case nsOpenDdlPrimitiveType::Int64:
      OutputString("int64", 5);
      break;
    case nsOpenDdlPrimitiveType::UInt8:
      OutputString("unsigned_int8", 13);
      break;
    case nsOpenDdlPrimitiveType::UInt16:
      OutputString("unsigned_int16", 14);
      break;
    case nsOpenDdlPrimitiveType::UInt32:
      OutputString("unsigned_int32", 14);
      break;
    case nsOpenDdlPrimitiveType::UInt64:
      OutputString("unsigned_int64", 14);
      break;
    case nsOpenDdlPrimitiveType::Float:
      OutputString("float", 5);
      break;
    case nsOpenDdlPrimitiveType::Double:
      OutputString("double", 6);
      break;
    case nsOpenDdlPrimitiveType::String:
      OutputString("string", 6);
      break;

    default:
      NS_REPORT_FAILURE("Unknown DDL primitive type {0}", (nsUInt32)type);
      break;
  }
}
void nsOpenDdlWriter::OutputPrimitiveTypeNameShort(nsOpenDdlPrimitiveType type)
{
  // Change to OpenDDL: We write uint8 etc. instead of unsigned_int

  switch (type)
  {
    case nsOpenDdlPrimitiveType::Bool:
      OutputString("bool", 4);
      break;
    case nsOpenDdlPrimitiveType::Int8:
      OutputString("int8", 4);
      break;
    case nsOpenDdlPrimitiveType::Int16:
      OutputString("int16", 5);
      break;
    case nsOpenDdlPrimitiveType::Int32:
      OutputString("int32", 5);
      break;
    case nsOpenDdlPrimitiveType::Int64:
      OutputString("int64", 5);
      break;
    case nsOpenDdlPrimitiveType::UInt8:
      OutputString("uint8", 5);
      break;
    case nsOpenDdlPrimitiveType::UInt16:
      OutputString("uint16", 6);
      break;
    case nsOpenDdlPrimitiveType::UInt32:
      OutputString("uint32", 6);
      break;
    case nsOpenDdlPrimitiveType::UInt64:
      OutputString("uint64", 6);
      break;
    case nsOpenDdlPrimitiveType::Float:
      OutputString("float", 5);
      break;
    case nsOpenDdlPrimitiveType::Double:
      OutputString("double", 6);
      break;
    case nsOpenDdlPrimitiveType::String:
      OutputString("string", 6);
      break;

    default:
      NS_REPORT_FAILURE("Unknown DDL primitive type {0}", (nsUInt32)type);
      break;
  }
}

void nsOpenDdlWriter::OutputPrimitiveTypeNameShortest(nsOpenDdlPrimitiveType type)
{
  // Change to OpenDDL: We write super short type strings

  switch (type)
  {
    case nsOpenDdlPrimitiveType::Bool:
      OutputString("b", 1);
      break;
    case nsOpenDdlPrimitiveType::Int8:
      OutputString("i1", 2);
      break;
    case nsOpenDdlPrimitiveType::Int16:
      OutputString("i2", 2);
      break;
    case nsOpenDdlPrimitiveType::Int32:
      OutputString("i3", 2);
      break;
    case nsOpenDdlPrimitiveType::Int64:
      OutputString("i4", 2);
      break;
    case nsOpenDdlPrimitiveType::UInt8:
      OutputString("u1", 2);
      break;
    case nsOpenDdlPrimitiveType::UInt16:
      OutputString("u2", 2);
      break;
    case nsOpenDdlPrimitiveType::UInt32:
      OutputString("u3", 2);
      break;
    case nsOpenDdlPrimitiveType::UInt64:
      OutputString("u4", 2);
      break;
    case nsOpenDdlPrimitiveType::Float:
      OutputString("f", 1);
      break;
    case nsOpenDdlPrimitiveType::Double:
      OutputString("d", 1);
      break;
    case nsOpenDdlPrimitiveType::String:
      OutputString("s", 1);
      break;

    default:
      NS_REPORT_FAILURE("Unknown DDL primitive type {0}", (nsUInt32)type);
      break;
  }
}

nsOpenDdlWriter::nsOpenDdlWriter()
{
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesBool == (int)nsOpenDdlPrimitiveType::Bool);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesInt8 == (int)nsOpenDdlPrimitiveType::Int8);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesInt16 == (int)nsOpenDdlPrimitiveType::Int16);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesInt32 == (int)nsOpenDdlPrimitiveType::Int32);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesInt64 == (int)nsOpenDdlPrimitiveType::Int64);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesUInt8 == (int)nsOpenDdlPrimitiveType::UInt8);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesUInt16 == (int)nsOpenDdlPrimitiveType::UInt16);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesUInt32 == (int)nsOpenDdlPrimitiveType::UInt32);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesUInt64 == (int)nsOpenDdlPrimitiveType::UInt64);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesFloat == (int)nsOpenDdlPrimitiveType::Float);
  NS_CHECK_AT_COMPILETIME((int)nsOpenDdlWriter::State::PrimitivesString == (int)nsOpenDdlPrimitiveType::String);

  m_StateStack.ExpandAndGetRef().m_State = State::Invalid;
  m_StateStack.ExpandAndGetRef().m_State = State::Empty;
}

// All,              ///< All whitespace is output. This is the default, it should be used for files that are read by humans.
// LessIndentation,  ///< Saves some space by using less space for indentation
// NoIndentation,    ///< Saves even more space by dropping all indentation from the output. The result will be noticeably less readable.
// NewlinesOnly,     ///< All unnecessary whitespace, except for newlines, is not output.
// None,             ///< No whitespace, not even newlines, is output. This should be used when DDL is used for data exchange, but probably not read
// by humans.

void nsOpenDdlWriter::BeginObject(nsStringView sType, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/, bool bSingleLine /*= false*/)
{
  {
    const auto state = m_StateStack.PeekBack().m_State;
    NS_IGNORE_UNUSED(state);
    NS_ASSERT_DEBUG(state == State::Empty || state == State::ObjectMultiLine || state == State::ObjectStart,
      "DDL Writer is in a state where no further objects may be created");
  }

  OutputObjectBeginning();

  {
    const auto state = m_StateStack.PeekBack().m_State;
    NS_IGNORE_UNUSED(state);
    NS_ASSERT_DEBUG(state != State::ObjectSingleLine, "Cannot put an object into another single-line object");
    NS_ASSERT_DEBUG(state != State::ObjectStart, "Object beginning should have been written");
  }

  OutputIndentation();
  OutputString(sType);

  OutputObjectName(sName, bGlobalName);

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


void nsOpenDdlWriter::OutputObjectBeginning()
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

bool IsDdlIdentifierCharacter(nsUInt32 uiByte);

void nsOpenDdlWriter::OutputObjectName(nsStringView sName, bool bGlobalName)
{
  if (!sName.IsEmpty())
  {
    // NS_ASSERT_DEBUG(nsStringUtils::FindSubString(szName, " ") == nullptr, "Spaces are not allowed in DDL object names: '{0}'", szName);


    /// \test This code path is untested
    bool bEscape = false;
    for (auto nameIt = sName.GetIteratorFront(); nameIt.IsValid(); ++nameIt)
    {
      if (!IsDdlIdentifierCharacter(nameIt.GetCharacter()))
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

    OutputString(sName);

    if (bEscape)
      OutputString("\'", 1);
  }
}

void nsOpenDdlWriter::EndObject()
{
  const auto state = m_StateStack.PeekBack().m_State;
  NS_ASSERT_DEBUG(state == State::ObjectSingleLine || state == State::ObjectMultiLine || state == State::ObjectStart, "No object is open");

  if (state == State::ObjectStart)
  {
    // object is empty

    OutputString("{}\n", 3);
    m_StateStack.PopBack();

    const auto newState = m_StateStack.PeekBack().m_State;
    NS_IGNORE_UNUSED(newState);
    NS_ASSERT_DEBUG(newState == State::ObjectSingleLine || newState == State::ObjectMultiLine, "No object is open");
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

void nsOpenDdlWriter::BeginPrimitiveList(nsOpenDdlPrimitiveType type, nsStringView sName /*= {}*/, bool bGlobalName /*= false*/)
{
  OutputObjectBeginning();

  const auto state = m_StateStack.PeekBack().m_State;
  NS_ASSERT_DEBUG(state == State::Empty || state == State::ObjectSingleLine || state == State::ObjectMultiLine,
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

  OutputObjectName(sName, bGlobalName);

  // more compact
  // if (m_bCompactMode)
  OutputString("{", 1);
  // else
  // OutputString(" {", 2);

  m_StateStack.ExpandAndGetRef().m_State = static_cast<State>(type);
}

void nsOpenDdlWriter::EndPrimitiveList()
{
  const auto state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEBUG(state >= State::PrimitivesBool && state <= State::PrimitivesString, "No primitive list is open");

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

void nsOpenDdlWriter::WritePrimitiveType(nsOpenDdlWriter::State exp)
{
  auto& state = m_StateStack.PeekBack();
  NS_ASSERT_DEBUG(state.m_State == exp, "Cannot write thie primitive type without have the correct primitive list open");

  if (state.m_bPrimitivesWritten)
  {
    // already wrote some primitives, so append a comma
    OutputString(",", 1);
  }

  state.m_bPrimitivesWritten = true;
}


void nsOpenDdlWriter::WriteBinaryAsHex(const void* pData, nsUInt32 uiBytes)
{
  char tmp[4];

  nsUInt8* pBytes = (nsUInt8*)pData;

  for (nsUInt32 i = 0; i < uiBytes; ++i)
  {
    nsStringUtils::snprintf(tmp, 4, "%02X", (nsUInt32)*pBytes);
    ++pBytes;

    OutputString(tmp, 2);
  }
}

void nsOpenDdlWriter::WriteBool(const bool* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesBool);

  if (m_bCompactMode || m_TypeStringMode == TypeStringMode::Shortest)
  {
    // Extension to OpenDDL: We write only '1' or '0' in compact mode

    if (pValues[0])
      OutputString("1", 1);
    else
      OutputString("0", 1);

    for (nsUInt32 i = 1; i < uiCount; ++i)
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

    for (nsUInt32 i = 1; i < uiCount; ++i)
    {
      if (pValues[i])
        OutputString(",true", 5);
      else
        OutputString(",false", 6);
    }
  }
}

void nsOpenDdlWriter::WriteInt8(const nsInt8* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt8);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteInt16(const nsInt16* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt16);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteInt32(const nsInt32* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt32);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteInt64(const nsInt64* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesInt64);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}


void nsOpenDdlWriter::WriteUInt8(const nsUInt8* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt8);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteUInt16(const nsUInt16* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt16);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteUInt32(const nsUInt32* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt32);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteUInt64(const nsUInt64* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesUInt64);

  m_sTemp.SetFormat("{0}", pValues[0]);
  OutputString(m_sTemp.GetData());

  for (nsUInt32 i = 1; i < uiCount; ++i)
  {
    m_sTemp.SetFormat(",{0}", pValues[i]);
    OutputString(m_sTemp.GetData());
  }
}

void nsOpenDdlWriter::WriteFloat(const float* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesFloat);

  if (m_FloatPrecisionMode == FloatPrecisionMode::Readable)
  {
    m_sTemp.SetFormat("{0}", pValues[0]);
    OutputString(m_sTemp.GetData());

    for (nsUInt32 i = 1; i < uiCount; ++i)
    {
      m_sTemp.SetFormat(",{0}", pValues[i]);
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

    for (nsUInt32 i = 1; i < uiCount; ++i)
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

void nsOpenDdlWriter::WriteDouble(const double* pValues, nsUInt32 uiCount /*= 1*/)
{
  NS_ASSERT_DEBUG(pValues != nullptr, "Invalid value array");
  NS_ASSERT_DEBUG(uiCount > 0, "This is pointless");

  WritePrimitiveType(State::PrimitivesDouble);

  if (m_FloatPrecisionMode == FloatPrecisionMode::Readable)
  {
    m_sTemp.SetFormat("{0}", pValues[0]);
    OutputString(m_sTemp.GetData());

    for (nsUInt32 i = 1; i < uiCount; ++i)
    {
      m_sTemp.SetFormat(",{0}", pValues[i]);
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

    for (nsUInt32 i = 1; i < uiCount; ++i)
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

void nsOpenDdlWriter::WriteString(const nsStringView& sString)
{
  WritePrimitiveType(State::PrimitivesString);

  OutputEscapedString(sString);
}

void nsOpenDdlWriter::WriteBinaryAsString(const void* pData, nsUInt32 uiBytes)
{
  /// \test nsOpenDdlWriter::WriteBinaryAsString

  WritePrimitiveType(State::PrimitivesString);

  OutputString("\"", 1);
  WriteBinaryAsHex(pData, uiBytes);
  OutputString("\"", 1);
}
