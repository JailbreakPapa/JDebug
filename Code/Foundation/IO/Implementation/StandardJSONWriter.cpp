#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONWriter.h>

nsStandardJSONWriter::JSONState::JSONState()
{
  m_State = Invalid;
  m_bRequireComma = false;
  m_bValueWasWritten = false;
}

nsStandardJSONWriter::CommaWriter::CommaWriter(nsStandardJSONWriter* pWriter)
{
  const nsStandardJSONWriter::State state = pWriter->m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV(state == nsStandardJSONWriter::Array || state == nsStandardJSONWriter::NamedArray || state == nsStandardJSONWriter::Variable,
    "Values can only be written inside BeginVariable() / EndVariable() and BeginArray() / EndArray().");

  m_pWriter = pWriter;

  if (m_pWriter->m_StateStack.PeekBack().m_bRequireComma)
  {
    // we are writing the comma now, so it is not required anymore
    m_pWriter->m_StateStack.PeekBack().m_bRequireComma = false;

    if (m_pWriter->m_StateStack.PeekBack().m_State == nsStandardJSONWriter::Array ||
        m_pWriter->m_StateStack.PeekBack().m_State == nsStandardJSONWriter::NamedArray)
    {
      if (pWriter->m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
      {
        if (pWriter->m_ArrayMode == nsJSONWriter::ArrayMode::InOneLine)
          m_pWriter->OutputString(",");
        else
          m_pWriter->OutputString(",\n");
      }
      else
      {
        if (pWriter->m_ArrayMode == nsJSONWriter::ArrayMode::InOneLine)
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
      if (pWriter->m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::None)
        m_pWriter->OutputString(",");
      else
        m_pWriter->OutputString(",\n");

      m_pWriter->OutputIndentation();
    }
  }
}

nsStandardJSONWriter::CommaWriter::~CommaWriter()
{
  m_pWriter->m_StateStack.PeekBack().m_bRequireComma = true;
  m_pWriter->m_StateStack.PeekBack().m_bValueWasWritten = true;
}

nsStandardJSONWriter::nsStandardJSONWriter()
{
  m_iIndentation = 0;
  m_pOutput = nullptr;
  JSONState s;
  s.m_State = nsStandardJSONWriter::Empty;
  m_StateStack.PushBack(s);
}

nsStandardJSONWriter::~nsStandardJSONWriter()
{
  if (!HadWriteError())
  {
    NS_ASSERT_DEV(m_StateStack.PeekBack().m_State == nsStandardJSONWriter::Empty, "The JSON stream must be closed properly.");
  }
}

void nsStandardJSONWriter::SetOutputStream(nsStreamWriter* pOutput)
{
  m_pOutput = pOutput;
}

void nsStandardJSONWriter::OutputString(nsStringView s)
{
  NS_ASSERT_DEBUG(m_pOutput != nullptr, "No output stream has been set yet.");

  if (m_pOutput->WriteBytes(s.GetStartPointer(), s.GetElementCount()).Failed())
  {
    SetWriteErrorState();
  }
}

void nsStandardJSONWriter::OutputEscapedString(nsStringView s)
{
  nsStringBuilder sEscaped = s;
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

void nsStandardJSONWriter::OutputIndentation()
{
  if (m_WhitespaceMode >= WhitespaceMode::NoIndentation)
    return;

  nsInt32 iIndentation = m_iIndentation * 2;

  if (m_WhitespaceMode == WhitespaceMode::LessIndentation)
    iIndentation = m_iIndentation;

  nsStringBuilder s;
  s.SetPrintf("%*s", iIndentation, "");

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteBool(bool value)
{
  CommaWriter cw(this);

  if (value)
    OutputString("true");
  else
    OutputString("false");
}

void nsStandardJSONWriter::WriteInt32(nsInt32 value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteUInt32(nsUInt32 value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteInt64(nsInt64 value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteUInt64(nsUInt64 value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteFloat(float value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteDouble(double value)
{
  CommaWriter cw(this);

  nsStringBuilder s;
  s.SetFormat("{0}", value);

  OutputString(s.GetData());
}

void nsStandardJSONWriter::WriteString(nsStringView value)
{
  CommaWriter cw(this);

  OutputEscapedString(value);
}

void nsStandardJSONWriter::WriteNULL()
{
  CommaWriter cw(this);

  OutputString("null");
}

void nsStandardJSONWriter::WriteTime(nsTime value)
{
  WriteDouble(value.GetSeconds());
}

void nsStandardJSONWriter::WriteColor(const nsColor& value)
{
  nsVec4 temp(value.r, value.g, value.b, value.a);

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2},{3})", nsArgF(value.r, 4), nsArgF(value.g, 4), nsArgF(value.b, 4), nsArgF(value.a, 4));
  else
    s.SetFormat("({0}, {1}, {2}, {3})", nsArgF(value.r, 4), nsArgF(value.g, 4), nsArgF(value.b, 4), nsArgF(value.a, 4));

  WriteBinaryData("color", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteColorGamma(const nsColorGammaUB& value)
{
  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2},{3})", value.r, value.g, value.b, value.a);
  else
    s.SetFormat("({0}, {1}, {2}, {3})", value.r, value.g, value.b, value.a);

  WriteBinaryData("gamma", value.GetData(), sizeof(nsColorGammaUB), s.GetData());
}

void nsStandardJSONWriter::WriteVec2(const nsVec2& value)
{
  nsVec2 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1})", nsArgF(value.x, 4), nsArgF(value.y, 4));
  else
    s.SetFormat("({0}, {1})", nsArgF(value.x, 4), nsArgF(value.y, 4));

  WriteBinaryData("vec2", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteVec3(const nsVec3& value)
{
  nsVec3 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2})", nsArgF(value.x, 4), nsArgF(value.y, 4), nsArgF(value.z, 4));
  else
    s.SetFormat("({0}, {1}, {2})", nsArgF(value.x, 4), nsArgF(value.y, 4), nsArgF(value.z, 4));

  WriteBinaryData("vec3", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteVec4(const nsVec4& value)
{
  nsVec4 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2},{3})", nsArgF(value.x, 4), nsArgF(value.y, 4), nsArgF(value.z, 4), nsArgF(value.w, 4));
  else
    s.SetFormat("({0}, {1}, {2}, {3})", nsArgF(value.x, 4), nsArgF(value.y, 4), nsArgF(value.z, 4), nsArgF(value.w, 4));

  WriteBinaryData("vec4", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteVec2I32(const nsVec2I32& value)
{
  CommaWriter cw(this);

  nsVec2I32 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(nsInt32));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1})", value.x, value.y);
  else
    s.SetFormat("({0}, {1})", value.x, value.y);

  WriteBinaryData("vec2i", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteVec3I32(const nsVec3I32& value)
{
  CommaWriter cw(this);

  nsVec3I32 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(nsInt32));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2})", value.x, value.y, value.z);
  else
    s.SetFormat("({0}, {1}, {2})", value.x, value.y, value.z);

  WriteBinaryData("vec3i", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteVec4I32(const nsVec4I32& value)
{
  CommaWriter cw(this);

  nsVec4I32 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(nsInt32));

  nsStringBuilder s;

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::NewlinesOnly)
    s.SetFormat("({0},{1},{2},{3})", value.x, value.y, value.z, value.w);
  else
    s.SetFormat("({0}, {1}, {2}, {3})", value.x, value.y, value.z, value.w);

  WriteBinaryData("vec4i", &temp, sizeof(temp), s.GetData());
}

void nsStandardJSONWriter::WriteQuat(const nsQuat& value)
{
  nsQuat temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("quat", &temp, sizeof(temp));
}

void nsStandardJSONWriter::WriteMat3(const nsMat3& value)
{
  nsMat3 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("mat3", &temp, sizeof(temp));
}

void nsStandardJSONWriter::WriteMat4(const nsMat4& value)
{
  nsMat4 temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt32*)&temp, sizeof(temp) / sizeof(float));

  WriteBinaryData("mat4", &temp, sizeof(temp));
}

void nsStandardJSONWriter::WriteUuid(const nsUuid& value)
{
  CommaWriter cw(this);

  nsUuid temp = value;

  nsEndianHelper::NativeToLittleEndian((nsUInt64*)&temp, sizeof(temp) / sizeof(nsUInt64));

  WriteBinaryData("uuid", &temp, sizeof(temp));
}

void nsStandardJSONWriter::WriteAngle(nsAngle value)
{
  WriteFloat(value.GetDegree());
}

void nsStandardJSONWriter::WriteDataBuffer(const nsDataBuffer& value)
{
  WriteBinaryData("data", value.GetData(), value.GetCount());
}

void nsStandardJSONWriter::BeginVariable(nsStringView sName)
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV(state == nsStandardJSONWriter::Empty || state == nsStandardJSONWriter::Object || state == nsStandardJSONWriter::NamedObject,
    "Variables can only be written inside objects.");

  if (m_StateStack.PeekBack().m_bRequireComma)
  {
    if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::None)
      OutputString(",");
    else
      OutputString(",\n");

    OutputIndentation();
  }

  OutputEscapedString(sName);

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString(":");
  else
    OutputString(" : ");

  JSONState s;
  s.m_State = nsStandardJSONWriter::Variable;
  m_StateStack.PushBack(s);
}

