#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Utilities/DGMLWriter.h>

// static
bool wdExpressionAST::NodeType::IsUnary(Enum nodeType)
{
  return nodeType > FirstUnary && nodeType < LastUnary;
}

// static
bool wdExpressionAST::NodeType::IsBinary(Enum nodeType)
{
  return nodeType > FirstBinary && nodeType < LastBinary;
}

// static
bool wdExpressionAST::NodeType::IsTernary(Enum nodeType)
{
  return nodeType > FirstTernary && nodeType < LastTernary;
}

// static
bool wdExpressionAST::NodeType::IsConstant(Enum nodeType)
{
  return nodeType == Constant;
}

// static
bool wdExpressionAST::NodeType::IsSwizzle(Enum nodeType)
{
  return nodeType == Swizzle;
}

// static
bool wdExpressionAST::NodeType::IsInput(Enum nodeType)
{
  return nodeType == Input;
}

// static
bool wdExpressionAST::NodeType::IsOutput(Enum nodeType)
{
  return nodeType == Output;
}

// static
bool wdExpressionAST::NodeType::IsFunctionCall(Enum nodeType)
{
  return nodeType == FunctionCall;
}

// static
bool wdExpressionAST::NodeType::IsConstructorCall(Enum nodeType)
{
  return nodeType == ConstructorCall;
}

// static
bool wdExpressionAST::NodeType::IsCommutative(Enum nodeType)
{
  return nodeType == Add || nodeType == Multiply ||
         nodeType == Min || nodeType == Max ||
         nodeType == BitwiseAnd || nodeType == BitwiseXor || nodeType == BitwiseOr ||
         nodeType == Equal || nodeType == NotEqual ||
         nodeType == LogicalAnd || nodeType == LogicalOr;
}

// static
bool wdExpressionAST::NodeType::AlwaysReturnsSingleElement(Enum nodeType)
{
  return nodeType == Length ||
         nodeType == All || nodeType == Any ||
         nodeType == Dot;
}

namespace
{
  static const char* s_szNodeTypeNames[] = {
    "Invalid",

    // Unary
    "",
    "Negate",
    "Absolute",
    "Saturate",
    "Sqrt",
    "Exp",
    "Ln",
    "Log2",
    "Log10",
    "Pow2",
    "Sin",
    "Cos",
    "Tan",
    "ASin",
    "ACos",
    "ATan",
    "RadToDeg",
    "DegToRad",
    "Round",
    "Floor",
    "Ceil",
    "Trunc",
    "Frac",
    "Length",
    "Normalize",
    "BitwiseNot",
    "LogicalNot",
    "All",
    "Any",
    "TypeConversion",
    "",

    // Binary
    "",
    "Add",
    "Subtract",
    "Multiply",
    "Divide",
    "Modulo",
    "Log",
    "Pow",
    "Min",
    "Max",
    "Dot",
    "Cross",
    "Reflect",
    "BitshiftLeft",
    "BitshiftRight",
    "BitwiseAnd",
    "BitwiseXor",
    "BitwiseOr",
    "Equal",
    "NotEqual",
    "Less",
    "LessEqual",
    "Greater",
    "GreaterEqual",
    "LogicalAnd",
    "LogicalOr",
    "",

    // Ternary
    "",
    "Clamp",
    "Select",
    "Lerp",
    "",

    "Constant",
    "Swizzle",
    "Input",
    "Output",

    "FunctionCall",
    "ConstructorCall",
  };

  static_assert(WD_ARRAY_SIZE(s_szNodeTypeNames) == wdExpressionAST::NodeType::Count);

  static constexpr wdUInt16 BuildSignature(wdExpression::RegisterType::Enum returnType, wdExpression::RegisterType::Enum a, wdExpression::RegisterType::Enum b = wdExpression::RegisterType::Unknown, wdExpression::RegisterType::Enum c = wdExpression::RegisterType::Unknown)
  {
    wdUInt32 signature = static_cast<wdUInt32>(returnType);
    signature |= a << wdExpression::RegisterType::MaxNumBits * 1;
    signature |= b << wdExpression::RegisterType::MaxNumBits * 2;
    signature |= c << wdExpression::RegisterType::MaxNumBits * 3;
    return static_cast<wdUInt16>(signature);
  }

  static constexpr wdExpression::RegisterType::Enum GetReturnTypeFromSignature(wdUInt16 uiSignature)
  {
    wdUInt32 uiMask = WD_BIT(wdExpression::RegisterType::MaxNumBits) - 1;
    return static_cast<wdExpression::RegisterType::Enum>(uiSignature & uiMask);
  }

  static constexpr wdExpression::RegisterType::Enum GetArgumentTypeFromSignature(wdUInt16 uiSignature, wdUInt32 uiArgumentIndex)
  {
    wdUInt32 uiShift = wdExpression::RegisterType::MaxNumBits * (uiArgumentIndex + 1);
    wdUInt32 uiMask = WD_BIT(wdExpression::RegisterType::MaxNumBits) - 1;
    return static_cast<wdExpression::RegisterType::Enum>((uiSignature >> uiShift) & uiMask);
  }

#define SIG1(r, a) BuildSignature(wdExpression::RegisterType::r, wdExpression::RegisterType::a)
#define SIG2(r, a, b) BuildSignature(wdExpression::RegisterType::r, wdExpression::RegisterType::a, wdExpression::RegisterType::b)
#define SIG3(r, a, b, c) BuildSignature(wdExpression::RegisterType::r, wdExpression::RegisterType::a, wdExpression::RegisterType::b, wdExpression::RegisterType::c)

  struct Overloads
  {
    wdUInt16 m_Signatures[4] = {};
  };

