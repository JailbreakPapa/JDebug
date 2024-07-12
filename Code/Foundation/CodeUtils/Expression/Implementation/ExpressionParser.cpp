#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/Tokenizer.h>

namespace
{
  struct AssignOperator
  {
    nsStringView m_sName;
    nsExpressionAST::NodeType::Enum m_NodeType;
  };

  static constexpr AssignOperator s_assignOperators[] = {
    {"+="_nssv, nsExpressionAST::NodeType::Add},
    {"-="_nssv, nsExpressionAST::NodeType::Subtract},
    {"*="_nssv, nsExpressionAST::NodeType::Multiply},
    {"/="_nssv, nsExpressionAST::NodeType::Divide},
    {"%="_nssv, nsExpressionAST::NodeType::Modulo},
    {"<<="_nssv, nsExpressionAST::NodeType::BitshiftLeft},
    {">>="_nssv, nsExpressionAST::NodeType::BitshiftRight},
    {"&="_nssv, nsExpressionAST::NodeType::BitwiseAnd},
    {"^="_nssv, nsExpressionAST::NodeType::BitwiseXor},
    {"|="_nssv, nsExpressionAST::NodeType::BitwiseOr},
  };

  struct BinaryOperator
  {
    nsStringView m_sName;
    nsExpressionAST::NodeType::Enum m_NodeType;
    int m_iPrecedence;
  };

  // Operator precedence according to https://en.cppreference.com/w/cpp/language/operator_precedence,
  // lower value means higher precedence
  // sorted by string length to simplify the test against a token stream
  static constexpr BinaryOperator s_binaryOperators[] = {
    {"&&"_nssv, nsExpressionAST::NodeType::LogicalAnd, 14},
    {"||"_nssv, nsExpressionAST::NodeType::LogicalOr, 15},
    {"<<"_nssv, nsExpressionAST::NodeType::BitshiftLeft, 7},
    {">>"_nssv, nsExpressionAST::NodeType::BitshiftRight, 7},
    {"=="_nssv, nsExpressionAST::NodeType::Equal, 10},
    {"!="_nssv, nsExpressionAST::NodeType::NotEqual, 10},
    {"<="_nssv, nsExpressionAST::NodeType::LessEqual, 9},
    {">="_nssv, nsExpressionAST::NodeType::GreaterEqual, 9},
    {"<"_nssv, nsExpressionAST::NodeType::Less, 9},
    {">"_nssv, nsExpressionAST::NodeType::Greater, 9},
    {"&"_nssv, nsExpressionAST::NodeType::BitwiseAnd, 11},
    {"^"_nssv, nsExpressionAST::NodeType::BitwiseXor, 12},
    {"|"_nssv, nsExpressionAST::NodeType::BitwiseOr, 13},
    {"?"_nssv, nsExpressionAST::NodeType::Select, 16},
    {"+"_nssv, nsExpressionAST::NodeType::Add, 6},
    {"-"_nssv, nsExpressionAST::NodeType::Subtract, 6},
    {"*"_nssv, nsExpressionAST::NodeType::Multiply, 5},
    {"/"_nssv, nsExpressionAST::NodeType::Divide, 5},
    {"%"_nssv, nsExpressionAST::NodeType::Modulo, 5},
  };

  static nsHashTable<nsHashedString, nsEnum<nsExpressionAST::DataType>> s_KnownTypes;
  static nsHashTable<nsHashedString, nsEnum<nsExpressionAST::NodeType>> s_BuiltinFunctions;

} // namespace

using namespace nsTokenParseUtils;

nsExpressionParser::nsExpressionParser()
{
  RegisterKnownTypes();
  RegisterBuiltinFunctions();
}

nsExpressionParser::~nsExpressionParser() = default;

// static
const nsHashTable<nsHashedString, nsEnum<nsExpressionAST::DataType>>& nsExpressionParser::GetKnownTypes()
{
  RegisterKnownTypes();

  return s_KnownTypes;
}

// static
const nsHashTable<nsHashedString, nsEnum<nsExpressionAST::NodeType>>& nsExpressionParser::GetBuiltinFunctions()
{
  RegisterBuiltinFunctions();

  return s_BuiltinFunctions;
}

