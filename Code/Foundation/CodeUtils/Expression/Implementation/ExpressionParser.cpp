#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/Tokenizer.h>

namespace
{
  struct AssignOperator
  {
    wdStringView m_sName;
    wdExpressionAST::NodeType::Enum m_NodeType;
  };

  static constexpr AssignOperator s_assignOperators[] = {
    {"+="_wdsv, wdExpressionAST::NodeType::Add},
    {"-="_wdsv, wdExpressionAST::NodeType::Subtract},
    {"*="_wdsv, wdExpressionAST::NodeType::Multiply},
    {"/="_wdsv, wdExpressionAST::NodeType::Divide},
    {"%="_wdsv, wdExpressionAST::NodeType::Modulo},
    {"<<="_wdsv, wdExpressionAST::NodeType::BitshiftLeft},
    {">>="_wdsv, wdExpressionAST::NodeType::BitshiftRight},
    {"&="_wdsv, wdExpressionAST::NodeType::BitwiseAnd},
    {"^="_wdsv, wdExpressionAST::NodeType::BitwiseXor},
    {"|="_wdsv, wdExpressionAST::NodeType::BitwiseOr},
  };

  struct BinaryOperator
  {
    wdStringView m_sName;
    wdExpressionAST::NodeType::Enum m_NodeType;
    int m_iPrecedence;
  };

  // Operator precedence according to https://en.cppreference.com/w/cpp/language/operator_precedence,
  // lower value means higher precedence
  // sorted by string length to simplify the test against a token stream
  static constexpr BinaryOperator s_binaryOperators[] = {
    {"&&"_wdsv, wdExpressionAST::NodeType::LogicalAnd, 14},
    {"||"_wdsv, wdExpressionAST::NodeType::LogicalOr, 15},
    {"<<"_wdsv, wdExpressionAST::NodeType::BitshiftLeft, 7},
    {">>"_wdsv, wdExpressionAST::NodeType::BitshiftRight, 7},
    {"=="_wdsv, wdExpressionAST::NodeType::Equal, 10},
    {"!="_wdsv, wdExpressionAST::NodeType::NotEqual, 10},
    {"<="_wdsv, wdExpressionAST::NodeType::LessEqual, 9},
    {">="_wdsv, wdExpressionAST::NodeType::GreaterEqual, 9},
    {"<"_wdsv, wdExpressionAST::NodeType::Less, 9},
    {">"_wdsv, wdExpressionAST::NodeType::Greater, 9},
    {"&"_wdsv, wdExpressionAST::NodeType::BitwiseAnd, 11},
    {"^"_wdsv, wdExpressionAST::NodeType::BitwiseXor, 12},
    {"|"_wdsv, wdExpressionAST::NodeType::BitwiseOr, 13},
    {"?"_wdsv, wdExpressionAST::NodeType::Select, 16},
    {"+"_wdsv, wdExpressionAST::NodeType::Add, 6},
    {"-"_wdsv, wdExpressionAST::NodeType::Subtract, 6},
    {"*"_wdsv, wdExpressionAST::NodeType::Multiply, 5},
    {"/"_wdsv, wdExpressionAST::NodeType::Divide, 5},
    {"%"_wdsv, wdExpressionAST::NodeType::Modulo, 5},
  };

} // namespace

using namespace wdTokenParseUtils;

wdExpressionParser::wdExpressionParser()
{
  RegisterKnownTypes();
  RegisterBuiltinFunctions();
}

wdExpressionParser::~wdExpressionParser() = default;

void wdExpressionParser::RegisterFunction(const wdExpression::FunctionDesc& funcDesc)
{
  WD_ASSERT_DEV(funcDesc.m_uiNumRequiredInputs <= funcDesc.m_InputTypes.GetCount(), "Not enough input types defined. {} inputs are required but only {} types given.", funcDesc.m_uiNumRequiredInputs, funcDesc.m_InputTypes.GetCount());

  auto& functionDescs = m_FunctionDescs[funcDesc.m_sName];
  if (functionDescs.Contains(funcDesc) == false)
  {
    functionDescs.PushBack(funcDesc);
  }
}

