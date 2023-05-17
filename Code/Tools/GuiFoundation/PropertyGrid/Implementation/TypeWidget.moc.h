#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <QWidget>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>

class QGridLayout;
class wdDocument;
class wdQtManipulatorLabel;
struct wdManipulatorManagerEvent;
class wdObjectAccessorBase;

class WD_GUIFOUNDATION_DLL wdQtTypeWidget : public QWidget
{
  Q_OBJECT
public:
  wdQtTypeWidget(QWidget* pParent, wdQtPropertyGridWidget* pGrid, wdObjectAccessorBase* pObjectAccessor, const wdRTTI* pType,
    const char* szIncludeProperties, const char* szExcludeProperties);
  ~wdQtTypeWidget();
  void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items);
  const wdHybridArray<wdPropertySelection, 8>& GetSelection() const { return m_Items; }
  const wdRTTI* GetType() const { return m_pType; }
  void PrepareToDie();

private:
  struct PropertyGroup
  {
    PropertyGroup(const wdGroupAttribute* pAttr, float& ref_fOrder)
    {
      if (pAttr)
      {
        m_sGroup = pAttr->GetGroup();
        m_sIconName = pAttr->GetIconName();
        m_fOrder = pAttr->GetOrder();
        if (m_fOrder == -1.0f)
        {
          ref_fOrder += 1.0f;
          m_fOrder = ref_fOrder;
        }
      }
      else
      {
        ref_fOrder += 1.0f;
        m_fOrder = ref_fOrder;
      }
    }

    void MergeGroup(const wdGroupAttribute* pAttr)
    {
      if (pAttr)
      {
        m_sGroup = pAttr->GetGroup();
        m_sIconName = pAttr->GetIconName();
        if (pAttr->GetOrder() != -1.0f)
        {
          m_fOrder = pAttr->GetOrder();
        }
      }
    }

    bool operator==(const PropertyGroup& rhs) { return m_sGroup == rhs.m_sGroup; }
    bool operator<(const PropertyGroup& rhs) { return m_fOrder < rhs.m_fOrder; }

    wdString m_sGroup;
    wdString m_sIconName;
    float m_fOrder = -1.0f;
    wdHybridArray<const wdAbstractProperty*, 8> m_Properties;
  };

  void BuildUI(const wdRTTI* pType, const char* szIncludeProperties, const char* szExcludeProperties);
  void BuildUI(const wdRTTI* pType, const wdMap<wdString, const wdManipulatorAttribute*>& manipulatorMap,
    const wdDynamicArray<wdUniquePtr<PropertyGroup>>& groups, const char* szIncludeProperties, const char* szExcludeProperties);

  void PropertyEventHandler(const wdDocumentObjectPropertyEvent& e);
  void CommandHistoryEventHandler(const wdCommandHistoryEvent& e);
  void ManipulatorManagerEventHandler(const wdManipulatorManagerEvent& e);

  void UpdateProperty(const wdDocumentObject* pObject, const wdString& sProperty);
  void FlushQueuedChanges();
  void UpdatePropertyMetaState();

protected:
  virtual void showEvent(QShowEvent* event) override;

private:
  bool m_bUndead = false;
  wdQtPropertyGridWidget* m_pGrid = nullptr;
  wdObjectAccessorBase* m_pObjectAccessor = nullptr;
  const wdRTTI* m_pType = nullptr;
  wdHybridArray<wdPropertySelection, 8> m_Items;

  struct PropertyWidgetData
  {
    wdQtPropertyWidget* m_pWidget;
    wdQtManipulatorLabel* m_pLabel;
    wdString m_sOriginalLabelText;
  };

  QGridLayout* m_pLayout;
  wdMap<wdString, PropertyWidgetData> m_PropertyWidgets;
  wdHybridArray<wdString, 1> m_QueuedChanges;
  QPalette m_Pal;
};
