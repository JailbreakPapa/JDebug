#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Status.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <ToolsFoundation/Document/DocumentManager.h>

#include <QMainWindow>

class nsQtContainerWindow;
class nsDocument;
class nsQtDocumentWindow;
class QLabel;
class QToolButton;

struct nsQtDocumentWindowEvent
{
  enum Type
  {
    WindowClosing,           ///< Sent shortly before the window is being deleted
    WindowClosed,            ///< Sent AFTER the window has been deleted. The pointer is given, but not valid anymore!
    WindowDecorationChanged, ///< Window title or icon has changed
    BeforeRedraw,            ///< Sent shortly before the content of the window is being redrawn
  };

  Type m_Type;
  nsQtDocumentWindow* m_pWindow;
};

/// \brief Base class for all document windows. Handles the most basic document window management.
class NS_GUIFOUNDATION_DLL nsQtDocumentWindow : public QMainWindow
{
  Q_OBJECT

public:
  static nsEvent<const nsQtDocumentWindowEvent&> s_Events;

public:
  nsQtDocumentWindow(nsDocument* pDocument);
  nsQtDocumentWindow(const char* szUniqueName);
  virtual ~nsQtDocumentWindow();

  void EnsureVisible();

  virtual nsString GetWindowIcon() const;
  virtual nsString GetDisplayName() const { return GetUniqueName(); }
  virtual nsString GetDisplayNameShort() const;

  const char* GetUniqueName() const { return m_sUniqueName; }

  /// \brief The 'GroupName' is used for serializing window layouts. It should be unique among different window types.
  virtual const char* GetWindowLayoutGroupName() const = 0;

  nsDocument* GetDocument() const { return m_pDocument; }

  nsStatus SaveDocument();

  bool CanCloseWindow();
  void CloseDocumentWindow();

  void ScheduleRestoreWindowLayout();

  bool IsVisibleInContainer() const { return m_bIsVisibleInContainer; }
  void SetTargetFramerate(nsInt16 iTargetFPS);

  void TriggerRedraw();

  virtual void RequestWindowTabContextMenu(const QPoint& globalPos);

  static const nsDynamicArray<nsQtDocumentWindow*>& GetAllDocumentWindows() { return s_AllDocumentWindows; }

  static nsQtDocumentWindow* FindWindowByDocument(const nsDocument* pDocument);
  nsQtContainerWindow* GetContainerWindow() const;

  /// \brief Shows the given message for the given duration in the statusbar, then shows the permanent message again.
  void ShowTemporaryStatusBarMsg(const nsFormatString& text, nsTime duration = nsTime::MakeFromSeconds(5));

  /// \brief Sets which text to show permanently in the statusbar. Set an empty string to clear the message.
  void SetPermanentStatusBarMsg(const nsFormatString& text);

  /// \brief For unit tests to take a screenshot of the window (may include multiple views) to do image comparisons.
  virtual void CreateImageCapture(const char* szOutputPath);

  /// \brief In 'safe' mode we want to prevent the documents from using the stored window layout state
  static bool s_bAllowRestoreWindowLayout;

protected:
  virtual void showEvent(QShowEvent* event) override;
  virtual void hideEvent(QHideEvent* event) override;
  virtual bool event(QEvent* event) override;
  virtual bool eventFilter(QObject* obj, QEvent* e) override;

  void FinishWindowCreation();

private Q_SLOTS:
  void SlotRestoreLayout();
  void SlotRedraw();
  void SlotQueuedDelete();
  void OnPermanentGlobalStatusClicked(bool);
  void OnStatusBarMessageChanged(const QString& sNewText);

private:
  void SaveWindowLayout();
  void RestoreWindowLayout();
  void DisableWindowLayoutSaving();

  void ShutdownDocumentWindow();

private:
  friend class nsQtContainerWindow;

  void SetVisibleInContainer(bool bVisible);

  bool m_bIsVisibleInContainer = false;
  bool m_bRedrawIsTriggered = false;
  bool m_bIsDrawingATM = false;
  bool m_bTriggerRedrawQueued = false;
  bool m_bAllowSaveWindowLayout = true;
  nsInt16 m_iTargetFramerate = 0;
  nsDocument* m_pDocument = nullptr;
  nsQtContainerWindow* m_pContainerWindow = nullptr;
  QLabel* m_pPermanentDocumentStatusText = nullptr;
  QToolButton* m_pPermanentGlobalStatusButton = nullptr;

private:
  void Constructor();
  void DocumentManagerEventHandler(const nsDocumentManager::Event& e);
  void DocumentEventHandler(const nsDocumentEvent& e);
  void UIServicesEventHandler(const nsQtUiServices::Event& e);
  void UIServicesTickEventHandler(const nsQtUiServices::TickEvent& e);

  virtual void InternalDeleteThis() { delete this; }
  virtual bool InternalCanCloseWindow();
  virtual void InternalCloseDocumentWindow();
  virtual void InternalVisibleInContainerChanged(bool bVisible) {}
  virtual void InternalRedraw() {}

  nsString m_sUniqueName;

  static nsDynamicArray<nsQtDocumentWindow*> s_AllDocumentWindows;
};
