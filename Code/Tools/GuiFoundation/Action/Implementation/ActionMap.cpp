#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMap.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdActionMapDescriptor, wdNoBase, 0, wdRTTINoAllocator);
//  WD_BEGIN_PROPERTIES
//  WD_END_PROPERTIES;
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdActionMap public functions
////////////////////////////////////////////////////////////////////////

wdActionMap::wdActionMap()
{
  // wdReflectedTypeDescriptor desc;
  // wdToolsReflectionUtils::GetReflectedTypeDescriptorFromRtti(wdGetStaticRTTI<wdActionMapDescriptor>(), desc);
  // m_pRtti = wdPhantomRttiManager::RegisterType(desc);
}

wdActionMap::~wdActionMap()
{
  // DestroyAllObjects();
}

void wdActionMap::MapAction(wdActionDescriptorHandle hAction, const char* szPath, float fOrder)
{
  wdStringBuilder sPath = szPath;
  sPath.MakeCleanPath();
  sPath.Trim("/");
  wdActionMapDescriptor d;
  d.m_hAction = hAction;
  d.m_sPath = sPath;
  d.m_fOrder = fOrder;

  WD_VERIFY(MapAction(d).IsValid(), "Mapping Failed");
}

wdUuid wdActionMap::MapAction(const wdActionMapDescriptor& desc)
{
  wdUuid ParentGUID;
  if (!FindObjectByPath(desc.m_sPath, ParentGUID))
  {
    return wdUuid();
  }

  auto it = m_Descriptors.Find(ParentGUID);

  wdTreeNode<wdActionMapDescriptor>* pParent = nullptr;
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
    const wdActionMapDescriptor* pDesc = GetDescriptor(pParent);
    if (pDesc->m_hAction.GetDescriptor()->m_Type == wdActionType::Action)
    {
      wdLog::Error("Can't map descriptor '{0}' as its parent is an action itself and thus can't have any children.",
        desc.m_hAction.GetDescriptor()->m_sActionName);
      return wdUuid();
    }
  }

  if (GetChildByName(pParent, desc.m_hAction.GetDescriptor()->m_sActionName) != nullptr)
  {
    wdLog::Error("Can't map descriptor as its name is already present: {0}", desc.m_hAction.GetDescriptor()->m_sActionName);
    return wdUuid();
  }

  wdInt32 iIndex = 0;
  for (iIndex = 0; iIndex < (wdInt32)pParent->GetChildren().GetCount(); ++iIndex)
  {
    const wdTreeNode<wdActionMapDescriptor>* pChild = pParent->GetChildren()[iIndex];
    const wdActionMapDescriptor* pDesc = GetDescriptor(pChild);

    if (desc.m_fOrder < pDesc->m_fOrder)
      break;
  }

  wdTreeNode<wdActionMapDescriptor>* pChild = pParent->InsertChild(desc, iIndex);

  m_Descriptors.Insert(pChild->GetGuid(), pChild);

  return pChild->GetGuid();
}

wdResult wdActionMap::UnmapAction(const wdUuid& guid)
{
  auto it = m_Descriptors.Find(guid);
  if (!it.IsValid())
    return WD_FAILURE;

  wdTreeNode<wdActionMapDescriptor>* pNode = it.Value();
  if (wdTreeNode<wdActionMapDescriptor>* pParent = pNode->GetParent())
  {
    pParent->RemoveChild(pNode->GetParentIndex());
  }
  m_Descriptors.Remove(it);
  return WD_SUCCESS;
}

wdResult wdActionMap::UnmapAction(wdActionDescriptorHandle hAction, const char* szPath)
{
  wdStringBuilder sPath = szPath;
  sPath.MakeCleanPath();
  sPath.Trim("/");
  wdActionMapDescriptor d;
  d.m_hAction = hAction;
  d.m_sPath = sPath;
  d.m_fOrder = 0.0f; // unused.
  return UnmapAction(d);
}

wdResult wdActionMap::UnmapAction(const wdActionMapDescriptor& desc)
{
  wdTreeNode<wdActionMapDescriptor>* pParent = nullptr;
  if (desc.m_sPath.IsEmpty())
  {
    pParent = &m_Root;
  }
  else
  {
    wdUuid ParentGUID;
    if (!FindObjectByPath(desc.m_sPath, ParentGUID))
      return WD_FAILURE;

    auto it = m_Descriptors.Find(ParentGUID);
    if (!it.IsValid())
      return WD_FAILURE;

    pParent = it.Value();
  }

  if (auto* pChild = GetChildByName(pParent, desc.m_hAction.GetDescriptor()->m_sActionName))
  {
    return UnmapAction(pChild->GetGuid());
  }
  return WD_FAILURE;
}

bool wdActionMap::FindObjectByPath(const wdStringView& sPath, wdUuid& out_guid) const
{
  out_guid = wdUuid();
  if (sPath.IsEmpty())
    return true;

  wdStringBuilder sPathBuilder(sPath);
  wdHybridArray<wdStringView, 8> parts;
  sPathBuilder.Split(false, parts, "/");

  const wdTreeNode<wdActionMapDescriptor>* pParent = &m_Root;
  for (const wdStringView& name : parts)
  {
    pParent = GetChildByName(pParent, name);
    if (pParent == nullptr)
      return false;
  }

  out_guid = pParent->GetGuid();
  return true;
}

const wdActionMapDescriptor* wdActionMap::GetDescriptor(const wdUuid& guid) const
{
  auto it = m_Descriptors.Find(guid);
  if (!it.IsValid())
    return nullptr;
  return GetDescriptor(it.Value());
}

const wdActionMapDescriptor* wdActionMap::GetDescriptor(const wdTreeNode<wdActionMapDescriptor>* pObject) const
{
  if (pObject == nullptr)
    return nullptr;

  return &pObject->m_Data;
}

const wdTreeNode<wdActionMapDescriptor>* wdActionMap::GetChildByName(
  const wdTreeNode<wdActionMapDescriptor>* pObject, const wdStringView& sName) const
{
  for (const wdTreeNode<wdActionMapDescriptor>* pChild : pObject->GetChildren())
  {
    const wdActionMapDescriptor& pDesc = pChild->m_Data;
    if (sName.IsEqual_NoCase(pDesc.m_hAction.GetDescriptor()->m_sActionName.GetData()))
    {
      return pChild;
    }
  }
  return nullptr;
}
