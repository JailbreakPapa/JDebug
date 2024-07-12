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

/// \brief This object caches files in a tokenized state. It can be shared among nsPreprocessor instances to improve performance when
/// they access the same files.
class NS_FOUNDATION_DLL nsTokenizedFileCache
{
public:
  struct FileData
  {
    nsTokenizer m_Tokens;
    nsTimestamp m_Timestamp;
  };

  /// \brief Checks whether \a sFileName is already in the cache, returns an iterator to it. If the iterator is invalid, the file is not cached yet.
  nsMap<nsString, FileData>::ConstIterator Lookup(const nsString& sFileName) const;

  /// \brief Removes the cached content for \a sFileName from the cache. Should be used when the file content has changed and needs to be re-read.
  void Remove(const nsString& sFileName);

  /// \brief Removes all files from the cache to ensure that they will be re-read.
  void Clear();

  /// \brief Stores \a FileContent for the file \a sFileName as the new cached data.
  ///
  //// The file content is tokenized first and all #line directives are evaluated, to update the line number and file origin for each token.
  /// Any errors are written to the given log.
  const nsTokenizer* Tokenize(const nsString& sFileName, nsArrayPtr<const nsUInt8> fileContent, const nsTimestamp& fileTimeStamp, nsLogInterface* pLog);

private:
  void SkipWhitespace(nsDeque<nsToken>& Tokens, nsUInt32& uiCurToken);

  mutable nsMutex m_Mutex;
  nsMap<nsString, FileData> m_Cache;
};

/// \brief nsPreprocessor implements a standard C preprocessor. It can be used to pre-process files to get the output after macro expansion and #ifdef
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
class NS_FOUNDATION_DLL nsPreprocessor
{
public:
  /// \brief Describes the type of #include that was encountered during preprocessing
  enum IncludeType
  {
    MainFile,        ///< This is used for the very first access to the main source file
    RelativeInclude, ///< An #include "file" has been encountered
    GlobalInclude    ///< An #include <file> has been encountered
  };

  /// \brief This type of callback is used to read an #include file. \a sAbsoluteFile is the path that the FileLocatorCB reported, the result needs
  /// to be stored in \a FileContent.
  using FileOpenCB = nsDelegate<nsResult(nsStringView, nsDynamicArray<nsUInt8>&, nsTimestamp&)>;

  /// \brief This type of callback is used to retrieve the absolute path of the \a sIncludeFile when #included inside \a sCurAbsoluteFile.
  ///
  /// Note that you should ensure that \a out_sAbsoluteFilePath is always identical (including casing and path slashes) when it is supposed to point
  /// to the same file, as this exact name is used for file lookup (and therefore also file caching).
  /// If it is not identical, file caching will not work, and on different OSes the file may be found or not.
  using FileLocatorCB = nsDelegate<nsResult(nsStringView, nsStringView, IncludeType, nsStringBuilder&)>;

  /// \brief Every time an unknown command (e.g. '#version') is encountered, this callback is used to determine whether the command shall be passed
  /// through.
  ///
  /// If the callback returns false, an error is generated and parsing fails. The callback thus acts as a whitelist for all commands that shall be
  /// passed through.
  using PassThroughUnknownCmdCB = nsDelegate<bool(nsStringView)>;

  using MacroParameters = nsDeque<nsTokenParseUtils::TokenStream>;

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

    EventType m_Type = EventType::Error;

