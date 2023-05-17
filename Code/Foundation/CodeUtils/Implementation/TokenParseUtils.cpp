#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/CodeUtils/Tokenizer.h>

namespace wdTokenParseUtils
{
  void SkipWhitespace(const TokenStream& tokens, wdUInt32& ref_uiCurToken)
  {
    while (ref_uiCurToken < tokens.GetCount() && ((tokens[ref_uiCurToken]->m_iType == wdTokenType::Whitespace) || (tokens[ref_uiCurToken]->m_iType == wdTokenType::BlockComment) || (tokens[ref_uiCurToken]->m_iType == wdTokenType::LineComment)))
      ++ref_uiCurToken;
  }

  void SkipWhitespaceAndNewline(const TokenStream& tokens, wdUInt32& ref_uiCurToken)
  {
    while (ref_uiCurToken < tokens.GetCount() && ((tokens[ref_uiCurToken]->m_iType == wdTokenType::Whitespace) || (tokens[ref_uiCurToken]->m_iType == wdTokenType::BlockComment) || (tokens[ref_uiCurToken]->m_iType == wdTokenType::Newline) || (tokens[ref_uiCurToken]->m_iType == wdTokenType::LineComment)))
      ++ref_uiCurToken;
  }

  bool IsEndOfLine(const TokenStream& tokens, wdUInt32 uiCurToken, bool bIgnoreWhitespace)
  {
    if (bIgnoreWhitespace)
      SkipWhitespace(tokens, uiCurToken);

    if (uiCurToken >= tokens.GetCount())
      return true;

    return tokens[uiCurToken]->m_iType == wdTokenType::Newline || tokens[uiCurToken]->m_iType == wdTokenType::EndOfFile;
  }

  void CopyRelevantTokens(const TokenStream& source, wdUInt32 uiFirstSourceToken, TokenStream& ref_destination, bool bPreserveNewLines)
  {
    ref_destination.Reserve(ref_destination.GetCount() + source.GetCount() - uiFirstSourceToken);

    {
      // skip all whitespace at the start of the replacement string
      wdUInt32 i = uiFirstSourceToken;
      SkipWhitespace(source, i);

      // add all the relevant tokens to the definition
      for (; i < source.GetCount(); ++i)
      {
        if (source[i]->m_iType == wdTokenType::BlockComment || source[i]->m_iType == wdTokenType::LineComment || source[i]->m_iType == wdTokenType::EndOfFile || (!bPreserveNewLines && source[i]->m_iType == wdTokenType::Newline))
          continue;

        ref_destination.PushBack(source[i]);
      }
    }

    // remove whitespace at end of macro
    while (!ref_destination.IsEmpty() && ref_destination.PeekBack()->m_iType == wdTokenType::Whitespace)
      ref_destination.PopBack();
  }

  bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken, wdUInt32* pAccepted)
  {
    SkipWhitespace(tokens, ref_uiCurToken);

    if (ref_uiCurToken >= tokens.GetCount())
      return false;

    if (tokens[ref_uiCurToken]->m_DataView == szToken)
    {
      if (pAccepted)
        *pAccepted = ref_uiCurToken;

      ref_uiCurToken++;
      return true;
    }

    return false;
  }

  bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, wdTokenType::Enum type, wdUInt32* pAccepted)
  {
    SkipWhitespace(tokens, ref_uiCurToken);

    if (ref_uiCurToken >= tokens.GetCount())
      return false;

    if (tokens[ref_uiCurToken]->m_iType == type)
    {
      if (pAccepted)
        *pAccepted = ref_uiCurToken;

      ref_uiCurToken++;
      return true;
    }

    return false;
  }

  bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted)
  {
    SkipWhitespace(tokens, ref_uiCurToken);

    if (ref_uiCurToken + 1 >= tokens.GetCount())
      return false;

    if (tokens[ref_uiCurToken]->m_DataView == szToken1 && tokens[ref_uiCurToken + 1]->m_DataView == szToken2)
    {
      if (pAccepted)
        *pAccepted = ref_uiCurToken;

      ref_uiCurToken += 2;
      return true;
    }

    return false;
  }

  bool AcceptUnless(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted)
  {
    SkipWhitespace(tokens, ref_uiCurToken);

    if (ref_uiCurToken + 1 >= tokens.GetCount())
      return false;

    if (tokens[ref_uiCurToken]->m_DataView == szToken1 && tokens[ref_uiCurToken + 1]->m_DataView != szToken2)
    {
      if (pAccepted)
        *pAccepted = ref_uiCurToken;

      ref_uiCurToken += 1;
      return true;
    }

    return false;
  }

  void CombineRelevantTokensToString(const TokenStream& tokens, wdUInt32 uiCurToken, wdStringBuilder& ref_sResult)
  {
    ref_sResult.Clear();
    wdStringBuilder sTemp;

    for (wdUInt32 t = uiCurToken; t < tokens.GetCount(); ++t)
    {
      if ((tokens[t]->m_iType == wdTokenType::LineComment) || (tokens[t]->m_iType == wdTokenType::BlockComment) || (tokens[t]->m_iType == wdTokenType::Newline) || (tokens[t]->m_iType == wdTokenType::EndOfFile))
        continue;

      sTemp = tokens[t]->m_DataView;
      ref_sResult.Append(sTemp.GetView());
    }
  }

  void CreateCleanTokenStream(const TokenStream& tokens, wdUInt32 uiCurToken, TokenStream& ref_destination, bool bKeepComments)
  {
    SkipWhitespace(tokens, uiCurToken);

    for (wdUInt32 t = uiCurToken; t < tokens.GetCount(); ++t)
    {
      if (tokens[t]->m_iType == wdTokenType::Newline)
      {
        // remove all whitespace before a newline
        while (!ref_destination.IsEmpty() && ref_destination.PeekBack()->m_iType == wdTokenType::Whitespace)
          ref_destination.PopBack();

        // if there is already a newline stored, discard the new one
        if (!ref_destination.IsEmpty() && ref_destination.PeekBack()->m_iType == wdTokenType::Newline)
          continue;
      }

      ref_destination.PushBack(tokens[t]);
    }
  }

  void CombineTokensToString(const TokenStream& tokens0, wdUInt32 uiCurToken, wdStringBuilder& ref_sResult, bool bKeepComments, bool bRemoveRedundantWhitespace, bool bInsertLine)
  {
    TokenStream Tokens;

    if (bRemoveRedundantWhitespace)
    {
      CreateCleanTokenStream(tokens0, uiCurToken, Tokens, bKeepComments);
      uiCurToken = 0;
    }
    else
      Tokens = tokens0;

    ref_sResult.Clear();
    wdStringBuilder sTemp;

    wdUInt32 uiCurLine = 0xFFFFFFFF;
    wdHashedString sCurFile;

    for (wdUInt32 t = uiCurToken; t < Tokens.GetCount(); ++t)
    {
      // skip all comments, if not desired
      if ((Tokens[t]->m_iType == wdTokenType::BlockComment || Tokens[t]->m_iType == wdTokenType::LineComment) && !bKeepComments)
        continue;

      if (Tokens[t]->m_iType == wdTokenType::EndOfFile)
        return;

      if (bInsertLine)
      {
        if (ref_sResult.IsEmpty())
        {
          ref_sResult.AppendFormat("#line {0} \"{1}\"\n", Tokens[t]->m_uiLine, Tokens[t]->m_File);
          uiCurLine = Tokens[t]->m_uiLine;
          sCurFile = Tokens[t]->m_File;
        }

        if (Tokens[t]->m_iType == wdTokenType::Newline)
        {
          ++uiCurLine;
        }

        if (t > 0 && Tokens[t - 1]->m_iType == wdTokenType::Newline)
        {
          if (Tokens[t]->m_uiLine != uiCurLine || Tokens[t]->m_File != sCurFile)
          {
            ref_sResult.AppendFormat("\n#line {0} \"{1}\"\n", Tokens[t]->m_uiLine, Tokens[t]->m_File);
            uiCurLine = Tokens[t]->m_uiLine;
            sCurFile = Tokens[t]->m_File;
          }
        }
      }

      sTemp = Tokens[t]->m_DataView;
      ref_sResult.Append(sTemp.GetView());
    }
  }
} // namespace wdTokenParseUtils

WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_TokenParseUtils);
