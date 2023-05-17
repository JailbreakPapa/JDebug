#include <Foundation/FoundationPCH.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/TranslationLookup.h>

bool wdTranslator::s_bHighlightUntranslated = false;
wdHybridArray<wdTranslator*, 4> wdTranslator::s_AllTranslators;

wdTranslator::wdTranslator()
{
  s_AllTranslators.PushBack(this);
}

wdTranslator::~wdTranslator()
{
  s_AllTranslators.RemoveAndSwap(this);
}

void wdTranslator::Reset() {}

void wdTranslator::Reload() {}

void wdTranslator::ReloadAllTranslators()
{
  WD_LOG_BLOCK("ReloadAllTranslators");

  for (wdTranslator* pTranslator : s_AllTranslators)
  {
    pTranslator->Reload();
  }
}

void wdTranslator::HighlightUntranslated(bool bHighlight)
{
  if (s_bHighlightUntranslated == bHighlight)
    return;

  s_bHighlightUntranslated = bHighlight;

  ReloadAllTranslators();
}

//////////////////////////////////////////////////////////////////////////

wdHybridArray<wdUniquePtr<wdTranslator>, 16> wdTranslationLookup::s_Translators;

void wdTranslationLookup::AddTranslator(wdUniquePtr<wdTranslator> pTranslator)
{
  s_Translators.PushBack(std::move(pTranslator));
}


const char* wdTranslationLookup::Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  for (wdUInt32 i = s_Translators.GetCount(); i > 0; --i)
  {
    const char* szResult = s_Translators[i - 1]->Translate(szString, uiStringHash, usage);

    if (szResult != nullptr)
      return szResult;
  }

  return szString;
}


void wdTranslationLookup::Clear()
{
  s_Translators.Clear();
}

//////////////////////////////////////////////////////////////////////////

void wdTranslatorFromFiles::AddTranslationFilesFromFolder(const char* szFolder)
{
  WD_LOG_BLOCK("AddTranslationFilesFromFolder", szFolder);

  if (!m_Folders.Contains(szFolder))
  {
    m_Folders.PushBack(szFolder);
  }

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)
  wdStringBuilder startPath;
  if (wdFileSystem::ResolvePath(szFolder, &startPath, nullptr).Failed())
    return;

  wdStringBuilder fullpath;

  wdFileSystemIterator it;
  it.StartSearch(startPath, wdFileSystemIteratorFlags::ReportFilesRecursive);


  while (it.IsValid())
  {
    fullpath = it.GetCurrentPath();
    fullpath.AppendPath(it.GetStats().m_sName);

    LoadTranslationFile(fullpath);

    it.Next();
  }

#endif
}

const char* wdTranslatorFromFiles::Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  return wdTranslatorStorage::Translate(szString, uiStringHash, usage);
}

void wdTranslatorFromFiles::Reload()
{
  wdTranslatorStorage::Reload();

  for (const auto& sFolder : m_Folders)
  {
    AddTranslationFilesFromFolder(sFolder);
  }
}

void wdTranslatorFromFiles::LoadTranslationFile(const char* szFullPath)
{
  WD_LOG_BLOCK("LoadTranslationFile", szFullPath);

  wdLog::Dev("Loading Localization File '{0}'", szFullPath);

  wdFileReader file;
  if (file.Open(szFullPath).Failed())
  {
    wdLog::Warning("Failed to open localization file '{0}'", szFullPath);
    return;
  }

  wdStringBuilder sContent;
  sContent.ReadAll(file);

  wdDeque<wdStringView> Lines;
  sContent.Split(false, Lines, "\n");

  wdHybridArray<wdStringView, 4> entries;

  wdStringBuilder sLine, sKey, sValue, sTooltip, sHelpUrl;
  for (const auto& line : Lines)
  {
    sLine = line;
    sLine.Trim(" \t\r\n");

    if (sLine.IsEmpty() || sLine.StartsWith("#"))
      continue;

    entries.Clear();
    sLine.Split(true, entries, ";");

    if (entries.GetCount() <= 1)
    {
      wdLog::Error("Invalid line in translation file: '{0}'", sLine);
      continue;
    }

    sKey = entries[0];
    sValue = entries[1];

    sTooltip.Clear();
    sHelpUrl.Clear();

    if (entries.GetCount() >= 3)
      sTooltip = entries[2];
    if (entries.GetCount() >= 4)
      sHelpUrl = entries[3];

    sKey.Trim(" \t\r\n");
    sValue.Trim(" \t\r\n");
    sTooltip.Trim(" \t\r\n");
    sHelpUrl.Trim(" \t\r\n");

    if (GetHighlightUntranslated())
    {
      sValue.Prepend("# ");
      sValue.Append(" (@", sKey, ")");
    }

    StoreTranslation(sValue, wdHashingUtils::StringHash(sKey), wdTranslationUsage::Default);
    StoreTranslation(sTooltip, wdHashingUtils::StringHash(sKey), wdTranslationUsage::Tooltip);
    StoreTranslation(sHelpUrl, wdHashingUtils::StringHash(sKey), wdTranslationUsage::HelpURL);
  }
}