    const nsToken* m_pToken = nullptr;
    nsStringView m_sInfo;
  };

  /// \brief Broadcasts events during the processing. This can be used to create detailed callstacks when an error is encountered.
  /// It also broadcasts errors and warnings with more detailed information than the log interface allows.
  nsEvent<const ProcessingEvent&> m_ProcessingEvents;

  nsPreprocessor();

  /// \brief All error output is sent to the given nsLogInterface.
  ///
  /// Note that when the preprocessor encounters any error, it will stop immediately and usually no output is generated.
  /// However, there are also a few cases where only a warning is generated, in this case preprocessing will continue without problems.
  ///
  /// Additionally errors and warnings are also broadcast through m_ProcessingEvents. So if you want to output more detailed information,
  /// that method should be preferred, because the events carry more information about the current file and line number etc.
  void SetLogInterface(nsLogInterface* pLog);

  /// \brief Allows to specify a custom cache object that should be used for storing the tokenized result of files.
  ///
  /// This allows to share one cache across multiple instances of nsPreprocessor and across time. E.g. it makes it possible
  /// to prevent having to read and tokenize include files that are referenced often.
  void SetCustomFileCache(nsTokenizedFileCache* pFileCache = nullptr);

  /// \brief If set to true, all #pragma commands are passed through to the output, otherwise they are removed.
  void SetPassThroughPragma(bool bPassThrough) { m_bPassThroughPragma = bPassThrough; }

  /// \brief If set to true, all #line commands are passed through to the output, otherwise they are removed.
  void SetPassThroughLine(bool bPassThrough) { m_bPassThroughLine = bPassThrough; }

  /// \brief Sets the callback that is used to determine whether an unknown command is passed through or triggers an error.
  void SetPassThroughUnknownCmdsCB(PassThroughUnknownCmdCB callback) { m_PassThroughUnknownCmdCB = callback; }

  /// \brief Sets the callback that is needed to read input data.
  ///
  /// The default file open function will just try to open files via nsFileReader.
  void SetFileOpenFunction(FileOpenCB openAbsFileCB);

  /// \brief Sets the callback that is needed to locate an input file
  ///
  /// The default file locator will assume that the main source file and all files #included in angle brackets can be opened without modification.
  /// Files #included in "" will be appended as relative paths to the path of the file they appeared in.
  void SetFileLocatorFunction(FileLocatorCB locateAbsFileCB);

  /// \brief Adds a #define to the preprocessor, even before any file is processed.
  ///
  /// This allows to have global macros that are always defined for all processed files, such as the current platform etc.
  /// \a sDefinition must be in the form of the text that follows a #define statement. So to define the macro "WIN32", just
  /// pass that string. You can define any macro that could also be defined in the source files.
  ///
  /// If the definition is invalid, NS_FAILURE is returned. Also the preprocessor might end up in an invalid state, so using it any
  /// further might fail (including crashing).
  nsResult AddCustomDefine(nsStringView sDefinition);

  /// \brief Processes the given file and returns the result as a stream of tokens.
  ///
  /// This function is useful when you want to further process the output afterwards and thus need it in a tokenized form anyway.
  nsResult Process(nsStringView sMainFile, nsTokenParseUtils::TokenStream& ref_tokenOutput);

  /// \brief Processes the given file and returns the result as a string.
  ///
  /// This function creates a string from the tokenized result. If \a bKeepComments is true, all block and line comments
  /// are included in the output string, otherwise they are removed.
  nsResult Process(nsStringView sMainFile, nsStringBuilder& ref_sOutput, bool bKeepComments = true, bool bRemoveRedundantWhitespace = false, bool bInsertLine = false);


private:
  struct FileData
  {
    FileData()
    {
      m_iCurrentLine = 1;
      m_iExpandDepth = 0;
    }

    nsHashedString m_sVirtualFileName;
    nsHashedString m_sFileName;
    nsInt32 m_iCurrentLine;
    nsInt32 m_iExpandDepth;
  };

  enum IfDefActivity
  {
    IsActive,
    IsInactive,
    WasActive,
  };

  struct CustomDefine
  {
    nsHybridArray<nsUInt8, 64> m_Content;
    nsTokenizer m_Tokenized;
  };

  // This class-local allocator is used to get rid of some of the memory allocation
  // tracking that would otherwise occur for allocations made by the preprocessor.
  // If changing its position in the class, make sure it always comes before all
  // other members that depend on it to ensure deallocations in those members
  // happen before the allocator get destroyed.
  nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::Nothing> m_ClassAllocator;

  bool m_bPassThroughPragma;
  bool m_bPassThroughLine;
  PassThroughUnknownCmdCB m_PassThroughUnknownCmdCB;

  // this file cache is used as long as the user does not provide his own
  nsTokenizedFileCache m_InternalFileCache;

  // pointer to the file cache that is in use
  nsTokenizedFileCache* m_pUsedFileCache;

  nsDeque<FileData> m_CurrentFileStack;

  nsLogInterface* m_pLog;

  nsDeque<CustomDefine> m_CustomDefines;

  struct IfDefState
  {
    IfDefState(IfDefActivity activeState = IfDefActivity::IsActive)
      : m_ActiveState(activeState)

    {
    }

    IfDefActivity m_ActiveState;
    bool m_bIsInElseClause = false;
  };

  nsDeque<IfDefState> m_IfdefActiveStack;

  nsResult ProcessFile(nsStringView sFile, nsTokenParseUtils::TokenStream& TokenOutput);
  nsResult ProcessCmd(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& TokenOutput);

