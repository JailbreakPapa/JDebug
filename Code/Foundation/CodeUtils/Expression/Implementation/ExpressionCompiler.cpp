#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/DGMLWriter.h>

namespace
{
#define ADD_OFFSET(opCode) static_cast<nsExpressionByteCode::OpCode::Enum>((opCode) + uiOffset)

  static nsExpressionByteCode::OpCode::Enum NodeTypeToOpCode(nsExpressionAST::NodeType::Enum nodeType, nsExpressionAST::DataType::Enum dataType, bool bRightIsConstant)
  {
    const nsExpression::RegisterType::Enum registerType = nsExpressionAST::DataType::GetRegisterType(dataType);
    const bool bFloat = registerType == nsExpression::RegisterType::Float;
    const bool bInt = registerType == nsExpression::RegisterType::Int;
    const nsUInt32 uiOffset = bRightIsConstant ? nsExpressionByteCode::OpCode::FirstBinaryWithConstant - nsExpressionByteCode::OpCode::FirstBinary : 0;

    switch (nodeType)
    {
      case nsExpressionAST::NodeType::Absolute:
        return bFloat ? nsExpressionByteCode::OpCode::AbsF_R : nsExpressionByteCode::OpCode::AbsI_R;
      case nsExpressionAST::NodeType::Sqrt:
        return nsExpressionByteCode::OpCode::SqrtF_R;

      case nsExpressionAST::NodeType::Exp:
        return nsExpressionByteCode::OpCode::ExpF_R;
      case nsExpressionAST::NodeType::Ln:
        return nsExpressionByteCode::OpCode::LnF_R;
      case nsExpressionAST::NodeType::Log2:
        return bFloat ? nsExpressionByteCode::OpCode::Log2F_R : nsExpressionByteCode::OpCode::Log2I_R;
      case nsExpressionAST::NodeType::Log10:
        return nsExpressionByteCode::OpCode::Log10F_R;
      case nsExpressionAST::NodeType::Pow2:
        return nsExpressionByteCode::OpCode::Pow2F_R;

      case nsExpressionAST::NodeType::Sin:
        return nsExpressionByteCode::OpCode::SinF_R;
      case nsExpressionAST::NodeType::Cos:
        return nsExpressionByteCode::OpCode::CosF_R;
      case nsExpressionAST::NodeType::Tan:
        return nsExpressionByteCode::OpCode::TanF_R;

      case nsExpressionAST::NodeType::ASin:
        return nsExpressionByteCode::OpCode::ASinF_R;
      case nsExpressionAST::NodeType::ACos:
        return nsExpressionByteCode::OpCode::ACosF_R;
      case nsExpressionAST::NodeType::ATan:
        return nsExpressionByteCode::OpCode::ATanF_R;

      case nsExpressionAST::NodeType::Round:
        return nsExpressionByteCode::OpCode::RoundF_R;
      case nsExpressionAST::NodeType::Floor:
        return nsExpressionByteCode::OpCode::FloorF_R;
      case nsExpressionAST::NodeType::Ceil:
        return nsExpressionByteCode::OpCode::CeilF_R;
      case nsExpressionAST::NodeType::Trunc:
        return nsExpressionByteCode::OpCode::TruncF_R;

      case nsExpressionAST::NodeType::BitwiseNot:
        return nsExpressionByteCode::OpCode::NotI_R;
      case nsExpressionAST::NodeType::LogicalNot:
        return nsExpressionByteCode::OpCode::NotB_R;

      case nsExpressionAST::NodeType::TypeConversion:
        return bFloat ? nsExpressionByteCode::OpCode::IToF_R : nsExpressionByteCode::OpCode::FToI_R;

      case nsExpressionAST::NodeType::Add:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::AddF_RR : nsExpressionByteCode::OpCode::AddI_RR);
      case nsExpressionAST::NodeType::Subtract:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::SubF_RR : nsExpressionByteCode::OpCode::SubI_RR);
      case nsExpressionAST::NodeType::Multiply:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::MulF_RR : nsExpressionByteCode::OpCode::MulI_RR);
      case nsExpressionAST::NodeType::Divide:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::DivF_RR : nsExpressionByteCode::OpCode::DivI_RR);
      case nsExpressionAST::NodeType::Min:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::MinF_RR : nsExpressionByteCode::OpCode::MinI_RR);
      case nsExpressionAST::NodeType::Max:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::MaxF_RR : nsExpressionByteCode::OpCode::MaxI_RR);

      case nsExpressionAST::NodeType::BitshiftLeft:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::ShlI_RR);
      case nsExpressionAST::NodeType::BitshiftRight:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::ShrI_RR);
      case nsExpressionAST::NodeType::BitwiseAnd:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::AndI_RR);
      case nsExpressionAST::NodeType::BitwiseXor:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::XorI_RR);
      case nsExpressionAST::NodeType::BitwiseOr:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::OrI_RR);

      case nsExpressionAST::NodeType::Equal:
        if (bFloat)
          return ADD_OFFSET(nsExpressionByteCode::OpCode::EqF_RR);
        else if (bInt)
          return ADD_OFFSET(nsExpressionByteCode::OpCode::EqI_RR);
        else
          return ADD_OFFSET(nsExpressionByteCode::OpCode::EqB_RR);
      case nsExpressionAST::NodeType::NotEqual:
        if (bFloat)
          return ADD_OFFSET(nsExpressionByteCode::OpCode::NEqF_RR);
        else if (bInt)
          return ADD_OFFSET(nsExpressionByteCode::OpCode::NEqI_RR);
        else
          return ADD_OFFSET(nsExpressionByteCode::OpCode::NEqB_RR);
      case nsExpressionAST::NodeType::Less:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::LtF_RR : nsExpressionByteCode::OpCode::LtI_RR);
      case nsExpressionAST::NodeType::LessEqual:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::LEqF_RR : nsExpressionByteCode::OpCode::LEqI_RR);
      case nsExpressionAST::NodeType::Greater:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::GtF_RR : nsExpressionByteCode::OpCode::GtI_RR);
      case nsExpressionAST::NodeType::GreaterEqual:
        return ADD_OFFSET(bFloat ? nsExpressionByteCode::OpCode::GEqF_RR : nsExpressionByteCode::OpCode::GEqI_RR);

      case nsExpressionAST::NodeType::LogicalAnd:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::AndB_RR);
      case nsExpressionAST::NodeType::LogicalOr:
        return ADD_OFFSET(nsExpressionByteCode::OpCode::OrB_RR);

      case nsExpressionAST::NodeType::Select:
        if (bFloat)
          return nsExpressionByteCode::OpCode::SelF_RRR;
        else if (bInt)
          return nsExpressionByteCode::OpCode::SelI_RRR;
        else
          return nsExpressionByteCode::OpCode::SelB_RRR;

      case nsExpressionAST::NodeType::Constant:
        return nsExpressionByteCode::OpCode::MovX_C;
      case nsExpressionAST::NodeType::Input:
        return bFloat ? nsExpressionByteCode::OpCode::LoadF : nsExpressionByteCode::OpCode::LoadI;
      case nsExpressionAST::NodeType::Output:
        return bFloat ? nsExpressionByteCode::OpCode::StoreF : nsExpressionByteCode::OpCode::StoreI;
      case nsExpressionAST::NodeType::FunctionCall:
        return nsExpressionByteCode::OpCode::Call;
      case nsExpressionAST::NodeType::ConstructorCall:
        NS_REPORT_FAILURE("Constructor calls should not exist anymore after AST transformations");
        return nsExpressionByteCode::OpCode::Nop;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        return nsExpressionByteCode::OpCode::Nop;
    }
  }

