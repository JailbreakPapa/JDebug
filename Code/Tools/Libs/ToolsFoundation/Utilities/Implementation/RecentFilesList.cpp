#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Utilities/ConversionUtils.h>
#include <ToolsFoundation/Utilities/RecentFilesList.h>

void nsRecentFilesList::Insert(nsStringView sFile, nsInt32 iContainerWindow)
{
  nsStringBuilder sCleanPath = sFile;
  sCleanPath.MakeCleanPath();

  nsString s = sCleanPath;

  for (nsUInt32 i = 0; i < m_Files.GetCount(); i++)
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

void nsRecentFilesList::Save(nsStringView sFile)
{
  nsDeferredFileWriter File;
  File.SetOutput(sFile);

  for (const RecentFile& file : m_Files)
  {
    nsStringBuilder sTemp;
    sTemp.SetFormat("{0}|{1}", file.m_File, file.m_iContainerWindow);
    File.WriteBytes(sTemp.GetData(), sTemp.GetElementCount()).IgnoreResult();
    File.WriteBytes("\n", sizeof(char)).IgnoreResult();
  }

  if (File.Close().Failed())
    nsLog::Error("Unable to open file '{0}' for writing!", sFile);
}

void nsRecentFilesList::Load(nsStringView sFile)
{
  m_Files.Clear();

  nsFileReader File;
  if (File.Open(sFile).Failed())
    return;

  nsStringBuilder sAllLines;
  sAllLines.ReadAll(File);

  nsHybridArray<nsStringView, 16> Lines;
  sAllLines.Split(false, Lines, "\n");

  nsStringBuilder sTemp, sTemp2;

  for (const nsStringView& sv : Lines)
  {
    sTemp = sv;
    nsHybridArray<nsStringView, 2> Parts;
    sTemp.Split(false, Parts, "|");

    if (!nsOSFile::ExistsFile(Parts[0].GetData(sTemp2)))
      continue;

    if (Parts.GetCount() == 1)
    {
      m_Files.PushBack(RecentFile(Parts[0], 0));
    }
    else if (Parts.GetCount() == 2)
    {
      nsStringBuilder sContainer = Parts[1];
      nsInt32 iContainerWindow = 0;
      nsConversionUtils::StringToInt(sContainer, iContainerWindow).IgnoreResult();
      m_Files.PushBack(RecentFile(Parts[0], iContainerWindow));
    }
  }
}
