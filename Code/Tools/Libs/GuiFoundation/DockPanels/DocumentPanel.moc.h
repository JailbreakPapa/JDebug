/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
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

