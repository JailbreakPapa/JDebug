#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/Implementation/PropertyEventHandler.h>
#include <QWidget>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsDocumentObject;
class nsQtTypeWidget;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QMenu;
class QComboBox;
class nsQtGroupBoxBase;
class nsQtAddSubElementButton;
class nsQtPropertyGridWidget;
class nsQtElementGroupButton;
class QMimeData;
struct nsCommandHistoryEvent;
class nsObjectAccessorBase;

/// \brief Base class for all property widgets
class NS_GUIFOUNDATION_DLL nsQtPropertyWidget : public QWidget
{
  Q_OBJECT;

public:
  explicit nsQtPropertyWidget();
  virtual ~nsQtPropertyWidget();

  void Init(nsQtPropertyGridWidget* pGrid, nsObjectAccessorBase* pObjectAccessor, const nsRTTI* pType, const nsAbstractProperty* pProp);
  const nsAbstractProperty* GetProperty() const { return m_pProp; }

  /// \brief This is called whenever the selection in the editor changes and thus the widget may need to display a different value.
  ///
  /// If the array holds more than one element, the user selected multiple objects. In this case, the code should check whether
  /// the values differ across the selected objects and if so, the widget should display "multiple values".
  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items);
  const nsHybridArray<nsPropertySelection, 8>& GetSelection() const { return m_Items; }

  /// \brief If this returns true (default), a QLabel is created and the text that GetLabel() returns is displayed.
  virtual bool HasLabel() const { return true; }

  /// \brief The return value is used to display a label, if HasLabel() returns true.
  virtual const char* GetLabel(nsStringBuilder& ref_sTmp) const;

  virtual void ExtendContextMenu(QMenu& ref_menu);

  /// \brief Whether the variable that the widget represents is currently set to the default value or has been modified.
  virtual void SetIsDefault(bool bIsDefault) { m_bIsDefault = bIsDefault; }

  /// \brief If the property is of type nsVariant this function returns whether all items have the same type.
  /// If true is returned, out_Type contains the common type. Note that 'invalid' can be a common type.
  bool GetCommonVariantSubType(
    const nsHybridArray<nsPropertySelection, 8>& items, const nsAbstractProperty* pProperty, nsVariantType::Enum& out_type);

  nsVariant GetCommonValue(const nsHybridArray<nsPropertySelection, 8>& items, const nsAbstractProperty* pProperty);
  void PrepareToDie();

  /// \brief By default disables the widget, but can be overridden to make a widget more interactable (for example to be able to copy text from it).
  virtual void SetReadOnly(bool bReadOnly = true);

public:
  static const nsRTTI* GetCommonBaseType(const nsHybridArray<nsPropertySelection, 8>& items);
  static QColor SetPaletteBackgroundColor(nsColorGammaUB inputColor, QPalette& ref_palette);

public Q_SLOTS:
  void OnCustomContextMenu(const QPoint& pt);

protected:
  void Broadcast(nsPropertyEvent::Type type);
  void PropertyChangedHandler(const nsPropertyEvent& ed);

  virtual void OnInit() = 0;
  bool IsUndead() const { return m_bUndead; }

protected:
  virtual void DoPrepareToDie() = 0;

  virtual bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

  nsQtPropertyGridWidget* m_pGrid = nullptr;
  nsObjectAccessorBase* m_pObjectAccessor = nullptr;
  const nsRTTI* m_pType = nullptr;
  const nsAbstractProperty* m_pProp = nullptr;
  nsHybridArray<nsPropertySelection, 8> m_Items;
  bool m_bIsDefault; ///< Whether the variable that the widget represents is currently set to the default value or has been modified.

private:
  bool m_bUndead;    ///< Widget is being destroyed
};


/// \brief Fallback widget for all property types for which no other widget type is registered
class NS_GUIFOUNDATION_DLL nsQtUnsupportedPropertyWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit nsQtUnsupportedPropertyWidget(const char* szMessage = nullptr);

protected:
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override {}

  QHBoxLayout* m_pLayout;
  QLabel* m_pWidget;
  nsString m_sMessage;
};


/// \brief Base class for most 'simple' property type widgets. Implements some of the standard functionality.
class NS_GUIFOUNDATION_DLL nsQtStandardPropertyWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit nsQtStandardPropertyWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;

protected:
  void BroadcastValueChanged(const nsVariant& NewValue);
  virtual void DoPrepareToDie() override {}

  const nsVariant& GetOldValue() const { return m_OldValue; }
  virtual void InternalSetValue(const nsVariant& value) = 0;

protected:
  nsVariant m_OldValue;
};


/// \brief Base class for more 'advanced' property type widgets for Pointer or Class type properties.
/// Implements some of nsQtTypeWidget functionality at property widget level.
class NS_GUIFOUNDATION_DLL nsQtEmbeddedClassPropertyWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit nsQtEmbeddedClassPropertyWidget();
  ~nsQtEmbeddedClassPropertyWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;

protected:
  void SetPropertyValue(const nsAbstractProperty* pProperty, const nsVariant& NewValue);

  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;
  virtual void OnPropertyChanged(const nsString& sProperty) = 0;

private:
  void PropertyEventHandler(const nsDocumentObjectPropertyEvent& e);
  void CommandHistoryEventHandler(const nsCommandHistoryEvent& e);
  void FlushQueuedChanges();

protected:
  bool m_bTemporaryCommand = false;
  const nsRTTI* m_pResolvedType = nullptr;
  nsHybridArray<nsPropertySelection, 8> m_ResolvedObjects;

  nsHybridArray<nsString, 1> m_QueuedChanges;
};


