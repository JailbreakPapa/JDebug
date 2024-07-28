#pragma once

#include <Core/World/World.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/UniquePtr.h>

class nsStringDeduplicationReadContext;
class nsProgress;
class nsProgressRange;

struct nsPrefabInstantiationOptions
{
  nsGameObjectHandle m_hParent;

  nsDynamicArray<nsGameObject*>* m_pCreatedRootObjectsOut = nullptr;
  nsDynamicArray<nsGameObject*>* m_pCreatedChildObjectsOut = nullptr;
  const nsUInt16* m_pOverrideTeamID = nullptr;

  bool m_bForceDynamic = false;

  /// \brief If the prefab has a single root node with this non-empty name, rather than creating a new object, instead the m_hParent object is used.
  nsTempHashedString m_ReplaceNamedRootWithParent;

  enum class RandomSeedMode
  {
    DeterministicFromParent, ///< nsWorld::CreateObject() will either derive a deterministic value from the parent object, or assign a random value, if no parent exists
    CompletelyRandom,        ///< nsWorld::CreateObject() will assign a random value to this object
    FixedFromSerialization,  ///< Keep deserialized random seed value
    CustomRootValue,         ///< Use the given seed root value to assign a deterministic (but different) value to each game object.
  };

  RandomSeedMode m_RandomSeedMode = RandomSeedMode::DeterministicFromParent;
  nsUInt32 m_uiCustomRandomSeedRootValue = 0;

  nsTime m_MaxStepTime = nsTime::MakeZero();

  nsProgress* m_pProgress = nullptr;
};

/// \brief Reads a world description from a stream. Allows to instantiate that world multiple times
///        in different locations and different nsWorld's.
///
/// The reader will ignore unknown component types and skip them during instantiation.
class NS_CORE_DLL nsWorldReader
{
public:
  /// \brief A context object is returned from InstantiateWorld or InstantiatePrefab if a maxStepTime greater than zero is specified.
  ///
  /// Call the Step() function periodically to complete the instantiation.
  /// Each step will try to spend not more than the given maxStepTime.
  /// E.g. this is useful if the instantiation cost of large prefabs needs to be distributed over multiple frames.
  class InstantiationContextBase
  {
  public:
    enum class StepResult
    {
      Continue,          ///< The available time slice is used up. Call Step() again to continue the process.
      ContinueNextFrame, ///< The process has reached a point where you need to call nsWorld::Update(). Otherwise no further progress can be made.
      Finished,          ///< The instantiation is finished and you can delete the context. Don't call 'Step()' on it again.
    };

    virtual ~InstantiationContextBase() = default;

    /// \Brief Advance the instantiation by one step
    /// \return Whether the operation is finished or needs to be repeated.
    virtual StepResult Step() = 0;

    /// \Brief Cancel the instantiation. This might lead to inconsistent states and must be used with care.
    virtual void Cancel() = 0;
  };

  nsWorldReader();
  ~nsWorldReader();

  /// \brief Reads all information about the world from the given stream.
  ///
  /// Call this once to populate nsWorldReader with information how to instantiate the world.
  /// Afterwards \a stream can be deleted.
  /// Call InstantiateWorld() or InstantiatePrefab() afterwards as often as you like
  /// to actually get an objects into an nsWorld.
  /// By default, the method will warn if it skips bytes in the stream that are of unknown
  /// types. The warnings can be suppressed by setting warningOnUnkownSkip to false.
  nsResult ReadWorldDescription(nsStreamReader& inout_stream, bool bWarningOnUnkownSkip = true);

  /// \brief Creates one instance of the world that was previously read by ReadWorldDescription().
  ///
  /// This is identical to calling InstantiatePrefab() with identity values, however, it is a bit
  /// more efficient, as unnecessary computations are skipped.
  ///
  /// If pOverrideTeamID is not null, every instantiated game object will get it passed in as its new value.
  /// This can be used to identify that the object belongs to a specific player or team.
  ///
  /// If maxStepTime is not zero the function will return a valid ptr to an InstantiationContextBase.
  /// This context will only spend the given amount of time in its Step() function.
  /// The function has to be periodically called until it returns true to complete the instantiation.
  ///
  /// If pProgress is a valid pointer it is used to track the progress of the instantiation. The nsProgress object
  /// has to be valid as long as the instantiation is in progress.
  nsUniquePtr<InstantiationContextBase> InstantiateWorld(nsWorld& ref_world, const nsUInt16* pOverrideTeamID = nullptr, nsTime maxStepTime = nsTime::MakeZero(), nsProgress* pProgress = nullptr);

  /// \brief Creates one instance of the world that was previously read by ReadWorldDescription().
  ///
  /// \param rootTransform is an additional transform that is applied to all root objects.
  /// \param hParent allows to attach the newly created objects immediately to a parent
  /// \param out_CreatedRootObjects If this is valid, all pointers the to created root objects are stored in this array
  ///
  /// If pOverrideTeamID is not null, every instantiated game object will get it passed in as its new value.
  /// This can be used to identify that the object belongs to a specific player or team.
  ///
  /// If maxStepTime is not zero the function will return a valid ptr to an InstantiationContextBase.
  /// This context will only spend the given amount of time in its Step() function.
  /// The function has to be periodically called until it returns true to complete the instantiation.
  ///
  /// If pProgress is a valid pointer it is used to track the progress of the instantiation. The nsProgress object
  /// has to be valid as long as the instantiation is in progress.
  nsUniquePtr<InstantiationContextBase> InstantiatePrefab(nsWorld& ref_world, const nsTransform& rootTransform, const nsPrefabInstantiationOptions& options);

