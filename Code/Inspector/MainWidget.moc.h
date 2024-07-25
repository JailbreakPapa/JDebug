#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Variant.h>
#include <Inspector/ui_MainWidget.h>
#include <QMainWindow>
#include <ads/DockManager.h>

class QTreeWidgetItem;

class nsQtMainWidget : public ads::CDockWidget, public Ui_MainWidget
{
  Q_OBJECT
public:
  static nsQtMainWidget* s_pWidget;

  nsQtMainWidget(QWidget* pParent = nullptr);
  ~nsQtMainWidget();

  void ResetStats();
  void UpdateStats();
  virtual void closeEvent(QCloseEvent* pEvent) override;

  static void ProcessTelemetry(void* pUnuseed);

public Q_SLOTS:
  void ShowStatIn(bool);

private Q_SLOTS:
  void on_ButtonConnect_clicked();

  void on_TreeStats_itemChanged(QTreeWidgetItem* item, int column);
  void on_TreeStats_customContextMenuRequested(const QPoint& p);

private:
  void SaveFavorites();
  void LoadFavorites();

  QTreeWidgetItem* CreateStat(nsStringView sPath, bool bParent);
  void SetFavorite(const nsString& sStat, bool bFavorite);

  nsUInt32 m_uiMaxStatSamples;
  nsTime m_MaxGlobalTime;

  struct StatSample
  {
    nsTime m_AtGlobalTime;
    double m_Value;
  };

  struct StatData
  {
    nsDeque<StatSample> m_History;

    nsVariant m_Value;
    QTreeWidgetItem* m_pItem;
    QTreeWidgetItem* m_pItemFavorite;

    StatData()
    {
      m_pItem = nullptr;
      m_pItemFavorite = nullptr;
    }
  };

  friend class nsQtStatVisWidget;
  nsMap<nsString, StatData> m_Stats;
  nsSet<nsString> m_Favorites;
};
