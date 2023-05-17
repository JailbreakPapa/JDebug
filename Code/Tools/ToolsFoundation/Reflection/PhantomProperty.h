#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdPhantomConstantProperty : public wdAbstractConstantProperty
{
public:
  wdPhantomConstantProperty(const wdReflectedPropertyDescriptor* pDesc);
  ~wdPhantomConstantProperty();

  virtual const wdRTTI* GetSpecificType() const override;
  virtual void* GetPropertyPointer() const override;
  virtual wdVariant GetConstant() const override { return m_Value; }

private:
  wdVariant m_Value;
  wdString m_sPropertyNameStorage;
  wdRTTI* m_pPropertyType;
};

class wdPhantomMemberProperty : public wdAbstractMemberProperty
{
public:
  wdPhantomMemberProperty(const wdReflectedPropertyDescriptor* pDesc);
  ~wdPhantomMemberProperty();

  virtual const wdRTTI* GetSpecificType() const override;
  virtual void* GetPropertyPointer(const void* pInstance) const override { return nullptr; }
  virtual void GetValuePtr(const void* pInstance, void* pObject) const override {}
  virtual void SetValuePtr(void* pInstance, const void* pObject) override {}

private:
  wdString m_sPropertyNameStorage;
  wdRTTI* m_pPropertyType;
};

class wdPhantomFunctionProperty : public wdAbstractFunctionProperty
{
public:
  wdPhantomFunctionProperty(wdReflectedFunctionDescriptor* pDesc);
  ~wdPhantomFunctionProperty();

  virtual wdFunctionType::Enum GetFunctionType() const override;
  virtual const wdRTTI* GetReturnType() const override;
  virtual wdBitflags<wdPropertyFlags> GetReturnFlags() const override;
  virtual wdUInt32 GetArgumentCount() const override;
  virtual const wdRTTI* GetArgumentType(wdUInt32 uiParamIndex) const override;
  virtual wdBitflags<wdPropertyFlags> GetArgumentFlags(wdUInt32 uiParamIndex) const override;
  virtual void Execute(void* pInstance, wdArrayPtr<wdVariant> values, wdVariant& ref_returnValue) const override;

private:
  wdString m_sPropertyNameStorage;
  wdEnum<wdFunctionType> m_FunctionType;
  wdFunctionArgumentDescriptor m_ReturnValue;
  wdDynamicArray<wdFunctionArgumentDescriptor> m_Arguments;
};


class wdPhantomArrayProperty : public wdAbstractArrayProperty
{
public:
  wdPhantomArrayProperty(const wdReflectedPropertyDescriptor* pDesc);
  ~wdPhantomArrayProperty();

  virtual const wdRTTI* GetSpecificType() const override;
  virtual wdUInt32 GetCount(const void* pInstance) const override { return 0; }
  virtual void GetValue(const void* pInstance, wdUInt32 uiIndex, void* pObject) const override {}
  virtual void SetValue(void* pInstance, wdUInt32 uiIndex, const void* pObject) override {}
  virtual void Insert(void* pInstance, wdUInt32 uiIndex, const void* pObject) override {}
  virtual void Remove(void* pInstance, wdUInt32 uiIndex) override {}
  virtual void Clear(void* pInstance) override {}
  virtual void SetCount(void* pInstance, wdUInt32 uiCount) override {}


private:
  wdString m_sPropertyNameStorage;
  wdRTTI* m_pPropertyType;
};


class wdPhantomSetProperty : public wdAbstractSetProperty
{
public:
  wdPhantomSetProperty(const wdReflectedPropertyDescriptor* pDesc);
  ~wdPhantomSetProperty();

  virtual const wdRTTI* GetSpecificType() const override;
  virtual bool IsEmpty(const void* pInstance) const override { return true; }
  virtual void Clear(void* pInstance) override {}
  virtual void Insert(void* pInstance, const void* pObject) override {}
  virtual void Remove(void* pInstance, const void* pObject) override {}
  virtual bool Contains(const void* pInstance, const void* pObject) const override { return false; }
  virtual void GetValues(const void* pInstance, wdDynamicArray<wdVariant>& out_keys) const override {}

private:
  wdString m_sPropertyNameStorage;
  wdRTTI* m_pPropertyType;
};


class wdPhantomMapProperty : public wdAbstractMapProperty
{
public:
  wdPhantomMapProperty(const wdReflectedPropertyDescriptor* pDesc);
  ~wdPhantomMapProperty();

  virtual const wdRTTI* GetSpecificType() const override;
  virtual bool IsEmpty(const void* pInstance) const override { return true; }
  virtual void Clear(void* pInstance) override {}
  virtual void Insert(void* pInstance, const char* szKey, const void* pObject) override {}
  virtual void Remove(void* pInstance, const char* szKey) override {}
  virtual bool Contains(const void* pInstance, const char* szKey) const override { return false; }
  virtual bool GetValue(const void* pInstance, const char* szKey, void* pObject) const override { return false; }
  virtual void GetKeys(const void* pInstance, wdHybridArray<wdString, 16>& out_keys) const override {}

private:
  wdString m_sPropertyNameStorage;
  wdRTTI* m_pPropertyType;
};
