#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Status.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <ToolsFoundation/Document/DocumentManager.h>

#include <QMainWindow>

class wdQtContainerWindow;
class wdDocument;
class wdQtDocumentWindow;
class QLabel;
class QToolButton;

struct wdQtDocumentWindowEvent
{
  enum Type
  {
    WindowClosing,           ///< Sent shortly before the window is being deleted
    WindowClosed,            ///< Sent AFTER the window has been deleted. The pointer is given, but not valid anymore!
    WindowDecorationChanged, ///< Window title or icon has changed
    BeforeRedraw,            ///< Sent shortly before the content of the window is being redrawn
  };

  Type m_Type;
  wdQtDocumentWindow* m_pWindow;
};

/// \brief Base class for all document windows. Handles the most basic document window management.
class WD_GUIFOUNDATION_DLL wdQtDocumentWindow : public QMainWindow
{
  Q_OBJECT

public:
  static wdEvent<const wdQtDocumentWindowEvent&> s_Events;

public:
  wdQtDocumentWindow(wdDocument* pDocument);
  wdQtDocumentWindow(const char* szUniqueName);
  virtual ~wdQtDocumentWindow();

  void EnsureVisible();

  virtual wdString GetWindowIcon() const;
  virtual wdString GetDisplayName() const { return GetUniqueName(); }
  virtual wdString GetDisplayNameShort() const;

  const char* GetUniqueName() const { return m_sUniqueName; }

  /// \brief The 'GroupName' is used for serializing window layouts. It should be unique among different window types.
  virtual const char* GetWindowLayoutGroupName() const = 0;

  wdDocument* GetDocument() const { return m_pDocument; }

  wdStatus SaveDocument();

  bool CanCloseWindow();
  void CloseDocumentWindow();

  void ScheduleRestoreWindowLayout();

  bool IsVisibleInContainer() const { return m_bIsVisibleInContainer; }
  void SetTargetFramerate(wdInt16 iTargetFPS);

  void TriggerRedraw();

  virtual void RequestWindowTabContextMenu(const QPoint& globalPos);

  static const wdDynamicArray<wdQtDocumentWindow*>& GetAllDocumentWindows() { return s_AllDocumentWindows; }

  static wdQtDocumentWindow* FindWindowByDocument(const wdDocument* pDocument);
  wdQtContainerWindow* GetContainerWindow() const;

  /// \brief Shows the given message for the given duration in the statusbar, then shows the permanent message again.
  void ShowTemporaryStatusBarMsg(const wdFormatString& text, wdTime duration = wdTime::Seconds(5));

  /// \brief Sets which text to show permanently in the statusbar. Set an empty string to clear the message.
  void SetPermanentStatusBarMsg(const wdFormatString& text);

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
  friend class wdQtContainerWindow;

  void SetVisibleInContainer(bool bVisible);

  bool m_bIsVisibleInContainer = false;
  bool m_bRedrawIsTriggered = false;
  bool m_bIsDrawingATM = false;
  bool m_bTriggerRedrawQueued = false;
  bool m_bAllowSaveWindowLayout = true;
  wdInt16 m_iTargetFramerate = 0;
  wdDocument* m_pDocument = nullptr;
  wdQtContainerWindow* m_pContainerWindow = nullptr;
  QLabel* m_pPermanentDocumentStatusText = nullptr;
  QToolButton* m_pPermanentGlobalStatusButton = nullptr;

private:
  void Constructor();
  void DocumentManagerEventHandler(const wdDocumentManager::Event& e);
  void DocumentEventHandler(const wdDocumentEvent& e);
  void UIServicesEventHandler(const wdQtUiServices::Event& e);
  void UIServicesTickEventHandler(const wdQtUiServices::TickEvent& e);

  virtual void InternalDeleteThis() { delete this; }
  virtual bool InternalCanCloseWindow();
  virtual void InternalCloseDocumentWindow();
  virtual void InternalVisibleInContainerChanged(bool bVisible) {}
  virtual void InternalRedraw() {}

  wdString m_sUniqueName;

  static wdDynamicArray<wdQtDocumentWindow*> s_AllDocumentWindows;
};

