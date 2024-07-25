#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class nsEventTrack;

class NS_GUIFOUNDATION_DLL nsEventTrackControlPointData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEventTrackControlPointData, nsReflectedClass);

public:
  nsTime GetTickAsTime() const { return nsTime::MakeFromSeconds(m_iTick / 4800.0); }
  void SetTickFromTime(nsTime time, nsInt64 iFps);
  const char* GetEventName() const { return m_sEvent.GetData(); }
  void SetEventName(const char* szSz) { m_sEvent.Assign(szSz); }

  nsInt64 m_iTick; // 4800 ticks per second
  nsHashedString m_sEvent;
};

class NS_GUIFOUNDATION_DLL nsEventTrackData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEventTrackData, nsReflectedClass);

public:
  nsInt64 TickFromTime(nsTime time) const;
  void ConvertToRuntimeData(nsEventTrack& out_result) const;

  nsUInt16 m_uiFramesPerSecond = 60;
  nsDynamicArray<nsEventTrackControlPointData> m_ControlPoints;
};

class NS_GUIFOUNDATION_DLL nsEventSet
{
public:
  bool IsModified() const { return m_bModified; }

  const nsSet<nsString>& GetAvailableEvents() const { return m_AvailableEvents; }

  void AddAvailableEvent(nsStringView sEvent);

  nsResult WriteToDDL(const char* szFile);
  nsResult ReadFromDDL(const char* szFile);

private:
  bool m_bModified = false;
  nsSet<nsString> m_AvailableEvents;
};
