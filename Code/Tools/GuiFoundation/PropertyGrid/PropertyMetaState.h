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
struct wdPropertyUiState
{
  enum Visibility
  {
    Default,   ///< Displayed normally, for editing (unless the property is read-only)
    Invisible, ///< Hides the property entirely
    Disabled,  ///< The property is shown but disabled, when multiple objects are selected and in one the property is invisible, in the other it is
               ///< disabled, the disabled state takes precedence
  };

  wdPropertyUiState()
  {
    m_Visibility = Visibility::Default;
  }

  Visibility m_Visibility;
  wdString m_sNewLabelText;
};

/// \brief Event that is broadcast whenever information about how to present properties is required
struct wdPropertyMetaStateEvent
{
  /// The object for which the information is queried
  const wdDocumentObject* m_pObject = nullptr;

  /// The map into which event handlers should write their information about the state of each property.
  /// The string is the property name that identifies the property in m_pObject.
  wdMap<wdString, wdPropertyUiState>* m_pPropertyStates = nullptr;
};

/// \brief Event that is broadcast whenever information about how to present elements in a container is required
struct wdContainerElementMetaStateEvent
{
  /// The object for which the information is queried
  const wdDocumentObject* m_pObject = nullptr;
  /// The Container property
  const char* m_szProperty = nullptr;
  /// The map into which event handlers should write their information about the state of each container element.
  /// The wdVariant should be the key of the container element, either wdUInt32 for arrays and sets or wdString for maps.
  wdHashTable<wdVariant, wdPropertyUiState>* m_pContainerElementStates = nullptr;
};

/// \brief This class allows to query additional information about how to present properties in the property grid
///
/// The property grid calls GetTypePropertiesState() and GetContainerElementsState() with the current selection of wdDocumentObject's.
/// This triggers the wdPropertyMetaStateEvent to be broadcast, which allows for other code to determine additional
/// information for the properties and write it into the event data.
class WD_GUIFOUNDATION_DLL wdPropertyMetaState
{
  WD_DECLARE_SINGLETON(wdPropertyMetaState);

public:
  wdPropertyMetaState();

  /// \brief Queries the property meta state for a single wdDocumentObject
  void GetTypePropertiesState(const wdDocumentObject* pObject, wdMap<wdString, wdPropertyUiState>& out_propertyStates);

  /// \brief Queries the property meta state for a multi selection of wdDocumentObject's
  ///
  /// This will query the information for every single selected object and then merge the result into one.
  void GetTypePropertiesState(const wdHybridArray<wdPropertySelection, 8>& items, wdMap<wdString, wdPropertyUiState>& out_propertyStates);

  /// \brief Queries the meta state for the elements of a single container property on one wdDocumentObject.
  void GetContainerElementsState(const wdDocumentObject* pObject, const char* szProperty, wdHashTable<wdVariant, wdPropertyUiState>& out_propertyStates);

  /// \brief Queries the meta state for the elements of a single container property on a multi selection of wdDocumentObjects.
  ///
  /// This will query the information for every single selected object and then merge the result into one.
  void GetContainerElementsState(const wdHybridArray<wdPropertySelection, 8>& items, const char* szProperty, wdHashTable<wdVariant, wdPropertyUiState>& out_propertyStates);

  /// Attach to this event to get notified of property state queries.
  /// Add information to wdPropertyMetaStateEvent::m_pPropertyStates to return data.
  wdEvent<wdPropertyMetaStateEvent&> m_Events;
  /// Attach to this event to get notified of container element state queries.
  /// Add information to wdContainerElementMetaStateEvent::m_pContainerElementStates to return data.
  wdEvent<wdContainerElementMetaStateEvent&> m_ContainerEvents;

private:
  wdMap<wdString, wdPropertyUiState> m_Temp;
  wdHashTable<wdVariant, wdPropertyUiState> m_Temp2;
};
