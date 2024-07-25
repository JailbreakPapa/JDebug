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
class nsAction;

/// \brief Glue class that maps nsActions to QActions. QActions are only created if the nsAction is actually mapped somewhere. Document and Global actions are manually executed and don't solely rely on Qt's ShortcutContext setting to prevent ambiguous action shortcuts.
class NS_GUIFOUNDATION_DLL nsQtProxy : public QObject
{
  Q_OBJECT

public:
  nsQtProxy();
  virtual ~nsQtProxy();

  virtual void Update() = 0;

  virtual void SetAction(nsAction* pAction);
  nsAction* GetAction() { return m_pAction; }

  /// \brief Converts the QKeyEvent into a shortcut and tries to find a matching action in the document and global action list.
  ///
  /// Document actions are not mapped as ShortcutContext::WindowShortcut because docking allows for multiple documents to be mapped into the same window. Instead, ShortcutContext::WidgetWithChildrenShortcut is used to prevent ambiguous action shortcuts and the actions are executed manually via filtering QEvent::ShortcutOverride at the dock widget level.
  /// The function always has to be called two times:
  /// A: QEvent::ShortcutOverride: Only check with bTestOnly = true that we want to override the shortcut. This will instruct Qt to send the event as a regular key press event to the widget that accepted the override.
  /// B: QEvent::keyPressEvent: Execute the actual action with bTestOnly = false;
  ///
  /// \param pDocument The document for which matching actions should be searched for. If null, only global actions are searched.
  /// \param pEvent The key event that should be converted into a shortcut.
  /// \param bTestOnly Accept the event and return true but don't execute the action. Use this inside QEvent::ShortcutOverride.
  /// \return Whether the key event was consumed and an action executed.
  static bool TriggerDocumentAction(nsDocument* pDocument, QKeyEvent* pEvent, bool bTestOnly);

  static nsRttiMappedObjectFactory<nsQtProxy>& GetFactory();
  static QSharedPointer<nsQtProxy> GetProxy(nsActionContext& ref_context, nsActionDescriptorHandle hAction);

protected:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, QtProxies);
  static nsRttiMappedObjectFactory<nsQtProxy> s_Factory;
  static nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>> s_GlobalActions;
  static nsMap<const nsDocument*, nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>> s_DocumentActions;
  static nsMap<QWidget*, nsMap<nsActionDescriptorHandle, QWeakPointer<nsQtProxy>>> s_WindowActions;
  static QObject* s_pSignalProxy;

protected:
  nsAction* m_pAction;
};

class NS_GUIFOUNDATION_DLL nsQtActionProxy : public nsQtProxy
{
  Q_OBJECT

public:
  virtual QAction* GetQAction() = 0;
};

class NS_GUIFOUNDATION_DLL nsQtCategoryProxy : public nsQtProxy
{
  Q_OBJECT
public:
  virtual void Update() override {}
};

class NS_GUIFOUNDATION_DLL nsQtMenuProxy : public nsQtProxy
{
  Q_OBJECT

public:
  nsQtMenuProxy();
  ~nsQtMenuProxy();

  virtual void Update() override;
  virtual void SetAction(nsAction* pAction) override;

  virtual QMenu* GetQMenu();

protected:
  QMenu* m_pMenu;
};

class NS_GUIFOUNDATION_DLL nsQtButtonProxy : public nsQtActionProxy
{
  Q_OBJECT

public:
  nsQtButtonProxy();
  ~nsQtButtonProxy();

  virtual void Update() override;
  virtual void SetAction(nsAction* pAction) override;

  virtual QAction* GetQAction() override;

private Q_SLOTS:
  void OnTriggered();

private:
  void StatusUpdateEventHandler(nsAction* pAction);

private:
  QPointer<QAction> m_pQtAction;
};


class NS_GUIFOUNDATION_DLL nsQtDynamicMenuProxy : public nsQtMenuProxy
{
  Q_OBJECT

public:
  virtual void SetAction(nsAction* pAction) override;

private Q_SLOTS:
  void SlotMenuAboutToShow();
  void SlotMenuEntryTriggered();

private:
  nsHybridArray<nsDynamicMenuAction::Item, 16> m_Entries;
};

class NS_GUIFOUNDATION_DLL nsQtDynamicActionAndMenuProxy : public nsQtDynamicMenuProxy
{
  Q_OBJECT

public:
  nsQtDynamicActionAndMenuProxy();
  ~nsQtDynamicActionAndMenuProxy();

  virtual void Update() override;
  virtual void SetAction(nsAction* pAction) override;
  virtual QAction* GetQAction();

private Q_SLOTS:
  void OnTriggered();

private:
  void StatusUpdateEventHandler(nsAction* pAction);

private:
  QPointer<QAction> m_pQtAction;
};


class NS_GUIFOUNDATION_DLL nsQtLabeledSlider : public QWidget
{
  Q_OBJECT

public:
  nsQtLabeledSlider(QWidget* pParent);

  QLabel* m_pLabel;
  QSlider* m_pSlider;
};


class NS_GUIFOUNDATION_DLL nsQtSliderWidgetAction : public QWidgetAction
{
  Q_OBJECT

public:
  nsQtSliderWidgetAction(QWidget* pParent);
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

  nsInt32 m_iMinimum;
  nsInt32 m_iMaximum;
  nsInt32 m_iValue;
};

class NS_GUIFOUNDATION_DLL nsQtSliderProxy : public nsQtActionProxy
{
  Q_OBJECT

public:
  nsQtSliderProxy();
  ~nsQtSliderProxy();

  virtual void Update() override;
  virtual void SetAction(nsAction* pAction) override;

  virtual QAction* GetQAction() override;

private Q_SLOTS:
  void OnValueChanged(int value);

private:
  void StatusUpdateEventHandler(nsAction* pAction);

private:
  QPointer<nsQtSliderWidgetAction> m_pQtAction;
};