  static Overloads s_NodeTypeOverloads[] = {
    {}, // Invalid,

    // Unary
    {},                                   // FirstUnary,
    {SIG1(Float, Float), SIG1(Int, Int)}, // Negate,
    {SIG1(Float, Float), SIG1(Int, Int)}, // Absolute,
    {SIG1(Float, Float), SIG1(Int, Int)}, // Saturate,
    {SIG1(Float, Float)},                 // Sqrt,
    {SIG1(Float, Float)},                 // Exp,
    {SIG1(Float, Float)},                 // Ln,
    {SIG1(Float, Float), SIG1(Int, Int)}, // Log2,
    {SIG1(Float, Float)},                 // Log10,
    {SIG1(Float, Float), SIG1(Int, Int)}, // Pow2,
    {SIG1(Float, Float)},                 // Sin,
    {SIG1(Float, Float)},                 // Cos,
    {SIG1(Float, Float)},                 // Tan,
    {SIG1(Float, Float)},                 // ASin,
    {SIG1(Float, Float)},                 // ACos,
    {SIG1(Float, Float)},                 // ATan,
    {SIG1(Float, Float)},                 // RadToDeg,
    {SIG1(Float, Float)},                 // DegToRad,
    {SIG1(Float, Float)},                 // Round,
    {SIG1(Float, Float)},                 // Floor,
    {SIG1(Float, Float)},                 // Ceil,
    {SIG1(Float, Float)},                 // Trunc,
    {SIG1(Float, Float)},                 // Frac,
    {SIG1(Float, Float)},                 // Length,
    {SIG1(Float, Float)},                 // Normalize,
    {SIG1(Int, Int)},                     // BitwiseNot,
    {SIG1(Bool, Bool)},                   // LogicalNot,
    {SIG1(Bool, Bool)},                   // All,
    {SIG1(Bool, Bool)},                   // Any,
    {},                                   // TypeConversion,
    {},                                   // LastUnary,

    // Binary
    {},                                                                       // FirstBinary,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Add,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Subtract,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Multiply,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Divide,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Modulo,
    {SIG2(Float, Float, Float)},                                              // Log,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Pow,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Min,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Max,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Dot,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Cross,
    {SIG2(Float, Float, Float), SIG2(Int, Int, Int)},                         // Reflect,
    {SIG2(Int, Int, Int)},                                                    // BitshiftLeft,
    {SIG2(Int, Int, Int)},                                                    // BitshiftRight,
    {SIG2(Int, Int, Int)},                                                    // BitwiseAnd,
    {SIG2(Int, Int, Int)},                                                    // BitwiseXor,
    {SIG2(Int, Int, Int)},                                                    // BitwiseOr,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int), SIG2(Bool, Bool, Bool)}, // Equal,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int), SIG2(Bool, Bool, Bool)}, // NotEqual,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int)},                         // Less,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int)},                         // LessEqual,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int)},                         // Greater,
    {SIG2(Bool, Float, Float), SIG2(Bool, Int, Int)},                         // GreaterEqual,
    {SIG2(Bool, Bool, Bool)},                                                 // LogicalAnd,
    {SIG2(Bool, Bool, Bool)},                                                 // LogicalOr,
    {},                                                                       // LastBinary,

    // Ternary
    {},                                                                                         // FirstTernary,
    {SIG3(Float, Float, Float, Float), SIG3(Int, Int, Int, Int)},                               // Clamp,
    {SIG3(Float, Bool, Float, Float), SIG3(Int, Bool, Int, Int), SIG3(Bool, Bool, Bool, Bool)}, // Select,
    {SIG3(Float, Float, Float, Float)},                                                         // Lerp,
    {},                                                                                         // LastTernary,

    {}, // Constant,
    {}, // Swizzle,
    {}, // Input,
    {}, // Output,

    {}, // FunctionCall,
    {}, // ConstructorCall,
  };

  static_assert(WD_ARRAY_SIZE(s_NodeTypeOverloads) == wdExpressionAST::NodeType::Count);
} // namespace

#undef SIG1
#undef SIG2
#undef SIG3

// static
const char* wdExpressionAST::NodeType::GetName(Enum nodeType)
{
  WD_ASSERT_DEBUG(nodeType >= 0 && nodeType < WD_ARRAY_SIZE(s_szNodeTypeNames), "Out of bounds access");
  return s_szNodeTypeNames[nodeType];
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  static wdVariantType::Enum s_DataTypeVariantTypes[] = {
    wdVariantType::Invalid, // Unknown,
    wdVariantType::Invalid, // Unknown2,
    wdVariantType::Invalid, // Unknown3,
    wdVariantType::Invalid, // Unknown4,

    wdVariantType::Bool,    // Bool,
    wdVariantType::Invalid, // Bool2,
    wdVariantType::Invalid, // Bool3,
    wdVariantType::Invalid, // Bool4,

    wdVariantType::Int32,    // Int,
    wdVariantType::Vector2I, // Int2,
    wdVariantType::Vector3I, // Int3,
    wdVariantType::Vector4I, // Int4,

    wdVariantType::Float,   // Float,
    wdVariantType::Vector2, // Float2,
    wdVariantType::Vector3, // Float3,
    wdVariantType::Vector4, // Float4,
  };
  static_assert(WD_ARRAY_SIZE(s_DataTypeVariantTypes) == (size_t)wdExpressionAST::DataType::Count);

  static wdExpressionAST::DataType::Enum s_DataTypeFromStreamType[] = {
    wdExpressionAST::DataType::Float,  // Half,
    wdExpressionAST::DataType::Float2, // Half2,
    wdExpressionAST::DataType::Float3, // Half3,
    wdExpressionAST::DataType::Float4, // Half4,

    wdExpressionAST::DataType::Float,  // Float,
    wdExpressionAST::DataType::Float2, // Float2,
    wdExpressionAST::DataType::Float3, // Float3,
    wdExpressionAST::DataType::Float4, // Float4,

    wdExpressionAST::DataType::Int,  // Byte,
    wdExpressionAST::DataType::Int2, // Byte2,
    wdExpressionAST::DataType::Int3, // Byte3,
    wdExpressionAST::DataType::Int4, // Byte4,

    wdExpressionAST::DataType::Int,  // Short,
    wdExpressionAST::DataType::Int2, // Short2,
    wdExpressionAST::DataType::Int3, // Short3,
    wdExpressionAST::DataType::Int4, // Short4,

    wdExpressionAST::DataType::Int,  // Int,
    wdExpressionAST::DataType::Int2, // Int2,
    wdExpressionAST::DataType::Int3, // Int3,
    wdExpressionAST::DataType::Int4, // Int4,
  };
  static_assert(WD_ARRAY_SIZE(s_DataTypeFromStreamType) == (size_t)wdProcessingStream::DataType::Count);

  static_assert(wdExpressionAST::DataType::Float >> 2 == wdExpression::RegisterType::Float);
  static_assert(wdExpressionAST::DataType::Int >> 2 == wdExpression::RegisterType::Int);
  static_assert(wdExpressionAST::DataType::Bool >> 2 == wdExpression::RegisterType::Bool);
  static_assert(wdExpressionAST::DataType::Unknown >> 2 == wdExpression::RegisterType::Unknown);

  static const char* s_szDataTypeNames[] = {
    "Unknown",  // Unknown,
    "Unknown2", // Unknown2,
    "Unknown3", // Unknown3,
    "Unknown4", // Unknown4,

    "Bool",  // Bool,
    "Bool2", // Bool2,
    "Bool3", // Bool3,
    "Bool4", // Bool4,

    "Int",  // Int,
    "Int2", // Int2,
    "Int3", // Int3,
    "Int4", // Int4,

    "Float",  // Float,
    "Float2", // Float2,
    "Float3", // Float3,
    "Float4", // Float4,
  };

  static_assert(WD_ARRAY_SIZE(s_szDataTypeNames) == wdExpressionAST::DataType::Count);
} // namespace


