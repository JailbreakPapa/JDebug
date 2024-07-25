#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Inspector/ui_MemoryWidget.h>
#include <QAction>
#include <QGraphicsView>
#include <QPointer>
#include <ads/DockWidget.h>

class QTreeWidgetItem;

class nsQtMemoryWidget : public ads::CDockWidget, public Ui_MemoryWidget
{
public:
  Q_OBJECT

public:
  static const nsUInt8 s_uiMaxColors = 9;

  nsQtMemoryWidget(QWidget* pParent = 0);

  static nsQtMemoryWidget* s_pWidget;

private Q_SLOTS:

  void on_ListAllocators_itemChanged(QTreeWidgetItem* item);
  void on_ComboTimeframe_currentIndexChanged(int index);
  void on_actionEnableOnlyThis_triggered(bool);
  void on_actionEnableAll_triggered(bool);
  void on_actionDisableAll_triggered(bool);

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();
  void UpdateStats();

private:
  void CustomContextMenuRequested(const QPoint& pos);

  QGraphicsPathItem* m_pPath[s_uiMaxColors];
  QGraphicsPathItem* m_pPathMax;
  QGraphicsScene m_Scene;

  nsTime m_LastUsedMemoryStored;
  nsTime m_LastUpdatedAllocatorList;

  nsUInt32 m_uiMaxSamples;
  nsUInt32 m_uiDisplaySamples;

  nsUInt8 m_uiColorsUsed;
  bool m_bAllocatorsChanged;

  struct AllocatorData
  {
    nsDeque<nsUInt64> m_UsedMemory;
    nsString m_sName;

    bool m_bStillInUse = true;
    bool m_bReceivedData = false;
    bool m_bDisplay = true;
    nsUInt8 m_uiColor = 0xFF;
    nsUInt32 m_uiParentId = nsInvalidIndex;
    nsUInt64 m_uiAllocs = 0;
    nsUInt64 m_uiDeallocs = 0;
    nsUInt64 m_uiLiveAllocs = 0;
    nsUInt64 m_uiMaxUsedMemoryRecently = 0;
    nsUInt64 m_uiMaxUsedMemory = 0;
    QTreeWidgetItem* m_pTreeItem = nullptr;
  };

  AllocatorData m_Accu;

  nsMap<nsUInt32, AllocatorData> m_AllocatorData;
};
