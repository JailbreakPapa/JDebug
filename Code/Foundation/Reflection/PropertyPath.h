#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/Variant.h>

class nsAbstractProperty;


///\brief Reflected property step that can be used to init an nsPropertyPath
struct NS_FOUNDATION_DLL nsPropertyPathStep
{
  nsString m_sProperty;
  nsVariant m_Index;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsPropertyPathStep);

///\brief Stores a path from an object of a given type to a property inside of it.
/// Once initialized to a specific path, the target property/object of the path can be read or written on
/// multiple root objects.
/// An empty path is allowed in which case WriteToLeafObject/ReadFromLeafObject will return pRootObject directly.
///
/// TODO: read/write methods and ResolvePath should return a failure state.
class NS_FOUNDATION_DLL nsPropertyPath
{
public:
  nsPropertyPath();
  ~nsPropertyPath();

  /// \brief Returns true if InitializeFromPath() has been successfully called and it is therefore possible to use the other functions.
  bool IsValid() const;

  ///\brief Resolves a path in the syntax 'propertyName[index]/propertyName[index]/...' into steps.
  /// The '[index]' part is only added for properties that require indices (arrays and maps).
  nsResult InitializeFromPath(const nsRTTI& rootObjectRtti, const char* szPath);
  ///\brief Resolves a path provided as an array of nsPropertyPathStep.
  nsResult InitializeFromPath(const nsRTTI* pRootObjectRtti, const nsArrayPtr<const nsPropertyPathStep> path);

  ///\brief Applies the entire path and allows writing to the target object.
  nsResult WriteToLeafObject(void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeaf, const nsRTTI& pType)> func) const;
  ///\brief Applies the entire path and allows reading from the target object.
  nsResult ReadFromLeafObject(void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeaf, const nsRTTI& pType)> func) const;

  ///\brief Applies the path up to the last step and allows a functor to write to the final property.
  nsResult WriteProperty(
    void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeafObject, const nsRTTI& pLeafType, const nsAbstractProperty* pProp, const nsVariant& index)> func) const;
  ///\brief Applies the path up to the last step and allows a functor to read from the final property.
  nsResult ReadProperty(
    void* pRootObject, const nsRTTI& type, nsDelegate<void(void* pLeafObject, const nsRTTI& pLeafType, const nsAbstractProperty* pProp, const nsVariant& index)> func) const;

  ///\brief Convenience function that writes 'value' to the 'pRootObject' at the current path.
  void SetValue(void* pRootObject, const nsRTTI& type, const nsVariant& value) const;
  ///\brief Convenience function that writes 'value' to the 'pRootObject' at the current path.
  template <typename T>
  NS_ALWAYS_INLINE void SetValue(T* pRootObject, const nsVariant& value) const
  {
    SetValue(pRootObject, *nsGetStaticRTTI<T>(), value);
  }

  ///\brief Convenience function that reads the value from 'pRootObject' at the current path and stores it in 'out_value'.
  void GetValue(void* pRootObject, const nsRTTI& type, nsVariant& out_value) const;
  ///\brief Convenience function that reads the value from 'pRootObject' at the current path and stores it in 'out_value'.
  template <typename T>
  NS_ALWAYS_INLINE void GetValue(T* pRootObject, nsVariant& out_value) const
  {
    GetValue(pRootObject, *nsGetStaticRTTI<T>(), out_value);
  }

private:
  struct ResolvedStep
  {
    const nsAbstractProperty* m_pProperty = nullptr;
    nsVariant m_Index;
  };

  static nsResult ResolvePath(void* pCurrentObject, const nsRTTI* pType, const nsArrayPtr<const ResolvedStep> path, bool bWriteToObject,
    const nsDelegate<void(void* pLeaf, const nsRTTI& pType)>& func);

  bool m_bIsValid = false;
  nsHybridArray<ResolvedStep, 2> m_PathSteps;
};
