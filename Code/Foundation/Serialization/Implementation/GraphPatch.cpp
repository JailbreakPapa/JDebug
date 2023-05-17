#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Serialization/RttiConverter.h>

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdGraphPatch);

wdGraphPatch::wdGraphPatch(const char* szType, wdUInt32 uiTypeVersion, PatchType type)
  : m_szType(szType)
  , m_uiTypeVersion(uiTypeVersion)
  , m_PatchType(type)
{
}

const char* wdGraphPatch::GetType() const
{
  return m_szType;
}

wdUInt32 wdGraphPatch::GetTypeVersion() const
{
  return m_uiTypeVersion;
}


wdGraphPatch::PatchType wdGraphPatch::GetPatchType() const
{
  return m_PatchType;
}

WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_GraphPatch);
