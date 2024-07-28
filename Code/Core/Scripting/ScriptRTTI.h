#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/SharedPtr.h>

class nsWorld;

class NS_CORE_DLL nsScriptRTTI : public nsRTTI, public nsRefCountingImpl
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsScriptRTTI);

public:
  enum
  {
    NumInplaceFunctions = 7
  };

  using FunctionList = nsSmallArray<nsUniquePtr<nsAbstractFunctionProperty>, NumInplaceFunctions>;
  using MessageHandlerList = nsSmallArray<nsUniquePtr<nsAbstractMessageHandler>, NumInplaceFunctions>;

  nsScriptRTTI(nsStringView sName, const nsRTTI* pParentType, FunctionList&& functions, MessageHandlerList&& messageHandlers);
  ~nsScriptRTTI();

  const nsAbstractFunctionProperty* GetFunctionByIndex(nsUInt32 uiIndex) const;

private:
  nsString m_sTypeNameStorage;
  FunctionList m_FunctionStorage;
  MessageHandlerList m_MessageHandlerStorage;
  nsSmallArray<const nsAbstractFunctionProperty*, NumInplaceFunctions> m_FunctionRawPtrs;
  nsSmallArray<nsAbstractMessageHandler*, NumInplaceFunctions> m_MessageHandlerRawPtrs;
};

class NS_CORE_DLL nsScriptFunctionProperty : public nsAbstractFunctionProperty
{
public:
  nsScriptFunctionProperty(nsStringView sName);
  ~nsScriptFunctionProperty();

private:
  nsHashedString m_sPropertyNameStorage;
};

struct nsScriptMessageDesc
{
  const nsRTTI* m_pType = nullptr;
  nsArrayPtr<const nsAbstractProperty* const> m_Properties;
};

class NS_CORE_DLL nsScriptMessageHandler : public nsAbstractMessageHandler
{
public:
  nsScriptMessageHandler(const nsScriptMessageDesc& desc);
  ~nsScriptMessageHandler();

  void FillMessagePropertyValues(const nsMessage& msg, nsDynamicArray<nsVariant>& out_propertyValues);

private:
  nsArrayPtr<const nsAbstractProperty* const> m_Properties;
};

class NS_CORE_DLL nsScriptInstance
{
public:
  nsScriptInstance(nsReflectedClass& inout_owner, nsWorld* pWorld);
  virtual ~nsScriptInstance() = default;

  nsReflectedClass& GetOwner() { return m_Owner; }
  nsWorld* GetWorld() { return m_pWorld; }

  virtual void SetInstanceVariables(const nsArrayMap<nsHashedString, nsVariant>& parameters);
  virtual void SetInstanceVariable(const nsHashedString& sName, const nsVariant& value) = 0;
  virtual nsVariant GetInstanceVariable(const nsHashedString& sName) = 0;

private:
  nsReflectedClass& m_Owner;
  nsWorld* m_pWorld = nullptr;
};

struct NS_CORE_DLL nsScriptAllocator
{
  static nsAllocator* GetAllocator();
};

/// \brief creates a new instance of type using the script allocator
#define NS_SCRIPT_NEW(type, ...) NS_NEW(nsScriptAllocator::GetAllocator(), type, __VA_ARGS__)
