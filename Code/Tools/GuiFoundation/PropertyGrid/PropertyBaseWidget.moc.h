#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyEventHandler.h>
#include <QWidget>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdDocumentObject;
class wdQtTypeWidget;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QMenu;
class wdQtGroupBoxBase;
class wdQtAddSubElementButton;
class wdQtPropertyGridWidget;
class wdQtElementGroupButton;
class QMimeData;
struct wdCommandHistoryEvent;
class wdObjectAccessorBase;

/// \brief Base class for all property widgets
class WD_GUIFOUNDATION_DLL wdQtPropertyWidget : public QWidget
{
  Q_OBJECT;

public:
  explicit wdQtPropertyWidget();
  virtual ~wdQtPropertyWidget();

  void Init(wdQtPropertyGridWidget* pGrid, wdObjectAccessorBase* pObjectAccessor, const wdRTTI* pType, const wdAbstractProperty* pProp);
  const wdAbstractProperty* GetProperty() const { return m_pProp; }

  /// \brief This is called whenever the selection in the editor changes and thus the widget may need to display a different value.
  ///
  /// If the array holds more than one element, the user selected multiple objects. In this case, the code should check whether
  /// the values differ across the selected objects and if so, the widget should display "multiple values".
  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items);
  const wdHybridArray<wdPropertySelection, 8>& GetSelection() const { return m_Items; }

  /// \brief If this returns true (default), a QLabel is created and the text that GetLabel() returns is displayed.
  virtual bool HasLabel() const { return true; }

  /// \brief The return value is used to display a label, if HasLabel() returns true.
  virtual const char* GetLabel(wdStringBuilder& ref_sTmp) const;

  virtual void ExtendContextMenu(QMenu& ref_menu);

  /// \brief Whether the variable that the widget represents is currently set to the default value or has been modified.
  virtual void SetIsDefault(bool bIsDefault) { m_bIsDefault = bIsDefault; }

  /// \brief If the property is of type wdVariant this function returns whether all items have the same type.
  /// If true is returned, out_Type contains the common type. Note that 'invalid' can be a common type.
  bool GetCommonVariantSubType(
    const wdHybridArray<wdPropertySelection, 8>& items, const wdAbstractProperty* pProperty, wdVariantType::Enum& out_type);

  wdVariant GetCommonValue(const wdHybridArray<wdPropertySelection, 8>& items, const wdAbstractProperty* pProperty);
  void PrepareToDie();

public:
  static const wdRTTI* GetCommonBaseType(const wdHybridArray<wdPropertySelection, 8>& items);
  static QColor SetPaletteBackgroundColor(wdColorGammaUB inputColor, QPalette& ref_palette);

public Q_SLOTS:
  void OnCustomContextMenu(const QPoint& pt);

protected:
  void Broadcast(wdPropertyEvent::Type type);
  void PropertyChangedHandler(const wdPropertyEvent& ed);

  virtual void OnInit() = 0;
  bool IsUndead() const { return m_bUndead; }

protected:
  virtual void DoPrepareToDie() = 0;

  virtual bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

  wdQtPropertyGridWidget* m_pGrid = nullptr;
  wdObjectAccessorBase* m_pObjectAccessor = nullptr;
  const wdRTTI* m_pType = nullptr;
  const wdAbstractProperty* m_pProp = nullptr;
  wdHybridArray<wdPropertySelection, 8> m_Items;
  bool m_bIsDefault; ///< Whether the variable that the widget represents is currently set to the default value or has been modified.

private:
  bool m_bUndead; ///< Widget is being destroyed
};


/// \brief Fallback widget for all property types for which no other widget type is registered
class WD_GUIFOUNDATION_DLL wdQtUnsupportedPropertyWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit wdQtUnsupportedPropertyWidget(const char* szMessage = nullptr);

protected:
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override {}

  QHBoxLayout* m_pLayout;
  QLabel* m_pWidget;
  wdString m_sMessage;
};


/// \brief Base class for most 'simple' property type widgets. Implements some of the standard functionality.
class WD_GUIFOUNDATION_DLL wdQtStandardPropertyWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit wdQtStandardPropertyWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;