void nsExpressionParser::RegisterFunction(const nsExpression::FunctionDesc& funcDesc)
{
  NS_ASSERT_DEV(funcDesc.m_uiNumRequiredInputs <= funcDesc.m_InputTypes.GetCount(), "Not enough input types defined. {} inputs are required but only {} types given.", funcDesc.m_uiNumRequiredInputs, funcDesc.m_InputTypes.GetCount());

  auto& functionDescs = m_FunctionDescs[funcDesc.m_sName];
  if (functionDescs.Contains(funcDesc) == false)
  {
    functionDescs.PushBack(funcDesc);
  }
}

void nsExpressionParser::UnregisterFunction(const nsExpression::FunctionDesc& funcDesc)
{
  if (auto pFunctionDescs = m_FunctionDescs.GetValue(funcDesc.m_sName))
  {
    pFunctionDescs->RemoveAndCopy(funcDesc);
  }
}

nsResult nsExpressionParser::Parse(nsStringView sCode, nsArrayPtr<nsExpression::StreamDesc> inputs, nsArrayPtr<nsExpression::StreamDesc> outputs, const Options& options, nsExpressionAST& out_ast)
{
  if (sCode.IsEmpty())
    return NS_FAILURE;

  m_Options = options;

  m_pAST = &out_ast;
  SetupInAndOutputs(inputs, outputs);

  nsTokenizer tokenizer;
  tokenizer.Tokenize(nsArrayPtr<const nsUInt8>((const nsUInt8*)sCode.GetStartPointer(), sCode.GetElementCount()), nsLog::GetThreadLocalLogSystem());

  nsUInt32 readTokens = 0;
  while (tokenizer.GetNextLine(readTokens, m_TokenStream).Succeeded())
  {
    m_uiCurrentToken = 0;

    while (m_uiCurrentToken < m_TokenStream.GetCount())
    {
      NS_SUCCEED_OR_RETURN(ParseStatement());

      if (m_uiCurrentToken < m_TokenStream.GetCount() && AcceptStatementTerminator() == false)
      {
        auto pCurrentToken = m_TokenStream[m_uiCurrentToken];
        ReportError(pCurrentToken, nsFmt("Syntax error, unexpected token '{}'", pCurrentToken->m_DataView));
        return NS_FAILURE;
      }
    }
  }

  NS_SUCCEED_OR_RETURN(CheckOutputs());

  return NS_SUCCESS;
}

// static
void nsExpressionParser::RegisterKnownTypes()
{
  if (s_KnownTypes.IsEmpty() == false)
    return;

  s_KnownTypes.Insert(nsMakeHashedString("var"), nsExpressionAST::DataType::Unknown);

  s_KnownTypes.Insert(nsMakeHashedString("vec2"), nsExpressionAST::DataType::Float2);
  s_KnownTypes.Insert(nsMakeHashedString("vec3"), nsExpressionAST::DataType::Float3);
  s_KnownTypes.Insert(nsMakeHashedString("vec4"), nsExpressionAST::DataType::Float4);

  s_KnownTypes.Insert(nsMakeHashedString("vec2i"), nsExpressionAST::DataType::Int2);
  s_KnownTypes.Insert(nsMakeHashedString("vec3i"), nsExpressionAST::DataType::Int3);
  s_KnownTypes.Insert(nsMakeHashedString("vec4i"), nsExpressionAST::DataType::Int4);

  nsStringBuilder sTypeName;
  for (nsUInt32 type = nsExpressionAST::DataType::Bool; type < nsExpressionAST::DataType::Count; ++type)
  {
    sTypeName = nsExpressionAST::DataType::GetName(static_cast<nsExpressionAST::DataType::Enum>(type));
    sTypeName.ToLower();

    nsHashedString sTypeNameHashed;
    sTypeNameHashed.Assign(sTypeName);

    s_KnownTypes.Insert(sTypeNameHashed, static_cast<nsExpressionAST::DataType::Enum>(type));
  }
}

