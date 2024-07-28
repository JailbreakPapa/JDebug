#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Core/Scripting/ScriptCoroutine.h>
#include <Core/Scripting/ScriptRTTI.h>

class nsWorld;
using nsScriptClassResourceHandle = nsTypedResourceHandle<class nsScriptClassResource>;

class NS_CORE_DLL nsScriptClassResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsScriptClassResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsScriptClassResource);

public:
  nsScriptClassResource();
  ~nsScriptClassResource();

  const nsSharedPtr<nsScriptRTTI>& GetType() const { return m_pType; }

  virtual nsUniquePtr<nsScriptInstance> Instantiate(nsReflectedClass& inout_owner, nsWorld* pWorld) const = 0;

protected:
  nsSharedPtr<nsScriptRTTI> CreateScriptType(nsStringView sName, const nsRTTI* pBaseType, nsScriptRTTI::FunctionList&& functions, nsScriptRTTI::MessageHandlerList&& messageHandlers);
  void DeleteScriptType();

  nsSharedPtr<nsScriptCoroutineRTTI> CreateScriptCoroutineType(nsStringView sScriptClassName, nsStringView sFunctionName, nsUniquePtr<nsRTTIAllocator>&& pAllocator);
  void DeleteAllScriptCoroutineTypes();

  nsSharedPtr<nsScriptRTTI> m_pType;
  nsDynamicArray<nsSharedPtr<nsScriptCoroutineRTTI>> m_CoroutineTypes;
};
