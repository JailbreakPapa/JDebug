#include <TestFramework/TestFrameworkPCH.h>

#ifdef NS_USE_QT

#  include <QApplication>
#  include <QPalette>
#  include <TestFramework/Framework/Qt/qtTestModel.h>

////////////////////////////////////////////////////////////////////////
// nsQtTestModelEntry public functions
////////////////////////////////////////////////////////////////////////

nsQtTestModelEntry::nsQtTestModelEntry(const nsTestFrameworkResult* pResult, nsInt32 iTestIndex, nsInt32 iSubTestIndex)
  : m_pResult(pResult)
  , m_iTestIndex(iTestIndex)
  , m_iSubTestIndex(iSubTestIndex)

{
}

nsQtTestModelEntry::~nsQtTestModelEntry()
{
  ClearEntries();
}

void nsQtTestModelEntry::ClearEntries()
{
  for (nsInt32 i = (nsInt32)m_SubEntries.size() - 1; i >= 0; --i)
  {
    delete m_SubEntries[i];
  }
  m_SubEntries.clear();
}
nsUInt32 nsQtTestModelEntry::GetNumSubEntries() const

{
  return (nsInt32)m_SubEntries.size();
}

nsQtTestModelEntry* nsQtTestModelEntry::GetSubEntry(nsUInt32 uiIndex) const
{
  if (uiIndex >= GetNumSubEntries())
    return nullptr;

  return m_SubEntries[uiIndex];
}

void nsQtTestModelEntry::AddSubEntry(nsQtTestModelEntry* pEntry)
{
  pEntry->m_pParentEntry = this;
  pEntry->m_uiIndexInParent = (nsUInt32)m_SubEntries.size();
  m_SubEntries.push_back(pEntry);
}

nsQtTestModelEntry::nsTestModelEntryType nsQtTestModelEntry::GetNodeType() const
{
  return (m_iTestIndex == -1) ? RootNode : ((m_iSubTestIndex == -1) ? TestNode : SubTestNode);
}

const nsTestResultData* nsQtTestModelEntry::GetTestResult() const
{
  switch (GetNodeType())
  {
    case nsQtTestModelEntry::TestNode:
    case nsQtTestModelEntry::SubTestNode:
      return &m_pResult->GetTestResultData(m_iTestIndex, m_iSubTestIndex);
    default:
      return nullptr;
  }
}

static QColor ToneColor(const QColor& inputColor, const QColor& toneColor)
{
  qreal fHue = toneColor.hueF();
  qreal fSaturation = 1.0f;
  qreal fLightness = inputColor.lightnessF();
  fLightness = nsMath::Clamp(fLightness, 0.20, 0.80);
  return QColor::fromHslF(fHue, fSaturation, fLightness);
}

////////////////////////////////////////////////////////////////////////
// nsQtTestModel public functions
////////////////////////////////////////////////////////////////////////

nsQtTestModel::nsQtTestModel(QObject* pParent, nsQtTestFramework* pTestFramework)
  : QAbstractItemModel(pParent)
  , m_pTestFramework(pTestFramework)
  , m_Root(nullptr)
{
  QPalette palette = QApplication::palette();
  m_pResult = &pTestFramework->GetTestResult();

  // Derive state colors from the current active palette.
  m_SucessColor = ToneColor(palette.text().color(), QColor(Qt::green)).toRgb();
  m_FailedColor = ToneColor(palette.text().color(), QColor(Qt::red)).toRgb();
  m_CustomStatusColor = ToneColor(palette.text().color(), QColor(Qt::yellow)).toRgb();

  m_TestColor = ToneColor(palette.base().color(), QColor(Qt::cyan)).toRgb();
  m_SubTestColor = ToneColor(palette.base().color(), QColor(Qt::blue)).toRgb();

  m_TestIcon = QIcon(":/Icons/Icons/pie.png");
  m_TestIconOff = QIcon(":/Icons/Icons/pie_off.png");

  UpdateModel();
}

nsQtTestModel::~nsQtTestModel()
{
  m_Root.ClearEntries();
}

void nsQtTestModel::Reset()
{
  beginResetModel();
  endResetModel();
}

void nsQtTestModel::InvalidateAll()
{
  dataChanged(QModelIndex(), QModelIndex());
}

void nsQtTestModel::TestDataChanged(nsInt32 iTestIndex, nsInt32 iSubTestIndex)
{
  QModelIndex TestModelIndex = index(iTestIndex, 0);
  // Invalidate whole test row
  Q_EMIT dataChanged(TestModelIndex, index(iTestIndex, columnCount() - 1));

  // Invalidate all sub-tests
  const nsQtTestModelEntry* pEntry = (nsQtTestModelEntry*)TestModelIndex.internalPointer();
  nsInt32 iChildren = (nsInt32)pEntry->GetNumSubEntries();
  Q_EMIT dataChanged(index(0, 0, TestModelIndex), index(iChildren - 1, columnCount() - 1, TestModelIndex));
}


