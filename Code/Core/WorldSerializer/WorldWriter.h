#pragma once

#include <Core/World/World.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Types/TagSet.h>

/// \brief Stores an entire nsWorld in a stream.
///
/// Used for exporting a world in binary form either as a level or as a prefab (though there is no
/// difference).
/// Can be used for saving a game, if the exact state of the world shall be stored (e.g. like in an FPS).
class NS_CORE_DLL nsWorldWriter
{
public:
  /// \brief Writes all content in \a world to \a stream.
  ///
  /// All game objects with tags that overlap with \a pExclude will be ignored.
  void WriteWorld(nsStreamWriter& inout_stream, nsWorld& ref_world, const nsTagSet* pExclude = nullptr);

  /// \brief Only writes the given root objects and all their children to the stream.
  void WriteObjects(nsStreamWriter& inout_stream, const nsDeque<const nsGameObject*>& rootObjects);

  /// \brief Only writes the given root objects and all their children to the stream.
  void WriteObjects(nsStreamWriter& inout_stream, nsArrayPtr<const nsGameObject*> rootObjects);

  /// \brief Writes the given game object handle to the stream.
  ///
  /// \note If the handle belongs to an object that is not part of the serialized scene, e.g. an object
  /// that was excluded by a tag, this function will assert.
  void WriteGameObjectHandle(const nsGameObjectHandle& hObject);

  /// \brief Writes the given component handle to the stream.
  ///
  /// \note If the handle belongs to a component that is not part of the serialized scene, e.g. an object
  /// that was excluded by a tag, this function will assert.
  void WriteComponentHandle(const nsComponentHandle& hComponent);

  /// \brief Accesses the stream to which data is written. Use this in component serialization functions
  /// to write data to the stream.
  nsStreamWriter& GetStream() const { return *m_pStream; }

  /// \brief Returns an array containing all game object pointers that were written to the stream as root objects
  const nsDeque<const nsGameObject*>& GetAllWrittenRootObjects() const { return m_AllRootObjects; }

  /// \brief Returns an array containing all game object pointers that were written to the stream as child objects
  const nsDeque<const nsGameObject*>& GetAllWrittenChildObjects() const { return m_AllChildObjects; }

private:
  void Clear();
  nsResult WriteToStream();
  void AssignGameObjectIndices();
  void AssignComponentHandleIndices(const nsMap<nsString, const nsRTTI*>& sortedTypes);
  void IncludeAllComponentBaseTypes();
  void IncludeAllComponentBaseTypes(const nsRTTI* pRtti);
  void Traverse(nsGameObject* pObject);

  nsVisitorExecution::Enum ObjectTraverser(nsGameObject* pObject);
  void WriteGameObject(const nsGameObject* pObject);
  void WriteComponentTypeInfo(const nsRTTI* pRtti);
  void WriteComponentCreationData(const nsDeque<const nsComponent*>& components);
  void WriteComponentSerializationData(const nsDeque<const nsComponent*>& components);

  nsStreamWriter* m_pStream = nullptr;
  const nsTagSet* m_pExclude = nullptr;

  nsDeque<const nsGameObject*> m_AllRootObjects;
  nsDeque<const nsGameObject*> m_AllChildObjects;
  nsMap<nsGameObjectHandle, nsUInt32> m_WrittenGameObjectHandles;

  struct Components
  {
    nsUInt16 m_uiSerializedTypeIndex = 0;
    nsDeque<const nsComponent*> m_Components;
    nsMap<nsComponentHandle, nsUInt32> m_HandleToIndex;
  };

  nsHashTable<const nsRTTI*, Components> m_AllComponents;
};
