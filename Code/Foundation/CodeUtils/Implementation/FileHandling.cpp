#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/ConversionUtils.h>

using namespace wdTokenParseUtils;

wdMap<wdString, wdTokenizedFileCache::FileData>::ConstIterator wdTokenizedFileCache::Lookup(const wdString& sFileName) const
{
  WD_LOCK(m_Mutex);
  auto it = m_Cache.Find(sFileName);
  return it;
}

void wdTokenizedFileCache::Remove(const wdString& sFileName)
{
  WD_LOCK(m_Mutex);
  m_Cache.Remove(sFileName);
}

void wdTokenizedFileCache::Clear()
{
  WD_LOCK(m_Mutex);
  m_Cache.Clear();
}

void wdTokenizedFileCache::SkipWhitespace(wdDeque<wdToken>& Tokens, wdUInt32& uiCurToken)
{
  while (uiCurToken < Tokens.GetCount() && (Tokens[uiCurToken].m_iType == wdTokenType::BlockComment || Tokens[uiCurToken].m_iType == wdTokenType::LineComment || Tokens[uiCurToken].m_iType == wdTokenType::Newline || Tokens[uiCurToken].m_iType == wdTokenType::Whitespace))
    ++uiCurToken;
}

const wdTokenizer* wdTokenizedFileCache::Tokenize(const wdString& sFileName, wdArrayPtr<const wdUInt8> fileContent, const wdTimestamp& fileTimeStamp, wdLogInterface* pLog)
{
  WD_LOCK(m_Mutex);

  auto& data = m_Cache[sFileName];

  data.m_Timestamp = fileTimeStamp;
  wdTokenizer* pTokenizer = &data.m_Tokens;
  pTokenizer->Tokenize(fileContent, pLog);

  wdDeque<wdToken>& Tokens = pTokenizer->GetTokens();

  wdHashedString sFile;
  sFile.Assign(sFileName);

  wdInt32 iLineOffset = 0;

  for (wdUInt32 i = 0; i + 1 < Tokens.GetCount(); ++i)
  {
    const wdUInt32 uiCurLine = Tokens[i].m_uiLine;

    Tokens[i].m_File = sFile;
    Tokens[i].m_uiLine += iLineOffset;

    if (Tokens[i].m_iType == wdTokenType::NonIdentifier && Tokens[i].m_DataView.IsEqual("#"))
    {
      wdUInt32 uiNext = i + 1;

      SkipWhitespace(Tokens, uiNext);

      if (uiNext < Tokens.GetCount() && Tokens[uiNext].m_iType == wdTokenType::Identifier && Tokens[uiNext].m_DataView.IsEqual("line"))
      {
        ++uiNext;
        SkipWhitespace(Tokens, uiNext);

        if (uiNext < Tokens.GetCount() && Tokens[uiNext].m_iType == wdTokenType::Integer)
        {
          wdInt32 iNextLine = 0;

          const wdString sNumber = Tokens[uiNext].m_DataView;
          if (wdConversionUtils::StringToInt(sNumber, iNextLine).Succeeded())
          {
            iLineOffset = (iNextLine - uiCurLine) - 1;

            ++uiNext;
            SkipWhitespace(Tokens, uiNext);

            if (uiNext < Tokens.GetCount())
            {
              if (Tokens[uiNext].m_iType == wdTokenType::String1)
              {
                wdStringBuilder sFileName2 = Tokens[uiNext].m_DataView;
                sFileName2.Shrink(1, 1); // remove surrounding "

                sFile.Assign(sFileName2);
              }
            }
          }
        }
      }
    }
  }

  return pTokenizer;
}


void wdPreprocessor::SetLogInterface(wdLogInterface* pLog)
{
  m_pLog = pLog;
}

void wdPreprocessor::SetFileOpenFunction(FileOpenCB openAbsFileCB)
{
  m_FileOpenCallback = openAbsFileCB;
}

void wdPreprocessor::SetFileLocatorFunction(FileLocatorCB locateAbsFileCB)
{
  m_FileLocatorCallback = locateAbsFileCB;
}

