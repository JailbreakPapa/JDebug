#pragma once

#include <ToolsFoundation/Reflection/ReflectedType.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

class nsDocumentObject;
struct nsStatus;

/// \brief Provides access to the properties of an nsRTTI compatible data storage.
class NS_TOOLSFOUNDATION_DLL nsIReflectedTypeAccessor
{
public:
  /// \brief Constructor for the nsIReflectedTypeAccessor.
  ///
  /// It is a valid implementation to pass an invalid handle. Note that in this case there is no way to determine
  /// what is actually stored inside. However, it can be useful to use e.g. the nsReflectedTypeDirectAccessor
  /// to set properties on the engine runtime side without having the nsPhantomRttiManager initialized.
  nsIReflectedTypeAccessor(const nsRTTI* pRtti, nsDocumentObject* pOwner)
    : m_pRtti(pRtti)
    , m_pOwner(pOwner)
  {
  } // [tested]

  /// \brief Returns the nsRTTI* of the wrapped instance type.
  const nsRTTI* GetType() const { return m_pRtti; } // [tested]

  /// \brief Returns the value of the property defined by its path. Return value is invalid iff the path was invalid.
  virtual const nsVariant GetValue(nsStringView sProperty, nsVariant index = nsVariant(), nsStatus* pRes = nullptr) const = 0;

  /// \brief Sets a property defined by its path to the given value. Returns whether the operation was successful.
  virtual bool SetValue(nsStringView sProperty, const nsVariant& value, nsVariant index = nsVariant()) = 0;

  virtual nsInt32 GetCount(nsStringView sProperty) const = 0;
  virtual bool GetKeys(nsStringView sProperty, nsDynamicArray<nsVariant>& out_keys) const = 0;

  virtual bool InsertValue(nsStringView sProperty, nsVariant index, const nsVariant& value) = 0;
  virtual bool RemoveValue(nsStringView sProperty, nsVariant index) = 0;
  virtual bool MoveValue(nsStringView sProperty, nsVariant oldIndex, nsVariant newIndex) = 0;

  virtual nsVariant GetPropertyChildIndex(nsStringView sProperty, const nsVariant& value) const = 0;

  const nsDocumentObject* GetOwner() const { return m_pOwner; }

  bool GetValues(nsStringView sProperty, nsDynamicArray<nsVariant>& out_values) const;


private:
  friend class nsDocumentObjectManager;
  friend class nsDocumentObject;

  const nsRTTI* m_pRtti;
  nsDocumentObject* m_pOwner;
};
