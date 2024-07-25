#pragma once

#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageManager.h>

/// \brief An nsIReflectedTypeAccessor implementation that also stores the actual data that is defined in the passed nsRTTI.
///
/// This class is used to store data on the tool side for classes that are not known to the tool but exist outside of it
/// like engine components. As this is basically a complex value map the used type can be hot-reloaded. For this, the
/// nsRTTI just needs to be updated with its new definition in the nsPhantomRttiManager and all nsReflectedTypeStorageAccessor
/// will be automatically rearranged to match the new class layout.
class NS_TOOLSFOUNDATION_DLL nsReflectedTypeStorageAccessor : public nsIReflectedTypeAccessor
{
  friend class nsReflectedTypeStorageManager;

public:
  nsReflectedTypeStorageAccessor(const nsRTTI* pReflectedType, nsDocumentObject* pOwner);                                           // [tested]
  ~nsReflectedTypeStorageAccessor();

  virtual const nsVariant GetValue(nsStringView sProperty, nsVariant index = nsVariant(), nsStatus* pRes = nullptr) const override; // [tested]
  virtual bool SetValue(nsStringView sProperty, const nsVariant& value, nsVariant index = nsVariant()) override;                    // [tested]

  virtual nsInt32 GetCount(nsStringView sProperty) const override;
  virtual bool GetKeys(nsStringView sProperty, nsDynamicArray<nsVariant>& out_keys) const override;

  virtual bool InsertValue(nsStringView sProperty, nsVariant index, const nsVariant& value) override;
  virtual bool RemoveValue(nsStringView sProperty, nsVariant index) override;
  virtual bool MoveValue(nsStringView sProperty, nsVariant oldIndex, nsVariant newIndex) override;

  virtual nsVariant GetPropertyChildIndex(nsStringView sProperty, const nsVariant& value) const override;

private:
  nsDynamicArray<nsVariant> m_Data;
  const nsReflectedTypeStorageManager::ReflectedTypeStorageMapping* m_pMapping;
};
