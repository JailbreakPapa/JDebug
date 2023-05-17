#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/Variant.h>

class wdAbstractProperty;

///\brief Reflected property step that can be used to init an wdPropertyPath
struct WD_FOUNDATION_DLL wdPropertyPathStep
{
  wdString m_sProperty;
  wdVariant m_Index;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdPropertyPathStep);

///\brief Stores a path from an object of a given type to a property inside of it.
/// Once initialized to a specific path, the target property/object of the path can be read or written on
/// multiple root objects.
/// An empty path is allowed in which case WriteToLeafObject/ReadFromLeafObject will return pRootObject directly.
///
/// TODO: read/write methods and ResolvePath should return a failure state.
class WD_FOUNDATION_DLL wdPropertyPath
{
public:
  wdPropertyPath();
  ~wdPropertyPath();

  /// \brief Returns true if InitializeFromPath() has been successfully called and it is therefore possible to use the other functions.
  bool IsValid() const;

  ///\brief Resolves a path in the syntax 'propertyName[index]/propertyName[index]/...' into steps.
  /// The '[index]' part is only added for properties that require indices (arrays and maps).
  wdResult InitializeFromPath(const wdRTTI& rootObjectRtti, const char* szPath);
  ///\brief Resolves a path provided as an array of wdPropertyPathStep.
  wdResult InitializeFromPath(const wdRTTI& rootObjectRtti, const wdArrayPtr<const wdPropertyPathStep> path);

  ///\brief Applies the entire path and allows writing to the target object.
  wdResult WriteToLeafObject(void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeaf, const wdRTTI& pType)> func) const;
  ///\brief Applies the entire path and allows reading from the target object.
  wdResult ReadFromLeafObject(void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeaf, const wdRTTI& pType)> func) const;

  ///\brief Applies the path up to the last step and allows a functor to write to the final property.
  wdResult WriteProperty(
    void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeafObject, const wdRTTI& pLeafType, wdAbstractProperty* pProp, const wdVariant& index)> func) const;
  ///\brief Applies the path up to the last step and allows a functor to read from the final property.
  wdResult ReadProperty(
    void* pRootObject, const wdRTTI& type, wdDelegate<void(void* pLeafObject, const wdRTTI& pLeafType, const wdAbstractProperty* pProp, const wdVariant& index)> func) const;

  ///\brief Convenience function that writes 'value' to the 'pRootObject' at the current path.
  void SetValue(void* pRootObject, const wdRTTI& type, const wdVariant& value) const;
  ///\brief Convenience function that writes 'value' to the 'pRootObject' at the current path.
  template <typename T>
  WD_ALWAYS_INLINE void SetValue(T* pRootObject, const wdVariant& value) const
  {
    SetValue(pRootObject, *wdGetStaticRTTI<T>(), value);
  }

  ///\brief Convenience function that reads the value from 'pRootObject' at the current path and stores it in 'out_value'.
  void GetValue(void* pRootObject, const wdRTTI& type, wdVariant& out_value) const;
  ///\brief Convenience function that reads the value from 'pRootObject' at the current path and stores it in 'out_value'.
  template <typename T>
  WD_ALWAYS_INLINE void GetValue(T* pRootObject, wdVariant& out_value) const
  {
    GetValue(pRootObject, *wdGetStaticRTTI<T>(), out_value);
  }

private:
  struct ResolvedStep
  {
    wdAbstractProperty* m_pProperty = nullptr;
    wdVariant m_Index;
  };

  static wdResult ResolvePath(void* pCurrentObject, const wdRTTI* pType, const wdArrayPtr<const ResolvedStep> path, bool bWriteToObject,
    const wdDelegate<void(void* pLeaf, const wdRTTI& pType)>& func);

  bool m_bIsValid = false;
  wdHybridArray<ResolvedStep, 2> m_PathSteps;
};
