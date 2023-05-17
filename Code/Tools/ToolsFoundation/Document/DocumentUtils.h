#pragma once

#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocumentObject;
struct wdDocumentTypeDescriptor;

class WD_TOOLSFOUNDATION_DLL wdDocumentUtils
{
public:
  static wdStatus IsValidSaveLocationForDocument(const char* szDocument, const wdDocumentTypeDescriptor** out_pTypeDesc = nullptr);
};
