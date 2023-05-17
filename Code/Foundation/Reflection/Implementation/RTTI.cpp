#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Implementation/AbstractProperty.h>
#include <Foundation/Reflection/Implementation/MessageHandler.h>

#include <Foundation/Communication/Message.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/HashTable.h>

struct wdTypeHashTable
{
  wdMutex m_Mutex;
  wdHashTable<const char*, wdRTTI*, wdHashHelper<const char*>, wdStaticAllocatorWrapper> m_Table;
};

wdTypeHashTable* GetTypeHashTable()
{
  // Prevent static initialization hazard between first wdRTTI instance
  // and the hash table and also make sure it is sufficiently sized before first use.
  auto CreateTable = []() -> wdTypeHashTable* {
    wdTypeHashTable* table = new wdTypeHashTable();
    table->m_Table.Reserve(512);
    return table;
  };
  static wdTypeHashTable* table = CreateTable();
  return table;
}

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdRTTI);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, Reflection)

  //BEGIN_SUBSYSTEM_DEPENDENCIES
  //  "FileSystem"
  //END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdPlugin::Events().AddEventHandler(wdRTTI::PluginEventHandler);
    wdRTTI::AssignPlugin("Static");
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdPlugin::Events().RemoveEventHandler(wdRTTI::PluginEventHandler);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdRTTI::wdRTTI(const char* szName, const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType,
  wdBitflags<wdTypeFlags> flags, wdRTTIAllocator* pAllocator, wdArrayPtr<wdAbstractProperty*> properties, wdArrayPtr<wdAbstractProperty*> functions,
  wdArrayPtr<wdPropertyAttribute*> attributes, wdArrayPtr<wdAbstractMessageHandler*> messageHandlers, wdArrayPtr<wdMessageSenderInfo> messageSenders,
  const wdRTTI* (*fnVerifyParent)())
{
  UpdateType(pParentType, uiTypeSize, uiTypeVersion, uiVariantType, flags);

  m_bGatheredDynamicMessageHandlers = false;
  m_szPluginName = nullptr;
  m_szTypeName = szName;
  m_pAllocator = pAllocator;
  m_Properties = properties;
  m_Functions = wdMakeArrayPtr<wdAbstractFunctionProperty*>(reinterpret_cast<wdAbstractFunctionProperty**>(functions.GetPtr()), functions.GetCount());
  m_Attributes = attributes;
  m_MessageHandlers = messageHandlers;
  m_uiMsgIdOffset = 0;
  m_MessageSenders = messageSenders;

  m_VerifyParent = fnVerifyParent;

  // This part is not guaranteed to always work here!
  // pParentType is (apparently) always the correct pointer to the base class BUT it is not guaranteed to have been constructed at this
  // point in time! Therefore the message handler hierarchy is initialized delayed in DispatchMessage
  //
  // However, I don't know where we could do these debug checks where they are guaranteed to be executed.
  // For now they are executed here and one might also do that in e.g. the game application
  {
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
    VerifyCorrectness();
#endif
  }

  if (m_szTypeName)
    RegisterType();
}

wdRTTI::~wdRTTI()
{
  if (m_szTypeName)
    UnregisterType();
}

void wdRTTI::GatherDynamicMessageHandlers()
{
  // This cannot be done in the constructor, because the parent types are not guaranteed to be initialized at that point

  if (m_bGatheredDynamicMessageHandlers)
    return;

  m_bGatheredDynamicMessageHandlers = true;

  wdUInt32 uiMinMsgId = wdInvalidIndex;
  wdUInt32 uiMaxMsgId = 0;

  const wdRTTI* pInstance = this;
  while (pInstance != nullptr)
  {
    for (wdUInt32 i = 0; i < pInstance->m_MessageHandlers.GetCount(); ++i)
    {
      wdUInt32 id = pInstance->m_MessageHandlers[i]->GetMessageId();
      uiMinMsgId = wdMath::Min(uiMinMsgId, id);
      uiMaxMsgId = wdMath::Max(uiMaxMsgId, id);
    }

    pInstance = pInstance->m_pParentType;
  }

  if (uiMinMsgId != wdInvalidIndex)
  {
    m_uiMsgIdOffset = uiMinMsgId;
    wdUInt32 uiNeededCapacity = uiMaxMsgId - uiMinMsgId + 1;

    m_DynamicMessageHandlers.SetCount(uiNeededCapacity);

    pInstance = this;
    while (pInstance != nullptr)
    {
      for (wdUInt32 i = 0; i < pInstance->m_MessageHandlers.GetCount(); ++i)
      {
        wdAbstractMessageHandler* pHandler = pInstance->m_MessageHandlers[i];
        wdUInt32 uiIndex = pHandler->GetMessageId() - m_uiMsgIdOffset;

        // this check ensures that handlers in base classes do not override the derived handlers
        if (m_DynamicMessageHandlers[uiIndex] == nullptr)
        {
          m_DynamicMessageHandlers[uiIndex] = pHandler;
        }
      }

      pInstance = pInstance->m_pParentType;
    }
  }
}

