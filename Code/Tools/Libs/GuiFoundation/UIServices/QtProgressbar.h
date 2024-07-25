#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Time.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class QProgressDialog;
class QWinTaskbarProgress;
class QWinTaskbarButton;
class nsProgress;
struct nsProgressEvent;

/// \brief A Qt implementation to display the state of an nsProgress instance.
///
/// Create a single instance of this at application startup and link it to an nsProgress instance.
/// Whenever the instance's progress state changes, this class will display a simple progress bar.
class NS_GUIFOUNDATION_DLL nsQtProgressbar
{
public:
  nsQtProgressbar();
  ~nsQtProgressbar();

  /// \brief Sets the nsProgress instance that should be visualized.
  void SetProgressbar(nsProgress* pProgress);

  bool IsProcessingEvents() const { return m_iNestedProcessEvents > 0; }

private:
  void ProgressbarEventHandler(const nsProgressEvent& e);

  void EnsureCreated();
  void EnsureDestroyed();

  QProgressDialog* m_pDialog = nullptr;
  nsProgress* m_pProgress = nullptr;
  nsInt32 m_iNestedProcessEvents = 0;

  QMetaObject::Connection m_OnDialogDestroyed;
};
