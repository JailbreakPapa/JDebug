/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
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
