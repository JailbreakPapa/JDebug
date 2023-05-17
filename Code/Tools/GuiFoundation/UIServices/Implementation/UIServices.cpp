#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/Profiling.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <QUrl>

WD_IMPLEMENT_SINGLETON(wdQtUiServices);

wdEvent<const wdQtUiServices::Event&> wdQtUiServices::s_Events;
wdEvent<const wdQtUiServices::TickEvent&> wdQtUiServices::s_TickEvent;

wdMap<wdString, QIcon> wdQtUiServices::s_IconsCache;
wdMap<wdString, QImage> wdQtUiServices::s_ImagesCache;
wdMap<wdString, QPixmap> wdQtUiServices::s_PixmapsCache;
bool wdQtUiServices::s_bHeadless;
wdQtUiServices::TickEvent wdQtUiServices::s_LastTickEvent;

static wdQtUiServices* g_pInstance = nullptr;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, QtUiServices)

  ON_CORESYSTEMS_STARTUP
  {
    g_pInstance = WD_DEFAULT_NEW(wdQtUiServices);
    wdQtUiServices::GetSingleton()->Init();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    WD_DEFAULT_DELETE(g_pInstance);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdQtUiServices::wdQtUiServices()
  : m_SingletonRegistrar(this)
{
  int id = qRegisterMetaType<wdUuid>();
  m_pColorDlg = nullptr;
}


bool wdQtUiServices::IsHeadless()
{
  return s_bHeadless;
}


void wdQtUiServices::SetHeadless(bool bHeadless)
{
  s_bHeadless = true;
}

void wdQtUiServices::SaveState()
{
  QSettings Settings;
  Settings.beginGroup("EditorGUI");
  {
    Settings.setValue("ColorDlgGeom", m_ColorDlgGeometry);
  }
  Settings.endGroup();
}


const QIcon& wdQtUiServices::GetCachedIconResource(const char* szIdentifier)
{
  const wdString sIdentifier = szIdentifier;
  auto& map = s_IconsCache;

  auto it = map.Find(sIdentifier);

  if (it.IsValid())
    return it.Value();

  QIcon icon(QString::fromUtf8(szIdentifier));

  // Workaround for QIcon being stupid and treating failed to load icons as not-null.
  if (!icon.pixmap(QSize(16, 16)).isNull())
    map[sIdentifier] = icon;
  else
    map[sIdentifier] = QIcon();

  return map[sIdentifier];
}


const QImage& wdQtUiServices::GetCachedImageResource(const char* szIdentifier)
{
  const wdString sIdentifier = szIdentifier;
  auto& map = s_ImagesCache;

  auto it = map.Find(sIdentifier);

  if (it.IsValid())
    return it.Value();

  map[sIdentifier] = QImage(QString::fromUtf8(szIdentifier));

  return map[sIdentifier];
}

const QPixmap& wdQtUiServices::GetCachedPixmapResource(const char* szIdentifier)
{
  const wdString sIdentifier = szIdentifier;
  auto& map = s_PixmapsCache;

  auto it = map.Find(sIdentifier);

  if (it.IsValid())
    return it.Value();

  map[sIdentifier] = QPixmap(QString::fromUtf8(szIdentifier));

  return map[sIdentifier];
}

wdResult wdQtUiServices::AddToGitIgnore(const char* szGitIgnoreFile, const char* szPattern)
{
  wdStringBuilder ignoreFile;

  {
    wdFileReader file;
    if (file.Open(szGitIgnoreFile).Succeeded())
    {
      ignoreFile.ReadAll(file);
    }
  }

  ignoreFile.Trim("\n\r");

  const wdUInt32 len = wdStringUtils::GetStringElementCount(szPattern);

  // pattern already present ?
  if (const char* szFound = ignoreFile.FindSubString(szPattern))
  {
    if (szFound == ignoreFile.GetData() || // right at the start
        *(szFound - 1) == '\n')            // after a new line
    {
      const char end = *(szFound + len);

      if (end == '\0' || end == '\r' || end == '\n') // line does not continue with an extended pattern
      {
        return WD_SUCCESS;
      }
    }
  }

  ignoreFile.AppendWithSeparator("\n", szPattern);
  ignoreFile.Append("\n\n");

  {
    wdFileWriter file;
    WD_SUCCEED_OR_RETURN(file.Open(szGitIgnoreFile));

    WD_SUCCEED_OR_RETURN(file.WriteBytes(ignoreFile.GetData(), ignoreFile.GetElementCount()));
  }

  return WD_SUCCESS;
}

void wdQtUiServices::CheckForUpdates()
{
  Event e;
  e.m_Type = Event::Type::CheckForUpdates;
  s_Events.Broadcast(e);
}

void wdQtUiServices::Init()
{
  s_LastTickEvent.m_fRefreshRate = 60.0;
  if (QScreen* pScreen = QApplication::primaryScreen())
  {
    s_LastTickEvent.m_fRefreshRate = pScreen->refreshRate();
  }

  QTimer::singleShot((wdInt32)wdMath::Floor(1000.0 / s_LastTickEvent.m_fRefreshRate), this, SLOT(TickEventHandler()));
}

void wdQtUiServices::TickEventHandler()
{
  WD_PROFILE_SCOPE("TickEvent");

  WD_ASSERT_DEV(!m_bIsDrawingATM, "Implementation error");
  wdTime startTime = wdTime::Now();

  m_bIsDrawingATM = true;
  s_LastTickEvent.m_uiFrame++;
  s_LastTickEvent.m_Time = startTime;
  s_LastTickEvent.m_Type = TickEvent::Type::StartFrame;
  s_TickEvent.Broadcast(s_LastTickEvent);

  s_LastTickEvent.m_Type = TickEvent::Type::EndFrame;
  s_TickEvent.Broadcast(s_LastTickEvent);
  m_bIsDrawingATM = false;

  const wdTime endTime = wdTime::Now();
  wdTime lastFrameTime = endTime - startTime;

  wdTime delay = wdTime::Milliseconds(1000.0 / s_LastTickEvent.m_fRefreshRate);
  delay -= lastFrameTime;
  delay = wdMath::Max(delay, wdTime::Zero());

  QTimer::singleShot((wdInt32)wdMath::Floor(delay.GetMilliseconds()), this, SLOT(TickEventHandler()));
}

void wdQtUiServices::LoadState()
{
  QSettings Settings;
  Settings.beginGroup("EditorGUI");
  {
    m_ColorDlgGeometry = Settings.value("ColorDlgGeom").toByteArray();
  }
  Settings.endGroup();
}

void wdQtUiServices::ShowAllDocumentsTemporaryStatusBarMessage(const wdFormatString& msg, wdTime timeOut)
{
  wdStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowDocumentTemporaryStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_Time = timeOut;

  s_Events.Broadcast(e, 1);
}

void wdQtUiServices::ShowAllDocumentsPermanentStatusBarMessage(const wdFormatString& msg, Event::TextType type)
{
  wdStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowDocumentPermanentStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_TextType = type;

  s_Events.Broadcast(e, 1);
}

void wdQtUiServices::ShowGlobalStatusBarMessage(const wdFormatString& msg)
{
  wdStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowGlobalStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_Time = wdTime::Seconds(0);

  s_Events.Broadcast(e);
}


bool wdQtUiServices::OpenFileInDefaultProgram(const char* szPath)
{
  return QDesktopServices::openUrl(QUrl::fromLocalFile(szPath));
}

void wdQtUiServices::OpenInExplorer(const char* szPath, bool bIsFile)
{
  QStringList args;

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  if (bIsFile)
    args << "/select,";

  args << QDir::toNativeSeparators(szPath);

  QProcess::startDetached("explorer", args);
#elif WD_ENABLED(WD_PLATFORM_LINUX)
  wdStringBuilder parentDir;

  if (bIsFile)
  {
    parentDir = szPath;
    parentDir = parentDir.GetFileDirectory();
    szPath = parentDir.GetData();
  }
  args << QDir::toNativeSeparators(szPath);

  QProcess::startDetached("xdg-open", args);
#else
  WD_ASSERT_NOT_IMPLEMENTED
#endif
}

wdStatus wdQtUiServices::OpenInVsCode(const QStringList& arguments)
{
  QString sVsCodeExe =
    QStandardPaths::locate(QStandardPaths::GenericDataLocation, "Programs/Microsoft VS Code/Code.exe", QStandardPaths::LocateOption::LocateFile);

  if (!QFile().exists(sVsCodeExe))
  {
    QSettings settings("\\HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\Applications\\Code.exe\\shell\\open\\command", QSettings::NativeFormat);
    QString sVsCodeExeKey = settings.value(".", "").value<QString>();

    if (sVsCodeExeKey.length() > 5)
    {
      // Remove shell parameter and normalize QT Compatible path, QFile expects the file separator to be '/' regardless of operating system
      sVsCodeExe = sVsCodeExeKey.left(sVsCodeExeKey.length() - 5).replace("\\", "/").replace("\"", "");
    }
  }

  if (!QFile().exists(sVsCodeExe))
  {
    return wdStatus("Installation of Visual Studio Code could not be located.\n"
                    "Please visit 'https://code.visualstudio.com/download' to download the 'User Installer' of Visual Studio Code.");
  }

  QProcess proc;
  if (proc.startDetached(sVsCodeExe, arguments) == false)
  {
    return wdStatus("Failed to launch Visual Studio Code.");
  }

  return wdStatus(WD_SUCCESS);
}
