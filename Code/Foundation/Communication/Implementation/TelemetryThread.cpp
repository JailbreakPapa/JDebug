#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Threading/Thread.h>

class wdTelemetryThread : public wdThread
{
public:
  wdTelemetryThread()
    : wdThread("wdTelemetryThread")
  {
    m_bKeepRunning = true;
  }

  volatile bool m_bKeepRunning;

private:
  virtual wdUInt32 Run()
  {
    wdTime LastPing;

    while (m_bKeepRunning)
    {
      wdTelemetry::UpdateNetwork();

      // Send a Ping every once in a while
      if (wdTelemetry::s_ConnectionMode == wdTelemetry::Client)
      {
        wdTime tNow = wdTime::Now();

        if (tNow - LastPing > wdTime::Milliseconds(500))
        {
          LastPing = tNow;

          wdTelemetry::UpdateServerPing();
        }
      }

      wdThreadUtils::Sleep(wdTime::Milliseconds(10));
    }

    return 0;
  }
};

static wdTelemetryThread* g_pBroadcastThread = nullptr;
wdMutex wdTelemetry::s_TelemetryMutex;


wdMutex& wdTelemetry::GetTelemetryMutex()
{
  return s_TelemetryMutex;
}

void wdTelemetry::StartTelemetryThread()
{
  if (!g_pBroadcastThread)
  {
    g_pBroadcastThread = WD_DEFAULT_NEW(wdTelemetryThread);
    g_pBroadcastThread->Start();
  }
}

void wdTelemetry::StopTelemetryThread()
{
  if (g_pBroadcastThread)
  {
    g_pBroadcastThread->m_bKeepRunning = false;
    g_pBroadcastThread->Join();

    WD_DEFAULT_DELETE(g_pBroadcastThread);
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_TelemetryThread);
