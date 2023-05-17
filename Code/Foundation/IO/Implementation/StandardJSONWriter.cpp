#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONWriter.h>

wdStandardJSONWriter::JSONState::JSONState()
{
  m_State = Invalid;
  m_bRequireComma = false;
  m_bValueWasWritten = false;
}

wdStandardJSONWriter::CommaWriter::CommaWriter(wdStandardJSONWriter* pWriter)
{
  const wdStandardJSONWriter::State state = pWriter->m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV(state == wdStandardJSONWriter::Array || state == wdStandardJSONWriter::NamedArray || state == wdStandardJSONWriter::Variable,
    "Values can only be written inside BeginVariable() / EndVariable() and BeginArray() / EndArray().");

  m_pWriter = pWriter;

  if (m_pWriter->m_StateStack.PeekBack().m_bRequireComma)
  {
    // we are writing the comma now, so it is not required anymore
    m_pWriter->m_StateStack.PeekBack().m_bRequireComma = false;

    if (m_pWriter->m_StateStack.PeekBack().m_State == wdStandardJSONWriter::Array ||
        m_pWriter->m_StateStack.PeekBack().m_State == wdStandardJSONWriter::NamedArray)
    {
      if (pWriter->m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
      {
        if (pWriter->m_ArrayMode == wdJSONWriter::ArrayMode::InOneLine)
          m_pWriter->OutputString(",");
        else
          m_pWriter->OutputString(",\n");
      }
      else
      {
        if (pWriter->m_ArrayMode == wdJSONWriter::ArrayMode::InOneLine)
          m_pWriter->OutputString(", ");
        else
        {
          m_pWriter->OutputString(",\n");
          m_pWriter->OutputIndentation();
        }
      }
    }
    else
    {
      if (pWriter->m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::None)
        m_pWriter->OutputString(",");
      else
        m_pWriter->OutputString(",\n");

      m_pWriter->OutputIndentation();
    }
  }
}

wdStandardJSONWriter::CommaWriter::~CommaWriter()
{
  m_pWriter->m_StateStack.PeekBack().m_bRequireComma = true;
  m_pWriter->m_StateStack.PeekBack().m_bValueWasWritten = true;
}

wdStandardJSONWriter::wdStandardJSONWriter()
{
  m_iIndentation = 0;
  m_pOutput = nullptr;
  JSONState s;
  s.m_State = wdStandardJSONWriter::Empty;
  m_StateStack.PushBack(s);
}

wdStandardJSONWriter::~wdStandardJSONWriter()
{
  if (!HadWriteError())
  {
    WD_ASSERT_DEV(m_StateStack.PeekBack().m_State == wdStandardJSONWriter::Empty, "The JSON stream must be closed properly.");
  }
}

void wdStandardJSONWriter::SetOutputStream(wdStreamWriter* pOutput)
{
  m_pOutput = pOutput;
}

void wdStandardJSONWriter::OutputString(wdStringView s)
{
  WD_ASSERT_DEBUG(m_pOutput != nullptr, "No output stream has been set yet.");

  if (m_pOutput->WriteBytes(s.GetStartPointer(), s.GetElementCount()).Failed())
  {
    SetWriteErrorState();
  }
}

void wdStandardJSONWriter::OutputEscapedString(wdStringView s)
{
  wdStringBuilder sEscaped = s;
  sEscaped.ReplaceAll("\\", "\\\\");
  // sEscaped.ReplaceAll("/", "\\/"); // this is not necessary to escape
  sEscaped.ReplaceAll("\"", "\\\"");
  sEscaped.ReplaceAll("\b", "\\b");
  sEscaped.ReplaceAll("\r", "\\r");
  sEscaped.ReplaceAll("\f", "\\f");
  sEscaped.ReplaceAll("\n", "\\n");
  sEscaped.ReplaceAll("\t", "\\t");

  OutputString("\"");
  OutputString(sEscaped);
  OutputString("\"");
}

void wdStandardJSONWriter::OutputIndentation()
{
  if (m_WhitespaceMode >= WhitespaceMode::NoIndentation)
    return;

  wdInt32 iIndentation = m_iIndentation * 2;

  if (m_WhitespaceMode == WhitespaceMode::LessIndentation)
    iIndentation = m_iIndentation;

  wdStringBuilder s;
  s.Printf("%*s", iIndentation, "");

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteBool(bool value)
{
  CommaWriter cw(this);

  if (value)
    OutputString("true");
  else
    OutputString("false");
}

void wdStandardJSONWriter::WriteInt32(wdInt32 value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteUInt32(wdUInt32 value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteInt64(wdInt64 value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteUInt64(wdUInt64 value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteFloat(float value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteDouble(double value)
{
  CommaWriter cw(this);

  wdStringBuilder s;
  s.Format("{0}", value);

  OutputString(s.GetData());
}

void wdStandardJSONWriter::WriteString(wdStringView value)
{
  CommaWriter cw(this);

  OutputEscapedString(value);
}

void wdStandardJSONWriter::WriteNULL()
{
  CommaWriter cw(this);

  OutputString("null");
}

void wdStandardJSONWriter::WriteTime(wdTime value)
{
  WriteDouble(value.GetSeconds());
}

void wdStandardJSONWriter::WriteColor(const wdColor& value)
{
  wdVec4 temp(value.r, value.g, value.b, value.a);

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2},{3})", wdArgF(value.r, 4), wdArgF(value.g, 4), wdArgF(value.b, 4), wdArgF(value.a, 4));
  else
    s.Format("({0}, {1}, {2}, {3})", wdArgF(value.r, 4), wdArgF(value.g, 4), wdArgF(value.b, 4), wdArgF(value.a, 4));

  WriteBinaryData("color", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteColorGamma(const wdColorGammaUB& value)
{
  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2},{3})", value.r, value.g, value.b, value.a);
  else
    s.Format("({0}, {1}, {2}, {3})", value.r, value.g, value.b, value.a);

  WriteBinaryData("gamma", value.GetData(), sizeof(wdColorGammaUB), s.GetData());
}

void wdStandardJSONWriter::WriteVec2(const wdVec2& value)
{
  wdVec2 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1})", wdArgF(value.x, 4), wdArgF(value.y, 4));
  else
    s.Format("({0}, {1})", wdArgF(value.x, 4), wdArgF(value.y, 4));

  WriteBinaryData("vec2", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteVec3(const wdVec3& value)
{
  wdVec3 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2})", wdArgF(value.x, 4), wdArgF(value.y, 4), wdArgF(value.z, 4));
  else
    s.Format("({0}, {1}, {2})", wdArgF(value.x, 4), wdArgF(value.y, 4), wdArgF(value.z, 4));

  WriteBinaryData("vec3", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteVec4(const wdVec4& value)
{
  wdVec4 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2},{3})", wdArgF(value.x, 4), wdArgF(value.y, 4), wdArgF(value.z, 4), wdArgF(value.w, 4));
  else
    s.Format("({0}, {1}, {2}, {3})", wdArgF(value.x, 4), wdArgF(value.y, 4), wdArgF(value.z, 4), wdArgF(value.w, 4));

  WriteBinaryData("vec4", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteVec2I32(const wdVec2I32& value)
{
  CommaWriter cw(this);

  wdVec2I32 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(wdInt32));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1})", value.x, value.y);
  else
    s.Format("({0}, {1})", value.x, value.y);

  WriteBinaryData("vec2i", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteVec3I32(const wdVec3I32& value)
{
  CommaWriter cw(this);

  wdVec3I32 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(wdInt32));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2})", value.x, value.y, value.z);
  else
    s.Format("({0}, {1}, {2})", value.x, value.y, value.z);

  WriteBinaryData("vec3i", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteVec4I32(const wdVec4I32& value)
{
  CommaWriter cw(this);

  wdVec4I32 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(wdInt32));

  wdStringBuilder s;

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::NewlinesOnly)
    s.Format("({0},{1},{2},{3})", value.x, value.y, value.z, value.w);
  else
    s.Format("({0}, {1}, {2}, {3})", value.x, value.y, value.z, value.w);

  WriteBinaryData("vec4i", &temp, sizeof(temp), s.GetData());
}

void wdStandardJSONWriter::WriteQuat(const wdQuat& value)
{
  wdQuat temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("quat", &temp, sizeof(temp));
}

void wdStandardJSONWriter::WriteMat3(const wdMat3& value)
{
  wdMat3 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("mat3", &temp, sizeof(temp));
}

void wdStandardJSONWriter::WriteMat4(const wdMat4& value)
{
  wdMat4 temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("mat4", &temp, sizeof(temp));
}

void wdStandardJSONWriter::WriteUuid(const wdUuid& value)
{
  CommaWriter cw(this);

  wdUuid temp = value;

  wdEndianHelper::NativeToLittleEndian((wdUInt64*)&temp, sizeof(temp) / sizeof(wdUInt64));

  WriteBinaryData("uuid", &temp, sizeof(temp));
}

void wdStandardJSONWriter::WriteAngle(wdAngle value)
{
  WriteFloat(value.GetDegree());
}

void wdStandardJSONWriter::WriteDataBuffer(const wdDataBuffer& value)
{
  WriteBinaryData("data", value.GetData(), value.GetCount());
}

void wdStandardJSONWriter::BeginVariable(const char* szName)
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV(state == wdStandardJSONWriter::Empty || state == wdStandardJSONWriter::Object || state == wdStandardJSONWriter::NamedObject,
    "Variables can only be written inside objects.");

  if (m_StateStack.PeekBack().m_bRequireComma)
  {
    if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::None)
      OutputString(",");
    else
      OutputString(",\n");

    OutputIndentation();
  }

  OutputEscapedString(szName);

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString(":");
  else
    OutputString(" : ");

  JSONState s;
  s.m_State = wdStandardJSONWriter::Variable;
  m_StateStack.PushBack(s);
}

