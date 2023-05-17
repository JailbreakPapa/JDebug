#pragma once

#include <Foundation/Basics.h>
#include <Foundation/CodeUtils/TokenParseUtils.h>
#include <Foundation/CodeUtils/Tokenizer.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Time/Timestamp.h>

/// \brief This object caches files in a tokenized state. It can be shared among wdPreprocessor instances to improve performance when
/// they access the same files.
class WD_FOUNDATION_DLL wdTokenizedFileCache
{
public:
  struct FileData
  {
    wdTokenizer m_Tokens;
    wdTimestamp m_Timestamp;
  };

  /// \brief Checks whether \a sFileName is already in the cache, returns an iterator to it. If the iterator is invalid, the file is not cached yet.
  wdMap<wdString, FileData>::ConstIterator Lookup(const wdString& sFileName) const;

  /// \brief Removes the cached content for \a sFileName from the cache. Should be used when the file content has changed and needs to be re-read.
  void Remove(const wdString& sFileName);

  /// \brief Removes all files from the cache to ensure that they will be re-read.
  void Clear();

  /// \brief Stores \a FileContent for the file \a sFileName as the new cached data.
  ///
  //// The file content is tokenized first and all #line directives are evaluated, to update the line number and file origin for each token.
  /// Any errors are written to the given log.
  const wdTokenizer* Tokenize(const wdString& sFileName, wdArrayPtr<const wdUInt8> fileContent, const wdTimestamp& fileTimeStamp, wdLogInterface* pLog);

private:
  void SkipWhitespace(wdDeque<wdToken>& Tokens, wdUInt32& uiCurToken);

  mutable wdMutex m_Mutex;
  wdMap<wdString, FileData> m_Cache;
};

/// \brief wdPreprocessor implements a standard C preprocessor. It can be used to pre-process files to get the output after macro expansion and #ifdef
/// handling.
///
/// For a detailed documentation about the C preprocessor, see https://gcc.gnu.org/onlinedocs/cpp/
///
/// This class implements all standard features:
///   * object and function macros
///   * Full evaluation of #if, #ifdef etc. including mathematical operations such as #if A > 42
///   * Parameter stringification
///   * Parameter concatenation
///   * __LINE__ and __FILE__ macros
///   * Fully correct #line evaluation for error output
///   * Correct handling of __VA_ARGS__
///   * #include handling
///   * #pragma once
///   * #warning and #error for custom failure messages
class WD_FOUNDATION_DLL wdPreprocessor
{
public:
  /// \brief Describes the type of #include that was encountered during preprocessing
  enum IncludeType
  {
    MainFile,        ///< This is used for the very first access to the main source file
    RelativeInclude, ///< An #include "file" has been encountered
    GlobalInclude    ///< An #include <file> has been encountered
  };

  /// \brief This type of callback is used to read an #include file. \a szAbsoluteFile is the path that the FileLocatorCB reported, the result needs
  /// to be stored in \a FileContent.
  typedef wdDelegate<wdResult(const char* szAbsoluteFile, wdDynamicArray<wdUInt8>& FileContent, wdTimestamp& out_FileModification)> FileOpenCB;

  /// \brief This type of callback is used to retrieve the absolute path of the \a szIncludeFile when #included inside \a szCurAbsoluteFile.
  ///
  /// Note that you should ensure that \a out_sAbsoluteFilePath is always identical (including casing and path slashes) when it is supposed to point
  /// to the same file, as this exact name is used for file lookup (and therefore also file caching).
  /// If it is not identical, file caching will not work, and on different OSes the file may be found or not.
  typedef wdDelegate<wdResult(const char* szCurAbsoluteFile, const char* szIncludeFile, IncludeType IncType, wdStringBuilder& out_sAbsoluteFilePath)> FileLocatorCB;