void wdExpressionParser::UnregisterFunction(const wdExpression::FunctionDesc& funcDesc)
{
  if (auto pFunctionDescs = m_FunctionDescs.GetValue(funcDesc.m_sName))
  {
    pFunctionDescs->RemoveAndCopy(funcDesc);
  }
}

wdResult wdExpressionParser::Parse(wdStringView sCode, wdArrayPtr<wdExpression::StreamDesc> inputs, wdArrayPtr<wdExpression::StreamDesc> outputs, const Options& options, wdExpressionAST& out_ast)
{
  if (sCode.IsEmpty())
    return WD_FAILURE;

  m_Options = options;

  m_pAST = &out_ast;
  SetupInAndOutputs(inputs, outputs);

  wdTokenizer tokenizer;
  tokenizer.Tokenize(wdArrayPtr<const wdUInt8>((const wdUInt8*)sCode.GetStartPointer(), sCode.GetElementCount()), wdLog::GetThreadLocalLogSystem());

  wdUInt32 readTokens = 0;
  while (tokenizer.GetNextLine(readTokens, m_TokenStream).Succeeded())
  {
    m_uiCurrentToken = 0;

    while (m_uiCurrentToken < m_TokenStream.GetCount())
    {
      WD_SUCCEED_OR_RETURN(ParseStatement());

      if (m_uiCurrentToken < m_TokenStream.GetCount() && AcceptStatementTerminator() == false)
      {
        auto pCurrentToken = m_TokenStream[m_uiCurrentToken];
        ReportError(pCurrentToken, wdFmt("Syntax error, unexpected token '{}'", pCurrentToken->m_DataView));
        return WD_FAILURE;
      }
    }
  }

  WD_SUCCEED_OR_RETURN(CheckOutputs());

  return WD_SUCCESS;
}

void wdExpressionParser::RegisterKnownTypes()
{
  m_KnownTypes.Insert(wdMakeHashedString("var"), wdExpressionAST::DataType::Unknown);

  m_KnownTypes.Insert(wdMakeHashedString("vec2"), wdExpressionAST::DataType::Float2);
  m_KnownTypes.Insert(wdMakeHashedString("vec3"), wdExpressionAST::DataType::Float3);
  m_KnownTypes.Insert(wdMakeHashedString("vec4"), wdExpressionAST::DataType::Float4);

  m_KnownTypes.Insert(wdMakeHashedString("vec2i"), wdExpressionAST::DataType::Int2);
  m_KnownTypes.Insert(wdMakeHashedString("vec3i"), wdExpressionAST::DataType::Int3);
  m_KnownTypes.Insert(wdMakeHashedString("vec4i"), wdExpressionAST::DataType::Int4);

  wdStringBuilder sTypeName;
  for (wdUInt32 type = wdExpressionAST::DataType::Bool; type < wdExpressionAST::DataType::Count; ++type)
  {
    sTypeName = wdExpressionAST::DataType::GetName(static_cast<wdExpressionAST::DataType::Enum>(type));
    sTypeName.ToLower();

    wdHashedString sTypeNameHashed;
    sTypeNameHashed.Assign(sTypeName);

    m_KnownTypes.Insert(sTypeNameHashed, static_cast<wdExpressionAST::DataType::Enum>(type));
  }
}

