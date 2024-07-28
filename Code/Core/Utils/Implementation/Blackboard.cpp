#include <Core/CorePCH.h>

#include <Core/Utils/Blackboard.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_BITFLAGS(nsBlackboardEntryFlags, 1)
  NS_BITFLAGS_CONSTANTS(nsBlackboardEntryFlags::Save, nsBlackboardEntryFlags::OnChangeEvent,
    nsBlackboardEntryFlags::UserFlag0, nsBlackboardEntryFlags::UserFlag1, nsBlackboardEntryFlags::UserFlag2, nsBlackboardEntryFlags::UserFlag3, nsBlackboardEntryFlags::UserFlag4, nsBlackboardEntryFlags::UserFlag5, nsBlackboardEntryFlags::UserFlag6, nsBlackboardEntryFlags::UserFlag7)
NS_END_STATIC_REFLECTED_BITFLAGS;
// clang-format on

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsBlackboard, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_GetOrCreateGlobal, In, "Name")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardNamesEnum"))),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_FindGlobal, In, "Name")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardNamesEnum"))),

    NS_SCRIPT_FUNCTION_PROPERTY(GetName),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_SetEntryValue, In, "Name", In, "Value")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardKeysEnum"))),
    NS_SCRIPT_FUNCTION_PROPERTY(GetEntryValue, In, "Name", In, "Fallback")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardKeysEnum"))),
    NS_SCRIPT_FUNCTION_PROPERTY(IncrementEntryValue, In, "Name")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardKeysEnum"))),
    NS_SCRIPT_FUNCTION_PROPERTY(DecrementEntryValue, In, "Name")->AddAttributes(new nsFunctionArgumentAttributes(0, new nsDynamicStringEnumAttribute("BlackboardKeysEnum"))),
    NS_SCRIPT_FUNCTION_PROPERTY(GetBlackboardChangeCounter),
    NS_SCRIPT_FUNCTION_PROPERTY(GetBlackboardEntryChangeCounter)
  }
  NS_END_FUNCTIONS;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_SUBSYSTEM_DECLARATION(Core, Blackboard)

  ON_CORESYSTEMS_SHUTDOWN
  {
    NS_LOCK(nsBlackboard::s_GlobalBlackboardsMutex);
    nsBlackboard::s_GlobalBlackboards.Clear();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

// static
nsMutex nsBlackboard::s_GlobalBlackboardsMutex;
nsHashTable<nsHashedString, nsSharedPtr<nsBlackboard>> nsBlackboard::s_GlobalBlackboards;

// static
nsSharedPtr<nsBlackboard> nsBlackboard::Create(nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
{
  return NS_NEW(pAllocator, nsBlackboard, false);
}

// static
nsSharedPtr<nsBlackboard> nsBlackboard::GetOrCreateGlobal(const nsHashedString& sBlackboardName, nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
{
  NS_LOCK(s_GlobalBlackboardsMutex);

  auto it = s_GlobalBlackboards.Find(sBlackboardName);

  if (it.IsValid())
  {
    return it.Value();
  }

  nsSharedPtr<nsBlackboard> pShrd = NS_NEW(pAllocator, nsBlackboard, true);
  pShrd->m_sName = sBlackboardName;
  s_GlobalBlackboards.Insert(sBlackboardName, pShrd);

  return pShrd;
}

// static
nsSharedPtr<nsBlackboard> nsBlackboard::FindGlobal(const nsTempHashedString& sBlackboardName)
{
  NS_LOCK(s_GlobalBlackboardsMutex);

  nsSharedPtr<nsBlackboard> pBlackboard;
  s_GlobalBlackboards.TryGetValue(sBlackboardName, pBlackboard);
  return pBlackboard;
}

nsBlackboard::nsBlackboard(bool bIsGlobal)
{
  m_bIsGlobal = bIsGlobal;
}

nsBlackboard::~nsBlackboard() = default;

void nsBlackboard::SetName(nsStringView sName)
{
  NS_LOCK(s_GlobalBlackboardsMutex);
  m_sName.Assign(sName);
}

void nsBlackboard::RemoveEntry(const nsHashedString& sName)
{
  if (m_Entries.Remove(sName))
  {
    ++m_uiBlackboardChangeCounter;
  }
}

void nsBlackboard::RemoveAllEntries()
{
  if (m_Entries.IsEmpty() == false)
  {
    ++m_uiBlackboardChangeCounter;
  }

  m_Entries.Clear();
}

void nsBlackboard::ImplSetEntryValue(const nsHashedString& sName, Entry& entry, const nsVariant& value)
{
  if (entry.m_Value != value)
  {
    ++m_uiBlackboardEntryChangeCounter;
    ++entry.m_uiChangeCounter;

    if (entry.m_Flags.IsSet(nsBlackboardEntryFlags::OnChangeEvent))
    {
      EntryEvent e;
      e.m_sName = sName;
      e.m_OldValue = entry.m_Value;
      e.m_pEntry = &entry;

      entry.m_Value = value;

      m_EntryEvents.Broadcast(e, 1); // limited recursion is allowed
    }
    else
    {
      entry.m_Value = value;
    }
  }
}

void nsBlackboard::SetEntryValue(nsStringView sName, const nsVariant& value)
{
  const nsTempHashedString sNameTH(sName);

  auto itEntry = m_Entries.Find(sNameTH);

  if (!itEntry.IsValid())
  {
    nsHashedString sNameHS;
    sNameHS.Assign(sName);
    m_Entries[sNameHS].m_Value = value;

    ++m_uiBlackboardChangeCounter;
  }
  else
  {
    ImplSetEntryValue(itEntry.Key(), itEntry.Value(), value);
  }
}

void nsBlackboard::SetEntryValue(const nsHashedString& sName, const nsVariant& value)
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
  {
    m_Entries[sName].m_Value = value;

    ++m_uiBlackboardChangeCounter;
  }
  else
  {
    ImplSetEntryValue(itEntry.Key(), itEntry.Value(), value);
  }
}

void nsBlackboard::Reflection_SetEntryValue(nsStringView sName, const nsVariant& value)
{
  SetEntryValue(sName, value);
}

bool nsBlackboard::HasEntry(const nsTempHashedString& sName) const
{
  return m_Entries.Find(sName).IsValid();
}

nsResult nsBlackboard::SetEntryFlags(const nsTempHashedString& sName, nsBitflags<nsBlackboardEntryFlags> flags)
{
  auto itEntry = m_Entries.Find(sName);
  if (!itEntry.IsValid())
    return NS_FAILURE;

  itEntry.Value().m_Flags = flags;
  return NS_SUCCESS;
}

const nsBlackboard::Entry* nsBlackboard::GetEntry(const nsTempHashedString& sName) const
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
    return nullptr;

  return &itEntry.Value();
}

nsVariant nsBlackboard::GetEntryValue(const nsTempHashedString& sName, const nsVariant& fallback /*= nsVariant()*/) const
{
  auto pEntry = m_Entries.GetValue(sName);
  return pEntry != nullptr ? pEntry->m_Value : fallback;
}

nsVariant nsBlackboard::IncrementEntryValue(const nsTempHashedString& sName)
{
  auto pEntry = m_Entries.GetValue(sName);
  if (pEntry != nullptr && pEntry->m_Value.IsNumber())
  {
    nsVariant one = nsVariant(1).ConvertTo(pEntry->m_Value.GetType());
    pEntry->m_Value = pEntry->m_Value + one;
    return pEntry->m_Value;
  }

  return nsVariant();
}

nsVariant nsBlackboard::DecrementEntryValue(const nsTempHashedString& sName)
{
  auto pEntry = m_Entries.GetValue(sName);
  if (pEntry != nullptr && pEntry->m_Value.IsNumber())
  {
    nsVariant one = nsVariant(1).ConvertTo(pEntry->m_Value.GetType());
    pEntry->m_Value = pEntry->m_Value - one;
    return pEntry->m_Value;
  }

  return nsVariant();
}

nsBitflags<nsBlackboardEntryFlags> nsBlackboard::GetEntryFlags(const nsTempHashedString& sName) const
{
  auto itEntry = m_Entries.Find(sName);

  if (!itEntry.IsValid())
  {
    return nsBlackboardEntryFlags::Invalid;
  }

  return itEntry.Value().m_Flags;
}

nsResult nsBlackboard::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(1);

  nsUInt32 uiEntries = 0;

  for (auto it : m_Entries)
  {
    if (it.Value().m_Flags.IsSet(nsBlackboardEntryFlags::Save))
    {
      ++uiEntries;
    }
  }

  inout_stream << uiEntries;

  for (auto it : m_Entries)
  {
    const Entry& e = it.Value();

    if (e.m_Flags.IsSet(nsBlackboardEntryFlags::Save))
    {
      inout_stream << it.Key();
      inout_stream << e.m_Flags;
      inout_stream << e.m_Value;
    }
  }

  return NS_SUCCESS;
}

nsResult nsBlackboard::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream.ReadVersion(1);

  nsUInt32 uiEntries = 0;
  inout_stream >> uiEntries;

  for (nsUInt32 e = 0; e < uiEntries; ++e)
  {
    nsHashedString name;
    inout_stream >> name;

    nsBitflags<nsBlackboardEntryFlags> flags;
    inout_stream >> flags;

    nsVariant value;
    inout_stream >> value;

    SetEntryValue(name, value);
    SetEntryFlags(name, flags).AssertSuccess();
  }

  return NS_SUCCESS;
}

// static
nsBlackboard* nsBlackboard::Reflection_GetOrCreateGlobal(const nsHashedString& sName)
{
  return GetOrCreateGlobal(sName).Borrow();
}

// static
nsBlackboard* nsBlackboard::Reflection_FindGlobal(nsTempHashedString sName)
{
  return FindGlobal(sName);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsBlackboardCondition, nsNoBase, 1, nsRTTIDefaultAllocator<nsBlackboardCondition>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("EntryName", m_sEntryName)->AddAttributes(new nsDynamicStringEnumAttribute("BlackboardKeysEnum")),
    NS_ENUM_MEMBER_PROPERTY("Operator", nsComparisonOperator, m_Operator),
    NS_MEMBER_PROPERTY("ComparisonValue", m_fComparisonValue),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

bool nsBlackboardCondition::IsConditionMet(const nsBlackboard& blackboard) const
{
  auto pEntry = blackboard.GetEntry(m_sEntryName);
  if (pEntry != nullptr && pEntry->m_Value.IsNumber())
  {
    double fEntryValue = pEntry->m_Value.ConvertTo<double>();
    return nsComparisonOperator::Compare(m_Operator, fEntryValue, m_fComparisonValue);
  }

  return false;
}

constexpr nsTypeVersion s_BlackboardConditionVersion = 1;

nsResult nsBlackboardCondition::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_BlackboardConditionVersion);

  inout_stream << m_sEntryName;
  inout_stream << m_Operator;
  inout_stream << m_fComparisonValue;
  return NS_SUCCESS;
}

nsResult nsBlackboardCondition::Deserialize(nsStreamReader& inout_stream)
{
  const nsTypeVersion uiVersion = inout_stream.ReadVersion(s_BlackboardConditionVersion);
  NS_IGNORE_UNUSED(uiVersion);

  inout_stream >> m_sEntryName;
  inout_stream >> m_Operator;
  inout_stream >> m_fComparisonValue;
  return NS_SUCCESS;
}

NS_STATICLINK_FILE(Core, Core_Utils_Implementation_Blackboard);
