#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

BOOL CALLBACK wdMonitorEnumProc(HMONITOR pMonitor, HDC pHdcMonitor, LPRECT pLprcMonitor, LPARAM data)
{
  wdHybridArray<wdScreenInfo, 2>* pScreens = (wdHybridArray<wdScreenInfo, 2>*)data;

  MONITORINFOEXW info;
  info.cbSize = sizeof(info);

  if (!GetMonitorInfoW(pMonitor, &info))
    return TRUE;

  // In Windows screen coordinates are from top/left to bottom/right
  // ie. 0,0 is left/top , resx/resy is right/bottom

  auto& mon = pScreens->ExpandAndGetRef();
  mon.m_iOffsetX = info.rcMonitor.left;
  mon.m_iOffsetY = info.rcMonitor.top;
  mon.m_iResolutionX = info.rcMonitor.right - info.rcMonitor.left;
  mon.m_iResolutionY = info.rcMonitor.bottom - info.rcMonitor.top;
  mon.m_sDisplayName = info.szDevice;
  mon.m_bIsPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;

  DISPLAY_DEVICEW ddev;
  ddev.cb = sizeof(ddev);

  if (EnumDisplayDevicesW(info.szDevice, 0, &ddev, 1) == TRUE)
  {
    mon.m_sDisplayName = ddev.DeviceString;
  }

  return TRUE;
}

wdResult wdScreen::EnumerateScreens(wdHybridArray<wdScreenInfo, 2>& out_screens)
{
  out_screens.Clear();
  if (EnumDisplayMonitors(nullptr, nullptr, wdMonitorEnumProc, (LPARAM)&out_screens) == FALSE)
    return WD_FAILURE;

  if (out_screens.IsEmpty())
    return WD_FAILURE;

  return WD_SUCCESS;
}
