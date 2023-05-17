#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/DocumentUtils.h>

wdStatus wdDocumentUtils::IsValidSaveLocationForDocument(const char* szDocument, const wdDocumentTypeDescriptor** out_pTypeDesc)
{
  const wdDocumentTypeDescriptor* pTypeDesc = nullptr;
  if (wdDocumentManager::FindDocumentTypeFromPath(szDocument, true, pTypeDesc).Failed())
  {
    wdStringBuilder sTemp;
    sTemp.Format("The selected file extension '{0}' is not registered with any known type.\nCannot create file '{1}'",
      wdPathUtils::GetFileExtension(szDocument), szDocument);
    return wdStatus(sTemp.GetData());
  }

  if (wdDocument* pDocument = pTypeDesc->m_pManager->GetDocumentByPath(szDocument))
  {
    return wdStatus("The selected document is already open. You need to close the document before you can re-create it.");
  }

  if (out_pTypeDesc)
  {
    *out_pTypeDesc = pTypeDesc;
  }
  return wdStatus(WD_SUCCESS);
}
