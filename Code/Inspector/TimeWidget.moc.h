#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Inspector/ui_TimeWidget.h>
#include <QGraphicsView>
#include <QListWidgetItem>
#include <ads/DockWidget.h>

class nsQtTimeWidget : public ads::CDockWidget, public Ui_TimeWidget
{
public:
  Q_OBJECT

public:
  static const nsUInt8 s_uiMaxColors = 9;

  nsQtTimeWidget(QWidget* pParent = 0);

  static nsQtTimeWidget* s_pWidget;

private Q_SLOTS:

  void on_ListClocks_itemChanged(QListWidgetItem* item);
  void on_ComboTimeframe_currentIndexChanged(int index);

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();
  void UpdateStats();

private:
  QGraphicsPathItem* m_pPath[s_uiMaxColors];
  QGraphicsPathItem* m_pPathMax;
  QGraphicsScene m_Scene;

  nsUInt32 m_uiMaxSamples;

  nsUInt8 m_uiColorsUsed;
  bool m_bClocksChanged;

  nsTime m_MaxGlobalTime;
  nsTime m_DisplayInterval;
  nsTime m_LastUpdatedClockList;

  struct TimeSample
  {
    nsTime m_AtGlobalTime;
    nsTime m_Timestep;
  };

  struct ClockData
  {
    nsDeque<TimeSample> m_TimeSamples;

    bool m_bDisplay = true;
    nsUInt8 m_uiColor = 0xFF;
    nsTime m_MinTimestep = nsTime::MakeFromSeconds(60.0);
    nsTime m_MaxTimestep;
    QListWidgetItem* m_pListItem = nullptr;
  };

  nsMap<nsString, ClockData> m_ClockData;
};
