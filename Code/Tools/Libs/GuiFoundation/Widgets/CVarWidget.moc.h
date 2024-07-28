#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Core/Console/Console.h>
#include <Foundation/Basics.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Variant.h>
#include <GuiFoundation/ui_CVarWidget.h>
#include <QItemDelegate>
#include <QPointer>
#include <QWidget>

class QStandardItemModel;
class QSortFilterProxyModel;
class nsQtCVarModel;
class nsQtCVarWidget;

class nsQtCVarItemDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  explicit nsQtCVarItemDelegate(QObject* pParent = nullptr)
    : QItemDelegate(pParent)
  {
  }

  virtual QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  virtual void setEditorData(QWidget* pEditor, const QModelIndex& index) const override;
  virtual void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& index) const override;

  nsQtCVarModel* m_pModel = nullptr;

private:
  mutable QModelIndex m_Index;

private Q_SLOTS:
  void onComboChanged(int);
};

class nsQtCVarModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  nsQtCVarModel(nsQtCVarWidget* pOwner);
  ~nsQtCVarModel();

  void BeginResetModel();
  void EndResetModel();

public: // QAbstractItemModel interface
  virtual QVariant headerData(int iSection, Qt::Orientation orientation, int iRole = Qt::DisplayRole) const override;
  virtual QVariant data(const QModelIndex& index, int iRole) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int iRole = Qt::EditRole) override;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
  virtual QModelIndex index(int iRow, int iColumn, const QModelIndex& parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& index) const override;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

public:
  struct Entry
  {
    nsString m_sFullName;
    QString m_sDisplayString;
    Entry* m_pParentEntry = nullptr;
    nsDynamicArray<Entry*> m_ChildEntries;

    QString m_sPlugin;      // in which plugin a CVar is defined
    QString m_sDescription; // CVar description text
    nsVariant m_Value;
  };

  Entry* CreateEntry(const char* szName);

  nsQtCVarWidget* m_pOwner = nullptr;
  nsDynamicArray<Entry*> m_RootEntries;
  nsDeque<Entry> m_AllEntries;
};

/// \brief Data used by nsQtCVarWidget to represent CVar states
struct NS_GUIFOUNDATION_DLL nsCVarWidgetData
{
  mutable bool m_bNewEntry = true;

  nsString m_sPlugin;      // in which plugin a CVar is defined
  nsString m_sDescription; // CVar description text
  nsUInt8 m_uiType = 0;    // nsCVarType

  // 'union' over the different possible CVar types
  bool m_bValue = false;
  float m_fValue = 0.0f;
  nsInt32 m_iValue = 0;
  nsString m_sValue;
};

/// \brief Displays CVar values in a table and allows to modify them.
class NS_GUIFOUNDATION_DLL nsQtCVarWidget : public QWidget, public Ui_CVarWidget
{
  Q_OBJECT

public:
  nsQtCVarWidget(QWidget* pParent);
  ~nsQtCVarWidget();

  /// \brief Clears the table
  void Clear();

  /// \brief Recreates the full UI. This is necessary when elements were added or removed.
  void RebuildCVarUI(const nsMap<nsString, nsCVarWidgetData>& cvars);

  /// \brief Updates the existing UI. This is sufficient if values changed only.
  void UpdateCVarUI(const nsMap<nsString, nsCVarWidgetData>& cvars);

  void AddConsoleStrings(const nsStringBuilder& sEncoded);

  nsConsole& GetConsole() { return m_Console; }

Q_SIGNALS:
  void onBoolChanged(const char* szCVar, bool bNewValue);
  void onFloatChanged(const char* szCVar, float fNewValue);
  void onIntChanged(const char* szCVar, int iNewValue);
  void onStringChanged(const char* szCVar, const char* szNewValue);

private Q_SLOTS:
  void SearchTextChanged(const QString& text);
  void ConsoleEnterPressed();
  void ConsoleSpecialKeyPressed(Qt::Key key);

private:
  QPointer<nsQtCVarModel> m_pItemModel;
  QPointer<QSortFilterProxyModel> m_pFilterModel;
  QPointer<nsQtCVarItemDelegate> m_pItemDelegate;

  void OnConsoleEvent(const nsConsoleEvent& e);

  nsConsole m_Console;
};