void nsStandardJSONWriter::EndVariable()
{
  NS_ASSERT_DEV(m_StateStack.PeekBack().m_State == nsStandardJSONWriter::Variable, "EndVariable() must be called in sync with BeginVariable().");
  NS_ASSERT_DEV(m_StateStack.PeekBack().m_bValueWasWritten, "EndVariable() cannot be called without writing any value in between.");

  End();
}

void nsStandardJSONWriter::BeginArray(nsStringView sName)
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV((state == nsStandardJSONWriter::Empty) ||
                  ((state == nsStandardJSONWriter::Object || state == nsStandardJSONWriter::NamedObject) && !sName.IsEmpty()) ||
                  ((state == nsStandardJSONWriter::Array || state == nsStandardJSONWriter::NamedArray) && sName.IsEmpty()) ||
                  (state == nsStandardJSONWriter::Variable && sName == nullptr),
    "Inside objects you can only begin arrays when also giving them a (non-empty) name.\n"
    "Inside arrays you can only nest anonymous arrays, so names are forbidden.\n"
    "Inside variables you cannot specify a name again.");

  if (sName != nullptr)
    BeginVariable(sName);

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
  s.m_State = (sName == nullptr) ? nsStandardJSONWriter::Array : nsStandardJSONWriter::NamedArray;
  m_StateStack.PushBack(s);
  ++m_iIndentation;
}