void wdStandardJSONWriter::EndVariable()
{
  WD_ASSERT_DEV(m_StateStack.PeekBack().m_State == wdStandardJSONWriter::Variable, "EndVariable() must be called in sync with BeginVariable().");
  WD_ASSERT_DEV(m_StateStack.PeekBack().m_bValueWasWritten, "EndVariable() cannot be called without writing any value in between.");

  End();
}

void wdStandardJSONWriter::BeginArray(const char* szName)
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV((state == wdStandardJSONWriter::Empty) ||
                  ((state == wdStandardJSONWriter::Object || state == wdStandardJSONWriter::NamedObject) && !wdStringUtils::IsNullOrEmpty(szName)) ||
                  ((state == wdStandardJSONWriter::Array || state == wdStandardJSONWriter::NamedArray) && szName == nullptr) ||
                  (state == wdStandardJSONWriter::Variable && szName == nullptr),
    "Inside objects you can only begin arrays when also giving them a (non-empty) name.\n"
    "Inside arrays you can only nest anonymous arrays, so names are forbidden.\n"
    "Inside variables you cannot specify a name again.");

  if (szName != nullptr)
    BeginVariable(szName);

  m_StateStack.PeekBack().m_bValueWasWritten = true;

  if (m_StateStack.PeekBack().m_bRequireComma)
  {
    if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
      OutputString(",");
    else
      OutputString(", ");
  }

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("[");
  else
    OutputString("[ ");

  JSONState s;
  s.m_State = (szName == nullptr) ? wdStandardJSONWriter::Array : wdStandardJSONWriter::NamedArray;
  m_StateStack.PushBack(s);
  ++m_iIndentation;
}

