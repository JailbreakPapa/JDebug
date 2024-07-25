#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsPhantomConstantProperty : public nsAbstractConstantProperty
{
public:
  nsPhantomConstantProperty(const nsReflectedPropertyDescriptor* pDesc);
  ~nsPhantomConstantProperty();

  virtual const nsRTTI* GetSpecificType() const override;
  virtual void* GetPropertyPointer() const override;
  virtual nsVariant GetConstant() const override { return m_Value; }

private:
  nsVariant m_Value;
  nsString m_sPropertyNameStorage;
  const nsRTTI* m_pPropertyType;
};

class nsPhantomMemberProperty : public nsAbstractMemberProperty
{
public:
  nsPhantomMemberProperty(const nsReflectedPropertyDescriptor* pDesc);
  ~nsPhantomMemberProperty();

  virtual const nsRTTI* GetSpecificType() const override;
  virtual void* GetPropertyPointer(const void* pInstance) const override { return nullptr; }
  virtual void GetValuePtr(const void* pInstance, void* pObject) const override {}
  virtual void SetValuePtr(void* pInstance, const void* pObject) const override {}

private:
  nsString m_sPropertyNameStorage;
  const nsRTTI* m_pPropertyType;
};

class nsPhantomFunctionProperty : public nsAbstractFunctionProperty
{
public:
  nsPhantomFunctionProperty(nsReflectedFunctionDescriptor* pDesc);
  ~nsPhantomFunctionProperty();

  virtual nsFunctionType::Enum GetFunctionType() const override;
  virtual const nsRTTI* GetReturnType() const override;
  virtual nsBitflags<nsPropertyFlags> GetReturnFlags() const override;
  virtual nsUInt32 GetArgumentCount() const override;
  virtual const nsRTTI* GetArgumentType(nsUInt32 uiParamIndex) const override;
  virtual nsBitflags<nsPropertyFlags> GetArgumentFlags(nsUInt32 uiParamIndex) const override;
  virtual void Execute(void* pInstance, nsArrayPtr<nsVariant> values, nsVariant& ref_returnValue) const override;

private:
  nsString m_sPropertyNameStorage;
  nsEnum<nsFunctionType> m_FunctionType;
  nsFunctionArgumentDescriptor m_ReturnValue;
  nsDynamicArray<nsFunctionArgumentDescriptor> m_Arguments;
};


class nsPhantomArrayProperty : public nsAbstractArrayProperty
{
public:
  nsPhantomArrayProperty(const nsReflectedPropertyDescriptor* pDesc);
  ~nsPhantomArrayProperty();

  virtual const nsRTTI* GetSpecificType() const override;
  virtual nsUInt32 GetCount(const void* pInstance) const override { return 0; }
  virtual void GetValue(const void* pInstance, nsUInt32 uiIndex, void* pObject) const override {}
  virtual void SetValue(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override {}
  virtual void Insert(void* pInstance, nsUInt32 uiIndex, const void* pObject) const override {}
  virtual void Remove(void* pInstance, nsUInt32 uiIndex) const override {}
  virtual void Clear(void* pInstance) const override {}
  virtual void SetCount(void* pInstance, nsUInt32 uiCount) const override {}


private:
  nsString m_sPropertyNameStorage;
  const nsRTTI* m_pPropertyType;
};


class nsPhantomSetProperty : public nsAbstractSetProperty
{
public:
  nsPhantomSetProperty(const nsReflectedPropertyDescriptor* pDesc);
  ~nsPhantomSetProperty();

  virtual const nsRTTI* GetSpecificType() const override;
  virtual bool IsEmpty(const void* pInstance) const override { return true; }
  virtual void Clear(void* pInstance) const override {}
  virtual void Insert(void* pInstance, const void* pObject) const override {}
  virtual void Remove(void* pInstance, const void* pObject) const override {}
  virtual bool Contains(const void* pInstance, const void* pObject) const override { return false; }
  virtual void GetValues(const void* pInstance, nsDynamicArray<nsVariant>& out_keys) const override {}

private:
  nsString m_sPropertyNameStorage;
  const nsRTTI* m_pPropertyType;
};


class nsPhantomMapProperty : public nsAbstractMapProperty
{
public:
  nsPhantomMapProperty(const nsReflectedPropertyDescriptor* pDesc);
  ~nsPhantomMapProperty();

  virtual const nsRTTI* GetSpecificType() const override;
  virtual bool IsEmpty(const void* pInstance) const override { return true; }
  virtual void Clear(void* pInstance) const override {}
  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) const override {}
  virtual void Remove(void* pInstance, const char* szKey) const override {}
  virtual bool Contains(const void* pInstance, const char* szKey) const override { return false; }
  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const override { return false; }
  virtual void GetKeys(const void* pInstance, nsHybridArray<nsString, 16>& out_keys) const override {}

private:
  nsString m_sPropertyNameStorage;
  const nsRTTI* m_pPropertyType;
};
