
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>

class wdProcessingStreamProcessor;
class wdProcessingStreamGroup;

struct wdStreamGroupElementRemovedEvent
{
  wdProcessingStreamGroup* m_pStreamGroup;
  wdUInt64 m_uiElementIndex;
};

struct wdStreamGroupElementsClearedEvent
{
  wdProcessingStreamGroup* m_pStreamGroup;
};

/// \brief A stream group encapsulates the streams and the corresponding data processors.
class WD_FOUNDATION_DLL wdProcessingStreamGroup
{
public:
  /// \brief Constructor
  wdProcessingStreamGroup();

  /// \brief Destructor
  ~wdProcessingStreamGroup();

  void Clear();

  /// \brief Adds a stream processor to the stream group.
  /// Ownership is transferred to the stream group and the processor will be deallocated using the RTTI deallocator on destruction.
  /// Processors are executed in the order they are added to the stream group.
  void AddProcessor(wdProcessingStreamProcessor* pProcessor);

  /// \brief Removes the given stream processor from the group.
  void RemoveProcessor(wdProcessingStreamProcessor* pProcessor);

  /// \brief Removes all stream processors from the group.
  void ClearProcessors();

  /// \brief Adds a stream with the given name to the stream group. Adding a stream two times with the same name will return nullptr for the second
  /// attempt to signal an error.
  wdProcessingStream* AddStream(wdStringView sName, wdProcessingStream::DataType type);

  /// \brief Removes the stream with the given name, if it exists.
  void RemoveStreamByName(wdStringView sName);

  /// \brief Returns the stream by it's name, returns nullptr if not existent. More efficient since direct use of wdHashedString.
  wdProcessingStream* GetStreamByName(wdStringView sName) const;

  /// \brief Resizes all streams to contain storage for uiNumElements. Any pending remove and spawn operations will be reset!
  void SetSize(wdUInt64 uiNumElements);

  /// \brief Removes an element (e.g. due to the death of a particle etc.), this will be enqueued (and thus is safe to be called from within data
  /// processors).
  void RemoveElement(wdUInt64 uiElementIndex);

  /// \brief Spawns a number of new elements, they will be added as newly initialized stream elements. Safe to call from data processors since the
  /// spawning will be queued.
  void InitializeElements(wdUInt64 uiNumElements);

  /// \brief Runs the stream processors which have been added to the stream group.
  void Process();

  /// \brief Returns the number of elements the streams store.
  inline wdUInt64 GetNumElements() const { return m_uiNumElements; }

  /// \brief Returns the number of currently active elements.
  inline wdUInt64 GetNumActiveElements() const { return m_uiNumActiveElements; }

  /// \brief Returns the highest number of active elements since the last SetSize() call.
  inline wdUInt64 GetHighestNumActiveElements() const { return m_uiHighestNumActiveElements; }

  /// \brief Subscribe to this event to be informed when (shortly before) items are deleted.
  wdEvent<const wdStreamGroupElementRemovedEvent&> m_ElementRemovedEvent;

private:
  /// \brief Internal helper function which removes any pending elements and spawns new elements as needed
  void RunPendingDeletions();

  void EnsureStreamAssignmentValid();

  void RunPendingSpawns();

  void SortProcessorsByPriority();

  wdHybridArray<wdProcessingStreamProcessor*, 8> m_Processors;

  wdHybridArray<wdProcessingStream*, 8> m_DataStreams;

  wdHybridArray<wdUInt64, 64> m_PendingRemoveIndices;

  wdUInt64 m_uiPendingNumberOfElementsToSpawn;

  wdUInt64 m_uiNumElements;

  wdUInt64 m_uiNumActiveElements;

  wdUInt64 m_uiHighestNumActiveElements;

  bool m_bStreamAssignmentDirty;
};
