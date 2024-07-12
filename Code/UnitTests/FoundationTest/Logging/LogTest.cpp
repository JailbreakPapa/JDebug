#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/HTMLWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Threading/Thread.h>
#include <TestFramework/Utilities/TestLogInterface.h>

NS_CREATE_SIMPLE_TEST_GROUP(Logging);

namespace
{

  class LogTestLogInterface : public nsLogInterface
  {
  public:
    virtual void HandleLogMessage(const nsLoggingEventData& le) override
    {
      switch (le.m_EventType)
      {
        case nsLogMsgType::Flush:
          m_Result.Append("[Flush]\n");
          return;
        case nsLogMsgType::BeginGroup:
          m_Result.Append(">", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::EndGroup:
          m_Result.Append("<", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::ErrorMsg:
          m_Result.Append("E:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::SeriousWarningMsg:
          m_Result.Append("SW:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::WarningMsg:
          m_Result.Append("W:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::SuccessMsg:
          m_Result.Append("S:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::InfoMsg:
          m_Result.Append("I:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::DevMsg:
          m_Result.Append("E:", le.m_sTag, " ", le.m_sText, "\n");
          break;
        case nsLogMsgType::DebugMsg:
          m_Result.Append("D:", le.m_sTag, " ", le.m_sText, "\n");
          break;

        default:
          NS_REPORT_FAILURE("Invalid msg type");
          break;
      }
    }

    nsStringBuilder m_Result;
  };

} // namespace

NS_CREATE_SIMPLE_TEST(Logging, Log)
{
  LogTestLogInterface log;
  LogTestLogInterface log2;
  nsLogSystemScope logScope(&log);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Output")
  {
    NS_LOG_BLOCK("Verse 1", "Portal: Still Alive");

    nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::All);

    nsLog::Success("{0}", "This was a triumph.");
    nsLog::Info("{0}", "I'm making a note here:");
    nsLog::Error("{0}", "Huge Success");
    nsLog::Info("{0}", "It's hard to overstate my satisfaction.");
    nsLog::Dev("{0}", "Aperture Science. We do what we must, because we can,");
    nsLog::Dev("{0}", "For the good of all of us, except the ones who are dead.");
    nsLog::Flush();
    nsLog::Flush(); // second flush should be ignored

    {
      NS_LOG_BLOCK("Verse 2");

      nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::DevMsg);

      nsLog::Dev("But there's no sense crying over every mistake.");
      nsLog::Debug("You just keep on trying 'till you run out of cake.");
      nsLog::Info("And the science gets done, and you make a neat gun");
      nsLog::Error("for the people who are still alive.");
    }

    {
      NS_LOG_BLOCK("Verse 3");

      nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::InfoMsg);

      nsLog::Info("I'm not even angry.");
      nsLog::Debug("I'm being so sincere right now.");
      nsLog::Dev("Even though you broke my heart and killed me.");
      nsLog::Info("And tore me to pieces,");
      nsLog::Dev("and threw every piece into a fire.");
      nsLog::Info("As they burned it hurt because I was so happy for you.");
      nsLog::Error("Now these points of data make a beautiful line");
      nsLog::Dev("and we're off the beta, we're releasing on time.");
      nsLog::Flush();
      nsLog::Flush();

      {
        NS_LOG_BLOCK("Verse 4");

        nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::SuccessMsg);

        nsLog::Info("So I'm glad I got burned,");
        nsLog::Debug("think of all the things we learned");
        nsLog::Debug("for the people who are still alive.");

        {
          nsLogSystemScope logScope2(&log2);
          NS_LOG_BLOCK("Interlude");
          nsLog::Info("Well here we are again. It's always such a pleasure.");
          nsLog::Error("Remember when you tried to kill me twice?");
        }

        {
          NS_LOG_BLOCK("Verse 5");

          nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::WarningMsg);

          nsLog::Debug("Go ahead and leave me.");
          nsLog::Info("I think I prefer to stay inside.");
          nsLog::Dev("Maybe you'll find someone else, to help you.");
          nsLog::Dev("Maybe Black Mesa.");
          nsLog::Info("That was a joke. Haha. Fat chance.");
          nsLog::Warning("Anyway, this cake is great.");
          nsLog::Success("It's so delicious and moist.");
          nsLog::Dev("Look at me still talking when there's science to do.");
          nsLog::Error("When I look up there it makes me glad I'm not you.");
          nsLog::Info("I've experiments to run,");
          nsLog::SeriousWarning("there is research to be done on the people who are still alive.");
        }
      }
    }
  }

  {
    NS_LOG_BLOCK("Verse 6", "Last One");

    nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::ErrorMsg);

    nsLog::Dev("And believe me I am still alive.");
    nsLog::Info("I'm doing science and I'm still alive.");
    nsLog::Success("I feel fantastic and I'm still alive.");
    nsLog::Warning("While you're dying I'll be still alive.");
    nsLog::Error("And when you're dead I will be, still alive.");
    nsLog::Debug("Still alive, still alive.");
  }

