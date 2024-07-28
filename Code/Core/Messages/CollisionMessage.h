#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsMsgCollision : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgCollision, nsMessage);

  nsGameObjectHandle m_hObjectA;
  nsGameObjectHandle m_hObjectB;

  nsComponentHandle m_hComponentA;
  nsComponentHandle m_hComponentB;

  nsVec3 m_vPosition; ///< The collision position in world space.
  nsVec3 m_vNormal;   ///< The collision normal on the surface of object B.
  nsVec3 m_vImpulse;  ///< The collision impulse applied from object A to object B.
};
