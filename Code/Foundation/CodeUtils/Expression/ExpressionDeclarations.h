#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/SmallArray.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <Foundation/SimdMath/SimdVec4i.h>
#include <Foundation/Types/Variant.h>

class wdStreamWriter;
class wdStreamReader;

namespace wdExpression
{
  struct Register
  {
    WD_DECLARE_POD_TYPE();

    Register() {}

    union
    {
      wdSimdVec4b b;
      wdSimdVec4i i;
      wdSimdVec4f f;
    };
  };

  struct RegisterType
  {
    using StorageType = wdUInt8;

    enum Enum
    {
      Unknown,

      Bool,
      Int,
      Float,

      Count,

      Default = Float,
      MaxNumBits = 4,
    };

    static const char* GetName(Enum registerType);
  };

  using Output = wdArrayPtr<Register>;
  using Inputs = wdArrayPtr<wdArrayPtr<const Register>>; // Inputs are in SOA form, means inner array contains all values for one input parameter, one for each instance.
  using GlobalData = wdHashTable<wdHashedString, wdVariant>;

  /// \brief Describes an input or output stream for a expression VM
  struct StreamDesc
  {
    wdHashedString m_sName;
    wdProcessingStream::DataType m_DataType;

    bool operator==(const StreamDesc& other) const
    {
      return m_sName == other.m_sName && m_DataType == other.m_DataType;
    }

    wdResult Serialize(wdStreamWriter& inout_stream) const;
    wdResult Deserialize(wdStreamReader& inout_stream);
  };

  /// \brief Describes an expression function and its signature, e.g. how many input parameter it has and their type
  struct FunctionDesc
  {
    wdHashedString m_sName;
    wdSmallArray<wdEnum<wdExpression::RegisterType>, 8> m_InputTypes;
    wdUInt8 m_uiNumRequiredInputs = 0;
    wdEnum<wdExpression::RegisterType> m_OutputType;

    bool operator==(const FunctionDesc& other) const
    {
      return m_sName == other.m_sName &&
             m_InputTypes == other.m_InputTypes &&
             m_uiNumRequiredInputs == other.m_uiNumRequiredInputs &&
             m_OutputType == other.m_OutputType;
    }

    bool operator<(const FunctionDesc& other) const;

    wdResult Serialize(wdStreamWriter& inout_stream) const;
    wdResult Deserialize(wdStreamReader& inout_stream);

    wdHashedString GetMangledName() const;
  };

  using Function = void (*)(wdExpression::Inputs, wdExpression::Output, const wdExpression::GlobalData&);
  using ValidateGlobalDataFunction = wdResult (*)(const wdExpression::GlobalData&);

} // namespace wdExpression

/// \brief Describes an external function that can be called in expressions.
///  These functions need to be state-less and thread-safe.
struct wdExpressionFunction
{
  wdExpression::FunctionDesc m_Desc;

  wdExpression::Function m_Func;

  // Optional validation function used to validate required global data for an expression function
  wdExpression::ValidateGlobalDataFunction m_ValidateGlobalDataFunc;
};

struct WD_FOUNDATION_DLL wdDefaultExpressionFunctions
{
  static wdExpressionFunction s_RandomFunc;
  static wdExpressionFunction s_PerlinNoiseFunc;
};
