#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace nsTokenParseUtils;

nsResult nsPreprocessor::CopyTokensAndEvaluateDefined(const TokenStream& Source, nsUInt32 uiFirstSourceToken, TokenStream& Destination)
{
  Destination.Clear();
  Destination.Reserve(Source.GetCount() - uiFirstSourceToken);

  {
    // skip all whitespace at the start of the replacement string
    nsUInt32 uiCurToken = uiFirstSourceToken;
    SkipWhitespace(Source, uiCurToken);

    // add all the relevant tokens to the definition
    while (uiCurToken < Source.GetCount())
    {
      if (Source[uiCurToken]->m_iType == nsTokenType::BlockComment || Source[uiCurToken]->m_iType == nsTokenType::LineComment || Source[uiCurToken]->m_iType == nsTokenType::EndOfFile || Source[uiCurToken]->m_iType == nsTokenType::Newline)
      {
        ++uiCurToken;
        continue;
      }

      if (Source[uiCurToken]->m_DataView.IsEqual("defined"))
      {
        ++uiCurToken;

        const bool bParenthesis = Accept(Source, uiCurToken, "(");

        nsUInt32 uiIdentifier = uiCurToken;
        if (Expect(Source, uiCurToken, nsTokenType::Identifier, &uiIdentifier).Failed())
          return NS_FAILURE;

        nsToken* pReplacement = nullptr;

        const bool bDefined = m_Macros.Find(Source[uiIdentifier]->m_DataView).IsValid();

        // broadcast that 'defined' is being evaluated
        {
          ProcessingEvent pe;
          pe.m_pToken = Source[uiIdentifier];
          pe.m_Type = ProcessingEvent::CheckDefined;
          pe.m_sInfo = bDefined ? "defined" : "undefined";
          m_ProcessingEvents.Broadcast(pe);
        }

        pReplacement = AddCustomToken(Source[uiIdentifier], bDefined ? "1" : "0");

        Destination.PushBack(pReplacement);

        if (bParenthesis)
        {
          if (Expect(Source, uiCurToken, ")").Failed())
            return NS_FAILURE;
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
  while (!Destination.IsEmpty() && Destination.PeekBack()->m_iType == nsTokenType::Whitespace)
    Destination.PopBack();

  return NS_SUCCESS;
}

nsResult nsPreprocessor::EvaluateCondition(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  iResult = 0;

  TokenStream Copied(&m_ClassAllocator);
  if (CopyTokensAndEvaluateDefined(Tokens, uiCurToken, Copied).Failed())
    return NS_FAILURE;

  TokenStream Expanded(&m_ClassAllocator);

  if (Expand(Copied, Expanded).Failed())
    return NS_FAILURE;

  if (Expanded.IsEmpty())
  {
    PP_LOG0(Error, "After expansion the condition is empty", Tokens[uiCurToken]);
    return NS_FAILURE;
  }

  nsUInt32 uiCurToken2 = 0;
  if (ParseExpressionOr(Expanded, uiCurToken2, iResult).Failed())
    return NS_FAILURE;

  return ExpectEndOfLine(Expanded, uiCurToken2);
}

nsResult nsPreprocessor::ParseFactor(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  while (Accept(Tokens, uiCurToken, "+"))
  {
  }

  if (Accept(Tokens, uiCurToken, "-"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return NS_FAILURE;

    iResult = -iResult;
    return NS_SUCCESS;
  }

  if (Accept(Tokens, uiCurToken, "~"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return NS_FAILURE;

    iResult = ~iResult;
    return NS_SUCCESS;
  }

  if (Accept(Tokens, uiCurToken, "!"))
  {
    if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
      return NS_FAILURE;

    iResult = (iResult != 0) ? 0 : 1;
    return NS_SUCCESS;
  }

  nsUInt32 uiValueToken = uiCurToken;
  if (Accept(Tokens, uiCurToken, nsTokenType::Identifier, &uiValueToken) || Accept(Tokens, uiCurToken, nsTokenType::Integer, &uiValueToken))
  {
    const nsString sVal = Tokens[uiValueToken]->m_DataView;

    nsInt32 iResult32 = 0;

    if (sVal == "true")
    {
      iResult32 = 1;
    }
    else if (sVal == "false")
    {
      iResult32 = 0;
    }
    else if (nsConversionUtils::StringToInt(sVal, iResult32).Failed())
    {
      // this is not an error, all unknown identifiers are assumed to be zero

      // broadcast that we encountered this unknown identifier
      ProcessingEvent pe;
      pe.m_pToken = Tokens[uiValueToken];
      pe.m_Type = ProcessingEvent::EvaluateUnknown;
      m_ProcessingEvents.Broadcast(pe);
    }

    iResult = (nsInt64)iResult32;

    return NS_SUCCESS;
  }
  else if (Accept(Tokens, uiCurToken, "("))
  {
    if (ParseExpressionOr(Tokens, uiCurToken, iResult).Failed())
      return NS_FAILURE;

    return Expect(Tokens, uiCurToken, ")");
  }

  uiCurToken = nsMath::Min(uiCurToken, Tokens.GetCount() - 1);
  PP_LOG0(Error, "Syntax error, expected identifier, number or '('", Tokens[uiCurToken]);

  return NS_FAILURE;
}

nsResult nsPreprocessor::ParseExpressionPlus(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionMul(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, "+"))
    {
      nsInt64 iNextValue = 0;
      if (ParseExpressionMul(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult += iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "-"))
    {
      nsInt64 iNextValue = 0;
      if (ParseExpressionMul(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult -= iNextValue;
    }
    else
      break;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionShift(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionPlus(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, ">", ">"))
    {
      nsInt64 iNextValue = 0;
      if (ParseExpressionPlus(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult >>= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "<", "<"))
    {
      nsInt64 iNextValue = 0;
      if (ParseExpressionPlus(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult <<= iNextValue;
    }
    else
      break;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionOr(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionAnd(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (Accept(Tokens, uiCurToken, "|", "|"))
  {
    nsInt64 iNextValue = 0;
    if (ParseExpressionAnd(Tokens, uiCurToken, iNextValue).Failed())
      return NS_FAILURE;

    iResult = (iResult != 0 || iNextValue != 0) ? 1 : 0;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionAnd(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionBitOr(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (Accept(Tokens, uiCurToken, "&", "&"))
  {
    nsInt64 iNextValue = 0;
    if (ParseExpressionBitOr(Tokens, uiCurToken, iNextValue).Failed())
      return NS_FAILURE;

    iResult = (iResult != 0 && iNextValue != 0) ? 1 : 0;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionBitOr(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionBitXor(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (AcceptUnless(Tokens, uiCurToken, "|", "|"))
  {
    nsInt64 iNextValue = 0;
    if (ParseExpressionBitXor(Tokens, uiCurToken, iNextValue).Failed())
      return NS_FAILURE;

    iResult |= iNextValue;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionBitAnd(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseCondition(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (AcceptUnless(Tokens, uiCurToken, "&", "&"))
  {
    nsInt64 iNextValue = 0;
    if (ParseCondition(Tokens, uiCurToken, iNextValue).Failed())
      return NS_FAILURE;

    iResult &= iNextValue;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ParseExpressionBitXor(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseExpressionBitAnd(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (Accept(Tokens, uiCurToken, "^"))
  {
    nsInt64 iNextValue = 0;
    if (ParseExpressionBitAnd(Tokens, uiCurToken, iNextValue).Failed())
      return NS_FAILURE;

    iResult ^= iNextValue;
  }

  return NS_SUCCESS;
}
nsResult nsPreprocessor::ParseExpressionMul(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  if (ParseFactor(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  while (true)
  {
    if (Accept(Tokens, uiCurToken, "*"))
    {
      nsInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult *= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "/"))
    {
      nsInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult /= iNextValue;
    }
    else if (Accept(Tokens, uiCurToken, "%"))
    {
      nsInt64 iNextValue = 0;
      if (ParseFactor(Tokens, uiCurToken, iNextValue).Failed())
        return NS_FAILURE;

      iResult %= iNextValue;
    }
    else
      break;
  }

  return NS_SUCCESS;
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

nsResult nsPreprocessor::ParseCondition(const TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult)
{
  nsInt64 iResult1 = 0;
  if (ParseExpressionShift(Tokens, uiCurToken, iResult1).Failed())
    return NS_FAILURE;

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
    return NS_SUCCESS;
  }

  nsInt64 iResult2 = 0;
  if (ParseExpressionShift(Tokens, uiCurToken, iResult2).Failed())
    return NS_FAILURE;

  switch (Operator)
  {
    case Comparison::Equal:
      iResult = (iResult1 == iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::GreaterThan:
      iResult = (iResult1 > iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::GreaterThanEqual:
      iResult = (iResult1 >= iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::LessThan:
      iResult = (iResult1 < iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::LessThanEqual:
      iResult = (iResult1 <= iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::Unequal:
      iResult = (iResult1 != iResult2) ? 1 : 0;
      return NS_SUCCESS;
    case Comparison::None:
      nsLog::Error(m_pLog, "Unknown operator");
      return NS_FAILURE;
  }

  return NS_FAILURE;
}