// static
wdVariantType::Enum wdExpressionAST::DataType::GetVariantType(Enum dataType)
{
  WD_ASSERT_DEBUG(dataType >= 0 && dataType < WD_ARRAY_SIZE(s_DataTypeVariantTypes), "Out of bounds access");
  return s_DataTypeVariantTypes[dataType];
}

// static
wdExpressionAST::DataType::Enum wdExpressionAST::DataType::FromStreamType(wdProcessingStream::DataType dataType)
{
  WD_ASSERT_DEBUG(static_cast<wdUInt32>(dataType) >= 0 && static_cast<wdUInt32>(dataType) < WD_ARRAY_SIZE(s_DataTypeFromStreamType), "Out of bounds access");
  return s_DataTypeFromStreamType[static_cast<wdUInt32>(dataType)];
}

// static
const char* wdExpressionAST::DataType::GetName(Enum dataType)
{
  WD_ASSERT_DEBUG(dataType >= 0 && dataType < WD_ARRAY_SIZE(s_szDataTypeNames), "Out of bounds access");
  return s_szDataTypeNames[dataType];
}

//////////////////////////////////////////////////////////////////////////

namespace
{
  static const char* s_szVectorComponentNames[] = {
    "x",
    "y",
    "z",
    "w",
  };

  static const char* s_szVectorComponentAltNames[] = {
    "r",
    "g",
    "b",
    "a",
  };

  static_assert(WD_ARRAY_SIZE(s_szVectorComponentNames) == wdExpressionAST::VectorComponent::Count);
  static_assert(WD_ARRAY_SIZE(s_szVectorComponentAltNames) == wdExpressionAST::VectorComponent::Count);
} // namespace

// static
const char* wdExpressionAST::VectorComponent::GetName(Enum vectorComponent)
{
  WD_ASSERT_DEBUG(vectorComponent >= 0 && vectorComponent < WD_ARRAY_SIZE(s_szVectorComponentNames), "Out of bounds access");
  return s_szVectorComponentNames[vectorComponent];
}

wdExpressionAST::VectorComponent::Enum wdExpressionAST::VectorComponent::FromChar(wdUInt32 uiChar)
{
  for (wdUInt32 i = 0; i < Count; ++i)
  {
    if (uiChar == s_szVectorComponentNames[i][0] || uiChar == s_szVectorComponentAltNames[i][0])
    {
      return static_cast<Enum>(i);
    }
  }

  return Count;
}

//////////////////////////////////////////////////////////////////////////

wdExpressionAST::wdExpressionAST()
  : m_Allocator("Expression AST", wdFoundation::GetAlignedAllocator())
{
  static_assert(sizeof(Node) == 8);
#if WD_ENABLED(WD_PLATFORM_64BIT)
  static_assert(sizeof(UnaryOperator) == 16);
  static_assert(sizeof(BinaryOperator) == 24);
  static_assert(sizeof(TernaryOperator) == 32);
  static_assert(sizeof(Constant) == 32);
  static_assert(sizeof(Swizzle) == 24);
  static_assert(sizeof(Input) == 24);
  static_assert(sizeof(Output) == 32);
  static_assert(sizeof(FunctionCall) == 96);
  static_assert(sizeof(ConstructorCall) == 48);
#endif
}

wdExpressionAST::~wdExpressionAST() = default;

wdExpressionAST::UnaryOperator* wdExpressionAST::CreateUnaryOperator(NodeType::Enum type, Node* pOperand, DataType::Enum returnType /*= DataType::Unknown*/)
{
  WD_ASSERT_DEBUG(NodeType::IsUnary(type), "Type '{}' is not an unary operator", NodeType::GetName(type));

  auto pUnaryOperator = WD_NEW(&m_Allocator, UnaryOperator);
  pUnaryOperator->m_Type = type;
  pUnaryOperator->m_ReturnType = returnType;
  pUnaryOperator->m_pOperand = pOperand;

  ResolveOverloads(pUnaryOperator);

  return pUnaryOperator;
}

wdExpressionAST::BinaryOperator* wdExpressionAST::CreateBinaryOperator(NodeType::Enum type, Node* pLeftOperand, Node* pRightOperand)
{
  WD_ASSERT_DEBUG(NodeType::IsBinary(type), "Type '{}' is not a binary operator", NodeType::GetName(type));

  auto pBinaryOperator = WD_NEW(&m_Allocator, BinaryOperator);
  pBinaryOperator->m_Type = type;
  pBinaryOperator->m_ReturnType = DataType::Unknown;
  pBinaryOperator->m_pLeftOperand = pLeftOperand;
  pBinaryOperator->m_pRightOperand = pRightOperand;

  ResolveOverloads(pBinaryOperator);

  return pBinaryOperator;
}