void nsExpressionParser::RegisterBuiltinFunctions()
{
  if (s_BuiltinFunctions.IsEmpty() == false)
    return;

  // Unary
  s_BuiltinFunctions.Insert(nsMakeHashedString("abs"), nsExpressionAST::NodeType::Absolute);
  s_BuiltinFunctions.Insert(nsMakeHashedString("saturate"), nsExpressionAST::NodeType::Saturate);
  s_BuiltinFunctions.Insert(nsMakeHashedString("sqrt"), nsExpressionAST::NodeType::Sqrt);
  s_BuiltinFunctions.Insert(nsMakeHashedString("exp"), nsExpressionAST::NodeType::Exp);
  s_BuiltinFunctions.Insert(nsMakeHashedString("ln"), nsExpressionAST::NodeType::Ln);
  s_BuiltinFunctions.Insert(nsMakeHashedString("log2"), nsExpressionAST::NodeType::Log2);
  s_BuiltinFunctions.Insert(nsMakeHashedString("log10"), nsExpressionAST::NodeType::Log10);
  s_BuiltinFunctions.Insert(nsMakeHashedString("pow2"), nsExpressionAST::NodeType::Pow2);
  s_BuiltinFunctions.Insert(nsMakeHashedString("sin"), nsExpressionAST::NodeType::Sin);
  s_BuiltinFunctions.Insert(nsMakeHashedString("cos"), nsExpressionAST::NodeType::Cos);
  s_BuiltinFunctions.Insert(nsMakeHashedString("tan"), nsExpressionAST::NodeType::Tan);
  s_BuiltinFunctions.Insert(nsMakeHashedString("asin"), nsExpressionAST::NodeType::ASin);
  s_BuiltinFunctions.Insert(nsMakeHashedString("acos"), nsExpressionAST::NodeType::ACos);
  s_BuiltinFunctions.Insert(nsMakeHashedString("atan"), nsExpressionAST::NodeType::ATan);
  s_BuiltinFunctions.Insert(nsMakeHashedString("radToDeg"), nsExpressionAST::NodeType::RadToDeg);
  s_BuiltinFunctions.Insert(nsMakeHashedString("rad_to_deg"), nsExpressionAST::NodeType::RadToDeg);
  s_BuiltinFunctions.Insert(nsMakeHashedString("degToRad"), nsExpressionAST::NodeType::DegToRad);
  s_BuiltinFunctions.Insert(nsMakeHashedString("deg_to_rad"), nsExpressionAST::NodeType::DegToRad);
  s_BuiltinFunctions.Insert(nsMakeHashedString("round"), nsExpressionAST::NodeType::Round);
  s_BuiltinFunctions.Insert(nsMakeHashedString("floor"), nsExpressionAST::NodeType::Floor);
  s_BuiltinFunctions.Insert(nsMakeHashedString("ceil"), nsExpressionAST::NodeType::Ceil);
  s_BuiltinFunctions.Insert(nsMakeHashedString("trunc"), nsExpressionAST::NodeType::Trunc);
  s_BuiltinFunctions.Insert(nsMakeHashedString("frac"), nsExpressionAST::NodeType::Frac);
  s_BuiltinFunctions.Insert(nsMakeHashedString("length"), nsExpressionAST::NodeType::Length);
  s_BuiltinFunctions.Insert(nsMakeHashedString("normalize"), nsExpressionAST::NodeType::Normalize);
  s_BuiltinFunctions.Insert(nsMakeHashedString("all"), nsExpressionAST::NodeType::All);
  s_BuiltinFunctions.Insert(nsMakeHashedString("any"), nsExpressionAST::NodeType::Any);

  // Binary
  s_BuiltinFunctions.Insert(nsMakeHashedString("mod"), nsExpressionAST::NodeType::Modulo);
  s_BuiltinFunctions.Insert(nsMakeHashedString("log"), nsExpressionAST::NodeType::Log);
  s_BuiltinFunctions.Insert(nsMakeHashedString("pow"), nsExpressionAST::NodeType::Pow);
  s_BuiltinFunctions.Insert(nsMakeHashedString("min"), nsExpressionAST::NodeType::Min);
  s_BuiltinFunctions.Insert(nsMakeHashedString("max"), nsExpressionAST::NodeType::Max);
  s_BuiltinFunctions.Insert(nsMakeHashedString("dot"), nsExpressionAST::NodeType::Dot);
  s_BuiltinFunctions.Insert(nsMakeHashedString("cross"), nsExpressionAST::NodeType::Cross);
  s_BuiltinFunctions.Insert(nsMakeHashedString("reflect"), nsExpressionAST::NodeType::Reflect);

  // Ternary
  s_BuiltinFunctions.Insert(nsMakeHashedString("clamp"), nsExpressionAST::NodeType::Clamp);
  s_BuiltinFunctions.Insert(nsMakeHashedString("lerp"), nsExpressionAST::NodeType::Lerp);
  s_BuiltinFunctions.Insert(nsMakeHashedString("smoothstep"), nsExpressionAST::NodeType::SmoothStep);
  s_BuiltinFunctions.Insert(nsMakeHashedString("smootherstep"), nsExpressionAST::NodeType::SmootherStep);
}