protected:
  void BroadcastValueChanged(const wdVariant& NewValue);
  virtual void DoPrepareToDie() override {}

  const wdVariant& GetOldValue() const { return m_OldValue; }
  virtual void InternalSetValue(const wdVariant& value) = 0;

protected:
  wdVariant m_OldValue;
};


/// \brief Base class for more 'advanced' property type widgets for Pointer or Class type properties.
/// Implements some of wdQtTypeWidget functionality at property widget level.
class WD_GUIFOUNDATION_DLL wdQtEmbeddedClassPropertyWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit wdQtEmbeddedClassPropertyWidget();
  ~wdQtEmbeddedClassPropertyWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;

protected:
  void SetPropertyValue(const wdAbstractProperty* pProperty, const wdVariant& NewValue);

  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;
  virtual void OnPropertyChanged(const wdString& sProperty) = 0;

private:
  void PropertyEventHandler(const wdDocumentObjectPropertyEvent& e);
  void CommandHistoryEventHandler(const wdCommandHistoryEvent& e);
  void FlushQueuedChanges();

protected:
  bool m_bTemporaryCommand;
  const wdRTTI* m_pResolvedType;
  wdHybridArray<wdPropertySelection, 8> m_ResolvedObjects;

  wdHybridArray<wdString, 1> m_QueuedChanges;
};


/// Used for pointers and embedded classes.
/// Does not inherit from wdQtEmbeddedClassPropertyWidget as it just embeds
/// a wdQtTypeWidget for the property's value which handles everything already.
class WD_GUIFOUNDATION_DLL wdQtPropertyTypeWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit wdQtPropertyTypeWidget(bool bAddCollapsibleGroup = false);
  virtual ~wdQtPropertyTypeWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return false; }
  virtual void SetIsDefault(bool bIsDefault) override;

protected:
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;

protected:
  QHBoxLayout* m_pLayout;
  wdQtGroupBoxBase* m_pGroup;
  QHBoxLayout* m_pGroupLayout;
  wdQtTypeWidget* m_pTypeWidget;
};

/// \brief Used for property types that are pointers.
class WD_GUIFOUNDATION_DLL wdQtPropertyPointerWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit wdQtPropertyPointerWidget();
  virtual ~wdQtPropertyPointerWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return false; }


public Q_SLOTS:
  void OnDeleteButtonClicked();

protected:
  virtual void OnInit() override;
  void StructureEventHandler(const wdDocumentObjectStructureEvent& e);
  virtual void DoPrepareToDie() override;
  void UpdateTitle(const wdRTTI* pType = nullptr);

protected:
  QHBoxLayout* m_pLayout = nullptr;
  wdQtGroupBoxBase* m_pGroup = nullptr;
  wdQtAddSubElementButton* m_pAddButton = nullptr;
  wdQtElementGroupButton* m_pDeleteButton = nullptr;
  QHBoxLayout* m_pGroupLayout = nullptr;
  wdQtTypeWidget* m_pTypeWidget = nullptr;
};


/// \brief Base class for all container properties
class WD_GUIFOUNDATION_DLL wdQtPropertyContainerWidget : public wdQtPropertyWidget
{
  Q_OBJECT;

public:
  wdQtPropertyContainerWidget();
  virtual ~wdQtPropertyContainerWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return false; }
  virtual void SetIsDefault(bool bIsDefault) override;

public Q_SLOTS:
  void OnElementButtonClicked();
  void OnDragStarted(QMimeData& ref_mimeData);
  void OnContainerContextMenu(const QPoint& pt);
  void OnCustomElementContextMenu(const QPoint& pt);

