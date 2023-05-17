
#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>

class wdProcessingStreamGroup;

/// \brief Base class for all stream processor implementations.
class WD_FOUNDATION_DLL wdProcessingStreamProcessor : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdProcessingStreamProcessor, wdReflectedClass);

public:
  /// \brief Base constructor
  wdProcessingStreamProcessor();

  /// \brief Base destructor.
  virtual ~wdProcessingStreamProcessor();

  /// Used for sorting processors, to ensure a certain order. Lower priority == executed first.
  float m_fPriority = 0.0f;

protected:
  friend class wdProcessingStreamGroup;

  /// \brief Internal method which needs to be implemented, gets the concrete stream bindings.
  /// This is called every time the streams are resized. Implementations should check that their required streams exist and are of the correct data
  /// types.
  virtual wdResult UpdateStreamBindings() = 0;

  /// \brief This method needs to be implemented in order to initialize new elements to specific values.
  virtual void InitializeElements(wdUInt64 uiStartIndex, wdUInt64 uiNumElements) = 0;

  /// \brief The actual method which processes the data, will be called with the number of elements to process.
  virtual void Process(wdUInt64 uiNumElements) = 0;

  /// \brief Back pointer to the stream group - will be set to the owner stream group when adding the stream processor to the group.
  /// Can be used to get stream pointers in UpdateStreamBindings();
  wdProcessingStreamGroup* m_pStreamGroup;
};
