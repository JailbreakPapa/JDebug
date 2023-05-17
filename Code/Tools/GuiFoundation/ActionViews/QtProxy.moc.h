#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Variant.h>
#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Factory/RttiMappedObjectFactory.h>

#include <QEvent>
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QWidget>
#include <QWidgetAction>

class QAction;
class QMenu;
class QLabel;
class QSlider;
class wdAction;

/// \brief Glue class that maps wdActions to QActions. QActions are only created if the wdAction is actually mapped somewhere. Document and Global actions are manually executed and don't solely rely on Qt's ShortcutContext setting to prevent ambiguous action shortcuts.
class WD_GUIFOUNDATION_DLL wdQtProxy : public QObject
{
  Q_OBJECT

public:
  wdQtProxy();
  virtual ~wdQtProxy();

  virtual void Update() = 0;

  virtual void SetAction(wdAction* pAction);
  wdAction* GetAction() { return m_pAction; }

  /// \brief Converts the QKeyEvent into a shortcut and tries to find a matching action in the document and global action list.
  ///
  /// Document actions are not mapped as ShortcutContext::WindowShortcut because docking allows for multiple documents to be mapped into the same window. Instead, ShortcutContext::WidgetWithChildrenShortcut is used to prevent ambiguous action shortcuts and the actions are executed manually via filtering QEvent::ShortcutOverride at the dock widget level.
  ///
  /// \param pDocument The document for which matching actions should be searched for. If null, only global actions are searched.
  /// \param event The key event that should be converted into a shortcut.
  /// \return Whether the key event was consumed and an action executed.
  static bool TriggerDocumentAction(wdDocument* pDocument, QKeyEvent* pEvent);

  static wdRttiMappedObjectFactory<wdQtProxy>& GetFactory();
  static QSharedPointer<wdQtProxy> GetProxy(wdActionContext& ref_context, wdActionDescriptorHandle hAction);

protected:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, QtProxies);
  static wdRttiMappedObjectFactory<wdQtProxy> s_Factory;
  static wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>> s_GlobalActions;
  static wdMap<const wdDocument*, wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>> s_DocumentActions;
  static wdMap<QWidget*, wdMap<wdActionDescriptorHandle, QWeakPointer<wdQtProxy>>> s_WindowActions;
  static QObject* s_pSignalProxy;

protected:
  wdAction* m_pAction;
};

class WD_GUIFOUNDATION_DLL wdQtActionProxy : public wdQtProxy
{
  Q_OBJECT

public:
  virtual QAction* GetQAction() = 0;
};

class WD_GUIFOUNDATION_DLL wdQtCategoryProxy : public wdQtProxy
{
  Q_OBJECT
public:
  virtual void Update() override {}
};

class WD_GUIFOUNDATION_DLL wdQtMenuProxy : public wdQtProxy
{
  Q_OBJECT

public:
  wdQtMenuProxy();
  ~wdQtMenuProxy();

  virtual void Update() override;
  virtual void SetAction(wdAction* pAction) override;

  virtual QMenu* GetQMenu();

protected:
  QMenu* m_pMenu;
};

class WD_GUIFOUNDATION_DLL wdQtButtonProxy : public wdQtActionProxy
{
  Q_OBJECT

public:
  wdQtButtonProxy();
  ~wdQtButtonProxy();

  virtual void Update() override;
  virtual void SetAction(wdAction* pAction) override;

  virtual QAction* GetQAction() override;

private Q_SLOTS:
  void OnTriggered();

private:
  void StatusUpdateEventHandler(wdAction* pAction);

private:
  QPointer<QAction> m_pQtAction;
};


class WD_GUIFOUNDATION_DLL wdQtDynamicMenuProxy : public wdQtMenuProxy
{
  Q_OBJECT

public:
  virtual void SetAction(wdAction* pAction) override;

private Q_SLOTS:
  void SlotMenuAboutToShow();
  void SlotMenuEntryTriggered();

private:
  wdHybridArray<wdDynamicMenuAction::Item, 16> m_Entries;
};

class WD_GUIFOUNDATION_DLL wdQtDynamicActionAndMenuProxy : public wdQtDynamicMenuProxy
{
  Q_OBJECT

public:
  wdQtDynamicActionAndMenuProxy();
  ~wdQtDynamicActionAndMenuProxy();

  virtual void Update() override;
  virtual void SetAction(wdAction* pAction) override;
  virtual QAction* GetQAction();

private Q_SLOTS:
  void OnTriggered();

private:
  void StatusUpdateEventHandler(wdAction* pAction);

private:
  QPointer<QAction> m_pQtAction;
};


class WD_GUIFOUNDATION_DLL wdQtLabeledSlider : public QWidget
{
  Q_OBJECT

public:
  wdQtLabeledSlider(QWidget* pParent);

  QLabel* m_pLabel;
  QSlider* m_pSlider;
};


class WD_GUIFOUNDATION_DLL wdQtSliderWidgetAction : public QWidgetAction
{
  Q_OBJECT

public:
  wdQtSliderWidgetAction(QWidget* pParent);
  void setMinimum(int value);
  void setMaximum(int value);
  void setValue(int value);

Q_SIGNALS:
  void valueChanged(int value);

private Q_SLOTS:
  void OnValueChanged(int value);

protected:
  virtual QWidget* createWidget(QWidget* parent) override;
  virtual bool eventFilter(QObject* obj, QEvent* e) override;

  wdInt32 m_iMinimum;
  wdInt32 m_iMaximum;
  wdInt32 m_iValue;
};

class WD_GUIFOUNDATION_DLL wdQtSliderProxy : public wdQtActionProxy
{
  Q_OBJECT

public:
  wdQtSliderProxy();
  ~wdQtSliderProxy();

  virtual void Update() override;
  virtual void SetAction(wdAction* pAction) override;

  virtual QAction* GetQAction() override;

private Q_SLOTS:
  void OnValueChanged(int value);

private:
  void StatusUpdateEventHandler(wdAction* pAction);

private:
  QPointer<wdQtSliderWidgetAction> m_pQtAction;
};