void wdRTTI::SetupParentHierarchy()
{
  m_ParentHierarchy.Clear();

  for (const wdRTTI* rtti = this; rtti != nullptr; rtti = rtti->m_pParentType)
  {
    m_ParentHierarchy.PushBack(rtti);
  }
}

void wdRTTI::VerifyCorrectness() const
{
  if (m_VerifyParent != nullptr)
  {
    WD_ASSERT_DEV(m_VerifyParent() == m_pParentType, "Type '{0}': The given parent type '{1}' does not match the actual parent type '{2}'",
      m_szTypeName, (m_pParentType != nullptr) ? m_pParentType->GetTypeName() : "null",
      (m_VerifyParent() != nullptr) ? m_VerifyParent()->GetTypeName() : "null");
  }

  {
    wdSet<wdStringView> Known;

    const wdRTTI* pInstance = this;

    while (pInstance != nullptr)
    {
      for (wdUInt32 i = 0; i < pInstance->m_Properties.GetCount(); ++i)
      {
        const bool bNewProperty = !Known.Find(pInstance->m_Properties[i]->GetPropertyName()).IsValid();
        Known.Insert(pInstance->m_Properties[i]->GetPropertyName());

        WD_ASSERT_DEV(bNewProperty, "{0}: The property with name '{1}' is already defined in type '{2}'.", m_szTypeName,
          pInstance->m_Properties[i]->GetPropertyName(), pInstance->GetTypeName());
      }

      pInstance = pInstance->m_pParentType;
    }
  }

  {
    for (wdAbstractProperty* pFunc : m_Functions)
    {
      WD_ASSERT_DEV(pFunc->GetCategory() == wdPropertyCategory::Function, "Invalid function property '{}'", pFunc->GetPropertyName());
    }
  }
}

void wdRTTI::VerifyCorrectnessForAllTypes()
{
  wdRTTI* pRtti = wdRTTI::GetFirstInstance();

  while (pRtti)
  {
    pRtti->VerifyCorrectness();
    pRtti = pRtti->GetNextInstance();
  }
}


void wdRTTI::UpdateType(const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType, wdBitflags<wdTypeFlags> flags)
{
  m_pParentType = pParentType;
  m_uiVariantType = uiVariantType;
  m_uiTypeSize = uiTypeSize;
  m_uiTypeVersion = uiTypeVersion;
  m_TypeFlags = flags;
  m_ParentHierarchy.Clear();
}

void wdRTTI::RegisterType()
{
  m_uiTypeNameHash = wdHashingUtils::StringHash(m_szTypeName);

  auto pTable = GetTypeHashTable();
  WD_LOCK(pTable->m_Mutex);
  pTable->m_Table.Insert(m_szTypeName, this);
}

void wdRTTI::UnregisterType()
{
  auto pTable = GetTypeHashTable();
  WD_LOCK(pTable->m_Mutex);
  pTable->m_Table.Remove(m_szTypeName);
}

void wdRTTI::GetAllProperties(wdHybridArray<wdAbstractProperty*, 32>& out_properties) const
{
  out_properties.Clear();

  if (m_pParentType)
    m_pParentType->GetAllProperties(out_properties);

  out_properties.PushBackRange(GetProperties());
}

wdRTTI* wdRTTI::FindTypeByName(const char* szName)
{
  wdRTTI* pInstance = nullptr;
  {
    auto pTable = GetTypeHashTable();
    WD_LOCK(pTable->m_Mutex);
    if (pTable->m_Table.TryGetValue(szName, pInstance))
      return pInstance;
  }

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  pInstance = wdRTTI::GetFirstInstance();

  while (pInstance)
  {
    if (wdStringUtils::IsEqual(pInstance->GetTypeName(), szName))
    {
      WD_REPORT_FAILURE("The hash table lookup should have already found the RTTI type '{}'", szName);
      return pInstance;
    }

    pInstance = pInstance->GetNextInstance();
  }
#endif

  return nullptr;
}

