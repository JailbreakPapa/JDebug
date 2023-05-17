#pragma once

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class wdDocument;

struct WD_GUIFOUNDATION_DLL wdActionMapDescriptor
{
  wdActionDescriptorHandle m_hAction; ///< Action to be mapped
  wdString
    m_sPath; ///< Path where the action should be mapped excluding the action's name, e.g. "File/New" for a menu item "File -> New -> Project..." .
  float m_fOrder; ///< Ordering key to sort actions in the mapping path.
};
WD_DECLARE_REFLECTABLE_TYPE(WD_NO_LINKAGE, wdActionMapDescriptor);

template <typename T>
class wdTreeNode
{
public:
  wdTreeNode()
    : m_pParent(nullptr)
  {
  }
  wdTreeNode(const T& data)
    : m_Data(data)
    , m_pParent(nullptr)
  {
  }
  ~wdTreeNode()
  {
    while (!m_Children.IsEmpty())
    {
      RemoveChild(0);
    }
  }

  const wdUuid& GetGuid() const { return m_Guid; }
  const wdTreeNode<T>* GetParent() const { return m_pParent; }
  wdTreeNode<T>* GetParent() { return m_pParent; }
  const wdHybridArray<wdTreeNode<T>*, 8>& GetChildren() const { return m_Children; }
  wdHybridArray<wdTreeNode<T>*, 8>& GetChildren() { return m_Children; }

  wdTreeNode<T>* InsertChild(const T& data, wdUInt32 uiIndex)
  {
    wdTreeNode<T>* pNode = WD_DEFAULT_NEW(wdTreeNode<T>, data);
    pNode->m_Guid.CreateNewUuid();
    m_Children.Insert(pNode, uiIndex);
    pNode->m_pParent = this;
    return pNode;
  }

  bool RemoveChild(wdUInt32 uiIndex)
  {
    if (uiIndex > m_Children.GetCount())
      return false;

    wdTreeNode<T>* pChild = m_Children[uiIndex];
    m_Children.RemoveAtAndCopy(uiIndex);
    WD_DEFAULT_DELETE(pChild);
    return true;
  }

  wdUInt32 GetParentIndex() const
  {
    WD_ASSERT_DEV(m_pParent != nullptr, "Can't compute parent index if no parent is present!");
    for (wdUInt32 i = 0; i < m_pParent->GetChildren().GetCount(); i++)
    {
      if (m_pParent->GetChildren()[i] == this)
        return i;
    }
    WD_REPORT_FAILURE("Couldn't find oneself in own parent!");
    return -1;
  }

  T m_Data;
  wdUuid m_Guid;

private:
  wdTreeNode<T>* m_pParent;
  wdHybridArray<wdTreeNode<T>*, 8> m_Children;
};


class WD_GUIFOUNDATION_DLL wdActionMap
{
public:
  typedef wdTreeNode<wdActionMapDescriptor> TreeNode;
  wdActionMap();
  ~wdActionMap();

  void MapAction(wdActionDescriptorHandle hAction, const char* szPath, float fM_fOrder);
  wdUuid MapAction(const wdActionMapDescriptor& desc);
  wdResult UnmapAction(wdActionDescriptorHandle hAction, const char* szPath);
  wdResult UnmapAction(const wdActionMapDescriptor& desc);
  wdResult UnmapAction(const wdUuid& guid);

  const TreeNode* GetRootObject() const { return &m_Root; }
  const wdActionMapDescriptor* GetDescriptor(const wdUuid& guid) const;
  const wdActionMapDescriptor* GetDescriptor(const wdTreeNode<wdActionMapDescriptor>* pObject) const;

private:
  bool FindObjectByPath(const wdStringView& sPath, wdUuid& out_guid) const;
  const wdTreeNode<wdActionMapDescriptor>* GetChildByName(const wdTreeNode<wdActionMapDescriptor>* pObject, const wdStringView& sName) const;

private:
  TreeNode m_Root;
  wdMap<wdUuid, wdTreeNode<wdActionMapDescriptor>*> m_Descriptors;
};
