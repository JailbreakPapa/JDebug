
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/DataProcessing/Stream/ProcessingStream.h>

class nsProcessingStreamProcessor;
class nsProcessingStreamGroup;

struct nsStreamGroupElementRemovedEvent
{
  nsProcessingStreamGroup* m_pStreamGroup;
  nsUInt64 m_uiElementIndex;
};

struct nsStreamGroupElementsClearedEvent
{
  nsProcessingStreamGroup* m_pStreamGroup;
};

/// \brief A stream group encapsulates the streams and the corresponding data processors.
class NS_FOUNDATION_DLL nsProcessingStreamGroup
{
public:
  /// \brief Constructor
  nsProcessingStreamGroup();

  /// \brief Destructor
  ~nsProcessingStreamGroup();

  void Clear();

  /// \brief Adds a stream processor to the stream group.
  /// Ownership is transferred to the stream group and the processor will be deallocated using the RTTI deallocator on destruction.
  /// Processors are executed in the order they are added to the stream group.
  void AddProcessor(nsProcessingStreamProcessor* pProcessor);

  /// \brief Removes the given stream processor from the group.
  void RemoveProcessor(nsProcessingStreamProcessor* pProcessor);

  /// \brief Removes all stream processors from the group.
  void ClearProcessors();

  /// \brief Adds a stream with the given name to the stream group. Adding a stream two times with the same name will return nullptr for the second
  /// attempt to signal an error.
  nsProcessingStream* AddStream(nsStringView sName, nsProcessingStream::DataType type);

  /// \brief Removes the stream with the given name, if it exists.
  void RemoveStreamByName(nsStringView sName);

  /// \brief Returns the stream by it's name, returns nullptr if not existent. More efficient since direct use of nsHashedString.
  nsProcessingStream* GetStreamByName(nsStringView sName) const;

  /// \brief Resizes all streams to contain storage for uiNumElements. Any pending remove and spawn operations will be reset!
  void SetSize(nsUInt64 uiNumElements);

  /// \brief Removes an element (e.g. due to the death of a particle etc.), this will be enqueued (and thus is safe to be called from within data
  /// processors).
  void RemoveElement(nsUInt64 uiElementIndex);

  /// \brief Spawns a number of new elements, they will be added as newly initialized stream elements. Safe to call from data processors since the
  /// spawning will be queued.
  void InitializeElements(nsUInt64 uiNumElements);

  /// \brief Runs the stream processors which have been added to the stream group.
  void Process();

  /// \brief Returns the number of elements the streams store.
  inline nsUInt64 GetNumElements() const { return m_uiNumElements; }

  /// \brief Returns the number of currently active elements.
  inline nsUInt64 GetNumActiveElements() const { return m_uiNumActiveElements; }

  /// \brief Returns the highest number of active elements since the last SetSize() call.
  inline nsUInt64 GetHighestNumActiveElements() const { return m_uiHighestNumActiveElements; }

  /// \brief Subscribe to this event to be informed when (shortly before) items are deleted.
  nsEvent<const nsStreamGroupElementRemovedEvent&> m_ElementRemovedEvent;

private:
  /// \brief Internal helper function which removes any pending elements and spawns new elements as needed
  void RunPendingDeletions();

  void EnsureStreamAssignmentValid();

  void RunPendingSpawns();

  void SortProcessorsByPriority();

  nsHybridArray<nsProcessingStreamProcessor*, 8> m_Processors;

  nsHybridArray<nsProcessingStream*, 8> m_DataStreams;

  nsHybridArray<nsUInt64, 64> m_PendingRemoveIndices;

  nsUInt64 m_uiPendingNumberOfElementsToSpawn;

  nsUInt64 m_uiNumElements;

  nsUInt64 m_uiNumActiveElements;

  nsUInt64 m_uiHighestNumActiveElements;

  bool m_bStreamAssignmentDirty;
};
