#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

using namespace wdTokenParseUtils;

wdPreprocessor::MacroDefinition::MacroDefinition()
{
  m_MacroIdentifier = nullptr;
  m_bIsFunction = false;
  m_bCurrentlyExpanding = false;
  m_iNumParameters = -1;
  m_bHasVarArgs = false;
}

void wdPreprocessor::CopyTokensReplaceParams(const TokenStream& Source, wdUInt32 uiFirstSourceToken, TokenStream& Destination, const wdHybridArray<wdString, 16>& parameters)
{
  Destination.Clear();
  Destination.Reserve(Source.GetCount() - uiFirstSourceToken);

  {
    // skip all whitespace at the start of the replacement string
    wdUInt32 i = uiFirstSourceToken;
    SkipWhitespace(Source, i);

    // add all the relevant tokens to the definition
    for (; i < Source.GetCount(); ++i)
    {
      if (Source[i]->m_iType == wdTokenType::BlockComment || Source[i]->m_iType == wdTokenType::LineComment || Source[i]->m_iType == wdTokenType::EndOfFile || Source[i]->m_iType == wdTokenType::Newline)
        continue;

      if (Source[i]->m_iType == wdTokenType::Identifier)
      {
        for (wdUInt32 p = 0; p < parameters.GetCount(); ++p)
        {
          if (Source[i]->m_DataView == parameters[p])
          {
            // create a custom token for the parameter, for better error messages
            wdToken* pParamToken = AddCustomToken(Source[i], parameters[p]);
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
  while (!Destination.IsEmpty() && Destination.PeekBack()->m_iType == wdTokenType::Whitespace)
    Destination.PopBack();
}

wdResult wdPreprocessor::ExtractParameterName(const TokenStream& Tokens, wdUInt32& uiCurToken, wdString& sIdentifierName)
{
  SkipWhitespace(Tokens, uiCurToken);

  if (uiCurToken + 2 < Tokens.GetCount() && Tokens[uiCurToken + 0]->m_DataView == "." && Tokens[uiCurToken + 1]->m_DataView == "." && Tokens[uiCurToken + 2]->m_DataView == ".")
  {
    sIdentifierName = "...";
    uiCurToken += 3;
  }
  else
  {
    wdUInt32 uiParamToken = uiCurToken;

    if (Expect(Tokens, uiCurToken, wdTokenType::Identifier, &uiParamToken).Failed())
      return WD_FAILURE;

    sIdentifierName = Tokens[uiParamToken]->m_DataView;
  }

  // skip a trailing comma
  if (Accept(Tokens, uiCurToken, ","))
    SkipWhitespace(Tokens, uiCurToken);

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ExtractAllMacroParameters(const TokenStream& Tokens, wdUInt32& uiCurToken, wdDeque<TokenStream>& AllParameters)
{
  if (Expect(Tokens, uiCurToken, "(").Failed())
    return WD_FAILURE;

  do
  {
    // add one parameter
    // note: we always add one extra parameter value, because MACRO() is actually a macro call with one empty parameter
    // the same for MACRO(a,) is a macro with two parameters, the second being empty
    AllParameters.SetCount(AllParameters.GetCount() + 1);

    if (ExtractParameterValue(Tokens, uiCurToken, AllParameters.PeekBack()).Failed())
      return WD_FAILURE;

    // reached the end of the parameter list
    if (Accept(Tokens, uiCurToken, ")"))
      return WD_SUCCESS;
  } while (Accept(Tokens, uiCurToken, ",")); // continue with the next parameter

  wdString s = Tokens[uiCurToken]->m_DataView;
  PP_LOG(Error, "',' or ')' expected, got '{0}' instead", Tokens[uiCurToken], s);

  return WD_FAILURE;
}

wdResult wdPreprocessor::ExtractParameterValue(const TokenStream& Tokens, wdUInt32& uiCurToken, TokenStream& ParamTokens)
{
  SkipWhitespaceAndNewline(Tokens, uiCurToken);
  const wdUInt32 uiFirstToken = wdMath::Min(uiCurToken, Tokens.GetCount() - 1);

  wdInt32 iParenthesis = 0;

  // get all tokens up until a comma or the last closing parenthesis
  // ignore commas etc. as long as they are surrounded with parenthesis
  for (; uiCurToken < Tokens.GetCount(); ++uiCurToken)
  {
    if (Tokens[uiCurToken]->m_iType == wdTokenType::BlockComment || Tokens[uiCurToken]->m_iType == wdTokenType::LineComment || Tokens[uiCurToken]->m_iType == wdTokenType::Newline)
      continue;

    if (Tokens[uiCurToken]->m_iType == wdTokenType::EndOfFile)
      break; // outputs an error

    if (iParenthesis == 0)
    {
      if (Tokens[uiCurToken]->m_DataView == "," || Tokens[uiCurToken]->m_DataView == ")")
      {
        if (!ParamTokens.IsEmpty() && ParamTokens.PeekBack()->m_iType == wdTokenType::Whitespace)
        {
          ParamTokens.PopBack();
        }
        return WD_SUCCESS;
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
  return WD_FAILURE;
}

void wdPreprocessor::StringifyTokens(const TokenStream& Tokens, wdStringBuilder& sResult, bool bSurroundWithQuotes)
{
  wdUInt32 uiCurToken = 0;

  sResult.Clear();

  if (bSurroundWithQuotes)
    sResult = "\"";

  wdStringBuilder sTemp;

  SkipWhitespace(Tokens, uiCurToken);

  wdUInt32 uiLastNonWhitespace = Tokens.GetCount();

  while (uiLastNonWhitespace > 0)
  {
    if (Tokens[uiLastNonWhitespace - 1]->m_iType != wdTokenType::Whitespace && Tokens[uiLastNonWhitespace - 1]->m_iType != wdTokenType::Newline && Tokens[uiLastNonWhitespace - 1]->m_iType != wdTokenType::BlockComment && Tokens[uiLastNonWhitespace - 1]->m_iType != wdTokenType::LineComment)
      break;

    --uiLastNonWhitespace;
  }

  for (wdUInt32 t = uiCurToken; t < uiLastNonWhitespace; ++t)
  {
    // comments, newlines etc. are stripped out
    if ((Tokens[t]->m_iType == wdTokenType::LineComment) || (Tokens[t]->m_iType == wdTokenType::BlockComment) || (Tokens[t]->m_iType == wdTokenType::Newline) || (Tokens[t]->m_iType == wdTokenType::EndOfFile))
      continue;

    sTemp = Tokens[t]->m_DataView;

    // all whitespace becomes a single white space
    if (Tokens[t]->m_iType == wdTokenType::Whitespace)
      sTemp = " ";

    // inside strings, all backslashes and double quotes are escaped
    if ((Tokens[t]->m_iType == wdTokenType::String1) || (Tokens[t]->m_iType == wdTokenType::String2))
    {
      sTemp.ReplaceAll("\\", "\\\\");
      sTemp.ReplaceAll("\"", "\\\"");
    }

    sResult.Append(sTemp.GetView());
  }

  if (bSurroundWithQuotes)
    sResult.Append("\"");
}



WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_Macros);
