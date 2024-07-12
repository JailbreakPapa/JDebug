#pragma once

#ifdef NS_USE_QT

#  include <QAbstractItemModel>
#  include <QDockWidget>
#  include <TestFramework/TestFrameworkDLL.h>
#  include <TestFramework/ui_qtLogMessageDock.h>
#  include <vector>

class nsQtTestFramework;
struct nsTestResultData;
class nsQtLogMessageModel;
class nsTestFrameworkResult;

/// \brief Dock widget that lists the output of a given nsResult struct.
class NS_TEST_DLL nsQtLogMessageDock : public QDockWidget, public Ui_qtLogMessageDock
{
  Q_OBJECT
public:
  nsQtLogMessageDock(QObject* pParent, const nsTestFrameworkResult* pResult);
  virtual ~nsQtLogMessageDock();

public Q_SLOTS:
  void resetModel();
  void currentTestResultChanged(const nsTestResultData* pTestResult);
  void currentTestSelectionChanged(const nsTestResultData* pTestResult);

private:
  nsQtLogMessageModel* m_pModel;
};

/// \brief Model used by nsQtLogMessageDock to list the output entries in nsResult.
class NS_TEST_DLL nsQtLogMessageModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  nsQtLogMessageModel(QObject* pParent, const nsTestFrameworkResult* pResult);
  virtual ~nsQtLogMessageModel();

  void resetModel();
  QModelIndex GetFirstIndexOfTestSelection();
  QModelIndex GetLastIndexOfTestSelection();

public Q_SLOTS:
  void currentTestResultChanged(const nsTestResultData* pTestResult);
  void currentTestSelectionChanged(const nsTestResultData* pTestResult);

public: // QAbstractItemModel interface
  virtual QVariant data(const QModelIndex& index, int iRole) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(int iSection, Qt::Orientation orientation, int iRole = Qt::DisplayRole) const override;
  virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& index) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
  void UpdateVisibleEntries();

private:
  const nsTestResultData* m_pCurrentTestSelection;
  const nsTestFrameworkResult* m_pTestResult;
  std::vector<nsUInt32> m_VisibleEntries;
  std::vector<nsUInt8> m_VisibleEntriesIndention;
};

#endif
