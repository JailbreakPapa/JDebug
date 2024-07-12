#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/ThreadUtils.h>

namespace
{
  void WriteOutProfilingCapture(const char* szFilePath)
  {
    nsStringBuilder outputPath = nsTestFramework::GetInstance()->GetAbsOutputPath();
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(outputPath.GetData(), "test", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

    nsFileWriter fileWriter;
    if (fileWriter.Open(szFilePath) == NS_SUCCESS)
    {
      nsProfilingSystem::ProfilingData profilingData;
      nsProfilingSystem::Capture(profilingData);
      profilingData.Write(fileWriter).IgnoreResult();
      nsLog::Info("Profiling capture saved to '{0}'.", fileWriter.GetFilePathAbsolute().GetData());
    }
  }
} // namespace

NS_CREATE_SIMPLE_TEST_GROUP(Profiling);

NS_CREATE_SIMPLE_TEST(Profiling, Profiling)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Nested scopes")
  {
    nsProfilingSystem::Clear();

    {
      NS_PROFILE_SCOPE("Prewarm scope");
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));
    }

    nsTime endTime = nsTime::Now() + nsTime::MakeFromMilliseconds(1);

    {
      NS_PROFILE_SCOPE("Outer scope");

      {
        NS_PROFILE_SCOPE("Inner scope");

        while (nsTime::Now() < endTime)
        {
        }
      }
    }

    WriteOutProfilingCapture(":output/profilingScopes.json");
  }
}