void wdStandardJSONWriter::EndArray()
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV(
    state == wdStandardJSONWriter::Array || state == wdStandardJSONWriter::NamedArray, "EndArray() must be called in sync with BeginArray().");


  const State CurState = m_StateStack.PeekBack().m_State;

  End();

  if (CurState == wdStandardJSONWriter::NamedArray)
    EndVariable();
}

void wdStandardJSONWriter::BeginObject(const char* szName)
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV((state == wdStandardJSONWriter::Empty) ||
                  ((state == wdStandardJSONWriter::Object || state == wdStandardJSONWriter::NamedObject) && !wdStringUtils::IsNullOrEmpty(szName)) ||
                  ((state == wdStandardJSONWriter::Array || state == wdStandardJSONWriter::NamedArray) && szName == nullptr) ||
                  (state == wdStandardJSONWriter::Variable && szName == nullptr),
    "Inside objects you can only begin objects when also giving them a (non-empty) name.\n"
    "Inside arrays you can only nest anonymous objects, so names are forbidden.\n"
    "Inside variables you cannot specify a name again.");

  if (szName != nullptr)
    BeginVariable(szName);

  m_StateStack.PeekBack().m_bValueWasWritten = true;

  if (m_StateStack.PeekBack().m_bRequireComma)
  {
    if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::None)
      OutputString(",");
    else
      OutputString(",\n");

    OutputIndentation();
  }

  if (m_WhitespaceMode >= wdJSONWriter::WhitespaceMode::None)
    OutputString("{");
  else
    OutputString("{\n");

  JSONState s;
  s.m_State = (szName == nullptr) ? wdStandardJSONWriter::Object : wdStandardJSONWriter::NamedObject;
  m_StateStack.PushBack(s);
  ++m_iIndentation;

  OutputIndentation();
}

