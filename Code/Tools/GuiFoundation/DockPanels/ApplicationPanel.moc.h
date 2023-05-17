#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Project/ToolsProject.h>
#include <ads/DockWidget.h>

class wdQtContainerWindow;

/// \brief Base class for all panels that are supposed to be application wide (not tied to some document).
class WD_GUIFOUNDATION_DLL wdQtApplicationPanel : public ads::CDockWidget
{
public:
  Q_OBJECT

public:
  wdQtApplicationPanel(const char* szPanelName);
  ~wdQtApplicationPanel();

  void EnsureVisible();

  static const wdDynamicArray<wdQtApplicationPanel*>& GetAllApplicationPanels() { return s_AllApplicationPanels; }

protected:
  virtual void ToolsProjectEventHandler(const wdToolsProjectEvent& e);
  virtual bool event(QEvent* event) override;

private:
  friend class wdQtContainerWindow;

  static wdDynamicArray<wdQtApplicationPanel*> s_AllApplicationPanels;

  wdQtContainerWindow* m_pContainerWindow;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_GUIFOUNDATION_DLL, wdQtApplicationPanel);

