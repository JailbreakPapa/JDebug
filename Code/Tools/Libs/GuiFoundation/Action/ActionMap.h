#pragma once

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class nsDocument;

struct NS_GUIFOUNDATION_DLL nsActionMapDescriptor
{
  nsActionDescriptorHandle m_hAction; ///< Action to be mapped
  nsString m_sPath;                   ///< Path where the action should be mapped excluding the action's name, e.g. "File/New" for a menu item "File -> New -> Project..." .
  float m_fOrder;                     ///< Ordering key to sort actions in the mapping path.
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsActionMapDescriptor);

template <typename T>
class nsTreeNode
{
public:
  nsTreeNode()
    : m_pParent(nullptr)
  {
  }
  nsTreeNode(const T& data)
    : m_Data(data)
    , m_pParent(nullptr)
  {
  }
  ~nsTreeNode()
  {
    while (!m_Children.IsEmpty())
    {
      RemoveChild(0);
    }
  }

  const nsUuid& GetGuid() const { return m_Guid; }
  const nsTreeNode<T>* GetParent() const { return m_pParent; }
  nsTreeNode<T>* GetParent() { return m_pParent; }
  const nsHybridArray<nsTreeNode<T>*, 8>& GetChildren() const { return m_Children; }
  nsHybridArray<nsTreeNode<T>*, 8>& GetChildren() { return m_Children; }

  nsTreeNode<T>* InsertChild(const T& data, nsUInt32 uiIndex)
  {
    nsTreeNode<T>* pNode = NS_DEFAULT_NEW(nsTreeNode<T>, data);
    pNode->m_Guid = nsUuid::MakeUuid();
    m_Children.InsertAt(uiIndex, pNode);
    pNode->m_pParent = this;
    return pNode;
  }

  bool RemoveChild(nsUInt32 uiIndex)
  {
    if (uiIndex > m_Children.GetCount())
      return false;

    nsTreeNode<T>* pChild = m_Children[uiIndex];
    m_Children.RemoveAtAndCopy(uiIndex);
    NS_DEFAULT_DELETE(pChild);
    return true;
  }

  nsUInt32 GetParentIndex() const
  {
    NS_ASSERT_DEV(m_pParent != nullptr, "Can't compute parent index if no parent is present!");
    for (nsUInt32 i = 0; i < m_pParent->GetChildren().GetCount(); i++)
    {
      if (m_pParent->GetChildren()[i] == this)
        return i;
    }
    NS_REPORT_FAILURE("Couldn't find oneself in own parent!");
    return -1;
  }

  T m_Data;
  nsUuid m_Guid;

private:
  nsTreeNode<T>* m_pParent;
  nsHybridArray<nsTreeNode<T>*, 8> m_Children;
};

/// \brief Defines the structure of how actions are organized in a particular context.
///
/// Actions are usually commands that are exposed through UI.
/// For instance a button in a toolbar or a menu entry.
///
/// Actions are unique. Each action only exists once in nsActionManager.
///
/// An action map defines where in a menu an action shows up.
/// Actions are usually grouped by categories. So for example all actions related to opening, closing
/// or saving a document may be in one group. Their position within that group is defined through
/// an 'order' value. This allows plugins to insert actions easily.
///
/// A window might use multiple action maps to build different structures.
/// For example, usually there is one action map for a window menu, and another map for a toolbar.
/// These will contain different actions, and they are organized differently.
///
/// Action maps are created through nsActionMapManager and are simply identified by name.
class NS_GUIFOUNDATION_DLL nsActionMap
{
public:
  using TreeNode = nsTreeNode<nsActionMapDescriptor>;
  nsActionMap();
  ~nsActionMap();

  /// \brief Adds the given action to into the category or menu identified by sPath.
  ///
  /// All actions added to the same path will be sorted by 'fOrder' and the ones with the smaller values show up at the top.
  ///
  /// sPath must either be a fully qualified path OR the name of a uniquely named category or menu.
  /// If sPath is empty, the action (which may be a category itself) will be mapped into the root.
  /// This is common for top-level menus and for toolbars.
  ///
  /// If sPath is a fully qualified path, the segments are separated by slashes (/)
  /// and each segment must name either a category (see NS_REGISTER_CATEGORY) or a menu (see NS_REGISTER_MENU).
  ///
  /// sPath may also name a category or menu WITHOUT it being a full path. In this case the name must be unique.
  /// If sPath isn't empty and doesn't contain a slash, the system searches all available actions that are already in the action map.
  /// This allows you to insert an action into a category, without knowing the full path to that category.
  /// By convention, categories that are meant to be used that way are named "G.Something". The idea is, that where that category
  /// really shows up (and whether it is its own menu or just an area somewhere) may change in the future, or may be different
  /// in different contexts.
  ///
  /// To make it easier to use 'global' category names combined with an additional relative path, there is an overload of this function
  /// that takes an additional sSubPath argument.
  void MapAction(nsActionDescriptorHandle hAction, nsStringView sPath, float fOrder);

  /// \brief An overload of MapAction that takes a dedicated sPath and sSubPath argument for convenience.
  ///
  /// If sPath is a 'global' name of a category, it is searched for (see SearchPathForAction()).
  /// Afterwards sSubPath is appended and the result is forwarded to MapAction() as a single path string.
  void MapAction(nsActionDescriptorHandle hAction, nsStringView sPath, nsStringView sSubPath, float fOrder);

  /// \brief Removes the named action from the action map. The same rules for 'global' names apply as for MapAction().
  nsResult UnmapAction(nsActionDescriptorHandle hAction, nsStringView sPath);

  /// \brief Searches for an action with the given name and returns the full path to it.
  ///
  /// This is mainly meant to be used with (unique) names to categories (or menus).
  nsResult SearchPathForAction(nsStringView sUniqueName, nsStringBuilder& out_sPath) const;

  const TreeNode* GetRootObject() const { return &m_Root; }

  const nsActionMapDescriptor* GetDescriptor(const nsTreeNode<nsActionMapDescriptor>* pObject) const;

private:
  nsUuid MapAction(const nsActionMapDescriptor& desc);
  nsResult UnmapAction(const nsActionMapDescriptor& desc);
  nsResult UnmapAction(const nsUuid& guid);

  const nsActionMapDescriptor* GetDescriptor(const nsUuid& guid) const;

  bool FindObjectByPath(nsStringView sPath, nsUuid& out_guid) const;
  bool FindObjectPathByName(const nsTreeNode<nsActionMapDescriptor>* pObject, nsStringView sName, nsStringBuilder& out_sPath) const;
  const nsTreeNode<nsActionMapDescriptor>* GetChildByName(const nsTreeNode<nsActionMapDescriptor>* pObject, nsStringView sName) const;

  TreeNode m_Root;
  nsMap<nsUuid, nsTreeNode<nsActionMapDescriptor>*> m_Descriptors;
};
