
inline bool nsExpressionParser::AcceptStatementTerminator()
{
  return nsTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, nsTokenType::Newline) ||
         nsTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, ";");
}

inline nsResult nsExpressionParser::Expect(nsStringView sToken, const nsToken** pExpectedToken)
{
  nsUInt32 uiAcceptedToken = 0;
  if (nsTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, sToken, &uiAcceptedToken) == false)
  {
    const nsUInt32 uiErrorToken = nsMath::Min(m_TokenStream.GetCount() - 1, m_uiCurrentToken);
    auto pToken = m_TokenStream[uiErrorToken];
    ReportError(pToken, nsFmt("Syntax error, expected {} but got {}", sToken, pToken->m_DataView));
    return NS_FAILURE;
  }

  if (pExpectedToken != nullptr)
  {
    *pExpectedToken = m_TokenStream[uiAcceptedToken];
  }

  return NS_SUCCESS;
}

inline nsResult nsExpressionParser::Expect(nsTokenType::Enum Type, const nsToken** pExpectedToken /*= nullptr*/)
{
  nsUInt32 uiAcceptedToken = 0;
  if (nsTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, Type, &uiAcceptedToken) == false)
  {
    const nsUInt32 uiErrorToken = nsMath::Min(m_TokenStream.GetCount() - 1, m_uiCurrentToken);
    auto pToken = m_TokenStream[uiErrorToken];
    ReportError(pToken, nsFmt("Syntax error, expected token type {} but got {}", nsTokenType::EnumNames[Type], nsTokenType::EnumNames[pToken->m_iType]));
    return NS_FAILURE;
  }

  if (pExpectedToken != nullptr)
  {
    *pExpectedToken = m_TokenStream[uiAcceptedToken];
  }

  return NS_SUCCESS;
}

inline void nsExpressionParser::ReportError(const nsToken* pToken, const nsFormatString& message0)
{
  nsStringBuilder tmp;
  nsStringView message = message0.GetText(tmp);
  nsLog::Error("{}({},{}): {}", pToken->m_File, pToken->m_uiLine, pToken->m_uiColumn, message);
}
