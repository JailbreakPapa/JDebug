#include <Foundation/FoundationPCH.h>

#include <Foundation/Basics.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamGroup.h>
#include <Foundation/DataProcessing/Stream/ProcessingStreamProcessor.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/MemoryUtils.h>

nsProcessingStreamGroup::nsProcessingStreamGroup()
{
  Clear();
}

nsProcessingStreamGroup::~nsProcessingStreamGroup()
{
  Clear();
}

void nsProcessingStreamGroup::Clear()
{
  ClearProcessors();

  m_uiPendingNumberOfElementsToSpawn = 0;
  m_uiNumElements = 0;
  m_uiNumActiveElements = 0;
  m_uiHighestNumActiveElements = 0;
  m_bStreamAssignmentDirty = true;

  for (nsProcessingStream* pStream : m_DataStreams)
  {
    NS_DEFAULT_DELETE(pStream);
  }

  m_DataStreams.Clear();
}

void nsProcessingStreamGroup::AddProcessor(nsProcessingStreamProcessor* pProcessor)
{
  NS_ASSERT_DEV(pProcessor != nullptr, "Stream processor may not be null!");

  if (pProcessor->m_pStreamGroup != nullptr)
  {
    nsLog::Debug("Stream processor is already assigned to a stream group!");
    return;
  }

  m_Processors.PushBack(pProcessor);

  pProcessor->m_pStreamGroup = this;

  m_bStreamAssignmentDirty = true;
}

void nsProcessingStreamGroup::RemoveProcessor(nsProcessingStreamProcessor* pProcessor)
{
  m_Processors.RemoveAndCopy(pProcessor);
  pProcessor->GetDynamicRTTI()->GetAllocator()->Deallocate(pProcessor);
}

void nsProcessingStreamGroup::ClearProcessors()
{
  m_bStreamAssignmentDirty = true;

  for (nsProcessingStreamProcessor* pProcessor : m_Processors)
  {
    pProcessor->GetDynamicRTTI()->GetAllocator()->Deallocate(pProcessor);
  }

  m_Processors.Clear();
}

nsProcessingStream* nsProcessingStreamGroup::AddStream(nsStringView sName, nsProcessingStream::DataType type)
{
  // Treat adding a stream two times as an error (return null)
  if (GetStreamByName(sName))
    return nullptr;

  nsHashedString Name;
  Name.Assign(sName);
  nsProcessingStream* pStream = NS_DEFAULT_NEW(nsProcessingStream, Name, type, nsProcessingStream::GetDataTypeSize(type), 16);

  m_DataStreams.PushBack(pStream);

  m_bStreamAssignmentDirty = true;

  return pStream;
}

void nsProcessingStreamGroup::RemoveStreamByName(nsStringView sName)
{
  nsHashedString Name;
  Name.Assign(sName);

  for (nsUInt32 i = 0; i < m_DataStreams.GetCount(); ++i)
  {
    if (m_DataStreams[i]->GetName() == Name)
    {
      NS_DEFAULT_DELETE(m_DataStreams[i]);
      m_DataStreams.RemoveAtAndSwap(i);

      m_bStreamAssignmentDirty = true;
      break;
    }
  }
}

nsProcessingStream* nsProcessingStreamGroup::GetStreamByName(nsStringView sName) const
{
  nsHashedString Name;
  Name.Assign(sName);

  for (nsProcessingStream* Stream : m_DataStreams)
  {
    if (Stream->GetName() == Name)
    {
      return Stream;
    }
  }

  return nullptr;
}

void nsProcessingStreamGroup::SetSize(nsUInt64 uiNumElements)
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
void nsProcessingStreamGroup::RemoveElement(nsUInt64 uiElementIndex)
{
  if (m_PendingRemoveIndices.Contains(uiElementIndex))
    return;

  NS_ASSERT_DEBUG(uiElementIndex < m_uiNumActiveElements, "Element which should be removed is outside of active element range!");

  m_PendingRemoveIndices.PushBack(uiElementIndex);
}

/// \brief Spawns a number of new elements, they will be added as newly initialized stream elements. Safe to call from data processors since the
/// spawning will be queued.
void nsProcessingStreamGroup::InitializeElements(nsUInt64 uiNumElements)
{
  m_uiPendingNumberOfElementsToSpawn += uiNumElements;
}

