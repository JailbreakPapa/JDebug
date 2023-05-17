#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_LogWidget.h>
#include <QWidget>

class wdQtLogModel;
class wdQtSearchWidget;

/// \brief The application wide panel that shows the engine log output and the editor log output
class WD_GUIFOUNDATION_DLL wdQtLogWidget : public QWidget, public Ui_LogWidget
{
  Q_OBJECT

public:
  wdQtLogWidget(QWidget* pParent);
  ~wdQtLogWidget();

  void ShowControls(bool bShow);

  wdQtLogModel* GetLog();
  wdQtSearchWidget* GetSearchWidget();
  void SetLogLevel(wdLogMsgType::Enum logLevel);
  wdLogMsgType::Enum GetLogLevel() const;

  virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private Q_SLOTS:
  void on_ButtonClearLog_clicked();
  void on_Search_textChanged(const QString& text);
  void on_ComboFilter_currentIndexChanged(int index);

private:
  wdQtLogModel* m_pLog;
  void ScrollToBottomIfAtEnd(int iNumElements);
};

