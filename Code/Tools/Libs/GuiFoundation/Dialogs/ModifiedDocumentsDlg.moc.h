#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_ModifiedDocumentsDlg.h>
#include <QDialog>
#include <ToolsFoundation/Document/Document.h>

class NS_GUIFOUNDATION_DLL nsQtModifiedDocumentsDlg : public QDialog, public Ui_DocumentList
{
public:
  Q_OBJECT

public:
  nsQtModifiedDocumentsDlg(QWidget* pParent, const nsHybridArray<nsDocument*, 32>& modifiedDocs);


private Q_SLOTS:
  void on_ButtonSaveSelected_clicked();
  void on_ButtonDontSave_clicked();
  void SlotSaveDocument();
  void SlotSelectionChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
  nsResult SaveDocument(nsDocument* pDoc);

  nsHybridArray<nsDocument*, 32> m_ModifiedDocs;
};