wdExpressionAST::TernaryOperator* wdExpressionAST::CreateTernaryOperator(NodeType::Enum type, Node* pFirstOperand, Node* pSecondOperand, Node* pThirdOperand)
{
  WD_ASSERT_DEBUG(NodeType::IsTernary(type), "Type '{}' is not a ternary operator", NodeType::GetName(type));

  auto pTernaryOperator = WD_NEW(&m_Allocator, TernaryOperator);
  pTernaryOperator->m_Type = type;
  pTernaryOperator->m_ReturnType = DataType::Unknown;
  pTernaryOperator->m_pFirstOperand = pFirstOperand;
  pTernaryOperator->m_pSecondOperand = pSecondOperand;
  pTernaryOperator->m_pThirdOperand = pThirdOperand;

  ResolveOverloads(pTernaryOperator);

  return pTernaryOperator;
}

wdExpressionAST::Constant* wdExpressionAST::CreateConstant(const wdVariant& value, DataType::Enum dataType /*= DataType::Float*/)
{
  wdVariantType::Enum variantType = DataType::GetVariantType(dataType);
  WD_ASSERT_DEV(variantType != wdVariantType::Invalid, "Invalid constant type '{}'", DataType::GetName(dataType));

  auto pConstant = WD_NEW(&m_Allocator, Constant);
  pConstant->m_Type = NodeType::Constant;
  pConstant->m_ReturnType = dataType;
  pConstant->m_Value = value.ConvertTo(DataType::GetVariantType(dataType));

  WD_ASSERT_DEV(pConstant->m_Value.IsValid(), "Invalid constant value or conversion to target data type failed");

  return pConstant;
}

wdExpressionAST::Swizzle* wdExpressionAST::CreateSwizzle(wdStringView sSwizzle, Node* pExpression)
{
  wdEnum<VectorComponent> components[4];
  wdUInt32 numComponents = 0;

  for (auto it : sSwizzle)
  {
    if (numComponents == WD_ARRAY_SIZE(components))
      return nullptr;

    wdEnum<VectorComponent> component = VectorComponent::FromChar(it);
    if (component == VectorComponent::Count)
      return nullptr;

    components[numComponents] = component;
    ++numComponents;
  }

  return CreateSwizzle(wdMakeArrayPtr(components, numComponents), pExpression);
}

wdExpressionAST::Swizzle* wdExpressionAST::CreateSwizzle(wdEnum<VectorComponent> component, Node* pExpression)
{
  return CreateSwizzle(wdMakeArrayPtr(&component, 1), pExpression);
}

wdExpressionAST::Swizzle* wdExpressionAST::CreateSwizzle(wdArrayPtr<wdEnum<VectorComponent>> swizzle, Node* pExpression)
{
  WD_ASSERT_DEV(swizzle.GetCount() >= 1 && swizzle.GetCount() <= 4, "Invalid number of vector components for swizzle.");
  WD_ASSERT_DEV(pExpression->m_ReturnType != DataType::Unknown, "Expression return type must be known.");

  auto pSwizzle = WD_NEW(&m_Allocator, Swizzle);
  pSwizzle->m_Type = NodeType::Swizzle;
  pSwizzle->m_ReturnType = DataType::FromRegisterType(DataType::GetRegisterType(pExpression->m_ReturnType), swizzle.GetCount());

  wdMemoryUtils::Copy(pSwizzle->m_Components, swizzle.GetPtr(), swizzle.GetCount());
  pSwizzle->m_NumComponents = swizzle.GetCount();
  pSwizzle->m_pExpression = pExpression;

  return pSwizzle;
}

wdExpressionAST::Input* wdExpressionAST::CreateInput(const wdExpression::StreamDesc& desc)
{
  auto pInput = WD_NEW(&m_Allocator, Input);
  pInput->m_Type = NodeType::Input;
  pInput->m_ReturnType = DataType::FromStreamType(desc.m_DataType);
  pInput->m_Desc = desc;

  return pInput;
}

wdExpressionAST::Output* wdExpressionAST::CreateOutput(const wdExpression::StreamDesc& desc, Node* pExpression)
{
  auto pOutput = WD_NEW(&m_Allocator, Output);
  pOutput->m_Type = NodeType::Output;
  pOutput->m_ReturnType = DataType::FromStreamType(desc.m_DataType);
  pOutput->m_uiNumInputElements = static_cast<wdUInt8>(DataType::GetElementCount(pOutput->m_ReturnType));
  pOutput->m_Desc = desc;
  pOutput->m_pExpression = pExpression;

  return pOutput;
}

wdExpressionAST::FunctionCall* wdExpressionAST::CreateFunctionCall(const wdExpression::FunctionDesc& desc, wdArrayPtr<Node*> arguments)
{
  return CreateFunctionCall(wdMakeArrayPtr(&desc, 1), arguments);
}

wdExpressionAST::FunctionCall* wdExpressionAST::CreateFunctionCall(wdArrayPtr<const wdExpression::FunctionDesc> descs, wdArrayPtr<Node*> arguments)
{
  auto pFunctionCall = WD_NEW(&m_Allocator, FunctionCall);
  pFunctionCall->m_Type = NodeType::FunctionCall;
  pFunctionCall->m_ReturnType = DataType::Unknown;

  for (auto& desc : descs)
  {
    auto it = m_FunctionDescs.Insert(desc);

    pFunctionCall->m_Descs.PushBack(&it.Key());
  }

  pFunctionCall->m_Arguments = arguments;

  ResolveOverloads(pFunctionCall);

  return pFunctionCall;
}

wdExpressionAST::ConstructorCall* wdExpressionAST::CreateConstructorCall(DataType::Enum dataType, wdArrayPtr<Node*> arguments)
{
  WD_ASSERT_DEV(dataType >= DataType::Bool, "Invalid data type for constructor");

  auto pConstructorCall = WD_NEW(&m_Allocator, ConstructorCall);
  pConstructorCall->m_Type = NodeType::ConstructorCall;
  pConstructorCall->m_ReturnType = dataType;
  pConstructorCall->m_Arguments = arguments;

  ResolveOverloads(pConstructorCall);

  return pConstructorCall;
}

