#include <Foundation/FoundationPCH.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/TranslationLookup.h>

bool nsTranslator::s_bHighlightUntranslated = false;
nsHybridArray<nsTranslator*, 4> nsTranslator::s_AllTranslators;

nsTranslator::nsTranslator()
{
  s_AllTranslators.PushBack(this);
}

nsTranslator::~nsTranslator()
{
  s_AllTranslators.RemoveAndSwap(this);
}

void nsTranslator::Reset() {}

void nsTranslator::Reload() {}

void nsTranslator::ReloadAllTranslators()
{
  NS_LOG_BLOCK("ReloadAllTranslators");

  for (nsTranslator* pTranslator : s_AllTranslators)
  {
    pTranslator->Reload();
  }
}

void nsTranslator::HighlightUntranslated(bool bHighlight)
{
  if (s_bHighlightUntranslated == bHighlight)
    return;

  s_bHighlightUntranslated = bHighlight;

  ReloadAllTranslators();
}

//////////////////////////////////////////////////////////////////////////

nsHybridArray<nsUniquePtr<nsTranslator>, 16> nsTranslationLookup::s_Translators;

void nsTranslationLookup::AddTranslator(nsUniquePtr<nsTranslator> pTranslator)
{
  s_Translators.PushBack(std::move(pTranslator));
}


nsStringView nsTranslationLookup::Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  for (nsUInt32 i = s_Translators.GetCount(); i > 0; --i)
  {
    nsStringView sResult = s_Translators[i - 1]->Translate(sString, uiStringHash, usage);

    if (!sResult.IsEmpty())
      return sResult;
  }

  if (usage != nsTranslationUsage::Default)
    return {};

  return sString;
}


void nsTranslationLookup::Clear()
{
  s_Translators.Clear();
}

//////////////////////////////////////////////////////////////////////////

void nsTranslatorFromFiles::AddTranslationFilesFromFolder(const char* szFolder)
{
  NS_LOG_BLOCK("AddTranslationFilesFromFolder", szFolder);

  if (!m_Folders.Contains(szFolder))
  {
    m_Folders.PushBack(szFolder);
  }

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)
  nsStringBuilder startPath;
  if (nsFileSystem::ResolvePath(szFolder, &startPath, nullptr).Failed())
    return;

  nsStringBuilder fullpath;

  nsFileSystemIterator it;
  it.StartSearch(startPath, nsFileSystemIteratorFlags::ReportFilesRecursive);


  while (it.IsValid())
  {
    fullpath = it.GetCurrentPath();
    fullpath.AppendPath(it.GetStats().m_sName);

    LoadTranslationFile(fullpath);

    it.Next();
  }

#endif
}

nsStringView nsTranslatorFromFiles::Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  return nsTranslatorStorage::Translate(sString, uiStringHash, usage);
}

void nsTranslatorFromFiles::Reload()
{
  nsTranslatorStorage::Reload();

  for (const auto& sFolder : m_Folders)
  {
    AddTranslationFilesFromFolder(sFolder);
  }
}

void nsTranslatorFromFiles::LoadTranslationFile(const char* szFullPath)
{
  NS_LOG_BLOCK("LoadTranslationFile", szFullPath);

  nsLog::Dev("Loading Localization File '{0}'", szFullPath);

  nsFileReader file;
  if (file.Open(szFullPath).Failed())
  {
    nsLog::Warning("Failed to open localization file '{0}'", szFullPath);
    return;
  }

  nsStringBuilder sContent;
  sContent.ReadAll(file);

  nsDeque<nsStringView> Lines;
  sContent.Split(false, Lines, "\n");

  nsHybridArray<nsStringView, 4> entries;

  nsStringBuilder sLine, sKey, sValue, sTooltip, sHelpUrl;
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
      nsLog::Error("Invalid line in translation file: '{0}'", sLine);
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

    StoreTranslation(sValue, nsHashingUtils::StringHash(sKey), nsTranslationUsage::Default);
    StoreTranslation(sTooltip, nsHashingUtils::StringHash(sKey), nsTranslationUsage::Tooltip);
    StoreTranslation(sHelpUrl, nsHashingUtils::StringHash(sKey), nsTranslationUsage::HelpURL);
  }
}

//////////////////////////////////////////////////////////////////////////

void nsTranslatorStorage::StoreTranslation(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  m_Translations[(nsUInt32)usage][uiStringHash] = sString;
}

nsStringView nsTranslatorStorage::Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  auto it = m_Translations[(nsUInt32)usage].Find(uiStringHash);
  if (it.IsValid())
    return it.Value().GetData();

  return {};
}

void nsTranslatorStorage::Reset()
{
  for (nsUInt32 i = 0; i < (nsUInt32)nsTranslationUsage::ENUM_COUNT; ++i)
  {
    m_Translations[i].Clear();
  }
}

void nsTranslatorStorage::Reload()
{
  Reset();
}

//////////////////////////////////////////////////////////////////////////

bool nsTranslatorLogMissing::s_bActive = true;

nsStringView nsTranslatorLogMissing::Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  if (!nsTranslatorLogMissing::s_bActive && !GetHighlightUntranslated())
    return {};

  if (usage != nsTranslationUsage::Default)
    return {};

  nsStringView sResult = nsTranslatorStorage::Translate(sString, uiStringHash, usage);

  if (sResult.IsEmpty())
  {
    nsLog::Warning("Missing translation: {0};", sString);

    StoreTranslation(sString, uiStringHash, usage);
  }

  return {};
}

nsStringView nsTranslatorMakeMoreReadable::Translate(nsStringView sString, nsUInt64 uiStringHash, nsTranslationUsage usage)
{
  if (usage != nsTranslationUsage::Default)
    return {};

  nsStringView sResult = nsTranslatorStorage::Translate(sString, uiStringHash, usage);

  if (!sResult.IsEmpty())
    return sResult;

  nsStringBuilder result;
  nsStringBuilder tmp = sString;
  tmp.Trim(" _-");

  tmp.TrimWordStart("ns");

  nsStringView sComponent = "Component";
  if (tmp.EndsWith(sComponent) && tmp.GetElementCount() > sComponent.GetElementCount())
  {
    tmp.Shrink(0, sComponent.GetElementCount());
  }

  auto IsUpper = [](nsUInt32 c)
  { return c == nsStringUtils::ToUpperChar(c); };
  auto IsNumber = [](nsUInt32 c)
  { return c >= '0' && c <= '9'; };

  nsUInt32 uiPrev = ' ';
  nsUInt32 uiCur = ' ';
  nsUInt32 uiNext = ' ';

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

    if (uiPrev != '[' && uiCur != ']' && IsNumber(uiPrev) != IsNumber(uiCur))
    {
      result.Append(" ");
      result.Append(uiCur);
      continue;
    }

    if (IsNumber(uiPrev) && IsNumber(uiCur))
    {
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
  while (result.ReplaceAll("  ", " ") > 0)
  {
    // remove double whitespaces
  }

  if (GetHighlightUntranslated())
  {
    result.Append(" (@", sString, ")");
  }

  StoreTranslation(result, uiStringHash, usage);

  return nsTranslatorStorage::Translate(sString, uiStringHash, usage);
}
