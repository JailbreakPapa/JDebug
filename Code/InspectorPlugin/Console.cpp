#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Core/Console/Console.h>
#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Configuration/CVar.h>

static void TelemetryMessage(void* pPassThrough)
{
  nsTelemetryMessage Msg;
  nsStringBuilder input;

  while (nsTelemetry::RetrieveMessage('CMD', Msg) == NS_SUCCESS)
  {
    if (Msg.GetMessageID() == 'EXEC' || Msg.GetMessageID() == 'COMP')
    {
      Msg.GetReader() >> input;

      if (nsConsole::GetMainConsole())
      {
        if (auto pInt = nsConsole::GetMainConsole()->GetCommandInterpreter())
        {
          nsCommandInterpreterState s;
          s.m_sInput = input;

          nsStringBuilder encoded;

          if (Msg.GetMessageID() == 'EXEC')
          {
            pInt->Interpret(s);
          }
          else
          {
            pInt->AutoComplete(s);
            encoded.AppendFormat(";;00||<{}", s.m_sInput);
          }

          for (const auto& l : s.m_sOutput)
          {
            encoded.AppendFormat(";;{}||{}", nsArgI((nsInt32)l.m_Type, 2, true), l.m_sText);
          }

          nsTelemetryMessage msg;
          msg.SetMessageID('CMD', 'RES');
          msg.GetWriter() << encoded;
          nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
        }
      }
    }
  }
}

void AddConsoleEventHandler()
{
  nsTelemetry::AcceptMessagesForSystem('CMD', true, TelemetryMessage, nullptr);
}

void RemoveConsoleEventHandler()
{
  nsTelemetry::AcceptMessagesForSystem('CMD', false);
}