void nsExpressionParser::SetupInAndOutputs(nsArrayPtr<nsExpression::StreamDesc> inputs, nsArrayPtr<nsExpression::StreamDesc> outputs)
{
  m_KnownVariables.Clear();

  for (auto& inputDesc : inputs)
  {
    auto pInput = m_pAST->CreateInput(inputDesc);
    m_pAST->m_InputNodes.PushBack(pInput);
    m_KnownVariables.Insert(inputDesc.m_sName, pInput);
  }

  for (auto& outputDesc : outputs)
  {
    auto pOutputNode = m_pAST->CreateOutput(outputDesc, nullptr);
    m_pAST->m_OutputNodes.PushBack(pOutputNode);
    m_KnownVariables.Insert(outputDesc.m_sName, pOutputNode);
  }
}

nsResult nsExpressionParser::ParseStatement()
{
  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  if (AcceptStatementTerminator())
  {
    // empty statement
    return NS_SUCCESS;
  }

  if (m_uiCurrentToken >= m_TokenStream.GetCount())
    return NS_FAILURE;

  const nsToken* pIdentifierToken = m_TokenStream[m_uiCurrentToken];
  if (pIdentifierToken->m_iType != nsTokenType::Identifier)
  {
    ReportError(pIdentifierToken, "Syntax error, expected type or variable");
  }

  nsEnum<nsExpressionAST::DataType> type;
  if (ParseType(pIdentifierToken->m_DataView, type).Succeeded())
  {
    return ParseVariableDefinition(type);
  }

  return ParseAssignment();
}

