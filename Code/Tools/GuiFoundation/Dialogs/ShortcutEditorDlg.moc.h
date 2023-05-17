#pragma once

#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ShortcutEditorDlg.h>
#include <QDialog>

struct wdActionDescriptor;

class WD_GUIFOUNDATION_DLL wdQtShortcutEditorDlg : public QDialog, public Ui_ShortcutEditor
{
public:
  Q_OBJECT

public:
  wdQtShortcutEditorDlg(QWidget* pParent);
  ~wdQtShortcutEditorDlg();

  void UpdateTable();

private Q_SLOTS:
  void SlotSelectionChanged();
  void on_KeyEditor_editingFinished();

  void UpdateKeyEdit();

  void on_KeyEditor_keySequenceChanged(const QKeySequence& keySequence);
  void on_ButtonAssign_clicked();
  void on_ButtonRemove_clicked();
  void on_ButtonReset_clicked();

private:
  wdInt32 m_iSelectedAction;
  wdHybridArray<wdActionDescriptor*, 32> m_ActionDescs;
};