  /// \brief Every time an unknown command (e.g. '#version') is encountered, this callback is used to determine whether the command shall be passed
  /// through.
  ///
  /// If the callback returns false, an error is generated and parsing fails. The callback thus acts as a whitelist for all commands that shall be
  /// passed through.
  typedef wdDelegate<bool(const char* szUnknownCommand)> PassThroughUnknownCmdCB;

  typedef wdDeque<wdTokenParseUtils::TokenStream> MacroParameters;

  /// \brief The event data that the processor broadcasts
  ///
  /// Please note that m_pToken contains a lot of interesting information, such as
  /// the current file and line number and of course the current piece of text.
  struct ProcessingEvent
  {
    /// \brief The event types that the processor broadcasts
    enum EventType
    {
      BeginExpansion,  ///< A macro is now going to be expanded
      EndExpansion,    ///< A macro is finished being expanded
      Error,           ///< An error was encountered
      Warning,         ///< A warning has been output.
      CheckDefined,    ///< A 'defined(X)' is being evaluated
      CheckIfdef,      ///< A '#ifdef X' is being evaluated
      CheckIfndef,     ///< A '#ifndef X' is being evaluated
      EvaluateUnknown, ///< Inside an #if an unknown identifier has been encountered, it will be evaluated as zero
      Define,          ///< A #define X has been stored
      Redefine,        ///< A #define for an already existing macro name (also logged as a warning)
    };

    ProcessingEvent()
    {
      m_Type = Error;
      m_pToken = nullptr;
      m_szInfo = "";
    }

    EventType m_Type;

    const wdToken* m_pToken;
    const char* m_szInfo;
  };

  /// \brief Broadcasts events during the processing. This can be used to create detailed callstacks when an error is encountered.
  /// It also broadcasts errors and warnings with more detailed information than the log interface allows.
  wdEvent<const ProcessingEvent&> m_ProcessingEvents;

  wdPreprocessor();

  /// \brief All error output is sent to the given wdLogInterface.
  ///
  /// Note that when the preprocessor encounters any error, it will stop immediately and usually no output is generated.
  /// However, there are also a few cases where only a warning is generated, in this case preprocessing will continue without problems.
  ///
  /// Additionally errors and warnings are also broadcast through m_ProcessingEvents. So if you want to output more detailed information,
  /// that method should be preferred, because the events carry more information about the current file and line number etc.
  void SetLogInterface(wdLogInterface* pLog);

  /// \brief Allows to specify a custom cache object that should be used for storing the tokenized result of files.
  ///
  /// This allows to share one cache across multiple instances of wdPreprocessor and across time. E.g. it makes it possible
  /// to prevent having to read and tokenize include files that are referenced often.
  void SetCustomFileCache(wdTokenizedFileCache* pFileCache = nullptr);

  /// \brief If set to true, all #pragma commands are passed through to the output, otherwise they are removed.
  void SetPassThroughPragma(bool bPassThrough) { m_bPassThroughPragma = bPassThrough; }

  /// \brief If set to true, all #line commands are passed through to the output, otherwise they are removed.
  void SetPassThroughLine(bool bPassThrough) { m_bPassThroughLine = bPassThrough; }

  /// \brief Sets the callback that is used to determine whether an unknown command is passed through or triggers an error.
  void SetPassThroughUnknownCmdsCB(PassThroughUnknownCmdCB callback) { m_PassThroughUnknownCmdCB = callback; }

  /// \brief Sets the callback that is needed to read input data.
  ///
  /// The default file open function will just try to open files via wdFileReader.
  void SetFileOpenFunction(FileOpenCB openAbsFileCB);

  /// \brief Sets the callback that is needed to locate an input file
  ///
  /// The default file locator will assume that the main source file and all files #included in angle brackets can be opened without modification.
  /// Files #included in "" will be appended as relative paths to the path of the file they appeared in.
  void SetFileLocatorFunction(FileLocatorCB locateAbsFileCB);