nsResult nsExpressionParser::ParseType(nsStringView sTypeName, nsEnum<nsExpressionAST::DataType>& out_type)
{
  nsTempHashedString sTypeNameHashed(sTypeName);
  if (s_KnownTypes.TryGetValue(sTypeNameHashed, out_type))
  {
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsExpressionParser::ParseVariableDefinition(nsEnum<nsExpressionAST::DataType> type)
{
  // skip type
  NS_SUCCEED_OR_RETURN(Expect(nsTokenType::Identifier));

  const nsToken* pIdentifierToken = nullptr;
  NS_SUCCEED_OR_RETURN(Expect(nsTokenType::Identifier, &pIdentifierToken));

  nsHashedString sHashedVarName;
  sHashedVarName.Assign(pIdentifierToken->m_DataView);

  nsExpressionAST::Node* pVariableNode;
  if (m_KnownVariables.TryGetValue(sHashedVarName, pVariableNode))
  {
    const char* szExisting = "a variable";
    if (nsExpressionAST::NodeType::IsInput(pVariableNode->m_Type))
    {
      szExisting = "an input";
    }
    else if (nsExpressionAST::NodeType::IsOutput(pVariableNode->m_Type))
    {
      szExisting = "an output";
    }

    ReportError(pIdentifierToken, nsFmt("Local variable '{}' cannot be defined because {} of the same name already exists", pIdentifierToken->m_DataView, szExisting));
    return NS_FAILURE;
  }

  NS_SUCCEED_OR_RETURN(Expect("="));
  nsExpressionAST::Node* pExpression = ParseExpression();
  if (pExpression == nullptr)
    return NS_FAILURE;

  m_KnownVariables.Insert(sHashedVarName, EnsureExpectedType(pExpression, type));
  return NS_SUCCESS;
}

nsResult nsExpressionParser::ParseAssignment()
{
  const nsToken* pIdentifierToken = nullptr;
  NS_SUCCEED_OR_RETURN(Expect(nsTokenType::Identifier, &pIdentifierToken));

  const nsStringView sIdentifier = pIdentifierToken->m_DataView;
  nsExpressionAST::Node* pVarNode = GetVariable(sIdentifier);
  if (pVarNode == nullptr)
  {
    ReportError(pIdentifierToken, "Syntax error, expected a valid variable");
    return NS_FAILURE;
  }

  nsStringView sPartialAssignmentMask;
  if (Accept(m_TokenStream, m_uiCurrentToken, "."))
  {
    const nsToken* pSwizzleToken = nullptr;
    if (Expect(nsTokenType::Identifier, &pSwizzleToken).Failed())
    {
      ReportError(m_TokenStream[m_uiCurrentToken], "Invalid partial assignment");
      return NS_FAILURE;
    }

    sPartialAssignmentMask = pSwizzleToken->m_DataView;
  }

  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  nsExpressionAST::NodeType::Enum assignOperator = nsExpressionAST::NodeType::Invalid;
  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(s_assignOperators); ++i)
  {
    auto& op = s_assignOperators[i];
    if (AcceptOperator(op.m_sName))
    {
      assignOperator = op.m_NodeType;
      m_uiCurrentToken += op.m_sName.GetElementCount();
      break;
    }
  }

  if (assignOperator == nsExpressionAST::NodeType::Invalid)
  {
    NS_SUCCEED_OR_RETURN(Expect("="));
  }

  nsExpressionAST::Node* pExpression = ParseExpression();
  if (pExpression == nullptr)
    return NS_FAILURE;

  if (assignOperator != nsExpressionAST::NodeType::Invalid)
  {
    pExpression = m_pAST->CreateBinaryOperator(assignOperator, Unpack(pVarNode), pExpression);
  }

  if (sPartialAssignmentMask.IsEmpty() == false)
  {
    auto pConstructor = m_pAST->CreateConstructorCall(Unpack(pVarNode, false), pExpression, sPartialAssignmentMask);
    if (pConstructor == nullptr)
    {
      ReportError(pIdentifierToken, nsFmt("Invalid partial assignment .{} = {}", sPartialAssignmentMask, nsExpressionAST::DataType::GetName(pExpression->m_ReturnType)));
      return NS_FAILURE;
    }

    pExpression = pConstructor;
  }

  if (nsExpressionAST::NodeType::IsInput(pVarNode->m_Type))
  {
    ReportError(pIdentifierToken, nsFmt("Input '{}' is not assignable", sIdentifier));
    return NS_FAILURE;
  }
  else if (nsExpressionAST::NodeType::IsOutput(pVarNode->m_Type))
  {
    auto pOutput = static_cast<nsExpressionAST::Output*>(pVarNode);
    pOutput->m_pExpression = pExpression;
    return NS_SUCCESS;
  }

  nsHashedString sHashedVarName;
  sHashedVarName.Assign(sIdentifier);
  m_KnownVariables[sHashedVarName] = EnsureExpectedType(pExpression, pVarNode->m_ReturnType);
  return NS_SUCCESS;
}

nsExpressionAST::Node* nsExpressionParser::ParseFactor()
{
  nsUInt32 uiIdentifierToken = 0;
  if (Accept(m_TokenStream, m_uiCurrentToken, nsTokenType::Identifier, &uiIdentifierToken))
  {
    auto pIdentifierToken = m_TokenStream[uiIdentifierToken];
    const nsStringView sIdentifier = pIdentifierToken->m_DataView;

    if (Accept(m_TokenStream, m_uiCurrentToken, "("))
    {
      return ParseSwizzle(ParseFunctionCall(sIdentifier));
    }
    else if (sIdentifier == "true")
    {
      return m_pAST->CreateConstant(true, nsExpressionAST::DataType::Bool);
    }
    else if (sIdentifier == "false")
    {
      return m_pAST->CreateConstant(false, nsExpressionAST::DataType::Bool);
    }
    else if (sIdentifier == "PI")
    {
      return m_pAST->CreateConstant(nsMath::Pi<float>(), nsExpressionAST::DataType::Float);
    }
    else
    {
      auto pVariable = GetVariable(sIdentifier);
      if (pVariable == nullptr)
      {
        ReportError(pIdentifierToken, nsFmt("Undeclared identifier '{}'", sIdentifier));
        return nullptr;
      }
      return ParseSwizzle(Unpack(pVariable));
    }
  }

  nsUInt32 uiValueToken = 0;
  if (Accept(m_TokenStream, m_uiCurrentToken, nsTokenType::Integer, &uiValueToken))
  {
    const nsString sVal = m_TokenStream[uiValueToken]->m_DataView;

    nsInt64 iConstant = 0;
    if (sVal.StartsWith_NoCase("0x"))
    {
      nsUInt64 uiHexConstant = 0;
      nsConversionUtils::ConvertHexStringToUInt64(sVal, uiHexConstant).IgnoreResult();
      iConstant = uiHexConstant;
    }
    else
    {
      nsConversionUtils::StringToInt64(sVal, iConstant).IgnoreResult();
    }

    return m_pAST->CreateConstant((int)iConstant, nsExpressionAST::DataType::Int);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, nsTokenType::Float, &uiValueToken))
  {
    const nsString sVal = m_TokenStream[uiValueToken]->m_DataView;

    double fConstant = 0;
    nsConversionUtils::StringToFloat(sVal, fConstant).IgnoreResult();

    return m_pAST->CreateConstant((float)fConstant, nsExpressionAST::DataType::Float);
  }

  if (Accept(m_TokenStream, m_uiCurrentToken, "("))
  {
    auto pExpression = ParseExpression();
    if (Expect(")").Failed())
      return nullptr;

    return ParseSwizzle(pExpression);
  }

  return nullptr;
}

