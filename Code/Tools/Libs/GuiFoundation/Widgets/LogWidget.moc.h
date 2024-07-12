/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_LogWidget.h>
#include <QWidget>

class nsQtLogModel;
class nsQtSearchWidget;

/// \brief The application wide panel that shows the engine log output and the editor log output
class NS_GUIFOUNDATION_DLL nsQtLogWidget : public QWidget, public Ui_LogWidget
{
  Q_OBJECT

public:
  nsQtLogWidget(QWidget* pParent);
  ~nsQtLogWidget();

  void ShowControls(bool bShow);

  nsQtLogModel* GetLog();
  nsQtSearchWidget* GetSearchWidget();
  void SetLogLevel(nsLogMsgType::Enum logLevel);
  nsLogMsgType::Enum GetLogLevel() const;

  virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private Q_SLOTS:
  void on_ButtonClearLog_clicked();
  void on_Search_textChanged(const QString& text);
  void on_ComboFilter_currentIndexChanged(int index);

private:
  nsQtLogModel* m_pLog;
  void ScrollToBottomIfAtEnd(int iNumElements);
};

