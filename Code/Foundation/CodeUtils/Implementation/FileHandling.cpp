#include <Foundation/FoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Utilities/ConversionUtils.h>

using namespace nsTokenParseUtils;

nsMap<nsString, nsTokenizedFileCache::FileData>::ConstIterator nsTokenizedFileCache::Lookup(const nsString& sFileName) const
{
  NS_LOCK(m_Mutex);
  auto it = m_Cache.Find(sFileName);
  return it;
}

void nsTokenizedFileCache::Remove(const nsString& sFileName)
{
  NS_LOCK(m_Mutex);
  m_Cache.Remove(sFileName);
}

void nsTokenizedFileCache::Clear()
{
  NS_LOCK(m_Mutex);
  m_Cache.Clear();
}

void nsTokenizedFileCache::SkipWhitespace(nsDeque<nsToken>& Tokens, nsUInt32& uiCurToken)
{
  while (uiCurToken < Tokens.GetCount() && (Tokens[uiCurToken].m_iType == nsTokenType::BlockComment || Tokens[uiCurToken].m_iType == nsTokenType::LineComment || Tokens[uiCurToken].m_iType == nsTokenType::Newline || Tokens[uiCurToken].m_iType == nsTokenType::Whitespace))
    ++uiCurToken;
}

const nsTokenizer* nsTokenizedFileCache::Tokenize(const nsString& sFileName, nsArrayPtr<const nsUInt8> fileContent, const nsTimestamp& fileTimeStamp, nsLogInterface* pLog)
{
  NS_LOCK(m_Mutex);

  bool bExisted = false;
  auto it = m_Cache.FindOrAdd(sFileName, &bExisted);
  if (bExisted)
  {
    return &it.Value().m_Tokens;
  }

  auto& data = it.Value();

  data.m_Timestamp = fileTimeStamp;
  nsTokenizer* pTokenizer = &data.m_Tokens;
  pTokenizer->Tokenize(fileContent, pLog);

  nsDeque<nsToken>& Tokens = pTokenizer->GetTokens();

  nsHashedString sFile;
  sFile.Assign(sFileName);

  nsInt32 iLineOffset = 0;

  for (nsUInt32 i = 0; i + 1 < Tokens.GetCount(); ++i)
  {
    const nsUInt32 uiCurLine = Tokens[i].m_uiLine;

    Tokens[i].m_File = sFile;
    Tokens[i].m_uiLine += iLineOffset;

    if (Tokens[i].m_iType == nsTokenType::NonIdentifier && Tokens[i].m_DataView.IsEqual("#"))
    {
      nsUInt32 uiNext = i + 1;

      SkipWhitespace(Tokens, uiNext);

      if (uiNext < Tokens.GetCount() && Tokens[uiNext].m_iType == nsTokenType::Identifier && Tokens[uiNext].m_DataView.IsEqual("line"))
      {
        ++uiNext;
        SkipWhitespace(Tokens, uiNext);

        if (uiNext < Tokens.GetCount() && Tokens[uiNext].m_iType == nsTokenType::Integer)
        {
          nsInt32 iNextLine = 0;

          const nsString sNumber = Tokens[uiNext].m_DataView;
          if (nsConversionUtils::StringToInt(sNumber, iNextLine).Succeeded())
          {
            iLineOffset = (iNextLine - uiCurLine) - 1;

            ++uiNext;
            SkipWhitespace(Tokens, uiNext);

            if (uiNext < Tokens.GetCount())
            {
              if (Tokens[uiNext].m_iType == nsTokenType::String1)
              {
                nsStringBuilder sFileName2 = Tokens[uiNext].m_DataView;
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


void nsPreprocessor::SetLogInterface(nsLogInterface* pLog)
{
  m_pLog = pLog;
}

void nsPreprocessor::SetFileOpenFunction(FileOpenCB openAbsFileCB)
{
  m_FileOpenCallback = openAbsFileCB;
}

void nsPreprocessor::SetFileLocatorFunction(FileLocatorCB locateAbsFileCB)
{
  m_FileLocatorCallback = locateAbsFileCB;
}

nsResult nsPreprocessor::DefaultFileLocator(nsStringView sCurAbsoluteFile, nsStringView sIncludeFile, nsPreprocessor::IncludeType incType, nsStringBuilder& out_sAbsoluteFilePath)
{
  nsStringBuilder& s = out_sAbsoluteFilePath;

  if (incType == nsPreprocessor::RelativeInclude)
  {
    s = sCurAbsoluteFile;
    s.PathParentDirectory();
    s.AppendPath(sIncludeFile);
    s.MakeCleanPath();
  }
  else
  {
    s = sIncludeFile;
    s.MakeCleanPath();
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::DefaultFileOpen(nsStringView sAbsoluteFile, nsDynamicArray<nsUInt8>& ref_fileContent, nsTimestamp& out_fileModification)
{
  nsFileReader r;
  if (r.Open(sAbsoluteFile).Failed())
    return NS_FAILURE;

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
  nsFileStats stats;
  if (nsFileSystem::GetFileStats(sAbsoluteFile, stats).Succeeded())
    out_fileModification = stats.m_LastModificationTime;
#endif

  nsUInt8 Temp[4096];

  while (nsUInt64 uiRead = r.ReadBytes(Temp, 4096))
  {
    ref_fileContent.PushBackRange(nsArrayPtr<nsUInt8>(Temp, (nsUInt32)uiRead));
  }

  return NS_SUCCESS;
}

nsResult nsPreprocessor::OpenFile(nsStringView sFile, const nsTokenizer** pTokenizer)
{
  NS_ASSERT_DEV(m_FileOpenCallback.IsValid(), "OpenFile callback has not been set");
  NS_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "File locator callback has not been set");

  *pTokenizer = nullptr;

  auto it = m_pUsedFileCache->Lookup(sFile);

  if (it.IsValid())
  {
    *pTokenizer = &it.Value().m_Tokens;
    return NS_SUCCESS;
  }

  nsTimestamp stamp;

  nsDynamicArray<nsUInt8> Content;
  if (m_FileOpenCallback(sFile, Content, stamp).Failed())
  {
    nsLog::Error(m_pLog, "Could not open file '{0}'", sFile);
    return NS_FAILURE;
  }

  nsArrayPtr<const nsUInt8> ContentView = Content;

  // the file open callback gives us raw data for the opened file
  // the tokenizer doesn't like the Utf8 BOM, so skip it here, if we detect it
  if (ContentView.GetCount() >= 3) // length of a BOM
  {
    const char* dataStart = reinterpret_cast<const char*>(ContentView.GetPtr());

    if (nsUnicodeUtils::SkipUtf8Bom(dataStart))
    {
      ContentView = nsArrayPtr<const nsUInt8>((const nsUInt8*)dataStart, Content.GetCount() - 3);
    }
  }

  *pTokenizer = m_pUsedFileCache->Tokenize(sFile, ContentView, stamp, m_pLog);

  return NS_SUCCESS;
}


nsResult nsPreprocessor::HandleInclude(const TokenStream& Tokens0, nsUInt32 uiCurToken, nsUInt32 uiDirectiveToken, TokenStream& TokenOutput)
{
  NS_ASSERT_DEV(m_FileLocatorCallback.IsValid(), "File locator callback has not been set");

  TokenStream Tokens;
  if (Expand(Tokens0, Tokens).Failed())
    return NS_FAILURE;

  SkipWhitespace(Tokens, uiCurToken);

  nsStringBuilder sPath;

  IncludeType IncType = IncludeType::GlobalInclude;


  nsUInt32 uiAccepted;
  if (Accept(Tokens, uiCurToken, nsTokenType::String1, &uiAccepted))
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
      return NS_FAILURE;

    TokenStream PathTokens;

    while (uiCurToken < Tokens.GetCount())
    {
      if (Tokens[uiCurToken]->m_iType == nsTokenType::Newline)
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
      return NS_FAILURE;
    }
  }

  if (ExpectEndOfLine(Tokens, uiCurToken).Failed())
  {
    PP_LOG0(Error, "Expected end-of-line", Tokens[uiCurToken]);
    return NS_FAILURE;
  }

  NS_ASSERT_DEV(!m_CurrentFileStack.IsEmpty(), "Implementation error.");

  nsStringBuilder sOtherFile;

  if (m_FileLocatorCallback(m_CurrentFileStack.PeekBack().m_sFileName.GetData(), sPath, IncType, sOtherFile).Failed())
  {
    PP_LOG(Error, "#include file '{0}' could not be located", Tokens[uiAccepted], sPath);
    return NS_FAILURE;
  }

  const nsTempHashedString sOtherFileHashed(sOtherFile);

  // if this has been included before, and contains a #pragma once, do not include it again
  if (m_PragmaOnce.Find(sOtherFileHashed).IsValid())
    return NS_SUCCESS;

  if (ProcessFile(sOtherFile, TokenOutput).Failed())
    return NS_FAILURE;

  if (uiCurToken < Tokens.GetCount() && (Tokens[uiCurToken]->m_iType == nsTokenType::Newline || Tokens[uiCurToken]->m_iType == nsTokenType::EndOfFile))
    TokenOutput.PushBack(Tokens[uiCurToken]);

  return NS_SUCCESS;
}
