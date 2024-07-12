#include <Foundation/FoundationPCH.h>

#include <Foundation/System/Screen.h>

void nsScreen::PrintScreenInfo(const nsHybridArray<nsScreenInfo, 2>& screens, nsLogInterface* pLog /*= nsLog::GetThreadLocalLogSystem()*/)
{
  NS_LOG_BLOCK(pLog, "Screens");

  nsLog::Info(pLog, "Found {0} screens", screens.GetCount());

  for (const auto& screen : screens)
  {
    nsLog::Dev(pLog, "'{0}': Offset = ({1}, {2}), Resolution = ({3}, {4}){5}", screen.m_sDisplayName, screen.m_iOffsetX, screen.m_iOffsetY, screen.m_iResolutionX, screen.m_iResolutionY, screen.m_bIsPrimary ? " (primary)" : "");
  }
}