protected:
  struct Element
  {
    Element() = default;

    Element(wdQtGroupBoxBase* pSubGroup, wdQtPropertyWidget* pWidget, wdQtElementGroupButton* pHelpButton)
      : m_pSubGroup(pSubGroup)
      , m_pWidget(pWidget)
      , m_pHelpButton(pHelpButton)
    {
    }

    wdQtGroupBoxBase* m_pSubGroup = nullptr;
    wdQtPropertyWidget* m_pWidget = nullptr;
    wdQtElementGroupButton* m_pHelpButton = nullptr;
  };

  virtual wdQtGroupBoxBase* CreateElement(QWidget* pParent);
  virtual wdQtPropertyWidget* CreateWidget(wdUInt32 index);
  virtual Element& AddElement(wdUInt32 index);
  virtual void RemoveElement(wdUInt32 index);
  virtual void UpdateElement(wdUInt32 index) = 0;
  void UpdateElements();
  virtual wdUInt32 GetRequiredElementCount() const;
  virtual void UpdatePropertyMetaState();

  void Clear();
  virtual void OnInit() override;

  void DeleteItems(wdHybridArray<wdPropertySelection, 8>& items);
  void MoveItems(wdHybridArray<wdPropertySelection, 8>& items, wdInt32 iMove);
  virtual void DoPrepareToDie() override;
  virtual void dragEnterEvent(QDragEnterEvent* event) override;
  virtual void dragMoveEvent(QDragMoveEvent* event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
  virtual void dropEvent(QDropEvent* event) override;
  virtual void paintEvent(QPaintEvent* event) override;
  virtual void showEvent(QShowEvent* event) override;

private:
  bool updateDropIndex(QDropEvent* pEvent);

protected:
  QHBoxLayout* m_pLayout;
  wdQtGroupBoxBase* m_pGroup;
  QVBoxLayout* m_pGroupLayout;
  wdQtAddSubElementButton* m_pAddButton;
  QPalette m_Pal;

  mutable wdHybridArray<wdVariant, 16> m_Keys;
  wdDynamicArray<Element> m_Elements;
  wdInt32 m_iDropSource = -1;
  wdInt32 m_iDropTarget = -1;
};


class WD_GUIFOUNDATION_DLL wdQtPropertyStandardTypeContainerWidget : public wdQtPropertyContainerWidget
{
  Q_OBJECT;

public:
  wdQtPropertyStandardTypeContainerWidget();
  virtual ~wdQtPropertyStandardTypeContainerWidget();

protected:
  virtual wdQtGroupBoxBase* CreateElement(QWidget* pParent) override;
  virtual wdQtPropertyWidget* CreateWidget(wdUInt32 index) override;
  virtual Element& AddElement(wdUInt32 index) override;
  virtual void RemoveElement(wdUInt32 index) override;
  virtual void UpdateElement(wdUInt32 index) override;
};

class WD_GUIFOUNDATION_DLL wdQtPropertyTypeContainerWidget : public wdQtPropertyContainerWidget
{
  Q_OBJECT;

public:
  wdQtPropertyTypeContainerWidget();
  virtual ~wdQtPropertyTypeContainerWidget();

protected:
  virtual void OnInit() override;
  virtual void UpdateElement(wdUInt32 index) override;

  void StructureEventHandler(const wdDocumentObjectStructureEvent& e);
  void CommandHistoryEventHandler(const wdCommandHistoryEvent& e);

private:
  bool m_bNeedsUpdate;
};

class WD_GUIFOUNDATION_DLL wdQtVariantPropertyWidget : public wdQtStandardPropertyWidget
{
  Q_OBJECT;

public:
  wdQtVariantPropertyWidget();
  virtual ~wdQtVariantPropertyWidget();

  virtual void SetSelection(const wdHybridArray<wdPropertySelection, 8>& items) override;
  virtual void ExtendContextMenu(QMenu& ref_menu) override;

protected:
  virtual void OnInit() override{};
  virtual void InternalSetValue(const wdVariant& value) override;
  virtual void DoPrepareToDie() override;
  void ChangeVariantType(wdVariantType::Enum type);

protected:
  QHBoxLayout* m_pLayout = nullptr;
  QWidget* m_pSelectType = nullptr;
  wdQtPropertyWidget* m_pWidget = nullptr;
  const wdRTTI* m_pCurrentSubType = nullptr;
};
