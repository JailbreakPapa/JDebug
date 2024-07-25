#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Tracks/EventTrack.h>
#include <GuiFoundation/Widgets/EventTrackEditData.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEventTrackControlPointData, 1, nsRTTIDefaultAllocator<nsEventTrackControlPointData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Tick", m_iTick),
    NS_ACCESSOR_PROPERTY("Event", GetEventName, SetEventName),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEventTrackData, 3, nsRTTIDefaultAllocator<nsEventTrackData>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_MEMBER_PROPERTY("ControlPoints", m_ControlPoints),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsEventTrackControlPointData::SetTickFromTime(nsTime time, nsInt64 iFps)
{
  const nsUInt32 uiTicksPerStep = 4800 / iFps;
  m_iTick = (nsInt64)nsMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

nsInt64 nsEventTrackData::TickFromTime(nsTime time) const
{
  const nsUInt32 uiTicksPerStep = 4800 / m_uiFramesPerSecond;
  return (nsInt64)nsMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

void nsEventTrackData::ConvertToRuntimeData(nsEventTrack& out_result) const
{
  out_result.Clear();

  for (const auto& cp : m_ControlPoints)
  {
    out_result.AddControlPoint(cp.GetTickAsTime(), cp.m_sEvent);
  }
}

void nsEventSet::AddAvailableEvent(nsStringView sEvent)
{
  if (sEvent.IsEmpty())
    return;

  if (m_AvailableEvents.Contains(sEvent))
    return;

  m_bModified = true;
  m_AvailableEvents.Insert(sEvent);
}

nsResult nsEventSet::WriteToDDL(const char* szFile)
{
  nsDeferredFileWriter file;
  file.SetOutput(szFile);

  nsOpenDdlWriter ddl;
  ddl.SetOutputStream(&file);

  for (const auto& s : m_AvailableEvents)
  {
    ddl.BeginObject("Event", s.GetData());
    ddl.EndObject();
  }

  if (file.Close().Succeeded())
  {
    m_bModified = false;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsResult nsEventSet::ReadFromDDL(const char* szFile)
{
  m_AvailableEvents.Clear();

  nsFileReader file;
  if (file.Open(szFile).Failed())
    return NS_FAILURE;

  nsOpenDdlReader ddl;
  if (ddl.ParseDocument(file).Failed())
    return NS_FAILURE;

  auto* pRoot = ddl.GetRootElement();

  for (auto* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
  {
    if (pChild->IsCustomType("Event"))
    {
      AddAvailableEvent(pChild->GetName());
    }
  }

  m_bModified = false;
  return NS_SUCCESS;
}
