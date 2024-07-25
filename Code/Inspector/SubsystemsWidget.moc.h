#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_SubsystemsWidget.h>
#include <ads/DockWidget.h>

class nsQtSubsystemsWidget : public ads::CDockWidget, public Ui_SubsystemsWidget
{
public:
  Q_OBJECT

public:
  nsQtSubsystemsWidget(QWidget* pParent = 0);

  static nsQtSubsystemsWidget* s_pWidget;

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();
  void UpdateStats();

private:
  void UpdateSubSystems();

  struct SubsystemData
  {
    nsString m_sPlugin;
    bool m_bStartupDone[nsStartupStage::ENUM_COUNT];
    nsString m_sDependencies;
  };

  bool m_bUpdateSubsystems;
  nsMap<nsString, SubsystemData> m_Subsystems;
};