void wdExpressionParser::RegisterBuiltinFunctions()
{
  // Unary
  m_BuiltinFunctions.Insert(wdMakeHashedString("abs"), wdExpressionAST::NodeType::Absolute);
  m_BuiltinFunctions.Insert(wdMakeHashedString("saturate"), wdExpressionAST::NodeType::Saturate);
  m_BuiltinFunctions.Insert(wdMakeHashedString("sqrt"), wdExpressionAST::NodeType::Sqrt);
  m_BuiltinFunctions.Insert(wdMakeHashedString("exp"), wdExpressionAST::NodeType::Exp);
  m_BuiltinFunctions.Insert(wdMakeHashedString("ln"), wdExpressionAST::NodeType::Ln);
  m_BuiltinFunctions.Insert(wdMakeHashedString("log2"), wdExpressionAST::NodeType::Log2);
  m_BuiltinFunctions.Insert(wdMakeHashedString("log10"), wdExpressionAST::NodeType::Log10);
  m_BuiltinFunctions.Insert(wdMakeHashedString("pow2"), wdExpressionAST::NodeType::Pow2);
  m_BuiltinFunctions.Insert(wdMakeHashedString("sin"), wdExpressionAST::NodeType::Sin);
  m_BuiltinFunctions.Insert(wdMakeHashedString("cos"), wdExpressionAST::NodeType::Cos);
  m_BuiltinFunctions.Insert(wdMakeHashedString("tan"), wdExpressionAST::NodeType::Tan);
  m_BuiltinFunctions.Insert(wdMakeHashedString("asin"), wdExpressionAST::NodeType::ASin);
  m_BuiltinFunctions.Insert(wdMakeHashedString("acos"), wdExpressionAST::NodeType::ACos);
  m_BuiltinFunctions.Insert(wdMakeHashedString("atan"), wdExpressionAST::NodeType::ATan);
  m_BuiltinFunctions.Insert(wdMakeHashedString("radToDeg"), wdExpressionAST::NodeType::RadToDeg);
  m_BuiltinFunctions.Insert(wdMakeHashedString("rad_to_deg"), wdExpressionAST::NodeType::RadToDeg);
  m_BuiltinFunctions.Insert(wdMakeHashedString("degToRad"), wdExpressionAST::NodeType::DegToRad);
  m_BuiltinFunctions.Insert(wdMakeHashedString("deg_to_rad"), wdExpressionAST::NodeType::DegToRad);
  m_BuiltinFunctions.Insert(wdMakeHashedString("round"), wdExpressionAST::NodeType::Round);
  m_BuiltinFunctions.Insert(wdMakeHashedString("floor"), wdExpressionAST::NodeType::Floor);
  m_BuiltinFunctions.Insert(wdMakeHashedString("ceil"), wdExpressionAST::NodeType::Ceil);
  m_BuiltinFunctions.Insert(wdMakeHashedString("trunc"), wdExpressionAST::NodeType::Trunc);
  m_BuiltinFunctions.Insert(wdMakeHashedString("frac"), wdExpressionAST::NodeType::Frac);
  m_BuiltinFunctions.Insert(wdMakeHashedString("length"), wdExpressionAST::NodeType::Length);
  m_BuiltinFunctions.Insert(wdMakeHashedString("normalize"), wdExpressionAST::NodeType::Normalize);
  m_BuiltinFunctions.Insert(wdMakeHashedString("all"), wdExpressionAST::NodeType::All);
  m_BuiltinFunctions.Insert(wdMakeHashedString("any"), wdExpressionAST::NodeType::Any);

  // Binary
  m_BuiltinFunctions.Insert(wdMakeHashedString("mod"), wdExpressionAST::NodeType::Modulo);
  m_BuiltinFunctions.Insert(wdMakeHashedString("log"), wdExpressionAST::NodeType::Log);
  m_BuiltinFunctions.Insert(wdMakeHashedString("pow"), wdExpressionAST::NodeType::Pow);
  m_BuiltinFunctions.Insert(wdMakeHashedString("min"), wdExpressionAST::NodeType::Min);
  m_BuiltinFunctions.Insert(wdMakeHashedString("max"), wdExpressionAST::NodeType::Max);
  m_BuiltinFunctions.Insert(wdMakeHashedString("dot"), wdExpressionAST::NodeType::Dot);
  m_BuiltinFunctions.Insert(wdMakeHashedString("cross"), wdExpressionAST::NodeType::Cross);
  m_BuiltinFunctions.Insert(wdMakeHashedString("reflect"), wdExpressionAST::NodeType::Reflect);

  // Ternary
  m_BuiltinFunctions.Insert(wdMakeHashedString("clamp"), wdExpressionAST::NodeType::Clamp);
  m_BuiltinFunctions.Insert(wdMakeHashedString("lerp"), wdExpressionAST::NodeType::Lerp);
}

