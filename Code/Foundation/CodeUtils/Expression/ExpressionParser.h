#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/TokenParseUtils.h>

class NS_FOUNDATION_DLL nsExpressionParser
{
public:
  nsExpressionParser();
  ~nsExpressionParser();

  static const nsHashTable<nsHashedString, nsEnum<nsExpressionAST::DataType>>& GetKnownTypes();
  static const nsHashTable<nsHashedString, nsEnum<nsExpressionAST::NodeType>>& GetBuiltinFunctions();

  void RegisterFunction(const nsExpression::FunctionDesc& funcDesc);
  void UnregisterFunction(const nsExpression::FunctionDesc& funcDesc);

  struct Options
  {
    bool m_bTreatUnknownVariablesAsInputs = false;
  };

  nsResult Parse(nsStringView sCode, nsArrayPtr<nsExpression::StreamDesc> inputs, nsArrayPtr<nsExpression::StreamDesc> outputs, const Options& options, nsExpressionAST& out_ast);

private:
  static constexpr int s_iLowestPrecedence = 20;

  static void RegisterKnownTypes();
  static void RegisterBuiltinFunctions();
  void SetupInAndOutputs(nsArrayPtr<nsExpression::StreamDesc> inputs, nsArrayPtr<nsExpression::StreamDesc> outputs);

  nsResult ParseStatement();
  nsResult ParseType(nsStringView sTypeName, nsEnum<nsExpressionAST::DataType>& out_type);
  nsResult ParseVariableDefinition(nsEnum<nsExpressionAST::DataType> type);
  nsResult ParseAssignment();

  nsExpressionAST::Node* ParseFactor();
  nsExpressionAST::Node* ParseExpression(int iPrecedence = s_iLowestPrecedence);
  nsExpressionAST::Node* ParseUnaryExpression();
  nsExpressionAST::Node* ParseFunctionCall(nsStringView sFunctionName);
  nsExpressionAST::Node* ParseSwizzle(nsExpressionAST::Node* pExpression);

  bool AcceptStatementTerminator();
  bool AcceptOperator(nsStringView sName);
  bool AcceptBinaryOperator(nsExpressionAST::NodeType::Enum& out_binaryOp, int& out_iOperatorPrecedence, nsUInt32& out_uiOperatorLength);
  nsExpressionAST::Node* GetVariable(nsStringView sVarName);
  nsExpressionAST::Node* EnsureExpectedType(nsExpressionAST::Node* pNode, nsExpressionAST::DataType::Enum expectedType);
  nsExpressionAST::Node* Unpack(nsExpressionAST::Node* pNode, bool bUnassignedError = true);

  nsResult Expect(nsStringView sToken, const nsToken** pExpectedToken = nullptr);
  nsResult Expect(nsTokenType::Enum Type, const nsToken** pExpectedToken = nullptr);

  void ReportError(const nsToken* pToken, const nsFormatString& message);

  /// \brief Checks whether all outputs have been written
  nsResult CheckOutputs();

  Options m_Options;

  nsTokenParseUtils::TokenStream m_TokenStream;
  nsUInt32 m_uiCurrentToken = 0;
  nsExpressionAST* m_pAST = nullptr;

  nsHashTable<nsHashedString, nsExpressionAST::Node*> m_KnownVariables;
  nsHashTable<nsHashedString, nsHybridArray<nsExpression::FunctionDesc, 1>> m_FunctionDescs;
};

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionParser_inl.h>
