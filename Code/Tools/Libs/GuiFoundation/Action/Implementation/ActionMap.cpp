#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMap.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsActionMapDescriptor, nsNoBase, 0, nsRTTINoAllocator);
//  NS_BEGIN_PROPERTIES
//  NS_END_PROPERTIES;
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsActionMap public functions
////////////////////////////////////////////////////////////////////////

nsActionMap::nsActionMap() = default;

nsActionMap::~nsActionMap() = default;

void nsActionMap::MapAction(nsActionDescriptorHandle hAction, nsStringView sPath, nsStringView sSubPath, float fOrder)
{
  nsStringBuilder sFullPath = sPath;

  if (!sPath.IsEmpty() && sPath.FindSubString("/") == nullptr)
  {
    if (SearchPathForAction(sPath, sFullPath).Failed())
    {
      sFullPath = sPath;
    }
  }

  sFullPath.AppendPath(sSubPath);

  MapAction(hAction, sFullPath, fOrder);
}

void nsActionMap::MapAction(nsActionDescriptorHandle hAction, nsStringView sPath, float fOrder)
{
  nsStringBuilder sCleanPath = sPath;
  sCleanPath.MakeCleanPath();
  sCleanPath.Trim("/");
  nsActionMapDescriptor d;
  d.m_hAction = hAction;
  d.m_sPath = sCleanPath;
  d.m_fOrder = fOrder;

  if (!d.m_sPath.IsEmpty() && d.m_sPath.FindSubString("/") == nullptr)
  {
    nsStringBuilder sFullPath;
    if (SearchPathForAction(d.m_sPath, sFullPath).Succeeded())
    {
      d.m_sPath = sFullPath;
    }
  }

  NS_VERIFY(MapAction(d).IsValid(), "Mapping Failed");
}

nsUuid nsActionMap::MapAction(const nsActionMapDescriptor& desc)
{
  nsUuid ParentGUID;
  if (!FindObjectByPath(desc.m_sPath, ParentGUID))
  {
    return nsUuid();
  }

  auto it = m_Descriptors.Find(ParentGUID);

  nsTreeNode<nsActionMapDescriptor>* pParent = nullptr;
  if (it.IsValid())
  {
    pParent = it.Value();
  }

  if (desc.m_sPath.IsEmpty())
  {
    pParent = &m_Root;
  }
  else
  {
    const nsActionMapDescriptor* pDesc = GetDescriptor(pParent);
    if (pDesc->m_hAction.GetDescriptor()->m_Type == nsActionType::Action)
    {
      nsLog::Error("Can't map descriptor '{0}' as its parent is an action itself and thus can't have any children.",
        desc.m_hAction.GetDescriptor()->m_sActionName);
      return nsUuid();
    }
  }

  if (GetChildByName(pParent, desc.m_hAction.GetDescriptor()->m_sActionName) != nullptr)
  {
    nsLog::Error("Can't map descriptor as its name is already present: {0}", desc.m_hAction.GetDescriptor()->m_sActionName);
    return nsUuid();
  }

  nsInt32 iIndex = 0;
  for (iIndex = 0; iIndex < (nsInt32)pParent->GetChildren().GetCount(); ++iIndex)
  {
    const nsTreeNode<nsActionMapDescriptor>* pChild = pParent->GetChildren()[iIndex];
    const nsActionMapDescriptor* pDesc = GetDescriptor(pChild);

    if (desc.m_fOrder < pDesc->m_fOrder)
      break;
  }

  nsTreeNode<nsActionMapDescriptor>* pChild = pParent->InsertChild(desc, iIndex);

  m_Descriptors.Insert(pChild->GetGuid(), pChild);

  return pChild->GetGuid();
}


nsResult nsActionMap::UnmapAction(const nsUuid& guid)
{
  auto it = m_Descriptors.Find(guid);
  if (!it.IsValid())
    return NS_FAILURE;

  nsTreeNode<nsActionMapDescriptor>* pNode = it.Value();
  if (nsTreeNode<nsActionMapDescriptor>* pParent = pNode->GetParent())
  {
    pParent->RemoveChild(pNode->GetParentIndex());
  }
  m_Descriptors.Remove(it);
  return NS_SUCCESS;
}

