#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Tracks/EventTrack.h>
#include <GuiFoundation/Widgets/EventTrackEditData.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdEventTrackControlPointData, 1, wdRTTIDefaultAllocator<wdEventTrackControlPointData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Tick", m_iTick),
    WD_ACCESSOR_PROPERTY("Event", GetEventName, SetEventName),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdEventTrackData, 3, wdRTTIDefaultAllocator<wdEventTrackData>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_ARRAY_MEMBER_PROPERTY("ControlPoints", m_ControlPoints),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void wdEventTrackControlPointData::SetTickFromTime(wdTime time, wdInt64 iFps)
{
  const wdUInt32 uiTicksPerStep = 4800 / iFps;
  m_iTick = (wdInt64)wdMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

wdInt64 wdEventTrackData::TickFromTime(wdTime time) const
{
  const wdUInt32 uiTicksPerStep = 4800 / m_uiFramesPerSecond;
  return (wdInt64)wdMath::RoundToMultiple(time.GetSeconds() * 4800.0, (double)uiTicksPerStep);
}

void wdEventTrackData::ConvertToRuntimeData(wdEventTrack& out_result) const
{
  out_result.Clear();

  for (const auto& cp : m_ControlPoints)
  {
    out_result.AddControlPoint(cp.GetTickAsTime(), cp.m_sEvent);
  }
}

void wdEventSet::AddAvailableEvent(const char* szEvent)
{
  if (wdStringUtils::IsNullOrEmpty(szEvent))
    return;

  if (m_AvailableEvents.Contains(szEvent))
    return;

  m_bModified = true;
  m_AvailableEvents.Insert(szEvent);
}

wdResult wdEventSet::WriteToDDL(const char* szFile)
{
  wdDeferredFileWriter file;
  file.SetOutput(szFile);

  wdOpenDdlWriter ddl;
  ddl.SetOutputStream(&file);

  for (const auto& s : m_AvailableEvents)
  {
    ddl.BeginObject("Event", s.GetData());
    ddl.EndObject();
  }

  if (file.Close().Succeeded())
  {
    m_bModified = false;
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdResult wdEventSet::ReadFromDDL(const char* szFile)
{
  m_AvailableEvents.Clear();

  wdFileReader file;
  if (file.Open(szFile).Failed())
    return WD_FAILURE;

  wdOpenDdlReader ddl;
  if (ddl.ParseDocument(file).Failed())
    return WD_FAILURE;

  auto* pRoot = ddl.GetRootElement();

  for (auto* pChild = pRoot->GetFirstChild(); pChild != nullptr; pChild = pChild->GetSibling())
  {
    if (pChild->IsCustomType("Event"))
    {
      AddAvailableEvent(pChild->GetName());
    }
  }

  m_bModified = false;
  return WD_SUCCESS;
}
