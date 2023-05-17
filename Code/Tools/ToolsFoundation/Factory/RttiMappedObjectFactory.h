#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief A factory that creates the closest matching objects according to the passed type.
///
/// Creators can be registered at the factory for a specific type.
/// When the create function is called for a type, the parent type hierarchy is traversed until
/// the first type is found for which a creator is registered.
template <typename Object>
class wdRttiMappedObjectFactory
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdRttiMappedObjectFactory);

public:
  wdRttiMappedObjectFactory();
  ~wdRttiMappedObjectFactory();

  typedef Object* (*CreateObjectFunc)(const wdRTTI* pType);

  void RegisterCreator(const wdRTTI* pType, CreateObjectFunc creator);
  void UnregisterCreator(const wdRTTI* pType);
  Object* CreateObject(const wdRTTI* pType);

  struct Event
  {
    enum class Type
    {
      CreatorAdded,
      CreatorRemoved
    };

    Type m_Type;
    const wdRTTI* m_pRttiType;
  };

  wdEvent<const Event&> m_Events;

private:
  wdHashTable<const wdRTTI*, CreateObjectFunc> m_Creators;
};

#include <ToolsFoundation/Factory/Implementation/RttiMappedObjectFactory_inl.h>
