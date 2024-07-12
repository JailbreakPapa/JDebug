#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/CodeUtils/Tokenizer.h>

NS_CREATE_SIMPLE_TEST(CodeUtils, TokenParseUtils)
{
  const char* stringLiteral = R"(
// Some comment
/* A block comment
Some block
*/
Identifier
)";

  nsTokenizer tokenizer(nsFoundation::GetDefaultAllocator());
  tokenizer.Tokenize(nsMakeArrayPtr(reinterpret_cast<const nsUInt8*>(stringLiteral), nsStringUtils::GetStringElementCount(stringLiteral)), nsLog::GetThreadLocalLogSystem(), false);

  nsTokenParseUtils::TokenStream tokens;
  tokenizer.GetAllTokens(tokens);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipWhitespace / IsEndOfLine")
  {
    nsUInt32 uiCurToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    uiCurToken++;
    NS_TEST_BOOL(!nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, true));
    nsTokenParseUtils::SkipWhitespace(tokens, uiCurToken);
    NS_TEST_INT(uiCurToken, 2);
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    uiCurToken++;
    NS_TEST_BOOL(!nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, true));
    nsTokenParseUtils::SkipWhitespace(tokens, uiCurToken);
    NS_TEST_INT(uiCurToken, 4);
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    uiCurToken++;
    NS_TEST_BOOL(!nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    NS_TEST_BOOL(!nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, true));
    NS_TEST_INT(tokens[uiCurToken]->m_iType, nsTokenType::Identifier);
    uiCurToken++;
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    uiCurToken++;
    NS_TEST_INT(tokens[uiCurToken]->m_iType, nsTokenType::EndOfFile);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipWhitespaceAndNewline")
  {
    nsUInt32 uiCurToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    nsTokenParseUtils::SkipWhitespaceAndNewline(tokens, uiCurToken);
    NS_TEST_INT(uiCurToken, 5);
    NS_TEST_INT(tokens[uiCurToken]->m_iType, nsTokenType::Identifier);
    uiCurToken++;
    NS_TEST_BOOL(nsTokenParseUtils::IsEndOfLine(tokens, uiCurToken, false));
    nsTokenParseUtils::SkipWhitespaceAndNewline(tokens, uiCurToken);
    NS_TEST_INT(tokens[uiCurToken]->m_iType, nsTokenType::EndOfFile);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CopyRelevantTokens")
  {
    nsUInt32 uiCurToken = 0;
    nsTokenParseUtils::TokenStream relevantTokens;
    nsTokenParseUtils::CopyRelevantTokens(tokens, uiCurToken, relevantTokens, true);

    NS_TEST_INT(relevantTokens.GetCount(), 5);
    for (nsUInt32 i = 0; i < relevantTokens.GetCount(); ++i)
    {
      if (i == 3)
      {
        NS_TEST_INT(relevantTokens[i]->m_iType, nsTokenType::Identifier);
      }
      else
      {
        NS_TEST_INT(relevantTokens[i]->m_iType, nsTokenType::Newline);
      }
    }

    relevantTokens.Clear();
    nsTokenParseUtils::CopyRelevantTokens(tokens, uiCurToken, relevantTokens, false);
    NS_TEST_INT(relevantTokens.GetCount(), 1);
    NS_TEST_INT(relevantTokens[0]->m_iType, nsTokenType::Identifier);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Accept")
  {
    nsUInt32 uiCurToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, "\n"_nssv, nullptr));
    NS_TEST_INT(uiCurToken, 1);
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, nsTokenType::Newline, nullptr));
    NS_TEST_INT(uiCurToken, 3);
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, "\n"_nssv, nullptr));
    NS_TEST_INT(uiCurToken, 5);

    nsUInt32 uiIdentifierToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, nsTokenType::Identifier, &uiIdentifierToken));
    NS_TEST_INT(uiIdentifierToken, 5);
    NS_TEST_INT(uiCurToken, 6);

    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, nsTokenType::Newline, nullptr));

    NS_TEST_BOOL(!nsTokenParseUtils::Accept(tokens, uiCurToken, nsTokenType::Newline, nullptr));
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, nsTokenType::EndOfFile, nullptr));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Accept2")
  {
    nsUInt32 uiCurToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, "\n"_nssv, "// Some comment"_nssv, nullptr));
    NS_TEST_INT(uiCurToken, 2);
    nsUInt32 uiTouple1Token = 0;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, "\n"_nssv, "/* A block comment\nSome block\n*/"_nssv, &uiTouple1Token));
    NS_TEST_INT(uiTouple1Token, 2);
    NS_TEST_INT(uiCurToken, 4);
    NS_TEST_BOOL(!nsTokenParseUtils::AcceptUnless(tokens, uiCurToken, "\n"_nssv, "Identifier"_nssv, nullptr));
    uiCurToken++;
    nsUInt32 uiIdentifierToken = 0;
    NS_TEST_BOOL(nsTokenParseUtils::AcceptUnless(tokens, uiCurToken, "Identifier"_nssv, "ScaryString"_nssv, &uiIdentifierToken));
    NS_TEST_INT(uiIdentifierToken, 5);
    NS_TEST_INT(uiCurToken, 6);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Accept3")
  {
    nsUInt32 uiCurToken = 0;
    nsTokenParseUtils::TokenMatch templatePattern[] = {nsTokenType::Newline, nsTokenType::Newline, "Identifier"_nssv};
    nsHybridArray<nsUInt32, 8> acceptedTokens;
    NS_TEST_BOOL(!nsTokenParseUtils::Accept(tokens, uiCurToken, templatePattern, &acceptedTokens));
    uiCurToken++;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens, uiCurToken, templatePattern, &acceptedTokens));

    NS_TEST_INT(acceptedTokens.GetCount(), NS_ARRAY_SIZE(templatePattern));
    NS_TEST_INT(acceptedTokens[0], 2);
    NS_TEST_INT(acceptedTokens[1], 4);
    NS_TEST_INT(acceptedTokens[2], 5);
    NS_TEST_INT(uiCurToken, 6);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Accept4")
  {
    const char* vectorString = "Vec2(2.2, 1.1)";

    nsTokenizer tokenizer2(nsFoundation::GetDefaultAllocator());
    tokenizer2.Tokenize(nsMakeArrayPtr(reinterpret_cast<const nsUInt8*>(vectorString), nsStringUtils::GetStringElementCount(vectorString)), nsLog::GetThreadLocalLogSystem(), false);

    nsTokenParseUtils::TokenStream tokens2;
    tokenizer2.GetAllTokens(tokens2);

    nsUInt32 uiCurToken = 0;
    nsTokenParseUtils::TokenMatch templatePattern[] = {"Vec2"_nssv, "("_nssv, nsTokenType::Float, ","_nssv, nsTokenType::Float, ")"_nssv};
    nsHybridArray<nsUInt32, 6> acceptedTokens;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(tokens2, uiCurToken, templatePattern, &acceptedTokens));
    NS_TEST_INT(uiCurToken, 7);
    NS_TEST_INT(acceptedTokens.GetCount(), NS_ARRAY_SIZE(templatePattern));
    NS_TEST_INT(acceptedTokens[2], 2);
    NS_TEST_INT(acceptedTokens[4], 5);
    NS_TEST_STRING(tokens2[acceptedTokens[2]]->m_DataView, "2.2");
    NS_TEST_STRING(tokens2[acceptedTokens[4]]->m_DataView, "1.1");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CombineTokensToString")
  {
    nsUInt32 uiCurToken = 0;
    nsStringBuilder sResult;
    nsTokenParseUtils::CombineTokensToString(tokens, uiCurToken, sResult);
    NS_TEST_STRING(sResult, stringLiteral);

    nsTokenParseUtils::CombineTokensToString(tokens, uiCurToken, sResult, false, true);
    NS_TEST_STRING(sResult, "\n\n\nIdentifier\n");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CombineRelevantTokensToString")
  {
    nsUInt32 uiCurToken = 0;
    nsStringBuilder sResult;
    nsTokenParseUtils::CombineRelevantTokensToString(tokens, uiCurToken, sResult);
    NS_TEST_STRING(sResult, "Identifier");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CreateCleanTokenStream")
  {
    const char* stringLiteralWithRedundantStuff = "\n\nID1 \nID2";

    nsTokenizer tokenizer2(nsFoundation::GetDefaultAllocator());
    tokenizer2.Tokenize(nsMakeArrayPtr(reinterpret_cast<const nsUInt8*>(stringLiteralWithRedundantStuff), nsStringUtils::GetStringElementCount(stringLiteralWithRedundantStuff)), nsLog::GetThreadLocalLogSystem(), false);

    nsTokenParseUtils::TokenStream tokens2;
    tokenizer2.GetAllTokens(tokens2);

    nsUInt32 uiCurToken = 0;
    nsTokenParseUtils::TokenStream result;
    nsTokenParseUtils::CreateCleanTokenStream(tokens2, uiCurToken, result);

    NS_TEST_INT(result.GetCount(), 5);

    nsTokenParseUtils::TokenMatch templatePattern[] = {nsTokenType::Newline, "ID1"_nssv, nsTokenType::Newline, "ID2"_nssv, nsTokenType::EndOfFile};
    nsHybridArray<nsUInt32, 8> acceptedTokens;
    NS_TEST_BOOL(nsTokenParseUtils::Accept(result, uiCurToken, templatePattern, nullptr));
    NS_TEST_INT(uiCurToken, 5);
  }
}
