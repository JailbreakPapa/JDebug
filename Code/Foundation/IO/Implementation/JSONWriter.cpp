#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/JSONWriter.h>

nsJSONWriter::nsJSONWriter() = default;
nsJSONWriter::~nsJSONWriter() = default;

void nsJSONWriter::AddVariableBool(nsStringView sName, bool value)
{
  BeginVariable(sName);
  WriteBool(value);
  EndVariable();
}

void nsJSONWriter::AddVariableInt32(nsStringView sName, nsInt32 value)
{
  BeginVariable(sName);
  WriteInt32(value);
  EndVariable();
}

void nsJSONWriter::AddVariableUInt32(nsStringView sName, nsUInt32 value)
{
  BeginVariable(sName);
  WriteUInt32(value);
  EndVariable();
}

void nsJSONWriter::AddVariableInt64(nsStringView sName, nsInt64 value)
{
  BeginVariable(sName);
  WriteInt64(value);
  EndVariable();
}

void nsJSONWriter::AddVariableUInt64(nsStringView sName, nsUInt64 value)
{
  BeginVariable(sName);
  WriteUInt64(value);
  EndVariable();
}

void nsJSONWriter::AddVariableFloat(nsStringView sName, float value)
{
  BeginVariable(sName);
  WriteFloat(value);
  EndVariable();
}

void nsJSONWriter::AddVariableDouble(nsStringView sName, double value)
{
  BeginVariable(sName);
  WriteDouble(value);
  EndVariable();
}

void nsJSONWriter::AddVariableString(nsStringView sName, nsStringView value)
{
  BeginVariable(sName);
  WriteString(value);
  EndVariable();
}

void nsJSONWriter::AddVariableNULL(nsStringView sName)
{
  BeginVariable(sName);
  WriteNULL();
  EndVariable();
}

void nsJSONWriter::AddVariableTime(nsStringView sName, nsTime value)
{
  BeginVariable(sName);
  WriteTime(value);
  EndVariable();
}

void nsJSONWriter::AddVariableUuid(nsStringView sName, nsUuid value)
{
  BeginVariable(sName);
  WriteUuid(value);
  EndVariable();
}

void nsJSONWriter::AddVariableAngle(nsStringView sName, nsAngle value)
{
  BeginVariable(sName);
  WriteAngle(value);
  EndVariable();
}

void nsJSONWriter::AddVariableColor(nsStringView sName, const nsColor& value)
{
  BeginVariable(sName);
  WriteColor(value);
  EndVariable();
}

