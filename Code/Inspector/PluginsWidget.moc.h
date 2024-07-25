#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_PluginsWidget.h>
#include <ads/DockWidget.h>

class nsQtPluginsWidget : public ads::CDockWidget, public Ui_PluginsWidget
{
public:
  Q_OBJECT

public:
  nsQtPluginsWidget(QWidget* pParent = 0);

  static nsQtPluginsWidget* s_pWidget;

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();
  void UpdateStats();

private:
  void UpdatePlugins();

  struct PluginsData
  {
    bool m_bReloadable;
    nsString m_sDependencies;
  };

  bool m_bUpdatePlugins;
  nsMap<nsString, PluginsData> m_Plugins;
};
