#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
WD_IMPLEMENT_SINGLETON(wdPropertyMetaState);

WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, PropertyMetaState)

  ON_CORESYSTEMS_STARTUP
  {
    WD_DEFAULT_NEW(wdPropertyMetaState);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (wdPropertyMetaState::GetSingleton())
    {
      auto ptr = wdPropertyMetaState::GetSingleton();
      WD_DEFAULT_DELETE(ptr);
    }
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdPropertyMetaState::wdPropertyMetaState()
  : m_SingletonRegistrar(this)
{
}

void wdPropertyMetaState::GetTypePropertiesState(const wdDocumentObject* pObject, wdMap<wdString, wdPropertyUiState>& out_propertyStates)
{
  wdPropertyMetaStateEvent eventData;
  eventData.m_pPropertyStates = &out_propertyStates;
  eventData.m_pObject = pObject;

  m_Events.Broadcast(eventData);
}

void wdPropertyMetaState::GetTypePropertiesState(const wdHybridArray<wdPropertySelection, 8>& items, wdMap<wdString, wdPropertyUiState>& out_propertyStates)
{
  for (const auto& sel : items)
  {
    m_Temp.Clear();
    GetTypePropertiesState(sel.m_pObject, m_Temp);

    for (auto it = m_Temp.GetIterator(); it.IsValid(); ++it)
    {
      auto& curState = out_propertyStates[it.Key()];

      curState.m_Visibility = wdMath::Max(curState.m_Visibility, it.Value().m_Visibility);
      curState.m_sNewLabelText = it.Value().m_sNewLabelText;
    }
  }
}

void wdPropertyMetaState::GetContainerElementsState(const wdDocumentObject* pObject, const char* szProperty, wdHashTable<wdVariant, wdPropertyUiState>& out_propertyStates)
{
  wdContainerElementMetaStateEvent eventData;
  eventData.m_pContainerElementStates = &out_propertyStates;
  eventData.m_pObject = pObject;
  eventData.m_szProperty = szProperty;

  m_ContainerEvents.Broadcast(eventData);
}

void wdPropertyMetaState::GetContainerElementsState(const wdHybridArray<wdPropertySelection, 8>& items, const char* szProperty, wdHashTable<wdVariant, wdPropertyUiState>& out_propertyStates)
{
  for (const auto& sel : items)
  {
    m_Temp2.Clear();
    GetContainerElementsState(sel.m_pObject, szProperty, m_Temp2);

    for (auto it = m_Temp2.GetIterator(); it.IsValid(); ++it)
    {
      auto& curState = out_propertyStates[it.Key()];

      curState.m_Visibility = wdMath::Max(curState.m_Visibility, it.Value().m_Visibility);
      curState.m_sNewLabelText = it.Value().m_sNewLabelText;
    }
  }
}
