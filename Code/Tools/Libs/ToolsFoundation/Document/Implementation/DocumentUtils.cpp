#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/DocumentUtils.h>

nsStatus nsDocumentUtils::IsValidSaveLocationForDocument(nsStringView sDocument, const nsDocumentTypeDescriptor** out_pTypeDesc)
{
  const nsDocumentTypeDescriptor* pTypeDesc = nullptr;
  if (nsDocumentManager::FindDocumentTypeFromPath(sDocument, true, pTypeDesc).Failed())
  {
    nsStringBuilder sTemp;
    sTemp.SetFormat("The selected file extension '{0}' is not registered with any known type.\nCannot create file '{1}'",
      nsPathUtils::GetFileExtension(sDocument), sDocument);
    return nsStatus(sTemp.GetData());
  }

  if (nsDocument* pDocument = pTypeDesc->m_pManager->GetDocumentByPath(sDocument))
  {
    return nsStatus("The selected document is already open. You need to close the document before you can re-create it.");
  }

  if (out_pTypeDesc)
  {
    *out_pTypeDesc = pTypeDesc;
  }
  return nsStatus(NS_SUCCESS);
}