public:
  static nsResult DefaultFileLocator(nsStringView sCurAbsoluteFile, nsStringView sIncludeFile, nsPreprocessor::IncludeType incType, nsStringBuilder& out_sAbsoluteFilePath);
  static nsResult DefaultFileOpen(nsStringView sAbsoluteFile, nsDynamicArray<nsUInt8>& ref_fileContent, nsTimestamp& out_fileModification);

private: // *** File Handling ***
  nsResult OpenFile(nsStringView sFile, const nsTokenizer** pTokenizer);

  FileOpenCB m_FileOpenCallback;
  FileLocatorCB m_FileLocatorCallback;
  nsSet<nsTempHashedString> m_PragmaOnce;

private: // *** Macro Definition ***
  bool RemoveDefine(nsStringView sName);
  nsResult HandleDefine(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken);

  struct MacroDefinition
  {
    MacroDefinition();

    const nsToken* m_MacroIdentifier;
    bool m_bIsFunction;
    bool m_bCurrentlyExpanding;
    bool m_bHasVarArgs;
    nsInt32 m_iNumParameters;
    nsTokenParseUtils::TokenStream m_Replacement;
  };

  nsResult StoreDefine(const nsToken* pMacroNameToken, const nsTokenParseUtils::TokenStream* pReplacementTokens, nsUInt32 uiFirstReplacementToken, nsInt32 iNumParameters, bool bUsesVarArgs);
  nsResult ExtractParameterName(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsString& sIdentifierName);

  nsMap<nsString256, MacroDefinition> m_Macros;

  static constexpr nsInt32 s_iMacroParameter0 = nsTokenType::ENUM_COUNT + 2;
  static nsString s_ParamNames[32];
  nsToken m_ParameterTokens[32];

