#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>

static constexpr nsTypeVersion s_uiTypeVersionContextVersion = 1;

NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsTypeVersionWriteContext)

nsTypeVersionWriteContext::nsTypeVersionWriteContext() = default;
nsTypeVersionWriteContext::~nsTypeVersionWriteContext() = default;

nsStreamWriter& nsTypeVersionWriteContext::Begin(nsStreamWriter& ref_originalStream)
{
  m_pOriginalStream = &ref_originalStream;

  NS_ASSERT_DEV(m_TempStreamStorage.GetStorageSize64() == 0, "Begin() can only be called once on a type version context.");
  m_TempStreamWriter.SetStorage(&m_TempStreamStorage);

  return m_TempStreamWriter;
}

nsResult nsTypeVersionWriteContext::End()
{
  NS_ASSERT_DEV(m_pOriginalStream != nullptr, "End() called before Begin()");

  WriteTypeVersions(*m_pOriginalStream);

  // Now append the original stream
  NS_SUCCEED_OR_RETURN(m_TempStreamStorage.CopyToStream(*m_pOriginalStream));

  return NS_SUCCESS;
}

void nsTypeVersionWriteContext::AddType(const nsRTTI* pRtti)
{
  if (m_KnownTypes.Insert(pRtti) == false)
  {
    if (const nsRTTI* pParentRtti = pRtti->GetParentType())
    {
      AddType(pParentRtti);
    }
  }
}

void nsTypeVersionWriteContext::WriteTypeVersions(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_uiTypeVersionContextVersion);

  const nsUInt32 uiNumTypes = m_KnownTypes.GetCount();
  inout_stream << uiNumTypes;

  nsMap<nsString, const nsRTTI*> sortedTypes;
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

NS_IMPLEMENT_SERIALIZATION_CONTEXT(nsTypeVersionReadContext)

nsTypeVersionReadContext::nsTypeVersionReadContext(nsStreamReader& inout_stream)
{
  auto version = inout_stream.ReadVersion(s_uiTypeVersionContextVersion);
  NS_IGNORE_UNUSED(version);

  nsUInt32 uiNumTypes = 0;
  inout_stream >> uiNumTypes;

  nsStringBuilder sTypeName;
  nsUInt32 uiTypeVersion;

  for (nsUInt32 i = 0; i < uiNumTypes; ++i)
  {
    inout_stream >> sTypeName;
    inout_stream >> uiTypeVersion;

    if (const nsRTTI* pType = nsRTTI::FindTypeByName(sTypeName))
    {
      m_TypeVersions.Insert(pType, uiTypeVersion);
    }
    else
    {
      nsLog::Warning("Ignoring unknown type '{}'", sTypeName);
    }
  }
}

nsTypeVersionReadContext::~nsTypeVersionReadContext() = default;

nsUInt32 nsTypeVersionReadContext::GetTypeVersion(const nsRTTI* pRtti) const
{
  nsUInt32 uiVersion = nsInvalidIndex;
  m_TypeVersions.TryGetValue(pRtti, uiVersion);

  return uiVersion;
}
