#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Time/Stopwatch.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QDesktopServices>
#include <QDir>
#include <QIcon>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <QUrl>

NS_IMPLEMENT_SINGLETON(nsQtUiServices);

nsEvent<const nsQtUiServices::Event&> nsQtUiServices::s_Events;
nsEvent<const nsQtUiServices::TickEvent&> nsQtUiServices::s_TickEvent;

nsMap<nsString, QIcon> nsQtUiServices::s_IconsCache;
nsMap<nsString, QImage> nsQtUiServices::s_ImagesCache;
nsMap<nsString, QPixmap> nsQtUiServices::s_PixmapsCache;
bool nsQtUiServices::s_bHeadless;
nsQtUiServices::TickEvent nsQtUiServices::s_LastTickEvent;

static nsQtUiServices* g_pInstance = nullptr;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, QtUiServices)

  ON_CORESYSTEMS_STARTUP
  {
    g_pInstance = NS_DEFAULT_NEW(nsQtUiServices);
    nsQtUiServices::GetSingleton()->Init();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    NS_DEFAULT_DELETE(g_pInstance);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsQtUiServices::nsQtUiServices()
  : m_SingletonRegistrar(this)
{
  qRegisterMetaType<nsUuid>();
  m_pColorDlg = nullptr;
}


bool nsQtUiServices::IsHeadless()
{
  return s_bHeadless;
}


void nsQtUiServices::SetHeadless(bool bHeadless)
{
  s_bHeadless = true;
}

void nsQtUiServices::SaveState()
{
  QSettings Settings;
  Settings.beginGroup("EditorGUI");
  {
    Settings.setValue("ColorDlgGeom", m_ColorDlgGeometry);
  }
  Settings.endGroup();
}

nsTime g_Total = nsTime::MakeZero();

const QIcon& nsQtUiServices::GetCachedIconResource(nsStringView sIdentifier, nsColor svgTintColor)
{
  nsStringBuilder sFullIdentifier = sIdentifier;
  auto& map = s_IconsCache;

  const bool bNeedsColoring = svgTintColor != nsColor::MakeZero() && sIdentifier.EndsWith_NoCase(".svg");

  if (bNeedsColoring)
  {
    sFullIdentifier.AppendFormat("-{}", nsColorGammaUB(svgTintColor));
  }

  auto it = map.Find(sFullIdentifier);

  if (it.IsValid())
    return it.Value();

  if (bNeedsColoring)
  {
    nsStopwatch sw;

    // read the icon from the Qt virtual file system (QResource)
    QFile file(nsString(sIdentifier).GetData());
    if (!file.open(QIODeviceBase::OpenModeFlag::ReadOnly))
    {
      // if it doesn't exist, return an empty QIcon

      map[sFullIdentifier] = QIcon();
      return map[sFullIdentifier];
    }

    // get the entire SVG file content
    nsStringBuilder sContent = QString(file.readAll()).toUtf8().data();

    // replace the occurrence of the color white ("#FFFFFF") with the desired target color
    {
      const nsColorGammaUB color8 = svgTintColor;

      nsStringBuilder rep;
      rep.SetFormat("#{}{}{}", nsArgI((int)color8.r, 2, true, 16), nsArgI((int)color8.g, 2, true, 16), nsArgI((int)color8.b, 2, true, 16));

      sContent.ReplaceAll_NoCase("#ffffff", rep);

      rep.Append(";");
      sContent.ReplaceAll_NoCase("#fff;", rep);
      rep.Shrink(0, 1);

      rep.Prepend("\"");
      rep.Append("\"");
      sContent.ReplaceAll_NoCase("\"#fff\"", rep);
    }

    // hash the content AFTER the color replacement, so it includes the custom color change
    const nsUInt32 uiSrcHash = nsHashingUtils::xxHash32String(sContent);

    // file the path to the temp file, including the source hash
    const nsStringBuilder sTempFolder = nsOSFile::GetTempDataFolder("nsEditor/QIcons");
    nsStringBuilder sTempIconFile(sTempFolder, "/", sIdentifier.GetFileName());
    sTempIconFile.AppendFormat("-{}.svg", uiSrcHash);

    // only write to the file system, if the target file doesn't exist yet, this saves more than half the time
    if (!nsOSFile::ExistsFile(sTempIconFile))
    {
      // now write the new SVG file back to a dummy file
      // yes, this is as stupid as it sounds, we really write the file BACK TO THE FILESYSTEM, rather than doing this stuff in-memory
      // that's because I wasn't able to figure out whether we can somehow read a QIcon from a string rather than from file
      // it doesn't appear to be easy at least, since we can only give it a path, not a memory stream or anything like that
      {
        // necessary for Qt to be able to write to the folder
        nsOSFile::CreateDirectoryStructure(sTempFolder).AssertSuccess();

        QFile fileOut(sTempIconFile.GetData());
        fileOut.open(QIODeviceBase::OpenModeFlag::WriteOnly);
        fileOut.write(sContent.GetData(), sContent.GetElementCount());
        fileOut.flush();
        fileOut.close();
      }
    }

    QIcon icon(sTempIconFile.GetData());

    if (!icon.pixmap(QSize(16, 16)).isNull())
      map[sFullIdentifier] = icon;
    else
      map[sFullIdentifier] = QIcon();

    nsTime local = sw.GetRunningTotal();
    g_Total += local;

    // kept here for debug purposes, but don't waste time on logging
    // nsLog::Info("Icon load time: {}, total = {}", local, g_Total);
  }
  else
  {
    const QString sFile = nsString(sIdentifier).GetData();

    if (QFile::exists(sFile)) // prevent Qt from spamming warnings about non-existing files by checking this manually
    {
      QIcon icon(sFile);

      // Workaround for QIcon being stupid and treating failed to load icons as not-null.
      if (!icon.pixmap(QSize(16, 16)).isNull())
        map[sFullIdentifier] = icon;
      else
        map[sFullIdentifier] = QIcon();
    }
    else
      map[sFullIdentifier] = QIcon();
  }

  return map[sFullIdentifier];
}


const QImage& nsQtUiServices::GetCachedImageResource(const char* szIdentifier)
{
  const nsString sIdentifier = szIdentifier;
  auto& map = s_ImagesCache;

  auto it = map.Find(sIdentifier);

  if (it.IsValid())
    return it.Value();

  map[sIdentifier] = QImage(QString::fromUtf8(szIdentifier));

  return map[sIdentifier];
}

const QPixmap& nsQtUiServices::GetCachedPixmapResource(const char* szIdentifier)
{
  const nsString sIdentifier = szIdentifier;
  auto& map = s_PixmapsCache;

  auto it = map.Find(sIdentifier);

  if (it.IsValid())
    return it.Value();

  map[sIdentifier] = QPixmap(QString::fromUtf8(szIdentifier));

  return map[sIdentifier];
}

nsResult nsQtUiServices::AddToGitIgnore(const char* szGitIgnoreFile, const char* szPattern)
{
  nsStringBuilder ignoreFile;

  {
    nsFileReader file;
    if (file.Open(szGitIgnoreFile).Succeeded())
    {
      ignoreFile.ReadAll(file);
    }
  }

  ignoreFile.Trim("\n\r");

  const nsUInt32 len = nsStringUtils::GetStringElementCount(szPattern);

  // pattern already present ?
  if (const char* szFound = ignoreFile.FindSubString(szPattern))
  {
    if (szFound == ignoreFile.GetData() || // right at the start
        *(szFound - 1) == '\n')            // after a new line
    {
      const char end = *(szFound + len);

      if (end == '\0' || end == '\r' || end == '\n') // line does not continue with an extended pattern
      {
        return NS_SUCCESS;
      }
    }
  }

  ignoreFile.AppendWithSeparator("\n", szPattern);
  ignoreFile.Append("\n\n");

  {
    nsFileWriter file;
    NS_SUCCEED_OR_RETURN(file.Open(szGitIgnoreFile));

    NS_SUCCEED_OR_RETURN(file.WriteBytes(ignoreFile.GetData(), ignoreFile.GetElementCount()));
  }

  return NS_SUCCESS;
}

void nsQtUiServices::CheckForUpdates()
{
  Event e;
  e.m_Type = Event::Type::CheckForUpdates;
  s_Events.Broadcast(e);
}

void nsQtUiServices::Init()
{
  s_LastTickEvent.m_fRefreshRate = 60.0;
  if (QScreen* pScreen = QApplication::primaryScreen())
  {
    s_LastTickEvent.m_fRefreshRate = pScreen->refreshRate();
  }

  QTimer::singleShot((nsInt32)nsMath::Floor(1000.0 / s_LastTickEvent.m_fRefreshRate), this, SLOT(TickEventHandler()));
}

void nsQtUiServices::TickEventHandler()
{
  NS_PROFILE_SCOPE("TickEvent");

  NS_ASSERT_DEV(!m_bIsDrawingATM, "Implementation error");
  nsTime startTime = nsTime::Now();

  m_bIsDrawingATM = true;
  s_LastTickEvent.m_uiFrame++;
  s_LastTickEvent.m_Time = startTime;
  s_LastTickEvent.m_Type = TickEvent::Type::StartFrame;
  s_TickEvent.Broadcast(s_LastTickEvent);

  s_LastTickEvent.m_Type = TickEvent::Type::EndFrame;
  s_TickEvent.Broadcast(s_LastTickEvent);
  m_bIsDrawingATM = false;

  const nsTime endTime = nsTime::Now();
  nsTime lastFrameTime = endTime - startTime;

  nsTime delay = nsTime::MakeFromMilliseconds(1000.0 / s_LastTickEvent.m_fRefreshRate);
  delay -= lastFrameTime;
  delay = nsMath::Max(delay, nsTime::MakeZero());

  QTimer::singleShot((nsInt32)nsMath::Floor(delay.GetMilliseconds()), this, SLOT(TickEventHandler()));
}

void nsQtUiServices::LoadState()
{
  QSettings Settings;
  Settings.beginGroup("EditorGUI");
  {
    m_ColorDlgGeometry = Settings.value("ColorDlgGeom").toByteArray();
  }
  Settings.endGroup();
}

void nsQtUiServices::ShowAllDocumentsTemporaryStatusBarMessage(const nsFormatString& msg, nsTime timeOut)
{
  nsStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowDocumentTemporaryStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_Time = timeOut;

  s_Events.Broadcast(e, 1);
}

void nsQtUiServices::ShowAllDocumentsPermanentStatusBarMessage(const nsFormatString& msg, Event::TextType type)
{
  nsStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowDocumentPermanentStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_TextType = type;

  s_Events.Broadcast(e, 1);
}

void nsQtUiServices::ShowGlobalStatusBarMessage(const nsFormatString& msg)
{
  nsStringBuilder tmp;

  Event e;
  e.m_Type = Event::ShowGlobalStatusBarText;
  e.m_sText = msg.GetText(tmp);
  e.m_Time = nsTime::MakeFromSeconds(0);

  s_Events.Broadcast(e);
}


bool nsQtUiServices::OpenFileInDefaultProgram(const char* szPath)
{
  return QDesktopServices::openUrl(QUrl::fromLocalFile(szPath));
}

void nsQtUiServices::OpenInExplorer(const char* szPath, bool bIsFile)
{
  QStringList args;

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  if (bIsFile)
    args << "/select,";

  args << QDir::toNativeSeparators(szPath);

  QProcess::startDetached("explorer", args);
#elif NS_ENABLED(NS_PLATFORM_LINUX)
  nsStringBuilder parentDir;

  if (bIsFile)
  {
    parentDir = szPath;
    parentDir = parentDir.GetFileDirectory();
    szPath = parentDir.GetData();
  }
  args << QDir::toNativeSeparators(szPath);

  QProcess::startDetached("xdg-open", args);
#else
  NS_ASSERT_NOT_IMPLEMENTED
#endif
}

nsStatus nsQtUiServices::OpenInVsCode(const QStringList& arguments)
{
  QString sVsCodeExe;
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  sVsCodeExe =
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
#endif

  if (sVsCodeExe.isEmpty() || !QFile().exists(sVsCodeExe))
  {
    // Try code executable in PATH
    if (QProcess::execute("code", {"--version"}) == 0)
    {
      sVsCodeExe = "code";
    }
    else
    {
      return nsStatus("Installation of Visual Studio Code could not be located.\n"
                      "Please visit 'https://code.visualstudio.com/download' to download the 'User Installer' of Visual Studio Code.");
    }
  }

  QProcess proc;
  if (proc.startDetached(sVsCodeExe, arguments) == false)
  {
    return nsStatus("Failed to launch Visual Studio Code.");
  }

  return nsStatus(NS_SUCCESS);
}
