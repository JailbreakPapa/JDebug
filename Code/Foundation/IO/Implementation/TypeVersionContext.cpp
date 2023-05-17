#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>

static const wdTypeVersion s_uiTypeVersionContextVersion = 1;

WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdTypeVersionWriteContext)

wdTypeVersionWriteContext::wdTypeVersionWriteContext() = default;
wdTypeVersionWriteContext::~wdTypeVersionWriteContext() = default;

wdStreamWriter& wdTypeVersionWriteContext::Begin(wdStreamWriter& ref_originalStream)
{
  m_pOriginalStream = &ref_originalStream;

  WD_ASSERT_DEV(m_TempStreamStorage.GetStorageSize64() == 0, "Begin() can only be called once on a type version context.");
  m_TempStreamWriter.SetStorage(&m_TempStreamStorage);

  return m_TempStreamWriter;
}

wdResult wdTypeVersionWriteContext::End()
{
  WD_ASSERT_DEV(m_pOriginalStream != nullptr, "End() called before Begin()");

  WriteTypeVersions(*m_pOriginalStream);

  // Now append the original stream
  WD_SUCCEED_OR_RETURN(m_TempStreamStorage.CopyToStream(*m_pOriginalStream));

  return WD_SUCCESS;
}

void wdTypeVersionWriteContext::AddType(const wdRTTI* pRtti)
{
  if (m_KnownTypes.Insert(pRtti) == false)
  {
    if (const wdRTTI* pParentRtti = pRtti->GetParentType())
    {
      AddType(pParentRtti);
    }
  }
}

void wdTypeVersionWriteContext::WriteTypeVersions(wdStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_uiTypeVersionContextVersion);

  const wdUInt32 uiNumTypes = m_KnownTypes.GetCount();
  inout_stream << uiNumTypes;

  wdMap<wdString, const wdRTTI*> sortedTypes;
  for (auto pType : m_KnownTypes)
  {
    sortedTypes.Insert(pType->GetTypeName(), pType);
  }

  for (const auto& it : sortedTypes)
  {
    inout_stream << it.Key();
    inout_stream << it.Value()->GetTypeVersion();
  }
}

//////////////////////////////////////////////////////////////////////////

WD_IMPLEMENT_SERIALIZATION_CONTEXT(wdTypeVersionReadContext)

wdTypeVersionReadContext::wdTypeVersionReadContext(wdStreamReader& inout_stream)
{
  auto version = inout_stream.ReadVersion(s_uiTypeVersionContextVersion);

  wdUInt32 uiNumTypes = 0;
  inout_stream >> uiNumTypes;

  wdStringBuilder sTypeName;
  wdUInt32 uiTypeVersion;

  for (wdUInt32 i = 0; i < uiNumTypes; ++i)
  {
    inout_stream >> sTypeName;
    inout_stream >> uiTypeVersion;

    if (const wdRTTI* pType = wdRTTI::FindTypeByName(sTypeName))
    {
      m_TypeVersions.Insert(pType, uiTypeVersion);
    }
    else
    {
      wdLog::Warning("Ignoring unknown type '{}'", sTypeName);
    }
  }
}

wdTypeVersionReadContext::~wdTypeVersionReadContext() = default;

wdUInt32 wdTypeVersionReadContext::GetTypeVersion(const wdRTTI* pRtti) const
{
  wdUInt32 uiVersion = wdInvalidIndex;
  m_TypeVersions.TryGetValue(pRtti, uiVersion);

  return uiVersion;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_TypeVersionContext);