//////////////////////////////////////////////////////////////////////////

void wdTranslatorStorage::StoreTranslation(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  m_Translations[(wdUInt32)usage][uiStringHash] = szString;
}

const char* wdTranslatorStorage::Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  auto it = m_Translations[(wdUInt32)usage].Find(uiStringHash);
  if (it.IsValid())
    return it.Value().GetData();

  return nullptr;
}

void wdTranslatorStorage::Reset()
{
  for (wdUInt32 i = 0; i < (wdUInt32)wdTranslationUsage::ENUM_COUNT; ++i)
  {
    m_Translations[i].Clear();
  }
}

void wdTranslatorStorage::Reload()
{
  Reset();
}

//////////////////////////////////////////////////////////////////////////

bool wdTranslatorLogMissing::s_bActive = true;

const char* wdTranslatorLogMissing::Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  if (!wdTranslatorLogMissing::s_bActive && !GetHighlightUntranslated())
    return nullptr;

  if (usage != wdTranslationUsage::Default)
    return nullptr;

  const char* szResult = wdTranslatorStorage::Translate(szString, uiStringHash, usage);

  if (szResult == nullptr)
  {
    wdLog::Warning("Missing translation: {0};", szString);

    StoreTranslation(szString, uiStringHash, usage);
  }

  return nullptr;
}

const char* wdTranslatorMakeMoreReadable::Translate(const char* szString, wdUInt64 uiStringHash, wdTranslationUsage usage)
{
  const char* szResult = wdTranslatorStorage::Translate(szString, uiStringHash, usage);

  if (szResult != nullptr)
    return szResult;


  wdStringBuilder result;
  wdStringBuilder tmp = szString;
  tmp.Trim(" _-");
  tmp.TrimWordStart("wd");
  tmp.TrimWordEnd("Component");

  auto IsUpper = [](wdUInt32 c) { return c == wdStringUtils::ToUpperChar(c); };
  auto IsNumber = [](wdUInt32 c) { return c >= '0' && c <= '9'; };

  wdUInt32 uiPrev = ' ';
  wdUInt32 uiCur = ' ';
  wdUInt32 uiNext = ' ';

  bool bContinue = true;

  for (auto it = tmp.GetIteratorFront(); bContinue; ++it)
  {
    uiPrev = uiCur;
    uiCur = uiNext;

    if (it.IsValid())
    {
      uiNext = it.GetCharacter();
    }
    else
    {
      uiNext = ' ';
      bContinue = false;
    }

    if (uiCur == '_')
      uiCur = ' ';

    if (uiCur == ':')
    {
      result.Clear();
      continue;
    }

    if (!IsNumber(uiPrev) && IsNumber(uiCur))
    {
      result.Append(" ");
      result.Append(uiCur);
      continue;
    }

    if (IsUpper(uiPrev) && IsUpper(uiCur) && !IsUpper(uiNext))
    {
      result.Append(" ");
      result.Append(uiCur);
      continue;
    }

    if (!IsUpper(uiCur) && IsUpper(uiNext))
    {
      result.Append(uiCur);
      result.Append(" ");
      continue;
    }

    result.Append(uiCur);
  }

  result.Trim(" ");

  if (GetHighlightUntranslated())
  {
    result.Append(" (@", szString, ")");
  }

  StoreTranslation(result, uiStringHash, usage);

  return wdTranslatorStorage::Translate(szString, uiStringHash, usage);
}

WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_TranslationLookup);