wdExpressionAST::ConstructorCall* wdExpressionAST::CreateConstructorCall(Node* pOldValue, Node* pNewValue, wdStringView sPartialAssignmentMask)
{
  wdExpression::RegisterType::Enum registerType = wdExpression::RegisterType::Unknown;
  wdSmallArray<Node*, 4> arguments;

  if (pOldValue != nullptr)
  {
    registerType = DataType::GetRegisterType(pOldValue->m_ReturnType);

    if (NodeType::IsConstructorCall(pOldValue->m_Type))
    {
      auto pConstructorCall = static_cast<ConstructorCall*>(pOldValue);
      arguments = pConstructorCall->m_Arguments;
    }
    else
    {
      const wdUInt32 uiNumElements = DataType::GetElementCount(pOldValue->m_ReturnType);
      if (uiNumElements == 1)
      {
        arguments.PushBack(pOldValue);
      }
      else
      {
        for (wdUInt32 i = 0; i < uiNumElements; ++i)
        {
          auto pSwizzle = CreateSwizzle(static_cast<VectorComponent::Enum>(i), pOldValue);
          arguments.PushBack(pSwizzle);
        }
      }
    }
  }

  const wdUInt32 uiNewValueElementCount = DataType::GetElementCount(pNewValue->m_ReturnType);
  wdUInt32 uiNewValueElementIndex = 0;
  for (auto it : sPartialAssignmentMask)
  {
    auto component = wdExpressionAST::VectorComponent::FromChar(it);
    if (component == wdExpressionAST::VectorComponent::Count)
    {
      return nullptr;
    }

    Node* pNewValueElement = nullptr;
    if (uiNewValueElementCount == 1)
    {
      pNewValueElement = pNewValue;
    }
    else
    {
      if (uiNewValueElementIndex >= uiNewValueElementCount)
      {
        return nullptr;
      }

      pNewValueElement = CreateSwizzle(static_cast<VectorComponent::Enum>(uiNewValueElementIndex), pNewValue);
      ++uiNewValueElementIndex;
    }

    wdUInt32 componentIndex = component;
    if (componentIndex >= arguments.GetCount())
    {
      while (componentIndex > arguments.GetCount())
      {
        arguments.PushBack(CreateConstant(0));
      }

      arguments.PushBack(pNewValueElement);
    }
    else
    {
      arguments[componentIndex] = pNewValueElement;
    }

    if (pOldValue == nullptr)
    {
      registerType = wdMath::Max(registerType, DataType::GetRegisterType(pNewValueElement->m_ReturnType));
    }
  }

  wdEnum<DataType> newType = DataType::FromRegisterType(registerType, arguments.GetCount());
  return CreateConstructorCall(newType, arguments);
}

// static
wdArrayPtr<wdExpressionAST::Node*> wdExpressionAST::GetChildren(Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto& pChild = static_cast<UnaryOperator*>(pNode)->m_pOperand;
    return wdMakeArrayPtr(&pChild, 1);
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto& pChildren = static_cast<BinaryOperator*>(pNode)->m_pLeftOperand;
    return wdMakeArrayPtr(&pChildren, 2);
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto& pChildren = static_cast<TernaryOperator*>(pNode)->m_pFirstOperand;
    return wdMakeArrayPtr(&pChildren, 3);
  }
  else if (NodeType::IsSwizzle(nodeType))
  {
    auto& pChild = static_cast<Swizzle*>(pNode)->m_pExpression;
    return wdMakeArrayPtr(&pChild, 1);
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto& pChild = static_cast<Output*>(pNode)->m_pExpression;
    return wdMakeArrayPtr(&pChild, 1);
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto& args = static_cast<FunctionCall*>(pNode)->m_Arguments;
    return args;
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    auto& args = static_cast<ConstructorCall*>(pNode)->m_Arguments;
    return args;
  }

  WD_ASSERT_DEV(NodeType::IsInput(nodeType) || NodeType::IsConstant(nodeType), "Unknown node type");
  return wdArrayPtr<Node*>();
}

// static
wdArrayPtr<const wdExpressionAST::Node*> wdExpressionAST::GetChildren(const Node* pNode)
{
  NodeType::Enum nodeType = pNode->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto& pChild = static_cast<const UnaryOperator*>(pNode)->m_pOperand;
    return wdMakeArrayPtr((const Node**)&pChild, 1);
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto& pChildren = static_cast<const BinaryOperator*>(pNode)->m_pLeftOperand;
    return wdMakeArrayPtr((const Node**)&pChildren, 2);
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto& pChildren = static_cast<const TernaryOperator*>(pNode)->m_pFirstOperand;
    return wdMakeArrayPtr((const Node**)&pChildren, 3);
  }
  else if (NodeType::IsSwizzle(nodeType))
  {
    auto& pChild = static_cast<const Swizzle*>(pNode)->m_pExpression;
    return wdMakeArrayPtr((const Node**)&pChild, 1);
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto& pChild = static_cast<const Output*>(pNode)->m_pExpression;
    return wdMakeArrayPtr((const Node**)&pChild, 1);
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto& args = static_cast<const FunctionCall*>(pNode)->m_Arguments;
    return wdArrayPtr<const Node*>((const Node**)args.GetData(), args.GetCount());
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    auto& args = static_cast<const ConstructorCall*>(pNode)->m_Arguments;
    return wdArrayPtr<const Node*>((const Node**)args.GetData(), args.GetCount());
  }

  WD_ASSERT_DEV(NodeType::IsInput(nodeType) || NodeType::IsConstant(nodeType), "Unknown node type");
  return wdArrayPtr<const Node*>();
}

namespace
{
  struct NodeInfo
  {
    WD_DECLARE_POD_TYPE();

    const wdExpressionAST::Node* m_pNode;
    wdUInt32 m_uiParentGraphNode;
  };
} // namespace

