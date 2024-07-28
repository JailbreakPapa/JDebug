#include <Core/CorePCH.h>

#include <Core/Messages/CollisionMessage.h>
#include <Core/Messages/CommonMessages.h>
#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/Messages/HierarchyChangedMessages.h>
#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/TriggerMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsMsgCollision);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgCollision, 1, nsRTTIDefaultAllocator<nsMsgCollision>)
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_STATIC_REFLECTED_ENUM(nsTriggerState, 1)
  NS_ENUM_CONSTANTS(nsTriggerState::Activated, nsTriggerState::Continuing, nsTriggerState::Deactivated)
NS_END_STATIC_REFLECTED_ENUM;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgDeleteGameObject);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgDeleteGameObject, 1, nsRTTIDefaultAllocator<nsMsgDeleteGameObject>)
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgComponentInternalTrigger);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgComponentInternalTrigger, 1, nsRTTIDefaultAllocator<nsMsgComponentInternalTrigger>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Message", m_sMessage),
    NS_MEMBER_PROPERTY("Payload", m_iPayload),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgUpdateLocalBounds);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgUpdateLocalBounds, 1, nsRTTIDefaultAllocator<nsMsgUpdateLocalBounds>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgSetPlaying);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgSetPlaying, 1, nsRTTIDefaultAllocator<nsMsgSetPlaying>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Play", m_bPlay)->AddAttributes(new nsDefaultValueAttribute(true)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgParentChanged);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgParentChanged, 1, nsRTTIDefaultAllocator<nsMsgParentChanged>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgChildrenChanged);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgChildrenChanged, 1, nsRTTIDefaultAllocator<nsMsgChildrenChanged>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgComponentsChanged);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgComponentsChanged, 1, nsRTTIDefaultAllocator<nsMsgComponentsChanged>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgTransformChanged);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgTransformChanged, 1, nsRTTIDefaultAllocator<nsMsgTransformChanged>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgSetFloatParameter);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgSetFloatParameter, 1, nsRTTIDefaultAllocator<nsMsgSetFloatParameter>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Name", m_sParameterName),
    NS_MEMBER_PROPERTY("Value", m_fValue),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgGenericEvent);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgGenericEvent, 1, nsRTTIDefaultAllocator<nsMsgGenericEvent>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Message", m_sMessage),
    NS_MEMBER_PROPERTY("Value", m_Value)->AddAttributes(new nsDefaultValueAttribute(0))
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgAnimationReachedEnd);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgAnimationReachedEnd, 1, nsRTTIDefaultAllocator<nsMsgAnimationReachedEnd>)
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgTriggerTriggered)
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgTriggerTriggered, 1, nsRTTIDefaultAllocator<nsMsgTriggerTriggered>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Message", m_sMessage),
    NS_ENUM_MEMBER_PROPERTY("TriggerState", nsTriggerState, m_TriggerState),
    NS_MEMBER_PROPERTY("GameObject", m_hTriggeringObject),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

//////////////////////////////////////////////////////////////////////////

// clang-format on

NS_STATICLINK_FILE(Core, Core_Messages_Implementation_Messages);
