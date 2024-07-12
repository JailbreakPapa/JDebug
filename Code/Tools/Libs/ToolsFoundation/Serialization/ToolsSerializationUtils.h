/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class nsDocumentObjectManager;
class nsDocumentObject;
class nsRTTI;

/// \brief Helper functions for serializing data
///
/// Also check out nsToolsReflectionUtils for related functionality.
class NS_TOOLSFOUNDATION_DLL nsToolsSerializationUtils
{
public:
  using FilterFunction = nsDelegate<bool(const nsAbstractProperty*)>;

  static void SerializeTypes(const nsSet<const nsRTTI*>& types, nsAbstractObjectGraph& ref_typesGraph);

  static void CopyProperties(const nsDocumentObject* pSource, const nsDocumentObjectManager* pSourceManager, void* pTarget, const nsRTTI* pTargetType, FilterFunction propertFilter = nullptr);
};
