#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/CodeUtils/Tokenizer.h>

namespace
{
  using TokenMatch = nsTokenParseUtils::TokenMatch;

  void CompareResults(const nsDynamicArray<TokenMatch>& expected, nsTokenizer& inout_tokenizer, bool bIgnoreWhitespace)
  {
    auto& tokens = inout_tokenizer.GetTokens();

    const nsUInt32 expectedCount = expected.GetCount();
    const nsUInt32 tokenCount = tokens.GetCount();

    nsUInt32 expectedIndex = 0, tokenIndex = 0;
    while (expectedIndex < expectedCount && tokenIndex < tokenCount)
    {
      auto& token = tokens[tokenIndex];
      if (bIgnoreWhitespace && (token.m_iType == nsTokenType::Whitespace || token.m_iType == nsTokenType::Newline))
      {
        tokenIndex++;
        continue;
      }

      auto& e = expected[expectedIndex];

      if (!NS_TEST_BOOL_MSG(e.m_Type == token.m_iType, "Token with index %u does not match in type, expected %d actual %d", expectedIndex, e.m_Type, token.m_iType))
      {
        return;
      }

      if (!NS_TEST_BOOL_MSG(e.m_sToken == token.m_DataView, "Token with index %u does not match, expected '%.*s' actual '%.*s'", expectedIndex, e.m_sToken.GetElementCount(), e.m_sToken.GetStartPointer(), token.m_DataView.GetElementCount(), token.m_DataView.GetStartPointer()))
      {
        return;
      }
      tokenIndex++;
      expectedIndex++;
    }

    // Skip remaining whitespace and newlines
    if (bIgnoreWhitespace)
    {
      while (tokenIndex < tokenCount)
      {
        auto& token = tokens[tokenIndex];
        if (token.m_iType != nsTokenType::Whitespace && token.m_iType != nsTokenType::Newline)
        {
          break;
        }
        tokenIndex++;
      }
    }

    if (NS_TEST_BOOL_MSG(tokenIndex == tokenCount - 1, "Not all tokens have been consumed"))
    {
      NS_TEST_BOOL_MSG(tokens[tokenIndex].m_iType == nsTokenType::EndOfFile, "Last token must be end of file token");
    }

    NS_TEST_BOOL_MSG(expectedIndex == expectedCount, "Not all expected values have been consumed");
  }
} // namespace

NS_CREATE_SIMPLE_TEST(CodeUtils, Tokenizer)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Token Types")
  {
    const char* stringLiteral = R"(
float f=10.3f + 100'000.0;
int i=100'000*12345;
// line comment
/*
block comment
*/
char c='f';
const char* bla =  "blup";
)";
    nsTokenizer tokenizer(nsFoundation::GetDefaultAllocator());
    tokenizer.Tokenize(nsMakeArrayPtr(reinterpret_cast<const nsUInt8*>(stringLiteral), nsStringUtils::GetStringElementCount(stringLiteral)), nsLog::GetThreadLocalLogSystem(), false);

    NS_TEST_BOOL(tokenizer.GetTokenizedData().IsEmpty());

    nsDynamicArray<TokenMatch> expectedResult;
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::Identifier, "float"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Identifier, "f"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::Float, "10.3f"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "+"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Float, "100'000.0"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::Identifier, "int"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Identifier, "i"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::Integer, "100'000"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "*"});
    expectedResult.PushBack({nsTokenType::Integer, "12345"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::LineComment, "// line comment"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::BlockComment, "/*\nblock comment\n*/"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::Identifier, "char"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Identifier, "c"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::String2, "'f'"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    expectedResult.PushBack({nsTokenType::Identifier, "const"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Identifier, "char"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "*"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::Identifier, "bla"});
    expectedResult.PushBack({nsTokenType::Whitespace, " "});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::Whitespace, "  "});
    expectedResult.PushBack({nsTokenType::String1, "\"blup\""});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});
    expectedResult.PushBack({nsTokenType::Newline, "\n"});

    CompareResults(expectedResult, tokenizer, false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Raw string literal")
  {
    const char* stringLiteral = R"token(
const char* test = R"(
eins
zwei)";
const char* test2 = R"foo(
vier,
fuenf
)foo";
)token";

    nsTokenizer tokenizer(nsFoundation::GetDefaultAllocator());
    tokenizer.Tokenize(nsMakeArrayPtr(reinterpret_cast<const nsUInt8*>(stringLiteral), nsStringUtils::GetStringElementCount(stringLiteral)), nsLog::GetThreadLocalLogSystem());

    nsDynamicArray<TokenMatch> expectedResult;
    expectedResult.PushBack({nsTokenType::Identifier, "const"});
    expectedResult.PushBack({nsTokenType::Identifier, "char"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "*"});
    expectedResult.PushBack({nsTokenType::Identifier, "test"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::RawString1Prefix, "R\"("});
    expectedResult.PushBack({nsTokenType::RawString1, "\neins\nzwei"});
    expectedResult.PushBack({nsTokenType::RawString1Postfix, ")\""});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});
    expectedResult.PushBack({nsTokenType::Identifier, "const"});
    expectedResult.PushBack({nsTokenType::Identifier, "char"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "*"});
    expectedResult.PushBack({nsTokenType::Identifier, "test2"});
    expectedResult.PushBack({nsTokenType::NonIdentifier, "="});
    expectedResult.PushBack({nsTokenType::RawString1Prefix, "R\"foo("});
    expectedResult.PushBack({nsTokenType::RawString1, "\nvier,\nfuenf\n"});
    expectedResult.PushBack({nsTokenType::RawString1Postfix, ")foo\""});
    expectedResult.PushBack({nsTokenType::NonIdentifier, ";"});

    CompareResults(expectedResult, tokenizer, true);
  }
}
