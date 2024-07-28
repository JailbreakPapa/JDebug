#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsMsgOnlyApplyToObject : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgOnlyApplyToObject, nsMessage);

  nsGameObjectHandle m_hObject;
};
