#pragma once

#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_PickDocumentObjectDlg.h>
#include <QDialog>

class wdDocumentObject;

class WD_GUIFOUNDATION_DLL wdQtPickDocumentObjectDlg : public QDialog, public Ui_PickDocumentObjectDlg
{
  Q_OBJECT

public:
  struct Element
  {
    const wdDocumentObject* m_pObject;
    wdString m_sDisplayName;
  };

  wdQtPickDocumentObjectDlg(QWidget* pParent, const wdArrayPtr<Element>& objects, const wdUuid& currentObject);

  /// Stores the result that the user picked
  const wdDocumentObject* m_pPickedObject = nullptr;

private Q_SLOTS:
  void on_ObjectTree_itemDoubleClicked(QTreeWidgetItem* pItem, int column);

private:
  void UpdateTable();

  wdArrayPtr<Element> m_Objects;
  wdUuid m_CurrentObject;
};

