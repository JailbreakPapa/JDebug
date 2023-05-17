#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class QProgressDialog;
class QWinTaskbarProgress;
class QWinTaskbarButton;
class wdProgress;
struct wdProgressEvent;

/// \brief A Qt implementation to display the state of an wdProgress instance.
///
/// Create a single instance of this at application startup and link it to an wdProgress instance.
/// Whenever the instance's progress state changes, this class will display a simple progress bar.
class WD_GUIFOUNDATION_DLL wdQtProgressbar
{
public:
  wdQtProgressbar();
  ~wdQtProgressbar();

  /// \brief Sets the wdProgress instance that should be visualized.
  void SetProgressbar(wdProgress* pProgress);

  bool IsProcessingEvents() const { return m_iNestedProcessEvents > 0; }

private:
  void ProgressbarEventHandler(const wdProgressEvent& e);

  void EnsureCreated();
  void EnsureDestroyed();

  QProgressDialog* m_pDialog = nullptr;
  wdProgress* m_pProgress = nullptr;
  wdInt32 m_iNestedProcessEvents = 0;

  QMetaObject::Connection m_OnDialogDestroyed;
};
