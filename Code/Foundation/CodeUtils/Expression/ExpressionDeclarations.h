#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/SmallArray.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <Foundation/SimdMath/SimdVec4i.h>
#include <Foundation/Types/Variant.h>

class nsStreamWriter;
class nsStreamReader;

namespace nsExpression
{
  struct Register
  {
    NS_DECLARE_POD_TYPE();

    Register(){}; // NOLINT: using = default doesn't work here.

    union
    {
      nsSimdVec4b b;
      nsSimdVec4i i;
      nsSimdVec4f f;
    };
  };

  struct RegisterType
  {
    using StorageType = nsUInt8;

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

  using Output = nsArrayPtr<Register>;
  using Inputs = nsArrayPtr<nsArrayPtr<const Register>>; // Inputs are in SOA form, means inner array contains all values for one input parameter, one for each instance.
  using GlobalData = nsHashTable<nsHashedString, nsVariant>;

  /// \brief Describes an input or output stream for a expression VM
  struct StreamDesc
  {
    nsHashedString m_sName;
    nsProcessingStream::DataType m_DataType;

    bool operator==(const StreamDesc& other) const
    {
      return m_sName == other.m_sName && m_DataType == other.m_DataType;
    }

    nsResult Serialize(nsStreamWriter& inout_stream) const;
    nsResult Deserialize(nsStreamReader& inout_stream);
  };

  /// \brief Describes an expression function and its signature, e.g. how many input parameter it has and their type
  struct FunctionDesc
  {
    nsHashedString m_sName;
    nsSmallArray<nsEnum<nsExpression::RegisterType>, 8> m_InputTypes;
    nsUInt8 m_uiNumRequiredInputs = 0;
    nsEnum<nsExpression::RegisterType> m_OutputType;

    bool operator==(const FunctionDesc& other) const
    {
      return m_sName == other.m_sName &&
             m_InputTypes == other.m_InputTypes &&
             m_uiNumRequiredInputs == other.m_uiNumRequiredInputs &&
             m_OutputType == other.m_OutputType;
    }

    bool operator<(const FunctionDesc& other) const;

    nsResult Serialize(nsStreamWriter& inout_stream) const;
    nsResult Deserialize(nsStreamReader& inout_stream);

    nsHashedString GetMangledName() const;
  };

  using Function = void (*)(nsExpression::Inputs, nsExpression::Output, const nsExpression::GlobalData&);
  using ValidateGlobalDataFunction = nsResult (*)(const nsExpression::GlobalData&);

} // namespace nsExpression

/// \brief Describes an external function that can be called in expressions.
///  These functions need to be state-less and thread-safe.
struct nsExpressionFunction
{
  nsExpression::FunctionDesc m_Desc;

  nsExpression::Function m_Func;

  // Optional validation function used to validate required global data for an expression function
  nsExpression::ValidateGlobalDataFunction m_ValidateGlobalDataFunc;
};

struct NS_FOUNDATION_DLL nsDefaultExpressionFunctions
{
  static nsExpressionFunction s_RandomFunc;
  static nsExpressionFunction s_PerlinNoiseFunc;
};

/// \brief Add this attribute a string property that should be interpreted as expression source.
///
/// The Inputs/Outputs property reference another array property on the same object that contains objects
/// with a name and a type property that can be used for real time error checking of the expression source.
class NS_FOUNDATION_DLL nsExpressionWidgetAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsExpressionWidgetAttribute, nsTypeWidgetAttribute);

public:
  nsExpressionWidgetAttribute() = default;
  nsExpressionWidgetAttribute(const char* szInputsProperty, const char* szOutputProperty)
    : m_sInputsProperty(szInputsProperty)
    , m_sOutputsProperty(szOutputProperty)
  {
  }

  const char* GetInputsProperty() const { return m_sInputsProperty; }
  const char* GetOutputsProperty() const { return m_sOutputsProperty; }

private:
  nsUntrackedString m_sInputsProperty;
  nsUntrackedString m_sOutputsProperty;
};