void wdExpressionParser::SetupInAndOutputs(wdArrayPtr<wdExpression::StreamDesc> inputs, wdArrayPtr<wdExpression::StreamDesc> outputs)
{
  m_KnownVariables.Clear();

  for (auto& inputDesc : inputs)
  {
    auto pInput = m_pAST->CreateInput(inputDesc);
    m_KnownVariables.Insert(inputDesc.m_sName, pInput);
  }

  for (auto& outputDesc : outputs)
  {
    auto pOutputNode = m_pAST->CreateOutput(outputDesc, nullptr);
    m_pAST->m_OutputNodes.PushBack(pOutputNode);
    m_KnownVariables.Insert(outputDesc.m_sName, pOutputNode);
  }
}

wdResult wdExpressionParser::ParseStatement()
{
  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  if (AcceptStatementTerminator())
  {
    // empty statement
    return WD_SUCCESS;
  }

  if (m_uiCurrentToken >= m_TokenStream.GetCount())
    return WD_FAILURE;

  const wdToken* pIdentifierToken = m_TokenStream[m_uiCurrentToken];
  if (pIdentifierToken->m_iType != wdTokenType::Identifier)
  {
    ReportError(pIdentifierToken, "Syntax error, expected type or variable");
  }

  wdEnum<wdExpressionAST::DataType> type;
  if (ParseType(pIdentifierToken->m_DataView, type).Succeeded())
  {
    return ParseVariableDefinition(type);
  }

  return ParseAssignment();
}

