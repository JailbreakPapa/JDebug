#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Profiling/ProfilingUtils.h>

nsResult nsProfilingUtils::SaveProfilingCapture(nsStringView sCapturePath)
{
  nsFileWriter fileWriter;
  if (fileWriter.Open(sCapturePath) == NS_SUCCESS)
  {
    nsProfilingSystem::ProfilingData profilingData;
    nsProfilingSystem::Capture(profilingData);
    // Set sort index to -1 so that the editor is always on top when opening the trace.
    profilingData.m_uiProcessSortIndex = -1;
    if (profilingData.Write(fileWriter).Failed())
    {
      nsLog::Error("Failed to write profiling capture: {0}.", sCapturePath);
      return NS_FAILURE;
    }

    nsLog::Info("Profiling capture saved to '{0}'.", fileWriter.GetFilePathAbsolute().GetData());
  }
  else
  {
    nsLog::Error("Could not write profiling capture to '{0}'.", fileWriter.GetFilePathAbsolute().GetData());
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}

nsResult nsProfilingUtils::MergeProfilingCaptures(nsStringView sCapturePath1, nsStringView sCapturePath2, nsStringView sMergedCapturePath)
{
  nsString sFirstProfilingJson;
  {
    nsFileReader reader;
    if (reader.Open(sCapturePath1).Failed())
    {
      nsLog::Error("Failed to read first profiling capture to be merged: {}.", sCapturePath1);
      return NS_FAILURE;
    }
    sFirstProfilingJson.ReadAll(reader);
  }
  nsString sSecondProfilingJson;
  {
    nsFileReader reader;
    if (reader.Open(sCapturePath2).Failed())
    {
      nsLog::Error("Failed to read second profiling capture to be merged: {}.", sCapturePath2);
      return NS_FAILURE;
    }
    sSecondProfilingJson.ReadAll(reader);
  }

  nsStringBuilder sMergedProfilingJson;
  {
    // Just glue the array together
    sMergedProfilingJson.Reserve(sFirstProfilingJson.GetElementCount() + 1 + sSecondProfilingJson.GetElementCount());
    const char* szEndArray = sFirstProfilingJson.FindLastSubString("]");
    sMergedProfilingJson.Append(nsStringView(sFirstProfilingJson.GetData(), static_cast<nsUInt32>(szEndArray - sFirstProfilingJson.GetData())));
    sMergedProfilingJson.Append(",");
    const char* szStartArray = sSecondProfilingJson.FindSubString("[") + 1;
    sMergedProfilingJson.Append(nsStringView(szStartArray, static_cast<nsUInt32>(sSecondProfilingJson.GetElementCount() - (szStartArray - sSecondProfilingJson.GetData()))));
  }

  nsFileWriter fileWriter;
  if (fileWriter.Open(sMergedCapturePath).Failed() || fileWriter.WriteBytes(sMergedProfilingJson.GetData(), sMergedProfilingJson.GetElementCount()).Failed())
  {
    nsLog::Error("Failed to write merged profiling capture: {}.", sMergedCapturePath);
    return NS_FAILURE;
  }
  nsLog::Info("Merged profiling capture saved to '{0}'.", fileWriter.GetFilePathAbsolute().GetData());
  return NS_SUCCESS;
}