wdRTTI* wdRTTI::FindTypeByNameHash(wdUInt64 uiNameHash)
{
  // TODO: actually reuse the hash table for the lookup

  wdRTTI* pInstance = wdRTTI::GetFirstInstance();

  while (pInstance)
  {
    if (pInstance->GetTypeNameHash() == uiNameHash)
      return pInstance;

    pInstance = pInstance->GetNextInstance();
  }

  return nullptr;
}

wdRTTI* wdRTTI::FindTypeByNameHash32(wdUInt32 uiNameHash)
{
  // TODO: actually reuse the hash table for the lookup

  wdRTTI* pInstance = wdRTTI::GetFirstInstance();

  while (pInstance)
  {
    if (wdHashingUtils::StringHashTo32(pInstance->GetTypeNameHash()) == uiNameHash)
      return pInstance;

    pInstance = pInstance->GetNextInstance();
  }

  return nullptr;
}

wdAbstractProperty* wdRTTI::FindPropertyByName(const char* szName, bool bSearchBaseTypes /* = true */) const
{
  const wdRTTI* pInstance = this;

  do
  {
    for (wdUInt32 p = 0; p < pInstance->m_Properties.GetCount(); ++p)
    {
      if (wdStringUtils::IsEqual(pInstance->m_Properties[p]->GetPropertyName(), szName))
      {
        return pInstance->m_Properties[p];
      }
    }

    if (!bSearchBaseTypes)
      return nullptr;

    pInstance = pInstance->m_pParentType;
  } while (pInstance != nullptr);

  return nullptr;
}

bool wdRTTI::DispatchMessage(void* pInstance, wdMessage& ref_msg) const
{
  WD_ASSERT_DEBUG(m_bGatheredDynamicMessageHandlers, "Message handler table should have been gathered at this point.\n"
                                                     "If this assert is triggered for a type loaded from a dynamic plugin,\n"
                                                     "you may have forgotten to instantiate an wdPlugin object inside your plugin DLL.");

  const wdUInt32 uiIndex = ref_msg.GetId() - m_uiMsgIdOffset;

  // m_DynamicMessageHandlers contains all message handlers of this type and all base types
  if (uiIndex < m_DynamicMessageHandlers.GetCount())
  {
    wdAbstractMessageHandler* pHandler = m_DynamicMessageHandlers[uiIndex];
    if (pHandler != nullptr)
    {
      (*pHandler)(pInstance, ref_msg);
      return true;
    }
  }

  return false;
}

bool wdRTTI::DispatchMessage(const void* pInstance, wdMessage& ref_msg) const
{
  WD_ASSERT_DEBUG(m_bGatheredDynamicMessageHandlers, "Message handler table should have been gathered at this point.\n"
                                                     "If this assert is triggered for a type loaded from a dynamic plugin,\n"
                                                     "you may have forgotten to instantiate an wdPlugin object inside your plugin DLL.");

  const wdUInt32 uiIndex = ref_msg.GetId() - m_uiMsgIdOffset;

  // m_DynamicMessageHandlers contains all message handlers of this type and all base types
  if (uiIndex < m_DynamicMessageHandlers.GetCount())
  {
    wdAbstractMessageHandler* pHandler = m_DynamicMessageHandlers[uiIndex];
    if (pHandler != nullptr && pHandler->IsConst())
    {
      (*pHandler)(pInstance, ref_msg);
      return true;
    }
  }

  return false;
}

const wdDynamicArray<const wdRTTI*>& wdRTTI::GetAllTypesDerivedFrom(
  const wdRTTI* pBaseType, wdDynamicArray<const wdRTTI*>& out_derivedTypes, bool bSortByName)
{
  for (auto pRtti = wdRTTI::GetFirstInstance(); pRtti != nullptr; pRtti = pRtti->GetNextInstance())
  {
    if (!pRtti->IsDerivedFrom(pBaseType))
      continue;

    out_derivedTypes.PushBack(pRtti);
  }

  if (bSortByName)
  {
    out_derivedTypes.Sort(
      [](const wdRTTI* p1, const wdRTTI* p2) -> bool { return wdStringUtils::Compare(p1->GetTypeName(), p2->GetTypeName()) < 0; });
  }

  return out_derivedTypes;
}

void wdRTTI::AssignPlugin(const char* szPluginName)
{
  // assigns the given plugin name to every wdRTTI instance that has no plugin assigned yet

  wdRTTI* pInstance = wdRTTI::GetFirstInstance();

  while (pInstance)
  {
    if (pInstance->m_szPluginName == nullptr)
    {
      pInstance->m_szPluginName = szPluginName;
      SanityCheckType(pInstance);

      pInstance->SetupParentHierarchy();
      pInstance->GatherDynamicMessageHandlers();
    }
    pInstance = pInstance->GetNextInstance();
  }
}

