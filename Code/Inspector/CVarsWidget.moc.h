#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/Widgets/CVarWidget.moc.h>
#include <Inspector/ui_CVarsWidget.h>
#include <ads/DockWidget.h>

class nsQtCVarsWidget : public ads::CDockWidget, public Ui_CVarsWidget
{
public:
  Q_OBJECT

public:
  nsQtCVarsWidget(QWidget* pParent = 0);

  static nsQtCVarsWidget* s_pWidget;

private Q_SLOTS:
  void BoolChanged(nsStringView sCVar, bool newValue);
  void FloatChanged(nsStringView sCVar, float newValue);
  void IntChanged(nsStringView sCVar, int newValue);
  void StringChanged(nsStringView sCVar, nsStringView sNewValue);

public:
  static void ProcessTelemetry(void* pUnuseed);
  static void ProcessTelemetryConsole(void* pUnuseed);

  void ResetStats();

private:
  // void UpdateCVarsTable(bool bRecreate);


  void SendCVarUpdateToServer(nsStringView sName, const nsCVarWidgetData& cvd);
  void SyncAllCVarsToServer();

  nsMap<nsString, nsCVarWidgetData> m_CVars;
  nsMap<nsString, nsCVarWidgetData> m_CVarsBackup;
};