#undef ADD_OFFSET
} // namespace

nsExpressionCompiler::nsExpressionCompiler() = default;
nsExpressionCompiler::~nsExpressionCompiler() = default;

nsResult nsExpressionCompiler::Compile(nsExpressionAST& ref_ast, nsExpressionByteCode& out_byteCode, nsStringView sDebugAstOutputPath /*= nsStringView()*/)
{
  out_byteCode.Clear();

  NS_SUCCEED_OR_RETURN(TransformAndOptimizeAST(ref_ast, sDebugAstOutputPath));
  NS_SUCCEED_OR_RETURN(BuildNodeInstructions(ref_ast));
  NS_SUCCEED_OR_RETURN(UpdateRegisterLifetime(ref_ast));
  NS_SUCCEED_OR_RETURN(AssignRegisters());
  NS_SUCCEED_OR_RETURN(GenerateByteCode(ref_ast, out_byteCode));

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::TransformAndOptimizeAST(nsExpressionAST& ast, nsStringView sDebugAstOutputPath)
{
  DumpAST(ast, sDebugAstOutputPath, "_00");

  NS_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, nsMakeDelegate(&nsExpressionAST::TypeDeductionAndConversion, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_01_TypeConv");

  NS_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, nsMakeDelegate(&nsExpressionAST::ReplaceVectorInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_02_ReplacedVectorInst");

  NS_SUCCEED_OR_RETURN(ast.ScalarizeInputs());
  NS_SUCCEED_OR_RETURN(ast.ScalarizeOutputs());
  NS_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, nsMakeDelegate(&nsExpressionAST::ScalarizeVectorInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_03_Scalarized");

  NS_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, nsMakeDelegate(&nsExpressionAST::FoldConstants, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_04_ConstantFolded1");

  NS_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, nsMakeDelegate(&nsExpressionAST::ReplaceUnsupportedInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_05_ReplacedUnsupportedInst");

  NS_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, nsMakeDelegate(&nsExpressionAST::FoldConstants, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_06_ConstantFolded2");

  NS_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, nsMakeDelegate(&nsExpressionAST::CommonSubexpressionElimination, &ast)));
  NS_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, nsMakeDelegate(&nsExpressionAST::Validate, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_07_Optimized");

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::BuildNodeInstructions(const nsExpressionAST& ast)
{
  m_NodeStack.Clear();
  m_NodeInstructions.Clear();
  auto& nodeStackTemp = m_NodeInstructions;

  // Build node instruction order aka post order tree traversal
  for (nsExpressionAST::Node* pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return NS_FAILURE;

    NS_ASSERT_DEV(nodeStackTemp.IsEmpty(), "Implementation error");

    nodeStackTemp.PushBack(pOutputNode);

    while (!nodeStackTemp.IsEmpty())
    {
      auto pCurrentNode = nodeStackTemp.PeekBack();
      nodeStackTemp.PopBack();

      if (pCurrentNode == nullptr)
      {
        return NS_FAILURE;
      }

      m_NodeStack.PushBack(pCurrentNode);

      if (nsExpressionAST::NodeType::IsBinary(pCurrentNode->m_Type))
      {
        auto pBinary = static_cast<const nsExpressionAST::BinaryOperator*>(pCurrentNode);
        nodeStackTemp.PushBack(pBinary->m_pLeftOperand);

        // Do not push the right operand if it is a constant, we don't want a separate mov instruction for it
        // since all binary operators can take a constant as right operand in place.
        const bool bRightIsConstant = nsExpressionAST::NodeType::IsConstant(pBinary->m_pRightOperand->m_Type);
        if (!bRightIsConstant)
        {
          nodeStackTemp.PushBack(pBinary->m_pRightOperand);
        }
      }
      else
      {
        auto children = nsExpressionAST::GetChildren(pCurrentNode);
        for (auto pChild : children)
        {
          nodeStackTemp.PushBack(pChild);
        }
      }
    }
  }

  if (m_NodeStack.IsEmpty())
  {
    // Nothing to compile
    return NS_FAILURE;
  }

  NS_ASSERT_DEV(m_NodeInstructions.IsEmpty(), "Implementation error");

  m_NodeToRegisterIndex.Clear();
  m_LiveIntervals.Clear();
  nsUInt32 uiNextRegisterIndex = 0;

  // De-duplicate nodes, build final instruction list and assign virtual register indices. Also determine their lifetime start.
  while (!m_NodeStack.IsEmpty())
  {
    auto pCurrentNode = m_NodeStack.PeekBack();
    m_NodeStack.PopBack();

    if (!m_NodeToRegisterIndex.Contains(pCurrentNode))
    {
      m_NodeInstructions.PushBack(pCurrentNode);

      if (nsExpressionAST::NodeType::IsOutput(pCurrentNode->m_Type))
        continue;

      m_NodeToRegisterIndex.Insert(pCurrentNode, uiNextRegisterIndex);
      ++uiNextRegisterIndex;

      nsUInt32 uiCurrentInstructionIndex = m_NodeInstructions.GetCount() - 1;
      m_LiveIntervals.PushBack({uiCurrentInstructionIndex, uiCurrentInstructionIndex, pCurrentNode});
      NS_ASSERT_DEV(m_LiveIntervals.GetCount() == uiNextRegisterIndex, "Implementation error");
    }
  }

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::UpdateRegisterLifetime(const nsExpressionAST& ast)
{
  nsUInt32 uiNumInstructions = m_NodeInstructions.GetCount();
  for (nsUInt32 uiInstructionIndex = 0; uiInstructionIndex < uiNumInstructions; ++uiInstructionIndex)
  {
    auto pCurrentNode = m_NodeInstructions[uiInstructionIndex];

    auto children = nsExpressionAST::GetChildren(pCurrentNode);
    for (auto pChild : children)
    {
      nsUInt32 uiRegisterIndex = nsInvalidIndex;
      if (m_NodeToRegisterIndex.TryGetValue(pChild, uiRegisterIndex))
      {
        auto& liveRegister = m_LiveIntervals[uiRegisterIndex];

        liveRegister.m_uiStart = nsMath::Min(liveRegister.m_uiStart, uiInstructionIndex);
        liveRegister.m_uiEnd = nsMath::Max(liveRegister.m_uiEnd, uiInstructionIndex);
      }
      else
      {
        NS_ASSERT_DEV(nsExpressionAST::NodeType::IsConstant(pChild->m_Type), "Must have a valid register for nodes that are not constants");
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::AssignRegisters()
{
  // This is an implementation of the linear scan register allocation algorithm without spilling
  // https://www2.seas.gwu.edu/~hchoi/teaching/cs160d/linearscan.pdf

  // Sort register lifetime by start index
  m_LiveIntervals.Sort([](const LiveInterval& a, const LiveInterval& b)
    { return a.m_uiStart < b.m_uiStart; });

  // Assign registers
  nsHybridArray<LiveInterval, 64> activeIntervals;
  nsHybridArray<nsUInt32, 64> freeRegisters;

  for (auto& liveInterval : m_LiveIntervals)
  {
    // Expire old intervals
    for (nsUInt32 uiActiveIndex = activeIntervals.GetCount(); uiActiveIndex-- > 0;)
    {
      auto& activeInterval = activeIntervals[uiActiveIndex];
      if (activeInterval.m_uiEnd <= liveInterval.m_uiStart)
      {
        nsUInt32 uiRegisterIndex = m_NodeToRegisterIndex[activeInterval.m_pNode];
        freeRegisters.PushBack(uiRegisterIndex);

        activeIntervals.RemoveAtAndCopy(uiActiveIndex);
      }
    }

    // Allocate register
    nsUInt32 uiNewRegister = 0;
    if (!freeRegisters.IsEmpty())
    {
      uiNewRegister = freeRegisters.PeekBack();
      freeRegisters.PopBack();
    }
    else
    {
      uiNewRegister = activeIntervals.GetCount();
    }
    m_NodeToRegisterIndex[liveInterval.m_pNode] = uiNewRegister;

    activeIntervals.PushBack(liveInterval);
  }

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::GenerateByteCode(const nsExpressionAST& ast, nsExpressionByteCode& out_byteCode)
{
  nsHybridArray<nsExpression::StreamDesc, 8> inputs;
  nsHybridArray<nsExpression::StreamDesc, 8> outputs;
  nsHybridArray<nsExpression::FunctionDesc, 4> functions;

  m_ByteCode.Clear();

  nsUInt32 uiMaxRegisterIndex = 0;

  m_InputToIndex.Clear();
  for (nsUInt32 i = 0; i < ast.m_InputNodes.GetCount(); ++i)
  {
    auto& desc = ast.m_InputNodes[i]->m_Desc;
    m_InputToIndex.Insert(desc.m_sName, i);

    inputs.PushBack(desc);
  }

  m_OutputToIndex.Clear();
  for (nsUInt32 i = 0; i < ast.m_OutputNodes.GetCount(); ++i)
  {
    auto& desc = ast.m_OutputNodes[i]->m_Desc;
    m_OutputToIndex.Insert(desc.m_sName, i);

    outputs.PushBack(desc);
  }

  m_FunctionToIndex.Clear();

  for (auto pCurrentNode : m_NodeInstructions)
  {
    const nsExpressionAST::NodeType::Enum nodeType = pCurrentNode->m_Type;
    nsExpressionAST::DataType::Enum dataType = pCurrentNode->m_ReturnType;
    if (dataType == nsExpressionAST::DataType::Unknown)
    {
      return NS_FAILURE;
    }

    bool bRightIsConstant = false;
    if (nsExpressionAST::NodeType::IsBinary(nodeType))
    {
      auto pBinary = static_cast<const nsExpressionAST::BinaryOperator*>(pCurrentNode);
      dataType = pBinary->m_pLeftOperand->m_ReturnType;
      bRightIsConstant = nsExpressionAST::NodeType::IsConstant(pBinary->m_pRightOperand->m_Type);
    }

    const auto opCode = NodeTypeToOpCode(nodeType, dataType, bRightIsConstant);
    if (opCode == nsExpressionByteCode::OpCode::Nop)
      return NS_FAILURE;

    nsUInt32 uiTargetRegister = m_NodeToRegisterIndex[pCurrentNode];
    if (nsExpressionAST::NodeType::IsOutput(nodeType) == false)
    {
      uiMaxRegisterIndex = nsMath::Max(uiMaxRegisterIndex, uiTargetRegister);
    }

    if (nsExpressionAST::NodeType::IsUnary(nodeType))
    {
      auto pUnary = static_cast<const nsExpressionAST::UnaryOperator*>(pCurrentNode);

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiTargetRegister);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pUnary->m_pOperand]);
    }
    else if (nsExpressionAST::NodeType::IsBinary(nodeType))
    {
      auto pBinary = static_cast<const nsExpressionAST::BinaryOperator*>(pCurrentNode);

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiTargetRegister);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pBinary->m_pLeftOperand]);

      if (bRightIsConstant)
      {
        NS_SUCCEED_OR_RETURN(GenerateConstantByteCode(static_cast<const nsExpressionAST::Constant*>(pBinary->m_pRightOperand)));
      }
      else
      {
        m_ByteCode.PushBack(m_NodeToRegisterIndex[pBinary->m_pRightOperand]);
      }
    }
    else if (nsExpressionAST::NodeType::IsTernary(nodeType))
    {
      auto pTernary = static_cast<const nsExpressionAST::TernaryOperator*>(pCurrentNode);

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiTargetRegister);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pFirstOperand]);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pSecondOperand]);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pThirdOperand]);
    }
    else if (nsExpressionAST::NodeType::IsConstant(nodeType))
    {
      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiTargetRegister);
      NS_SUCCEED_OR_RETURN(GenerateConstantByteCode(static_cast<const nsExpressionAST::Constant*>(pCurrentNode)));
    }
    else if (nsExpressionAST::NodeType::IsInput(nodeType))
    {
      auto& desc = static_cast<const nsExpressionAST::Input*>(pCurrentNode)->m_Desc;
      nsUInt32 uiInputIndex = 0;
      if (!m_InputToIndex.TryGetValue(desc.m_sName, uiInputIndex))
      {
        uiInputIndex = inputs.GetCount();
        m_InputToIndex.Insert(desc.m_sName, uiInputIndex);

        inputs.PushBack(desc);
      }

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiTargetRegister);
      m_ByteCode.PushBack(uiInputIndex);
    }
    else if (nsExpressionAST::NodeType::IsOutput(nodeType))
    {
      auto pOutput = static_cast<const nsExpressionAST::Output*>(pCurrentNode);
      auto& desc = pOutput->m_Desc;
      nsUInt32 uiOutputIndex = 0;
      NS_VERIFY(m_OutputToIndex.TryGetValue(desc.m_sName, uiOutputIndex), "Invalid output '{}'", desc.m_sName);

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiOutputIndex);
      m_ByteCode.PushBack(m_NodeToRegisterIndex[pOutput->m_pExpression]);
    }
    else if (nsExpressionAST::NodeType::IsFunctionCall(nodeType))
    {
      auto pFunctionCall = static_cast<const nsExpressionAST::FunctionCall*>(pCurrentNode);
      auto pDesc = pFunctionCall->m_Descs[pCurrentNode->m_uiOverloadIndex];
      nsHashedString sMangledName = pDesc->GetMangledName();

      nsUInt32 uiFunctionIndex = 0;
      if (!m_FunctionToIndex.TryGetValue(sMangledName, uiFunctionIndex))
      {
        uiFunctionIndex = functions.GetCount();
        m_FunctionToIndex.Insert(sMangledName, uiFunctionIndex);

        functions.PushBack(*pDesc);
        functions.PeekBack().m_sName = std::move(sMangledName);
      }

      m_ByteCode.PushBack(opCode);
      m_ByteCode.PushBack(uiFunctionIndex);
      m_ByteCode.PushBack(uiTargetRegister);

      m_ByteCode.PushBack(pFunctionCall->m_Arguments.GetCount());
      for (auto pArg : pFunctionCall->m_Arguments)
      {
        nsUInt32 uiArgRegister = m_NodeToRegisterIndex[pArg];
        m_ByteCode.PushBack(uiArgRegister);
      }
    }
    else
    {
      NS_ASSERT_NOT_IMPLEMENTED;
    }
  }

  out_byteCode.Init(m_ByteCode, inputs, outputs, functions, uiMaxRegisterIndex + 1, m_NodeInstructions.GetCount());
  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::GenerateConstantByteCode(const nsExpressionAST::Constant* pConstant)
{
  if (pConstant->m_ReturnType == nsExpressionAST::DataType::Float)
  {
    m_ByteCode.PushBack(*reinterpret_cast<const nsUInt32*>(&pConstant->m_Value.Get<float>()));
    return NS_SUCCESS;
  }
  else if (pConstant->m_ReturnType == nsExpressionAST::DataType::Int)
  {
    m_ByteCode.PushBack(pConstant->m_Value.Get<int>());
    return NS_SUCCESS;
  }
  else if (pConstant->m_ReturnType == nsExpressionAST::DataType::Bool)
  {
    m_ByteCode.PushBack(pConstant->m_Value.Get<bool>() ? 0xFFFFFFFF : 0);
    return NS_SUCCESS;
  }

  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

nsResult nsExpressionCompiler::TransformASTPreOrder(nsExpressionAST& ast, TransformFunc func)
{
  m_NodeStack.Clear();
  m_TransformCache.Clear();

  for (nsExpressionAST::Output*& pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return NS_FAILURE;

    NS_SUCCEED_OR_RETURN(TransformOutputNode(pOutputNode, func));

    m_NodeStack.PushBack(pOutputNode);

    while (!m_NodeStack.IsEmpty())
    {
      auto pParent = m_NodeStack.PeekBack();
      m_NodeStack.PopBack();

      auto children = nsExpressionAST::GetChildren(pParent);
      for (auto& pChild : children)
      {
        NS_SUCCEED_OR_RETURN(TransformNode(pChild, func));

        m_NodeStack.PushBack(pChild);
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::TransformASTPostOrder(nsExpressionAST& ast, TransformFunc func)
{
  m_NodeStack.Clear();
  m_NodeInstructions.Clear();
  auto& nodeStackTemp = m_NodeInstructions;

  for (nsExpressionAST::Node* pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return NS_FAILURE;

    nodeStackTemp.PushBack(pOutputNode);

    while (!nodeStackTemp.IsEmpty())
    {
      auto pParent = nodeStackTemp.PeekBack();
      nodeStackTemp.PopBack();

      m_NodeStack.PushBack(pParent);

      auto children = nsExpressionAST::GetChildren(pParent);
      for (auto pChild : children)
      {
        if (pChild != nullptr)
        {
          nodeStackTemp.PushBack(pChild);
        }
      }
    }
  }

  m_TransformCache.Clear();

  while (!m_NodeStack.IsEmpty())
  {
    auto pParent = m_NodeStack.PeekBack();
    m_NodeStack.PopBack();

    auto children = nsExpressionAST::GetChildren(pParent);
    for (auto& pChild : children)
    {
      NS_SUCCEED_OR_RETURN(TransformNode(pChild, func));
    }
  }

  for (nsExpressionAST::Output*& pOutputNode : ast.m_OutputNodes)
  {
    NS_SUCCEED_OR_RETURN(TransformOutputNode(pOutputNode, func));
  }

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::TransformNode(nsExpressionAST::Node*& pNode, TransformFunc& func)
{
  if (pNode == nullptr)
    return NS_SUCCESS;

  nsExpressionAST::Node* pNewNode = nullptr;
  if (m_TransformCache.TryGetValue(pNode, pNewNode) == false)
  {
    pNewNode = func(pNode);
    if (pNewNode == nullptr)
    {
      return NS_FAILURE;
    }

    m_TransformCache.Insert(pNode, pNewNode);
  }

  pNode = pNewNode;

  return NS_SUCCESS;
}

nsResult nsExpressionCompiler::TransformOutputNode(nsExpressionAST::Output*& pOutputNode, TransformFunc& func)
{
  if (pOutputNode == nullptr)
    return NS_SUCCESS;

  auto pNewOutput = func(pOutputNode);
  if (pNewOutput != pOutputNode)
  {
    if (pNewOutput != nullptr && nsExpressionAST::NodeType::IsOutput(pNewOutput->m_Type))
    {
      pOutputNode = static_cast<nsExpressionAST::Output*>(pNewOutput);
    }
    else
    {
      nsLog::Error("Transformed output node for '{}' is invalid", pOutputNode->m_Desc.m_sName);
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

void nsExpressionCompiler::DumpAST(const nsExpressionAST& ast, nsStringView sOutputPath, nsStringView sSuffix)
{
  if (sOutputPath.IsEmpty())
    return;

  nsDGMLGraph dgmlGraph;
  ast.PrintGraph(dgmlGraph);

  nsStringView sExt = sOutputPath.GetFileExtension();
  nsStringBuilder sFullPath;
  sFullPath.Append(sOutputPath.GetFileDirectory(), sOutputPath.GetFileName(), sSuffix, ".", sExt);

  nsDGMLGraphWriter dgmlGraphWriter;
  if (dgmlGraphWriter.WriteGraphToFile(sFullPath, dgmlGraph).Succeeded())
  {
    nsLog::Info("AST was dumped to: {}", sFullPath);
  }
  else
  {
    nsLog::Error("Failed to dump AST to: {}", sFullPath);
  }
}
