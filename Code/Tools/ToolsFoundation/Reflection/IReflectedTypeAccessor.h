#pragma once

#include <ToolsFoundation/Reflection/ReflectedType.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

class wdDocumentObject;
struct wdStatus;

/// \brief Provides access to the properties of an wdRTTI compatible data storage.
class WD_TOOLSFOUNDATION_DLL wdIReflectedTypeAccessor
{
public:
  /// \brief Constructor for the wdIReflectedTypeAccessor.
  ///
  /// It is a valid implementation to pass an invalid handle. Note that in this case there is no way to determine
  /// what is actually stored inside. However, it can be useful to use e.g. the wdReflectedTypeDirectAccessor
  /// to set properties on the engine runtime side without having the wdPhantomRttiManager initialized.
  wdIReflectedTypeAccessor(const wdRTTI* pRtti, wdDocumentObject* pOwner)
    : m_pRtti(pRtti)
    , m_pOwner(pOwner)
  {
  } // [tested]

  /// \brief Returns the wdRTTI* of the wrapped instance type.
  const wdRTTI* GetType() const { return m_pRtti; } // [tested]

  /// \brief Returns the value of the property defined by its path. Return value is invalid iff the path was invalid.
  virtual const wdVariant GetValue(const char* szProperty, wdVariant index = wdVariant(), wdStatus* pRes = nullptr) const = 0;

  /// \brief Sets a property defined by its path to the given value. Returns whether the operation was successful.
  virtual bool SetValue(const char* szProperty, const wdVariant& value, wdVariant index = wdVariant()) = 0;

  virtual wdInt32 GetCount(const char* szProperty) const = 0;
  virtual bool GetKeys(const char* szProperty, wdDynamicArray<wdVariant>& out_keys) const = 0;

  virtual bool InsertValue(const char* szProperty, wdVariant index, const wdVariant& value) = 0;
  virtual bool RemoveValue(const char* szProperty, wdVariant index) = 0;
  virtual bool MoveValue(const char* szProperty, wdVariant oldIndex, wdVariant newIndex) = 0;

  virtual wdVariant GetPropertyChildIndex(const char* szProperty, const wdVariant& value) const = 0;

  const wdDocumentObject* GetOwner() const { return m_pOwner; }

  bool GetValues(const char* szProperty, wdDynamicArray<wdVariant>& out_values) const;


private:
  friend class wdDocumentObjectManager;
  friend class wdDocumentObject;

  const wdRTTI* m_pRtti;
  wdDocumentObject* m_pOwner;
};
