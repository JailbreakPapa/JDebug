#pragma once

#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_PickDocumentObjectDlg.h>
#include <QDialog>

class nsDocumentObject;

class NS_GUIFOUNDATION_DLL nsQtPickDocumentObjectDlg : public QDialog, public Ui_PickDocumentObjectDlg
{
  Q_OBJECT

public:
  struct Element
  {
    const nsDocumentObject* m_pObject;
    nsString m_sDisplayName;
  };

  nsQtPickDocumentObjectDlg(QWidget* pParent, const nsArrayPtr<Element>& objects, const nsUuid& currentObject);

  /// Stores the result that the user picked
  const nsDocumentObject* m_pPickedObject = nullptr;

private Q_SLOTS:
  void on_ObjectTree_itemDoubleClicked(QTreeWidgetItem* pItem, int column);

private:
  void UpdateTable();

  nsArrayPtr<Element> m_Objects;
  nsUuid m_CurrentObject;
};