void wdExpressionAST::PrintGraph(wdDGMLGraph& inout_graph) const
{
  wdHybridArray<NodeInfo, 64> nodeStack;

  wdStringBuilder sTmp;
  for (auto pOutputNode : m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      continue;

    sTmp = NodeType::GetName(pOutputNode->m_Type);
    sTmp.Append("(", DataType::GetName(pOutputNode->m_ReturnType), ")");
    sTmp.Append(": ", pOutputNode->m_Desc.m_sName);

    wdDGMLGraph::NodeDesc nd;
    nd.m_Color = wdColorScheme::LightUI(wdColorScheme::Blue);
    wdUInt32 uiGraphNode = inout_graph.AddNode(sTmp, &nd);

    nodeStack.PushBack({pOutputNode->m_pExpression, uiGraphNode});
  }

  wdHashTable<const Node*, wdUInt32> nodeCache;

  while (!nodeStack.IsEmpty())
  {
    NodeInfo currentNodeInfo = nodeStack.PeekBack();
    nodeStack.PopBack();

    wdUInt32 uiGraphNode = 0;
    if (currentNodeInfo.m_pNode != nullptr)
    {
      if (!nodeCache.TryGetValue(currentNodeInfo.m_pNode, uiGraphNode))
      {
        NodeType::Enum nodeType = currentNodeInfo.m_pNode->m_Type;
        sTmp = NodeType::GetName(nodeType);
        sTmp.Append("(", DataType::GetName(currentNodeInfo.m_pNode->m_ReturnType), ")");
        wdColor color = wdColor::White;

        if (NodeType::IsConstant(nodeType))
        {
          sTmp.AppendFormat(": {0}", static_cast<const Constant*>(currentNodeInfo.m_pNode)->m_Value.ConvertTo<wdString>());
        }
        else if (NodeType::IsSwizzle(nodeType))
        {
          auto pSwizzleNode = static_cast<const Swizzle*>(currentNodeInfo.m_pNode);
          sTmp.Append(": ");
          for (wdUInt32 i = 0; i < pSwizzleNode->m_NumComponents; ++i)
          {
            sTmp.Append(VectorComponent::GetName(pSwizzleNode->m_Components[i]));
          }
        }
        else if (NodeType::IsInput(nodeType))
        {
          auto pInputNode = static_cast<const Input*>(currentNodeInfo.m_pNode);
          sTmp.Append(": ", pInputNode->m_Desc.m_sName);
          color = wdColorScheme::LightUI(wdColorScheme::Green);
        }
        else if (NodeType::IsFunctionCall(nodeType))
        {
          auto pFunctionCall = static_cast<const FunctionCall*>(currentNodeInfo.m_pNode);
          if (pFunctionCall->m_uiOverloadIndex != 0xFF)
          {
            auto pDesc = pFunctionCall->m_Descs[currentNodeInfo.m_pNode->m_uiOverloadIndex];
            sTmp.Append(": ", pDesc->GetMangledName());
          }
          else
          {
            sTmp.Append(": ", pFunctionCall->m_Descs[0]->m_sName);
          }
          color = wdColorScheme::LightUI(wdColorScheme::Yellow);
        }

        wdDGMLGraph::NodeDesc nd;
        nd.m_Color = color;
        uiGraphNode = inout_graph.AddNode(sTmp, &nd);
        nodeCache.Insert(currentNodeInfo.m_pNode, uiGraphNode);

        // push children
        auto children = GetChildren(currentNodeInfo.m_pNode);
        for (auto pChild : children)
        {
          nodeStack.PushBack({pChild, uiGraphNode});
        }
      }
    }
    else
    {
      wdDGMLGraph::NodeDesc nd;
      nd.m_Color = wdColor::OrangeRed;
      uiGraphNode = inout_graph.AddNode("Invalid", &nd);
    }

    inout_graph.AddConnection(uiGraphNode, currentNodeInfo.m_uiParentGraphNode);
  }
}

