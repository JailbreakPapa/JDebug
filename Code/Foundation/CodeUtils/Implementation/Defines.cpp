#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace nsTokenParseUtils;

bool nsPreprocessor::RemoveDefine(nsStringView sName)
{
  auto it = m_Macros.Find(sName);

  if (it.IsValid())
  {
    m_Macros.Remove(it);
    return true;
  }

  return false;
}


nsResult nsPreprocessor::StoreDefine(const nsToken* pMacroNameToken, const TokenStream* pReplacementTokens, nsUInt32 uiFirstReplacementToken, nsInt32 iNumParameters, bool bUsesVarArgs)
{
  if ((pMacroNameToken->m_DataView.IsEqual("defined")) || (pMacroNameToken->m_DataView.IsEqual("__FILE__")) || (pMacroNameToken->m_DataView.IsEqual("__LINE__")))
  {
    PP_LOG(Error, "Macro name '{0}' is reserved", pMacroNameToken, pMacroNameToken->m_DataView);
    return NS_FAILURE;
  }

  MacroDefinition md;
  md.m_MacroIdentifier = pMacroNameToken;
  md.m_bIsFunction = iNumParameters >= 0;
  md.m_iNumParameters = nsMath::Max(0, iNumParameters);
  md.m_bHasVarArgs = bUsesVarArgs;

  // removes whitespace at start and end, skips comments, newlines, etc.
  if (pReplacementTokens)
    CopyRelevantTokens(*pReplacementTokens, uiFirstReplacementToken, md.m_Replacement, false);

  if (!md.m_Replacement.IsEmpty() && md.m_Replacement.PeekBack()->m_DataView == "#")
  {
    PP_LOG(Error, "Macro '{0}' ends with invalid character '#'", md.m_Replacement.PeekBack(), pMacroNameToken->m_DataView);
    return NS_FAILURE;
  }

  /* make sure all replacements are not empty
  {
    nsToken Whitespace;
    Whitespace.m_File = pMacroNameToken->m_File;
    Whitespace.m_iType = nsTokenType::Whitespace;
    Whitespace.m_uiColumn = pMacroNameToken->m_uiColumn + sMacroName.GetCharacterCount() + 1;
    Whitespace.m_uiLine = pMacroNameToken->m_uiLine;

    md.m_Replacement.PushBack(AddCustomToken(&Whitespace, ""));
  }*/

  bool bExisted = false;
  auto it = m_Macros.FindOrAdd(pMacroNameToken->m_DataView, &bExisted);

  ProcessingEvent pe;
  pe.m_Type = bExisted ? ProcessingEvent::Redefine : ProcessingEvent::Define;
  pe.m_pToken = pMacroNameToken;
  m_ProcessingEvents.Broadcast(pe);

  if (bExisted)
  {
    PP_LOG(Warning, "Redefinition of macro '{0}'", pMacroNameToken, pMacroNameToken->m_DataView);
    // return NS_FAILURE;
  }

  it.Value() = md;
  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleDefine(const TokenStream& Tokens, nsUInt32& uiCurToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  nsUInt32 uiNameToken = uiCurToken;

  if (Expect(Tokens, uiCurToken, nsTokenType::Identifier, &uiNameToken).Failed())
    return NS_FAILURE;

  // check if we got an empty macro definition
  if (IsEndOfLine(Tokens, uiCurToken, true))
  {
    nsStringBuilder sDefine = Tokens[uiNameToken]->m_DataView;

    return StoreDefine(Tokens[uiNameToken], nullptr, 0, -1, false);
  }

  // first determine whether this is a function macro (before skipping whitespace)
  if (Tokens[uiCurToken]->m_DataView != "(")
  {
    // add the rest of the macro definition as the replacement
    return StoreDefine(Tokens[uiNameToken], &Tokens, uiCurToken, -1, false);
  }

  // extract parameter names
  {
    bool bVarArgsFounds = false;

    // skip the opening parenthesis (
    if (Expect(Tokens, uiCurToken, "(").Failed())
      return NS_FAILURE;

    nsHybridArray<nsString, 16> parameters;

    while (!Accept(Tokens, uiCurToken, ")"))
    {
      if (uiCurToken >= Tokens.GetCount())
      {
        PP_LOG(Error, "Could not extract macro parameter {0}, reached end of token stream first", Tokens[Tokens.GetCount() - 1], parameters.GetCount());
        return NS_FAILURE;
      }

      const nsUInt32 uiCurParamToken = uiCurToken;

      nsString sParam;
      if (ExtractParameterName(Tokens, uiCurToken, sParam) == NS_FAILURE)
      {
        PP_LOG(Error, "Could not extract macro parameter {0}", Tokens[uiCurParamToken], parameters.GetCount());
        return NS_FAILURE;
      }

      if (bVarArgsFounds)
      {
        PP_LOG0(Error, "No additional parameters are allowed after '...'", Tokens[uiCurParamToken]);
        return NS_FAILURE;
      }

      /// \todo Make sure the same parameter name is not used twice

      if (sParam == "...")
      {
        bVarArgsFounds = true;
        sParam = "__VA_ARGS__";
      }

      parameters.PushBack(sParam);
    }

    TokenStream ReplacementTokens;
    CopyTokensReplaceParams(Tokens, uiCurToken, ReplacementTokens, parameters);

    NS_SUCCEED_OR_RETURN(StoreDefine(Tokens[uiNameToken], &ReplacementTokens, 0, parameters.GetCount(), bVarArgsFounds));
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::AddCustomDefine(nsStringView sDefinition)
{
  m_CustomDefines.PushBack();
  m_CustomDefines.PeekBack().m_Content.SetCountUninitialized(sDefinition.GetElementCount());
  nsMemoryUtils::Copy(&m_CustomDefines.PeekBack().m_Content[0], (nsUInt8*)sDefinition.GetStartPointer(), m_CustomDefines.PeekBack().m_Content.GetCount());
  m_CustomDefines.PeekBack().m_Tokenized.Tokenize(m_CustomDefines.PeekBack().m_Content, m_pLog);

  nsUInt32 uiFirstToken = 0;
  nsHybridArray<const nsToken*, 32> Tokens;

  if (m_CustomDefines.PeekBack().m_Tokenized.GetNextLine(uiFirstToken, Tokens).Failed())
    return NS_FAILURE;

  nsDeque<nsToken>& NewTokens = m_CustomDefines.PeekBack().m_Tokenized.GetTokens();

  nsHashedString sFile;
  sFile.Assign("<CustomDefines>");

  nsUInt32 uiColumn = 1;
  for (nsUInt32 t = 0; t < NewTokens.GetCount(); ++t)
  {
    NewTokens[t].m_File = sFile;
    NewTokens[t].m_uiLine = m_CustomDefines.GetCount();
    NewTokens[t].m_uiColumn = uiColumn;

    uiColumn += nsStringUtils::GetCharacterCount(NewTokens[t].m_DataView.GetStartPointer(), NewTokens[t].m_DataView.GetEndPointer());
  }

  nsUInt32 uiCurToken = 0;
  return HandleDefine(Tokens, uiCurToken);
}
