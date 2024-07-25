#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QWidgetAction>

class nsQtSearchWidget;
class QTreeWidget;
class QTreeWidgetItem;
class nsQtTreeSearchFilterModel;
class QStandardItemModel;
class QTreeView;
class QStandardItem;

/// \brief Implements an item for insertion into a QMenu that shows a search bar and a hierarchical list of options.
///
/// Fill the searchable menu object with items (use slashes to indicate hierarchy) then use QMenu::addAction to insert it
/// into another QMenu.
/// Connect to MenuItemTriggered() to handle the item activation and also call QMenu::close() on the parent menu.
class NS_GUIFOUNDATION_DLL nsQtSearchableMenu : public QWidgetAction
{
  Q_OBJECT
public:
  /// \brief The parent should usually be a QMenu into which this QWidgetAction is inserted as an action.
  nsQtSearchableMenu(QObject* pParent);

  /// \brief Use slashes in the szInternalPath to separate sub-items.
  void AddItem(nsStringView sDisplayName, const char* szInternalPath, const QVariant& variant, QIcon icon = QIcon());

  /// \brief Returns the currently entered search text.
  QString GetSearchText() const;

  /// \brief Sets up the internal data model and ensures the menu's search bar gets input focus. Do this after adding the item to the parent menu.
  void Finalize(const QString& sSearchText);

Q_SIGNALS:
  /// \brief Signaled when an item is double clicked or otherwise selected for activation.
  void MenuItemTriggered(const QString& sName, const QVariant& variant);

  /// \brief Triggered whenever the search text is modified.
  void SearchTextChanged(const QString& sText);

private Q_SLOTS:
  void OnItemActivated(const QModelIndex& index);
  void OnEnterPressed();
  void OnSpecialKeyPressed(Qt::Key key);
  void OnSearchChanged(const QString& text);
  void OnShow();

protected:
  virtual bool eventFilter(QObject*, QEvent*) override;

private:
  QStandardItem* CreateCategoryMenu(nsStringView sCategory);
  bool SelectFirstLeaf(QModelIndex parent);

  QWidget* m_pGroup;
  nsQtSearchWidget* m_pSearch;
  nsQtTreeSearchFilterModel* m_pFilterModel;
  QTreeView* m_pTreeView;
  QStandardItemModel* m_pItemModel;
  nsMap<nsString, QStandardItem*> m_Hierarchy;
};