wdResult wdPreprocessor::DefaultFileLocator(const char* szCurAbsoluteFile, const char* szIncludeFile, wdPreprocessor::IncludeType incType, wdStringBuilder& out_sAbsoluteFilePath)
{
  wdStringBuilder& s = out_sAbsoluteFilePath;

  if (incType == wdPreprocessor::RelativeInclude)
  {
    s = szCurAbsoluteFile;
    s.PathParentDirectory();
    s.AppendPath(szIncludeFile);
    s.MakeCleanPath();
  }
  else
  {
    s = szIncludeFile;
    s.MakeCleanPath();
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::DefaultFileOpen(const char* szAbsoluteFile, wdDynamicArray<wdUInt8>& ref_fileContent, wdTimestamp& out_fileModification)
{
  wdFileReader r;
  if (r.Open(szAbsoluteFile).Failed())
    return WD_FAILURE;

#if WD_ENABLED(WD_SUPPORTS_FILE_STATS)
  wdFileStats stats;
  if (wdFileSystem::GetFileStats(szAbsoluteFile, stats).Succeeded())
    out_fileModification = stats.m_LastModificationTime;
#endif

  wdUInt8 Temp[4096];

  while (wdUInt64 uiRead = r.ReadBytes(Temp, 4096))
  {
    ref_fileContent.PushBackRange(wdArrayPtr<wdUInt8>(Temp, (wdUInt32)uiRead));
  }

  return WD_SUCCESS;
}

wdResult wdPreprocessor::OpenFile(const char* szFile, const wdTokenizer** pTokenizer)
{
  WD_ASSERT_DEV(m_FileOpenCallback.IsValid(), "OpenFile callback has not been set");
  WD_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "File locator callback has not been set");

  *pTokenizer = nullptr;

  auto it = m_pUsedFileCache->Lookup(szFile);

  if (it.IsValid())
  {
    *pTokenizer = &it.Value().m_Tokens;
    return WD_SUCCESS;
  }

  wdTimestamp stamp;

  wdDynamicArray<wdUInt8> Content;
  if (m_FileOpenCallback(szFile, Content, stamp).Failed())
  {
    wdLog::Error(m_pLog, "Could not open file '{0}'", szFile);
    return WD_FAILURE;
  }

  wdArrayPtr<const wdUInt8> ContentView = Content;

  // the file open callback gives us raw data for the opened file
  // the tokenizer doesn't like the Utf8 BOM, so skip it here, if we detect it
  if (ContentView.GetCount() >= 3) // length of a BOM
  {
    const char* dataStart = reinterpret_cast<const char*>(ContentView.GetPtr());

    if (wdUnicodeUtils::SkipUtf8Bom(dataStart))
    {
      ContentView = wdArrayPtr<const wdUInt8>((const wdUInt8*)dataStart, Content.GetCount() - 3);
    }
  }

  *pTokenizer = m_pUsedFileCache->Tokenize(szFile, ContentView, stamp, m_pLog);

  return WD_SUCCESS;
}


wdResult wdPreprocessor::HandleInclude(const TokenStream& Tokens0, wdUInt32 uiCurToken, wdUInt32 uiDirectiveToken, TokenStream& TokenOutput)
{
  WD_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "File locator callback has not been set");

  TokenStream Tokens;
  if (Expand(Tokens0, Tokens).Failed())
    return WD_FAILURE;

  SkipWhitespace(Tokens, uiCurToken);

  wdStringBuilder sPath;

  IncludeType IncType = IncludeType::GlobalInclude;


  wdUInt32 uiAccepted;
  if (Accept(Tokens, uiCurToken, wdTokenType::String1, &uiAccepted))
  {
    IncType = IncludeType::RelativeInclude;
    sPath = Tokens[uiAccepted]->m_DataView;
    sPath.Shrink(1, 1); // remove " at start and end
  }
  else
  {
    // in global include paths (ie. <bla/blub.h>) we need to handle line comments special
    // because a path with two slashes will be a comment token, although it could be a valid path
    // so we concatenate just everything and then make sure it ends with a >

    if (Expect(Tokens, uiCurToken, "<", &uiAccepted).Failed())
      return WD_FAILURE;

    TokenStream PathTokens;

    while (uiCurToken < Tokens.GetCount())
    {
      if (Tokens[uiCurToken]->m_iType == wdTokenType::Newline)
      {
        break;
      }

      PathTokens.PushBack(Tokens[uiCurToken]);
      ++uiCurToken;
    }

    CombineTokensToString(PathTokens, 0, sPath, false);

    // remove all whitespace at the end (this could be part of a comment, so not tokenized as whitespace)
    while (sPath.EndsWith(" ") || sPath.EndsWith("\t"))
      sPath.Shrink(0, 1);

    // there must always be a > at the end, although it could be a separate token or part of a comment
    // so we check the string, instead of the tokens
    if (sPath.EndsWith(">"))
      sPath.Shrink(0, 1);
    else
    {
      PP_LOG(Error, "Invalid include path '{0}'", Tokens[uiAccepted], sPath);
      return WD_FAILURE;
    }
  }

  if (ExpectEndOfLine(Tokens, uiCurToken).Failed())
  {
    PP_LOG0(Error, "Expected end-of-line", Tokens[uiCurToken]);
    return WD_FAILURE;
  }

  WD_ASSERT_DEV(!m_CurrentFileStack.IsEmpty(), "Implementation error.");

  wdStringBuilder sOtherFile;

  if (m_FileLocatorCallback(m_CurrentFileStack.PeekBack().m_sFileName.GetData(), sPath, IncType, sOtherFile).Failed())
  {
    PP_LOG(Error, "#include file '{0}' could not be located", Tokens[uiAccepted], sPath);
    return WD_FAILURE;
  }

  const wdTempHashedString sOtherFileHashed(sOtherFile);

  // if this has been included before, and contains a #pragma once, do not include it again
  if (m_PragmaOnce.Find(sOtherFileHashed).IsValid())
    return WD_SUCCESS;

  if (ProcessFile(sOtherFile, TokenOutput).Failed())
    return WD_FAILURE;

  if (uiCurToken < Tokens.GetCount() && (Tokens[uiCurToken]->m_iType == wdTokenType::Newline || Tokens[uiCurToken]->m_iType == wdTokenType::EndOfFile))
    TokenOutput.PushBack(Tokens[uiCurToken]);

  return WD_SUCCESS;
}



WD_STATICLINK_FILE(Foundation, Foundation_CodeUtils_Implementation_FileHandling);