////////////////////////////////////////////////////////////////////////
// nsQtTestModel QAbstractItemModel functions
////////////////////////////////////////////////////////////////////////

QVariant nsQtTestModel::data(const QModelIndex& index, int iRole) const
{
  if (!index.isValid())
    return QVariant();

  const nsQtTestModelEntry* pEntry = (nsQtTestModelEntry*)index.internalPointer();
  const nsQtTestModelEntry* pParentEntry = pEntry->GetParentEntry();
  const nsQtTestModelEntry::nsTestModelEntryType entryType = pEntry->GetNodeType();

  const nsInt32 iExecutingTest = m_pTestFramework->GetCurrentTestIndex();
  const nsInt32 iExecutingSubTest = m_pTestFramework->GetCurrentSubTestIndex();

  const bool bIsExecuting = pEntry->GetTestIndex() == iExecutingTest && pEntry->GetSubTestIndex() == iExecutingSubTest;

  bool bTestEnabled = true;
  bool bParentEnabled = true;
  bool bIsSubTest = entryType == nsQtTestModelEntry::SubTestNode;
  const std::string& testUnavailableReason = m_pTestFramework->IsTestAvailable(bIsSubTest ? pParentEntry->GetTestIndex() : pEntry->GetTestIndex());

  if (bIsSubTest)
  {
    bTestEnabled = m_pTestFramework->IsSubTestEnabled(pEntry->GetTestIndex(), pEntry->GetSubTestIndex());
    bParentEnabled = m_pTestFramework->IsTestEnabled(pParentEntry->GetTestIndex());
  }
  else
  {
    bTestEnabled = m_pTestFramework->IsTestEnabled(pEntry->GetTestIndex());
  }

  const nsTestResultData& TestResult = *pEntry->GetTestResult();

  if (bIsExecuting && iRole == Qt::BackgroundRole)
  {
    return QColor(115, 100, 40);
  }

  // Name
  if (index.column() == Columns::Name)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        return QString(TestResult.m_sName.c_str());
      }
      case Qt::CheckStateRole:
      {
        return bTestEnabled ? Qt::Checked : Qt::Unchecked;
      }
      case Qt::DecorationRole:
      {
        return (bTestEnabled && bParentEnabled) ? m_TestIcon : m_TestIconOff;
      }
      case Qt::ForegroundRole:
      {
        if (!testUnavailableReason.empty())
        {
          QPalette palette = QApplication::palette();
          return palette.color(QPalette::Disabled, QPalette::Text);
        }
      }
      case Qt::ToolTipRole:
      {
        if (!testUnavailableReason.empty())
        {
          return QString("Test not available: %1").arg(testUnavailableReason.c_str());
        }
      }
      default:
        return QVariant();
    }
  }
  // Status
  else if (index.column() == Columns::Status)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        if (!testUnavailableReason.empty())
        {
          return QString("Test not available: %1").arg(testUnavailableReason.c_str());
        }
        else if (bTestEnabled && bParentEnabled)
        {
          if (bIsSubTest)
          {
            return QString("Enabled");
          }
          else
          {
            // Count sub-test status
            const nsUInt32 iSubTests = m_pResult->GetSubTestCount(pEntry->GetTestIndex());
            const nsUInt32 iEnabled = m_pTestFramework->GetSubTestEnabledCount(pEntry->GetTestIndex());

            if (iEnabled == iSubTests)
            {
              return QString("All Enabled");
            }

            return QString("%1 / %2 Enabled").arg(iEnabled).arg(iSubTests);
          }
        }
        else
        {
          return QString("Disabled");
        }
      }
      case Qt::TextAlignmentRole:
      {
        return Qt::AlignRight;
      }
      default:
        return QVariant();
    }
  }
  // Duration
  else if (index.column() == Columns::Duration)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        return QLocale(QLocale::English).toString(TestResult.m_fTestDuration, 'f', 4);
      }
      case Qt::TextAlignmentRole:
      {
        return Qt::AlignRight;
      }
      /*case Qt::BackgroundRole:
        {
          QPalette palette = QApplication::palette();
          return palette.alternateBase().color();
        }*/
      case UserRoles::Duration:
      {
        if (bIsSubTest && TestResult.m_bExecuted)
        {
          return TestResult.m_fTestDuration / pParentEntry->GetTestResult()->m_fTestDuration;
        }
        else if (TestResult.m_bExecuted)
        {
          return TestResult.m_fTestDuration / m_pTestFramework->GetTotalTestDuration();
        }
        return QVariant();
      }
      case UserRoles::DurationColor:
      {
        if (TestResult.m_bExecuted)
        {
          return (bIsSubTest ? m_SubTestColor : m_TestColor);
        }
        return QVariant();
      }
      default:
        return QVariant();
    }
  }
  // Errors
  else if (index.column() == Columns::Errors)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        return QString("%1 / %2")
          .arg(m_pResult->GetErrorMessageCount(pEntry->GetTestIndex(), pEntry->GetSubTestIndex()))
          .arg(m_pResult->GetOutputMessageCount(pEntry->GetTestIndex(), pEntry->GetSubTestIndex()));
      }
      case Qt::BackgroundRole:
      {
        QPalette palette = QApplication::palette();
        return palette.alternateBase().color();
      }
      case Qt::ForegroundRole:
      {
        if (TestResult.m_bExecuted)
        {
          return (m_pResult->GetErrorMessageCount(pEntry->GetTestIndex(), pEntry->GetSubTestIndex()) == 0) ? m_SucessColor : m_FailedColor;
        }
        return QVariant();
      }
      case Qt::TextAlignmentRole:
      {
        return Qt::AlignRight;
      }

      default:
        return QVariant();
    }
  }
  // Assert Count
  else if (index.column() == Columns::Asserts)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        return QString("%1").arg(TestResult.m_iTestAsserts);
      }
      case Qt::BackgroundRole:
      {
        QPalette palette = QApplication::palette();
        return palette.alternateBase().color();
      }
      case Qt::TextAlignmentRole:
      {
        return Qt::AlignRight;
      }

      default:
        return QVariant();
    }
  }
  // Progress
  else if (index.column() == Columns::Progress)
  {
    switch (iRole)
    {
      case Qt::DisplayRole:
      {
        if (!testUnavailableReason.empty())
        {
          return QString("Test not available: %1").arg(testUnavailableReason.c_str());
        }
        else if (bTestEnabled && bParentEnabled)
        {
          if (bIsSubTest)
          {
            if (TestResult.m_bExecuted)
            {
              return (TestResult.m_bSuccess) ? QString("Passed") : QString("Failed");
            }
            else
            {
              if (!TestResult.m_sCustomStatus.empty())
              {
                return QString(TestResult.m_sCustomStatus.c_str());
              }

              return QString("Pending");
            }
          }
          else
          {
            // Count sub-test status

            const nsUInt32 iEnabled = m_pTestFramework->GetSubTestEnabledCount(pEntry->GetTestIndex());
            const nsUInt32 iExecuted = m_pResult->GetSubTestCount(pEntry->GetTestIndex(), nsTestResultQuery::Executed);
            const nsUInt32 iSucceeded = m_pResult->GetSubTestCount(pEntry->GetTestIndex(), nsTestResultQuery::Success);

            if (TestResult.m_bExecuted && iExecuted == iEnabled)
            {
              return (TestResult.m_bSuccess && iExecuted == iSucceeded) ? QString("Passed") : QString("Failed");
            }
            else
            {
              return QString("%1 / %2 Executed").arg(iExecuted).arg(iEnabled);
            }
          }
        }
        else
        {
          return QString("Disabled");
        }
      }
      case Qt::BackgroundRole:
      {
        QPalette palette = QApplication::palette();
        return palette.alternateBase().color();
      }
      case Qt::ForegroundRole:
      {
        if (!testUnavailableReason.empty())
        {
          QPalette palette = QApplication::palette();
          return palette.color(QPalette::Disabled, QPalette::Text);
        }
        else if (TestResult.m_bExecuted)
        {
          return TestResult.m_bSuccess ? m_SucessColor : m_FailedColor;
        }
        else if (!TestResult.m_sCustomStatus.empty())
        {
          return m_CustomStatusColor;
        }

        return QVariant();
      }
      case Qt::TextAlignmentRole:
      {
        return Qt::AlignRight;
      }

      default:
        return QVariant();
    }
  }

  return QVariant();
}

