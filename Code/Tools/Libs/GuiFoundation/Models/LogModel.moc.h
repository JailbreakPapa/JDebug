#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Logging/LogEntry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QAbstractItemModel>

/// \brief The Qt model that represents log output for a view
class NS_GUIFOUNDATION_DLL nsQtLogModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  nsQtLogModel(QObject* pParent);
  void Clear();
  void SetLogLevel(nsLogMsgType::Enum logLevel);
  void SetSearchText(const char* szText);
  void AddLogMsg(const nsLogEntry& msg);

  nsUInt32 GetVisibleItemCount() const { return m_VisibleMessages.GetCount(); }

  nsUInt32 GetNumErrors() const { return m_uiNumErrors; }
  nsUInt32 GetNumSeriousWarnings() const { return m_uiNumSeriousWarnings; }
  nsUInt32 GetNumWarnings() const { return m_uiNumWarnings; }

public: // QAbstractItemModel interface
  virtual QVariant data(const QModelIndex& index, int iRole) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(int iSection, Qt::Orientation orientation, int iRole = Qt::DisplayRole) const override;
  virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& index) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

Q_SIGNALS:
  void NewErrorsOrWarnings(const char* szLatest, bool bError);

private Q_SLOTS:
  /// \brief Adds queued messages from a different thread to the model.
  void ProcessNewMessages();

private:
  void Invalidate();
  bool IsFiltered(const nsLogEntry& lm) const;
  void UpdateVisibleEntries() const;

  nsLogMsgType::Enum m_LogLevel;
  nsString m_sSearchText;
  nsDeque<nsLogEntry> m_AllMessages;

  mutable bool m_bIsValid;
  mutable nsDeque<const nsLogEntry*> m_VisibleMessages;
  mutable nsHybridArray<const nsLogEntry*, 16> m_BlockQueue;

  mutable nsMutex m_NewMessagesMutex;
  nsDeque<nsLogEntry> m_NewMessages;

  nsUInt32 m_uiNumErrors = 0;
  nsUInt32 m_uiNumSeriousWarnings = 0;
  nsUInt32 m_uiNumWarnings = 0;
};
