#include <ToolsFoundation/ToolsFoundationDLL.h>

#include <ToolsFoundation/FileSystem/Declarations.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsFileStatus, nsNoBase, 3, nsRTTIDefaultAllocator<nsFileStatus>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("LastModified", m_LastModified),
    NS_MEMBER_PROPERTY("Hash", m_uiHash),
    NS_MEMBER_PROPERTY("DocumentID", m_DocumentID),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on
