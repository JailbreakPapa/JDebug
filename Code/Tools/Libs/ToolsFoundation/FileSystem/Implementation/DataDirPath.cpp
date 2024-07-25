#include <ToolsFoundation/ToolsFoundationDLL.h>

#include <ToolsFoundation/FileSystem/DataDirPath.h>

bool nsDataDirPath::UpdateDataDirInfos(nsArrayPtr<nsString> dataDirRoots, nsUInt32 uiLastKnownDataDirIndex /*= 0*/) const
{
  const nsUInt32 uiCount = dataDirRoots.GetCount();
  for (nsUInt32 i = 0; i < uiCount; ++i)
  {
    nsUInt32 uiCurrentIndex = (uiLastKnownDataDirIndex + i) % uiCount;
    NS_ASSERT_DEBUG(!dataDirRoots[uiCurrentIndex].EndsWith_NoCase("/"), "");
    if (m_sAbsolutePath.StartsWith_NoCase(dataDirRoots[uiCurrentIndex]) && !dataDirRoots[uiCurrentIndex].IsEmpty())
    {
      m_uiDataDirIndex = static_cast<nsUInt8>(uiCurrentIndex);
      const char* szParentFolder = nsPathUtils::FindPreviousSeparator(m_sAbsolutePath.GetData(), m_sAbsolutePath.GetData() + dataDirRoots[uiCurrentIndex].GetElementCount());
      m_uiDataDirParent = static_cast<nsUInt16>(szParentFolder - m_sAbsolutePath.GetData());
      m_uiDataDirLength = static_cast<nsUInt8>(dataDirRoots[uiCurrentIndex].GetElementCount() - m_uiDataDirParent);
      return true;
    }
  }

  m_uiDataDirParent = 0;
  m_uiDataDirLength = 0;
  m_uiDataDirIndex = 0;
  return false;
}