void wdExpressionAST::ResolveOverloads(Node* pNode)
{
  if (pNode->m_uiOverloadIndex != 0xFF)
  {
    // already resolved
    return;
  }

  const NodeType::Enum nodeType = pNode->m_Type;
  if (nodeType == NodeType::TypeConversion)
  {
    WD_ASSERT_DEV(pNode->m_ReturnType != DataType::Unknown, "Return type must be specified for conversion nodes");
    pNode->m_uiOverloadIndex = 0;
    return;
  }

  auto CalculateMatchDistance = [](wdArrayPtr<Node*> children, wdArrayPtr<const wdEnum<wdExpression::RegisterType>> expectedTypes, wdUInt32 uiNumRequiredArgs, wdUInt32& ref_uiMaxNumElements) {
    if (children.GetCount() < uiNumRequiredArgs)
    {
      return wdInvalidIndex;
    }

    wdUInt32 uiMatchDistance = 0;
    ref_uiMaxNumElements = 1;
    for (wdUInt32 i = 0; i < wdMath::Min(children.GetCount(), expectedTypes.GetCount()); ++i)
    {
      auto& pChildNode = children[i];
      WD_ASSERT_DEV(pChildNode != nullptr && pChildNode->m_ReturnType != DataType::Unknown, "Invalid child node");

      auto childType = DataType::GetRegisterType(pChildNode->m_ReturnType);
      int iDistance = expectedTypes[i] - childType;
      if (iDistance < 0)
      {
        // Penalty to prevent 'narrowing' conversions
        iDistance *= -wdExpression::RegisterType::Count;
      }
      uiMatchDistance += iDistance;
      ref_uiMaxNumElements = wdMath::Max(ref_uiMaxNumElements, DataType::GetElementCount(pChildNode->m_ReturnType));
    }
    return uiMatchDistance;
  };

  if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsTernary(nodeType))
  {
    auto children = GetChildren(pNode);
    wdSmallArray<wdEnum<wdExpression::RegisterType>, 4> expectedTypes;
    wdUInt32 uiBestMatchDistance = wdInvalidIndex;

    for (wdUInt32 uiSigIndex = 0; uiSigIndex < WD_ARRAY_SIZE(Overloads::m_Signatures); ++uiSigIndex)
    {
      const wdUInt16 uiSignature = s_NodeTypeOverloads[nodeType].m_Signatures[uiSigIndex];
      if (uiSignature == 0)
        break;

      expectedTypes.Clear();
      for (wdUInt32 i = 0; i < children.GetCount(); ++i)
      {
        expectedTypes.PushBack(GetArgumentTypeFromSignature(uiSignature, i));
      }

      wdUInt32 uiMaxNumElements = 1;
      wdUInt32 uiMatchDistance = CalculateMatchDistance(children, expectedTypes, expectedTypes.GetCount(), uiMaxNumElements);
      if (uiMatchDistance < uiBestMatchDistance)
      {
        const wdUInt32 uiReturnTypeElements = NodeType::AlwaysReturnsSingleElement(nodeType) ? 1 : uiMaxNumElements;
        pNode->m_ReturnType = DataType::FromRegisterType(GetReturnTypeFromSignature(uiSignature), uiReturnTypeElements);
        pNode->m_uiNumInputElements = static_cast<wdUInt8>(uiMaxNumElements);
        pNode->m_uiOverloadIndex = static_cast<wdUInt8>(uiSigIndex);
        uiBestMatchDistance = uiMatchDistance;
      }
    }
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto pFunctionCall = static_cast<FunctionCall*>(pNode);
    wdUInt32 uiBestMatchDistance = wdInvalidIndex;

    for (wdUInt32 uiOverloadIndex = 0; uiOverloadIndex < pFunctionCall->m_Descs.GetCount(); ++uiOverloadIndex)
    {
      auto pFuncDesc = pFunctionCall->m_Descs[uiOverloadIndex];

      wdUInt32 uiMaxNumElements = 1;
      wdUInt32 uiMatchDistance = CalculateMatchDistance(pFunctionCall->m_Arguments, pFuncDesc->m_InputTypes, pFuncDesc->m_uiNumRequiredInputs, uiMaxNumElements);
      if (uiMatchDistance < uiBestMatchDistance)
      {
        pNode->m_ReturnType = DataType::FromRegisterType(pFuncDesc->m_OutputType, uiMaxNumElements);
        pNode->m_uiNumInputElements = static_cast<wdUInt8>(uiMaxNumElements);
        pNode->m_uiOverloadIndex = static_cast<wdUInt8>(uiOverloadIndex);
        uiBestMatchDistance = uiMatchDistance;
      }
    }

    if (pNode->m_ReturnType != DataType::Unknown)
    {
      auto pFuncDesc = pFunctionCall->m_Descs[pNode->m_uiOverloadIndex];

      // Trim arguments array to number of inputs
      if (pFunctionCall->m_Arguments.GetCount() > pFuncDesc->m_InputTypes.GetCount())
      {
        pFunctionCall->m_Arguments.SetCount(static_cast<wdUInt16>(pFuncDesc->m_InputTypes.GetCount()));
      }
    }
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    auto pConstructorCall = static_cast<ConstructorCall*>(pNode);
    auto& args = pConstructorCall->m_Arguments;
    const wdUInt32 uiElementCount = wdExpressionAST::DataType::GetElementCount(pNode->m_ReturnType);

    if (uiElementCount > 1 && args.GetCount() == 1 && wdExpressionAST::DataType::GetElementCount(args[0]->m_ReturnType) == 1)
    {
      for (wdUInt32 i = 0; i < uiElementCount - 1; ++i)
      {
        pConstructorCall->m_Arguments.PushBack(args[0]);
      }

      return;
    }

    wdSmallArray<Node*, 4> newArguments;
    Node* pZero = nullptr;

    wdUInt32 uiArgumentIndex = 0;
    wdUInt32 uiArgumentElementIndex = 0;

    for (wdUInt32 i = 0; i < uiElementCount; ++i)
    {
      if (uiArgumentIndex < args.GetCount())
      {
        auto pArg = args[uiArgumentIndex];
        WD_ASSERT_DEV(pArg != nullptr && pArg->m_ReturnType != DataType::Unknown, "Invalid argument node");

        const wdUInt32 uiArgElementCount = wdExpressionAST::DataType::GetElementCount(pArg->m_ReturnType);
        if (uiArgElementCount == 1)
        {
          newArguments.PushBack(pArg);
        }
        else if (uiArgumentElementIndex < uiArgElementCount)
        {
          newArguments.PushBack(CreateSwizzle(static_cast<VectorComponent::Enum>(uiArgumentElementIndex), pArg));
        }

        ++uiArgumentElementIndex;
        if (uiArgumentElementIndex >= uiArgElementCount)
        {
          ++uiArgumentIndex;
          uiArgumentElementIndex = 0;
        }
      }
      else
      {
        if (pZero == nullptr)
        {
          pZero = CreateConstant(0);
        }
        newArguments.PushBack(pZero);
      }
    }

    WD_ASSERT_DEBUG(newArguments.GetCount() == uiElementCount, "Not enough arguments");
    pConstructorCall->m_Arguments = newArguments;
  }
}

// static
wdExpressionAST::DataType::Enum wdExpressionAST::GetExpectedChildDataType(const Node* pNode, wdUInt32 uiChildIndex)
{
  const NodeType::Enum nodeType = pNode->m_Type;
  const DataType::Enum returnType = pNode->m_ReturnType;
  const wdUInt32 uiOverloadIndex = pNode->m_uiOverloadIndex;
  WD_ASSERT_DEV(returnType != DataType::Unknown, "Return type must not be unknown");

  if (nodeType == NodeType::TypeConversion || NodeType::IsSwizzle(nodeType))
  {
    return DataType::Unknown;
  }
  else if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsTernary(nodeType))
  {
    WD_ASSERT_DEV(uiOverloadIndex != 0xFF, "Unresolved overload");
    wdUInt16 uiSignature = s_NodeTypeOverloads[nodeType].m_Signatures[uiOverloadIndex];
    return DataType::FromRegisterType(GetArgumentTypeFromSignature(uiSignature, uiChildIndex), pNode->m_uiNumInputElements);
  }
  else if (NodeType::IsOutput(nodeType))
  {
    return returnType;
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    WD_ASSERT_DEV(uiOverloadIndex != 0xFF, "Unresolved overload");

    auto pDesc = static_cast<const FunctionCall*>(pNode)->m_Descs[uiOverloadIndex];
    return DataType::FromRegisterType(pDesc->m_InputTypes[uiChildIndex], pNode->m_uiNumInputElements);
  }
  else if (NodeType::IsConstructorCall(nodeType))
  {
    return DataType::FromRegisterType(DataType::GetRegisterType(returnType));
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return DataType::Unknown;
}

