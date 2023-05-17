#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Time/Timestamp.h>

wdLogWriter::HTML::~HTML()
{
  EndLog();
}

void wdLogWriter::HTML::BeginLog(wdStringView sFile, wdStringView sAppTitle)
{
  const wdUInt32 uiLogCache = 1024 * 10;

  wdStringBuilder sNewName;
  if (m_File.Open(sFile.GetData(sNewName), uiLogCache, wdFileShareMode::SharedReads) == WD_FAILURE)
  {
    for (wdUInt32 i = 1; i < 32; ++i)
    {
      const wdStringBuilder sName = wdPathUtils::GetFileName(sFile);

      sNewName.Format("{0}_{1}", sName, i);

      wdStringBuilder sPath = sFile;
      sPath.ChangeFileName(sNewName);

      if (m_File.Open(sPath.GetData(), uiLogCache) == WD_SUCCESS)
        break;
    }
  }

  if (!m_File.IsOpen())
  {
    wdLog::Error("Could not open Log-File \"{0}\".", sFile);
    return;
  }

  wdStringBuilder sText;
  sText.Format("<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=utf-8\"><TITLE>Log - {0}</TITLE></HEAD><BODY>", sAppTitle);

  m_File.WriteBytes(sText.GetData(), sizeof(char) * sText.GetElementCount()).IgnoreResult();
}

void wdLogWriter::HTML::EndLog()
{
  if (!m_File.IsOpen())
    return;

  WriteString("", 0);
  WriteString("", 0);
  WriteString(" <<< HTML-Log End >>> ", 0);
  WriteString("", 0);
  WriteString("", 0);

  wdStringBuilder sText;
  sText.Format("</BODY></HTML>");

  m_File.WriteBytes(sText.GetData(), sizeof(char) * sText.GetElementCount()).IgnoreResult();

  m_File.Close();
}

const wdFileWriter& wdLogWriter::HTML::GetOpenedLogFile() const
{
  return m_File;
}

void wdLogWriter::HTML::SetTimestampMode(wdLog::TimestampMode mode)
{
  m_TimestampMode = mode;
}

void wdLogWriter::HTML::LogMessageHandler(const wdLoggingEventData& eventData)
{
  if (!m_File.IsOpen())
    return;

  wdStringBuilder sOriginalText = eventData.m_sText;

  wdStringBuilder sTag = eventData.m_sTag;

  // Cannot write <, > or & to HTML, must be escaped
  sOriginalText.ReplaceAll("&", "&amp;");
  sOriginalText.ReplaceAll("<", "&lt;");
  sOriginalText.ReplaceAll(">", "&gt;");
  sOriginalText.ReplaceAll("\n", "<br>\n");

  sTag.ReplaceAll("&", "&amp;");
  sTag.ReplaceAll("<", "&lt;");
  sTag.ReplaceAll(">", "&gt;");

  wdStringBuilder sTimestamp;
  wdLog::GenerateFormattedTimestamp(m_TimestampMode, sTimestamp);

  bool bFlushWriteCache = false;

  wdStringBuilder sText;

  switch (eventData.m_EventType)
  {
    case wdLogMsgType::Flush:
      bFlushWriteCache = true;
      break;

    case wdLogMsgType::BeginGroup:
      sText.Format("<br><font color=\"#8080FF\"><b> <<< <u>{0}</u> >>> </b> ({1}) </font><br><table width=100%% border=0><tr width=100%%><td "
                   "width=10></td><td width=*>\n",
        sOriginalText, sTag);
      break;

    case wdLogMsgType::EndGroup:
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
      sText.Format("</td></tr></table><font color=\"#8080FF\"><b> <<< {0} ({1} sec)>>> </b></font><br><br>\n", sOriginalText, wdArgF(eventData.m_fSeconds, 4));
#else
      sText.Format("</td></tr></table><font color=\"#8080FF\"><b> <<< {0} ({1})>>> </b></font><br><br>\n", sOriginalText, "timing info not available");
#endif
      break;

    case wdLogMsgType::ErrorMsg:
      bFlushWriteCache = true;
      sText.Format("{0}<font color=\"#FF0000\"><b><u>Error:</u> {1}</b></font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::SeriousWarningMsg:
      bFlushWriteCache = true;
      sText.Format("{0}<font color=\"#FF4000\"><b><u>Seriously:</u> {1}</b></font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::WarningMsg:
      sText.Format("{0}<font color=\"#FF8000\"><u>Warning:</u> {1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::SuccessMsg:
      sText.Format("{0}<font color=\"#009000\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::InfoMsg:
      sText.Format("{0}<font color=\"#000000\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::DevMsg:
      sText.Format("{0}<font color=\"#3030F0\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case wdLogMsgType::DebugMsg:
      sText.Format("{0}<font color=\"#A000FF\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    default:
      sText.Format("{0}<font color=\"#A0A0A0\">{1}</font><br>\n", sTimestamp, sOriginalText);

      wdLog::Warning("Unknown Message Type {1}", eventData.m_EventType);
      break;
  }

  if (!sText.IsEmpty())
  {
    m_File.WriteBytes(sText.GetData(), sizeof(char) * sText.GetElementCount()).IgnoreResult();
  }

  if (bFlushWriteCache)
  {
    m_File.Flush().IgnoreResult();
  }
}

void wdLogWriter::HTML::WriteString(wdStringView sText, wdUInt32 uiColor)
{
  wdStringBuilder sTemp;
  sTemp.Format("<font color=\"#{0}\">{1}</font>", wdArgU(uiColor, 1, false, 16, true), sText);

  m_File.WriteBytes(sTemp.GetData(), sizeof(char) * sTemp.GetElementCount()).IgnoreResult();
}


WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_HTMLWriter);
