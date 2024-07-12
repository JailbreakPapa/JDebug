#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace nsTokenParseUtils;

nsPreprocessor::MacroDefinition::MacroDefinition()
{
  m_MacroIdentifier = nullptr;
  m_bIsFunction = false;
  m_bCurrentlyExpanding = false;
  m_iNumParameters = -1;
  m_bHasVarArgs = false;
}

void nsPreprocessor::CopyTokensReplaceParams(const TokenStream& Source, nsUInt32 uiFirstSourceToken, TokenStream& Destination, const nsHybridArray<nsString, 16>& parameters)
{
  Destination.Clear();
  Destination.Reserve(Source.GetCount() - uiFirstSourceToken);

  {
    // skip all whitespace at the start of the replacement string
    nsUInt32 i = uiFirstSourceToken;
    SkipWhitespace(Source, i);

    // add all the relevant tokens to the definition
    for (; i < Source.GetCount(); ++i)
    {
      if (Source[i]->m_iType == nsTokenType::BlockComment || Source[i]->m_iType == nsTokenType::LineComment || Source[i]->m_iType == nsTokenType::EndOfFile || Source[i]->m_iType == nsTokenType::Newline)
        continue;

      if (Source[i]->m_iType == nsTokenType::Identifier)
      {
        for (nsUInt32 p = 0; p < parameters.GetCount(); ++p)
        {
          if (Source[i]->m_DataView == parameters[p])
          {
            // create a custom token for the parameter, for better error messages
            nsToken* pParamToken = AddCustomToken(Source[i], parameters[p]);
            pParamToken->m_iType = s_iMacroParameter0 + p;

            Destination.PushBack(pParamToken);
            goto tokenfound;
          }
        }
      }

      Destination.PushBack(Source[i]);

    tokenfound:
      continue;
    }
  }

  // remove whitespace at end of macro
  while (!Destination.IsEmpty() && Destination.PeekBack()->m_iType == nsTokenType::Whitespace)
    Destination.PopBack();
}

