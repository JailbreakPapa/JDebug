#pragma once

#include <Core/Messages/EventMessage.h>
#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsTriggerState
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Activated,   ///< The trigger was just activated (area entered, key pressed, etc.)
    Continuing,  ///< The trigger is active for more than one frame now.
    Deactivated, ///< The trigger was just deactivated (left area, key released, etc.)

    Default = Activated
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsTriggerState);

/// \brief For internal use by components to trigger some known behavior. Usually components will post this message to themselves with a
/// delay, e.g. to trigger self destruction.
struct NS_CORE_DLL nsMsgComponentInternalTrigger : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgComponentInternalTrigger, nsMessage);

  /// Identifies what the message should trigger.
  nsHashedString m_sMessage;

  nsInt32 m_iPayload = 0;
};

/// \brief Sent when something enters or leaves a trigger
struct NS_CORE_DLL nsMsgTriggerTriggered : public nsEventMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgTriggerTriggered, nsEventMessage);

  /// Identifies what the message should trigger.
  nsHashedString m_sMessage;

  /// Messages are only sent for 'entered' ('Activated') and 'left' ('Deactivated')
  nsEnum<nsTriggerState> m_TriggerState;

  /// The object that entered the trigger volume.
  nsGameObjectHandle m_hTriggeringObject;
};
