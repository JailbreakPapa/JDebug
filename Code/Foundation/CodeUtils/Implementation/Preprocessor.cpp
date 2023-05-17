#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

wdString wdPreprocessor::s_ParamNames[32];

using namespace wdTokenParseUtils;

wdPreprocessor::wdPreprocessor()
  : m_ClassAllocator("wdPreprocessor", wdFoundation::GetDefaultAllocator())
  , m_CurrentFileStack(&m_ClassAllocator)
  , m_CustomDefines(&m_ClassAllocator)
  , m_IfdefActiveStack(&m_ClassAllocator)
  , m_Macros(wdCompareHelper<wdString256>(), &m_ClassAllocator)
  , m_MacroParamStack(&m_ClassAllocator)
  , m_MacroParamStackExpanded(&m_ClassAllocator)
  , m_CustomTokens(&m_ClassAllocator)
{
  SetCustomFileCache();
  m_pLog = nullptr;

  m_bPassThroughPragma = false;
  m_bPassThroughLine = false;

  m_FileLocatorCallback = DefaultFileLocator;
  m_FileOpenCallback = DefaultFileOpen;

  wdStringBuilder s;
  for (wdUInt32 i = 0; i < 32; ++i)
  {
    s.Format("__Param{0}__", i);
    s_ParamNames[i] = s;

    m_ParameterTokens[i].m_iType = s_iMacroParameter0 + i;
    m_ParameterTokens[i].m_DataView = s_ParamNames[i].GetView();
  }

  wdToken dummy;
  dummy.m_iType = wdTokenType::NonIdentifier;

  m_pTokenOpenParenthesis = AddCustomToken(&dummy, "(");
  m_pTokenClosedParenthesis = AddCustomToken(&dummy, ")");
  m_pTokenComma = AddCustomToken(&dummy, ",");
}

void wdPreprocessor::SetCustomFileCache(wdTokenizedFileCache* pFileCache)
{
  m_pUsedFileCache = &m_InternalFileCache;

  if (pFileCache != nullptr)
    m_pUsedFileCache = pFileCache;
}

wdToken* wdPreprocessor::AddCustomToken(const wdToken* pPrevious, const wdStringView& sNewText)
{
  CustomToken* pToken = &m_CustomTokens.ExpandAndGetRef();

  pToken->m_sIdentifierString = sNewText;
  pToken->m_Token = *pPrevious;
  pToken->m_Token.m_DataView = pToken->m_sIdentifierString;

  return &pToken->m_Token;
}

wdResult wdPreprocessor::ProcessFile(const char* szFile, TokenStream& TokenOutput)
{
  const wdTokenizer* pTokenizer = nullptr;

  if (OpenFile(szFile, &pTokenizer).Failed())
    return WD_FAILURE;

  FileData fd;
  fd.m_sFileName.Assign(szFile);
  fd.m_sVirtualFileName = fd.m_sFileName;

  m_CurrentFileStack.PushBack(fd);

  wdUInt32 uiNextToken = 0;
  TokenStream TokensLine(&m_ClassAllocator);
  TokenStream TokensCode(&m_ClassAllocator);

  while (pTokenizer->GetNextLine(uiNextToken, TokensLine).Succeeded())
  {
    wdUInt32 uiCurToken = 0;

    // if the line starts with a # it is a preprocessor command
    if (Accept(TokensLine, uiCurToken, "#"))
    {
      // code that was not yet expanded before the command -> expand now
      if (!TokensCode.IsEmpty())
      {
        if (Expand(TokensCode, TokenOutput).Failed())
          return WD_FAILURE;

        TokensCode.Clear();
      }

      // process the command
      if (ProcessCmd(TokensLine, TokenOutput).Failed())
        return WD_FAILURE;
    }
    else
    {
      // we are currently inside an inactive text block
      if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
        continue;

      // store for later expansion
      TokensCode.PushBackRange(TokensLine);
    }
  }

  // some remaining code at the end -> expand
  if (!TokensCode.IsEmpty())
  {
    if (Expand(TokensCode, TokenOutput).Failed())
      return WD_FAILURE;

    TokensCode.Clear();
  }

  m_CurrentFileStack.PopBack();

  return WD_SUCCESS;
}

