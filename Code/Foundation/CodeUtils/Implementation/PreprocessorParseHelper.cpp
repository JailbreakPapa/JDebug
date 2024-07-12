#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace nsTokenParseUtils;

nsResult nsPreprocessor::Expect(const TokenStream& Tokens, nsUInt32& uiCurToken, nsStringView sToken, nsUInt32* pAccepted)
{
  if (Tokens.GetCount() < 1)
  {
    nsLog::Error(m_pLog, "Expected token '{0}', got empty token stream", sToken);
    return NS_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, sToken, pAccepted))
    return NS_SUCCESS;

  const nsUInt32 uiErrorToken = nsMath::Min(Tokens.GetCount() - 1, uiCurToken);
  nsString sErrorToken = Tokens[uiErrorToken]->m_DataView;
  PP_LOG(Error, "Expected token '{0}' got '{1}'", Tokens[uiErrorToken], sToken, sErrorToken);

  return NS_FAILURE;
}

nsResult nsPreprocessor::Expect(const TokenStream& Tokens, nsUInt32& uiCurToken, nsTokenType::Enum Type, nsUInt32* pAccepted)
{
  if (Tokens.GetCount() < 1)
  {
    nsLog::Error(m_pLog, "Expected token of type '{0}', got empty token stream", nsTokenType::EnumNames[Type]);
    return NS_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, Type, pAccepted))
    return NS_SUCCESS;

  const nsUInt32 uiErrorToken = nsMath::Min(Tokens.GetCount() - 1, uiCurToken);
  PP_LOG(Error, "Expected token of type '{0}' got type '{1}' instead", Tokens[uiErrorToken], nsTokenType::EnumNames[Type], nsTokenType::EnumNames[Tokens[uiErrorToken]->m_iType]);

  return NS_FAILURE;
}

nsResult nsPreprocessor::Expect(const TokenStream& Tokens, nsUInt32& uiCurToken, nsStringView sToken1, nsStringView sToken2, nsUInt32* pAccepted)
{
  if (Tokens.GetCount() < 2)
  {
    nsLog::Error(m_pLog, "Expected tokens '{0}{1}', got empty token stream", sToken1, sToken2);
    return NS_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, sToken1, sToken2, pAccepted))
    return NS_SUCCESS;

  const nsUInt32 uiErrorToken = nsMath::Min(Tokens.GetCount() - 2, uiCurToken);
  nsString sErrorToken1 = Tokens[uiErrorToken]->m_DataView;
  nsString sErrorToken2 = Tokens[uiErrorToken + 1]->m_DataView;
  PP_LOG(Error, "Expected tokens '{0}{1}', got '{2}{3}'", Tokens[uiErrorToken], sToken1, sToken2, sErrorToken1, sErrorToken2);

  return NS_FAILURE;
}

nsResult nsPreprocessor::ExpectEndOfLine(const TokenStream& Tokens, nsUInt32 uiCurToken)
{
  if (!IsEndOfLine(Tokens, uiCurToken, true))
  {
    nsString sToken = Tokens[uiCurToken]->m_DataView;
    PP_LOG(Warning, "Expected end-of-line, found token '{0}'", Tokens[uiCurToken], sToken);
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}
