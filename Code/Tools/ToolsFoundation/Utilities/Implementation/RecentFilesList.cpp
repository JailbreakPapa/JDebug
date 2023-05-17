#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/ConversionUtils.h>
#include <ToolsFoundation/Utilities/RecentFilesList.h>

void wdRecentFilesList::Insert(const char* szFile, wdInt32 iContainerWindow)
{
  wdStringBuilder sCleanPath = szFile;
  sCleanPath.MakeCleanPath();

  wdString s = sCleanPath;

  for (wdUInt32 i = 0; i < m_Files.GetCount(); i++)
  {
    if (m_Files[i].m_File == s)
    {
      m_Files.RemoveAtAndCopy(i);
      break;
    }
  }
  m_Files.PushFront(RecentFile(s, iContainerWindow));

  if (m_Files.GetCount() > m_uiMaxElements)
    m_Files.SetCount(m_uiMaxElements);
}

void wdRecentFilesList::Save(const char* szFile)
{
  if (m_Files.IsEmpty())
    return;

  wdDeferredFileWriter File;
  File.SetOutput(szFile);

  for (const RecentFile& file : m_Files)
  {
    wdStringBuilder sTemp;
    sTemp.Format("{0}|{1}", file.m_File, file.m_iContainerWindow);
    File.WriteBytes(sTemp.GetData(), sTemp.GetElementCount()).IgnoreResult();
    File.WriteBytes("\n", sizeof(char)).IgnoreResult();
  }

  if (File.Close().Failed())
    wdLog::Error("Unable to open file '{0}' for writing!", szFile);
}

void wdRecentFilesList::Load(const char* szFile)
{
  m_Files.Clear();

  wdFileReader File;
  if (File.Open(szFile).Failed())
    return;

  wdStringBuilder sAllLines;
  sAllLines.ReadAll(File);

  wdHybridArray<wdStringView, 16> Lines;
  sAllLines.Split(false, Lines, "\n");

  wdStringBuilder sTemp, sTemp2;

  for (const wdStringView& sv : Lines)
  {
    sTemp = sv;
    wdHybridArray<wdStringView, 2> Parts;
    sTemp.Split(false, Parts, "|");

    if (!wdOSFile::ExistsFile(Parts[0].GetData(sTemp2)))
      continue;

    if (Parts.GetCount() == 1)
    {
      m_Files.PushBack(RecentFile(Parts[0], 0));
    }
    else if (Parts.GetCount() == 2)
    {
      wdStringBuilder sContainer = Parts[1];
      wdInt32 iContainerWindow = 0;
      wdConversionUtils::StringToInt(sContainer, iContainerWindow).IgnoreResult();
      m_Files.PushBack(RecentFile(Parts[0], iContainerWindow));
    }
  }
}
