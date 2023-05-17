#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/HashedString.h>

/// \brief Describes which kind of token an wdToken is.
struct WD_FOUNDATION_DLL wdTokenType
{
  enum Enum
  {
    Unknown,       ///< for internal use
    Whitespace,    ///< The token is a space or tab
    Identifier,    ///< a series of alphanumerics or underscores
    NonIdentifier, ///< Everything else
    Newline,       ///< Either '\n' or '\r\n'
    LineComment,   ///< A comment that starts with two slashes and ends at the next newline (or end of file)
    BlockComment,  ///< A comment that starts with a slash and a star, and ends at the next star/slash combination (or end of file)
    String1,       ///< A string enclosed in "
    String2,       ///< A string enclosed in '
    Integer,       ///< An integer number
    Float,         ///< A floating point number
    EndOfFile,     ///< End-of-file marker
    ENUM_COUNT,
  };

  static const char* EnumNames[ENUM_COUNT];
};

/// \brief Represents one piece of tokenized text in a document.
struct WD_FOUNDATION_DLL wdToken
{
  wdToken()
  {
    m_iType = wdTokenType::Unknown;
    m_uiLine = 0;
    m_uiColumn = 0;
    m_uiCustomFlags = 0;
  }

  /// Typically of type wdTokenType, but users can put anything in there, that they like
  wdInt32 m_iType;

  /// The line in which the token appeared
  wdUInt32 m_uiLine;

  /// The column in the line, at which the token string started.
  wdUInt32 m_uiColumn;

  /// The actual string data that represents the token. Note that this is a view to a substring of some larger text data.
  /// To get only the relevant piece as one zero-terminated string, assign m_DataView to an wdStringBuilder and read that instead.
  wdStringView m_DataView;

  /// For users to be able to store additional info for a token.
  wdUInt32 m_uiCustomFlags;

  /// The file in which the token appeared.
  wdHashedString m_File;
};

/// \brief Takes text and splits it up into wdToken objects. The result can be used for easier parsing.
///
/// The tokenizer is built to work on code that is similar to C. That means it will tokenize comments and
/// strings as they are defined in the C language. Also line breaks that end with a backslash are not
/// really considered as line breaks.\n
/// White space is defined as spaces and tabs.\n
/// Identifiers are names that consist of alphanumerics and underscores.\n
/// Non-Identifiers are everything else. However, they will currently never consist of more than a single character.
/// Ie. '++' will be tokenized as two consecutive non-Identifiers.\n
/// Parenthesis etc. will not be tokenized in any special way, they are all considered as non-Identifiers.
///
/// The token stream will always end with an end-of-file token.
class WD_FOUNDATION_DLL wdTokenizer
{
public:
  /// \brief Constructor.
  ///
  /// Takes an additional optional allocator. If no allocator is given the default allocator will be used.
  wdTokenizer(wdAllocatorBase* pAllocator = nullptr);

  ~wdTokenizer();

  /// \brief Clears any previous result and creates a new token stream for the given array.
  void Tokenize(wdArrayPtr<const wdUInt8> data, wdLogInterface* pLog);

  /// \brief Gives read access to the token stream.
  const wdDeque<wdToken>& GetTokens() const { return m_Tokens; }

  /// \brief Gives read and write access to the token stream.
  wdDeque<wdToken>& GetTokens() { return m_Tokens; }

  /// \brief Returns an array of all tokens. New line tokens are ignored.
  void GetAllLines(wdHybridArray<const wdToken*, 32>& ref_tokens) const;

  /// \brief Returns an array of tokens that represent the next line in the file.
  ///
  /// Returns WD_SUCCESS when there was more data to return, WD_FAILURE if the end of the file was reached already.
  /// uiFirstToken is the index from where to start. It will be updated automatically. Consecutive calls to GetNextLine()
  /// with the same uiFirstToken variable will give one line after the other.
  ///
  /// \note This function takes care of handling the 'backslash/newline' combination, as defined in the C language.
  /// That means all such sequences will be ignored. Therefore the tokens that are returned as one line might not
  /// contain all tokens that are actually in the stream. Also the tokens might have different line numbers, when
  /// two or more lines from the file are merged into one logical line.
  wdResult GetNextLine(wdUInt32& ref_uiFirstToken, wdHybridArray<const wdToken*, 32>& ref_tokens) const;

  wdResult GetNextLine(wdUInt32& ref_uiFirstToken, wdHybridArray<wdToken*, 32>& ref_tokens);

  /// \brief Returns the internal copy of the tokenized data
  const wdDynamicArray<wdUInt8>& GetTokenizedData() const { return m_Data; }

  /// \brief Enables treating lines that start with # character as line comments
  ///
  /// Needs to be set before tokenization to take effect.
  void SetTreatHashSignAsLineComment(bool bHashSignIsLineComment) { m_bHashSignIsLineComment = bHashSignIsLineComment; }

private:
  void NextChar();
  void AddToken();

  void HandleUnknown();
  void HandleString(char terminator);
  void HandleNumber();
  void HandleLineComment();
  void HandleBlockComment();
  void HandleWhitespace();
  void HandleIdentifier();
  void HandleNonIdentifier();

  wdLogInterface* m_pLog = nullptr;
  wdTokenType::Enum m_CurMode = wdTokenType::Unknown;
  wdStringView m_sIterator;
  wdUInt32 m_uiCurLine = 1;
  wdUInt32 m_uiCurColumn = -1;
  wdUInt32 m_uiCurChar = '\0';
  wdUInt32 m_uiNextChar = '\0';

  wdUInt32 m_uiLastLine = 1;
  wdUInt32 m_uiLastColumn = 1;

  const char* m_szCurCharStart = nullptr;
  const char* m_szNextCharStart = nullptr;
  const char* m_szTokenStart = nullptr;

  wdDeque<wdToken> m_Tokens;
  wdDynamicArray<wdUInt8> m_Data;

  bool m_bHashSignIsLineComment = false;
};
