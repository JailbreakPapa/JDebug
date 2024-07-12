#pragma once

#ifdef NS_USE_QT

#  include <QStyledItemDelegate>
#  include <TestFramework/Framework/Qt/qtTestFramework.h>
#  include <TestFramework/TestFrameworkDLL.h>

class nsQtTestFramework;

/// \brief Delegate for nsQtTestModel which shows bars for the test durations.
class NS_TEST_DLL nsQtTestDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  nsQtTestDelegate(QObject* pParent);
  virtual ~nsQtTestDelegate();

public: // QStyledItemDelegate interface
  virtual void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif
