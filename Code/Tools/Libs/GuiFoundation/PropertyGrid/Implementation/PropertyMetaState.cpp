#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
NS_IMPLEMENT_SINGLETON(nsPropertyMetaState);

NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, PropertyMetaState)

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsPropertyMetaState);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (nsPropertyMetaState::GetSingleton())
    {
      auto ptr = nsPropertyMetaState::GetSingleton();
      NS_DEFAULT_DELETE(ptr);
    }
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsPropertyMetaState::nsPropertyMetaState()
  : m_SingletonRegistrar(this)
{
}

void nsPropertyMetaState::GetTypePropertiesState(const nsDocumentObject* pObject, nsMap<nsString, nsPropertyUiState>& out_propertyStates)
{
  nsPropertyMetaStateEvent eventData;
  eventData.m_pPropertyStates = &out_propertyStates;
  eventData.m_pObject = pObject;

  m_Events.Broadcast(eventData);
}

void nsPropertyMetaState::GetTypePropertiesState(const nsHybridArray<nsPropertySelection, 8>& items, nsMap<nsString, nsPropertyUiState>& out_propertyStates)
{
  for (const auto& sel : items)
  {
    m_Temp.Clear();
    GetTypePropertiesState(sel.m_pObject, m_Temp);

    for (auto it = m_Temp.GetIterator(); it.IsValid(); ++it)
    {
      auto& curState = out_propertyStates[it.Key()];

      curState.m_Visibility = nsMath::Max(curState.m_Visibility, it.Value().m_Visibility);
      curState.m_sNewLabelText = it.Value().m_sNewLabelText;
    }
  }
}

void nsPropertyMetaState::GetContainerElementsState(const nsDocumentObject* pObject, const char* szProperty, nsHashTable<nsVariant, nsPropertyUiState>& out_propertyStates)
{
  nsContainerElementMetaStateEvent eventData;
  eventData.m_pContainerElementStates = &out_propertyStates;
  eventData.m_pObject = pObject;
  eventData.m_szProperty = szProperty;

  m_ContainerEvents.Broadcast(eventData);
}

void nsPropertyMetaState::GetContainerElementsState(const nsHybridArray<nsPropertySelection, 8>& items, const char* szProperty, nsHashTable<nsVariant, nsPropertyUiState>& out_propertyStates)
{
  for (const auto& sel : items)
  {
    m_Temp2.Clear();
    GetContainerElementsState(sel.m_pObject, szProperty, m_Temp2);

    for (auto it = m_Temp2.GetIterator(); it.IsValid(); ++it)
    {
      auto& curState = out_propertyStates[it.Key()];

      curState.m_Visibility = nsMath::Max(curState.m_Visibility, it.Value().m_Visibility);
      curState.m_sNewLabelText = it.Value().m_sNewLabelText;
    }
  }
}