nsResult nsActionMap::UnmapAction(nsActionDescriptorHandle hAction, nsStringView sPath)
{
  nsStringBuilder sCleanPath = sPath;
  sCleanPath.MakeCleanPath();
  sCleanPath.Trim("/");
  nsActionMapDescriptor d;
  d.m_hAction = hAction;
  d.m_sPath = sCleanPath;
  d.m_fOrder = 0.0f; // unused.

  if (!d.m_sPath.IsEmpty() && d.m_sPath.FindSubString("/") == nullptr)
  {
    nsStringBuilder sFullPath;
    if (SearchPathForAction(d.m_sPath, sFullPath).Succeeded())
    {
      d.m_sPath = sFullPath;
    }
  }

  return UnmapAction(d);
}

nsResult nsActionMap::UnmapAction(const nsActionMapDescriptor& desc)
{
  nsTreeNode<nsActionMapDescriptor>* pParent = nullptr;
  if (desc.m_sPath.IsEmpty())
  {
    pParent = &m_Root;
  }
  else
  {
    nsUuid ParentGUID;
    if (!FindObjectByPath(desc.m_sPath, ParentGUID))
      return NS_FAILURE;

    auto it = m_Descriptors.Find(ParentGUID);
    if (!it.IsValid())
      return NS_FAILURE;

    pParent = it.Value();
  }

  if (auto* pChild = GetChildByName(pParent, desc.m_hAction.GetDescriptor()->m_sActionName))
  {
    return UnmapAction(pChild->GetGuid());
  }
  return NS_FAILURE;
}

bool nsActionMap::FindObjectByPath(nsStringView sPath, nsUuid& out_guid) const
{
  out_guid = nsUuid();
  if (sPath.IsEmpty())
    return true;

  nsStringBuilder sPathBuilder(sPath);
  nsHybridArray<nsStringView, 8> parts;
  sPathBuilder.Split(false, parts, "/");

  const nsTreeNode<nsActionMapDescriptor>* pParent = &m_Root;
  for (const nsStringView& name : parts)
  {
    pParent = GetChildByName(pParent, name);
    if (pParent == nullptr)
      return false;
  }

  out_guid = pParent->GetGuid();
  return true;
}

nsResult nsActionMap::SearchPathForAction(nsStringView sUniqueName, nsStringBuilder& out_sPath) const
{
  out_sPath.Clear();

  if (FindObjectPathByName(&m_Root, sUniqueName, out_sPath))
  {
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

bool nsActionMap::FindObjectPathByName(const nsTreeNode<nsActionMapDescriptor>* pObject, nsStringView sName, nsStringBuilder& out_sPath) const
{
  nsStringView sObjectName;

  if (!pObject->m_Data.m_hAction.IsInvalidated())
  {
    sObjectName = pObject->m_Data.m_hAction.GetDescriptor()->m_sActionName;
  }

  out_sPath.AppendPath(sObjectName);

  if (sObjectName == sName)
    return true;

  for (const nsTreeNode<nsActionMapDescriptor>* pChild : pObject->GetChildren())
  {
    const nsActionMapDescriptor& pDesc = pChild->m_Data;

    if (FindObjectPathByName(pChild, sName, out_sPath))
      return true;
  }

  out_sPath.PathParentDirectory();
  return false;
}

const nsActionMapDescriptor* nsActionMap::GetDescriptor(const nsUuid& guid) const
{
  auto it = m_Descriptors.Find(guid);
  if (!it.IsValid())
    return nullptr;
  return GetDescriptor(it.Value());
}

const nsActionMapDescriptor* nsActionMap::GetDescriptor(const nsTreeNode<nsActionMapDescriptor>* pObject) const
{
  if (pObject == nullptr)
    return nullptr;

  return &pObject->m_Data;
}

const nsTreeNode<nsActionMapDescriptor>* nsActionMap::GetChildByName(const nsTreeNode<nsActionMapDescriptor>* pObject, nsStringView sName) const
{
  for (const nsTreeNode<nsActionMapDescriptor>* pChild : pObject->GetChildren())
  {
    const nsActionMapDescriptor& pDesc = pChild->m_Data;
    if (sName.IsEqual_NoCase(pDesc.m_hAction.GetDescriptor()->m_sActionName.GetData()))
    {
      return pChild;
    }
  }
  return nullptr;
}
