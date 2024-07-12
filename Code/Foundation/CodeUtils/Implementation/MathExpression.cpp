#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Expression/ExpressionCompiler.h>
#include <Foundation/CodeUtils/Expression/ExpressionParser.h>
#include <Foundation/CodeUtils/MathExpression.h>

static nsHashedString s_sOutput = nsMakeHashedString("output");

nsMathExpression::nsMathExpression() = default;

nsMathExpression::nsMathExpression(nsStringView sExpressionString)
{
  Reset(sExpressionString);
}

void nsMathExpression::Reset(nsStringView sExpressionString)
{
  m_sOriginalExpression.Assign(sExpressionString);
  m_ByteCode.Clear();
  m_bIsValid = false;

  if (sExpressionString.IsEmpty())
    return;

  nsStringBuilder tmp = s_sOutput.GetView();
  tmp.Append(" = ", sExpressionString);

  nsExpression::StreamDesc outputs[] = {
    {s_sOutput, nsProcessingStream::DataType::Float},
  };

  nsExpressionParser parser;
  nsExpressionParser::Options parserOptions;
  parserOptions.m_bTreatUnknownVariablesAsInputs = true;

  nsExpressionAST ast;
  if (parser.Parse(tmp, nsArrayPtr<nsExpression::StreamDesc>(), outputs, parserOptions, ast).Failed())
    return;

  nsExpressionCompiler compiler;
  if (compiler.Compile(ast, m_ByteCode).Failed())
    return;

  m_bIsValid = true;
}

float nsMathExpression::Evaluate(nsArrayPtr<Input> inputs)
{
  float fOutput = nsMath::NaN<float>();

  if (!IsValid() || m_ByteCode.IsEmpty())
  {
    nsLog::Error("Can't evaluate invalid math expression '{0}'", m_sOriginalExpression);
    return fOutput;
  }

  nsHybridArray<nsProcessingStream, 8> inputStreams;
  for (auto& input : inputs)
  {
    if (input.m_sName.IsEmpty())
      continue;

    inputStreams.PushBack(nsProcessingStream(input.m_sName, nsMakeArrayPtr(&input.m_fValue, 1).ToByteArray(), nsProcessingStream::DataType::Float));
  }

  nsProcessingStream outputStream(s_sOutput, nsMakeArrayPtr(&fOutput, 1).ToByteArray(), nsProcessingStream::DataType::Float);
  nsArrayPtr<nsProcessingStream> outputStreams = nsMakeArrayPtr(&outputStream, 1);

  if (m_VM.Execute(m_ByteCode, inputStreams, outputStreams, 1).Failed())
  {
    nsLog::Error("Failed to execute expression VM");
  }

  return fOutput;
}