  /// \brief Adds a #define to the preprocessor, even before any file is processed.
  ///
  /// This allows to have global macros that are always defined for all processed files, such as the current platform etc.
  /// \a szDefinition must be in the form of the text that follows a #define statement. So to define the macro "WIN32", just
  /// pass that string. You can define any macro that could also be defined in the source files.
  ///
  /// If the definition is invalid, WD_FAILURE is returned. Also the preprocessor might end up in an invalid state, so using it any
  /// further might fail (including crashing).
  wdResult AddCustomDefine(const char* szDefinition);

  /// \brief Processes the given file and returns the result as a stream of tokens.
  ///
  /// This function is useful when you want to further process the output afterwards and thus need it in a tokenized form anyway.
  wdResult Process(const char* szMainFile, wdTokenParseUtils::TokenStream& ref_tokenOutput);

  /// \brief Processes the given file and returns the result as a string.
  ///
  /// This function creates a string from the tokenized result. If \a bKeepComments is true, all block and line comments
  /// are included in the output string, otherwise they are removed.
  wdResult Process(const char* szMainFile, wdStringBuilder& ref_sOutput, bool bKeepComments = true, bool bRemoveRedundantWhitespace = false, bool bInsertLine = false);


private:
  struct FileData
  {
    FileData()
    {
      m_iCurrentLine = 1;
      m_iExpandDepth = 0;
    }

    wdHashedString m_sVirtualFileName;
    wdHashedString m_sFileName;
    wdInt32 m_iCurrentLine;
    wdInt32 m_iExpandDepth;
  };

  enum IfDefActivity
  {
    IsActive,
    IsInactive,
    WasActive,
  };

  struct CustomDefine
  {
    wdHybridArray<wdUInt8, 64> m_Content;
    wdTokenizer m_Tokenized;
  };

  // This class-local allocator is used to get rid of some of the memory allocation
  // tracking that would otherwise occur for allocations made by the preprocessor.
  // If changing its position in the class, make sure it always comes before all
  // other members that depend on it to ensure deallocations in those members
  // happen before the allocator get destroyed.
  wdAllocator<wdMemoryPolicies::wdHeapAllocation, wdMemoryTrackingFlags::None> m_ClassAllocator;

  bool m_bPassThroughPragma;
  bool m_bPassThroughLine;
  PassThroughUnknownCmdCB m_PassThroughUnknownCmdCB;

  // this file cache is used as long as the user does not provide his own
  wdTokenizedFileCache m_InternalFileCache;

  // pointer to the file cache that is in use
  wdTokenizedFileCache* m_pUsedFileCache;

  wdDeque<FileData> m_CurrentFileStack;

  wdLogInterface* m_pLog;

  wdDeque<CustomDefine> m_CustomDefines;

  struct IfDefState
  {
    IfDefState(IfDefActivity activeState = IfDefActivity::IsActive)
      : m_ActiveState(activeState)
      , m_bIsInElseClause(false)
    {
    }

    IfDefActivity m_ActiveState;
    bool m_bIsInElseClause;
  };

  wdDeque<IfDefState> m_IfdefActiveStack;

  wdResult ProcessFile(const char* szFile, wdTokenParseUtils::TokenStream& TokenOutput);
  wdResult ProcessCmd(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& TokenOutput);

public:
  static wdResult DefaultFileLocator(const char* szCurAbsoluteFile, const char* szIncludeFile, wdPreprocessor::IncludeType incType, wdStringBuilder& out_sAbsoluteFilePath);
  static wdResult DefaultFileOpen(const char* szAbsoluteFile, wdDynamicArray<wdUInt8>& ref_fileContent, wdTimestamp& out_fileModification);

private: // *** File Handling ***
  wdResult OpenFile(const char* szFile, const wdTokenizer** pTokenizer);

  FileOpenCB m_FileOpenCallback;
  FileLocatorCB m_FileLocatorCallback;
  wdSet<wdTempHashedString> m_PragmaOnce;

private: // *** Macro Definition ***
  bool RemoveDefine(const char* szName);
  wdResult HandleDefine(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken);

