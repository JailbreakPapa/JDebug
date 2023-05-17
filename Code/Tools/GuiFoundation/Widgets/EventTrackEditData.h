#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class wdEventTrack;

class WD_GUIFOUNDATION_DLL wdEventTrackControlPointData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdEventTrackControlPointData, wdReflectedClass);

public:
  wdTime GetTickAsTime() const { return wdTime::Seconds(m_iTick / 4800.0); }
  void SetTickFromTime(wdTime time, wdInt64 iFps);
  const char* GetEventName() const { return m_sEvent.GetData(); }
  void SetEventName(const char* szSz) { m_sEvent.Assign(szSz); }

  wdInt64 m_iTick; // 4800 ticks per second
  wdHashedString m_sEvent;
};

class WD_GUIFOUNDATION_DLL wdEventTrackData : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdEventTrackData, wdReflectedClass);

public:
  wdInt64 TickFromTime(wdTime time) const;
  void ConvertToRuntimeData(wdEventTrack& out_result) const;

  wdUInt16 m_uiFramesPerSecond = 60;
  wdDynamicArray<wdEventTrackControlPointData> m_ControlPoints;
};

class WD_GUIFOUNDATION_DLL wdEventSet
{
public:
  bool IsModified() const { return m_bModified; }

  const wdSet<wdString>& GetAvailableEvents() const { return m_AvailableEvents; }

  void AddAvailableEvent(const char* szEvent);

  wdResult WriteToDDL(const char* szFile);
  wdResult ReadFromDDL(const char* szFile);

private:
  bool m_bModified = false;
  wdSet<wdString> m_AvailableEvents;
};