void nsStandardJSONWriter::EndArray()
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV(
    state == nsStandardJSONWriter::Array || state == nsStandardJSONWriter::NamedArray, "EndArray() must be called in sync with BeginArray().");


  const State CurState = m_StateStack.PeekBack().m_State;

  End();

  if (CurState == nsStandardJSONWriter::NamedArray)
    EndVariable();
}

void nsStandardJSONWriter::BeginObject(nsStringView sName)
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV((state == nsStandardJSONWriter::Empty) ||
                  ((state == nsStandardJSONWriter::Object || state == nsStandardJSONWriter::NamedObject) && !sName.IsEmpty()) ||
                  ((state == nsStandardJSONWriter::Array || state == nsStandardJSONWriter::NamedArray) && sName.IsEmpty()) ||
                  (state == nsStandardJSONWriter::Variable && sName == nullptr),
    "Inside objects you can only begin objects when also giving them a (non-empty) name.\n"
    "Inside arrays you can only nest anonymous objects, so names are forbidden.\n"
    "Inside variables you cannot specify a name again.");

  if (sName != nullptr)
    BeginVariable(sName);

  m_StateStack.PeekBack().m_bValueWasWritten = true;

  if (m_StateStack.PeekBack().m_bRequireComma)
  {
    if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::None)
      OutputString(",");
    else
      OutputString(",\n");

    OutputIndentation();
  }

  if (m_WhitespaceMode >= nsJSONWriter::WhitespaceMode::None)
    OutputString("{");
  else
    OutputString("{\n");

  JSONState s;
  s.m_State = (sName == nullptr) ? nsStandardJSONWriter::Object : nsStandardJSONWriter::NamedObject;
  m_StateStack.PushBack(s);
  ++m_iIndentation;

  OutputIndentation();
}

void nsStandardJSONWriter::EndObject()
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;
  NS_IGNORE_UNUSED(state);
  NS_ASSERT_DEV(
    state == nsStandardJSONWriter::Object || state == nsStandardJSONWriter::NamedObject, "EndObject() must be called in sync with BeginObject().");

  const State CurState = m_StateStack.PeekBack().m_State;

  End();

  if (CurState == nsStandardJSONWriter::NamedObject)
    EndVariable();
}

void nsStandardJSONWriter::End()
{
  const nsStandardJSONWriter::State state = m_StateStack.PeekBack().m_State;

  if (m_StateStack.PeekBack().m_State == nsStandardJSONWriter::Array || m_StateStack.PeekBack().m_State == nsStandardJSONWriter::NamedArray)
  {
    --m_iIndentation;

    if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
      OutputString("]");
    else
      OutputString(" ]");
  }


  m_StateStack.PopBack();
  m_StateStack.PeekBack().m_bRequireComma = true;

  if (state == nsStandardJSONWriter::Object || state == nsStandardJSONWriter::NamedObject)
  {
    --m_iIndentation;

    if (m_WhitespaceMode < nsJSONWriter::WhitespaceMode::None)
      OutputString("\n");

    OutputIndentation();
    OutputString("}");
  }
}


void nsStandardJSONWriter::WriteBinaryData(nsStringView sDataType, const void* pData, nsUInt32 uiBytes, nsStringView sValueString)
{
  CommaWriter cw(this);

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("{\"$t\":\"");
  else
    OutputString("{ \"$t\" : \"");

  OutputString(sDataType);

  if (!sValueString.IsEmpty())
  {
    if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
      OutputString("\",\"$v\":\"");
    else
      OutputString("\", \"$v\" : \"");

    OutputString(sValueString);
  }

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("\",\"$b\":\"0x");
  else
    OutputString("\", \"$b\" : \"0x");

  nsStringBuilder s;

  nsUInt8* pBytes = (nsUInt8*)pData;

  for (nsUInt32 i = 0; i < uiBytes; ++i)
  {
    s.SetFormat("{0}", nsArgU((nsUInt32)*pBytes, 2, true, 16, true));
    ++pBytes;

    OutputString(s.GetData());
  }

  if (m_WhitespaceMode >= WhitespaceMode::NewlinesOnly)
    OutputString("\"}");
  else
    OutputString("\" }");
}