Qt::ItemFlags nsQtTestModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::ItemFlags();

  nsQtTestModelEntry* pEntry = (nsQtTestModelEntry*)index.internalPointer();
  if (pEntry == &m_Root)
    return Qt::ItemFlags();

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

QVariant nsQtTestModel::headerData(int iSection, Qt::Orientation orientation, int iRole) const
{
  if (orientation == Qt::Horizontal && iRole == Qt::DisplayRole)
  {
    switch (iSection)
    {
      case Columns::Name:
        return QString("Name");
      case Columns::Status:
        return QString("Status");
      case Columns::Duration:
        return QString("Duration (ms)");
      case Columns::Errors:
        return QString("Errors / Output");
      case Columns::Asserts:
        return QString("Checks");
      case Columns::Progress:
        return QString("Progress");
    }
  }
  return QVariant();
}

QModelIndex nsQtTestModel::index(int iRow, int iColumn, const QModelIndex& parent) const
{
  if (!hasIndex(iRow, iColumn, parent))
    return QModelIndex();

  const nsQtTestModelEntry* pParent = nullptr;

  if (!parent.isValid())
    pParent = &m_Root;
  else
    pParent = static_cast<nsQtTestModelEntry*>(parent.internalPointer());

  nsQtTestModelEntry* pEntry = pParent->GetSubEntry(iRow);
  return pEntry ? createIndex(iRow, iColumn, pEntry) : QModelIndex();
}

