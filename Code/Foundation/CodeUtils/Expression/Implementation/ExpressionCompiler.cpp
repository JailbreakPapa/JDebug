#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/DGMLWriter.h>

namespace
{
#define ADD_OFFSET(opCode) static_cast<wdExpressionByteCode::OpCode::Enum>((opCode) + uiOffset)

  static wdExpressionByteCode::OpCode::Enum NodeTypeToOpCode(wdExpressionAST::NodeType::Enum nodeType, wdExpressionAST::DataType::Enum dataType, bool bRightIsConstant)
  {
    const wdExpression::RegisterType::Enum registerType = wdExpressionAST::DataType::GetRegisterType(dataType);
    const bool bFloat = registerType == wdExpression::RegisterType::Float;
    const bool bInt = registerType == wdExpression::RegisterType::Int;
    const wdUInt32 uiOffset = bRightIsConstant ? wdExpressionByteCode::OpCode::FirstBinaryWithConstant - wdExpressionByteCode::OpCode::FirstBinary : 0;

    switch (nodeType)
    {
      case wdExpressionAST::NodeType::Absolute:
        return bFloat ? wdExpressionByteCode::OpCode::AbsF_R : wdExpressionByteCode::OpCode::AbsI_R;
      case wdExpressionAST::NodeType::Sqrt:
        return wdExpressionByteCode::OpCode::SqrtF_R;

      case wdExpressionAST::NodeType::Exp:
        return wdExpressionByteCode::OpCode::ExpF_R;
      case wdExpressionAST::NodeType::Ln:
        return wdExpressionByteCode::OpCode::LnF_R;
      case wdExpressionAST::NodeType::Log2:
        return bFloat ? wdExpressionByteCode::OpCode::Log2F_R : wdExpressionByteCode::OpCode::Log2I_R;
      case wdExpressionAST::NodeType::Log10:
        return wdExpressionByteCode::OpCode::Log10F_R;
      case wdExpressionAST::NodeType::Pow2:
        return wdExpressionByteCode::OpCode::Pow2F_R;

      case wdExpressionAST::NodeType::Sin:
        return wdExpressionByteCode::OpCode::SinF_R;
      case wdExpressionAST::NodeType::Cos:
        return wdExpressionByteCode::OpCode::CosF_R;
      case wdExpressionAST::NodeType::Tan:
        return wdExpressionByteCode::OpCode::TanF_R;

      case wdExpressionAST::NodeType::ASin:
        return wdExpressionByteCode::OpCode::ASinF_R;
      case wdExpressionAST::NodeType::ACos:
        return wdExpressionByteCode::OpCode::ACosF_R;
      case wdExpressionAST::NodeType::ATan:
        return wdExpressionByteCode::OpCode::ATanF_R;

      case wdExpressionAST::NodeType::Round:
        return wdExpressionByteCode::OpCode::RoundF_R;
      case wdExpressionAST::NodeType::Floor:
        return wdExpressionByteCode::OpCode::FloorF_R;
      case wdExpressionAST::NodeType::Ceil:
        return wdExpressionByteCode::OpCode::CeilF_R;
      case wdExpressionAST::NodeType::Trunc:
        return wdExpressionByteCode::OpCode::TruncF_R;

      case wdExpressionAST::NodeType::BitwiseNot:
        return wdExpressionByteCode::OpCode::NotI_R;
      case wdExpressionAST::NodeType::LogicalNot:
        return wdExpressionByteCode::OpCode::NotB_R;

      case wdExpressionAST::NodeType::TypeConversion:
        return bFloat ? wdExpressionByteCode::OpCode::IToF_R : wdExpressionByteCode::OpCode::FToI_R;

      case wdExpressionAST::NodeType::Add:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::AddF_RR : wdExpressionByteCode::OpCode::AddI_RR);
      case wdExpressionAST::NodeType::Subtract:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::SubF_RR : wdExpressionByteCode::OpCode::SubI_RR);
      case wdExpressionAST::NodeType::Multiply:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::MulF_RR : wdExpressionByteCode::OpCode::MulI_RR);
      case wdExpressionAST::NodeType::Divide:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::DivF_RR : wdExpressionByteCode::OpCode::DivI_RR);
      case wdExpressionAST::NodeType::Min:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::MinF_RR : wdExpressionByteCode::OpCode::MinI_RR);
      case wdExpressionAST::NodeType::Max:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::MaxF_RR : wdExpressionByteCode::OpCode::MaxI_RR);

      case wdExpressionAST::NodeType::BitshiftLeft:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::ShlI_RR);
      case wdExpressionAST::NodeType::BitshiftRight:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::ShrI_RR);
      case wdExpressionAST::NodeType::BitwiseAnd:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::AndI_RR);
      case wdExpressionAST::NodeType::BitwiseXor:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::XorI_RR);
      case wdExpressionAST::NodeType::BitwiseOr:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::OrI_RR);

      case wdExpressionAST::NodeType::Equal:
        if (bFloat)
          return ADD_OFFSET(wdExpressionByteCode::OpCode::EqF_RR);
        else if (bInt)
          return ADD_OFFSET(wdExpressionByteCode::OpCode::EqI_RR);
        else
          return ADD_OFFSET(wdExpressionByteCode::OpCode::EqB_RR);
      case wdExpressionAST::NodeType::NotEqual:
        if (bFloat)
          return ADD_OFFSET(wdExpressionByteCode::OpCode::NEqF_RR);
        else if (bInt)
          return ADD_OFFSET(wdExpressionByteCode::OpCode::NEqI_RR);
        else
          return ADD_OFFSET(wdExpressionByteCode::OpCode::NEqB_RR);
      case wdExpressionAST::NodeType::Less:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::LtF_RR : wdExpressionByteCode::OpCode::LtI_RR);
      case wdExpressionAST::NodeType::LessEqual:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::LEqF_RR : wdExpressionByteCode::OpCode::LEqI_RR);
      case wdExpressionAST::NodeType::Greater:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::GtF_RR : wdExpressionByteCode::OpCode::GtI_RR);
      case wdExpressionAST::NodeType::GreaterEqual:
        return ADD_OFFSET(bFloat ? wdExpressionByteCode::OpCode::GEqF_RR : wdExpressionByteCode::OpCode::GEqI_RR);

      case wdExpressionAST::NodeType::LogicalAnd:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::AndB_RR);
      case wdExpressionAST::NodeType::LogicalOr:
        return ADD_OFFSET(wdExpressionByteCode::OpCode::OrB_RR);

      case wdExpressionAST::NodeType::Select:
        if (bFloat)
          return wdExpressionByteCode::OpCode::SelF_RRR;
        else if (bInt)
          return wdExpressionByteCode::OpCode::SelI_RRR;
        else
          return wdExpressionByteCode::OpCode::SelB_RRR;

      case wdExpressionAST::NodeType::Constant:
        return wdExpressionByteCode::OpCode::MovX_C;
      case wdExpressionAST::NodeType::Input:
        return bFloat ? wdExpressionByteCode::OpCode::LoadF : wdExpressionByteCode::OpCode::LoadI;
      case wdExpressionAST::NodeType::Output:
        return bFloat ? wdExpressionByteCode::OpCode::StoreF : wdExpressionByteCode::OpCode::StoreI;
      case wdExpressionAST::NodeType::FunctionCall:
        return wdExpressionByteCode::OpCode::Call;
      case wdExpressionAST::NodeType::ConstructorCall:
        WD_REPORT_FAILURE("Constructor calls should not exist anymore after AST transformations");
        return wdExpressionByteCode::OpCode::Nop;

      default:
        WD_ASSERT_NOT_IMPLEMENTED;
        return wdExpressionByteCode::OpCode::Nop;
    }
  }

