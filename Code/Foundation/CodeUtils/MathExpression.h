#pragma once

#include <Foundation/CodeUtils/Expression/ExpressionByteCode.h>
#include <Foundation/CodeUtils/Expression/ExpressionVM.h>
#include <Foundation/Strings/String.h>

class nsLogInterface;

/// \brief A wrapper around nsExpression infrastructure to evaluate simple math expressions
class NS_FOUNDATION_DLL nsMathExpression
{
public:
  /// \brief Creates a new invalid math expression.
  ///
  /// Need to call Reset before you can do anything with it.
  nsMathExpression();

  /// \brief Initializes using a given expression.
  ///
  /// If anything goes wrong it is logged and the math expression is in an invalid state.
  /// \param log
  ///   If null, default log interface will be used.
  explicit nsMathExpression(nsStringView sExpressionString); // [tested]

  /// \brief Reinitializes using the given expression.
  ///
  /// An empty string or nullptr are considered to be 'invalid' expressions.
  void Reset(nsStringView sExpressionString);

  /// Whether the expression is valid and can be evaluated.
  bool IsValid() const { return m_bIsValid; }

  /// Returns the original expression string that this MathExpression can evaluate.
  nsStringView GetExpressionString() const { return m_sOriginalExpression; }

  struct Input
  {
    nsHashedString m_sName;
    float m_fValue;
  };

  /// \brief Evaluates parsed expression with the given inputs.
  ///
  /// Only way this function can fail is if the expression was not valid.
  /// \see IsValid
  float Evaluate(nsArrayPtr<Input> inputs = nsArrayPtr<Input>()); // [tested]

private:
  nsHashedString m_sOriginalExpression;
  bool m_bIsValid = false;

  nsExpressionByteCode m_ByteCode;
  nsExpressionVM m_VM;
};
