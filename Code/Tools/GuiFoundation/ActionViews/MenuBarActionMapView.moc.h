#pragma once

#include <GuiFoundation/Action/ActionMap.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QMenuBar>
#include <QSharedPointer>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

class QWidget;
class wdActionMap;
class QAction;
class wdQtProxy;

class WD_GUIFOUNDATION_DLL wdQtMenuBarActionMapView : public QMenuBar
{
  Q_OBJECT
  WD_DISALLOW_COPY_AND_ASSIGN(wdQtMenuBarActionMapView);

public:
  explicit wdQtMenuBarActionMapView(QWidget* pParent);
  ~wdQtMenuBarActionMapView();

  void SetActionContext(const wdActionContext& context);

private:
  void TreeEventHandler(const wdDocumentObjectStructureEvent& e);
  void TreePropertyEventHandler(const wdDocumentObjectPropertyEvent& e);

  void ClearView();
  void CreateView();

private:
  wdHashTable<wdUuid, QSharedPointer<wdQtProxy>> m_Proxies;

  wdActionContext m_Context;
  wdActionMap* m_pActionMap;
};

