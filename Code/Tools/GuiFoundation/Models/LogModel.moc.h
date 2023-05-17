#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Logging/LogEntry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QAbstractItemModel>

/// \brief The Qt model that represents log output for a view
class WD_GUIFOUNDATION_DLL wdQtLogModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  wdQtLogModel(QObject* pParent);
  void Clear();
  void SetLogLevel(wdLogMsgType::Enum logLevel);
  void SetSearchText(const char* szText);
  void AddLogMsg(const wdLogEntry& msg);

  wdUInt32 GetVisibleItemCount() const { return m_VisibleMessages.GetCount(); }

  wdUInt32 GetNumErrors() const { return m_uiNumErrors; }
  wdUInt32 GetNumSeriousWarnings() const { return m_uiNumSeriousWarnings; }
  wdUInt32 GetNumWarnings() const { return m_uiNumWarnings; }

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
  bool IsFiltered(const wdLogEntry& lm) const;
  void UpdateVisibleEntries() const;

  wdLogMsgType::Enum m_LogLevel;
  wdString m_sSearchText;
  wdDeque<wdLogEntry> m_AllMessages;

  mutable bool m_bIsValid;
  mutable wdDeque<const wdLogEntry*> m_VisibleMessages;
  mutable wdHybridArray<const wdLogEntry*, 16> m_BlockQueue;

  mutable wdMutex m_NewMessagesMutex;
  wdDeque<wdLogEntry> m_NewMessages;

  wdUInt32 m_uiNumErrors = 0;
  wdUInt32 m_uiNumSeriousWarnings = 0;
  wdUInt32 m_uiNumWarnings = 0;
};

