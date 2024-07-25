#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Basics.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <Inspector/ui_ResourceWidget.h>
#include <ads/DockWidget.h>

class nsQtResourceWidget : public ads::CDockWidget, public Ui_ResourceWidget
{
public:
  Q_OBJECT

public:
  nsQtResourceWidget(QWidget* pParent = 0);

  static nsQtResourceWidget* s_pWidget;

private Q_SLOTS:

  void on_LineFilterByName_textChanged();
  void on_ComboResourceTypes_currentIndexChanged(int state);
  void on_CheckShowDeleted_toggled(bool checked);
  void on_ButtonSave_clicked();

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();
  void UpdateStats();

  void UpdateTable();

private:
  void UpdateAll();

  struct ResourceData
  {
    ResourceData()
    {
      m_pMainItem = nullptr;
      m_bUpdate = true;
    }

    bool m_bUpdate;
    QTableWidgetItem* m_pMainItem;
    nsString m_sResourceID;
    nsString m_sResourceType;
    nsResourcePriority m_Priority;
    nsBitflags<nsResourceFlags> m_Flags;
    nsResourceLoadDesc m_LoadingState;
    nsResource::MemoryUsage m_Memory;
    nsString m_sResourceDescription;
  };

  bool m_bShowDeleted;
  nsString m_sTypeFilter;
  nsString m_sNameFilter;
  nsTime m_LastTableUpdate;
  bool m_bUpdateTable;

  bool m_bUpdateTypeBox;
  nsSet<nsString> m_ResourceTypes;
  nsHashTable<nsUInt64, ResourceData> m_Resources;
};
