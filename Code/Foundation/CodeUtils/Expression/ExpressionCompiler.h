#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/Types/Delegate.h>

class nsExpressionByteCode;

class NS_FOUNDATION_DLL nsExpressionCompiler
{
public:
  nsExpressionCompiler();
  ~nsExpressionCompiler();

  nsResult Compile(nsExpressionAST& ref_ast, nsExpressionByteCode& out_byteCode, nsStringView sDebugAstOutputPath = nsStringView());

private:
  nsResult TransformAndOptimizeAST(nsExpressionAST& ast, nsStringView sDebugAstOutputPath);
  nsResult BuildNodeInstructions(const nsExpressionAST& ast);
  nsResult UpdateRegisterLifetime(const nsExpressionAST& ast);
  nsResult AssignRegisters();
  nsResult GenerateByteCode(const nsExpressionAST& ast, nsExpressionByteCode& out_byteCode);
  nsResult GenerateConstantByteCode(const nsExpressionAST::Constant* pConstant);

  using TransformFunc = nsDelegate<nsExpressionAST::Node*(nsExpressionAST::Node*)>;
  nsResult TransformASTPreOrder(nsExpressionAST& ast, TransformFunc func);
  nsResult TransformASTPostOrder(nsExpressionAST& ast, TransformFunc func);
  nsResult TransformNode(nsExpressionAST::Node*& pNode, TransformFunc& func);
  nsResult TransformOutputNode(nsExpressionAST::Output*& pOutputNode, TransformFunc& func);

  void DumpAST(const nsExpressionAST& ast, nsStringView sOutputPath, nsStringView sSuffix);

  nsHybridArray<nsExpressionAST::Node*, 64> m_NodeStack;
  nsHybridArray<nsExpressionAST::Node*, 64> m_NodeInstructions;
  nsHashTable<const nsExpressionAST::Node*, nsUInt32> m_NodeToRegisterIndex;
  nsHashTable<nsExpressionAST::Node*, nsExpressionAST::Node*> m_TransformCache;

  nsHashTable<nsHashedString, nsUInt32> m_InputToIndex;
  nsHashTable<nsHashedString, nsUInt32> m_OutputToIndex;
  nsHashTable<nsHashedString, nsUInt32> m_FunctionToIndex;

  nsDynamicArray<nsUInt32> m_ByteCode;

  struct LiveInterval
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_uiStart;
    nsUInt32 m_uiEnd;
    const nsExpressionAST::Node* m_pNode;
  };

  nsDynamicArray<LiveInterval> m_LiveIntervals;
};