void wdStandardJSONWriter::EndObject()
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  WD_IGNORE_UNUSED(state);
  WD_ASSERT_DEV(
    state == wdStandardJSONWriter::Object || state == wdStandardJSONWriter::NamedObject, "EndObject() must be called in sync with BeginObject().");

  const State CurState = m_StateStack.PeekBack().m_State;

  End();

  if (CurState == wdStandardJSONWriter::NamedObject)
    EndVariable();
}

void wdStandardJSONWriter::End()
{
  const wdStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;

  if (m_StateStack.PeekBack().m_State == wdStandardJSONWriter::Array || m_StateStack.PeekBack().m_State == wdStandardJSONWriter::NamedArray)
  {
    --m_iIndentation;

    if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
      OutputString("]");
    else
      OutputString(" ]");
  }


  m_StateStack.PopBack();
  m_StateStack.PeekBack().m_bRequireComma = true;

  if (state == wdStandardJSONWriter::Object || state == wdStandardJSONWriter::NamedObject)
  {
    --m_iIndentation;

    if (m_WhitespaceMode < wdJSONWriter::WhitespaceMode::None)
      OutputString("\n");

    OutputIndentation();
    OutputString("}");
  }
}


void wdStandardJSONWriter::WriteBinaryData(const char* szDataType, const void* pData, wdUInt32 uiBytes, const char* szValueString)
{
  CommaWriter cw(this);

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("{\"$t\":\"");
  else
    OutputString("{ \"$t\" : \"");

  OutputString(szDataType);

  if (szValueString != nullptr)
  {
    if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
      OutputString("\",\"$v\":\"");
    else
      OutputString("\", \"$v\" : \"");

    OutputString(szValueString);
  }

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("\",\"$b\":\"0x");
  else
    OutputString("\", \"$b\" : \"0x");

  wdStringBuilder s;

  wdUInt8* pBytes = (wdUInt8*)pData;

  for (wdUInt32 i = 0; i < uiBytes; ++i)
  {
    s.Format("{0}", wdArgU((wdUInt32)*pBytes, 2, true, 16, true));
    ++pBytes;

    OutputString(s.GetData());
  }

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("\"}");
  else
    OutputString("\" }");
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_StandardJSONWriter);
