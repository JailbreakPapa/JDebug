#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsMsgParentChanged : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgParentChanged, nsMessage);

  enum class Type
  {
    ParentLinked,
    ParentUnlinked,
    Invalid
  };

  Type m_Type = Type::Invalid;
  nsGameObjectHandle m_hParent; // previous or new parent, depending on m_Type
};

struct NS_CORE_DLL nsMsgChildrenChanged : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgChildrenChanged, nsMessage);

  enum class Type
  {
    ChildAdded,
    ChildRemoved
  };

  Type m_Type;
  nsGameObjectHandle m_hParent;
  nsGameObjectHandle m_hChild;
};

struct NS_CORE_DLL nsMsgComponentsChanged : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgComponentsChanged, nsMessage);

  enum class Type
  {
    ComponentAdded,
    ComponentRemoved,
    Invalid
  };

  Type m_Type = Type::Invalid;
  nsGameObjectHandle m_hOwner;
  nsComponentHandle m_hComponent;
};