#undef ADD_OFFSET
} // namespace

wdExpressionCompiler::wdExpressionCompiler() = default;
wdExpressionCompiler::~wdExpressionCompiler() = default;

wdResult wdExpressionCompiler::Compile(wdExpressionAST& ref_ast, wdExpressionByteCode& out_byteCode, wdStringView sDebugAstOutputPath /*= wdStringView()*/)
{
  out_byteCode.Clear();

  WD_SUCCEED_OR_RETURN(TransformAndOptimizeAST(ref_ast, sDebugAstOutputPath));
  WD_SUCCEED_OR_RETURN(BuildNodeInstructions(ref_ast));
  WD_SUCCEED_OR_RETURN(UpdateRegisterLifetime(ref_ast));
  WD_SUCCEED_OR_RETURN(AssignRegisters());
  WD_SUCCEED_OR_RETURN(GenerateByteCode(ref_ast, out_byteCode));

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::TransformAndOptimizeAST(wdExpressionAST& ast, wdStringView sDebugAstOutputPath)
{
  DumpAST(ast, sDebugAstOutputPath, "_00");

  WD_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, wdMakeDelegate(&wdExpressionAST::TypeDeductionAndConversion, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_01_TypeConv");

  WD_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, wdMakeDelegate(&wdExpressionAST::ReplaceVectorInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_02_ReplacedVectorInst");

  WD_SUCCEED_OR_RETURN(ast.ScalarizeOutputs());
  WD_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, wdMakeDelegate(&wdExpressionAST::ScalarizeVectorInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_03_Scalarized");

  WD_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, wdMakeDelegate(&wdExpressionAST::FoldConstants, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_04_ConstantFolded1");

  WD_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, wdMakeDelegate(&wdExpressionAST::ReplaceUnsupportedInstructions, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_05_ReplacedUnsupportedInst");

  WD_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, wdMakeDelegate(&wdExpressionAST::FoldConstants, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_06_ConstantFolded2");

  WD_SUCCEED_OR_RETURN(TransformASTPostOrder(ast, wdMakeDelegate(&wdExpressionAST::CommonSubexpressionElimination, &ast)));
  WD_SUCCEED_OR_RETURN(TransformASTPreOrder(ast, wdMakeDelegate(&wdExpressionAST::Validate, &ast)));
  DumpAST(ast, sDebugAstOutputPath, "_07_Optimized");

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::BuildNodeInstructions(const wdExpressionAST& ast)
{
  m_NodeStack.Clear();
  m_NodeInstructions.Clear();
  auto& nodeStackTemp = m_NodeInstructions;

  // Build node instruction order aka post order tree traversal
  for (wdExpressionAST::Node* pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return WD_FAILURE;

    WD_ASSERT_DEV(nodeStackTemp.IsEmpty(), "Implementation error");

    nodeStackTemp.PushBack(pOutputNode);

    while (!nodeStackTemp.IsEmpty())
    {
      auto pCurrentNode = nodeStackTemp.PeekBack();
      nodeStackTemp.PopBack();

      if (pCurrentNode == nullptr)
      {
        return WD_FAILURE;
      }

      m_NodeStack.PushBack(pCurrentNode);

      if (wdExpressionAST::NodeType::IsBinary(pCurrentNode->m_Type))
      {
        auto pBinary = static_cast<const wdExpressionAST::BinaryOperator*>(pCurrentNode);
        nodeStackTemp.PushBack(pBinary->m_pLeftOperand);

        // Do not push the right operand if it is a constant, we don't want a separate mov instruction for it
        // since all binary operators can take a constant as right operand in place.
        const bool bRightIsConstant = wdExpressionAST::NodeType::IsConstant(pBinary->m_pRightOperand->m_Type);
        if (!bRightIsConstant)
        {
          nodeStackTemp.PushBack(pBinary->m_pRightOperand);
        }
      }
      else
      {
        auto children = wdExpressionAST::GetChildren(pCurrentNode);
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
    return WD_FAILURE;
  }

  WD_ASSERT_DEV(m_NodeInstructions.IsEmpty(), "Implementation error");

  m_NodeToRegisterIndex.Clear();
  m_LiveIntervals.Clear();
  wdUInt32 uiNextRegisterIndex = 0;

  // De-duplicate nodes, build final instruction list and assign virtual register indices. Also determine their lifetime start.
  while (!m_NodeStack.IsEmpty())
  {
    auto pCurrentNode = m_NodeStack.PeekBack();
    m_NodeStack.PopBack();

    if (!m_NodeToRegisterIndex.Contains(pCurrentNode))
    {
      m_NodeInstructions.PushBack(pCurrentNode);

      if (wdExpressionAST::NodeType::IsOutput(pCurrentNode->m_Type))
        continue;

      m_NodeToRegisterIndex.Insert(pCurrentNode, uiNextRegisterIndex);
      ++uiNextRegisterIndex;

      wdUInt32 uiCurrentInstructionIndex = m_NodeInstructions.GetCount() - 1;
      m_LiveIntervals.PushBack({uiCurrentInstructionIndex, uiCurrentInstructionIndex, pCurrentNode});
      WD_ASSERT_DEV(m_LiveIntervals.GetCount() == uiNextRegisterIndex, "Implementation error");
    }
  }

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::UpdateRegisterLifetime(const wdExpressionAST& ast)
{
  wdUInt32 uiNumInstructions = m_NodeInstructions.GetCount();
  for (wdUInt32 uiInstructionIndex = 0; uiInstructionIndex < uiNumInstructions; ++uiInstructionIndex)
  {
    auto pCurrentNode = m_NodeInstructions[uiInstructionIndex];

    auto children = wdExpressionAST::GetChildren(pCurrentNode);
    for (auto pChild : children)
    {
      wdUInt32 uiRegisterIndex = wdInvalidIndex;
      if (m_NodeToRegisterIndex.TryGetValue(pChild, uiRegisterIndex))
      {
        auto& liveRegister = m_LiveIntervals[uiRegisterIndex];

        liveRegister.m_uiStart = wdMath::Min(liveRegister.m_uiStart, uiInstructionIndex);
        liveRegister.m_uiEnd = wdMath::Max(liveRegister.m_uiEnd, uiInstructionIndex);
      }
      else
      {
        WD_ASSERT_DEV(wdExpressionAST::NodeType::IsConstant(pChild->m_Type), "Must have a valid register for nodes that are not constants");
      }
    }
  }

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::AssignRegisters()
{
  // This is an implementation of the linear scan register allocation algorithm without spilling
  // https://www2.seas.gwu.edu/~hchoi/teaching/cs160d/linearscan.pdf

  // Sort register lifetime by start index
  m_LiveIntervals.Sort([](const LiveInterval& a, const LiveInterval& b) { return a.m_uiStart < b.m_uiStart; });

  // Assign registers
  wdHybridArray<LiveInterval, 64> activeIntervals;
  wdHybridArray<wdUInt32, 64> freeRegisters;

  for (auto& liveInterval : m_LiveIntervals)
  {
    // Expire old intervals
    for (wdUInt32 uiActiveIndex = activeIntervals.GetCount(); uiActiveIndex-- > 0;)
    {
      auto& activeInterval = activeIntervals[uiActiveIndex];
      if (activeInterval.m_uiEnd <= liveInterval.m_uiStart)
      {
        wdUInt32 uiRegisterIndex = m_NodeToRegisterIndex[activeInterval.m_pNode];
        freeRegisters.PushBack(uiRegisterIndex);

        activeIntervals.RemoveAtAndCopy(uiActiveIndex);
      }
    }

    // Allocate register
    wdUInt32 uiNewRegister = 0;
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

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::GenerateByteCode(const wdExpressionAST& ast, wdExpressionByteCode& out_byteCode)
{
  auto& byteCode = out_byteCode.m_ByteCode;

  wdUInt32 uiMaxRegisterIndex = 0;

  m_InputToIndex.Clear();
  m_OutputToIndex.Clear();
  m_FunctionToIndex.Clear();

  for (auto pCurrentNode : m_NodeInstructions)
  {
    const wdExpressionAST::NodeType::Enum nodeType = pCurrentNode->m_Type;
    wdExpressionAST::DataType::Enum dataType = pCurrentNode->m_ReturnType;
    if (dataType == wdExpressionAST::DataType::Unknown)
    {
      return WD_FAILURE;
    }

    bool bRightIsConstant = false;
    if (wdExpressionAST::NodeType::IsBinary(nodeType))
    {
      auto pBinary = static_cast<const wdExpressionAST::BinaryOperator*>(pCurrentNode);
      dataType = pBinary->m_pLeftOperand->m_ReturnType;
      bRightIsConstant = wdExpressionAST::NodeType::IsConstant(pBinary->m_pRightOperand->m_Type);
    }

    const auto opCode = NodeTypeToOpCode(nodeType, dataType, bRightIsConstant);
    if (opCode == wdExpressionByteCode::OpCode::Nop)
      return WD_FAILURE;

    wdUInt32 uiTargetRegister = m_NodeToRegisterIndex[pCurrentNode];
    if (wdExpressionAST::NodeType::IsOutput(nodeType) == false)
    {
      uiMaxRegisterIndex = wdMath::Max(uiMaxRegisterIndex, uiTargetRegister);
    }

    if (wdExpressionAST::NodeType::IsUnary(nodeType))
    {
      auto pUnary = static_cast<const wdExpressionAST::UnaryOperator*>(pCurrentNode);

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiTargetRegister);
      byteCode.PushBack(m_NodeToRegisterIndex[pUnary->m_pOperand]);
    }
    else if (wdExpressionAST::NodeType::IsBinary(nodeType))
    {
      auto pBinary = static_cast<const wdExpressionAST::BinaryOperator*>(pCurrentNode);

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiTargetRegister);
      byteCode.PushBack(m_NodeToRegisterIndex[pBinary->m_pLeftOperand]);

      if (bRightIsConstant)
      {
        WD_SUCCEED_OR_RETURN(GenerateConstantByteCode(static_cast<const wdExpressionAST::Constant*>(pBinary->m_pRightOperand), out_byteCode));
      }
      else
      {
        byteCode.PushBack(m_NodeToRegisterIndex[pBinary->m_pRightOperand]);
      }
    }
    else if (wdExpressionAST::NodeType::IsTernary(nodeType))
    {
      auto pTernary = static_cast<const wdExpressionAST::TernaryOperator*>(pCurrentNode);

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiTargetRegister);
      byteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pFirstOperand]);
      byteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pSecondOperand]);
      byteCode.PushBack(m_NodeToRegisterIndex[pTernary->m_pThirdOperand]);
    }
    else if (wdExpressionAST::NodeType::IsConstant(nodeType))
    {
      byteCode.PushBack(opCode);
      byteCode.PushBack(uiTargetRegister);
      WD_SUCCEED_OR_RETURN(GenerateConstantByteCode(static_cast<const wdExpressionAST::Constant*>(pCurrentNode), out_byteCode));
    }
    else if (wdExpressionAST::NodeType::IsInput(nodeType))
    {
      auto& desc = static_cast<const wdExpressionAST::Input*>(pCurrentNode)->m_Desc;
      wdUInt32 uiInputIndex = 0;
      if (!m_InputToIndex.TryGetValue(desc.m_sName, uiInputIndex))
      {
        uiInputIndex = out_byteCode.m_Inputs.GetCount();
        m_InputToIndex.Insert(desc.m_sName, uiInputIndex);

        out_byteCode.m_Inputs.PushBack(desc);
      }

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiTargetRegister);
      byteCode.PushBack(uiInputIndex);
    }
    else if (wdExpressionAST::NodeType::IsOutput(nodeType))
    {
      auto pOutput = static_cast<const wdExpressionAST::Output*>(pCurrentNode);
      auto& desc = pOutput->m_Desc;
      wdUInt32 uiOutputIndex = 0;
      if (!m_OutputToIndex.TryGetValue(desc.m_sName, uiOutputIndex))
      {
        uiOutputIndex = out_byteCode.m_Outputs.GetCount();
        m_OutputToIndex.Insert(desc.m_sName, uiOutputIndex);

        out_byteCode.m_Outputs.PushBack(desc);
      }

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiOutputIndex);
      byteCode.PushBack(m_NodeToRegisterIndex[pOutput->m_pExpression]);
    }
    else if (wdExpressionAST::NodeType::IsFunctionCall(nodeType))
    {
      auto pFunctionCall = static_cast<const wdExpressionAST::FunctionCall*>(pCurrentNode);
      auto pDesc = pFunctionCall->m_Descs[pCurrentNode->m_uiOverloadIndex];
      wdHashedString sMangledName = pDesc->GetMangledName();

      wdUInt32 uiFunctionIndex = 0;
      if (!m_FunctionToIndex.TryGetValue(sMangledName, uiFunctionIndex))
      {
        uiFunctionIndex = out_byteCode.m_Functions.GetCount();
        m_FunctionToIndex.Insert(sMangledName, uiFunctionIndex);

        out_byteCode.m_Functions.PushBack(*pDesc);
        out_byteCode.m_Functions.PeekBack().m_sName = sMangledName;
      }

      byteCode.PushBack(opCode);
      byteCode.PushBack(uiFunctionIndex);
      byteCode.PushBack(uiTargetRegister);

      byteCode.PushBack(pFunctionCall->m_Arguments.GetCount());
      for (auto pArg : pFunctionCall->m_Arguments)
      {
        wdUInt32 uiArgRegister = m_NodeToRegisterIndex[pArg];
        byteCode.PushBack(uiArgRegister);
      }
    }
    else
    {
      WD_ASSERT_NOT_IMPLEMENTED;
    }
  }

  out_byteCode.m_uiNumInstructions = m_NodeInstructions.GetCount();
  out_byteCode.m_uiNumTempRegisters = uiMaxRegisterIndex + 1;

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::GenerateConstantByteCode(const wdExpressionAST::Constant* pConstant, wdExpressionByteCode& out_byteCode)
{
  auto& byteCode = out_byteCode.m_ByteCode;

  if (pConstant->m_ReturnType == wdExpressionAST::DataType::Float)
  {
    byteCode.PushBack(*reinterpret_cast<const wdUInt32*>(&pConstant->m_Value.Get<float>()));
    return WD_SUCCESS;
  }
  else if (pConstant->m_ReturnType == wdExpressionAST::DataType::Int)
  {
    byteCode.PushBack(pConstant->m_Value.Get<int>());
    return WD_SUCCESS;
  }
  else if (pConstant->m_ReturnType == wdExpressionAST::DataType::Bool)
  {
    byteCode.PushBack(pConstant->m_Value.Get<bool>() ? 0xFFFFFFFF : 0);
    return WD_SUCCESS;
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return WD_FAILURE;
}

wdResult wdExpressionCompiler::TransformASTPreOrder(wdExpressionAST& ast, TransformFunc func)
{
  m_NodeStack.Clear();
  m_TransformCache.Clear();

  for (wdExpressionAST::Output*& pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return WD_FAILURE;

    WD_SUCCEED_OR_RETURN(TransformOutputNode(pOutputNode, func));

    m_NodeStack.PushBack(pOutputNode);

    while (!m_NodeStack.IsEmpty())
    {
      auto pParent = m_NodeStack.PeekBack();
      m_NodeStack.PopBack();

      auto children = wdExpressionAST::GetChildren(pParent);
      for (auto& pChild : children)
      {
        WD_SUCCEED_OR_RETURN(TransformNode(pChild, func));

        m_NodeStack.PushBack(pChild);
      }
    }
  }

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::TransformASTPostOrder(wdExpressionAST& ast, TransformFunc func)
{
  m_NodeStack.Clear();
  m_NodeInstructions.Clear();
  auto& nodeStackTemp = m_NodeInstructions;

  for (wdExpressionAST::Node* pOutputNode : ast.m_OutputNodes)
  {
    if (pOutputNode == nullptr)
      return WD_FAILURE;

    nodeStackTemp.PushBack(pOutputNode);

    while (!nodeStackTemp.IsEmpty())
    {
      auto pParent = nodeStackTemp.PeekBack();
      nodeStackTemp.PopBack();

      m_NodeStack.PushBack(pParent);

      auto children = wdExpressionAST::GetChildren(pParent);
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

    auto children = wdExpressionAST::GetChildren(pParent);
    for (auto& pChild : children)
    {
      WD_SUCCEED_OR_RETURN(TransformNode(pChild, func));
    }
  }

  for (wdExpressionAST::Output*& pOutputNode : ast.m_OutputNodes)
  {
    WD_SUCCEED_OR_RETURN(TransformOutputNode(pOutputNode, func));
  }

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::TransformNode(wdExpressionAST::Node*& pNode, TransformFunc& func)
{
  if (pNode == nullptr)
    return WD_SUCCESS;

  wdExpressionAST::Node* pNewNode = nullptr;
  if (m_TransformCache.TryGetValue(pNode, pNewNode) == false)
  {
    pNewNode = func(pNode);
    if (pNewNode == nullptr)
    {
      return WD_FAILURE;
    }

    m_TransformCache.Insert(pNode, pNewNode);
  }

  pNode = pNewNode;

  return WD_SUCCESS;
}

wdResult wdExpressionCompiler::TransformOutputNode(wdExpressionAST::Output*& pOutputNode, TransformFunc& func)
{
  if (pOutputNode == nullptr)
    return WD_SUCCESS;

  auto pNewOutput = func(pOutputNode);
  if (pNewOutput != pOutputNode)
  {
    if (pNewOutput != nullptr && wdExpressionAST::NodeType::IsOutput(pNewOutput->m_Type))
    {
      pOutputNode = static_cast<wdExpressionAST::Output*>(pNewOutput);
    }
    else
    {
      wdLog::Error("Transformed output node for '{}' is invalid", pOutputNode->m_Desc.m_sName);
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

void wdExpressionCompiler::DumpAST(const wdExpressionAST& ast, wdStringView sOutputPath, wdStringView sSuffix)
{
  if (sOutputPath.IsEmpty())
    return;

  wdDGMLGraph dgmlGraph;
  ast.PrintGraph(dgmlGraph);

  wdStringView sExt = sOutputPath.GetFileExtension();
  wdStringBuilder sFullPath;
  sFullPath.Append(sOutputPath.GetFileDirectory(), sOutputPath.GetFileName(), sSuffix, ".", sExt);

  wdDGMLGraphWriter dgmlGraphWriter;
  if (dgmlGraphWriter.WriteGraphToFile(sFullPath, dgmlGraph).Succeeded())
  {
    wdLog::Info("AST was dumped to: {}", sFullPath);
  }
  else
  {
    wdLog::Error("Failed to dump AST to: {}", sFullPath);
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Expression_Implementation_ExpressionCompiler);
