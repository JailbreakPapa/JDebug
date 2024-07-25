#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/RefCounted.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

/// \brief Describes the current meta state of a property for display purposes in the property grid
struct nsPropertyUiState
{
  enum Visibility
  {
    Default,   ///< Displayed normally, for editing (unless the property is read-only)
    Invisible, ///< Hides the property entirely
    Disabled,  ///< The property is shown but disabled, when multiple objects are selected and in one the property is invisible, in the other it is
               ///< disabled, the disabled state takes precedence
  };

  nsPropertyUiState()
  {
    m_Visibility = Visibility::Default;
  }

  Visibility m_Visibility;
  nsString m_sNewLabelText;
};

/// \brief Event that is broadcast whenever information about how to present properties is required
struct nsPropertyMetaStateEvent
{
  /// The object for which the information is queried
  const nsDocumentObject* m_pObject = nullptr;

  /// The map into which event handlers should write their information about the state of each property.
  /// The string is the property name that identifies the property in m_pObject.
  nsMap<nsString, nsPropertyUiState>* m_pPropertyStates = nullptr;
};

/// \brief Event that is broadcast whenever information about how to present elements in a container is required
struct nsContainerElementMetaStateEvent
{
  /// The object for which the information is queried
  const nsDocumentObject* m_pObject = nullptr;
  /// The Container property
  const char* m_szProperty = nullptr;
  /// The map into which event handlers should write their information about the state of each container element.
  /// The nsVariant should be the key of the container element, either nsUInt32 for arrays and sets or nsString for maps.
  nsHashTable<nsVariant, nsPropertyUiState>* m_pContainerElementStates = nullptr;
};

/// \brief This class allows to query additional information about how to present properties in the property grid
///
/// The property grid calls GetTypePropertiesState() and GetContainerElementsState() with the current selection of nsDocumentObject's.
/// This triggers the nsPropertyMetaStateEvent to be broadcast, which allows for other code to determine additional
/// information for the properties and write it into the event data.
class NS_GUIFOUNDATION_DLL nsPropertyMetaState
{
  NS_DECLARE_SINGLETON(nsPropertyMetaState);

public:
  nsPropertyMetaState();

  /// \brief Queries the property meta state for a single nsDocumentObject
  void GetTypePropertiesState(const nsDocumentObject* pObject, nsMap<nsString, nsPropertyUiState>& out_propertyStates);

  /// \brief Queries the property meta state for a multi selection of nsDocumentObject's
  ///
  /// This will query the information for every single selected object and then merge the result into one.
  void GetTypePropertiesState(const nsHybridArray<nsPropertySelection, 8>& items, nsMap<nsString, nsPropertyUiState>& out_propertyStates);

  /// \brief Queries the meta state for the elements of a single container property on one nsDocumentObject.
  void GetContainerElementsState(const nsDocumentObject* pObject, const char* szProperty, nsHashTable<nsVariant, nsPropertyUiState>& out_propertyStates);

  /// \brief Queries the meta state for the elements of a single container property on a multi selection of nsDocumentObjects.
  ///
  /// This will query the information for every single selected object and then merge the result into one.
  void GetContainerElementsState(const nsHybridArray<nsPropertySelection, 8>& items, const char* szProperty, nsHashTable<nsVariant, nsPropertyUiState>& out_propertyStates);

  /// Attach to this event to get notified of property state queries.
  /// Add information to nsPropertyMetaStateEvent::m_pPropertyStates to return data.
  nsEvent<nsPropertyMetaStateEvent&> m_Events;
  /// Attach to this event to get notified of container element state queries.
  /// Add information to nsContainerElementMetaStateEvent::m_pContainerElementStates to return data.
  nsEvent<nsContainerElementMetaStateEvent&> m_ContainerEvents;

private:
  nsMap<nsString, nsPropertyUiState> m_Temp;
  nsHashTable<nsVariant, nsPropertyUiState> m_Temp2;
};
