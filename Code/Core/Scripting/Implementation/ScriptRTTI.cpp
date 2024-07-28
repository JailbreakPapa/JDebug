#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptRTTI.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Reflection/ReflectionUtils.h>

nsScriptRTTI::nsScriptRTTI(nsStringView sName, const nsRTTI* pParentType, FunctionList&& functions, MessageHandlerList&& messageHandlers)
  : nsRTTI(nullptr, pParentType, 0, 1, nsVariantType::Invalid, nsTypeFlags::Class, nullptr, nsArrayPtr<const nsAbstractProperty*>(), nsArrayPtr<const nsAbstractFunctionProperty*>(), nsArrayPtr<const nsPropertyAttribute*>(), nsArrayPtr<nsAbstractMessageHandler*>(), nsArrayPtr<nsMessageSenderInfo>(), nullptr)
  , m_sTypeNameStorage(sName)
  , m_FunctionStorage(std::move(functions))
  , m_MessageHandlerStorage(std::move(messageHandlers))
{
  m_sTypeName = m_sTypeNameStorage.GetData();

  for (auto& pFunction : m_FunctionStorage)
  {
    if (pFunction != nullptr)
    {
      m_FunctionRawPtrs.PushBack(pFunction.Borrow());
    }
  }

  for (auto& pMessageHandler : m_MessageHandlerStorage)
  {
    if (pMessageHandler != nullptr)
    {
      m_MessageHandlerRawPtrs.PushBack(pMessageHandler.Borrow());
    }
  }

  m_Functions = m_FunctionRawPtrs;
  m_MessageHandlers = m_MessageHandlerRawPtrs;

  RegisterType();

  SetupParentHierarchy();
  GatherDynamicMessageHandlers();
}

nsScriptRTTI::~nsScriptRTTI()
{
  UnregisterType();
  m_sTypeName = nullptr;
}

const nsAbstractFunctionProperty* nsScriptRTTI::GetFunctionByIndex(nsUInt32 uiIndex) const
{
  if (uiIndex < m_FunctionStorage.GetCount())
  {
    return m_FunctionStorage.GetData()[uiIndex].Borrow();
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////

nsScriptFunctionProperty::nsScriptFunctionProperty(nsStringView sName)
  : nsAbstractFunctionProperty(nullptr)
{
  m_sPropertyNameStorage.Assign(sName);
  m_szPropertyName = m_sPropertyNameStorage.GetData();
}

nsScriptFunctionProperty::~nsScriptFunctionProperty() = default;

//////////////////////////////////////////////////////////////////////////

nsScriptMessageHandler::nsScriptMessageHandler(const nsScriptMessageDesc& desc)
  : m_Properties(desc.m_Properties)
{
  nsUniquePtr<nsMessage> pMessage = desc.m_pType->GetAllocator()->Allocate<nsMessage>();

  m_Id = pMessage->GetId();
  m_bIsConst = true;
}

nsScriptMessageHandler::~nsScriptMessageHandler() = default;

void nsScriptMessageHandler::FillMessagePropertyValues(const nsMessage& msg, nsDynamicArray<nsVariant>& out_propertyValues)
{
  out_propertyValues.Clear();

  for (auto pProp : m_Properties)
  {
    if (pProp->GetCategory() == nsPropertyCategory::Member)
    {
      out_propertyValues.PushBack(nsReflectionUtils::GetMemberPropertyValue(static_cast<const nsAbstractMemberProperty*>(pProp), &msg));
    }
    else
    {
      NS_ASSERT_NOT_IMPLEMENTED;
    }
  }
}

//////////////////////////////////////////////////////////////////////////

nsScriptInstance::nsScriptInstance(nsReflectedClass& inout_owner, nsWorld* pWorld)
  : m_Owner(inout_owner)
  , m_pWorld(pWorld)
{
}

void nsScriptInstance::SetInstanceVariables(const nsArrayMap<nsHashedString, nsVariant>& parameters)
{
  for (auto it : parameters)
  {
    SetInstanceVariable(it.key, it.value);
  }
}

//////////////////////////////////////////////////////////////////////////

// static
nsAllocator* nsScriptAllocator::GetAllocator()
{
  static nsProxyAllocator s_ScriptAllocator("Script", nsFoundation::GetDefaultAllocator());
  return &s_ScriptAllocator;
}
