
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>

class wdProcessingStream;

/// \brief This element spawner initializes new elements with 0 (by writing 0 bytes into the whole element)
class WD_FOUNDATION_DLL wdProcessingStreamSpawnerZeroInitialized : public wdProcessingStreamProcessor
{
  WD_ADD_DYNAMIC_REFLECTION(wdProcessingStreamSpawnerZeroInitialized, wdProcessingStreamProcessor);

public:
  wdProcessingStreamSpawnerZeroInitialized();

  /// \brief Which stream to zero initialize
  void SetStreamName(wdStringView sStreamName);

protected:
  virtual wdResult UpdateStreamBindings() override;

  virtual void InitializeElements(wdUInt64 uiStartIndex, wdUInt64 uiNumElements) override;
  virtual void Process(wdUInt64 uiNumElements) override {}

  wdHashedString m_sStreamName;

  wdProcessingStream* m_pStream;
};
