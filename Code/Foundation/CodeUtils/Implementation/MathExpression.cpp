#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/MathExpression.h>

static wdHashedString s_sOutput = wdMakeHashedString("output");

wdMathExpression::wdMathExpression() = default;

wdMathExpression::wdMathExpression(wdStringView sExpressionString)
{
  Reset(sExpressionString);
}

void wdMathExpression::Reset(wdStringView sExpressionString)
{
  m_sOriginalExpression.Assign(sExpressionString);
  m_ByteCode.Clear();
  m_bIsValid = false;

  if (sExpressionString.IsEmpty())
    return;

  wdStringBuilder tmp = s_sOutput.GetView();
  tmp.Append(" = ", sExpressionString);

  wdExpression::StreamDesc outputs[] = {
    {s_sOutput, wdProcessingStream::DataType::Float},
  };

  wdExpressionParser parser;
  wdExpressionParser::Options parserOptions;
  parserOptions.m_bTreatUnknownVariablesAsInputs = true;

  wdExpressionAST ast;
  if (parser.Parse(tmp, wdArrayPtr<wdExpression::StreamDesc>(), outputs, parserOptions, ast).Failed())
    return;

  wdExpressionCompiler compiler;
  if (compiler.Compile(ast, m_ByteCode).Failed())
    return;

  m_bIsValid = true;
}

float wdMathExpression::Evaluate(wdArrayPtr<Input> inputs)
{
  float fOutput = wdMath::NaN<float>();

  if (!IsValid() || m_ByteCode.IsEmpty())
  {
    wdLog::Error("Can't evaluate invalid math expression '{0}'", m_sOriginalExpression);
    return fOutput;
  }

  wdHybridArray<wdProcessingStream, 8> inputStreams;
  for (auto& input : inputs)
  {
    if (input.m_sName.IsEmpty())
      continue;

    inputStreams.PushBack(wdProcessingStream(input.m_sName, wdMakeArrayPtr(&input.m_fValue, 1).ToByteArray(), wdProcessingStream::DataType::Float));
  }

  wdProcessingStream outputStream(s_sOutput, wdMakeArrayPtr(&fOutput, 1).ToByteArray(), wdProcessingStream::DataType::Float);
  wdArrayPtr<wdProcessingStream> outputStreams = wdMakeArrayPtr(&outputStream, 1);

  if (m_VM.Execute(m_ByteCode, inputStreams, outputStreams, 1).Failed())
  {
    wdLog::Error("Failed to execute expression VM");
  }

  return fOutput;
}

WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_MathExpression);
