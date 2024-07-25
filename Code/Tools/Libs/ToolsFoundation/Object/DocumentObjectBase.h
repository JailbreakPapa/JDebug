#pragma once

#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageAccessor.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocumentObjectManager;

class NS_TOOLSFOUNDATION_DLL nsDocumentObject
{
public:
  nsDocumentObject()

    = default;
  virtual ~nsDocumentObject() = default;

  // Accessors
  const nsUuid& GetGuid() const { return m_Guid; }
  const nsRTTI* GetType() const { return GetTypeAccessor().GetType(); }

  const nsDocumentObjectManager* GetDocumentObjectManager() const { return m_pDocumentObjectManager; }
  nsDocumentObjectManager* GetDocumentObjectManager() { return m_pDocumentObjectManager; }

  virtual const nsIReflectedTypeAccessor& GetTypeAccessor() const = 0;
  nsIReflectedTypeAccessor& GetTypeAccessor();

  // Ownership
  const nsDocumentObject* GetParent() const { return m_pParent; }

  virtual void InsertSubObject(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& index);
  virtual void RemoveSubObject(nsDocumentObject* pObject);

  // Helper
  void ComputeObjectHash(nsUInt64& ref_uiHash) const;
  const nsHybridArray<nsDocumentObject*, 8>& GetChildren() const { return m_Children; }
  nsDocumentObject* GetChild(const nsUuid& guid);
  const nsDocumentObject* GetChild(const nsUuid& guid) const;
  nsStringView GetParentProperty() const { return m_sParentProperty; }
  const nsAbstractProperty* GetParentPropertyType() const;
  nsVariant GetPropertyIndex() const;
  bool IsOnHeap() const;
  nsUInt32 GetChildIndex(const nsDocumentObject* pChild) const;

private:
  friend class nsDocumentObjectManager;
  void HashPropertiesRecursive(const nsIReflectedTypeAccessor& acc, nsUInt64& uiHash, const nsRTTI* pType) const;

protected:
  nsUuid m_Guid;
  nsDocumentObjectManager* m_pDocumentObjectManager = nullptr;

  nsDocumentObject* m_pParent = nullptr;
  nsHybridArray<nsDocumentObject*, 8> m_Children;

  // Sub object data
  nsString m_sParentProperty;
};

class NS_TOOLSFOUNDATION_DLL nsDocumentStorageObject : public nsDocumentObject
{
public:
  nsDocumentStorageObject(const nsRTTI* pType)
    : nsDocumentObject()
    , m_ObjectPropertiesAccessor(pType, this)
  {
  }

  virtual ~nsDocumentStorageObject() = default;

  virtual const nsIReflectedTypeAccessor& GetTypeAccessor() const override { return m_ObjectPropertiesAccessor; }

protected:
  nsReflectedTypeStorageAccessor m_ObjectPropertiesAccessor;
};