wdResult wdPreprocessor::Process(const char* szMainFile, TokenStream& ref_tokenOutput)
{
  WD_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "No file locator callback has been set.");

  ref_tokenOutput.Clear();

  // Add a custom define for the __FILE__ macro
  {
    m_TokenFile.m_DataView = wdStringView("__FILE__");
    m_TokenFile.m_iType = wdTokenType::Identifier;

    MacroDefinition md;
    md.m_MacroIdentifier = &m_TokenFile;
    md.m_bIsFunction = false;
    md.m_iNumParameters = 0;
    md.m_bHasVarArgs = false;

    m_Macros.Insert("__FILE__", md);
  }

  // Add a custom define for the __LINE__ macro
  {
    m_TokenLine.m_DataView = wdStringView("__LINE__");
    m_TokenLine.m_iType = wdTokenType::Identifier;

    MacroDefinition md;
    md.m_MacroIdentifier = &m_TokenLine;
    md.m_bIsFunction = false;
    md.m_iNumParameters = 0;
    md.m_bHasVarArgs = false;

    m_Macros.Insert("__LINE__", md);
  }

  m_IfdefActiveStack.Clear();
  m_IfdefActiveStack.PushBack(IfDefActivity::IsActive);

  wdStringBuilder sFileToOpen;
  if (m_FileLocatorCallback("", szMainFile, IncludeType::MainFile, sFileToOpen).Failed())
  {
    wdLog::Error(m_pLog, "Could not locate file '{0}'", szMainFile);
    return WD_FAILURE;
  }

  if (ProcessFile(sFileToOpen, ref_tokenOutput).Failed())
    return WD_FAILURE;

  m_IfdefActiveStack.PopBack();

  if (!m_IfdefActiveStack.IsEmpty())
  {
    wdLog::Error(m_pLog, "Incomplete nesting of #if / #else / #endif");
    return WD_FAILURE;
  }

  if (!m_CurrentFileStack.IsEmpty())
  {
    wdLog::Error(m_pLog, "Internal error, file stack is not empty after processing. {0} elements, top stack item: '{1}'", m_CurrentFileStack.GetCount(), m_CurrentFileStack.PeekBack().m_sFileName);
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::Process(const char* szMainFile, wdStringBuilder& ref_sOutput, bool bKeepComments, bool bRemoveRedundantWhitespace, bool bInsertLine)
{
  ref_sOutput.Clear();

  TokenStream TokenOutput;
  if (Process(szMainFile, TokenOutput).Failed())
    return WD_FAILURE;

  // generate the final text output
  CombineTokensToString(TokenOutput, 0, ref_sOutput, bKeepComments, bRemoveRedundantWhitespace, bInsertLine);

  return WD_SUCCESS;
}

wdResult wdPreprocessor::ProcessCmd(const TokenStream& Tokens, TokenStream& TokenOutput)
{
  wdUInt32 uiCurToken = 0;

  wdUInt32 uiHashToken = 0;

  if (Expect(Tokens, uiCurToken, "#", &uiHashToken).Failed())
    return WD_FAILURE;

  // just a single hash sign is a valid preprocessor line
  if (IsEndOfLine(Tokens, uiCurToken, true))
    return WD_SUCCESS;

  wdUInt32 uiAccepted = uiCurToken;

  // if there is a #pragma once anywhere in the file (not only the active part), it will be flagged to not be included again
  // this is actually more efficient than include guards, because the file is never even looked at again, thus macro expansion
  // does not take place each and every time (which is unfortunately necessary with include guards)
  {
    wdUInt32 uiTempPos = uiCurToken;
    if (Accept(Tokens, uiTempPos, "pragma") && Accept(Tokens, uiTempPos, "once"))
    {
      uiCurToken = uiTempPos;
      m_PragmaOnce.Insert(m_CurrentFileStack.PeekBack().m_sFileName);

      // rather pointless to pass this through, as the output ends up as one big file
      // if (m_bPassThroughPragma)
      //  CopyRelevantTokens(Tokens, uiHashToken, TokenOutput);

      return ExpectEndOfLine(Tokens, uiCurToken);
    }
  }

  if (Accept(Tokens, uiCurToken, "ifdef", &uiAccepted))
    return HandleIfdef(Tokens, uiCurToken, uiAccepted, true);

  if (Accept(Tokens, uiCurToken, "ifndef", &uiAccepted))
    return HandleIfdef(Tokens, uiCurToken, uiAccepted, false);

  if (Accept(Tokens, uiCurToken, "else", &uiAccepted))
    return HandleElse(Tokens, uiCurToken, uiAccepted);

  if (Accept(Tokens, uiCurToken, "if", &uiAccepted))
    return HandleIf(Tokens, uiCurToken, uiAccepted);

  if (Accept(Tokens, uiCurToken, "elif", &uiAccepted))
    return HandleElif(Tokens, uiCurToken, uiAccepted);

  if (Accept(Tokens, uiCurToken, "endif", &uiAccepted))
    return HandleEndif(Tokens, uiCurToken, uiAccepted);

  // we are currently inside an inactive text block, so skip all the following commands
  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    // check that the following command is valid, even if it is ignored
    if (Accept(Tokens, uiCurToken, "line", &uiAccepted) || Accept(Tokens, uiCurToken, "include", &uiAccepted) || Accept(Tokens, uiCurToken, "define") || Accept(Tokens, uiCurToken, "undef", &uiAccepted) || Accept(Tokens, uiCurToken, "error", &uiAccepted) ||
        Accept(Tokens, uiCurToken, "warning", &uiAccepted) || Accept(Tokens, uiCurToken, "pragma"))
      return WD_SUCCESS;

    if (m_PassThroughUnknownCmdCB.IsValid())
    {
      wdString sCmd = Tokens[uiCurToken]->m_DataView;

      if (m_PassThroughUnknownCmdCB(sCmd))
        return WD_SUCCESS;
    }

    PP_LOG0(Error, "Expected a preprocessor command", Tokens[0]);
    return WD_FAILURE;
  }

  if (Accept(Tokens, uiCurToken, "line", &uiAccepted))
    return HandleLine(Tokens, uiCurToken, uiHashToken, TokenOutput);

  if (Accept(Tokens, uiCurToken, "include", &uiAccepted))
    return HandleInclude(Tokens, uiCurToken, uiAccepted, TokenOutput);

  if (Accept(Tokens, uiCurToken, "define"))
    return HandleDefine(Tokens, uiCurToken);

  if (Accept(Tokens, uiCurToken, "undef", &uiAccepted))
    return HandleUndef(Tokens, uiCurToken, uiAccepted);

  if (Accept(Tokens, uiCurToken, "error", &uiAccepted))
    return HandleErrorDirective(Tokens, uiCurToken, uiAccepted);

  if (Accept(Tokens, uiCurToken, "warning", &uiAccepted))
    return HandleWarningDirective(Tokens, uiCurToken, uiAccepted);

  // Pass #line and #pragma commands through unmodified, the user expects them to arrive in the final output properly
  if (Accept(Tokens, uiCurToken, "pragma"))
  {
    if (m_bPassThroughPragma)
      CopyRelevantTokens(Tokens, uiHashToken, TokenOutput, true);

    return WD_SUCCESS;
  }

  if (m_PassThroughUnknownCmdCB.IsValid())
  {
    wdString sCmd = Tokens[uiCurToken]->m_DataView;

    if (m_PassThroughUnknownCmdCB(sCmd))
    {
      TokenOutput.PushBackRange(Tokens);
      return WD_SUCCESS;
    }
  }

  PP_LOG0(Error, "Expected a preprocessor command", Tokens[0]);
  return WD_FAILURE;
}

