#pragma once

#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocumentObjectManager;

class WD_TOOLSFOUNDATION_DLL wdDocumentObject
{
public:
  wdDocumentObject()
    : m_pDocumentObjectManager(nullptr)
    , m_pParent(nullptr)
  {
  }
  virtual ~wdDocumentObject() {}

  // Accessors
  const wdUuid& GetGuid() const { return m_Guid; }
  const wdRTTI* GetType() const { return GetTypeAccessor().GetType(); }

  const wdDocumentObjectManager* GetDocumentObjectManager() const { return m_pDocumentObjectManager; }
  wdDocumentObjectManager* GetDocumentObjectManager() { return m_pDocumentObjectManager; }

  virtual const wdIReflectedTypeAccessor& GetTypeAccessor() const = 0;
  wdIReflectedTypeAccessor& GetTypeAccessor();

  // Ownership
  const wdDocumentObject* GetParent() const { return m_pParent; }

  virtual void InsertSubObject(wdDocumentObject* pObject, const char* szProperty, const wdVariant& index);
  virtual void RemoveSubObject(wdDocumentObject* pObject);

  // Helper
  void ComputeObjectHash(wdUInt64& ref_uiHash) const;
  const wdHybridArray<wdDocumentObject*, 8>& GetChildren() const { return m_Children; }
  wdDocumentObject* GetChild(const wdUuid& guid);
  const wdDocumentObject* GetChild(const wdUuid& guid) const;
  const char* GetParentProperty() const { return m_sParentProperty; }
  wdAbstractProperty* GetParentPropertyType() const;
  wdVariant GetPropertyIndex() const;
  bool IsOnHeap() const;
  wdUInt32 GetChildIndex(const wdDocumentObject* pChild) const;

private:
  friend class wdDocumentObjectManager;
  void HashPropertiesRecursive(const wdIReflectedTypeAccessor& acc, wdUInt64& uiHash, const wdRTTI* pType) const;

protected:
  wdUuid m_Guid;
  wdDocumentObjectManager* m_pDocumentObjectManager;

  wdDocumentObject* m_pParent;
  wdHybridArray<wdDocumentObject*, 8> m_Children;

  // Sub object data
  wdString m_sParentProperty;
};

class WD_TOOLSFOUNDATION_DLL wdDocumentStorageObject : public wdDocumentObject
{
public:
  wdDocumentStorageObject(const wdRTTI* pType)
    : wdDocumentObject()
    , m_ObjectPropertiesAccessor(pType, this)
  {
  }

  virtual ~wdDocumentStorageObject() {}

  virtual const wdIReflectedTypeAccessor& GetTypeAccessor() const override { return m_ObjectPropertiesAccessor; }

protected:
  wdReflectedTypeStorageAccessor m_ObjectPropertiesAccessor;
};
