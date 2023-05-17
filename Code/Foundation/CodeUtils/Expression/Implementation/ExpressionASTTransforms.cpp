#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Logging/Log.h>

namespace
{
  struct OperandChainIndices
  {
    wdUInt8 m_uiLeftOperand;
    wdUInt8 m_uiRightOperand;
  };

  struct MultiplicationChain
  {
    OperandChainIndices m_Chain[8];
  };

  static MultiplicationChain s_MultiplicationChains[] = {
    {OperandChainIndices{}},                                     // 0
    {OperandChainIndices{}},                                     // 1
    {OperandChainIndices{0, 0}},                                 // 2
    {OperandChainIndices{0, 0}, {1, 0}},                         // 3
    {OperandChainIndices{0, 0}, {1, 1}},                         // 4
    {OperandChainIndices{0, 0}, {1, 1}, {2, 0}},                 // 5
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}},                 // 6
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}, {3, 0}},         // 7
    {OperandChainIndices{0, 0}, {1, 1}, {2, 2}},                 // 8
    {OperandChainIndices{0, 0}, {1, 1}, {2, 2}, {3, 0}},         // 9
    {OperandChainIndices{0, 0}, {1, 1}, {2, 0}, {3, 3}},         // 10
    {OperandChainIndices{0, 0}, {1, 1}, {2, 0}, {3, 3}, {4, 0}}, // 11
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}, {3, 3}},         // 12
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}, {3, 3}, {4, 0}}, // 13
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}, {3, 0}, {4, 4}}, // 14
    {OperandChainIndices{0, 0}, {1, 0}, {2, 2}, {3, 3}, {4, 2}}, // 15
    {OperandChainIndices{0, 0}, {1, 1}, {2, 2}, {3, 3}},         // 16
  };

  static wdExpression::StreamDesc CreateScalarizedStreamDesc(const wdExpression::StreamDesc& desc, wdEnum<wdExpressionAST::VectorComponent> component)
  {
    wdStringBuilder sNewName = desc.m_sName.GetView();
    sNewName.Append(".", wdExpressionAST::VectorComponent::GetName(component));

    wdExpression::StreamDesc newDesc;
    newDesc.m_sName.Assign(sNewName);
    newDesc.m_DataType = static_cast<wdProcessingStream::DataType>((wdUInt32)desc.m_DataType & ~3u);

    return newDesc;
  }

} // namespace