nsResult nsPreprocessor::ExtractParameterName(const TokenStream& Tokens, nsUInt32& uiCurToken, nsString& sIdentifierName)
{
  SkipWhitespace(Tokens, uiCurToken);

  if (uiCurToken + 2 < Tokens.GetCount() && Tokens[uiCurToken + 0]->m_DataView == "." && Tokens[uiCurToken + 1]->m_DataView == "." && Tokens[uiCurToken + 2]->m_DataView == ".")
  {
    sIdentifierName = "...";
    uiCurToken += 3;
  }
  else
  {
    nsUInt32 uiParamToken = uiCurToken;

    if (Expect(Tokens, uiCurToken, nsTokenType::Identifier, &uiParamToken).Failed())
      return NS_FAILURE;

    sIdentifierName = Tokens[uiParamToken]->m_DataView;
  }

  // skip a trailing comma
  if (Accept(Tokens, uiCurToken, ","))
    SkipWhitespace(Tokens, uiCurToken);

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ExtractAllMacroParameters(const TokenStream& Tokens, nsUInt32& uiCurToken, nsDeque<TokenStream>& AllParameters)
{
  if (Expect(Tokens, uiCurToken, "(").Failed())
    return NS_FAILURE;

  do
  {
    // add one parameter
    // note: we always add one extra parameter value, because MACRO() is actually a macro call with one empty parameter
    // the same for MACRO(a,) is a macro with two parameters, the second being empty
    AllParameters.SetCount(AllParameters.GetCount() + 1);

    if (ExtractParameterValue(Tokens, uiCurToken, AllParameters.PeekBack()).Failed())
      return NS_FAILURE;

    // reached the end of the parameter list
    if (Accept(Tokens, uiCurToken, ")"))
      return NS_SUCCESS;
  } while (Accept(Tokens, uiCurToken, ",")); // continue with the next parameter

  nsString s = Tokens[uiCurToken]->m_DataView;
  PP_LOG(Error, "',' or ')' expected, got '{0}' instead", Tokens[uiCurToken], s);

  return NS_FAILURE;
}

nsResult nsPreprocessor::ExtractParameterValue(const TokenStream& Tokens, nsUInt32& uiCurToken, TokenStream& ParamTokens)
{
  SkipWhitespaceAndNewline(Tokens, uiCurToken);
  const nsUInt32 uiFirstToken = nsMath::Min(uiCurToken, Tokens.GetCount() - 1);

  nsInt32 iParenthesis = 0;

  // get all tokens up until a comma or the last closing parenthesis
  // ignore commas etc. as long as they are surrounded with parenthesis
  for (; uiCurToken < Tokens.GetCount(); ++uiCurToken)
  {
    if (Tokens[uiCurToken]->m_iType == nsTokenType::BlockComment || Tokens[uiCurToken]->m_iType == nsTokenType::LineComment || Tokens[uiCurToken]->m_iType == nsTokenType::Newline)
      continue;

    if (Tokens[uiCurToken]->m_iType == nsTokenType::EndOfFile)
      break; // outputs an error

    if (iParenthesis == 0)
    {
      if (Tokens[uiCurToken]->m_DataView == "," || Tokens[uiCurToken]->m_DataView == ")")
      {
        if (!ParamTokens.IsEmpty() && ParamTokens.PeekBack()->m_iType == nsTokenType::Whitespace)
        {
          ParamTokens.PopBack();
        }
        return NS_SUCCESS;
      }
    }

    if (Tokens[uiCurToken]->m_DataView == "(")
      ++iParenthesis;
    else if (Tokens[uiCurToken]->m_DataView == ")")
      --iParenthesis;

    ParamTokens.PushBack(Tokens[uiCurToken]);
  }

  // reached the end of the stream without encountering the closing parenthesis first
  PP_LOG0(Error, "Unexpected end of file during macro parameter extraction", Tokens[uiFirstToken]);
  return NS_FAILURE;
}

void nsPreprocessor::StringifyTokens(const TokenStream& Tokens, nsStringBuilder& sResult, bool bSurroundWithQuotes)
{
  nsUInt32 uiCurToken = 0;

  sResult.Clear();

  if (bSurroundWithQuotes)
    sResult = "\"";

  nsStringBuilder sTemp;

  SkipWhitespace(Tokens, uiCurToken);

  nsUInt32 uiLastNonWhitespace = Tokens.GetCount();

  while (uiLastNonWhitespace > 0)
  {
    if (Tokens[uiLastNonWhitespace - 1]->m_iType != nsTokenType::Whitespace && Tokens[uiLastNonWhitespace - 1]->m_iType != nsTokenType::Newline && Tokens[uiLastNonWhitespace - 1]->m_iType != nsTokenType::BlockComment && Tokens[uiLastNonWhitespace - 1]->m_iType != nsTokenType::LineComment)
      break;

    --uiLastNonWhitespace;
  }

  for (nsUInt32 t = uiCurToken; t < uiLastNonWhitespace; ++t)
  {
    // comments, newlines etc. are stripped out
    if ((Tokens[t]->m_iType == nsTokenType::LineComment) || (Tokens[t]->m_iType == nsTokenType::BlockComment) || (Tokens[t]->m_iType == nsTokenType::Newline) || (Tokens[t]->m_iType == nsTokenType::EndOfFile))
      continue;

    sTemp = Tokens[t]->m_DataView;

    // all whitespace becomes a single white space
    if (Tokens[t]->m_iType == nsTokenType::Whitespace)
      sTemp = " ";

    // inside strings, all backslashes and double quotes are escaped
    if ((Tokens[t]->m_iType == nsTokenType::String1) || (Tokens[t]->m_iType == nsTokenType::String2))
    {
      sTemp.ReplaceAll("\\", "\\\\");
      sTemp.ReplaceAll("\"", "\\\"");
    }

    sResult.Append(sTemp.GetView());
  }

  if (bSurroundWithQuotes)
    sResult.Append("\"");
}
