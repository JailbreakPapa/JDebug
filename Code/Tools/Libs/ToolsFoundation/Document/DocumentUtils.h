#pragma once

#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocumentObject;
struct nsDocumentTypeDescriptor;

class NS_TOOLSFOUNDATION_DLL nsDocumentUtils
{
public:
  static nsStatus IsValidSaveLocationForDocument(nsStringView sDocument, const nsDocumentTypeDescriptor** out_pTypeDesc = nullptr);
};
