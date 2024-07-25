#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Strings/String.h>
#include <Inspector/ui_DataTransferWidget.h>
#include <ads/DockWidget.h>

class nsQtDataWidget : public ads::CDockWidget, public Ui_DataTransferWidget
{
public:
  Q_OBJECT

public:
  nsQtDataWidget(QWidget* pParent = 0);

  static nsQtDataWidget* s_pWidget;

private Q_SLOTS:
  virtual void on_ButtonRefresh_clicked();
  virtual void on_ComboTransfers_currentIndexChanged(int index);
  virtual void on_ComboItems_currentIndexChanged(int index);
  virtual void on_ButtonSave_clicked();
  virtual void on_ButtonOpen_clicked();

public:
  static void ProcessTelemetry(void* pUnuseed);

  void ResetStats();

private:
  struct TransferDataObject
  {
    nsString m_sMimeType;
    nsString m_sExtension;
    nsContiguousMemoryStreamStorage m_Storage;
    nsString m_sFileName;
  };

  struct TransferData
  {
    nsMap<nsString, TransferDataObject> m_Items;
  };

  bool SaveToFile(TransferDataObject& item, nsStringView sFile);

  TransferDataObject* GetCurrentItem();
  TransferData* GetCurrentTransfer();

  nsMap<nsString, TransferData> m_Transfers;
};