/// Used for pointers and embedded classes.
/// Does not inherit from nsQtEmbeddedClassPropertyWidget as it just embeds
/// a nsQtTypeWidget for the property's value which handles everything already.
class NS_GUIFOUNDATION_DLL nsQtPropertyTypeWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit nsQtPropertyTypeWidget(bool bAddCollapsibleGroup = false);
  virtual ~nsQtPropertyTypeWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return false; }
  virtual void SetIsDefault(bool bIsDefault) override;

protected:
  virtual void OnInit() override;
  virtual void DoPrepareToDie() override;

protected:
  QVBoxLayout* m_pLayout;
  nsQtGroupBoxBase* m_pGroup;
  QVBoxLayout* m_pGroupLayout;
  nsQtTypeWidget* m_pTypeWidget;
};

/// \brief Used for property types that are pointers.
class NS_GUIFOUNDATION_DLL nsQtPropertyPointerWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  explicit nsQtPropertyPointerWidget();
  virtual ~nsQtPropertyPointerWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return false; }


public Q_SLOTS:
  void OnDeleteButtonClicked();

protected:
  virtual void OnInit() override;
  void StructureEventHandler(const nsDocumentObjectStructureEvent& e);
  virtual void DoPrepareToDie() override;
  void UpdateTitle(const nsRTTI* pType = nullptr);

protected:
  QHBoxLayout* m_pLayout = nullptr;
  nsQtGroupBoxBase* m_pGroup = nullptr;
  nsQtAddSubElementButton* m_pAddButton = nullptr;
  nsQtElementGroupButton* m_pDeleteButton = nullptr;
  QHBoxLayout* m_pGroupLayout = nullptr;
  nsQtTypeWidget* m_pTypeWidget = nullptr;
};


/// \brief Base class for all container properties
class NS_GUIFOUNDATION_DLL nsQtPropertyContainerWidget : public nsQtPropertyWidget
{
  Q_OBJECT;

public:
  nsQtPropertyContainerWidget();
  virtual ~nsQtPropertyContainerWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;
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

    Element(nsQtGroupBoxBase* pSubGroup, nsQtPropertyWidget* pWidget, nsQtElementGroupButton* pHelpButton)
      : m_pSubGroup(pSubGroup)
      , m_pWidget(pWidget)
      , m_pHelpButton(pHelpButton)
    {
    }

    nsQtGroupBoxBase* m_pSubGroup = nullptr;
    nsQtPropertyWidget* m_pWidget = nullptr;
    nsQtElementGroupButton* m_pHelpButton = nullptr;
  };

  virtual nsQtGroupBoxBase* CreateElement(QWidget* pParent);
  virtual nsQtPropertyWidget* CreateWidget(nsUInt32 index);
  virtual Element& AddElement(nsUInt32 index);
  virtual void RemoveElement(nsUInt32 index);
  virtual void UpdateElement(nsUInt32 index) = 0;
  void UpdateElements();
  virtual nsUInt32 GetRequiredElementCount() const;
  virtual void UpdatePropertyMetaState();

  void Clear();
  virtual void OnInit() override;

  void DeleteItems(nsHybridArray<nsPropertySelection, 8>& items);
  void MoveItems(nsHybridArray<nsPropertySelection, 8>& items, nsInt32 iMove);
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
  nsQtGroupBoxBase* m_pGroup;
  QVBoxLayout* m_pGroupLayout;
  nsQtAddSubElementButton* m_pAddButton = nullptr;
  QPalette m_Pal;

  mutable nsHybridArray<nsVariant, 16> m_Keys;
  nsDynamicArray<Element> m_Elements;
  nsInt32 m_iDropSource = -1;
  nsInt32 m_iDropTarget = -1;
};


class NS_GUIFOUNDATION_DLL nsQtPropertyStandardTypeContainerWidget : public nsQtPropertyContainerWidget
{
  Q_OBJECT;

public:
  nsQtPropertyStandardTypeContainerWidget();
  virtual ~nsQtPropertyStandardTypeContainerWidget();

protected:
  virtual nsQtGroupBoxBase* CreateElement(QWidget* pParent) override;
  virtual nsQtPropertyWidget* CreateWidget(nsUInt32 index) override;
  virtual Element& AddElement(nsUInt32 index) override;
  virtual void RemoveElement(nsUInt32 index) override;
  virtual void UpdateElement(nsUInt32 index) override;
};

class NS_GUIFOUNDATION_DLL nsQtPropertyTypeContainerWidget : public nsQtPropertyContainerWidget
{
  Q_OBJECT;

public:
  nsQtPropertyTypeContainerWidget();
  virtual ~nsQtPropertyTypeContainerWidget();

protected:
  virtual void OnInit() override;
  virtual void UpdateElement(nsUInt32 index) override;

  void StructureEventHandler(const nsDocumentObjectStructureEvent& e);
  void CommandHistoryEventHandler(const nsCommandHistoryEvent& e);

private:
  bool m_bNeedsUpdate = false;
};

class NS_GUIFOUNDATION_DLL nsQtVariantPropertyWidget : public nsQtStandardPropertyWidget
{
  Q_OBJECT;

public:
  nsQtVariantPropertyWidget();
  virtual ~nsQtVariantPropertyWidget();

protected:
  virtual void OnInit() override;
  virtual void InternalSetValue(const nsVariant& value) override;
  virtual void DoPrepareToDie() override;
  void UpdateTypeListSelection(nsVariantType::Enum type);
  void ChangeVariantType(nsVariantType::Enum type);

  virtual nsResult GetVariantTypeDisplayName(nsVariantType::Enum type, nsStringBuilder& out_sName) const;

protected:
  QVBoxLayout* m_pLayout = nullptr;
  QComboBox* m_pTypeList = nullptr;
  nsQtPropertyWidget* m_pWidget = nullptr;
  const nsRTTI* m_pCurrentSubType = nullptr;
};
