#include <Core/CorePCH.h>

#include <Core/Messages/ApplyOnlyToMessage.h>

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsMsgOnlyApplyToObject);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgOnlyApplyToObject, 1, nsRTTIDefaultAllocator<nsMsgOnlyApplyToObject>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Object", m_hObject),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


NS_STATICLINK_FILE(Core, Core_Messages_Implementation_ApplyOnlyToMessage);
