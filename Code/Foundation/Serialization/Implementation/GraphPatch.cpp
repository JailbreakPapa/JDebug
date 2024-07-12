#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Serialization/RttiConverter.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsGraphPatch);

nsGraphPatch::nsGraphPatch(const char* szType, nsUInt32 uiTypeVersion, PatchType type)
  : m_szType(szType)
  , m_uiTypeVersion(uiTypeVersion)
  , m_PatchType(type)
{
}

const char* nsGraphPatch::GetType() const
{
  return m_szType;
}

nsUInt32 nsGraphPatch::GetTypeVersion() const
{
  return m_uiTypeVersion;
}


nsGraphPatch::PatchType nsGraphPatch::GetPatchType() const
{
  return m_PatchType;
}
