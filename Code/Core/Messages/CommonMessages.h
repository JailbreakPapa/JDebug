#pragma once

#include <Core/Messages/EventMessage.h>
#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

/// \brief Common message for components that can be toggled between playing and paused states
struct NS_CORE_DLL nsMsgSetPlaying : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgSetPlaying, nsMessage);

  bool m_bPlay = true;
};

/// \brief Basic message to set some generic parameter to a float value.
struct NS_CORE_DLL nsMsgSetFloatParameter : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgSetFloatParameter, nsMessage);

  nsString m_sParameterName;
  float m_fValue = 0;
};

/// \brief For use in scripts to signal a custom event that some game event has occurred.
///
/// This is a simple message for simple use cases. Create custom messages for more elaborate cases where a string is not sufficient
/// information.
struct NS_CORE_DLL nsMsgGenericEvent : public nsEventMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgGenericEvent, nsEventMessage);

  /// A custom string to identify the intent.
  nsHashedString m_sMessage;
  nsVariant m_Value;
};

/// \brief Sent when an animation reached its end (either forwards or backwards playing)
///
/// This is sent regardless of whether the animation is played once, looped or back and forth,
/// ie. it should be sent at each 'end' point, even when it then starts another cycle.
struct NS_CORE_DLL nsMsgAnimationReachedEnd : public nsEventMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgAnimationReachedEnd, nsEventMessage);
};
