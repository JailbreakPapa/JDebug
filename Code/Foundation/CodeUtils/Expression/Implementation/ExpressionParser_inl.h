
inline bool wdExpressionParser::AcceptStatementTerminator()
{
  return wdTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, wdTokenType::Newline) ||
         wdTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, ";");
}

inline wdResult wdExpressionParser::Expect(const char* szToken, const wdToken** pExpectedToken)
{
  wdUInt32 uiAcceptedToken = 0;
  if (wdTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, szToken, &uiAcceptedToken) == false)
  {
    const wdUInt32 uiErrorToken = wdMath::Min(m_TokenStream.GetCount() - 1, m_uiCurrentToken);
    auto pToken = m_TokenStream[uiErrorToken];
    ReportError(pToken, wdFmt("Syntax error, expected {} but got {}", szToken, pToken->m_DataView));
    return WD_FAILURE;
  }

  if (pExpectedToken != nullptr)
  {
    *pExpectedToken = m_TokenStream[uiAcceptedToken];
  }

  return WD_SUCCESS;
}

inline wdResult wdExpressionParser::Expect(wdTokenType::Enum Type, const wdToken** pExpectedToken /*= nullptr*/)
{
  wdUInt32 uiAcceptedToken = 0;
  if (wdTokenParseUtils::Accept(m_TokenStream, m_uiCurrentToken, Type, &uiAcceptedToken) == false)
  {
    const wdUInt32 uiErrorToken = wdMath::Min(m_TokenStream.GetCount() - 1, m_uiCurrentToken);
    auto pToken = m_TokenStream[uiErrorToken];
    ReportError(pToken, wdFmt("Syntax error, expected token type {} but got {}", wdTokenType::EnumNames[Type], wdTokenType::EnumNames[pToken->m_iType]));
    return WD_FAILURE;
  }

  if (pExpectedToken != nullptr)
  {
    *pExpectedToken = m_TokenStream[uiAcceptedToken];
  }

  return WD_SUCCESS;
}

inline void wdExpressionParser::ReportError(const wdToken* pToken, const wdFormatString& message0)
{
  wdStringBuilder tmp;
  wdStringView message = message0.GetText(tmp);
  wdLog::Error("{}({},{}): {}", pToken->m_File, pToken->m_uiLine, pToken->m_uiColumn, message);
}