QModelIndex nsQtTestModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  nsQtTestModelEntry* pChild = static_cast<nsQtTestModelEntry*>(index.internalPointer());
  nsQtTestModelEntry* pParent = pChild->GetParentEntry();

  if (pParent == &m_Root)
    return QModelIndex();

  return createIndex(pParent->GetIndexInParent(), 0, pParent);
}

int nsQtTestModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;

  const nsQtTestModelEntry* pParent = nullptr;

  if (!parent.isValid())
    pParent = &m_Root;
  else
    pParent = static_cast<nsQtTestModelEntry*>(parent.internalPointer());

  return pParent->GetNumSubEntries();
}

int nsQtTestModel::columnCount(const QModelIndex& parent) const
{
  return Columns::ColumnCount;
}

bool nsQtTestModel::setData(const QModelIndex& index, const QVariant& value, int iRole)
{
  nsQtTestModelEntry* pEntry = static_cast<nsQtTestModelEntry*>(index.internalPointer());
  if (pEntry == nullptr || index.column() != Columns::Name || iRole != Qt::CheckStateRole)
    return false;

  if (pEntry->GetNodeType() == nsQtTestModelEntry::TestNode)
  {
    m_pTestFramework->SetTestEnabled(pEntry->GetTestIndex(), value.toBool());
    TestDataChanged(pEntry->GetIndexInParent(), -1);

    // if a test gets enabled in the UI, and all sub-tests are currently disabled,
    // enable all sub-tests as well
    // if some set of sub-tests is already enabled and some are disabled,
    // do not mess with the user's choice of enabled tests
    bool bEnableSubTests = value.toBool();
    for (nsUInt32 subIdx = 0; subIdx < pEntry->GetNumSubEntries(); ++subIdx)
    {
      if (m_pTestFramework->IsSubTestEnabled(pEntry->GetTestIndex(), subIdx))
      {
        bEnableSubTests = false;
        break;
      }
    }

    if (bEnableSubTests)
    {
      for (nsUInt32 subIdx = 0; subIdx < pEntry->GetNumSubEntries(); ++subIdx)
      {
        m_pTestFramework->SetSubTestEnabled(pEntry->GetTestIndex(), subIdx, true);
        TestDataChanged(pEntry->GetIndexInParent(), subIdx);
      }
    }
  }
  else
  {
    m_pTestFramework->SetSubTestEnabled(pEntry->GetTestIndex(), pEntry->GetSubTestIndex(), value.toBool());
    TestDataChanged(pEntry->GetParentEntry()->GetIndexInParent(), pEntry->GetIndexInParent());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////
// nsQtTestModel public slots
////////////////////////////////////////////////////////////////////////

void nsQtTestModel::UpdateModel()
{
  m_Root.ClearEntries();
  if (m_pResult == nullptr)
    return;

  const nsUInt32 uiTestCount = m_pResult->GetTestCount();
  for (nsUInt32 uiTestIndex = 0; uiTestIndex < uiTestCount; ++uiTestIndex)
  {
    nsQtTestModelEntry* pTestModelEntry = new nsQtTestModelEntry(m_pResult, uiTestIndex);
    m_Root.AddSubEntry(pTestModelEntry);

    const nsUInt32 uiSubTestCount = m_pResult->GetSubTestCount(uiTestIndex);
    for (nsUInt32 uiSubTestIndex = 0; uiSubTestIndex < uiSubTestCount; ++uiSubTestIndex)
    {
      nsQtTestModelEntry* pSubTestModelEntry = new nsQtTestModelEntry(m_pResult, uiTestIndex, uiSubTestIndex);
      pTestModelEntry->AddSubEntry(pSubTestModelEntry);
    }
  }
  // reset();
}


#endif
