/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/DocumentUtils.h>

nsStatus nsDocumentUtils::IsValidSaveLocationForDocument(nsStringView sDocument, const nsDocumentTypeDescriptor** out_pTypeDesc)
{
  const nsDocumentTypeDescriptor* pTypeDesc = nullptr;
  if (nsDocumentManager::FindDocumentTypeFromPath(sDocument, true, pTypeDesc).Failed())
  {
    nsStringBuilder sTemp;
    sTemp.Format("The selected file extension '{0}' is not registered with any known type.\nCannot create file '{1}'",
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
