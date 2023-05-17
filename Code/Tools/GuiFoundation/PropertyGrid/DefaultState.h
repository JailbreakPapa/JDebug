#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Foundation/Basics.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/Status.h>
#include <GuiFoundation/PropertyGrid/Declarations.h>

class wdDefaultStateProvider;
class wdObjectAccessorBase;
class wdDocumentObject;
class wdAbstractProperty;

/// \brief Registry for all wdDefaultStateProvider factory functions.
class WD_GUIFOUNDATION_DLL wdDefaultState
{
public:
  /// \brief The functor interface for the wdDefaultStateProvider factory function
  ///
  /// The return value is a sharedPtr as each implementation can decide whether to provide the same instance for all objects or whether a custom instance should be created for each object to allow for state caching (e.g. prefab root information). Returning nullptr is also valid for objects / containers for which the factory has no use (e.g. prefab default state provider on an object that does not belong to a prefab).
  /// The function is called for wdDefaultObjectState usage with the pProp field left blank.
  /// For wdDefaultContainerState usage pProp will point to the container property.
  typedef wdSharedPtr<wdDefaultStateProvider> (*CreateStateProviderFunc)(wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp);

  /// \brief Registers a wdDefaultStateProvider factory method. It is safe to register / unregister factories at any time.
  static void RegisterDefaultStateProvider(CreateStateProviderFunc func);
  /// \brief Unregisters a wdDefaultStateProvider factory method.
  static void UnregisterDefaultStateProvider(CreateStateProviderFunc func);

private:
  friend class wdDefaultObjectState;
  friend class wdDefaultContainerState;
  static wdDynamicArray<CreateStateProviderFunc> s_Factories;
};

/// \brief Object used to query and revert to the default state of all properties of an object.
///
/// This class should not be persisted in memory and just used on the stack to query all property states and then destroyed. It should also not be used across hierarchical changes of any kind (deleting objects etc).
class WD_GUIFOUNDATION_DLL wdDefaultObjectState
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDefaultObjectState);

public:
  /// \brief Constructor. Will collect the appropriate wdDefaultStateProviders to query the states.
  /// \param pAccessor Used to revert properties and query their current value.
  /// \param selection For which objects the default state should be queried. The wdPropertySelection::m_Index should be invalid.
  wdDefaultObjectState(wdObjectAccessorBase* pAccessor, const wdArrayPtr<wdPropertySelection> selection);

  /// \brief Returns the color of the top-most wdDefaultStateProvider of the first element of the selection.
  wdColorGammaUB GetBackgroundColor() const;
  /// \brief Returns the name of the top-most wdDefaultStateProvider of the first element of the selection.
  wdString GetStateProviderName() const;

  bool IsDefaultValue(const char* szProperty) const;
  bool IsDefaultValue(const wdAbstractProperty* pProp) const;
  wdStatus RevertProperty(const char* szProperty);
  wdStatus RevertProperty(const wdAbstractProperty* pProp);
  wdStatus RevertObject();
  wdVariant GetDefaultValue(const char* szProperty, wdUInt32 uiSelectionIndex = 0) const;
  wdVariant GetDefaultValue(const wdAbstractProperty* pProp, wdUInt32 uiSelectionIndex = 0) const;


private:
  wdObjectAccessorBase* m_pAccessor = nullptr;
  wdArrayPtr<wdPropertySelection> m_Selection;
  wdHybridArray<wdHybridArray<wdSharedPtr<wdDefaultStateProvider>, 4>, 1> m_Providers;
};

/// \brief Object used to query and revert to the default state of all elements of a container of an object.
///
/// This class should not be persisted in memory and just used on the stack to query all element states and then destroyed. It should also not be used across hierarchical changes of any kind (deleting objects etc).
class WD_GUIFOUNDATION_DLL wdDefaultContainerState
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDefaultContainerState);

public:
  /// \brief Constructor. Will collect the appropriate wdDefaultStateProviders to query the states.
  /// \param pAccessor Used to revert properties and query their current value.
  /// \param selection For which objects the default state should be queried. If wdPropertySelection::m_Index is set, IsDefaultElement and RevertElement will query the value under that index if the passed in index is invalid.
  /// \param szProperty The name of the container for which default states should be queried.
  wdDefaultContainerState(wdObjectAccessorBase* pAccessor, const wdArrayPtr<wdPropertySelection> selection, const char* szProperty);

  /// \brief Returns the color of the top-most wdDefaultStateProvider of the first element of the selection.
  /// \sa wdDefaultStateProvider::GetBackgroundColor
  wdColorGammaUB GetBackgroundColor() const;
  /// \brief Returns the name of the top-most wdDefaultStateProvider of the first element of the selection.
  /// \sa wdDefaultStateProvider::GetStateProviderName
  wdString GetStateProviderName() const;

  bool IsDefaultElement(wdVariant index) const;
  bool IsDefaultContainer() const;
  wdStatus RevertElement(wdVariant index);
  wdStatus RevertContainer();
  wdVariant GetDefaultElement(wdVariant index, wdUInt32 uiSelectionIndex = 0) const;
  wdVariant GetDefaultContainer(wdUInt32 uiSelectionIndex = 0) const;

