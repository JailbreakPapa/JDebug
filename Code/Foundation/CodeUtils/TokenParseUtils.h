#pragma once

#include <Foundation/CodeUtils/Tokenizer.h>
#include <Foundation/Containers/HybridArray.h>

namespace wdTokenParseUtils
{
  using TokenStream = wdHybridArray<const wdToken*, 32>;

  WD_FOUNDATION_DLL void SkipWhitespace(const TokenStream& tokens, wdUInt32& ref_uiCurToken);
  WD_FOUNDATION_DLL void SkipWhitespaceAndNewline(const TokenStream& tokens, wdUInt32& ref_uiCurToken);
  WD_FOUNDATION_DLL bool IsEndOfLine(const TokenStream& tokens, wdUInt32 uiCurToken, bool bIgnoreWhitespace);
  WD_FOUNDATION_DLL void CopyRelevantTokens(const TokenStream& source, wdUInt32 uiFirstSourceToken, TokenStream& ref_destination, bool bPreserveNewLines);

  WD_FOUNDATION_DLL bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken, wdUInt32* pAccepted = nullptr);
  WD_FOUNDATION_DLL bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, wdTokenType::Enum type, wdUInt32* pAccepted = nullptr);
  WD_FOUNDATION_DLL bool Accept(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted = nullptr);
  WD_FOUNDATION_DLL bool AcceptUnless(const TokenStream& tokens, wdUInt32& ref_uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted = nullptr);

  WD_FOUNDATION_DLL void CombineTokensToString(const TokenStream& tokens, wdUInt32 uiCurToken, wdStringBuilder& ref_sResult, bool bKeepComments = true, bool bRemoveRedundantWhitespace = false, bool bInsertLine = false);
  WD_FOUNDATION_DLL void CombineRelevantTokensToString(const TokenStream& tokens, wdUInt32 uiCurToken, wdStringBuilder& ref_sResult);
  WD_FOUNDATION_DLL void CreateCleanTokenStream(const TokenStream& tokens, wdUInt32 uiCurToken, TokenStream& ref_destination, bool bKeepComments);
} // namespace wdTokenParseUtils
