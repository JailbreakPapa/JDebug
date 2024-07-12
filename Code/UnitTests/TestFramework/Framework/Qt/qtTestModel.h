#pragma once

#ifdef NS_USE_QT

#  include <QAbstractItemModel>
#  include <QColor>
#  include <QIcon>
#  include <TestFramework/Framework/Qt/qtTestFramework.h>
#  include <TestFramework/TestFrameworkDLL.h>

class nsQtTestFramework;

/// \brief Helper class that stores the test hierarchy used in nsQtTestModel.
class nsQtTestModelEntry
{
public:
  nsQtTestModelEntry(const nsTestFrameworkResult* pResult, nsInt32 iTestIndex = -1, nsInt32 iSubTestIndex = -1);
  ~nsQtTestModelEntry();

private:
  nsQtTestModelEntry(nsQtTestModelEntry&);
  void operator=(nsQtTestModelEntry&);

public:
  enum nsTestModelEntryType
  {
    RootNode,
    TestNode,
    SubTestNode
  };

  void ClearEntries();
  nsUInt32 GetNumSubEntries() const;
  nsQtTestModelEntry* GetSubEntry(nsUInt32 uiIndex) const;
  void AddSubEntry(nsQtTestModelEntry* pEntry);
  nsQtTestModelEntry* GetParentEntry() const { return m_pParentEntry; }
  nsUInt32 GetIndexInParent() const { return m_uiIndexInParent; }
  nsTestModelEntryType GetNodeType() const;
  const nsTestResultData* GetTestResult() const;
  nsInt32 GetTestIndex() const { return m_iTestIndex; }
  nsInt32 GetSubTestIndex() const { return m_iSubTestIndex; }

private:
  const nsTestFrameworkResult* m_pResult;
  nsInt32 m_iTestIndex;
  nsInt32 m_iSubTestIndex;

  nsQtTestModelEntry* m_pParentEntry = nullptr;
  nsUInt32 m_uiIndexInParent = 0;
  std::deque<nsQtTestModelEntry*> m_SubEntries;
};

/// \brief A Model that lists all unit tests and sub-tests in a tree.
class NS_TEST_DLL nsQtTestModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  nsQtTestModel(QObject* pParent, nsQtTestFramework* pTestFramework);
  virtual ~nsQtTestModel();

  void Reset();
  void InvalidateAll();
  void TestDataChanged(nsInt32 iTestIndex, nsInt32 iSubTestIndex);

  struct UserRoles
  {
    enum Enum
    {
      Duration = Qt::UserRole,
      DurationColor = Qt::UserRole + 1,
    };
  };

  struct Columns
  {
    enum Enum
    {
      Name = 0,
      Status,
      Duration,
      Errors,
      Asserts,
      Progress,
      ColumnCount,
    };
  };

public: // QAbstractItemModel interface
  virtual QVariant data(const QModelIndex& index, int iRole) const override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QVariant headerData(int iSection, Qt::Orientation orientation, int iRole = Qt::DisplayRole) const override;
  virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& index) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int iRole = Qt::EditRole) override;

public Q_SLOTS:
  void UpdateModel();

private:
  nsQtTestFramework* m_pTestFramework;
  nsTestFrameworkResult* m_pResult;
  nsQtTestModelEntry m_Root;
  QColor m_SucessColor;
  QColor m_FailedColor;
  QColor m_CustomStatusColor;
  QColor m_TestColor;
  QColor m_SubTestColor;
  QIcon m_TestIcon;
  QIcon m_TestIconOff;
};

#endif