private:
  wdObjectAccessorBase* m_pAccessor = nullptr;
  const wdAbstractProperty* m_pProp = nullptr;
  wdArrayPtr<wdPropertySelection> m_Selection;
  wdHybridArray<wdHybridArray<wdSharedPtr<wdDefaultStateProvider>, 4>, 1> m_Providers;
};

/// \brief Interface for querying and restoring the default state of objects and containers.
///
/// The high level functions IsDefaultValue, RevertProperty, RevertObjectContainer don't need to be overwritten in most cases. Instead, just implementing the pure virtual methods is enough.
class WD_GUIFOUNDATION_DLL wdDefaultStateProvider : public wdRefCounted
{
public:
  /// \brief Parent hierarchy of state providers.
  ///
  /// wdDefaultContainerState and wdDefaultObjectState will build a hierarchy of parent default state providers depending on the root depth of all available providers (this is like virtual function overrides but with dynamic parent classes). If a provider can't handle a request, it should forward it to the first element in the superPtr array and pass in superPtr.GetSubArray(1) to that function call. Note that generally you don't need to check for validity of the ptr as the wdAttributeDefaultStateProvider has root depth of -1 and will thus always be the last one in line.
  using SuperArray = const wdArrayPtr<const wdSharedPtr<wdDefaultStateProvider>>;

  /// \brief Returns the root depth of this provider instance.
  ///
  /// This is through how many properties and objects we needed to pass through from the object and property passed into the factory method to find the root object / property that this provider represents.
  /// For example if we have this object hierarchy:
  /// A
  /// |-children- B
  ///             |-elements- C
  ///
  /// If A is a prefab and the factory method was called for C (with no property) then we need to walk up the hierarchy via elements container, the B object, the children container and then finally A. Thus, we need 4 hops to get the the prefab root which means the root depth for this provider instance is 4.
  virtual wdInt32 GetRootDepth() const = 0;

  /// \brief Returns a color to be used in the property grid. Only the hue of the color is used. If alpha is 0, the color is ignored and no tinting of the property grid takes place.
  virtual wdColorGammaUB GetBackgroundColor() const = 0;

  /// \brief Returns the name of this state provider. Can be used to check what the outer most provider is for GUI purposes.
  virtual wdString GetStateProviderName() const = 0;

  /// \brief Returns the default value of an object's property at a given index.
  /// \param superPtr Parent hierarchy of inner providers that should be called of this instance cannot handle the request. See SuperArray definition for details.
  /// \param pAccessor Accessor to be used for querying object values if necessary. Always valid.
  /// \param pObject The object for which the default value should be queried. Always valid.
  /// \param pProp The property for which the default value should be queried. Always valid.
  /// \param index For containers: If the index is valid, the container element's default value is requested. If not, the entire container (either array or dictionary) is requested.
  /// \return The default value. wdReflectionUtils::GetDefaultValue is a good example what is expected to be returned.
  /// \sa wdReflectionUtils::GetDefaultValue, wdDefaultStateProvider::DoesVariantMatchProperty
  virtual wdVariant GetDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) = 0;

  /// \brief Queries an array of diff operations that can be executed to revert the object container.
  /// \param superPtr superPtr Parent hierarchy of inner providers that should be called of this instance cannot handle the request. See SuperArray definition for details.
  /// \param pAccessor pAccessor Accessor to be used for querying object values if necessary. Always valid.
  /// \param pObject pObject The object which is to be reverted. Always valid.
  /// \param pProp pProp The container property which is to be reverted. Always valid.
  /// \param out_diff An array of diff operations that should be executed via wdDocumentObjectConverterReader::ApplyDiffToObject to revert the object / container to its default state.
  /// \return If failure is returned, the operation failed and the undo transaction should be canceled.
  /// \sa wdDocumentObjectConverterReader::ApplyDiffToObject
  virtual wdStatus CreateRevertContainerDiff(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDeque<wdAbstractGraphDiffOperation>& out_diff) = 0;

public:
  virtual bool IsDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant());
  virtual wdStatus RevertProperty(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant());
  virtual wdStatus RevertObjectContainer(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp);

  /// \brief A sanity check function that verifies that a given variant's value matches that expected of the property at the given index. If index is invalid and the property a container, the value must be an array or dictionary of the property's type.
  static bool DoesVariantMatchProperty(const wdVariant& value, const wdAbstractProperty* pProp, wdVariant index = wdVariant());
};
