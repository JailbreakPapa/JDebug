#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_ReflectionWidget.h>
#include <ads/DockWidget.h>

class nsQtReflectionWidget : public ads::CDockWidget, public Ui_ReflectionWidget
{
public:
  Q_OBJECT

public:
  nsQtReflectionWidget(QWidget* pParent = 0);

  static nsQtReflectionWidget* s_pWidget;

private Q_SLOTS:

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();

private:
  struct PropertyData
  {
    nsString m_sType;
    nsString m_sPropertyName;
    nsInt8 m_iCategory;
  };

  struct TypeData
  {
    TypeData() { m_pTreeItem = nullptr; }

    QTreeWidgetItem* m_pTreeItem;

    nsUInt32 m_uiSize;
    nsString m_sParentType;
    nsString m_sPlugin;

    nsHybridArray<PropertyData, 16> m_Properties;
  };

  bool UpdateTree();

  nsMap<nsString, TypeData> m_Types;
};
