#pragma once

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>

class wdDocumentObjectManager;
class wdDocumentObject;
class wdRTTI;

/// \brief Helper functions for serializing data
///
/// Also check out wdToolsReflectionUtils for related functionality.
class WD_TOOLSFOUNDATION_DLL wdToolsSerializationUtils
{
public:
  using FilterFunction = wdDelegate<bool(const wdAbstractProperty*)>;

  static void SerializeTypes(const wdSet<const wdRTTI*>& types, wdAbstractObjectGraph& ref_typesGraph);

  static void CopyProperties(const wdDocumentObject* pSource, const wdDocumentObjectManager* pSourceManager, void* pTarget, const wdRTTI* pTargetType, FilterFunction propertFilter = nullptr);
};
