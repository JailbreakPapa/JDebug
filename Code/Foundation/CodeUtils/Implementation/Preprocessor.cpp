#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>

nsString nsPreprocessor::s_ParamNames[32];

using namespace nsTokenParseUtils;

nsPreprocessor::nsPreprocessor()
  : m_ClassAllocator("nsPreprocessor", nsFoundation::GetDefaultAllocator())
  , m_CurrentFileStack(&m_ClassAllocator)
  , m_CustomDefines(&m_ClassAllocator)
  , m_IfdefActiveStack(&m_ClassAllocator)
  , m_Macros(nsCompareHelper<nsString256>(), &m_ClassAllocator)
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

  nsStringBuilder s;
  for (nsUInt32 i = 0; i < 32; ++i)
  {
    s.SetFormat("__Param{0}__", i);
    s_ParamNames[i] = s;

    m_ParameterTokens[i].m_iType = s_iMacroParameter0 + i;
    m_ParameterTokens[i].m_DataView = s_ParamNames[i].GetView();
  }

  nsToken dummy;
  dummy.m_iType = nsTokenType::NonIdentifier;

  m_pTokenOpenParenthesis = AddCustomToken(&dummy, "(");
  m_pTokenClosedParenthesis = AddCustomToken(&dummy, ")");
  m_pTokenComma = AddCustomToken(&dummy, ",");
}

void nsPreprocessor::SetCustomFileCache(nsTokenizedFileCache* pFileCache)
{
  m_pUsedFileCache = &m_InternalFileCache;

  if (pFileCache != nullptr)
    m_pUsedFileCache = pFileCache;
}

nsToken* nsPreprocessor::AddCustomToken(const nsToken* pPrevious, const nsStringView& sNewText)
{
  CustomToken* pToken = &m_CustomTokens.ExpandAndGetRef();

  pToken->m_sIdentifierString = sNewText;
  pToken->m_Token = *pPrevious;
  pToken->m_Token.m_DataView = pToken->m_sIdentifierString;

  return &pToken->m_Token;
}

nsResult nsPreprocessor::ProcessFile(nsStringView sFile, TokenStream& TokenOutput)
{
  const nsTokenizer* pTokenizer = nullptr;

  if (OpenFile(sFile, &pTokenizer).Failed())
    return NS_FAILURE;

  FileData fd;
  fd.m_sFileName.Assign(sFile);
  fd.m_sVirtualFileName = fd.m_sFileName;

  m_CurrentFileStack.PushBack(fd);

  nsUInt32 uiNextToken = 0;
  TokenStream TokensLine(&m_ClassAllocator);
  TokenStream TokensCode(&m_ClassAllocator);

  while (pTokenizer->GetNextLine(uiNextToken, TokensLine).Succeeded())
  {
    nsUInt32 uiCurToken = 0;

    // if the line starts with a # it is a preprocessor command
    if (Accept(TokensLine, uiCurToken, "#"))
    {
      // code that was not yet expanded before the command -> expand now
      if (!TokensCode.IsEmpty())
      {
        if (Expand(TokensCode, TokenOutput).Failed())
          return NS_FAILURE;

        TokensCode.Clear();
      }

      // process the command
      if (ProcessCmd(TokensLine, TokenOutput).Failed())
        return NS_FAILURE;
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
      return NS_FAILURE;

    TokensCode.Clear();
  }

  m_CurrentFileStack.PopBack();

  return NS_SUCCESS;
}

nsResult nsPreprocessor::Process(nsStringView sMainFile, TokenStream& ref_tokenOutput)
{
  NS_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "No file locator callback has been set.");

  ref_tokenOutput.Clear();

  // Add a custom define for the __FILE__ macro
  {
    m_TokenFile.m_DataView = nsStringView("__FILE__");
    m_TokenFile.m_iType = nsTokenType::Identifier;

    MacroDefinition md;
    md.m_MacroIdentifier = &m_TokenFile;
    md.m_bIsFunction = false;
    md.m_iNumParameters = 0;
    md.m_bHasVarArgs = false;

    m_Macros.Insert("__FILE__", md);
  }

  // Add a custom define for the __LINE__ macro
  {
    m_TokenLine.m_DataView = nsStringView("__LINE__");
    m_TokenLine.m_iType = nsTokenType::Identifier;

    MacroDefinition md;
    md.m_MacroIdentifier = &m_TokenLine;
    md.m_bIsFunction = false;
    md.m_iNumParameters = 0;
    md.m_bHasVarArgs = false;

    m_Macros.Insert("__LINE__", md);
  }

  m_IfdefActiveStack.Clear();
  m_IfdefActiveStack.PushBack(IfDefActivity::IsActive);

  nsStringBuilder sFileToOpen;
  if (m_FileLocatorCallback("", sMainFile, IncludeType::MainFile, sFileToOpen).Failed())
  {
    nsLog::Error(m_pLog, "Could not locate file '{0}'", sMainFile);
    return NS_FAILURE;
  }

  if (ProcessFile(sFileToOpen, ref_tokenOutput).Failed())
    return NS_FAILURE;

  m_IfdefActiveStack.PopBack();

  if (!m_IfdefActiveStack.IsEmpty())
  {
    nsLog::Error(m_pLog, "Incomplete nesting of #if / #else / #endif");
    return NS_FAILURE;
  }

  if (!m_CurrentFileStack.IsEmpty())
  {
    nsLog::Error(m_pLog, "Internal error, file stack is not empty after processing. {0} elements, top stack item: '{1}'", m_CurrentFileStack.GetCount(), m_CurrentFileStack.PeekBack().m_sFileName);
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::Process(nsStringView sMainFile, nsStringBuilder& ref_sOutput, bool bKeepComments, bool bRemoveRedundantWhitespace, bool bInsertLine)
{
  ref_sOutput.Clear();

  TokenStream TokenOutput;
  if (Process(sMainFile, TokenOutput).Failed())
    return NS_FAILURE;

  // generate the final text output
  CombineTokensToString(TokenOutput, 0, ref_sOutput, bKeepComments, bRemoveRedundantWhitespace, bInsertLine);

  return NS_SUCCESS;
}

nsResult nsPreprocessor::ProcessCmd(const TokenStream& Tokens, TokenStream& TokenOutput)
{
  nsUInt32 uiCurToken = 0;

  nsUInt32 uiHashToken = 0;

  if (Expect(Tokens, uiCurToken, "#", &uiHashToken).Failed())
    return NS_FAILURE;

  // just a single hash sign is a valid preprocessor line
  if (IsEndOfLine(Tokens, uiCurToken, true))
    return NS_SUCCESS;

  nsUInt32 uiAccepted = uiCurToken;

  // if there is a #pragma once anywhere in the file (not only the active part), it will be flagged to not be included again
  // this is actually more efficient than include guards, because the file is never even looked at again, thus macro expansion
  // does not take place each and every time (which is unfortunately necessary with include guards)
  {
    nsUInt32 uiTempPos = uiCurToken;
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
      return NS_SUCCESS;

    if (m_PassThroughUnknownCmdCB.IsValid())
    {
      nsString sCmd = Tokens[uiCurToken]->m_DataView;

      if (m_PassThroughUnknownCmdCB(sCmd))
        return NS_SUCCESS;
    }

    PP_LOG0(Error, "Expected a preprocessor command", Tokens[0]);
    return NS_FAILURE;
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

    return NS_SUCCESS;
  }

  if (m_PassThroughUnknownCmdCB.IsValid())
  {
    nsString sCmd = Tokens[uiCurToken]->m_DataView;

    if (m_PassThroughUnknownCmdCB(sCmd))
    {
      TokenOutput.PushBackRange(Tokens);
      return NS_SUCCESS;
    }
  }

  PP_LOG0(Error, "Expected a preprocessor command", Tokens[0]);
  return NS_FAILURE;
}

nsResult nsPreprocessor::HandleLine(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiHashToken, TokenStream& TokenOutput)
{
  // #line directives are just passed through, the actual #line detection is already done by the tokenizer
  // however we check them for validity here

  if (m_bPassThroughLine)
    CopyRelevantTokens(Tokens, uiHashToken, TokenOutput, true);

  nsUInt32 uiNumberToken = 0;
  if (Expect(Tokens, uiCurToken, nsTokenType::Integer, &uiNumberToken).Failed())
    return NS_FAILURE;

  nsInt32 iNextLine = 0;

  const nsString sNumber = Tokens[uiNumberToken]->m_DataView;
  if (nsConversionUtils::StringToInt(sNumber, iNextLine).Failed())
  {
    PP_LOG(Error, "Could not parse '{0}' as a line number", Tokens[uiNumberToken], sNumber);
    return NS_FAILURE;
  }

  nsUInt32 uiFileNameToken = 0;
  if (Accept(Tokens, uiCurToken, nsTokenType::String1, &uiFileNameToken))
  {
    // nsStringBuilder sFileName = Tokens[uiFileNameToken]->m_DataView;
    // sFileName.Shrink(1, 1); // remove surrounding "
    // m_CurrentFileStack.PeekBack().m_sVirtualFileName = sFileName;
  }
  else
  {
    if (ExpectEndOfLine(Tokens, uiCurToken).Failed())
      return NS_FAILURE;
  }

  // there is one case that is not handled here:
  // when the #line directive appears other than '#line number [file]', then the other parameters should be expanded
  // and then checked again for the above form
  // since this is probably not in common use, we ignore this case

  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleIfdef(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken, bool bIsIfdef)
{
  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return NS_SUCCESS;
  }

  nsUInt32 uiIdentifier = uiCurToken;
  if (Expect(Tokens, uiCurToken, nsTokenType::Identifier, &uiIdentifier).Failed())
    return NS_FAILURE;

  const bool bDefined = m_Macros.Find(Tokens[uiIdentifier]->m_DataView).IsValid();

  // broadcast that '#ifdef' is being evaluated
  {
    ProcessingEvent pe;
    pe.m_pToken = Tokens[uiIdentifier];
    pe.m_Type = bIsIfdef ? ProcessingEvent::CheckIfdef : ProcessingEvent::CheckIfndef;
    pe.m_sInfo = bDefined ? "defined" : "undefined";
    m_ProcessingEvents.Broadcast(pe);
  }

  m_IfdefActiveStack.PushBack(bIsIfdef == bDefined ? IfDefActivity::IsActive : IfDefActivity::IsInactive);

  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleElse(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  const IfDefActivity bCur = m_IfdefActiveStack.PeekBack().m_ActiveState;
  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#else'", Tokens[uiDirectiveToken]);
    return NS_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_bIsInElseClause)
  {
    PP_LOG0(Error, "Unexpected '#else'", Tokens[uiDirectiveToken]);
    return NS_FAILURE;
  }

  m_IfdefActiveStack.PeekBack().m_bIsInElseClause = true;

  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return NS_SUCCESS;
  }

  if (bCur == IfDefActivity::WasActive || bCur == IfDefActivity::IsActive)
    m_IfdefActiveStack.PushBack(IfDefActivity::WasActive);
  else
    m_IfdefActiveStack.PushBack(IfDefActivity::IsActive);

  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleIf(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return NS_SUCCESS;
  }

  nsInt64 iResult = 0;

  if (EvaluateCondition(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  m_IfdefActiveStack.PushBack(iResult != 0 ? IfDefActivity::IsActive : IfDefActivity::IsInactive);
  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleElif(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  const IfDefActivity Cur = m_IfdefActiveStack.PeekBack().m_ActiveState;
  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#elif'", Tokens[uiDirectiveToken]);
    return NS_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_bIsInElseClause)
  {
    PP_LOG0(Error, "Unexpected '#elif'", Tokens[uiDirectiveToken]);
    return NS_FAILURE;
  }

  if (m_IfdefActiveStack.PeekBack().m_ActiveState != IfDefActivity::IsActive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::IsInactive);
    return NS_SUCCESS;
  }

  nsInt64 iResult = 0;
  if (EvaluateCondition(Tokens, uiCurToken, iResult).Failed())
    return NS_FAILURE;

  if (Cur != IfDefActivity::IsInactive)
  {
    m_IfdefActiveStack.PushBack(IfDefActivity::WasActive);
    return NS_SUCCESS;
  }

  m_IfdefActiveStack.PushBack(iResult != 0 ? IfDefActivity::IsActive : IfDefActivity::IsInactive);
  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleEndif(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  NS_SUCCEED_OR_RETURN(ExpectEndOfLine(Tokens, uiCurToken));

  m_IfdefActiveStack.PopBack();

  if (m_IfdefActiveStack.IsEmpty())
  {
    PP_LOG0(Error, "Unexpected '#endif'", Tokens[uiDirectiveToken]);
    return NS_FAILURE;
  }
  else
  {
    m_IfdefActiveStack.PeekBack().m_bIsInElseClause = false;
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleUndef(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  nsUInt32 uiIdentifierToken = uiCurToken;

  if (Expect(Tokens, uiCurToken, nsTokenType::Identifier, &uiIdentifierToken).Failed())
    return NS_FAILURE;

  const nsString sUndef = Tokens[uiIdentifierToken]->m_DataView;
  if (!RemoveDefine(sUndef))
  {
    PP_LOG(Warning, "'#undef' of undefined macro '{0}'", Tokens[uiIdentifierToken], sUndef);
    return NS_SUCCESS;
  }

  // this is an error, but not one that will cause it to fail
  ExpectEndOfLine(Tokens, uiCurToken).IgnoreResult();

  return NS_SUCCESS;
}

nsResult nsPreprocessor::HandleErrorDirective(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  nsStringBuilder sTemp;
  CombineTokensToString(Tokens, uiCurToken, sTemp);

  while (sTemp.EndsWith("\n") || sTemp.EndsWith("\r"))
    sTemp.Shrink(0, 1);

  PP_LOG(Error, "#error '{0}'", Tokens[uiDirectiveToken], sTemp);

  return NS_FAILURE;
}

nsResult nsPreprocessor::HandleWarningDirective(const TokenStream& Tokens, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken)
{
  SkipWhitespace(Tokens, uiCurToken);

  nsStringBuilder sTemp;
  CombineTokensToString(Tokens, uiCurToken, sTemp);

  while (sTemp.EndsWith("\n") || sTemp.EndsWith("\r"))
    sTemp.Shrink(0, 1);

  PP_LOG(Warning, "#warning '{0}'", Tokens[uiDirectiveToken], sTemp);

  return NS_SUCCESS;
}
