#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Communication/Message.h>

struct NS_CORE_DLL nsMsgDeleteGameObject : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgDeleteGameObject, nsMessage);

  /// \brief If set to true, any parent/ancestor that has no other children or components will also be deleted.
  bool m_bDeleteEmptyParents = true;

  /// \brief This is used by nsOnComponentFinishedAction to orchestrate when an object shall really be deleted.
  bool m_bCancel = false;
};