wdResult wdPreprocessor::HandleLine(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiHashToken, TokenStream& TokenOutput)
{
  // #line directives are just passed through, the actual #line detection is already done by the tokenizer
  // however we check them for validity here

  if (m_bPassThroughLine)
    CopyRelevantTokens(Tokens, uiHashToken, TokenOutput, true);

  wdUInt32 uiNumberToken = 0;
  if (Expect(Tokens, uiCurToken, wdTokenType::Integer, &uiNumberToken).Failed())
    return WD_FAILURE;

  wdInt32 iNextLine = 0;

  const wdString sNumber = Tokens[uiNumberToken]->m_DataView;
  if (wdConversionUtils::StringToInt(sNumber, iNextLine).Failed())
  {
    PP_LOG(Error, "Could not parse '{0}' as a line number", Tokens[uiNumberToken], sNumber);
    return WD_FAILURE;
  }

  wdUInt32 uiFileNameToken = 0;
  if (Accept(Tokens, uiCurToken, wdTokenType::String1, &uiFileNameToken))
  {
    // wdStringBuilder sFileName = Tokens[uiFileNameToken]->m_DataView;
    // sFileName.Shrink(1, 1); // remove surrounding "
    // m_CurrentFileStack.PeekBack().m_sVirtualFileName = sFileName;
  }
  else
  {
    if (ExpectEndOfLine(Tokens, uiCurToken).Failed())
      return WD_FAILURE;
  }

  // there is one case that is not handled here:
  // when the #line directive appears other than '#line number [file]', then the other parameters should be expanded
  // and then checked again for the above form
  // since this is probably not in common use, we ignore this case

  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleIfdef(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken, bool bIsIfdef)
{
  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return WD_SUCCESS;
  }

  wdUInt32 uiIdentifier = uiCurToken;
  if (Expect(Tokens, uiCurToken, wdTokenType::Identifier, &uiIdentifier).Failed())
    return WD_FAILURE;

  const bool bDefined = m_Macros.Find(Tokens[uiIdentifier]->m_DataView).IsValid();

  // broadcast that '#ifdef' is being evaluated
  {
    ProcessingEvent pe;
    pe.m_pToken = Tokens[uiIdentifier];
    pe.m_Type = bIsIfdef ? ProcessingEvent::CheckIfdef : ProcessingEvent::CheckIfndef;
    pe.m_szInfo = bDefined ? "defined" : "undefined";
    m_ProcessingEvents.Broadcast(pe);
  }

  m_IfdefActiveStack.PushBack(bIsIfdef == bDefined ? IfDefActivity::IsActive : IfDefActivity::IsInactive);

  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleElse(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  const IfDefActivity bCur = m_IfdefActiveStack.PeekBack().m_ActiveState;
  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#else'", Tokens[uiDirectiveToken]);
    return WD_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_bIsInElseClause)
  {
    PP_LOG0(Error, "Unexpected '#else'", Tokens[uiDirectiveToken]);
    return WD_FAILURE;
  }

  m_IfdefActiveStack.PeekBack().m_bIsInElseClause = true;

  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return WD_SUCCESS;
  }

  if (bCur == IfDefActivity::WasActive || bCur == IfDefActivity::IsActive)
    m_IfdefActiveStack.PushBack(IfDefActivity::WasActive);
  else
    m_IfdefActiveStack.PushBack(IfDefActivity::IsActive);

  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleIf(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return WD_SUCCESS;
  }

  wdInt64 iResult = 0;

  if (EvaluateCondition(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  m_IfdefActiveStack.PushBack(iResult != 0 ? IfDefActivity::IsActive : IfDefActivity::IsInactive);
  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleElif(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  const IfDefActivity Cur = m_IfdefActiveStack.PeekBack().m_ActiveState;
  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#elif'", Tokens[uiDirectiveToken]);
    return WD_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_bIsInElseClause)
  {
    PP_LOG0(Error, "Unexpected '#elif'", Tokens[uiDirectiveToken]);
    return WD_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return WD_SUCCESS;
  }

  wdInt64 iResult = 0;
  if (EvaluateCondition(Tokens, uiCurToken, iResult).Failed())
    return WD_FAILURE;

  if (Cur != IfDefActivity::IsInactive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::WasActive);
    return WD_SUCCESS;
  }

  m_IfdefActiveStack.PushBack(iResult != 0 ? IfDefActivity::IsActive : IfDefActivity::IsInactive);
  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleEndif(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  WD_SUCCEED_OR_RETURN(ExpectEndOfLine(Tokens, uiCurToken));

  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#endif'", Tokens[uiDirectiveToken]);
    return WD_FAILURE;
  }
  else
  {
    m_IfdefActiveStack.PeekBack().m_bIsInElseClause = false;
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleUndef(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  wdUInt32 uiIdentifierToken = uiCurToken;

  if (Expect(Tokens, uiCurToken, wdTokenType::Identifier, &uiIdentifierToken).Failed())
    return WD_FAILURE;

  const wdString sUndef = Tokens[uiIdentifierToken]->m_DataView;
  if (!RemoveDefine(sUndef))
  {
    PP_LOG(Warning, "'#undef' of undefined macro '{0}'", Tokens[uiIdentifierToken], sUndef);
    return WD_SUCCESS;
  }

  // this is an error, but not one that will cause it to fail
  ExpectEndOfLine(Tokens, uiCurToken).IgnoreResult();

  return WD_SUCCESS;
}

wdResult wdPreprocessor::HandleErrorDirective(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  wdStringBuilder sTemp;
  CombineTokensToString(Tokens, uiCurToken, sTemp);

  while (sTemp.EndsWith("\n") || sTemp.EndsWith("\r"))
    sTemp.Shrink(0, 1);

  PP_LOG(Error, "#error '{0}'", Tokens[uiDirectiveToken], sTemp);

  return WD_FAILURE;
}

wdResult wdPreprocessor::HandleWarningDirective(const TokenStream& Tokens, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  wdStringBuilder sTemp;
  CombineTokensToString(Tokens, uiCurToken, sTemp);

  while (sTemp.EndsWith("\n") || sTemp.EndsWith("\r"))
    sTemp.Shrink(0, 1);

  PP_LOG(Warning, "#warning '{0}'", Tokens[uiDirectiveToken], sTemp);

  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_Preprocessor);
