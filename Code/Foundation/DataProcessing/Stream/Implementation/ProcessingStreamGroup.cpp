#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/MemoryUtils.h>

wdProcessingStreamGroup::wdProcessingStreamGroup()
{
  Clear();
}

wdProcessingStreamGroup::~wdProcessingStreamGroup()
{
  Clear();
}

void wdProcessingStreamGroup::Clear()
{
  ClearProcessors();

  m_uiPendingNumberOfElementsToSpawn = 0;
  m_uiNumElements = 0;
  m_uiNumActiveElements = 0;
  m_uiHighestNumActiveElements = 0;
  m_bStreamAssignmentDirty = true;

  for (wdProcessingStream* pStream : m_DataStreams)
  {
    WD_DEFAULT_DELETE(pStream);
  }

  m_DataStreams.Clear();
}

void wdProcessingStreamGroup::AddProcessor(wdProcessingStreamProcessor* pProcessor)
{
  WD_ASSERT_DEV(pProcessor != nullptr, "Stream processor may not be null!");

  if (pProcessor->m_pStreamGroup != nullptr)
  {
    wdLog::Debug("Stream processor is already assigned to a stream group!");
    return;
  }

  m_Processors.PushBack(pProcessor);

  pProcessor->m_pStreamGroup = this;

  m_bStreamAssignmentDirty = true;
}

void wdProcessingStreamGroup::RemoveProcessor(wdProcessingStreamProcessor* pProcessor)
{
  m_Processors.RemoveAndCopy(pProcessor);
  pProcessor->GetDynamicRTTI()->GetAllocator()->Deallocate(pProcessor);
}

void wdProcessingStreamGroup::ClearProcessors()
{
  m_bStreamAssignmentDirty = true;

  for (wdProcessingStreamProcessor* pProcessor : m_Processors)
  {
    pProcessor->GetDynamicRTTI()->GetAllocator()->Deallocate(pProcessor);
  }

  m_Processors.Clear();
}

wdProcessingStream* wdProcessingStreamGroup::AddStream(wdStringView sName, wdProcessingStream::DataType type)
{
  // Treat adding a stream two times as an error (return null)
  if (GetStreamByName(sName))
    return nullptr;

  wdHashedString Name;
  Name.Assign(sName);
  wdProcessingStream* pStream = WD_DEFAULT_NEW(wdProcessingStream, Name, type, wdProcessingStream::GetDataTypeSize(type), 16);

  m_DataStreams.PushBack(pStream);

  m_bStreamAssignmentDirty = true;

  return pStream;
}

void wdProcessingStreamGroup::RemoveStreamByName(wdStringView sName)
{
  wdHashedString Name;
  Name.Assign(sName);

  for (wdUInt32 i = 0; i < m_DataStreams.GetCount(); ++i)
  {
    if (m_DataStreams[i]->GetName() == Name)
    {
      WD_DEFAULT_DELETE(m_DataStreams[i]);
      m_DataStreams.RemoveAtAndSwap(i);

      m_bStreamAssignmentDirty = true;
      break;
    }
  }
}

wdProcessingStream* wdProcessingStreamGroup::GetStreamByName(wdStringView sName) const
{
  wdHashedString Name;
  Name.Assign(sName);

  for (wdProcessingStream* Stream : m_DataStreams)
  {
    if (Stream->GetName() == Name)
    {
      return Stream;
    }
  }

  return nullptr;
}

void wdProcessingStreamGroup::SetSize(wdUInt64 uiNumElements)
{
  if (m_uiNumElements == uiNumElements)
    return;

  m_uiNumElements = uiNumElements;

  // Also reset any pending remove and spawn operations since they refer to the old size and content
  m_PendingRemoveIndices.Clear();
  m_uiPendingNumberOfElementsToSpawn = 0;

  m_uiHighestNumActiveElements = 0;

  // Stream processors etc. may have pointers to the stream data for some reason.
  m_bStreamAssignmentDirty = true;
}

/// \brief Removes an element (e.g. due to the death of a particle etc.), this will be enqueued (and thus is safe to be called from within data
/// processors).
void wdProcessingStreamGroup::RemoveElement(wdUInt64 uiElementIndex)
{
  if (m_PendingRemoveIndices.Contains(uiElementIndex))
    return;

  WD_ASSERT_DEBUG(uiElementIndex < m_uiNumActiveElements, "Element which should be removed is outside of active element range!");

  m_PendingRemoveIndices.PushBack(uiElementIndex);
}

/// \brief Spawns a number of new elements, they will be added as newly initialized stream elements. Safe to call from data processors since the
/// spawning will be queued.
void wdProcessingStreamGroup::InitializeElements(wdUInt64 uiNumElements)
{
  m_uiPendingNumberOfElementsToSpawn += uiNumElements;
}