// Parsing the expression - recursive parser using "precedence climbing".
// http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm
nsExpressionAST::Node* nsExpressionParser::ParseExpression(int iPrecedence /* = s_iLowestPrecedence*/)
{
  auto pExpression = ParseUnaryExpression();
  if (pExpression == nullptr)
    return nullptr;

  nsExpressionAST::NodeType::Enum binaryOp;
  int iBinaryOpPrecedence = 0;
  nsUInt32 uiOperatorLength = 0;
  while (AcceptBinaryOperator(binaryOp, iBinaryOpPrecedence, uiOperatorLength) && iBinaryOpPrecedence < iPrecedence)
  {
    // Consume token.
    m_uiCurrentToken += uiOperatorLength;

    auto pSecondOperand = ParseExpression(iBinaryOpPrecedence);
    if (pSecondOperand == nullptr)
      return nullptr;

    if (binaryOp == nsExpressionAST::NodeType::Select)
    {
      if (Expect(":").Failed())
        return nullptr;

      auto pThirdOperand = ParseExpression(iBinaryOpPrecedence);
      if (pThirdOperand == nullptr)
        return nullptr;

      pExpression = m_pAST->CreateTernaryOperator(nsExpressionAST::NodeType::Select, pExpression, pSecondOperand, pThirdOperand);
    }
    else
    {
      pExpression = m_pAST->CreateBinaryOperator(binaryOp, pExpression, pSecondOperand);
    }
  }

  return pExpression;
}