void nsProcessingStreamGroup::Process()
{
  EnsureStreamAssignmentValid();

  // TODO: Identify which processors work on which streams and find independent groups and use separate tasks for them?
  for (nsProcessingStreamProcessor* pStreamProcessor : m_Processors)
  {
    pStreamProcessor->Process(m_uiNumActiveElements);
  }

  // Run any pending deletions which happened due to stream processor execution
  RunPendingDeletions();

  // spawning here (instead of before processing) allows for particles to exist for exactly one frame
  // they will be created, initialized, then rendered, and the next Process() will already delete them
  RunPendingSpawns();
}


void nsProcessingStreamGroup::RunPendingDeletions()
{
  nsStreamGroupElementRemovedEvent e;
  e.m_pStreamGroup = this;

  // Remove elements
  while (!m_PendingRemoveIndices.IsEmpty())
  {
    if (m_uiNumActiveElements == 0)
      break;

    const nsUInt64 uiLastActiveElementIndex = m_uiNumActiveElements - 1;

    const nsUInt64 uiElementToRemove = m_PendingRemoveIndices.PeekBack();
    m_PendingRemoveIndices.PopBack();

    NS_ASSERT_DEBUG(uiElementToRemove < m_uiNumActiveElements, "Invalid index to remove");

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
    for (nsUInt32 i = 0; i < m_PendingRemoveIndices.GetCount(); ++i)
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
    for (nsProcessingStream* pStream : m_DataStreams)
    {
      const nsUInt64 uiStreamElementStride = pStream->GetElementStride();
      const nsUInt64 uiStreamElementSize = pStream->GetElementSize();
      const void* pSourceData = nsMemoryUtils::AddByteOffset(pStream->GetData(), static_cast<std::ptrdiff_t>(uiLastActiveElementIndex * uiStreamElementStride));
      void* pTargetData = nsMemoryUtils::AddByteOffset(pStream->GetWritableData(), static_cast<std::ptrdiff_t>(uiElementToRemove * uiStreamElementStride));

      nsMemoryUtils::Copy<nsUInt8>(static_cast<nsUInt8*>(pTargetData), static_cast<const nsUInt8*>(pSourceData), static_cast<size_t>(uiStreamElementSize));
    }

    // And decrease the size since we swapped the last element to the location of the element we just removed
    m_uiNumActiveElements--;
  }

  m_PendingRemoveIndices.Clear();
}

void nsProcessingStreamGroup::EnsureStreamAssignmentValid()
{
  // If any stream processors or streams were added we may need to inform them.
  if (m_bStreamAssignmentDirty)
  {
    SortProcessorsByPriority();

    // Set the new size on all stream.
    for (nsProcessingStream* Stream : m_DataStreams)
    {
      Stream->SetSize(m_uiNumElements);
    }

    for (nsProcessingStreamProcessor* pStreamProcessor : m_Processors)
    {
      pStreamProcessor->UpdateStreamBindings().IgnoreResult();
    }

    m_bStreamAssignmentDirty = false;
  }
}

void nsProcessingStreamGroup::RunPendingSpawns()
{
  // Check if elements need to be spawned. If this is the case spawn them. (This is limited by the maximum number of elements).
  if (m_uiPendingNumberOfElementsToSpawn > 0)
  {
    m_uiPendingNumberOfElementsToSpawn = nsMath::Min(m_uiPendingNumberOfElementsToSpawn, m_uiNumElements - m_uiNumActiveElements);

    if (m_uiPendingNumberOfElementsToSpawn)
    {
      for (nsProcessingStreamProcessor* pSpawner : m_Processors)
      {
        pSpawner->InitializeElements(m_uiNumActiveElements, m_uiPendingNumberOfElementsToSpawn);
      }
    }

    m_uiNumActiveElements += m_uiPendingNumberOfElementsToSpawn;

    m_uiHighestNumActiveElements = nsMath::Max(m_uiNumActiveElements, m_uiHighestNumActiveElements);

    m_uiPendingNumberOfElementsToSpawn = 0;
  }
}

struct ProcessorComparer
{
  NS_ALWAYS_INLINE bool Less(const nsProcessingStreamProcessor* a, const nsProcessingStreamProcessor* b) const { return a->m_fPriority < b->m_fPriority; }
};

void nsProcessingStreamGroup::SortProcessorsByPriority()
{
  ProcessorComparer cmp;
  m_Processors.Sort(cmp);
}