void wdProcessingStreamGroup::Process()
{
  EnsureStreamAssignmentValid();

  // TODO: Identify which processors work on which streams and find independent groups and use separate tasks for them?
  for (wdProcessingStreamProcessor* pStreamProcessor : m_Processors)
  {
    pStreamProcessor->Process(m_uiNumActiveElements);
  }

  // Run any pending deletions which happened due to stream processor execution
  RunPendingDeletions();

  // spawning here (instead of before processing) allows for particles to exist for exactly one frame
  // they will be created, initialized, then rendered, and the next Process() will already delete them
  RunPendingSpawns();
}


void wdProcessingStreamGroup::RunPendingDeletions()
{
  wdStreamGroupElementRemovedEvent e;
  e.m_pStreamGroup = this;

  // Remove elements
  while (!m_PendingRemoveIndices.IsEmpty())
  {
    if (m_uiNumActiveElements == 0)
      break;

    const wdUInt64 uiLastActiveElementIndex = m_uiNumActiveElements - 1;

    const wdUInt64 uiElementToRemove = m_PendingRemoveIndices.PeekBack();
    m_PendingRemoveIndices.PopBack();

    WD_ASSERT_DEBUG(uiElementToRemove < m_uiNumActiveElements, "Invalid index to remove");

    // inform any interested party about the tragic death
    e.m_uiElementIndex = uiElementToRemove;
    m_ElementRemovedEvent.Broadcast(e);

    // If the element which should be removed is the last element we can just decrement the number of active elements
    // and no further work needs to be done
    if (uiElementToRemove == uiLastActiveElementIndex)
    {
      m_uiNumActiveElements--;
      continue;
    }

    // Since we swap with the last element we need to make sure that any pending removals of the (current) last element are updated
    // and point to the place where we moved the data to.
    for (wdUInt32 i = 0; i < m_PendingRemoveIndices.GetCount(); ++i)
    {
      // Is the pending remove in the array actually the last element we use to swap with? It's simply a matter of updating it to point to the new
      // index.
      if (m_PendingRemoveIndices[i] == uiLastActiveElementIndex)
      {
        m_PendingRemoveIndices[i] = uiElementToRemove;

        // We can break since the RemoveElement() operation takes care that each index can be in the array only once
        break;
      }
    }

    // Move the data
    for (wdProcessingStream* pStream : m_DataStreams)
    {
      const wdUInt64 uiStreamElementStride = pStream->GetElementStride();
      const wdUInt64 uiStreamElementSize = pStream->GetElementSize();
      const void* pSourceData = wdMemoryUtils::AddByteOffset(pStream->GetData(), static_cast<ptrdiff_t>(uiLastActiveElementIndex * uiStreamElementStride));
      void* pTargetData = wdMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<ptrdiff_t>(uiElementToRemove * uiStreamElementStride));

      wdMemoryUtils::Copy<wdUInt8>(static_cast<wdUInt8*>(pTargetData), static_cast<const wdUInt8*>(pSourceData), static_cast<size_t>(uiStreamElementSize));
    }

    // And decrease the size since we swapped the last element to the location of the element we just removed
    m_uiNumActiveElements--;
  }

  m_PendingRemoveIndices.Clear();
}

void wdProcessingStreamGroup::EnsureStreamAssignmentValid()
{
  // If any stream processors or streams were added we may need to inform them.
  if (m_bStreamAssignmentDirty)
  {
    SortProcessorsByPriority();

    // Set the new size on all stream.
    for (wdProcessingStream* Stream : m_DataStreams)
    {
      Stream->SetSize(m_uiNumElements);
    }

    for (wdProcessingStreamProcessor* pStreamProcessor : m_Processors)
    {
      pStreamProcessor->UpdateStreamBindings().IgnoreResult();
    }

    m_bStreamAssignmentDirty = false;
  }
}

void wdProcessingStreamGroup::RunPendingSpawns()
{
  // Check if elements need to be spawned. If this is the case spawn them. (This is limited by the maximum number of elements).
  if (m_uiPendingNumberOfElementsToSpawn > 0)
  {
    m_uiPendingNumberOfElementsToSpawn = wdMath::Min(m_uiPendingNumberOfElementsToSpawn, m_uiNumElements - m_uiNumActiveElements);

    if (m_uiPendingNumberOfElementsToSpawn)
    {
      for (wdProcessingStreamProcessor* pSpawner : m_Processors)
      {
        pSpawner->InitializeElements(m_uiNumActiveElements, m_uiPendingNumberOfElementsToSpawn);
      }
    }

    m_uiNumActiveElements += m_uiPendingNumberOfElementsToSpawn;

    m_uiHighestNumActiveElements = wdMath::Max(m_uiNumActiveElements, m_uiHighestNumActiveElements);

    m_uiPendingNumberOfElementsToSpawn = 0;
  }
}

struct ProcessorComparer
{
  WD_ALWAYS_INLINE bool Less(const wdProcessingStreamProcessor* a, const wdProcessingStreamProcessor* b) const { return a->m_fPriority < b->m_fPriority; }
};

void wdProcessingStreamGroup::SortProcessorsByPriority()
{
  ProcessorComparer cmp;
  m_Processors.Sort(cmp);
}

WD_STATICLINK_FILE(Foundation, Foundation_DataProcessing_Stream_Implementation_ProcessingStreamGroup);
