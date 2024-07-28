#include <Core/CorePCH.h>

#include <Core/Scripting/ScriptAttributes.h>
#include <Core/Scripting/ScriptClassResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsScriptClassResource, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsScriptClassResource);
// clang-format on

nsScriptClassResource::nsScriptClassResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsScriptClassResource::~nsScriptClassResource() = default;

nsSharedPtr<nsScriptRTTI> nsScriptClassResource::CreateScriptType(nsStringView sName, const nsRTTI* pBaseType, nsScriptRTTI::FunctionList&& functions, nsScriptRTTI::MessageHandlerList&& messageHandlers)
{
  nsScriptRTTI::FunctionList sortedFunctions;
  for (auto pFuncProp : pBaseType->GetFunctions())
  {
    auto pBaseClassFuncAttr = pFuncProp->GetAttributeByType<nsScriptBaseClassFunctionAttribute>();
    if (pBaseClassFuncAttr == nullptr)
      continue;

    nsStringView sBaseClassFuncName = pFuncProp->GetPropertyName();
    sBaseClassFuncName.TrimWordStart("Reflection_");

    nsUInt16 uiIndex = pBaseClassFuncAttr->GetIndex();
    sortedFunctions.EnsureCount(uiIndex + 1);

    for (nsUInt32 i = 0; i < functions.GetCount(); ++i)
    {
      auto& pScriptFuncProp = functions[i];
      if (pScriptFuncProp == nullptr)
        continue;

      if (sBaseClassFuncName == pScriptFuncProp->GetPropertyName())
      {
        sortedFunctions[uiIndex] = std::move(pScriptFuncProp);
        functions.RemoveAtAndSwap(i);
        break;
      }
    }
  }

  m_pType = NS_SCRIPT_NEW(nsScriptRTTI, sName, pBaseType, std::move(sortedFunctions), std::move(messageHandlers));
  return m_pType;
}

void nsScriptClassResource::DeleteScriptType()
{
  m_pType = nullptr;
}

nsSharedPtr<nsScriptCoroutineRTTI> nsScriptClassResource::CreateScriptCoroutineType(nsStringView sScriptClassName, nsStringView sFunctionName, nsUniquePtr<nsRTTIAllocator>&& pAllocator)
{
  nsStringBuilder sCoroutineTypeName;
  sCoroutineTypeName.Set(sScriptClassName, "::", sFunctionName, "<Coroutine>");

  nsSharedPtr<nsScriptCoroutineRTTI> pCoroutineType = NS_SCRIPT_NEW(nsScriptCoroutineRTTI, sCoroutineTypeName, std::move(pAllocator));
  m_CoroutineTypes.PushBack(pCoroutineType);

  return pCoroutineType;
}

void nsScriptClassResource::DeleteAllScriptCoroutineTypes()
{
  m_CoroutineTypes.Clear();
}


NS_STATICLINK_FILE(Core, Core_Scripting_Implementation_ScriptClassResource);
