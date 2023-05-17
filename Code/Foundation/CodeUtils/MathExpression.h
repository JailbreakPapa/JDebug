#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/Strings/String.h>

class wdLogInterface;

/// \brief A wrapper around wdExpression infrastructure to evaluate simple math expressions
class WD_FOUNDATION_DLL wdMathExpression
{
public:
  /// \brief Creates a new invalid math expression.
  ///
  /// Need to call Reset before you can do anything with it.
  wdMathExpression();

  /// \brief Initializes using a given expression.
  ///
  /// If anything goes wrong it is logged and the math expression is in an invalid state.
  /// \param log
  ///   If null, default log interface will be used.
  explicit wdMathExpression(wdStringView sExpressionString); // [tested]

  /// \brief Reinitializes using the given expression.
  ///
  /// An empty string or nullptr are considered to be 'invalid' expressions.
  void Reset(wdStringView sExpressionString);

  /// Whether the expression is valid and can be evaluated.
  bool IsValid() const { return m_bIsValid; }

  /// Returns the original expression string that this MathExpression can evaluate.
  const char* GetExpressionString() const { return m_sOriginalExpression; }

  struct Input
  {
    wdHashedString m_sName;
    float m_fValue;
  };

  /// \brief Evaluates parsed expression with the given inputs.
  ///
  /// Only way this function can fail is if the expression was not valid.
  /// \see IsValid
  float Evaluate(wdArrayPtr<Input> inputs = wdArrayPtr<Input>()); // [tested]

private:
  wdHashedString m_sOriginalExpression;
  bool m_bIsValid = false;

  wdExpressionByteCode m_ByteCode;
  wdExpressionVM m_VM;
};