  struct MacroDefinition
  {
    MacroDefinition();

    const wdToken* m_MacroIdentifier;
    bool m_bIsFunction;
    bool m_bCurrentlyExpanding;
    bool m_bHasVarArgs;
    wdInt32 m_iNumParameters;
    wdTokenParseUtils::TokenStream m_Replacement;
  };

  wdResult StoreDefine(const wdToken* pMacroNameToken, const wdTokenParseUtils::TokenStream* pReplacementTokens, wdUInt32 uiFirstReplacementToken, wdInt32 iNumParameters, bool bUsesVarArgs);
  wdResult ExtractParameterName(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdString& sIdentifierName);

  wdMap<wdString256, MacroDefinition> m_Macros;

  static const wdInt32 s_iMacroParameter0 = wdTokenType::ENUM_COUNT + 2;
  static wdString s_ParamNames[32];
  wdToken m_ParameterTokens[32];

private: // *** #if condition parsing ***
  wdResult EvaluateCondition(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseCondition(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseFactor(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionMul(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionOr(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionAnd(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionPlus(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionShift(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionBitOr(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionBitAnd(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);
  wdResult ParseExpressionBitXor(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdInt64& iResult);


private: // *** Parsing ***
  wdResult CopyTokensAndEvaluateDefined(const wdTokenParseUtils::TokenStream& Source, wdUInt32 uiFirstSourceToken, wdTokenParseUtils::TokenStream& Destination);
  void CopyTokensReplaceParams(const wdTokenParseUtils::TokenStream& Source, wdUInt32 uiFirstSourceToken, wdTokenParseUtils::TokenStream& Destination, const wdHybridArray<wdString, 16>& parameters);

  wdResult Expect(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, const char* szToken, wdUInt32* pAccepted = nullptr);
  wdResult Expect(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdTokenType::Enum Type, wdUInt32* pAccepted = nullptr);
  wdResult Expect(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, const char* szToken1, const char* szToken2, wdUInt32* pAccepted = nullptr);
  wdResult ExpectEndOfLine(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken);

private: // *** Macro Expansion ***
  wdResult Expand(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& Output);
  wdResult ExpandOnce(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& Output);
  wdResult ExpandObjectMacro(MacroDefinition& Macro, wdTokenParseUtils::TokenStream& Output, const wdToken* pMacroToken);
  wdResult ExpandFunctionMacro(MacroDefinition& Macro, const MacroParameters& Parameters, wdTokenParseUtils::TokenStream& Output, const wdToken* pMacroToken);
  wdResult ExpandMacroParam(const wdToken& MacroToken, wdUInt32 uiParam, wdTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  void PassThroughFunctionMacro(MacroDefinition& Macro, const MacroParameters& Parameters, wdTokenParseUtils::TokenStream& Output);
  wdToken* AddCustomToken(const wdToken* pPrevious, const wdStringView& sNewText);
  void OutputNotExpandableMacro(MacroDefinition& Macro, wdTokenParseUtils::TokenStream& Output);
  wdResult ExtractAllMacroParameters(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdDeque<wdTokenParseUtils::TokenStream>& AllParameters);
  wdResult ExtractParameterValue(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32& uiCurToken, wdTokenParseUtils::TokenStream& ParamTokens);

  wdResult InsertParameters(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);

  wdResult InsertStringifiedParameters(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  wdResult ConcatenateParameters(const wdTokenParseUtils::TokenStream& Tokens, wdTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  void MergeTokens(const wdToken* pFirst, const wdToken* pSecond, wdTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);

  struct CustomToken
  {
    wdToken m_Token;
    wdString m_sIdentifierString;
  };

  enum TokenFlags : wdUInt32
  {
    NoFurtherExpansion = WD_BIT(0),
  };

  wdToken m_TokenFile;
  wdToken m_TokenLine;
  const wdToken* m_pTokenOpenParenthesis;
  const wdToken* m_pTokenClosedParenthesis;
  const wdToken* m_pTokenComma;

  wdDeque<const MacroParameters*> m_MacroParamStack;
  wdDeque<const MacroParameters*> m_MacroParamStackExpanded;
  wdDeque<CustomToken> m_CustomTokens;

private: // *** Other ***
  static void StringifyTokens(const wdTokenParseUtils::TokenStream& Tokens, wdStringBuilder& sResult, bool bSurroundWithQuotes);
  wdToken* CreateStringifiedParameter(wdUInt32 uiParam, const wdToken* pParamToken, const MacroDefinition& Macro);

  wdResult HandleErrorDirective(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleWarningDirective(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleUndef(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);

  wdResult HandleEndif(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleElif(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleIf(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleElse(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken);
  wdResult HandleIfdef(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken, bool bIsIfdef);
  wdResult HandleInclude(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken, wdTokenParseUtils::TokenStream& TokenOutput);
  wdResult HandleLine(const wdTokenParseUtils::TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken, wdTokenParseUtils::TokenStream& TokenOutput);
};

#define PP_LOG0(Type, FormatStr, ErrorToken)                                                                                                        \
  {                                                                                                                                                 \
    ProcessingEvent pe;                                                                                                                             \
    pe.m_Type = ProcessingEvent::Type;                                                                                                              \
    pe.m_pToken = ErrorToken;                                                                                                                       \
    pe.m_szInfo = FormatStr;                                                                                                                        \
    if (pe.m_pToken->m_uiLine == 0 && pe.m_pToken->m_uiColumn == 0)                                                                                 \
    {                                                                                                                                               \
      const_cast<wdToken*>(pe.m_pToken)->m_uiLine = m_CurrentFileStack.PeekBack().m_iCurrentLine;                                                   \
      const_cast<wdToken*>(pe.m_pToken)->m_File = m_CurrentFileStack.PeekBack().m_sVirtualFileName;                                                 \
    }                                                                                                                                               \
    m_ProcessingEvents.Broadcast(pe);                                                                                                               \
    wdLog::Type(m_pLog, "File '{0}', Line {1} ({2}): " FormatStr, pe.m_pToken->m_File.GetString(), pe.m_pToken->m_uiLine, pe.m_pToken->m_uiColumn); \
  }

#define PP_LOG(Type, FormatStr, ErrorToken, ...)                                                                                                       \
  {                                                                                                                                                    \
    ProcessingEvent _pe;                                                                                                                               \
    _pe.m_Type = ProcessingEvent::Type;                                                                                                                \
    _pe.m_pToken = ErrorToken;                                                                                                                         \
    if (_pe.m_pToken->m_uiLine == 0 && _pe.m_pToken->m_uiColumn == 0)                                                                                  \
    {                                                                                                                                                  \
      const_cast<wdToken*>(_pe.m_pToken)->m_uiLine = m_CurrentFileStack.PeekBack().m_iCurrentLine;                                                     \
      const_cast<wdToken*>(_pe.m_pToken)->m_File = m_CurrentFileStack.PeekBack().m_sVirtualFileName;                                                   \
    }                                                                                                                                                  \
    wdStringBuilder sInfo;                                                                                                                             \
    sInfo.Format(FormatStr, ##__VA_ARGS__);                                                                                                            \
    _pe.m_szInfo = sInfo.GetData();                                                                                                                    \
    m_ProcessingEvents.Broadcast(_pe);                                                                                                                 \
    wdLog::Type(m_pLog, "File '{0}', Line {1} ({2}): {3}", _pe.m_pToken->m_File.GetString(), _pe.m_pToken->m_uiLine, _pe.m_pToken->m_uiColumn, sInfo); \
  }
