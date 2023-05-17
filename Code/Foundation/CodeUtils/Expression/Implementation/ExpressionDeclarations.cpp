#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/SimdMath/SimdNoise.h>
#include <Foundation/SimdMath/SimdRandom.h>

using namespace wdExpression;

namespace
{
  static const char* s_szRegisterTypeNames[] = {
    "Unknown",
    "Bool",
    "Int",
    "Float",
  };

  static_assert(WD_ARRAY_SIZE(s_szRegisterTypeNames) == RegisterType::Count);

  static const char* s_szRegisterTypeNamesShort[] = {
    "U",
    "B",
    "I",
    "F",
  };

  static_assert(WD_ARRAY_SIZE(s_szRegisterTypeNamesShort) == RegisterType::Count);

  static_assert(RegisterType::Count <= WD_BIT(RegisterType::MaxNumBits));
} // namespace

// static
const char* RegisterType::GetName(Enum registerType)
{
  WD_ASSERT_DEBUG(registerType >= 0 && registerType < WD_ARRAY_SIZE(s_szRegisterTypeNames), "Out of bounds access");
  return s_szRegisterTypeNames[registerType];
}

//////////////////////////////////////////////////////////////////////////

wdResult StreamDesc::Serialize(wdStreamWriter& inout_stream) const
{
  inout_stream << m_sName;
  inout_stream << static_cast<wdUInt8>(m_DataType);

  return WD_SUCCESS;
}

wdResult StreamDesc::Deserialize(wdStreamReader& inout_stream)
{
  inout_stream >> m_sName;

  wdUInt8 dataType = 0;
  inout_stream >> dataType;
  m_DataType = static_cast<wdProcessingStream::DataType>(dataType);

  return WD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

bool FunctionDesc::operator<(const FunctionDesc& other) const
{
  if (m_sName != other.m_sName)
    return m_sName < other.m_sName;

  if (m_uiNumRequiredInputs != other.m_uiNumRequiredInputs)
    return m_uiNumRequiredInputs < other.m_uiNumRequiredInputs;

  if (m_OutputType != other.m_OutputType)
    return m_OutputType < other.m_OutputType;

  return m_InputTypes.GetArrayPtr() < other.m_InputTypes.GetArrayPtr();
}

wdResult FunctionDesc::Serialize(wdStreamWriter& inout_stream) const
{
  inout_stream << m_sName;
  WD_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_InputTypes));
  inout_stream << m_uiNumRequiredInputs;
  inout_stream << m_OutputType;

  return WD_SUCCESS;
}

wdResult FunctionDesc::Deserialize(wdStreamReader& inout_stream)
{
  inout_stream >> m_sName;
  WD_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_InputTypes));
  inout_stream >> m_uiNumRequiredInputs;
  inout_stream >> m_OutputType;

  return WD_SUCCESS;
}

wdHashedString FunctionDesc::GetMangledName() const
{
  wdStringBuilder sMangledName = m_sName.GetView();
  sMangledName.Append("_");

  for (auto inputType : m_InputTypes)
  {
    sMangledName.Append(s_szRegisterTypeNamesShort[inputType]);
  }

  wdHashedString sResult;
  sResult.Assign(sMangledName);
  return sResult;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  static const wdEnum<RegisterType> s_RandomInputTypes[] = {RegisterType::Int, RegisterType::Int};

  static void Random(Inputs inputs, Output output, const GlobalData& globalData)
  {
    const Register* pPositions = inputs[0].GetPtr();
    const Register* pPositionsEnd = inputs[0].GetEndPtr();
    Register* pOutput = output.GetPtr();

    if (inputs.GetCount() >= 2)
    {
      const Register* pSeeds = inputs[1].GetPtr();

      while (pPositions < pPositionsEnd)
      {
        pOutput->f = wdSimdRandom::FloatZeroToOne(pPositions->i, wdSimdVec4u(pSeeds->i));

        ++pPositions;
        ++pSeeds;
        ++pOutput;
      }
    }
    else
    {
      while (pPositions < pPositionsEnd)
      {
        pOutput->f = wdSimdRandom::FloatZeroToOne(pPositions->i);

        ++pPositions;
        ++pOutput;
      }
    }
  }

  static wdSimdPerlinNoise s_PerlinNoise(12345);
  static const wdEnum<RegisterType> s_PerlinNoiseInputTypes[] = {
    RegisterType::Float,
    RegisterType::Float,
    RegisterType::Float,
    RegisterType::Int,
  };

  static void PerlinNoise(Inputs inputs, Output output, const GlobalData& globalData)
  {
    const Register* pPosX = inputs[0].GetPtr();
    const Register* pPosY = inputs[1].GetPtr();
    const Register* pPosZ = inputs[2].GetPtr();
    const Register* pPosXEnd = inputs[0].GetEndPtr();

    const wdUInt32 uiNumOctaves = (inputs.GetCount() >= 4) ? inputs[3][0].i.x() : 1;

    Register* pOutput = output.GetPtr();

    while (pPosX < pPosXEnd)
    {
      pOutput->f = s_PerlinNoise.NoiseZeroToOne(pPosX->f, pPosY->f, pPosZ->f, uiNumOctaves);

      ++pPosX;
      ++pPosY;
      ++pPosZ;
      ++pOutput;
    }
  }
} // namespace

wdExpressionFunction wdDefaultExpressionFunctions::s_RandomFunc = {
  {wdMakeHashedString("Random"), wdMakeArrayPtr(s_RandomInputTypes), 1, RegisterType::Float},
  &Random,
};

wdExpressionFunction wdDefaultExpressionFunctions::s_PerlinNoiseFunc = {
  {wdMakeHashedString("PerlinNoise"), wdMakeArrayPtr(s_PerlinNoiseInputTypes), 3, RegisterType::Float},
  &PerlinNoise,
};


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionDeclarations);
