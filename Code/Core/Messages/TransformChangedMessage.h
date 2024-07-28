#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsMsgTransformChanged : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgTransformChanged, nsMessage);

  nsTransform m_OldGlobalTransform;
  nsTransform m_NewGlobalTransform;
};
