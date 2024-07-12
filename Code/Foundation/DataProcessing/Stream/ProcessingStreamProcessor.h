
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>

class nsProcessingStreamGroup;

/// \brief Base class for all stream processor implementations.
class NS_FOUNDATION_DLL nsProcessingStreamProcessor : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsProcessingStreamProcessor, nsReflectedClass);

public:
  /// \brief Base constructor
  nsProcessingStreamProcessor();

  /// \brief Base destructor.
  virtual ~nsProcessingStreamProcessor();

  /// Used for sorting processors, to ensure a certain order. Lower priority == executed first.
  float m_fPriority = 0.0f;

protected:
  friend class nsProcessingStreamGroup;

  /// \brief Internal method which needs to be implemented, gets the concrete stream bindings.
  /// This is called every time the streams are resized. Implementations should check that their required streams exist and are of the correct data
  /// types.
  virtual nsResult UpdateStreamBindings() = 0;

  /// \brief This method needs to be implemented in order to initialize new elements to specific values.
  virtual void InitializeElements(nsUInt64 uiStartIndex, nsUInt64 uiNumElements) = 0;

  /// \brief The actual method which processes the data, will be called with the number of elements to process.
  virtual void Process(nsUInt64 uiNumElements) = 0;

  /// \brief Back pointer to the stream group - will be set to the owner stream group when adding the stream processor to the group.
  /// Can be used to get stream pointers in UpdateStreamBindings();
  nsProcessingStreamGroup* m_pStreamGroup = nullptr;
};
