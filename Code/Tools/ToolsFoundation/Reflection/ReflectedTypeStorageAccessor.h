#pragma once

#include <ToolsFoundation/Reflection/IReflectedTypeAccessor.h>
#include <ToolsFoundation/Reflection/ReflectedTypeStorageManager.h>

/// \brief An wdIReflectedTypeAccessor implementation that also stores the actual data that is defined in the passed wdRTTI.
///
/// This class is used to store data on the tool side for classes that are not known to the tool but exist outside of it
/// like engine components. As this is basically a complex value map the used type can be hot-reloaded. For this, the
/// wdRTTI just needs to be updated with its new definition in the wdPhantomRttiManager and all wdReflectedTypeStorageAccessor
/// will be automatically rearranged to match the new class layout.
class WD_TOOLSFOUNDATION_DLL wdReflectedTypeStorageAccessor : public wdIReflectedTypeAccessor
{
  friend class wdReflectedTypeStorageManager;

public:
  wdReflectedTypeStorageAccessor(const wdRTTI* pReflectedType, wdDocumentObject* pOwner); // [tested]
  ~wdReflectedTypeStorageAccessor();

  virtual const wdVariant GetValue(const char* szProperty, wdVariant index = wdVariant(), wdStatus* pRes = nullptr) const override; // [tested]
  virtual bool SetValue(const char* szProperty, const wdVariant& value, wdVariant index = wdVariant()) override;                   // [tested]

  virtual wdInt32 GetCount(const char* szProperty) const override;
  virtual bool GetKeys(const char* szProperty, wdDynamicArray<wdVariant>& out_keys) const override;

  virtual bool InsertValue(const char* szProperty, wdVariant index, const wdVariant& value) override;
  virtual bool RemoveValue(const char* szProperty, wdVariant index) override;
  virtual bool MoveValue(const char* szProperty, wdVariant oldIndex, wdVariant newIndex) override;

  virtual wdVariant GetPropertyChildIndex(const char* szProperty, const wdVariant& value) const override;

private:
  wdDynamicArray<wdVariant> m_Data;
  const wdReflectedTypeStorageManager::ReflectedTypeStorageMapping* m_pMapping;
};