void nsJSONWriter::AddVariableColorGamma(nsStringView sName, const nsColorGammaUB& value)
{
  BeginVariable(sName);
  WriteColorGamma(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec2(nsStringView sName, const nsVec2& value)
{
  BeginVariable(sName);
  WriteVec2(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec3(nsStringView sName, const nsVec3& value)
{
  BeginVariable(sName);
  WriteVec3(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec4(nsStringView sName, const nsVec4& value)
{
  BeginVariable(sName);
  WriteVec4(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec2I32(nsStringView sName, const nsVec2I32& value)
{
  BeginVariable(sName);
  WriteVec2I32(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec3I32(nsStringView sName, const nsVec3I32& value)
{
  BeginVariable(sName);
  WriteVec3I32(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVec4I32(nsStringView sName, const nsVec4I32& value)
{
  BeginVariable(sName);
  WriteVec4I32(value);
  EndVariable();
}

void nsJSONWriter::AddVariableQuat(nsStringView sName, const nsQuat& value)
{
  BeginVariable(sName);
  WriteQuat(value);
  EndVariable();
}

void nsJSONWriter::AddVariableMat3(nsStringView sName, const nsMat3& value)
{
  BeginVariable(sName);
  WriteMat3(value);
  EndVariable();
}

void nsJSONWriter::AddVariableMat4(nsStringView sName, const nsMat4& value)
{
  BeginVariable(sName);
  WriteMat4(value);
  EndVariable();
}

void nsJSONWriter::AddVariableDataBuffer(nsStringView sName, const nsDataBuffer& value)
{
  BeginVariable(sName);
  WriteDataBuffer(value);
  EndVariable();
}

void nsJSONWriter::AddVariableVariant(nsStringView sName, const nsVariant& value)
{
  BeginVariable(sName);
  WriteVariant(value);
  EndVariable();
}

void nsJSONWriter::WriteColor(const nsColor& value)
{
  NS_REPORT_FAILURE("The complex data type nsColor is not supported by this JSON writer.");
}

void nsJSONWriter::WriteColorGamma(const nsColorGammaUB& value)
{
  NS_REPORT_FAILURE("The complex data type nsColorGammaUB is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec2(const nsVec2& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec2 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec3(const nsVec3& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec3 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec4(const nsVec4& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec4 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec2I32(const nsVec2I32& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec2I32 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec3I32(const nsVec3I32& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec3I32 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVec4I32(const nsVec4I32& value)
{
  NS_REPORT_FAILURE("The complex data type nsVec4I32 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteQuat(const nsQuat& value)
{
  NS_REPORT_FAILURE("The complex data type nsQuat is not supported by this JSON writer.");
}

void nsJSONWriter::WriteMat3(const nsMat3& value)
{
  NS_REPORT_FAILURE("The complex data type nsMat3 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteMat4(const nsMat4& value)
{
  NS_REPORT_FAILURE("The complex data type nsMat4 is not supported by this JSON writer.");
}

void nsJSONWriter::WriteDataBuffer(const nsDataBuffer& value)
{
  NS_REPORT_FAILURE("The complex data type nsDateBuffer is not supported by this JSON writer.");
}

void nsJSONWriter::WriteVariant(const nsVariant& value)
{
  switch (value.GetType())
  {
    case nsVariant::Type::Invalid:
      // NS_REPORT_FAILURE("Variant of Type 'Invalid' cannot be written as JSON.");
      WriteNULL();
      return;
    case nsVariant::Type::Bool:
      WriteBool(value.Get<bool>());
      return;
    case nsVariant::Type::Int8:
      WriteInt32(value.Get<nsInt8>());
      return;
    case nsVariant::Type::UInt8:
      WriteUInt32(value.Get<nsUInt8>());
      return;
    case nsVariant::Type::Int16:
      WriteInt32(value.Get<nsInt16>());
      return;
    case nsVariant::Type::UInt16:
      WriteUInt32(value.Get<nsUInt16>());
      return;
    case nsVariant::Type::Int32:
      WriteInt32(value.Get<nsInt32>());
      return;
    case nsVariant::Type::UInt32:
      WriteUInt32(value.Get<nsUInt32>());
      return;
    case nsVariant::Type::Int64:
      WriteInt64(value.Get<nsInt64>());
      return;
    case nsVariant::Type::UInt64:
      WriteUInt64(value.Get<nsUInt64>());
      return;
    case nsVariant::Type::Float:
      WriteFloat(value.Get<float>());
      return;
    case nsVariant::Type::Double:
      WriteDouble(value.Get<double>());
      return;
    case nsVariant::Type::Color:
      WriteColor(value.Get<nsColor>());
      return;
    case nsVariant::Type::ColorGamma:
      WriteColorGamma(value.Get<nsColorGammaUB>());
      return;
    case nsVariant::Type::Vector2:
      WriteVec2(value.Get<nsVec2>());
      return;
    case nsVariant::Type::Vector3:
      WriteVec3(value.Get<nsVec3>());
      return;
    case nsVariant::Type::Vector4:
      WriteVec4(value.Get<nsVec4>());
      return;
    case nsVariant::Type::Vector2I:
      WriteVec2I32(value.Get<nsVec2I32>());
      return;
    case nsVariant::Type::Vector3I:
      WriteVec3I32(value.Get<nsVec3I32>());
      return;
    case nsVariant::Type::Vector4I:
      WriteVec4I32(value.Get<nsVec4I32>());
      return;
    case nsVariant::Type::Quaternion:
      WriteQuat(value.Get<nsQuat>());
      return;
    case nsVariant::Type::Matrix3:
      WriteMat3(value.Get<nsMat3>());
      return;
    case nsVariant::Type::Matrix4:
      WriteMat4(value.Get<nsMat4>());
      return;
    case nsVariant::Type::String:
      WriteString(value.Get<nsString>().GetData());
      return;
    case nsVariant::Type::StringView:
    {
      nsStringBuilder s = value.Get<nsStringView>();
      WriteString(s.GetData());
      return;
    }
    case nsVariant::Type::Time:
      WriteTime(value.Get<nsTime>());
      return;
    case nsVariant::Type::Uuid:
      WriteUuid(value.Get<nsUuid>());
      return;
    case nsVariant::Type::Angle:
      WriteAngle(value.Get<nsAngle>());
      return;
    case nsVariant::Type::DataBuffer:
      WriteDataBuffer(value.Get<nsDataBuffer>());
      return;
    case nsVariant::Type::VariantArray:
    {
      BeginArray();

      const auto& ar = value.Get<nsVariantArray>();

      for (const auto& val : ar)
      {
        WriteVariant(val);
      }

      EndArray();
    }
      return;
    case nsVariant::Type::VariantDictionary:
    {
      BeginObject();

      const auto& dict = value.Get<nsVariantDictionary>();

      for (auto& kv : dict)
      {
        AddVariableVariant(kv.Key(), kv.Value());
      }
      EndObject();
    }
      return;

    default:
      break;
  }

  NS_REPORT_FAILURE("The Variant Type {0} is not supported by nsJSONWriter::WriteVariant.", value.GetType());
}


bool nsJSONWriter::HadWriteError() const
{
  return m_bHadWriteError;
}

void nsJSONWriter::SetWriteErrorState()
{
  m_bHadWriteError = true;
}