  /// \todo This test will fail if NS_COMPILE_FOR_DEVELOPMENT is disabled.
  /// We also currently don't test nsLog::Debug, because our build machines compile in release and then the text below would need to be
  /// different.

  const char* szResult = log.m_Result;
  const char* szExpected = "\
>Portal: Still Alive Verse 1\n\
S: This was a triumph.\n\
I: I'm making a note here:\n\
E: Huge Success\n\
I: It's hard to overstate my satisfaction.\n\
E: Aperture Science. We do what we must, because we can,\n\
E: For the good of all of us, except the ones who are dead.\n\
[Flush]\n\
> Verse 2\n\
E: But there's no sense crying over every mistake.\n\
I: And the science gets done, and you make a neat gun\n\
E: for the people who are still alive.\n\
< Verse 2\n\
> Verse 3\n\
I: I'm not even angry.\n\
I: And tore me to pieces,\n\
I: As they burned it hurt because I was so happy for you.\n\
E: Now these points of data make a beautiful line\n\
[Flush]\n\
> Verse 4\n\
> Verse 5\n\
W: Anyway, this cake is great.\n\
E: When I look up there it makes me glad I'm not you.\n\
SW: there is research to be done on the people who are still alive.\n\
< Verse 5\n\
< Verse 4\n\
< Verse 3\n\
<Portal: Still Alive Verse 1\n\
>Last One Verse 6\n\
E: And when you're dead I will be, still alive.\n\
<Last One Verse 6\n\
";

  NS_TEST_STRING(szResult, szExpected);

  const char* szResult2 = log2.m_Result;
  const char* szExpected2 = "\
> Interlude\n\
I: Well here we are again. It's always such a pleasure.\n\
E: Remember when you tried to kill me twice?\n\
< Interlude\n\
";

  NS_TEST_STRING(szResult2, szExpected2);
}

NS_CREATE_SIMPLE_TEST(Logging, GlobalTestLog)
{
  nsLog::GetThreadLocalLogSystem()->SetLogLevel(nsLogMsgType::All);

  {
    nsTestLogInterface log;
    nsTestLogSystemScope scope(&log, true);

    log.ExpectMessage("managed to break", nsLogMsgType::ErrorMsg);
    log.ExpectMessage("my heart", nsLogMsgType::WarningMsg);
    log.ExpectMessage("see you", nsLogMsgType::WarningMsg, 10);

    {
      class LogThread : public nsThread
      {
      public:
        virtual nsUInt32 Run() override
        {
          nsLog::Warning("I see you!");
          nsLog::Debug("Test debug");
          return 0;
        }
      };

      LogThread thread[10];

      for (nsUInt32 i = 0; i < 10; ++i)
      {
        thread[i].Start();
      }

      nsLog::Error("The only thing you managed to break so far");
      nsLog::Warning("is my heart");

      for (nsUInt32 i = 0; i < 10; ++i)
      {
        thread[i].Join();
      }
    }
  }
}
