#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Types/Delegate.h>

class wdExpressionByteCode;

class WD_FOUNDATION_DLL wdExpressionCompiler
{
public:
  wdExpressionCompiler();
  ~wdExpressionCompiler();

  wdResult Compile(wdExpressionAST& ref_ast, wdExpressionByteCode& out_byteCode, wdStringView sDebugAstOutputPath = wdStringView());

private:
  wdResult TransformAndOptimizeAST(wdExpressionAST& ast, wdStringView sDebugAstOutputPath);
  wdResult BuildNodeInstructions(const wdExpressionAST& ast);
  wdResult UpdateRegisterLifetime(const wdExpressionAST& ast);
  wdResult AssignRegisters();
  wdResult GenerateByteCode(const wdExpressionAST& ast, wdExpressionByteCode& out_byteCode);
  wdResult GenerateConstantByteCode(const wdExpressionAST::Constant* pConstant, wdExpressionByteCode& out_byteCode);

  using TransformFunc = wdDelegate<wdExpressionAST::Node*(wdExpressionAST::Node*)>;
  wdResult TransformASTPreOrder(wdExpressionAST& ast, TransformFunc func);
  wdResult TransformASTPostOrder(wdExpressionAST& ast, TransformFunc func);
  wdResult TransformNode(wdExpressionAST::Node*& pNode, TransformFunc& func);
  wdResult TransformOutputNode(wdExpressionAST::Output*& pOutputNode, TransformFunc& func);

  void DumpAST(const wdExpressionAST& ast, wdStringView sOutputPath, wdStringView sSuffix);

  wdHybridArray<wdExpressionAST::Node*, 64> m_NodeStack;
  wdHybridArray<wdExpressionAST::Node*, 64> m_NodeInstructions;
  wdHashTable<const wdExpressionAST::Node*, wdUInt32> m_NodeToRegisterIndex;
  wdHashTable<wdExpressionAST::Node*, wdExpressionAST::Node*> m_TransformCache;

  wdHashTable<wdHashedString, wdUInt32> m_InputToIndex;
  wdHashTable<wdHashedString, wdUInt32> m_OutputToIndex;
  wdHashTable<wdHashedString, wdUInt32> m_FunctionToIndex;

  struct LiveInterval
  {
    WD_DECLARE_POD_TYPE();

    wdUInt32 m_uiStart;
    wdUInt32 m_uiEnd;
    const wdExpressionAST::Node* m_pNode;
  };

  wdDynamicArray<LiveInterval> m_LiveIntervals;
};
