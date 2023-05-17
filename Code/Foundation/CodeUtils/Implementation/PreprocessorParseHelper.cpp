#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace wdTokenParseUtils;

wdResult wdPreprocessor::Expect(const TokenStream& Tokens, wdUInt32& uiCurToken, const char* szToken, wdUInt32* pAccepted)
{
  if (Tokens.GetCount() < 1)
  {
    wdLog::Error(m_pLog, "Expected token '{0}', got empty token stream", szToken);
    return WD_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, szToken, pAccepted))
    return WD_SUCCESS;

  const wdUInt32 uiErrorToken = wdMath::Min(Tokens.GetCount() - 1, uiCurToken);
  wdString sErrorToken = Tokens[uiErrorToken]->m_DataView;
  PP_LOG(Error, "Expected token '{0}' got '{1}'", Tokens[uiErrorToken], szToken, sErrorToken);

  return WD_FAILURE;
}

wdResult wdPreprocessor::Expect(const TokenStream& Tokens, wdUInt32& uiCurToken, wdTokenType::Enum Type, wdUInt32* pAccepted)
{
  if (Tokens.GetCount() < 1)
  {
    wdLog::Error(m_pLog, "Expected token of type '{0}', got empty token stream", wdTokenType::EnumNames[Type]);
    return WD_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, Type, pAccepted))
    return WD_SUCCESS;

  const wdUInt32 uiErrorToken = wdMath::Min(Tokens.GetCount() - 1, uiCurToken);
  PP_LOG(Error, "Expected token of type '{0}' got type '{1}' instead", Tokens[uiErrorToken], wdTokenType::EnumNames[Type], wdTokenType::EnumNames[Tokens[uiErrorToken]->m_iType]);

  return WD_FAILURE;
}

wdResult wdPreprocessor::Expect(const TokenStream& Tokens, wdUInt32& uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted)
{
  if (Tokens.GetCount() < 2)
  {
    wdLog::Error(m_pLog, "Expected tokens '{0}{1}', got empty token stream", szToken1, szToken2);
    return WD_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, szToken1, szToken2, pAccepted))
    return WD_SUCCESS;

  const wdUInt32 uiErrorToken = wdMath::Min(Tokens.GetCount() - 2, uiCurToken);
  wdString sErrorToken1 = Tokens[uiErrorToken]->m_DataView;
  wdString sErrorToken2 = Tokens[uiErrorToken + 1]->m_DataView;
  PP_LOG(Error, "Expected tokens '{0}{1}', got '{2}{3}'", Tokens[uiErrorToken], szToken1, szToken2, sErrorToken1, sErrorToken2);

  return WD_FAILURE;
}

wdResult wdPreprocessor::ExpectEndOfLine(const TokenStream& Tokens, wdUInt32 uiCurToken)
{
  if (!IsEndOfLine(Tokens, uiCurToken, true))
  {
    wdString sToken = Tokens[uiCurToken]->m_DataView;
    PP_LOG(Warning, "Expected end-of-line, found token '{0}'", Tokens[uiCurToken], sToken);
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_PreprocessorParseHelper);
