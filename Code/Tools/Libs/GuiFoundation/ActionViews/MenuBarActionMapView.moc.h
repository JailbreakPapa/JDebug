/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QMenuBar>
#include <QSharedPointer>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class nsActionMap;
class QAction;
class nsQtProxy;

class NS_GUIFOUNDATION_DLL nsQtMenuBarActionMapView : public QMenuBar
{
  Q_OBJECT
  NS_DISALLOW_COPY_AND_ASSIGN(nsQtMenuBarActionMapView);

public:
  explicit nsQtMenuBarActionMapView(QWidget* pParent);
  ~nsQtMenuBarActionMapView();

  void SetActionContext(const nsActionContext& context);

private:
  void TreeEventHandler(const nsDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const nsDocumentObjectPropertyEvent& e);

  void ClearView();
  void CreateView();

private:
  nsHashTable<nsUuid, QSharedPointer<nsQtProxy>> m_Proxies;

  nsActionContext m_Context;
  nsActionMap* m_pActionMap;
};

