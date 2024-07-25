#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <QWidget>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>

class QGridLayout;
class nsDocument;
class nsQtManipulatorLabel;
struct nsManipulatorManagerEvent;
class nsObjectAccessorBase;

class NS_GUIFOUNDATION_DLL nsQtTypeWidget : public QWidget
{
  Q_OBJECT
public:
  nsQtTypeWidget(QWidget* pParent, nsQtPropertyGridWidget* pGrid, nsObjectAccessorBase* pObjectAccessor, const nsRTTI* pType,
    const char* szIncludeProperties, const char* szExcludeProperties);
  ~nsQtTypeWidget();
  void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items);
  const nsHybridArray<nsPropertySelection, 8>& GetSelection() const { return m_Items; }
  const nsRTTI* GetType() const { return m_pType; }
  void PrepareToDie();

private:
  struct PropertyGroup
  {
    PropertyGroup(const nsGroupAttribute* pAttr, float& ref_fOrder)
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

    void MergeGroup(const nsGroupAttribute* pAttr)
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

    nsString m_sGroup;
    nsString m_sIconName;
    float m_fOrder = -1.0f;
    nsHybridArray<const nsAbstractProperty*, 8> m_Properties;
  };

  void BuildUI(const nsRTTI* pType, const char* szIncludeProperties, const char* szExcludeProperties);
  void BuildUI(const nsRTTI* pType, const nsMap<nsString, const nsManipulatorAttribute*>& manipulatorMap,
    const nsDynamicArray<nsUniquePtr<PropertyGroup>>& groups, const char* szIncludeProperties, const char* szExcludeProperties);

  void PropertyEventHandler(const nsDocumentObjectPropertyEvent& e);
  void CommandHistoryEventHandler(const nsCommandHistoryEvent& e);
  void ManipulatorManagerEventHandler(const nsManipulatorManagerEvent& e);

  void UpdateProperty(const nsDocumentObject* pObject, const nsString& sProperty);
  void FlushQueuedChanges();
  void UpdatePropertyMetaState();

protected:
  virtual void showEvent(QShowEvent* event) override;

private:
  bool m_bUndead = false;
  nsQtPropertyGridWidget* m_pGrid = nullptr;
  nsObjectAccessorBase* m_pObjectAccessor = nullptr;
  const nsRTTI* m_pType = nullptr;
  nsHybridArray<nsPropertySelection, 8> m_Items;

  struct PropertyWidgetData
  {
    nsQtPropertyWidget* m_pWidget;
    nsQtManipulatorLabel* m_pLabel;
    nsString m_sOriginalLabelText;
  };

  QGridLayout* m_pLayout;
  nsMap<nsString, PropertyWidgetData> m_PropertyWidgets;
  nsHybridArray<nsString, 1> m_QueuedChanges;
  QPalette m_Pal;
};