wdResult wdExpressionParser::ParseType(wdStringView sTypeName, wdEnum<wdExpressionAST::DataType>& out_type)
{
  wdTempHashedString sTypeNameHashed(sTypeName);
  if (m_KnownTypes.TryGetValue(sTypeNameHashed, out_type))
  {
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdExpressionParser::ParseVariableDefinition(wdEnum<wdExpressionAST::DataType> type)
{
  // skip type
  WD_SUCCEED_OR_RETURN(Expect(wdTokenType::Identifier));

  const wdToken* pIdentifierToken = nullptr;
  WD_SUCCEED_OR_RETURN(Expect(wdTokenType::Identifier, &pIdentifierToken));

  wdHashedString sHashedVarName;
  sHashedVarName.Assign(pIdentifierToken->m_DataView);

  wdExpressionAST::Node* pVariableNode;
  if (m_KnownVariables.TryGetValue(sHashedVarName, pVariableNode))
  {
    const char* szExisting = "a variable";
    if (wdExpressionAST::NodeType::IsInput(pVariableNode->m_Type))
    {
      szExisting = "an input";
    }
    else if (wdExpressionAST::NodeType::IsOutput(pVariableNode->m_Type))
    {
      szExisting = "an output";
    }

    ReportError(pIdentifierToken, wdFmt("Local variable '{}' cannot be defined because {} of the same name already exists", pIdentifierToken->m_DataView, szExisting));
    return WD_FAILURE;
  }

  WD_SUCCEED_OR_RETURN(Expect("="));
  wdExpressionAST::Node* pExpression = ParseExpression();
  if (pExpression == nullptr)
    return WD_FAILURE;

  m_KnownVariables.Insert(sHashedVarName, EnsureExpectedType(pExpression, type));
  return WD_SUCCESS;
}

wdResult wdExpressionParser::ParseAssignment()
{
  const wdToken* pIdentifierToken = nullptr;
  WD_SUCCEED_OR_RETURN(Expect(wdTokenType::Identifier, &pIdentifierToken));

  const wdStringView sIdentifier = pIdentifierToken->m_DataView;
  wdExpressionAST::Node* pVarNode = GetVariable(sIdentifier);
  if (pVarNode == nullptr)
  {
    ReportError(pIdentifierToken, "Syntax error, expected a valid variable");
    return WD_FAILURE;
  }

  wdStringView sPartialAssignmentMask;
  if (Accept(m_TokenStream, m_uiCurrentToken, "."))
  {
    const wdToken* pSwizzleToken = nullptr;
    if (Expect(wdTokenType::Identifier, &pSwizzleToken).Failed())
    {
      ReportError(m_TokenStream[m_uiCurrentToken], "Invalid partial assignment");
      return WD_FAILURE;
    }

    sPartialAssignmentMask = pSwizzleToken->m_DataView;
  }

  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  wdExpressionAST::NodeType::Enum assignOperator = wdExpressionAST::NodeType::Invalid;
  for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(s_assignOperators); ++i)
  {
    auto& op = s_assignOperators[i];
    if (AcceptOperator(op.m_sName))
    {
      assignOperator = op.m_NodeType;
      m_uiCurrentToken += op.m_sName.GetElementCount();
      break;
    }
  }

  if (assignOperator == wdExpressionAST::NodeType::Invalid)
  {
    WD_SUCCEED_OR_RETURN(Expect("="));
  }

  wdExpressionAST::Node* pExpression = ParseExpression();
  if (pExpression == nullptr)
    return WD_FAILURE;

  if (assignOperator != wdExpressionAST::NodeType::Invalid)
  {
    pExpression = m_pAST->CreateBinaryOperator(assignOperator, Unpack(pVarNode), pExpression);
  }

  if (sPartialAssignmentMask.IsEmpty() == false)
  {
    auto pConstructor = m_pAST->CreateConstructorCall(Unpack(pVarNode, false), pExpression, sPartialAssignmentMask);
    if (pConstructor == nullptr)
    {
      ReportError(pIdentifierToken, wdFmt("Invalid partial assignment .{} = {}", sPartialAssignmentMask, wdExpressionAST::DataType::GetName(pExpression->m_ReturnType)));
      return WD_FAILURE;
    }

    pExpression = pConstructor;
  }

  if (wdExpressionAST::NodeType::IsInput(pVarNode->m_Type))
  {
    ReportError(pIdentifierToken, wdFmt("Input '{}' is not assignable", sIdentifier));
    return WD_FAILURE;
  }
  else if (wdExpressionAST::NodeType::IsOutput(pVarNode->m_Type))
  {
    auto pOutput = static_cast<wdExpressionAST::Output*>(pVarNode);
    pOutput->m_pExpression = pExpression;
    return WD_SUCCESS;
  }

  wdHashedString sHashedVarName;
  sHashedVarName.Assign(sIdentifier);
  m_KnownVariables[sHashedVarName] = EnsureExpectedType(pExpression, pVarNode->m_ReturnType);
  return WD_SUCCESS;
}

wdExpressionAST::Node* wdExpressionParser::ParseFactor()
{
  wdUInt32 uiIdentifierToken = 0;
  if (Accept(m_TokenStream, m_uiCurrentToken, wdTokenType::Identifier, &uiIdentifierToken))
  {
    auto pIdentifierToken = m_TokenStream[uiIdentifierToken];
    const wdStringView sIdentifier = pIdentifierToken->m_DataView;

    wdExpressionAST::Node* pNode = nullptr;

    if (Accept(m_TokenStream, m_uiCurrentToken, "("))
    {
      return ParseSwizzle(ParseFunctionCall(sIdentifier));
    }
    else if (sIdentifier == "true")
    {
      return m_pAST->CreateConstant(true, wdExpressionAST::DataType::Bool);
    }
    else if (sIdentifier == "false")
    {
      return m_pAST->CreateConstant(false, wdExpressionAST::DataType::Bool);
    }
    else if (sIdentifier == "PI")
    {
      return m_pAST->CreateConstant(wdMath::Pi<float>(), wdExpressionAST::DataType::Float);
    }
    else
    {
      auto pVariable = GetVariable(sIdentifier);
      if (pVariable == nullptr)
      {
        ReportError(pIdentifierToken, wdFmt("Undeclared identifier '{}'", sIdentifier));
      }
      return ParseSwizzle(Unpack(pVariable));
    }
  }

  wdUInt32 uiValueToken = 0;
  if (Accept(m_TokenStream, m_uiCurrentToken, wdTokenType::Integer, &uiValueToken))
  {
    const wdString sVal = m_TokenStream[uiValueToken]->m_DataView;

    wdInt64 iConstant = 0;
    if (sVal.StartsWith_NoCase("0x"))
    {
      wdUInt64 uiHexConstant = 0;
      wdConversionUtils::ConvertHexStringToUInt64(sVal, uiHexConstant).IgnoreResult();
      iConstant = uiHexConstant;
    }
    else
    {
      wdConversionUtils::StringToInt64(sVal, iConstant).IgnoreResult();
    }

    return m_pAST->CreateConstant((int)iConstant, wdExpressionAST::DataType::Int);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, wdTokenType::Float, &uiValueToken))
  {
    const wdString sVal = m_TokenStream[uiValueToken]->m_DataView;

    double fConstant = 0;
    wdConversionUtils::StringToFloat(sVal, fConstant).IgnoreResult();

    return m_pAST->CreateConstant((float)fConstant, wdExpressionAST::DataType::Float);
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
wdExpressionAST::Node* wdExpressionParser::ParseExpression(int iPrecedence /* = s_iLowestPrecedence*/)
{
  auto pExpression = ParseUnaryExpression();
  if (pExpression == nullptr)
    return nullptr;

  wdExpressionAST::NodeType::Enum binaryOp;
  int iBinaryOpPrecedence = 0;
  wdUInt32 uiOperatorLength = 0;
  while (AcceptBinaryOperator(binaryOp, iBinaryOpPrecedence, uiOperatorLength) && iBinaryOpPrecedence < iPrecedence)
  {
    // Consume token.
    m_uiCurrentToken += uiOperatorLength;

    auto pSecondOperand = ParseExpression(iBinaryOpPrecedence);
    if (pSecondOperand == nullptr)
      return nullptr;

    if (binaryOp == wdExpressionAST::NodeType::Select)
    {
      if (Expect(":").Failed())
        return nullptr;

      auto pThirdOperand = ParseExpression(iBinaryOpPrecedence);
      if (pThirdOperand == nullptr)
        return nullptr;

      pExpression = m_pAST->CreateTernaryOperator(wdExpressionAST::NodeType::Select, pExpression, pSecondOperand, pThirdOperand);
    }
    else
    {
      pExpression = m_pAST->CreateBinaryOperator(binaryOp, pExpression, pSecondOperand);
    }
  }

  return pExpression;
}

wdExpressionAST::Node* wdExpressionParser::ParseUnaryExpression()
{
  while (Accept(m_TokenStream, m_uiCurrentToken, "+"))
  {
  }

  if (Accept(m_TokenStream, m_uiCurrentToken, "-"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(wdExpressionAST::NodeType::Negate, pOperand);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, "~"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(wdExpressionAST::NodeType::BitwiseNot, pOperand);
  }
  else if (Accept(m_TokenStream, m_uiCurrentToken, "!"))
  {
    auto pOperand = ParseUnaryExpression();
    if (pOperand == nullptr)
      return nullptr;

    return m_pAST->CreateUnaryOperator(wdExpressionAST::NodeType::LogicalNot, pOperand);
  }

  return ParseFactor();
}

wdExpressionAST::Node* wdExpressionParser::ParseFunctionCall(wdStringView sFunctionName)
{
  // "(" of the function call
  const wdToken* pFunctionToken = m_TokenStream[m_uiCurrentToken - 1];

  wdSmallArray<wdExpressionAST::Node*, 8> arguments;
  if (Accept(m_TokenStream, m_uiCurrentToken, ")") == false)
  {
    do
    {
      arguments.PushBack(ParseExpression());
    } while (Accept(m_TokenStream, m_uiCurrentToken, ","));

    if (Expect(")").Failed())
      return nullptr;
  }

  auto CheckArgumentCount = [&](wdUInt32 uiExpectedArgumentCount) -> wdResult {
    if (arguments.GetCount() != uiExpectedArgumentCount)
    {
      ReportError(pFunctionToken, wdFmt("Invalid argument count for '{}'. Expected {} but got {}", sFunctionName, uiExpectedArgumentCount, arguments.GetCount()));
      return WD_FAILURE;
    }
    return WD_SUCCESS;
  };

  wdHashedString sHashedFuncName;
  sHashedFuncName.Assign(sFunctionName);

  wdEnum<wdExpressionAST::DataType> dataType;
  if (m_KnownTypes.TryGetValue(sHashedFuncName, dataType))
  {
    wdUInt32 uiElementCount = wdExpressionAST::DataType::GetElementCount(dataType);
    if (arguments.GetCount() > uiElementCount)
    {
      ReportError(pFunctionToken, wdFmt("Invalid argument count for '{}'. Expected 0 - {} but got {}", sFunctionName, uiElementCount, arguments.GetCount()));
      return nullptr;
    }

    return m_pAST->CreateConstructorCall(dataType, arguments);
  }

  wdEnum<wdExpressionAST::NodeType> builtinType;
  if (m_BuiltinFunctions.TryGetValue(sHashedFuncName, builtinType))
  {
    if (wdExpressionAST::NodeType::IsUnary(builtinType))
    {
      if (CheckArgumentCount(1).Failed())
        return nullptr;

      return m_pAST->CreateUnaryOperator(builtinType, arguments[0]);
    }
    else if (wdExpressionAST::NodeType::IsBinary(builtinType))
    {
      if (CheckArgumentCount(2).Failed())
        return nullptr;

      return m_pAST->CreateBinaryOperator(builtinType, arguments[0], arguments[1]);
    }
    else if (wdExpressionAST::NodeType::IsTernary(builtinType))
    {
      if (CheckArgumentCount(3).Failed())
        return nullptr;

      return m_pAST->CreateTernaryOperator(builtinType, arguments[0], arguments[1], arguments[2]);
    }

    WD_ASSERT_NOT_IMPLEMENTED;
    return nullptr;
  }

  // external function
  const wdHybridArray<wdExpression::FunctionDesc, 1>* pFunctionDescs = nullptr;
  if (m_FunctionDescs.TryGetValue(sHashedFuncName, pFunctionDescs))
  {
    wdUInt32 uiMinArgumentCount = wdInvalidIndex;
    for (auto& funcDesc : *pFunctionDescs)
    {
      uiMinArgumentCount = wdMath::Min<wdUInt32>(uiMinArgumentCount, funcDesc.m_uiNumRequiredInputs);
    }

    if (arguments.GetCount() < uiMinArgumentCount)
    {
      ReportError(pFunctionToken, wdFmt("Invalid argument count for '{}'. Expected at least {} but got {}", sFunctionName, uiMinArgumentCount, arguments.GetCount()));
      return nullptr;
    }

    return m_pAST->CreateFunctionCall(*pFunctionDescs, arguments);
  }

  ReportError(pFunctionToken, wdFmt("Undeclared function '{}'", sFunctionName));
  return nullptr;
}

wdExpressionAST::Node* wdExpressionParser::ParseSwizzle(wdExpressionAST::Node* pExpression)
{
  if (Accept(m_TokenStream, m_uiCurrentToken, "."))
  {
    const wdToken* pSwizzleToken = nullptr;
    if (Expect(wdTokenType::Identifier, &pSwizzleToken).Failed())
      return nullptr;

    pExpression = m_pAST->CreateSwizzle(pSwizzleToken->m_DataView, pExpression);
    if (pExpression == nullptr)
    {
      ReportError(pSwizzleToken, wdFmt("Invalid swizzle '{}'", pSwizzleToken->m_DataView));
    }
  }

  return pExpression;
}

// Does NOT advance the current token beyond the operator!
bool wdExpressionParser::AcceptOperator(wdStringView sName)
{
  const wdUInt32 uiOperatorLength = sName.GetElementCount();

  if (m_uiCurrentToken + uiOperatorLength - 1 >= m_TokenStream.GetCount())
    return false;

  for (wdUInt32 charIndex = 0; charIndex < uiOperatorLength; ++charIndex)
  {
    if (m_TokenStream[m_uiCurrentToken + charIndex]->m_DataView.GetCharacter() != sName.GetStartPointer()[charIndex])
    {
      return false;
    }
  }

  return true;
}

// Does NOT advance the current token beyond the binary operator!
bool wdExpressionParser::AcceptBinaryOperator(wdExpressionAST::NodeType::Enum& out_binaryOp, int& out_iOperatorPrecedence, wdUInt32& out_uiOperatorLength)
{
  SkipWhitespace(m_TokenStream, m_uiCurrentToken);

  for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(s_binaryOperators); ++i)
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

wdExpressionAST::Node* wdExpressionParser::GetVariable(wdStringView sVarName)
{
  wdHashedString sHashedVarName;
  sHashedVarName.Assign(sVarName);

  wdExpressionAST::Node* pVariableNode;
  if (m_KnownVariables.TryGetValue(sHashedVarName, pVariableNode) == false && m_Options.m_bTreatUnknownVariablesAsInputs)
  {
    pVariableNode = m_pAST->CreateInput({sHashedVarName, wdProcessingStream::DataType::Float});
    m_KnownVariables.Insert(sHashedVarName, pVariableNode);
  }

  return pVariableNode;
}

wdExpressionAST::Node* wdExpressionParser::EnsureExpectedType(wdExpressionAST::Node* pNode, wdExpressionAST::DataType::Enum expectedType)
{
  if (expectedType != wdExpressionAST::DataType::Unknown)
  {
    const auto nodeRegisterType = wdExpressionAST::DataType::GetRegisterType(pNode->m_ReturnType);
    const auto expectedRegisterType = wdExpressionAST::DataType::GetRegisterType(expectedType);
    if (nodeRegisterType != expectedRegisterType)
    {
      pNode = m_pAST->CreateUnaryOperator(wdExpressionAST::NodeType::TypeConversion, pNode, expectedType);
    }

    const wdUInt32 nodeElementCount = wdExpressionAST::DataType::GetElementCount(pNode->m_ReturnType);
    const wdUInt32 expectedElementCount = wdExpressionAST::DataType::GetElementCount(expectedType);
    if (nodeElementCount < expectedElementCount)
    {
      pNode = m_pAST->CreateConstructorCall(expectedType, wdMakeArrayPtr(&pNode, 1));
    }
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionParser::Unpack(wdExpressionAST::Node* pNode, bool bUnassignedError /*= true*/)
{
  if (wdExpressionAST::NodeType::IsOutput(pNode->m_Type))
  {
    auto pOutput = static_cast<wdExpressionAST::Output*>(pNode);
    if (pOutput->m_pExpression == nullptr && bUnassignedError)
    {
      ReportError(m_TokenStream[m_uiCurrentToken], wdFmt("Output '{}' has not been assigned yet", pOutput->m_Desc.m_sName));
    }

    return pOutput->m_pExpression;
  }

  return pNode;
}

wdResult wdExpressionParser::CheckOutputs()
{
  for (auto pOutputNode : m_pAST->m_OutputNodes)
  {
    if (pOutputNode->m_pExpression == nullptr)
    {
      wdLog::Error("Output '{}' was never written", pOutputNode->m_Desc.m_sName);
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionParser);
