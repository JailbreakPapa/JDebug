
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>

class nsProcessingStream;

/// \brief This element spawner initializes new elements with 0 (by writing 0 bytes into the whole element)
class NS_FOUNDATION_DLL nsProcessingStreamSpawnerZeroInitialized : public nsProcessingStreamProcessor
{
  NS_ADD_DYNAMIC_REFLECTION(nsProcessingStreamSpawnerZeroInitialized, nsProcessingStreamProcessor);

public:
  nsProcessingStreamSpawnerZeroInitialized();

  /// \brief Which stream to zero initialize
  void SetStreamName(nsStringView sStreamName);

protected:
  virtual nsResult UpdateStreamBindings() override;

  virtual void InitializeElements(nsUInt64 uiStartIndex, nsUInt64 uiNumElements) override;
  virtual void Process(nsUInt64 uiNumElements) override {}

  nsHashedString m_sStreamName;

  nsProcessingStream* m_pStream = nullptr;
};