// static
void wdExpressionAST::UpdateHash(Node* pNode)
{
  wdHybridArray<wdUInt32, 16> valuesToHash;

  const wdUInt32* pBaseValues = reinterpret_cast<const wdUInt32*>(pNode);
  valuesToHash.PushBack(pBaseValues[0]);
  valuesToHash.PushBack(pBaseValues[1]);

  NodeType::Enum nodeType = pNode->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto pUnary = static_cast<const UnaryOperator*>(pNode);
    valuesToHash.PushBack(pUnary->m_pOperand->m_uiHash);
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto pBinary = static_cast<const BinaryOperator*>(pNode);
    wdUInt32 uiHashLeft = pBinary->m_pLeftOperand->m_uiHash;
    wdUInt32 uiHashRight = pBinary->m_pRightOperand->m_uiHash;

    // Sort by hash value for commutative operations so operand order doesn't matter
    if (NodeType::IsCommutative(nodeType) && uiHashLeft > uiHashRight)
    {
      wdMath::Swap(uiHashLeft, uiHashRight);
    }

    valuesToHash.PushBack(uiHashLeft);
    valuesToHash.PushBack(uiHashRight);
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto pTernary = static_cast<const TernaryOperator*>(pNode);
    valuesToHash.PushBack(pTernary->m_pFirstOperand->m_uiHash);
    valuesToHash.PushBack(pTernary->m_pSecondOperand->m_uiHash);
    valuesToHash.PushBack(pTernary->m_pThirdOperand->m_uiHash);
  }
  else if (NodeType::IsConstant(nodeType))
  {
    auto pConstant = static_cast<const Constant*>(pNode);
    const wdUInt64 uiValueHash = pConstant->m_Value.ComputeHash();
    valuesToHash.PushBack(static_cast<wdUInt32>(uiValueHash));
    valuesToHash.PushBack(static_cast<wdUInt32>(uiValueHash >> 32u));
  }
  else if (NodeType::IsInput(nodeType))
  {
    auto pInput = static_cast<const Input*>(pNode);
    const wdUInt64 uiNameHash = pInput->m_Desc.m_sName.GetHash();
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash));
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash >> 32u));
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto pOutput = static_cast<const Output*>(pNode);
    const wdUInt64 uiNameHash = pOutput->m_Desc.m_sName.GetHash();
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash));
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash >> 32u));
    valuesToHash.PushBack(pOutput->m_pExpression->m_uiHash);
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto pFunctionCall = static_cast<const FunctionCall*>(pNode);
    const wdUInt64 uiNameHash = pFunctionCall->m_Descs[0]->m_sName.GetHash();
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash));
    valuesToHash.PushBack(static_cast<wdUInt32>(uiNameHash >> 32u));

    for (auto pArg : pFunctionCall->m_Arguments)
    {
      valuesToHash.PushBack(pArg->m_uiHash);
    }
  }
  else
  {
    WD_ASSERT_NOT_IMPLEMENTED;
  }

  pNode->m_uiHash = wdHashingUtils::xxHash32(valuesToHash.GetData(), valuesToHash.GetCount() * sizeof(wdUInt32));
}

// static
bool wdExpressionAST::IsEqual(const Node* pNodeA, const Node* pNodeB)
{
  const wdUInt32 uiBaseValuesA = *reinterpret_cast<const wdUInt32*>(pNodeA);
  const wdUInt32 uiBaseValuesB = *reinterpret_cast<const wdUInt32*>(pNodeB);
  if (uiBaseValuesA != uiBaseValuesB)
  {
    return false;
  }

  NodeType::Enum nodeType = pNodeA->m_Type;
  if (NodeType::IsUnary(nodeType))
  {
    auto pUnaryA = static_cast<const UnaryOperator*>(pNodeA);
    auto pUnaryB = static_cast<const UnaryOperator*>(pNodeB);

    return pUnaryA->m_pOperand == pUnaryB->m_pOperand;
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto pBinaryA = static_cast<const BinaryOperator*>(pNodeA);
    auto pBinaryB = static_cast<const BinaryOperator*>(pNodeB);

    auto pLeftA = pBinaryA->m_pLeftOperand;
    auto pLeftB = pBinaryB->m_pLeftOperand;
    auto pRightA = pBinaryA->m_pRightOperand;
    auto pRightB = pBinaryB->m_pRightOperand;

    if (NodeType::IsCommutative(nodeType))
    {
      if (pLeftA > pRightA)
        wdMath::Swap(pLeftA, pRightA);

      if (pLeftB > pRightB)
        wdMath::Swap(pLeftB, pRightB);
    }

    return pLeftA == pLeftB && pRightA == pRightB;
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto pTernaryA = static_cast<const TernaryOperator*>(pNodeA);
    auto pTernaryB = static_cast<const TernaryOperator*>(pNodeB);

    return pTernaryA->m_pFirstOperand == pTernaryB->m_pFirstOperand &&
           pTernaryA->m_pSecondOperand == pTernaryB->m_pSecondOperand &&
           pTernaryA->m_pThirdOperand == pTernaryB->m_pThirdOperand;
  }
  else if (NodeType::IsConstant(nodeType))
  {
    auto pConstantA = static_cast<const Constant*>(pNodeA);
    auto pConstantB = static_cast<const Constant*>(pNodeB);

    return pConstantA->m_Value == pConstantB->m_Value;
  }
  else if (NodeType::IsInput(nodeType))
  {
    auto pInputA = static_cast<const Input*>(pNodeA);
    auto pInputB = static_cast<const Input*>(pNodeB);

    return pInputA->m_Desc == pInputB->m_Desc;
  }
  else if (NodeType::IsOutput(nodeType))
  {
    auto pOutputA = static_cast<const Output*>(pNodeA);
    auto pOutputB = static_cast<const Output*>(pNodeB);

    return pOutputA->m_Desc == pOutputB->m_Desc && pOutputA->m_pExpression == pOutputB->m_pExpression;
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    auto pFunctionCallA = static_cast<const FunctionCall*>(pNodeA);
    auto pFunctionCallB = static_cast<const FunctionCall*>(pNodeB);

    return pFunctionCallA->m_Descs[pFunctionCallA->m_uiOverloadIndex] == pFunctionCallB->m_Descs[pFunctionCallB->m_uiOverloadIndex] &&
           pFunctionCallA->m_Arguments == pFunctionCallB->m_Arguments;
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return false;
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionAST);