nsExpressionAST::Node* nsExpressionParser::ParseUnaryExpression()
{
  while (Accept(m_TokenStream, m_uiCurrentToken, "+"))
  {
  }

  if (Accept(m_TokenStream, m_uiCurrentToken, "-"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(nsExpressionAST::NodeType::Negate, pOperand);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, "~"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(nsExpressionAST::NodeType::BitwiseNot, pOperand);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, "!"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(nsExpressionAST::NodeType::LogicalNot, pOperand);
  }

  return ParseFactor();
}

nsExpressionAST::Node* nsExpressionParser::ParseFunctionCall(nsStringView sFunctionName)
{
  // "(" of the function call
  const nsToken* pFunctionToken = m_TokenStream[m_uiCurrentToken - 1];

  nsSmallArray<nsExpressionAST::Node*, 8> arguments;
  if (Accept(m_TokenStream, m_uiCurrentToken, ")") == false)
  {
    do
    {
      arguments.PushBack(ParseExpression());
    } while (Accept(m_TokenStream, m_uiCurrentToken, ","));

    if (Expect(")").Failed())
      return nullptr;
  }

  auto CheckArgumentCount = [&](nsUInt32 uiExpectedArgumentCount) -> nsResult
  {
    if (arguments.GetCount() != uiExpectedArgumentCount)
    {
      ReportError(pFunctionToken, nsFmt("Invalid argument count for '{}'. Expected {} but got {}", sFunctionName, uiExpectedArgumentCount, arguments.GetCount()));
      return NS_FAILURE;
    }
    return NS_SUCCESS;
  };

  nsHashedString sHashedFuncName;
  sHashedFuncName.Assign(sFunctionName);

  nsEnum<nsExpressionAST::DataType> dataType;
  if (s_KnownTypes.TryGetValue(sHashedFuncName, dataType))
  {
    nsUInt32 uiElementCount = nsExpressionAST::DataType::GetElementCount(dataType);
    if (arguments.GetCount() > uiElementCount)
    {
      ReportError(pFunctionToken, nsFmt("Invalid argument count for '{}'. Expected 0 - {} but got {}", sFunctionName, uiElementCount, arguments.GetCount()));
      return nullptr;
    }

    return m_pAST->CreateConstructorCall(dataType, arguments);
  }

  nsEnum<nsExpressionAST::NodeType> builtinType;
  if (s_BuiltinFunctions.TryGetValue(sHashedFuncName, builtinType))
  {
    if (nsExpressionAST::NodeType::IsUnary(builtinType))
    {
      if (CheckArgumentCount(1).Failed())
        return nullptr;

      return m_pAST->CreateUnaryOperator(builtinType, arguments[0]);
    }
    else if (nsExpressionAST::NodeType::IsBinary(builtinType))
    {
      if (CheckArgumentCount(2).Failed())
        return nullptr;

      return m_pAST->CreateBinaryOperator(builtinType, arguments[0], arguments[1]);
    }
    else if (nsExpressionAST::NodeType::IsTernary(builtinType))
    {
      if (CheckArgumentCount(3).Failed())
        return nullptr;

      return m_pAST->CreateTernaryOperator(builtinType, arguments[0], arguments[1], arguments[2]);
    }

    NS_ASSERT_NOT_IMPLEMENTED;
    return nullptr;
  }

  // external function
  const nsHybridArray<nsExpression::FunctionDesc, 1>* pFunctionDescs = nullptr;
  if (m_FunctionDescs.TryGetValue(sHashedFuncName, pFunctionDescs))
  {
    nsUInt32 uiMinArgumentCount = nsInvalidIndex;
    for (auto& funcDesc : *pFunctionDescs)
    {
      uiMinArgumentCount = nsMath::Min<nsUInt32>(uiMinArgumentCount, funcDesc.m_uiNumRequiredInputs);
    }

    if (arguments.GetCount() < uiMinArgumentCount)
    {
      ReportError(pFunctionToken, nsFmt("Invalid argument count for '{}'. Expected at least {} but got {}", sFunctionName, uiMinArgumentCount, arguments.GetCount()));
      return nullptr;
    }

    return m_pAST->CreateFunctionCall(*pFunctionDescs, arguments);
  }

  ReportError(pFunctionToken, nsFmt("Undeclared function '{}'", sFunctionName));
  return nullptr;
}

nsExpressionAST::Node* nsExpressionParser::ParseSwizzle(nsExpressionAST::Node* pExpression)
{
  if (Accept(m_TokenStream, m_uiCurrentToken, "."))
  {
    const nsToken* pSwizzleToken = nullptr;
    if (Expect(nsTokenType::Identifier, &pSwizzleToken).Failed())
      return nullptr;

    pExpression = m_pAST->CreateSwizzle(pSwizzleToken->m_DataView, pExpression);
    if (pExpression == nullptr)
    {
      ReportError(pSwizzleToken, nsFmt("Invalid swizzle '{}'", pSwizzleToken->m_DataView));
    }
  }

  return pExpression;
}

// Does NOT advance the current token beyond the operator!
bool nsExpressionParser::AcceptOperator(nsStringView sName)
{
  const nsUInt32 uiOperatorLength = sName.GetElementCount();

  if (m_uiCurrentToken + uiOperatorLength - 1 >= m_TokenStream.GetCount())
    return false;

  for (nsUInt32 charIndex = 0; charIndex < uiOperatorLength; ++charIndex)
  {
    if (m_TokenStream[m_uiCurrentToken + charIndex]->m_DataView.GetCharacter() != sName.GetStartPointer()[charIndex])
    {
      return false;
    }
  }

  return true;
}

// Does NOT advance the current token beyond the binary operator!
bool nsExpressionParser::AcceptBinaryOperator(nsExpressionAST::NodeType::Enum& out_binaryOp, int& out_iOperatorPrecedence, nsUInt32& out_uiOperatorLength)
{
  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(s_binaryOperators); ++i)
  {
    auto& op = s_binaryOperators[i];
    if (AcceptOperator(op.m_sName))
    {
      out_binaryOp = op.m_NodeType;
      out_iOperatorPrecedence = op.m_iPrecedence;
      out_uiOperatorLength = op.m_sName.GetElementCount();
      return true;
    }
  }

  return false;
}

