#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionAST.h>
#include <Foundation/CodeUtils/TokenParseUtils.h>

class WD_FOUNDATION_DLL wdExpressionParser
{
public:
  wdExpressionParser();
  ~wdExpressionParser();

  void RegisterFunction(const wdExpression::FunctionDesc& funcDesc);
  void UnregisterFunction(const wdExpression::FunctionDesc& funcDesc);

  struct Options
  {
    bool m_bTreatUnknownVariablesAsInputs = false;
  };

  wdResult Parse(wdStringView sCode, wdArrayPtr<wdExpression::StreamDesc> inputs, wdArrayPtr<wdExpression::StreamDesc> outputs, const Options& options, wdExpressionAST& out_ast);

private:
  static constexpr int s_iLowestPrecedence = 20;

  void RegisterKnownTypes();
  void RegisterBuiltinFunctions();
  void SetupInAndOutputs(wdArrayPtr<wdExpression::StreamDesc> inputs, wdArrayPtr<wdExpression::StreamDesc> outputs);

  wdResult ParseStatement();
  wdResult ParseType(wdStringView sTypeName, wdEnum<wdExpressionAST::DataType>& out_type);
  wdResult ParseVariableDefinition(wdEnum<wdExpressionAST::DataType> type);
  wdResult ParseAssignment();

  wdExpressionAST::Node* ParseFactor();
  wdExpressionAST::Node* ParseExpression(int iPrecedence = s_iLowestPrecedence);
  wdExpressionAST::Node* ParseUnaryExpression();
  wdExpressionAST::Node* ParseFunctionCall(wdStringView sFunctionName);
  wdExpressionAST::Node* ParseSwizzle(wdExpressionAST::Node* pExpression);

  bool AcceptStatementTerminator();
  bool AcceptOperator(wdStringView sName);
  bool AcceptBinaryOperator(wdExpressionAST::NodeType::Enum& out_binaryOp, int& out_iOperatorPrecedence, wdUInt32& out_uiOperatorLength);
  wdExpressionAST::Node* GetVariable(wdStringView sVarName);
  wdExpressionAST::Node* EnsureExpectedType(wdExpressionAST::Node* pNode, wdExpressionAST::DataType::Enum expectedType);
  wdExpressionAST::Node* Unpack(wdExpressionAST::Node* pNode, bool bUnassignedError = true);

  wdResult Expect(const char* szToken, const wdToken** pExpectedToken = nullptr);
  wdResult Expect(wdTokenType::Enum Type, const wdToken** pExpectedToken = nullptr);

  void ReportError(const wdToken* pToken, const wdFormatString& message);

  /// \brief Checks whether all outputs have been written
  wdResult CheckOutputs();

  Options m_Options;

  wdTokenParseUtils::TokenStream m_TokenStream;
  wdUInt32 m_uiCurrentToken = 0;
  wdExpressionAST* m_pAST = nullptr;

  wdHashTable<wdHashedString, wdEnum<wdExpressionAST::DataType>> m_KnownTypes;

  wdHashTable<wdHashedString, wdExpressionAST::Node*> m_KnownVariables;
  wdHashTable<wdHashedString, wdEnum<wdExpressionAST::NodeType>> m_BuiltinFunctions;
  wdHashTable<wdHashedString, wdHybridArray<wdExpression::FunctionDesc, 1>> m_FunctionDescs;
};

#include <Foundation/CodeUtils/Expression/Implementation/ExpressionParser_inl.h>
