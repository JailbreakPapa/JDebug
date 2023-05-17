#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ModifiedDocumentsDlg.h>
#include <QDialog>
#include <ToolsFoundation/Document/Document.h>

class WD_GUIFOUNDATION_DLL wdQtModifiedDocumentsDlg : public QDialog, public Ui_DocumentList
{
public:
  Q_OBJECT

public:
  wdQtModifiedDocumentsDlg(QWidget* pParent, const wdHybridArray<wdDocument*, 32>& modifiedDocs);


private Q_SLOTS:
  void on_ButtonSaveSelected_clicked();
  void on_ButtonDontSave_clicked();
  void SlotSaveDocument();
  void SlotSelectionChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
  wdResult SaveDocument(wdDocument* pDoc);

  wdHybridArray<wdDocument*, 32> m_ModifiedDocs;
};

