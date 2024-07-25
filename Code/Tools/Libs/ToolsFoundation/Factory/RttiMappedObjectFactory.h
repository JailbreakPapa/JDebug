#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief A factory that creates the closest matching objects according to the passed type.
///
/// Creators can be registered at the factory for a specific type.
/// When the create function is called for a type, the parent type hierarchy is traversed until
/// the first type is found for which a creator is registered.
template <typename Object>
class nsRttiMappedObjectFactory
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsRttiMappedObjectFactory);

public:
  nsRttiMappedObjectFactory();
  ~nsRttiMappedObjectFactory();

  using CreateObjectFunc = Object* (*)(const nsRTTI*);

  void RegisterCreator(const nsRTTI* pType, CreateObjectFunc creator);
  void UnregisterCreator(const nsRTTI* pType);
  Object* CreateObject(const nsRTTI* pType);

  struct Event
  {
    enum class Type
    {
      CreatorAdded,
      CreatorRemoved
    };

    Type m_Type;
    const nsRTTI* m_pRttiType;
  };

  nsEvent<const Event&> m_Events;

private:
  nsHashTable<const nsRTTI*, CreateObjectFunc> m_Creators;
};

#include <ToolsFoundation/Factory/Implementation/RttiMappedObjectFactory_inl.h>