  /// \brief Gives access to the stream of data. Use this inside component deserialization functions to read data.
  nsStreamReader& GetStream() const { return *m_pStream; }

  /// \brief Used during component deserialization to read a handle to a game object.
  nsGameObjectHandle ReadGameObjectHandle();

  /// \brief Used during component deserialization to read a handle to a component.
  void ReadComponentHandle(nsComponentHandle& out_hComponent);

  /// \brief Used during component deserialization to query the actual version number with which the
  /// given component type was written. The version number is given through the NS_BEGIN_COMPONENT_TYPE
  /// macro. Whenever the serialization of a component changes, that number should be increased.
  nsUInt32 GetComponentTypeVersion(const nsRTTI* pRtti) const;

  /// \brief Returns whether world contains a component of given type.
  bool HasComponentOfType(const nsRTTI* pRtti) const;

  /// \brief Clears all data.
  void ClearAndCompact();

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const;

  using FindComponentTypeCallback = nsDelegate<const nsRTTI*(nsStringView sTypeName)>;

  /// \brief An optional callback to redirect the lookup of a component type name to an nsRTTI type.
  ///
  /// If specified, this is used by ALL world readers. The intention is to use this either for logging purposes,
  /// or to implement a whitelist or blacklist for specific component types.
  /// E.g. if the callback returns nullptr, the component type is 'unknown' and skipped by the world reader.
  /// Thus one can remove unwanted component types.
  /// Theoretically one could also redirect an old (or renamed) component type to a new one,
  /// given that their deserialization code is compatible.
  static FindComponentTypeCallback s_FindComponentTypeCallback;

  nsUInt32 GetRootObjectCount() const;
  nsUInt32 GetChildObjectCount() const;

  static void SetMaxStepTime(InstantiationContextBase* pContext, nsTime maxStepTime);
  static nsTime GetMaxStepTime(InstantiationContextBase* pContext);

private:
  struct GameObjectToCreate
  {
    nsGameObjectDesc m_Desc;
    nsString m_sGlobalKey;
    nsUInt32 m_uiParentHandleIdx;
  };

  void ReadGameObjectDesc(GameObjectToCreate& godesc);
  void ReadComponentTypeInfo(nsUInt32 uiComponentTypeIdx);
  void ReadComponentDataToMemStream(bool warningOnUnknownSkip = true);
  void ClearHandles();
  nsUniquePtr<InstantiationContextBase> Instantiate(nsWorld& world, bool bUseTransform, const nsTransform& rootTransform, const nsPrefabInstantiationOptions& options);

  nsStreamReader* m_pStream = nullptr;
  nsWorld* m_pWorld = nullptr;

  nsUInt8 m_uiVersion = 0;
  nsDynamicArray<nsGameObjectHandle> m_IndexToGameObjectHandle;

  nsDynamicArray<GameObjectToCreate> m_RootObjectsToCreate;
  nsDynamicArray<GameObjectToCreate> m_ChildObjectsToCreate;

  struct ComponentTypeInfo
  {
    const nsRTTI* m_pRtti = nullptr;
    nsDynamicArray<nsComponentHandle> m_ComponentIndexToHandle;
    nsUInt32 m_uiNumComponents = 0;
  };

  nsDynamicArray<ComponentTypeInfo> m_ComponentTypes;
  nsHashTable<const nsRTTI*, nsUInt32> m_ComponentTypeVersions;
  nsDefaultMemoryStreamStorage m_ComponentCreationStream;
  nsDefaultMemoryStreamStorage m_ComponentDataStream;
  nsUInt64 m_uiTotalNumComponents = 0;

  nsUniquePtr<nsStringDeduplicationReadContext> m_pStringDedupReadContext;

  class InstantiationContext : public InstantiationContextBase
  {
  public:
    InstantiationContext(nsWorldReader& ref_worldReader, bool bUseTransform, const nsTransform& rootTransform, const nsPrefabInstantiationOptions& options);
    ~InstantiationContext();

    virtual StepResult Step() override;
    virtual void Cancel() override;

    template <bool UseTransform>
    bool CreateGameObjects(const nsDynamicArray<GameObjectToCreate>& objects, nsGameObjectHandle hParent, nsDynamicArray<nsGameObject*>* out_pCreatedObjects, nsTime endTime);

    bool CreateComponents(nsTime endTime);
    bool DeserializeComponents(nsTime endTime);
    bool AddComponentsToBatch(nsTime endTime);

    void SetMaxStepTime(nsTime stepTime);
    nsTime GetMaxStepTime() const;

  private:
    void BeginNextProgressStep(nsStringView sName);
    void SetSubProgressCompletion(double fCompletion);

    friend class nsWorldReader;
    nsWorldReader& m_WorldReader;

    bool m_bUseTransform = false;
    nsTransform m_RootTransform;

    nsPrefabInstantiationOptions m_Options;

    nsComponentInitBatchHandle m_hComponentInitBatch;

    // Current state
    struct Phase
    {
      enum Enum
      {
        Invalid = -1,
        CreateRootObjects,
        CreateChildObjects,
        CreateComponents,
        DeserializeComponents,
        AddComponentsToBatch,
        InitComponents,

        Count
      };
    };

    Phase::Enum m_Phase = Phase::Invalid;
    nsUInt32 m_uiCurrentIndex = 0; // object or component
    nsUInt32 m_uiCurrentComponentTypeIndex = 0;
    nsUInt64 m_uiCurrentNumComponentsProcessed = 0;
    nsMemoryStreamReader m_CurrentReader;

    nsUniquePtr<nsProgressRange> m_pOverallProgressRange;
    nsUniquePtr<nsProgressRange> m_pSubProgressRange;
  };
};
