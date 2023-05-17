#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QDockWidget>

class wdDocument;

class WD_GUIFOUNDATION_DLL wdQtDocumentPanel : public QDockWidget
{
public:
  Q_OBJECT

public:
  wdQtDocumentPanel(QWidget* pParent, wdDocument* pDocument);
  ~wdQtDocumentPanel();

  // prevents closing of the dockwidget, even with Alt+F4
  virtual void closeEvent(QCloseEvent* e) override;
  virtual bool event(QEvent* pEvent) override;

  static const wdDynamicArray<wdQtDocumentPanel*>& GetAllDocumentPanels() { return s_AllDocumentPanels; }

private:
  wdDocument* m_pDocument = nullptr;

  static wdDynamicArray<wdQtDocumentPanel*> s_AllDocumentPanels;
};

