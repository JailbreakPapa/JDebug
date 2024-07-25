#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QDockWidget>

class nsDocument;

class NS_GUIFOUNDATION_DLL nsQtDocumentPanel : public QDockWidget
{
public:
  Q_OBJECT

public:
  nsQtDocumentPanel(QWidget* pParent, nsDocument* pDocument);
  ~nsQtDocumentPanel();

  // prevents closing of the dockwidget, even with Alt+F4
  virtual void closeEvent(QCloseEvent* e) override;
  virtual bool event(QEvent* pEvent) override;

  static const nsDynamicArray<nsQtDocumentPanel*>& GetAllDocumentPanels() { return s_AllDocumentPanels; }

private:
  nsDocument* m_pDocument = nullptr;

  static nsDynamicArray<nsQtDocumentPanel*> s_AllDocumentPanels;
};
