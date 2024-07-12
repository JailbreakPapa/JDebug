#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Time/Timestamp.h>

nsLogWriter::HTML::~HTML()
{
  EndLog();
}

void nsLogWriter::HTML::BeginLog(nsStringView sFile, nsStringView sAppTitle)
{
  const nsUInt32 uiLogCache = 1024 * 10;

  nsStringBuilder sNewName;
  if (m_File.Open(sFile.GetData(sNewName), uiLogCache, nsFileShareMode::SharedReads) == NS_FAILURE)
  {
    for (nsUInt32 i = 1; i < 32; ++i)
    {
      const nsStringBuilder sName = nsPathUtils::GetFileName(sFile);

      sNewName.SetFormat("{0}_{1}", sName, i);

      nsStringBuilder sPath = sFile;
      sPath.ChangeFileName(sNewName);

      if (m_File.Open(sPath.GetData(), uiLogCache) == NS_SUCCESS)
        break;
    }
  }

  if (!m_File.IsOpen())
  {
    nsLog::Error("Could not open Log-File \"{0}\".", sFile);
    return;
  }

  nsStringBuilder sText;
  sText.SetFormat("<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=utf-8\"><TITLE>Log - {0}</TITLE></HEAD><BODY>", sAppTitle);

  m_File.WriteBytes(sText.GetData(), sizeof(char) * sText.GetElementCount()).IgnoreResult();
}

void nsLogWriter::HTML::EndLog()
{
  if (!m_File.IsOpen())
    return;

  WriteString("", 0);
  WriteString("", 0);
  WriteString(" <<< HTML-Log End >>> ", 0);
  WriteString("", 0);
  WriteString("", 0);

  nsStringBuilder sText;
  sText.SetFormat("</BODY></HTML>");

  m_File.WriteBytes(sText.GetData(), sizeof(char) * sText.GetElementCount()).IgnoreResult();

  m_File.Close();
}

const nsFileWriter& nsLogWriter::HTML::GetOpenedLogFile() const
{
  return m_File;
}

void nsLogWriter::HTML::SetTimestampMode(nsLog::TimestampMode mode)
{
  m_TimestampMode = mode;
}

void nsLogWriter::HTML::LogMessageHandler(const nsLoggingEventData& eventData)
{
  if (!m_File.IsOpen())
    return;

  nsStringBuilder sOriginalText = eventData.m_sText;

  nsStringBuilder sTag = eventData.m_sTag;

  // Cannot write <, > or & to HTML, must be escaped
  sOriginalText.ReplaceAll("&", "&amp;");
  sOriginalText.ReplaceAll("<", "&lt;");
  sOriginalText.ReplaceAll(">", "&gt;");
  sOriginalText.ReplaceAll("\n", "<br>\n");

  sTag.ReplaceAll("&", "&amp;");
  sTag.ReplaceAll("<", "&lt;");
  sTag.ReplaceAll(">", "&gt;");

  nsStringBuilder sTimestamp;
  nsLog::GenerateFormattedTimestamp(m_TimestampMode, sTimestamp);

  bool bFlushWriteCache = false;

  nsStringBuilder sText;

  switch (eventData.m_EventType)
  {
    case nsLogMsgType::Flush:
      bFlushWriteCache = true;
      break;

    case nsLogMsgType::BeginGroup:
      sText.SetFormat("<br><font color=\"#8080FF\"><b> <<< <u>{0}</u> >>> </b> ({1}) </font><br><table width=100%% border=0><tr width=100%%><td "
                      "width=10></td><td width=*>\n",
        sOriginalText, sTag);
      break;

    case nsLogMsgType::EndGroup:
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
      sText.SetFormat("</td></tr></table><font color=\"#8080FF\"><b> <<< {0} ({1} sec)>>> </b></font><br><br>\n", sOriginalText, nsArgF(eventData.m_fSeconds, 4));
#else
      sText.SetFormat("</td></tr></table><font color=\"#8080FF\"><b> <<< {0} ({1})>>> </b></font><br><br>\n", sOriginalText, "timing info not available");
#endif
      break;

    case nsLogMsgType::ErrorMsg:
      bFlushWriteCache = true;
      sText.SetFormat("{0}<font color=\"#FF0000\"><b><u>Error:</u> {1}</b></font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::SeriousWarningMsg:
      bFlushWriteCache = true;
      sText.SetFormat("{0}<font color=\"#FF4000\"><b><u>Seriously:</u> {1}</b></font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::WarningMsg:
      sText.SetFormat("{0}<font color=\"#FF8000\"><u>Warning:</u> {1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::SuccessMsg:
      sText.SetFormat("{0}<font color=\"#009000\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::InfoMsg:
      sText.SetFormat("{0}<font color=\"#000000\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::DevMsg:
      sText.SetFormat("{0}<font color=\"#3030F0\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    case nsLogMsgType::DebugMsg:
      sText.SetFormat("{0}<font color=\"#A000FF\">{1}</font><br>\n", sTimestamp, sOriginalText);
      break;

    default:
      sText.SetFormat("{0}<font color=\"#A0A0A0\">{1}</font><br>\n", sTimestamp, sOriginalText);

      nsLog::Warning("Unknown Message Type {1}", eventData.m_EventType);
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

void nsLogWriter::HTML::WriteString(nsStringView sText, nsUInt32 uiColor)
{
  nsStringBuilder sTemp;
  sTemp.SetFormat("<font color=\"#{0}\">{1}</font>", nsArgU(uiColor, 1, false, 16, true), sText);

  m_File.WriteBytes(sTemp.GetData(), sizeof(char) * sTemp.GetElementCount()).IgnoreResult();
}