// warning C4505: 'IsValidIdentifierName': unreferenced function with internal linkage has been removed
// happens in Release builds, because the function is only used in a debug assert
#define WD_MSVC_WARNING_NUMBER 4505
#include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>

static bool IsValidIdentifierName(const char* szIdentifier)
{
  // empty strings are not valid
  if (wdStringUtils::IsNullOrEmpty(szIdentifier))
    return false;

  // digits are not allowed as the first character
  if (szIdentifier[0] >= '0' && szIdentifier[0] <= '9')
    return false;

  for (const char* s = szIdentifier; *s != '\0'; ++s)
  {
    const char c = *s;

    if (c >= 'a' && c <= 'z')
      continue;
    if (c >= 'A' && c <= 'Z')
      continue;
    if (c >= '0' && c <= '9')
      continue;
    if (c >= '_')
      continue;
    if (c >= ':')
      continue;

    return false;
  }

  return true;
}

#include <Foundation/Basics/Compiler/MSVC/RestoreWarning_MSVC.h>

void wdRTTI::SanityCheckType(wdRTTI* pType)
{
  WD_ASSERT_DEV(pType->GetTypeFlags().IsSet(wdTypeFlags::StandardType) + pType->GetTypeFlags().IsSet(wdTypeFlags::IsEnum) +
                    pType->GetTypeFlags().IsSet(wdTypeFlags::Bitflags) + pType->GetTypeFlags().IsSet(wdTypeFlags::Class) ==
                  1,
    "Types are mutually exclusive!");

  for (auto pProp : pType->m_Properties)
  {
    const wdRTTI* pSpecificType = pProp->GetSpecificType();

    WD_ASSERT_DEBUG(IsValidIdentifierName(pProp->GetPropertyName()), "Property name is invalid: '{0}'", pProp->GetPropertyName());

    if (pProp->GetCategory() != wdPropertyCategory::Function)
    {
      WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::StandardType) + pProp->GetFlags().IsSet(wdPropertyFlags::IsEnum) +
                        pProp->GetFlags().IsSet(wdPropertyFlags::Bitflags) + pProp->GetFlags().IsSet(wdPropertyFlags::Class) <=
                      1,
        "Types are mutually exclusive!");
    }

    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Constant:
      {
        WD_ASSERT_DEV(pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::StandardType), "Only standard type constants are supported!");
      }
      break;
      case wdPropertyCategory::Member:
      {
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::StandardType) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::StandardType),
          "Property-Type missmatch!");
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::IsEnum) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::IsEnum),
          "Property-Type missmatch! Use WD_BEGIN_STATIC_REFLECTED_ENUM for type and WD_ENUM_MEMBER_PROPERTY / "
          "WD_ENUM_ACCESSOR_PROPERTY for property.");
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::Bitflags) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::Bitflags),
          "Property-Type missmatch! Use WD_BEGIN_STATIC_REFLECTED_ENUM for type and WD_BITFLAGS_MEMBER_PROPERTY / "
          "WD_BITFLAGS_ACCESSOR_PROPERTY for property.");
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::Class) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::Class),
          "If wdPropertyFlags::Class is set, the property type must be wdTypeFlags::Class and vise versa.");
      }
      break;
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      case wdPropertyCategory::Map:
      {
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::StandardType) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::StandardType),
          "Property-Type missmatch!");
        WD_ASSERT_DEV(pProp->GetFlags().IsSet(wdPropertyFlags::Class) == pSpecificType->GetTypeFlags().IsSet(wdTypeFlags::Class),
          "If wdPropertyFlags::Class is set, the property type must be wdTypeFlags::Class and vise versa.");
      }
      break;
      case wdPropertyCategory::Function:
        WD_REPORT_FAILURE("Functions need to be put into the WD_BEGIN_FUNCTIONS / WD_END_FUNCTIONS; block.");
        break;
    }
  }
}

void wdRTTI::PluginEventHandler(const wdPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case wdPluginEvent::BeforeLoading:
    {
      // before a new plugin is loaded, make sure all current wdRTTI instances
      // are assigned to the proper plugin
      // all not-yet assigned rtti instances cannot be in any plugin, so assign them to the 'static' plugin
      AssignPlugin("Static");
    }
    break;

    case wdPluginEvent::AfterLoadingBeforeInit:
    {
      // after we loaded a new plugin, but before it is initialized,
      // find all new rtti instances and assign them to that new plugin
      AssignPlugin(EventData.m_szPluginBinary);

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
      wdRTTI::VerifyCorrectnessForAllTypes();
#endif
    }
    break;

    default:
      break;
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Reflection_Implementation_RTTI);