wdExpressionAST::Node* wdExpressionAST::TypeDeductionAndConversion(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;
  const DataType::Enum returnType = pNode->m_ReturnType;

  if (returnType == DataType::Unknown)
  {
    wdLog::Error("No matching overload found for '{}'", NodeType::GetName(nodeType));
    return nullptr;
  }

  auto children = GetChildren(pNode);
  for (wdUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    if (pChildNode == nullptr)
    {
      return nullptr;
    }

    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);

    if (expectedChildDataType != DataType::Unknown && pChildNode->m_ReturnType != expectedChildDataType)
    {
      const auto childRegisterType = DataType::GetRegisterType(pChildNode->m_ReturnType);
      const wdUInt32 childElementCount = DataType::GetElementCount(pChildNode->m_ReturnType);
      const auto expectedRegisterType = DataType::GetRegisterType(expectedChildDataType);
      const wdUInt32 expectedElementCount = DataType::GetElementCount(expectedChildDataType);

      if (childRegisterType != expectedRegisterType)
      {
        pChildNode = CreateUnaryOperator(NodeType::TypeConversion, pChildNode, DataType::FromRegisterType(expectedRegisterType, childElementCount));
      }

      if (childElementCount == 1 && expectedElementCount > 1)
      {
        pChildNode = CreateConstructorCall(expectedChildDataType, wdMakeArrayPtr(&pChildNode, 1));
      }
      else if (childElementCount < expectedElementCount)
      {
        wdLog::Error("Cannot implicitly convert '{}' to '{}'", DataType::GetName(pChildNode->m_ReturnType), DataType::GetName(expectedChildDataType));
        return nullptr;
      }
    }
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::ReplaceVectorInstructions(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;
  const DataType::Enum returnType = pNode->m_ReturnType;
  const wdUInt32 uiNumInputElements = pNode->m_uiNumInputElements;

  if (nodeType == NodeType::Length)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pDot = ReplaceVectorInstructions(CreateBinaryOperator(NodeType::Dot, pUnaryNode->m_pOperand, pUnaryNode->m_pOperand));
    return CreateUnaryOperator(NodeType::Sqrt, pDot);
  }
  else if (nodeType == NodeType::Normalize)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pLength = ReplaceVectorInstructions(CreateUnaryOperator(NodeType::Length, pUnaryNode->m_pOperand));
    return CreateBinaryOperator(NodeType::Divide, pUnaryNode->m_pOperand, CreateConstructorCall(returnType, wdMakeArrayPtr(&pLength, 1)));
  }
  else if (nodeType == NodeType::All || nodeType == NodeType::Any)
  {
    if (uiNumInputElements == 1)
      return pNode;

    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pX = CreateSwizzle(VectorComponent::X, pUnaryNode->m_pOperand);
    Node* pResult = pX;

    for (wdUInt32 i = 1; i < uiNumInputElements; ++i)
    {
      auto pI = CreateSwizzle(static_cast<VectorComponent::Enum>(i), pUnaryNode->m_pOperand);
      pResult = CreateBinaryOperator(nodeType == NodeType::All ? NodeType::LogicalAnd : NodeType::LogicalOr, pResult, pI);
    }

    return pResult;
  }
  else if (nodeType == NodeType::Dot)
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pAx = CreateSwizzle(VectorComponent::X, pBinaryNode->m_pLeftOperand);
    auto pBx = CreateSwizzle(VectorComponent::X, pBinaryNode->m_pRightOperand);
    auto pResult = CreateBinaryOperator(NodeType::Multiply, pAx, pBx);

    for (wdUInt32 i = 1; i < uiNumInputElements; ++i)
    {
      auto pAi = CreateSwizzle(static_cast<VectorComponent::Enum>(i), pBinaryNode->m_pLeftOperand);
      auto pBi = CreateSwizzle(static_cast<VectorComponent::Enum>(i), pBinaryNode->m_pRightOperand);
      pResult = CreateBinaryOperator(NodeType::Add, pResult, CreateBinaryOperator(NodeType::Multiply, pAi, pBi));
    }

    return pResult;
  }
  else if (nodeType == NodeType::Cross)
  {
    if (uiNumInputElements != 3)
    {
      wdLog::Error("Cross product is only defined for vec3");
      return nullptr;
    }

    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pA = pBinaryNode->m_pLeftOperand;
    auto pB = pBinaryNode->m_pRightOperand;

    // a.yzx * b.zxy - a.zxy * b.yzx
    wdEnum<VectorComponent> yzx[] = {VectorComponent::Y, VectorComponent::Z, VectorComponent::X};
    wdEnum<VectorComponent> zxy[] = {VectorComponent::Z, VectorComponent::X, VectorComponent::Y};
    auto pMul0 = CreateBinaryOperator(NodeType::Multiply, CreateSwizzle(wdMakeArrayPtr(yzx), pA), CreateSwizzle(wdMakeArrayPtr(zxy), pB));
    auto pMul1 = CreateBinaryOperator(NodeType::Multiply, CreateSwizzle(wdMakeArrayPtr(zxy), pA), CreateSwizzle(wdMakeArrayPtr(yzx), pB));
    return CreateBinaryOperator(NodeType::Subtract, pMul0, pMul1);
  }
  else if (nodeType == NodeType::Reflect)
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pA = pBinaryNode->m_pLeftOperand;
    auto pN = pBinaryNode->m_pRightOperand;

    // a - n * 2 * dot(a, n)
    auto pDot = ReplaceVectorInstructions(CreateBinaryOperator(NodeType::Dot, pA, pN));
    auto pTwo = CreateConstant(2, DataType::FromRegisterType(DataType::GetRegisterType(returnType)));
    Node* pMul = CreateBinaryOperator(NodeType::Multiply, pDot, pTwo);
    pMul = CreateBinaryOperator(NodeType::Multiply, pN, CreateConstructorCall(returnType, wdMakeArrayPtr(&pMul, 1)));
    return CreateBinaryOperator(NodeType::Subtract, pA, pMul);
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::ScalarizeVectorInstructions(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;

  if (nodeType == NodeType::Swizzle)
  {
    auto pSwizzleNode = static_cast<Swizzle*>(pNode);
    if (pSwizzleNode->m_NumComponents == 1)
    {
      wdEnum<VectorComponent> component = pSwizzleNode->m_Components[0];
      Node* pChildNode = pSwizzleNode->m_pExpression;
      NodeType::Enum childNodeType = pChildNode->m_Type;
      DataType::Enum childReturnTypeSingleElement = DataType::FromRegisterType(DataType::GetRegisterType(pChildNode->m_ReturnType));

      if (NodeType::IsConstant(childNodeType))
      {
        auto pConstantNode = static_cast<const Constant*>(pChildNode);
        const wdUInt32 uiNumConstantElements = DataType::GetElementCount(pConstantNode->m_ReturnType);
        if (static_cast<wdUInt32>(component) >= uiNumConstantElements)
        {
          wdLog::Error("Invalid subscript .{} for constant of type '{}'", VectorComponent::GetName(component), DataType::GetName(pConstantNode->m_ReturnType));
          return nullptr;
        }

        wdVariant newValue = pConstantNode->m_Value[component];
        return CreateConstant(newValue, childReturnTypeSingleElement);
      }
      else if (NodeType::IsSwizzle(childNodeType))
      {
        auto pChildSwizzleNode = static_cast<Swizzle*>(pChildNode);
        if (static_cast<wdUInt32>(component) >= pChildSwizzleNode->m_NumComponents)
        {
          wdLog::Error("Invalid Swizzle");
          return nullptr;
        }
        return ScalarizeVectorInstructions(CreateSwizzle(pChildSwizzleNode->m_Components[component], pChildSwizzleNode->m_pExpression));
      }
      else if (NodeType::IsInput(childNodeType))
      {
        auto pInput = static_cast<const Input*>(pChildNode);
        const wdUInt32 uiNumInputElements = DataType::GetElementCount(pInput->m_ReturnType);
        if (static_cast<wdUInt32>(component) >= uiNumInputElements)
        {
          wdLog::Error("Invalid subscript .{} for input '{}' of type '{}'", VectorComponent::GetName(component), pInput->m_Desc.m_sName, DataType::GetName(pInput->m_ReturnType));
          return nullptr;
        }
        return CreateInput(CreateScalarizedStreamDesc(pInput->m_Desc, component));
      }
      else if (NodeType::IsConstructorCall(childNodeType))
      {
        auto pConstructorCall = static_cast<const ConstructorCall*>(pChildNode);
        auto pArg = pConstructorCall->m_Arguments[component];
        return ScalarizeVectorInstructions(pArg);
      }

      auto innerChildren = GetChildren(pChildNode);
      wdSmallArray<Node*, 8> newSwizzleNodes;
      for (auto pInnerChildNode : innerChildren)
      {
        newSwizzleNodes.PushBack(CreateSwizzle(component, pInnerChildNode));
      }

      if (NodeType::IsUnary(childNodeType))
      {
        return CreateUnaryOperator(childNodeType, newSwizzleNodes[0], childReturnTypeSingleElement);
      }
      else if (NodeType::IsBinary(childNodeType))
      {
        return CreateBinaryOperator(childNodeType, newSwizzleNodes[0], newSwizzleNodes[1]);
      }
      else if (NodeType::IsTernary(childNodeType))
      {
        return CreateTernaryOperator(childNodeType, newSwizzleNodes[0], newSwizzleNodes[1], newSwizzleNodes[2]);
      }
      else if (NodeType::IsFunctionCall(childNodeType))
      {
        auto pFunctionCall = static_cast<const FunctionCall*>(pChildNode);
        return CreateFunctionCall(*pFunctionCall->m_Descs[pFunctionCall->m_uiOverloadIndex], std::move(newSwizzleNodes));
      }

      WD_ASSERT_NOT_IMPLEMENTED;
    }
    else
    {
      wdLog::Error("Failed to scalarize AST");
      return nullptr;
    }
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::ReplaceUnsupportedInstructions(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;
  const DataType::Enum returnType = pNode->m_ReturnType;

  if (nodeType == NodeType::Negate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pZero = CreateConstant(0, returnType);
    auto pValue = pUnaryNode->m_pOperand;
    return CreateBinaryOperator(NodeType::Subtract, pZero, pValue);
  }
  else if (nodeType == NodeType::Saturate)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pZero = CreateConstant(0, returnType);
    auto pOne = CreateConstant(1, returnType);
    auto pValue = pUnaryNode->m_pOperand;
    return CreateBinaryOperator(NodeType::Max, pZero, CreateBinaryOperator(NodeType::Min, pOne, pValue));
  }
  else if (nodeType == NodeType::Pow2)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (returnType == DataType::Int)
    {
      auto pOne = CreateConstant(1, returnType);
      auto pValue = pUnaryNode->m_pOperand;
      return CreateBinaryOperator(NodeType::BitshiftLeft, pOne, pValue);
    }
  }
  else if (nodeType == NodeType::RadToDeg)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pValue = pUnaryNode->m_pOperand;
    auto pMultiplier = CreateConstant(wdAngle::RadToDegMultiplier<float>(), returnType);
    return CreateBinaryOperator(NodeType::Multiply, pValue, pMultiplier);
  }
  else if (nodeType == NodeType::DegToRad)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pValue = pUnaryNode->m_pOperand;
    auto pMultiplier = CreateConstant(wdAngle::DegToRadMultiplier<float>(), returnType);
    return CreateBinaryOperator(NodeType::Multiply, pValue, pMultiplier);
  }
  else if (nodeType == NodeType::Frac)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pValue = pUnaryNode->m_pOperand;
    return CreateBinaryOperator(NodeType::Subtract, pValue, CreateUnaryOperator(NodeType::Trunc, pValue));
  }
  else if (nodeType == NodeType::Modulo)
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pLeftValue = pBinaryNode->m_pLeftOperand;
    auto pRightValue = pBinaryNode->m_pRightOperand;
    Node* pQuotient = CreateBinaryOperator(NodeType::Divide, pLeftValue, pRightValue);
    if (returnType == DataType::Float)
    {
      pQuotient = CreateUnaryOperator(NodeType::Trunc, pQuotient);
    }
    return CreateBinaryOperator(NodeType::Subtract, pLeftValue, CreateBinaryOperator(NodeType::Multiply, pRightValue, pQuotient));
  }
  else if (nodeType == NodeType::Log)
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pBase = pBinaryNode->m_pLeftOperand;
    auto pValue = pBinaryNode->m_pRightOperand;
    auto pLog2Value = CreateUnaryOperator(NodeType::Log2, pValue);
    if (NodeType::IsConstant(pBase->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pBase);
      const float fBaseValue = pConstantNode->m_Value.Get<float>();
      if (fBaseValue == 2.0f)
      {
        return pLog2Value;
      }
      else
      {
        auto pFactor = CreateConstant(1.0f / wdMath::Log2(fBaseValue));
        return CreateBinaryOperator(NodeType::Multiply, pLog2Value, pFactor);
      }
    }
    else
    {
      return CreateBinaryOperator(NodeType::Divide, pLog2Value, CreateUnaryOperator(NodeType::Log2, pBase));
    }
  }
  else if (nodeType == NodeType::Pow)
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    auto pBase = pBinaryNode->m_pLeftOperand;
    auto pExp = pBinaryNode->m_pRightOperand;
    if (NodeType::IsConstant(pBase->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pBase);
      const double fBaseValue = pConstantNode->m_Value.ConvertTo<double>();
      if (fBaseValue == 2.0f)
      {
        auto pPow2 = CreateUnaryOperator(NodeType::Pow2, pExp);
        return ReplaceUnsupportedInstructions(pPow2);
      }
    }

    if (NodeType::IsConstant(pExp->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pExp);
      const double fExpValue = pConstantNode->m_Value.ConvertTo<double>();
      if (fExpValue == 1.0)
      {
        return pBase;
      }

      const bool isWholeNumber = fExpValue == wdMath::Trunc(fExpValue);
      if (isWholeNumber && fExpValue > 1 && fExpValue < WD_ARRAY_SIZE(s_MultiplicationChains))
      {
        wdHybridArray<Node*, 8> multiplierStack;
        multiplierStack.PushBack(pBase);

        const auto& chain = s_MultiplicationChains[(wdUInt32)fExpValue].m_Chain;
        wdUInt32 uiIndex = 0;
        do
        {
          auto& operandIndices = chain[uiIndex];
          auto pLeft = multiplierStack[operandIndices.m_uiLeftOperand];
          auto pRight = multiplierStack[operandIndices.m_uiRightOperand];
          auto pMultiply = CreateBinaryOperator(NodeType::Multiply, pLeft, pRight);
          multiplierStack.PushBack(pMultiply);

          ++uiIndex;
        } while (chain[uiIndex].m_uiLeftOperand != 0 || chain[uiIndex].m_uiRightOperand != 0);

        return multiplierStack.PeekBack();
      }
    }

    // reformulate x^y as 2 ^ (y * log2(x)). Only works with floating point math.
    if (pBase->m_ReturnType == DataType::Int)
    {
      pBase = CreateUnaryOperator(NodeType::TypeConversion, pBase, DataType::Float);
    }

    if (pExp->m_ReturnType == DataType::Int)
    {
      pExp = CreateUnaryOperator(NodeType::TypeConversion, pExp, DataType::Float);
    }

    auto pLog2Base = CreateUnaryOperator(NodeType::Log2, pBase);
    auto pPow2 = CreateUnaryOperator(NodeType::Pow2, CreateBinaryOperator(NodeType::Multiply, pExp, pLog2Base));
    if (returnType == DataType::Int)
    {
      return CreateUnaryOperator(NodeType::TypeConversion, CreateUnaryOperator(NodeType::Round, pPow2), DataType::Int);
    }
    return pPow2;
  }
  else if (nodeType == NodeType::Clamp)
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    auto pValue = pTernaryNode->m_pFirstOperand;
    auto pMinValue = pTernaryNode->m_pSecondOperand;
    auto pMaxValue = pTernaryNode->m_pThirdOperand;
    return CreateBinaryOperator(NodeType::Max, pMinValue, CreateBinaryOperator(NodeType::Min, pMaxValue, pValue));
  }
  else if (nodeType == NodeType::Lerp)
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    auto pAValue = pTernaryNode->m_pFirstOperand;
    auto pBValue = pTernaryNode->m_pSecondOperand;
    auto pSValue = pTernaryNode->m_pThirdOperand;
    auto pBMinusA = CreateBinaryOperator(NodeType::Subtract, pBValue, pAValue);
    return CreateBinaryOperator(NodeType::Add, pAValue, CreateBinaryOperator(NodeType::Multiply, pSValue, pBMinusA));
  }
  else if (nodeType == NodeType::TypeConversion)
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    auto pValue = pUnaryNode->m_pOperand;
    if (returnType == DataType::Bool)
    {
      auto pZero = CreateConstant(0, pValue->m_ReturnType);
      return CreateBinaryOperator(NodeType::NotEqual, pValue, pZero);
    }
    else if (pValue->m_ReturnType == DataType::Bool)
    {
      auto pOne = CreateConstant(1, returnType);
      auto pZero = CreateConstant(0, returnType);
      return CreateTernaryOperator(NodeType::Select, pValue, pOne, pZero);
    }
  }
  else if (nodeType == NodeType::ConstructorCall)
  {
    auto pConstructorCallNode = static_cast<const ConstructorCall*>(pNode);
    if (pConstructorCallNode->m_Arguments.GetCount() > 1)
    {
      wdLog::Error("Constructor of type '{}' has too many arguments", DataType::GetName(returnType));
      return nullptr;
    }

    return pConstructorCallNode->m_Arguments[0];
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::FoldConstants(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;
  const DataType::Enum returnType = pNode->m_ReturnType;

  if (NodeType::IsUnary(nodeType))
  {
    auto pUnaryNode = static_cast<const UnaryOperator*>(pNode);
    if (NodeType::IsConstant(pUnaryNode->m_pOperand->m_Type))
    {
      auto pConstantNode = static_cast<Constant*>(pUnaryNode->m_pOperand);
      if (nodeType == NodeType::TypeConversion)
      {
        return CreateConstant(pConstantNode->m_Value, returnType);
      }

      if (returnType == DataType::Bool)
      {
        const bool bValue = pConstantNode->m_Value.Get<bool>();

        switch (nodeType)
        {
          case NodeType::LogicalNot:
            return CreateConstant(!bValue, returnType);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (returnType == DataType::Int)
      {
        const int iValue = pConstantNode->m_Value.Get<int>();

        switch (nodeType)
        {
          case NodeType::Negate:
            return CreateConstant(-iValue, returnType);
          case NodeType::Absolute:
            return CreateConstant(wdMath::Abs(iValue), returnType);
          case NodeType::Saturate:
            return CreateConstant(wdMath::Saturate(iValue), returnType);
          case NodeType::Log2:
            return CreateConstant(wdMath::Log2i(iValue), returnType);
          case NodeType::Pow2:
            return CreateConstant(wdMath::Pow2(iValue), returnType);
          case NodeType::BitwiseNot:
            return CreateConstant(~iValue, returnType);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (returnType == DataType::Float)
      {
        const float fValue = pConstantNode->m_Value.Get<float>();

        switch (nodeType)
        {
          case NodeType::Negate:
            return CreateConstant(-fValue);
          case NodeType::Absolute:
            return CreateConstant(wdMath::Abs(fValue));
          case NodeType::Saturate:
            return CreateConstant(wdMath::Saturate(fValue));
          case NodeType::Sqrt:
            return CreateConstant(wdMath::Sqrt(fValue));
          case NodeType::Exp:
            return CreateConstant(wdMath::Exp(fValue));
          case NodeType::Ln:
            return CreateConstant(wdMath::Ln(fValue));
          case NodeType::Log2:
            return CreateConstant(wdMath::Log2(fValue));
          case NodeType::Log10:
            return CreateConstant(wdMath::Log10(fValue));
          case NodeType::Pow2:
            return CreateConstant(wdMath::Pow2(fValue));
          case NodeType::Sin:
            return CreateConstant(wdMath::Sin(wdAngle::Radian(fValue)));
          case NodeType::Cos:
            return CreateConstant(wdMath::Cos(wdAngle::Radian(fValue)));
          case NodeType::Tan:
            return CreateConstant(wdMath::Tan(wdAngle::Radian(fValue)));
          case NodeType::ASin:
            return CreateConstant(wdMath::ASin(fValue).GetRadian());
          case NodeType::ACos:
            return CreateConstant(wdMath::ACos(fValue).GetRadian());
          case NodeType::ATan:
            return CreateConstant(wdMath::ATan(fValue).GetRadian());
          case NodeType::RadToDeg:
            return CreateConstant(wdAngle::RadToDeg(fValue));
          case NodeType::DegToRad:
            return CreateConstant(wdAngle::DegToRad(fValue));
          case NodeType::Round:
            return CreateConstant(wdMath::Round(fValue));
          case NodeType::Floor:
            return CreateConstant(wdMath::Floor(fValue));
          case NodeType::Ceil:
            return CreateConstant(wdMath::Ceil(fValue));
          case NodeType::Trunc:
            return CreateConstant(wdMath::Trunc(fValue));
          case NodeType::Frac:
            return CreateConstant(wdMath::Fraction(fValue));
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
    }
  }
  else if (NodeType::IsBinary(nodeType))
  {
    auto pBinaryNode = static_cast<const BinaryOperator*>(pNode);
    const bool bLeftIsConstant = NodeType::IsConstant(pBinaryNode->m_pLeftOperand->m_Type);
    const bool bRightIsConstant = NodeType::IsConstant(pBinaryNode->m_pRightOperand->m_Type);
    if (bLeftIsConstant && bRightIsConstant)
    {
      auto pLeftConstant = static_cast<const Constant*>(pBinaryNode->m_pLeftOperand);
      auto pRightConstant = static_cast<const Constant*>(pBinaryNode->m_pRightOperand);
      DataType::Enum leftType = pLeftConstant->m_ReturnType;

      if (leftType == DataType::Bool)
      {
        const bool bLeftValue = pLeftConstant->m_Value.Get<bool>();
        const bool bRightValue = pRightConstant->m_Value.Get<bool>();

        switch (nodeType)
        {
          case NodeType::Equal:
            return CreateConstant(bLeftValue == bRightValue, returnType);
          case NodeType::NotEqual:
            return CreateConstant(bLeftValue != bRightValue, returnType);
          case NodeType::LogicalAnd:
            return CreateConstant(bLeftValue && bRightValue, returnType);
          case NodeType::LogicalOr:
            return CreateConstant(bLeftValue || bRightValue, returnType);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (leftType == DataType::Int)
      {
        const int iLeftValue = pLeftConstant->m_Value.Get<int>();
        const int iRightValue = pRightConstant->m_Value.Get<int>();

        switch (nodeType)
        {
          case NodeType::Add:
            return CreateConstant(iLeftValue + iRightValue, returnType);
          case NodeType::Subtract:
            return CreateConstant(iLeftValue - iRightValue, returnType);
          case NodeType::Multiply:
            return CreateConstant(iLeftValue * iRightValue, returnType);
          case NodeType::Divide:
            return CreateConstant(iLeftValue / iRightValue, returnType);
          case NodeType::Modulo:
            return CreateConstant(iLeftValue % iRightValue, returnType);
          case NodeType::Pow:
            return CreateConstant(wdMath::Pow(iLeftValue, iRightValue), returnType);
          case NodeType::Min:
            return CreateConstant(wdMath::Min(iLeftValue, iRightValue), returnType);
          case NodeType::Max:
            return CreateConstant(wdMath::Max(iLeftValue, iRightValue), returnType);
          case NodeType::BitshiftLeft:
            return CreateConstant(iLeftValue << iRightValue, returnType);
          case NodeType::BitshiftRight:
            return CreateConstant(iLeftValue >> iRightValue, returnType);
          case NodeType::BitwiseAnd:
            return CreateConstant(iLeftValue & iRightValue, returnType);
          case NodeType::BitwiseXor:
            return CreateConstant(iLeftValue ^ iRightValue, returnType);
          case NodeType::BitwiseOr:
            return CreateConstant(iLeftValue | iRightValue, returnType);
          case NodeType::Equal:
            return CreateConstant(iLeftValue == iRightValue, returnType);
          case NodeType::NotEqual:
            return CreateConstant(iLeftValue != iRightValue, returnType);
          case NodeType::Less:
            return CreateConstant(iLeftValue < iRightValue, returnType);
          case NodeType::LessEqual:
            return CreateConstant(iLeftValue <= iRightValue, returnType);
          case NodeType::Greater:
            return CreateConstant(iLeftValue > iRightValue, returnType);
          case NodeType::GreaterEqual:
            return CreateConstant(iLeftValue >= iRightValue, returnType);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (leftType == DataType::Float)
      {
        const float fLeftValue = pLeftConstant->m_Value.Get<float>();
        const float fRightValue = pRightConstant->m_Value.Get<float>();

        switch (nodeType)
        {
          case NodeType::Add:
            return CreateConstant(fLeftValue + fRightValue, returnType);
          case NodeType::Subtract:
            return CreateConstant(fLeftValue - fRightValue, returnType);
          case NodeType::Multiply:
            return CreateConstant(fLeftValue * fRightValue, returnType);
          case NodeType::Divide:
            return CreateConstant(fLeftValue / fRightValue, returnType);
          case NodeType::Modulo:
            return CreateConstant(wdMath::Mod(fLeftValue, fRightValue), returnType);
          case NodeType::Log:
            return CreateConstant(wdMath::Log(fLeftValue, fRightValue), returnType);
          case NodeType::Pow:
            return CreateConstant(wdMath::Pow(fLeftValue, fRightValue), returnType);
          case NodeType::Min:
            return CreateConstant(wdMath::Min(fLeftValue, fRightValue), returnType);
          case NodeType::Max:
            return CreateConstant(wdMath::Max(fLeftValue, fRightValue), returnType);
          case NodeType::Equal:
            return CreateConstant(fLeftValue == fRightValue, returnType);
          case NodeType::NotEqual:
            return CreateConstant(fLeftValue != fRightValue, returnType);
          case NodeType::Less:
            return CreateConstant(fLeftValue < fRightValue, returnType);
          case NodeType::LessEqual:
            return CreateConstant(fLeftValue <= fRightValue, returnType);
          case NodeType::Greater:
            return CreateConstant(fLeftValue > fRightValue, returnType);
          case NodeType::GreaterEqual:
            return CreateConstant(fLeftValue >= fRightValue, returnType);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
    }
    else if (bLeftIsConstant)
    {
      auto pOperand = pBinaryNode->m_pRightOperand;
      auto pConstant = static_cast<Constant*>(pBinaryNode->m_pLeftOperand);
      DataType::Enum leftType = pConstant->m_ReturnType;

      if (leftType == DataType::Bool)
      {
        const bool bValue = pConstant->m_Value.Get<bool>();

        switch (nodeType)
        {
          case NodeType::Equal:
            return CreateBinaryOperator(NodeType::Equal, pOperand, pConstant);
          case NodeType::NotEqual:
            return CreateBinaryOperator(NodeType::NotEqual, pOperand, pConstant);
          case NodeType::LogicalAnd:
            if (bValue == false)
            {
              return CreateConstant(false, returnType);
            }
            return CreateBinaryOperator(NodeType::LogicalAnd, pOperand, pConstant);
          case NodeType::LogicalOr:
            if (bValue == true)
            {
              return CreateConstant(true, returnType);
            }
            return CreateBinaryOperator(NodeType::LogicalOr, pOperand, pConstant);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
      else if (leftType == DataType::Int || leftType == DataType::Float)
      {
        const double fValue = pConstant->m_Value.ConvertTo<double>();

        switch (nodeType)
        {
          case NodeType::Add:
            if (fValue == 0.0f)
            {
              return pOperand;
            }
            return CreateBinaryOperator(NodeType::Add, pOperand, pConstant);
          case NodeType::Subtract:
            return pNode;
          case NodeType::Multiply:
            if (fValue == 0.0f)
            {
              return CreateConstant(0, returnType);
            }
            else if (fValue == 1.0f)
            {
              return pOperand;
            }
            return CreateBinaryOperator(NodeType::Multiply, pOperand, pConstant);
          case NodeType::Divide:
          case NodeType::Modulo:
          case NodeType::Log:
          case NodeType::Pow:
            return pNode;
          case NodeType::Min:
            return CreateBinaryOperator(NodeType::Min, pOperand, pConstant);
          case NodeType::Max:
            return CreateBinaryOperator(NodeType::Max, pOperand, pConstant);
          case NodeType::BitshiftLeft:
          case NodeType::BitshiftRight:
            return pNode;
          case NodeType::BitwiseAnd:
            return CreateBinaryOperator(NodeType::BitwiseAnd, pOperand, pConstant);
          case NodeType::BitwiseXor:
            return CreateBinaryOperator(NodeType::BitwiseXor, pOperand, pConstant);
          case NodeType::BitwiseOr:
            return CreateBinaryOperator(NodeType::BitwiseOr, pOperand, pConstant);
          case NodeType::Equal:
            return CreateBinaryOperator(NodeType::Equal, pOperand, pConstant);
          case NodeType::NotEqual:
            return CreateBinaryOperator(NodeType::NotEqual, pOperand, pConstant);
          case NodeType::Less:
            return CreateBinaryOperator(NodeType::Greater, pOperand, pConstant);
          case NodeType::LessEqual:
            return CreateBinaryOperator(NodeType::GreaterEqual, pOperand, pConstant);
          case NodeType::Greater:
            return CreateBinaryOperator(NodeType::Less, pOperand, pConstant);
          case NodeType::GreaterEqual:
            return CreateBinaryOperator(NodeType::LessEqual, pOperand, pConstant);
          default:
            WD_ASSERT_NOT_IMPLEMENTED;
            return pNode;
        }
      }
    }
    else if (bRightIsConstant)
    {
      auto pOperand = pBinaryNode->m_pLeftOperand;
      auto pConstant = static_cast<Constant*>(pBinaryNode->m_pRightOperand);
      DataType::Enum leftType = pOperand->m_ReturnType;

      if (leftType == DataType::Bool)
      {
        const bool bRightValue = pConstant->m_Value.Get<bool>();

        if (nodeType == NodeType::LogicalAnd && bRightValue == false)
        {
          return CreateConstant(false, returnType);
        }
        else if (nodeType == NodeType::LogicalOr && bRightValue == true)
        {
          return CreateConstant(true, returnType);
        }
      }
      else if (leftType == DataType::Int)
      {
        const int iRightValue = pConstant->m_Value.Get<int>();

        if ((nodeType == NodeType::Add || nodeType == NodeType::Subtract) && iRightValue == 0)
        {
          return pOperand;
        }
        else if (nodeType == NodeType::Multiply && iRightValue == 0)
        {
          return CreateConstant(0, returnType);
        }
        else if ((nodeType == NodeType::Multiply || nodeType == NodeType::Divide) && iRightValue == 1)
        {
          return pOperand;
        }
        else if (nodeType == NodeType::Divide && wdMath::IsPowerOf2(iRightValue))
        {
          auto pShiftValue = CreateConstant(wdMath::Log2i(iRightValue), returnType);
          auto pDivision = CreateBinaryOperator(NodeType::BitshiftRight, CreateUnaryOperator(NodeType::Absolute, pOperand), pShiftValue);
          auto pZero = CreateConstant(0, returnType);
          auto pGreaterZero = CreateBinaryOperator(NodeType::Greater, pOperand, pZero);
          return CreateTernaryOperator(NodeType::Select, pGreaterZero, pDivision, CreateBinaryOperator(NodeType::Subtract, pZero, pDivision));
        }
        else if (nodeType == NodeType::Pow && iRightValue == 1)
        {
          return pOperand;
        }
        else if ((nodeType == NodeType::BitshiftLeft || nodeType == NodeType::BitshiftRight) && iRightValue == 0)
        {
          return pOperand;
        }
      }
      else if (leftType == DataType::Float)
      {
        const float fRightValue = pConstant->m_Value.Get<float>();

        if ((nodeType == NodeType::Add || nodeType == NodeType::Subtract) && fRightValue == 0.0f)
        {
          return pOperand;
        }
        else if (nodeType == NodeType::Multiply && fRightValue == 0.0f)
        {
          return CreateConstant(0.0f, returnType);
        }
        else if ((nodeType == NodeType::Multiply || nodeType == NodeType::Divide) && fRightValue == 1.0f)
        {
          return pOperand;
        }
        else if (nodeType == NodeType::Divide)
        {
          auto pMulValue = CreateConstant(1.0f / fRightValue, returnType);
          return CreateBinaryOperator(NodeType::Multiply, pOperand, pMulValue);
        }
        else if (nodeType == NodeType::Pow && fRightValue == 1.0f)
        {
          return pOperand;
        }
      }
    }
  }
  else if (NodeType::IsTernary(nodeType))
  {
    auto pTernaryNode = static_cast<const TernaryOperator*>(pNode);
    if (nodeType == NodeType::Clamp || nodeType == NodeType::Lerp)
    {
      return pNode;
    }
    else if (nodeType == NodeType::Select)
    {
      if (NodeType::IsConstant(pTernaryNode->m_pFirstOperand->m_Type))
      {
        auto pConstantNode = static_cast<Constant*>(pTernaryNode->m_pFirstOperand);
        const bool bValue = pConstantNode->m_Value.Get<bool>();
        return bValue ? pTernaryNode->m_pSecondOperand : pTernaryNode->m_pThirdOperand;
      }

      return pNode;
    }

    WD_ASSERT_NOT_IMPLEMENTED;
    return pNode;
  }

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::CommonSubexpressionElimination(Node* pNode)
{
  UpdateHash(pNode);

  auto& nodesForHash = m_NodeDeduplicationTable[pNode->m_uiHash];
  for (auto pExistingNode : nodesForHash)
  {
    if (IsEqual(pNode, pExistingNode))
    {
      return pExistingNode;
    }
  }

  nodesForHash.PushBack(pNode);

  return pNode;
}

wdExpressionAST::Node* wdExpressionAST::Validate(Node* pNode)
{
  const NodeType::Enum nodeType = pNode->m_Type;

  if (pNode->m_ReturnType == DataType::Unknown)
  {
    wdLog::Error("Unresolved return type on '{}'", NodeType::GetName(nodeType));
    return nullptr;
  }

  if (NodeType::IsUnary(nodeType) || NodeType::IsBinary(nodeType) || NodeType::IsTernary(nodeType))
  {
    if (pNode->m_uiOverloadIndex == 0xFF)
    {
      wdLog::Error("Unresolved overload on '{}'", NodeType::GetName(nodeType));
      return nullptr;
    }
  }
  else if (NodeType::IsConstant(nodeType))
  {
    auto pConstantNode = static_cast<Constant*>(pNode);
    if (pConstantNode->m_Value.IsValid() == false)
    {
      wdLog::Error("Invalid constant value");
      return nullptr;
    }
  }
  else if (NodeType::IsFunctionCall(nodeType))
  {
    if (pNode->m_uiOverloadIndex == 0xFF)
    {
      wdLog::Error("Unresolved function overload on");
      return nullptr;
    }

    auto pFunctionCall = static_cast<FunctionCall*>(pNode);
    auto pDesc = pFunctionCall->m_Descs[pNode->m_uiOverloadIndex];
    if (pFunctionCall->m_Arguments.GetCount() < pDesc->m_uiNumRequiredInputs)
    {
      wdLog::Error("Not enough arguments for function '{}'", pDesc->m_sName);
      return nullptr;
    }
  }

  auto children = GetChildren(pNode);
  for (wdUInt32 i = 0; i < children.GetCount(); ++i)
  {
    auto& pChildNode = children[i];
    DataType::Enum expectedChildDataType = GetExpectedChildDataType(pNode, i);

    if (expectedChildDataType != DataType::Unknown && pChildNode->m_ReturnType != expectedChildDataType)
    {
      wdLog::Error("Invalid data type for argument {} on '{}'. Expected {} got {}", i, NodeType::GetName(nodeType), DataType::GetName(expectedChildDataType), DataType::GetName(pChildNode->m_ReturnType));
      return nullptr;
    }
  }

  return pNode;
}

wdResult wdExpressionAST::ScalarizeOutputs()
{
  for (wdUInt32 uiOutputIndex = 0; uiOutputIndex < m_OutputNodes.GetCount(); ++uiOutputIndex)
  {
    const auto pOutput = m_OutputNodes[uiOutputIndex];
    if (pOutput == nullptr || pOutput->m_pExpression == nullptr)
      return WD_FAILURE;

    const wdUInt32 uiNumElements = pOutput->m_uiNumInputElements;
    if (uiNumElements > 1)
    {
      m_OutputNodes.RemoveAtAndCopy(uiOutputIndex);

      for (wdUInt32 i = 0; i < uiNumElements; ++i)
      {
        wdEnum<VectorComponent> component = static_cast<VectorComponent::Enum>(i);
        auto pSwizzle = CreateSwizzle(component, pOutput->m_pExpression);
        auto pNewOutput = CreateOutput(CreateScalarizedStreamDesc(pOutput->m_Desc, component), pSwizzle);
        m_OutputNodes.Insert(pNewOutput, uiOutputIndex + i);
      }
    }
  }

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionASTTransforms);
