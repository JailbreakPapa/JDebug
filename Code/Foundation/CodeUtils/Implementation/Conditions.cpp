#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace wdTokenParseUtils;

wdResult wdPreprocessor::CopyTokensAndEvaluateDefined(const TokenStream& Source, wdUInt32 uiFirstSourceToken, TokenStream& Destination)
{
  Destination.Clear();
  Destination.Reserve(Source.GetCount() - uiFirstSourceToken);

  {
    // skip all whitespace at the start of the replacement string
    wdUInt32 uiCurToken = uiFirstSourceToken;
    SkipWhitespace(Source, uiCurToken);

    // add all the relevant tokens to the definition
    while (uiCurToken < Source.GetCount())
    {
      if (Source[uiCurToken]->m_iType == wdTokenType::BlockComment || Source[uiCurToken]->m_iType == wdTokenType::LineComment || Source[uiCurToken]->m_iType == wdTokenType::EndOfFile || Source[uiCurToken]->m_iType == wdTokenType::Newline)
      {
        ++uiCurToken;
        continue;
      }

      if (Source[uiCurToken]->m_DataView.IsEqual("defined"))
      {
        ++uiCurToken;

        const bool bParenthesis = Accept(Source, uiCurToken, "(");

        wdUInt32 uiIdentifier = uiCurToken;
        if (Expect(Source, uiCurToken, wdTokenType::Identifier, &uiIdentifier).Failed())
          return WD_FAILURE;

        wdToken* pReplacement = nullptr;

        const bool bDefined = m_Macros.Find(Source[uiIdentifier]->m_DataView).IsValid();

        // broadcast that 'defined' is being evaluated
        {
          ProcessingEvent pe;
          pe.m_pToken = Source[uiIdentifier];
          pe.m_Type = ProcessingEvent::CheckDefined;
          pe.m_szInfo = bDefined ? "defined" : "undefined";
          m_ProcessingEvents.Broadcast(pe);
        }

        pReplacement = AddCustomToken(Source[uiIdentifier], bDefined ? "1" : "0");

        Destination.PushBack(pReplacement);

        if (bParenthesis)
        {
          if (Expect(Source, uiCurToken, ")").Failed())
            return WD_FAILURE;
        }
      }
      else
      {
        Destination.PushBack(Source[uiCurToken]);
        ++uiCurToken;
      }
    }
  }

  // remove whitespace at end of macro
  while (!Destination.IsEmpty() && Destination.PeekBack()->m_iType == wdTokenType::Whitespace)
    Destination.PopBack();

  return WD_SUCCESS;
}

wdResult wdPreprocessor::EvaluateCondition(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  iResult = 0;

  TokenStream Copied(&m_ClassAllocator);
  if (CopyTokensAndEvaluateDefined(Tokens, uiCurToken, Copied).Failed())
    return WD_FAILURE;

  TokenStream Expanded(&m_ClassAllocator);

  if (Expand(Copied, Expanded).Failed())
    return WD_FAILURE;

  if (Expanded.IsEmpty())
  {
    PP_LOG0(Error, "After expansion the condition is empty", Tokens[uiCurToken]);
    return WD_FAILURE;
  }

  wdUInt32 uiCurToken2 = 0;
  if (ParseExpressionOr(Expanded, uiCurToken2, iResult).Failed())
    return WD_FAILURE;

  return ExpectEndOfLine(Expanded, uiCurToken2);
}