nsExpressionAST::Node* nsExpressionParser::GetVariable(nsStringView sVarName)
{
  nsHashedString sHashedVarName;
  sHashedVarName.Assign(sVarName);

  nsExpressionAST::Node* pVariableNode = nullptr;
  if (m_KnownVariables.TryGetValue(sHashedVarName, pVariableNode) == false && m_Options.m_bTreatUnknownVariablesAsInputs)
  {
    pVariableNode = m_pAST->CreateInput({sHashedVarName, nsProcessingStream::DataType::Float});
    m_KnownVariables.Insert(sHashedVarName, pVariableNode);
  }

  return pVariableNode;
}

nsExpressionAST::Node* nsExpressionParser::EnsureExpectedType(nsExpressionAST::Node* pNode, nsExpressionAST::DataType::Enum expectedType)
{
  if (expectedType != nsExpressionAST::DataType::Unknown)
  {
    const auto nodeRegisterType = nsExpressionAST::DataType::GetRegisterType(pNode->m_ReturnType);
    const auto expectedRegisterType = nsExpressionAST::DataType::GetRegisterType(expectedType);
    if (nodeRegisterType != expectedRegisterType)
    {
      pNode = m_pAST->CreateUnaryOperator(nsExpressionAST::NodeType::TypeConversion, pNode, expectedType);
    }

    const nsUInt32 nodeElementCount = nsExpressionAST::DataType::GetElementCount(pNode->m_ReturnType);
    const nsUInt32 expectedElementCount = nsExpressionAST::DataType::GetElementCount(expectedType);
    if (nodeElementCount < expectedElementCount)
    {
      pNode = m_pAST->CreateConstructorCall(expectedType, nsMakeArrayPtr(&pNode, 1));
    }
  }

  return pNode;
}

nsExpressionAST::Node* nsExpressionParser::Unpack(nsExpressionAST::Node* pNode, bool bUnassignedError /*= true*/)
{
  if (nsExpressionAST::NodeType::IsOutput(pNode->m_Type))
  {
    auto pOutput = static_cast<nsExpressionAST::Output*>(pNode);
    if (pOutput->m_pExpression == nullptr && bUnassignedError)
    {
      ReportError(m_TokenStream[m_uiCurrentToken], nsFmt("Output '{}' has not been assigned yet", pOutput->m_Desc.m_sName));
    }

    return pOutput->m_pExpression;
  }

  return pNode;
}

nsResult nsExpressionParser::CheckOutputs()
{
  for (auto pOutputNode : m_pAST->m_OutputNodes)
  {
    if (pOutputNode->m_pExpression == nullptr)
    {
      nsLog::Error("Output '{}' was never written", pOutputNode->m_Desc.m_sName);
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}
