#include <QApplication>
#include <QSettings>
#include <qstylefactory.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Configuration/Startup.h>
#include <Proto/MainWindow.moc.h>

class nsProtoApp : public nsApplication
{
public:
  using SUPER = nsApplication;

  nsProtoApp()
    : nsApplication("Proto")
  {
  }

  void SetStyleSheet()
  {
    QApplication::setStyle(QStyleFactory::create("fusion"));
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(221, 221, 221));
    palette.setColor(QPalette::Button, QColor(60, 60, 60));
    palette.setColor(QPalette::Light, QColor(62, 62, 62));
    palette.setColor(QPalette::Midlight, QColor(58, 58, 58));
    palette.setColor(QPalette::Dark, QColor(40, 40, 40));
    palette.setColor(QPalette::Mid, QColor(54, 54, 54));
    palette.setColor(QPalette::Text, QColor(221, 221, 221));
    palette.setColor(QPalette::BrightText, QColor(221, 221, 221));
    palette.setColor(QPalette::ButtonText, QColor(221, 221, 221));
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::Window, QColor(40, 40, 40));
    palette.setColor(QPalette::Shadow, QColor(0, 0, 0));
    palette.setColor(QPalette::Highlight, QColor(86, 117, 148, 255));
    palette.setColor(QPalette::HighlightedText, QColor(52, 52, 52));
    palette.setColor(QPalette::Link, QColor(52, 52, 52));
    palette.setColor(QPalette::LinkVisited, QColor(86, 117, 148, 255));
    palette.setColor(QPalette::AlternateBase, QColor(37, 37, 40));
    QBrush NoRoleBrush(QColor(0, 0, 0), Qt::NoBrush);
    palette.setBrush(QPalette::NoRole, NoRoleBrush);
    palette.setColor(QPalette::ToolTipBase, QColor(52, 52, 52));
    palette.setColor(QPalette::ToolTipText, QColor(221, 221, 221));
    palette.setColor(QPalette::PlaceholderText, QColor(200, 200, 200).darker());

    palette.setColor(QPalette::Disabled, QPalette::Window, QColor(25, 25, 25));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, QPalette::Button, QColor(35, 35, 35));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(105, 105, 105));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(86, 117, 148, 255));

    QApplication::setPalette(palette);
  }

  virtual nsResult BeforeCoreSystemsStartup() override
  {
    nsStartup::AddApplicationTag("tool");
    nsStartup::AddApplicationTag("inspector");

    return nsApplication::BeforeCoreSystemsStartup();
  }

  virtual Execution Run() override
  {
    int iArgs = GetArgumentCount();
    char** cArgs = (char**)GetArgumentsArray();

    QApplication app(iArgs, cArgs);
    QCoreApplication::setOrganizationDomain("www.wdstudios.tech");
    QCoreApplication::setOrganizationName("WDStudios");
    QCoreApplication::setApplicationName("Proto");
    QCoreApplication::setApplicationVersion("1.0.0");

    SetStyleSheet();

    nsQtMainWindow MainWindow;

    QSettings Settings;
    const QString sServer = Settings.value("LastConnection", QLatin1String("localhost:1040")).toString();

    nsTelemetry::ConnectToServer(sServer.toUtf8().data()).IgnoreResult();

    MainWindow.show();
    SetReturnCode(app.exec());

    nsTelemetry::CloseConnection();

    return nsApplication::Execution::Quit;
  }
};

NS_APPLICATION_ENTRY_POINT(nsProtoApp);