wdResult wdPreprocessor::ParseFactor(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  while (Accept(Tokens, uiCurToken, "+"))
  {
  }

  if (Accept(Tokens, uiCurToken, "-"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return WD_FAILURE;

    iResult = -iResult;
    return WD_SUCCESS;
  }

  if (Accept(Tokens, uiCurToken, "~"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return WD_FAILURE;

    iResult = ~iResult;
    return WD_SUCCESS;
  }

  if (Accept(Tokens, uiCurToken, "!"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return WD_FAILURE;

    iResult = (iResult != 0) ? 0 : 1;
    return WD_SUCCESS;
  }

  wdUInt32 uiValueToken = uiCurToken;
  if (Accept(Tokens, uiCurToken, wdTokenType::Identifier, &uiValueToken) || Accept(Tokens, uiCurToken, wdTokenType::Integer, &uiValueToken))
  {
    const wdString sVal = Tokens[uiValueToken]->m_DataView;

    wdInt32 iResult32 = 0;

    if (sVal == "true")
    {
      iResult32 = 1;
    }
    else if (sVal == "false")
    {
      iResult32 = 0;
    }
    else if (wdConversionUtils::StringToInt(sVal, iResult32).Failed())
    {
      // this is not an error, all unknown identifiers are assumed to be zero

      // broadcast that we encountered this unknown identifier
      ProcessingEvent pe;
      pe.m_pToken = Tokens[uiValueToken];
      pe.m_Type = ProcessingEvent::EvaluateUnknown;
      m_ProcessingEvents.Broadcast(pe);
    }

    iResult = (wdInt64)iResult32;

    return WD_SUCCESS;
  }
  else if (Accept(Tokens, uiCurToken, "("))
  {
    if (ParseExpressionOr(Tokens, uiCurToken, iResult).Failed())
      return WD_FAILURE;

    return Expect(Tokens, uiCurToken, ")");
  }

  uiCurToken = wdMath::Min(uiCurToken, Tokens.GetCount() - 1);
  PP_LOG0(Error, "Syntax error, expected identifier, number or '('", Tokens[uiCurToken]);

  return WD_FAILURE;
}

wdResult wdPreprocessor::ParseExpressionPlus(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionMul(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, "+"))
    {
      wdInt64 iNextValue = 0;
      if (ParseExpressionMul(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult += iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "-"))
    {
      wdInt64 iNextValue = 0;
      if (ParseExpressionMul(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult -= iNextValue;
    }
    else
      break;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionShift(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionPlus(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, ">", ">"))
    {
      wdInt64 iNextValue = 0;
      if (ParseExpressionPlus(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult >>= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "<", "<"))
    {
      wdInt64 iNextValue = 0;
      if (ParseExpressionPlus(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult <<= iNextValue;
    }
    else
      break;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionOr(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionAnd(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (Accept(Tokens, uiCurToken, "|", "|"))
  {
    wdInt64 iNextValue = 0;
    if (ParseExpressionAnd(Tokens, uiCurToken, iNextValue).Failed())
      return WD_FAILURE;

    iResult = (iResult != 0 || iNextValue != 0) ? 1 : 0;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionAnd(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionBitOr(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (Accept(Tokens, uiCurToken, "&", "&"))
  {
    wdInt64 iNextValue = 0;
    if (ParseExpressionBitOr(Tokens, uiCurToken, iNextValue).Failed())
      return WD_FAILURE;

    iResult = (iResult != 0 && iNextValue != 0) ? 1 : 0;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionBitOr(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionBitXor(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (AcceptUnless(Tokens, uiCurToken, "|", "|"))
  {
    wdInt64 iNextValue = 0;
    if (ParseExpressionBitXor(Tokens, uiCurToken, iNextValue).Failed())
      return WD_FAILURE;

    iResult |= iNextValue;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionBitAnd(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseCondition(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (AcceptUnless(Tokens, uiCurToken, "&", "&"))
  {
    wdInt64 iNextValue = 0;
    if (ParseCondition(Tokens, uiCurToken, iNextValue).Failed())
      return WD_FAILURE;

    iResult &= iNextValue;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ParseExpressionBitXor(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseExpressionBitAnd(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (Accept(Tokens, uiCurToken, "^"))
  {
    wdInt64 iNextValue = 0;
    if (ParseExpressionBitAnd(Tokens, uiCurToken, iNextValue).Failed())
      return WD_FAILURE;

    iResult ^= iNextValue;
  }

  return WD_SUCCESS;
}
wdResult wdPreprocessor::ParseExpressionMul(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, "*"))
    {
      wdInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult *= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "/"))
    {
      wdInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult /= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "%"))
    {
      wdInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return WD_FAILURE;

      iResult %= iNextValue;
    }
    else
      break;
  }

  return WD_SUCCESS;
}

enum class Comparison
{
  None,
  Equal,
  Unequal,
  LessThan,
  GreaterThan,
  LessThanEqual,
  GreaterThanEqual
};

wdResult wdPreprocessor::ParseCondition(const TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult)
{
  wdInt64 iResult1 = 0;
  if (ParseExpressionShift(Tokens, uiCurToken, iResult1).Failed())
    return WD_FAILURE;

  Comparison Operator = Comparison::None;

  if (Accept(Tokens, uiCurToken, "=", "="))
    Operator = Comparison::Equal;
  else if (Accept(Tokens, uiCurToken, "!", "="))
    Operator = Comparison::Unequal;
  else if (Accept(Tokens, uiCurToken, ">", "="))
    Operator = Comparison::GreaterThanEqual;
  else if (Accept(Tokens, uiCurToken, "<", "="))
    Operator = Comparison::LessThanEqual;
  else if (AcceptUnless(Tokens, uiCurToken, ">", ">"))
    Operator = Comparison::GreaterThan;
  else if (AcceptUnless(Tokens, uiCurToken, "<", "<"))
    Operator = Comparison::LessThan;
  else
  {
    iResult = iResult1;
    return WD_SUCCESS;
  }

  wdInt64 iResult2 = 0;
  if (ParseExpressionShift(Tokens, uiCurToken, iResult2).Failed())
    return WD_FAILURE;

  switch (Operator)
  {
    case Comparison::Equal:
      iResult = (iResult1 == iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::GreaterThan:
      iResult = (iResult1 > iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::GreaterThanEqual:
      iResult = (iResult1 >= iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::LessThan:
      iResult = (iResult1 < iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::LessThanEqual:
      iResult = (iResult1 <= iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::Unequal:
      iResult = (iResult1 != iResult2) ? 1 : 0;
      return WD_SUCCESS;
    case Comparison::None:
      wdLog::Error(m_pLog, "Unknown operator");
      return WD_FAILURE;
  }

  return WD_FAILURE;
}



WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_Conditions);
