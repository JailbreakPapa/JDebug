#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONWriter.h>

wdJSONWriter::wdJSONWriter() = default;
wdJSONWriter::~wdJSONWriter() = default;

void wdJSONWriter::AddVariableBool(const char* szName, bool value)
{
  BeginVariable(szName);
  WriteBool(value);
  EndVariable();
}

void wdJSONWriter::AddVariableInt32(const char* szName, wdInt32 value)
{
  BeginVariable(szName);
  WriteInt32(value);
  EndVariable();
}

void wdJSONWriter::AddVariableUInt32(const char* szName, wdUInt32 value)
{
  BeginVariable(szName);
  WriteUInt32(value);
  EndVariable();
}

void wdJSONWriter::AddVariableInt64(const char* szName, wdInt64 value)
{
  BeginVariable(szName);
  WriteInt64(value);
  EndVariable();
}

void wdJSONWriter::AddVariableUInt64(const char* szName, wdUInt64 value)
{
  BeginVariable(szName);
  WriteUInt64(value);
  EndVariable();
}

void wdJSONWriter::AddVariableFloat(const char* szName, float value)
{
  BeginVariable(szName);
  WriteFloat(value);
  EndVariable();
}

void wdJSONWriter::AddVariableDouble(const char* szName, double value)
{
  BeginVariable(szName);
  WriteDouble(value);
  EndVariable();
}

void wdJSONWriter::AddVariableString(const char* szName, wdStringView value)
{
  BeginVariable(szName);
  WriteString(value);
  EndVariable();
}

void wdJSONWriter::AddVariableNULL(const char* szName)
{
  BeginVariable(szName);
  WriteNULL();
  EndVariable();
}

void wdJSONWriter::AddVariableTime(const char* szName, wdTime value)
{
  BeginVariable(szName);
  WriteTime(value);
  EndVariable();
}

void wdJSONWriter::AddVariableUuid(const char* szName, wdUuid value)
{
  BeginVariable(szName);
  WriteUuid(value);
  EndVariable();
}

void wdJSONWriter::AddVariableAngle(const char* szName, wdAngle value)
{
  BeginVariable(szName);
  WriteAngle(value);
  EndVariable();
}

void wdJSONWriter::AddVariableColor(const char* szName, const wdColor& value)
{
  BeginVariable(szName);
  WriteColor(value);
  EndVariable();
}

