#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_GlobalEventsWidget.h>
#include <ads/DockWidget.h>

class nsQtGlobalEventsWidget : public ads::CDockWidget, public Ui_GlobalEventsWidget
{
public:
  Q_OBJECT

public:
  nsQtGlobalEventsWidget(QWidget* pParent = 0);

  static nsQtGlobalEventsWidget* s_pWidget;

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();

private:
  void UpdateTable(bool bRecreate);

  struct GlobalEventsData
  {
    nsInt32 m_iTableRow;
    nsUInt32 m_uiTimesFired;
    nsUInt16 m_uiNumHandlers;
    nsUInt16 m_uiNumHandlersOnce;

    GlobalEventsData()
    {
      m_iTableRow = -1;

      m_uiTimesFired = 0;
      m_uiNumHandlers = 0;
      m_uiNumHandlersOnce = 0;
    }
  };

  nsMap<nsString, GlobalEventsData> m_Events;
};