private: // *** #if condition parsing ***
  nsResult EvaluateCondition(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseCondition(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseFactor(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionMul(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionOr(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionAnd(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionPlus(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionShift(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionBitOr(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionBitAnd(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);
  nsResult ParseExpressionBitXor(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsInt64& iResult);


private: // *** Parsing ***
  nsResult CopyTokensAndEvaluateDefined(const nsTokenParseUtils::TokenStream& Source, nsUInt32 uiFirstSourceToken, nsTokenParseUtils::TokenStream& Destination);
  void CopyTokensReplaceParams(const nsTokenParseUtils::TokenStream& Source, nsUInt32 uiFirstSourceToken, nsTokenParseUtils::TokenStream& Destination, const nsHybridArray<nsString, 16>& parameters);

  nsResult Expect(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsStringView sToken, nsUInt32* pAccepted = nullptr);
  nsResult Expect(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsTokenType::Enum Type, nsUInt32* pAccepted = nullptr);
  nsResult Expect(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsStringView sToken1, nsStringView sToken2, nsUInt32* pAccepted = nullptr);
  nsResult ExpectEndOfLine(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken);

private: // *** Macro Expansion ***
  nsResult Expand(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& Output);
  nsResult ExpandOnce(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& Output);
  nsResult ExpandObjectMacro(MacroDefinition& Macro, nsTokenParseUtils::TokenStream& Output, const nsToken* pMacroToken);
  nsResult ExpandFunctionMacro(MacroDefinition& Macro, const MacroParameters& Parameters, nsTokenParseUtils::TokenStream& Output, const nsToken* pMacroToken);
  nsResult ExpandMacroParam(const nsToken& MacroToken, nsUInt32 uiParam, nsTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  void PassThroughFunctionMacro(MacroDefinition& Macro, const MacroParameters& Parameters, nsTokenParseUtils::TokenStream& Output);
  nsToken* AddCustomToken(const nsToken* pPrevious, const nsStringView& sNewText);
  void OutputNotExpandableMacro(MacroDefinition& Macro, nsTokenParseUtils::TokenStream& Output);
  nsResult ExtractAllMacroParameters(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsDeque<nsTokenParseUtils::TokenStream>& AllParameters);
  nsResult ExtractParameterValue(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32& uiCurToken, nsTokenParseUtils::TokenStream& ParamTokens);

  nsResult InsertParameters(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);

  nsResult InsertStringifiedParameters(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  nsResult ConcatenateParameters(const nsTokenParseUtils::TokenStream& Tokens, nsTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);
  void MergeTokens(const nsToken* pFirst, const nsToken* pSecond, nsTokenParseUtils::TokenStream& Output, const MacroDefinition& Macro);

  struct CustomToken
  {
    nsToken m_Token;
    nsString m_sIdentifierString;
  };

  enum TokenFlags : nsUInt32
  {
    NoFurtherExpansion = NS_BIT(0),
  };

  nsToken m_TokenFile;
  nsToken m_TokenLine;
  const nsToken* m_pTokenOpenParenthesis;
  const nsToken* m_pTokenClosedParenthesis;
  const nsToken* m_pTokenComma;

  nsDeque<const MacroParameters*> m_MacroParamStack;
  nsDeque<const MacroParameters*> m_MacroParamStackExpanded;
  nsDeque<CustomToken> m_CustomTokens;

private: // *** Other ***
  static void StringifyTokens(const nsTokenParseUtils::TokenStream& Tokens, nsStringBuilder& sResult, bool bSurroundWithQuotes);
  nsToken* CreateStringifiedParameter(nsUInt32 uiParam, const nsToken* pParamToken, const MacroDefinition& Macro);

  nsResult HandleErrorDirective(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleWarningDirective(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleUndef(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);

  nsResult HandleEndif(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleElif(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleIf(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleElse(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken);
  nsResult HandleIfdef(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken, bool bIsIfdef);
  nsResult HandleInclude(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken, nsTokenParseUtils::TokenStream& TokenOutput);
  nsResult HandleLine(const nsTokenParseUtils::TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken, nsTokenParseUtils::TokenStream& TokenOutput);
};

#define PP_LOG0(Type, FormatStr, ErrorToken)                                                                                                        \
  {                                                                                                                                                 \
    ProcessingEvent pe;                                                                                                                             \
    pe.m_Type = ProcessingEvent::Type;                                                                                                              \
    pe.m_pToken = ErrorToken;                                                                                                                       \
    pe.m_sInfo = FormatStr;                                                                                                                         \
    if (pe.m_pToken->m_uiLine == 0 && pe.m_pToken->m_uiColumn == 0)                                                                                 \
    {                                                                                                                                               \
      const_cast<nsToken*>(pe.m_pToken)->m_uiLine = m_CurrentFileStack.PeekBack().m_iCurrentLine;                                                   \
      const_cast<nsToken*>(pe.m_pToken)->m_File = m_CurrentFileStack.PeekBack().m_sVirtualFileName;                                                 \
    }                                                                                                                                               \
    m_ProcessingEvents.Broadcast(pe);                                                                                                               \
    nsLog::Type(m_pLog, "File '{0}', Line {1} ({2}): " FormatStr, pe.m_pToken->m_File.GetString(), pe.m_pToken->m_uiLine, pe.m_pToken->m_uiColumn); \
  }

#define PP_LOG(Type, FormatStr, ErrorToken, ...)                                                                                                       \
  {                                                                                                                                                    \
    ProcessingEvent _pe;                                                                                                                               \
    _pe.m_Type = ProcessingEvent::Type;                                                                                                                \
    _pe.m_pToken = ErrorToken;                                                                                                                         \
    if (_pe.m_pToken->m_uiLine == 0 && _pe.m_pToken->m_uiColumn == 0)                                                                                  \
    {                                                                                                                                                  \
      const_cast<nsToken*>(_pe.m_pToken)->m_uiLine = m_CurrentFileStack.PeekBack().m_iCurrentLine;                                                     \
      const_cast<nsToken*>(_pe.m_pToken)->m_File = m_CurrentFileStack.PeekBack().m_sVirtualFileName;                                                   \
    }                                                                                                                                                  \
    nsStringBuilder sInfo;                                                                                                                             \
    sInfo.SetFormat(FormatStr, ##__VA_ARGS__);                                                                                                         \
    _pe.m_sInfo = sInfo;                                                                                                                               \
    m_ProcessingEvents.Broadcast(_pe);                                                                                                                 \
    nsLog::Type(m_pLog, "File '{0}', Line {1} ({2}): {3}", _pe.m_pToken->m_File.GetString(), _pe.m_pToken->m_uiLine, _pe.m_pToken->m_uiColumn, sInfo); \
  }