void wdJSONWriter::AddVariableColorGamma(const char* szName, const wdColorGammaUB& value)
{
  BeginVariable(szName);
  WriteColorGamma(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec2(const char* szName, const wdVec2& value)
{
  BeginVariable(szName);
  WriteVec2(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec3(const char* szName, const wdVec3& value)
{
  BeginVariable(szName);
  WriteVec3(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec4(const char* szName, const wdVec4& value)
{
  BeginVariable(szName);
  WriteVec4(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec2I32(const char* szName, const wdVec2I32& value)
{
  BeginVariable(szName);
  WriteVec2I32(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec3I32(const char* szName, const wdVec3I32& value)
{
  BeginVariable(szName);
  WriteVec3I32(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVec4I32(const char* szName, const wdVec4I32& value)
{
  BeginVariable(szName);
  WriteVec4I32(value);
  EndVariable();
}

void wdJSONWriter::AddVariableQuat(const char* szName, const wdQuat& value)
{
  BeginVariable(szName);
  WriteQuat(value);
  EndVariable();
}

void wdJSONWriter::AddVariableMat3(const char* szName, const wdMat3& value)
{
  BeginVariable(szName);
  WriteMat3(value);
  EndVariable();
}

void wdJSONWriter::AddVariableMat4(const char* szName, const wdMat4& value)
{
  BeginVariable(szName);
  WriteMat4(value);
  EndVariable();
}

void wdJSONWriter::AddVariableDataBuffer(const char* szName, const wdDataBuffer& value)
{
  BeginVariable(szName);
  WriteDataBuffer(value);
  EndVariable();
}

void wdJSONWriter::AddVariableVariant(const char* szName, const wdVariant& value)
{
  BeginVariable(szName);
  WriteVariant(value);
  EndVariable();
}

void wdJSONWriter::WriteColor(const wdColor& value)
{
  WD_REPORT_FAILURE("The complex data type wdColor is not supported by this JSON writer.");
}

void wdJSONWriter::WriteColorGamma(const wdColorGammaUB& value)
{
  WD_REPORT_FAILURE("The complex data type wdColorGammaUB is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec2(const wdVec2& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec2 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec3(const wdVec3& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec3 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec4(const wdVec4& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec4 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec2I32(const wdVec2I32& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec2I32 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec3I32(const wdVec3I32& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec3I32 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVec4I32(const wdVec4I32& value)
{
  WD_REPORT_FAILURE("The complex data type wdVec4I32 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteQuat(const wdQuat& value)
{
  WD_REPORT_FAILURE("The complex data type wdQuat is not supported by this JSON writer.");
}

void wdJSONWriter::WriteMat3(const wdMat3& value)
{
  WD_REPORT_FAILURE("The complex data type wdMat3 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteMat4(const wdMat4& value)
{
  WD_REPORT_FAILURE("The complex data type wdMat4 is not supported by this JSON writer.");
}

void wdJSONWriter::WriteDataBuffer(const wdDataBuffer& value)
{
  WD_REPORT_FAILURE("The complex data type wdDateBuffer is not supported by this JSON writer.");
}

void wdJSONWriter::WriteVariant(const wdVariant& value)
{
  switch (value.GetType())
  {
    case wdVariant::Type::Invalid:
      // WD_REPORT_FAILURE("Variant of Type 'Invalid' cannot be written as JSON.");
      WriteNULL();
      return;
    case wdVariant::Type::Bool:
      WriteBool(value.Get<bool>());
      return;
    case wdVariant::Type::Int8:
      WriteInt32(value.Get<wdInt8>());
      return;
    case wdVariant::Type::UInt8:
      WriteUInt32(value.Get<wdUInt8>());
      return;
    case wdVariant::Type::Int16:
      WriteInt32(value.Get<wdInt16>());
      return;
    case wdVariant::Type::UInt16:
      WriteUInt32(value.Get<wdUInt16>());
      return;
    case wdVariant::Type::Int32:
      WriteInt32(value.Get<wdInt32>());
      return;
    case wdVariant::Type::UInt32:
      WriteUInt32(value.Get<wdUInt32>());
      return;
    case wdVariant::Type::Int64:
      WriteInt64(value.Get<wdInt64>());
      return;
    case wdVariant::Type::UInt64:
      WriteUInt64(value.Get<wdUInt64>());
      return;
    case wdVariant::Type::Float:
      WriteFloat(value.Get<float>());
      return;
    case wdVariant::Type::Double:
      WriteDouble(value.Get<double>());
      return;
    case wdVariant::Type::Color:
      WriteColor(value.Get<wdColor>());
      return;
    case wdVariant::Type::ColorGamma:
      WriteColorGamma(value.Get<wdColorGammaUB>());
      return;
    case wdVariant::Type::Vector2:
      WriteVec2(value.Get<wdVec2>());
      return;
    case wdVariant::Type::Vector3:
      WriteVec3(value.Get<wdVec3>());
      return;
    case wdVariant::Type::Vector4:
      WriteVec4(value.Get<wdVec4>());
      return;
    case wdVariant::Type::Vector2I:
      WriteVec2I32(value.Get<wdVec2I32>());
      return;
    case wdVariant::Type::Vector3I:
      WriteVec3I32(value.Get<wdVec3I32>());
      return;
    case wdVariant::Type::Vector4I:
      WriteVec4I32(value.Get<wdVec4I32>());
      return;
    case wdVariant::Type::Quaternion:
      WriteQuat(value.Get<wdQuat>());
      return;
    case wdVariant::Type::Matrix3:
      WriteMat3(value.Get<wdMat3>());
      return;
    case wdVariant::Type::Matrix4:
      WriteMat4(value.Get<wdMat4>());
      return;
    case wdVariant::Type::String:
      WriteString(value.Get<wdString>().GetData());
      return;
    case wdVariant::Type::StringView:
    {
      wdStringBuilder s = value.Get<wdStringView>();
      WriteString(s.GetData());
      return;
    }
    case wdVariant::Type::Time:
      WriteTime(value.Get<wdTime>());
      return;
    case wdVariant::Type::Uuid:
      WriteUuid(value.Get<wdUuid>());
      return;
    case wdVariant::Type::Angle:
      WriteAngle(value.Get<wdAngle>());
      return;
    case wdVariant::Type::DataBuffer:
      WriteDataBuffer(value.Get<wdDataBuffer>());
      return;
    case wdVariant::Type::VariantArray:
    {
      BeginArray();

      const auto& ar = value.Get<wdVariantArray>();

      for (const auto& val : ar)
      {
        WriteVariant(val);
      }

      EndArray();
    }
      return;

    default:
      break;
  }

  WD_REPORT_FAILURE("The Variant Type {0} is not supported by wdJSONWriter::WriteVariant.", value.GetType());
}


bool wdJSONWriter::HadWriteError() const
{
  return m_bHadWriteError;
}

void wdJSONWriter::SetWriteErrorState()
{
  m_bHadWriteError = true;
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_JSONWriter);
