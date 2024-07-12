#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionDeclarations.h>
#include <Foundation/SimdMath/SimdNoise.h>
#include <Foundation/SimdMath/SimdRandom.h>

using namespace nsExpression;

namespace
{
  static const char* s_szRegisterTypeNames[] = {
    "Unknown",
    "Bool",
    "Int",
    "Float",
  };

  static_assert(NS_ARRAY_SIZE(s_szRegisterTypeNames) == RegisterType::Count);

  static const char* s_szRegisterTypeNamesShort[] = {
    "U",
    "B",
    "I",
    "F",
  };

  static_assert(NS_ARRAY_SIZE(s_szRegisterTypeNamesShort) == RegisterType::Count);

  static_assert(RegisterType::Count <= NS_BIT(RegisterType::MaxNumBits));
} // namespace

// static
const char* RegisterType::GetName(Enum registerType)
{
  NS_ASSERT_DEBUG(registerType >= 0 && registerType < NS_ARRAY_SIZE(s_szRegisterTypeNames), "Out of bounds access");
  return s_szRegisterTypeNames[registerType];
}

//////////////////////////////////////////////////////////////////////////

nsResult StreamDesc::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_sName;
  inout_stream << static_cast<nsUInt8>(m_DataType);

  return NS_SUCCESS;
}

nsResult StreamDesc::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream >> m_sName;

  nsUInt8 dataType = 0;
  inout_stream >> dataType;
  m_DataType = static_cast<nsProcessingStream::DataType>(dataType);

  return NS_SUCCESS;
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

nsResult FunctionDesc::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_sName;
  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_InputTypes));
  inout_stream << m_uiNumRequiredInputs;
  inout_stream << m_OutputType;

  return NS_SUCCESS;
}

nsResult FunctionDesc::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream >> m_sName;
  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_InputTypes));
  inout_stream >> m_uiNumRequiredInputs;
  inout_stream >> m_OutputType;

  return NS_SUCCESS;
}

nsHashedString FunctionDesc::GetMangledName() const
{
  nsStringBuilder sMangledName = m_sName.GetView();
  sMangledName.Append("_");

  for (auto inputType : m_InputTypes)
  {
    sMangledName.Append(s_szRegisterTypeNamesShort[inputType]);
  }

  nsHashedString sResult;
  sResult.Assign(sMangledName);
  return sResult;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  static const nsEnum<RegisterType> s_RandomInputTypes[] = {RegisterType::Int, RegisterType::Int};

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
        pOutput->f = nsSimdRandom::FloatZeroToOne(pPositions->i, nsSimdVec4u(pSeeds->i));

        ++pPositions;
        ++pSeeds;
        ++pOutput;
      }
    }
    else
    {
      while (pPositions < pPositionsEnd)
      {
        pOutput->f = nsSimdRandom::FloatZeroToOne(pPositions->i);

        ++pPositions;
        ++pOutput;
      }
    }
  }

  static nsSimdPerlinNoise s_PerlinNoise(12345);
  static const nsEnum<RegisterType> s_PerlinNoiseInputTypes[] = {
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

    const nsUInt32 uiNumOctaves = (inputs.GetCount() >= 4) ? inputs[3][0].i.x() : 1;

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

nsExpressionFunction nsDefaultExpressionFunctions::s_RandomFunc = {
  {nsMakeHashedString("Random"), nsMakeArrayPtr(s_RandomInputTypes), 1, RegisterType::Float},
  &Random,
};

nsExpressionFunction nsDefaultExpressionFunctions::s_PerlinNoiseFunc = {
  {nsMakeHashedString("PerlinNoise"), nsMakeArrayPtr(s_PerlinNoiseInputTypes), 3, RegisterType::Float},
  &PerlinNoise,
};

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExpressionWidgetAttribute, 1, nsRTTIDefaultAllocator<nsExpressionWidgetAttribute>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("InputsProperty", m_sInputsProperty),
    NS_MEMBER_PROPERTY("OutputsProperty", m_sOutputsProperty),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


NS_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionDeclarations);
