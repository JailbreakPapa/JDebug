#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Utilities/EnumerableClass.h>


// *****************************************
// ***** Runtime Type Information Data *****

struct wdRTTIAllocator;
class wdAbstractProperty;
class wdAbstractFunctionProperty;
class wdAbstractMessageHandler;
struct wdMessageSenderInfo;
class wdPropertyAttribute;
class wdMessage;
typedef wdUInt16 wdMessageId;

/// \brief This enumerable class holds information about reflected types. Each instance represents one type that is known to the reflection
/// system.
///
/// Instances of this class are typically created through the macros from the StaticRTTI.h header.
/// Each instance represents one type. This class holds information about derivation hierarchies and exposed properties. You can thus find
/// out whether a type is derived from some base class and what properties of which types are available. Properties can then be read and
/// modified on instances of this type.
class WD_FOUNDATION_DLL wdRTTI : public wdEnumerable<wdRTTI>
{
  WD_DECLARE_ENUMERABLE_CLASS(wdRTTI);

public:
  /// \brief The constructor requires all the information about the type that this object represents.
  wdRTTI(const char* szName, const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType,
    wdBitflags<wdTypeFlags> flags, wdRTTIAllocator* pAllocator, wdArrayPtr<wdAbstractProperty*> properties, wdArrayPtr<wdAbstractProperty*> functions,
    wdArrayPtr<wdPropertyAttribute*> attributes, wdArrayPtr<wdAbstractMessageHandler*> messageHandlers,
    wdArrayPtr<wdMessageSenderInfo> messageSenders, const wdRTTI* (*fnVerifyParent)());


  ~wdRTTI();

  /// \brief Can be called in debug builds to check that all reflected objects are correctly set up.
  void VerifyCorrectness() const;

  /// \brief Calls VerifyCorrectness() on all wdRTTI objects.
  static void VerifyCorrectnessForAllTypes();

  /// \brief Returns the name of this type.
  WD_ALWAYS_INLINE const char* GetTypeName() const { return m_szTypeName; } // [tested]

  /// \brief Returns the hash of the name of this type.
  WD_ALWAYS_INLINE wdUInt64 GetTypeNameHash() const { return m_uiTypeNameHash; } // [tested]

  /// \brief Returns the type that is the base class of this type. May be nullptr if this type has no base class.
  WD_ALWAYS_INLINE const wdRTTI* GetParentType() const { return m_pParentType; } // [tested]

  /// \brief Returns the corresponding variant type for this type or Invalid if there is none.
  WD_ALWAYS_INLINE wdVariantType::Enum GetVariantType() const { return static_cast<wdVariantType::Enum>(m_uiVariantType); }

  /// \brief Returns true if this type is derived from the given type (or of the same type).
  WD_ALWAYS_INLINE bool IsDerivedFrom(const wdRTTI* pBaseType) const // [tested]
  {
    const wdUInt32 thisGeneration = m_ParentHierarchy.GetCount();
    const wdUInt32 baseGeneration = pBaseType->m_ParentHierarchy.GetCount();
    WD_ASSERT_DEBUG(thisGeneration > 0 && baseGeneration > 0, "SetupParentHierarchy() has not been called");
    return thisGeneration >= baseGeneration && m_ParentHierarchy.GetData()[thisGeneration - baseGeneration] == pBaseType;
  }

  /// \brief Returns true if this type is derived from or identical to the given type.
  template <typename BASE>
  WD_ALWAYS_INLINE bool IsDerivedFrom() const // [tested]
  {
    return IsDerivedFrom(wdGetStaticRTTI<BASE>());
  }

  /// \brief Returns the object through which instances of this type can be allocated.
  WD_ALWAYS_INLINE wdRTTIAllocator* GetAllocator() const { return m_pAllocator; } // [tested]

  /// \brief Returns the array of properties that this type has. Does NOT include properties from base classes.
  WD_ALWAYS_INLINE const wdArrayPtr<wdAbstractProperty*>& GetProperties() const { return m_Properties; } // [tested]

  WD_ALWAYS_INLINE const wdArrayPtr<wdAbstractFunctionProperty*>& GetFunctions() const { return m_Functions; }

  WD_ALWAYS_INLINE const wdArrayPtr<wdPropertyAttribute*>& GetAttributes() const { return m_Attributes; }

  /// \brief Returns the first attribute that derives from the given type, or nullptr if nothing is found.
  template <typename Type>
  const Type* GetAttributeByType() const;

  /// \brief Returns the list of properties that this type has, including derived properties from all base classes.
  void GetAllProperties(wdHybridArray<wdAbstractProperty*, 32>& out_properties) const; // [tested]

  /// \brief Returns the size (in bytes) of an instance of this type.
  WD_ALWAYS_INLINE wdUInt32 GetTypeSize() const { return m_uiTypeSize; } // [tested]

  /// \brief Returns the version number of this type.
  WD_ALWAYS_INLINE wdUInt32 GetTypeVersion() const { return m_uiTypeVersion; }

  /// \brief Returns the type flags.
  WD_ALWAYS_INLINE const wdBitflags<wdTypeFlags>& GetTypeFlags() const { return m_TypeFlags; } // [tested]

  /// \brief Searches all wdRTTI instances for the one with the given name, or nullptr if no such type exists.
  static wdRTTI* FindTypeByName(const char* szName); // [tested]

  /// \brief Searches all wdRTTI instances for the one with the given hashed name, or nullptr if no such type exists.
  static wdRTTI* FindTypeByNameHash(wdUInt64 uiNameHash); // [tested]
  static wdRTTI* FindTypeByNameHash32(wdUInt32 uiNameHash);

  /// \brief Will iterate over all properties of this type and (optionally) the base types to search for a property with the given name.
  wdAbstractProperty* FindPropertyByName(const char* szName, bool bSearchBaseTypes = true) const; // [tested]

  /// \brief Returns the name of the plugin which this type is declared in.
  WD_ALWAYS_INLINE const char* GetPluginName() const { return m_szPluginName; } // [tested]

  /// \brief Returns the array of message handlers that this type has.
  WD_ALWAYS_INLINE const wdArrayPtr<wdAbstractMessageHandler*>& GetMessageHandlers() const { return m_MessageHandlers; }

  /// \brief Dispatches the given message to the proper message handler, if there is one available. Returns true if so, false if no message
  /// handler for this type exists.
  bool DispatchMessage(void* pInstance, wdMessage& ref_msg) const;

  /// \brief Dispatches the given message to the proper message handler, if there is one available. Returns true if so, false if no message
  /// handler for this type exists.
  bool DispatchMessage(const void* pInstance, wdMessage& ref_msg) const;

  /// \brief Returns whether this type can handle the given message type.
  template <typename MessageType>
  WD_ALWAYS_INLINE bool CanHandleMessage() const
  {
    return CanHandleMessage(MessageType::GetTypeMsgId());
  }

  /// \brief Returns whether this type can handle the message type with the given id.
  inline bool CanHandleMessage(wdMessageId id) const
  {
    WD_ASSERT_DEBUG(m_bGatheredDynamicMessageHandlers, "Message handler table should have been gathered at this point.\n"
                                                       "If this assert is triggered for a type loaded from a dynamic plugin,\n"
                                                       "you may have forgotten to instantiate an wdPlugin object inside your plugin DLL.");

    const wdUInt32 uiIndex = id - m_uiMsgIdOffset;
    return uiIndex < m_DynamicMessageHandlers.GetCount() && m_DynamicMessageHandlers[uiIndex] != nullptr;
  }

  WD_ALWAYS_INLINE const wdArrayPtr<wdMessageSenderInfo>& GetMessageSender() const { return m_MessageSenders; }

  /// \brief Writes all types derived from \a pBaseType to the provided array. Optionally sorts the array by type name to yield a stable result.
  ///
  /// Returns the provided array, such that the function can be used in a foreach loop right away.
  static const wdDynamicArray<const wdRTTI*>& GetAllTypesDerivedFrom(
    const wdRTTI* pBaseType, wdDynamicArray<const wdRTTI*>& out_derivedTypes, bool bSortByName);

protected:
  const char* m_szPluginName;
  const char* m_szTypeName;
  wdArrayPtr<wdAbstractProperty*> m_Properties;
  wdArrayPtr<wdAbstractFunctionProperty*> m_Functions;
  wdArrayPtr<wdPropertyAttribute*> m_Attributes;
  void UpdateType(const wdRTTI* pParentType, wdUInt32 uiTypeSize, wdUInt32 uiTypeVersion, wdUInt32 uiVariantType, wdBitflags<wdTypeFlags> flags);
  void RegisterType();
  void UnregisterType();

  void GatherDynamicMessageHandlers();
  void SetupParentHierarchy();

  const wdRTTI* m_pParentType;
  wdRTTIAllocator* m_pAllocator;

  wdUInt32 m_uiVariantType;
  wdUInt32 m_uiTypeSize;
  wdUInt32 m_uiTypeVersion = 0;
  wdUInt64 m_uiTypeNameHash = 0;
  wdBitflags<wdTypeFlags> m_TypeFlags;
  wdUInt32 m_uiMsgIdOffset;

  bool m_bGatheredDynamicMessageHandlers;
  const wdRTTI* (*m_VerifyParent)();

  wdArrayPtr<wdAbstractMessageHandler*> m_MessageHandlers;
  wdDynamicArray<wdAbstractMessageHandler*, wdStaticAllocatorWrapper>
    m_DynamicMessageHandlers; // do not track this data, it won't be deallocated before shutdown

  wdArrayPtr<wdMessageSenderInfo> m_MessageSenders;
  wdHybridArray<const wdRTTI*, 8> m_ParentHierarchy;

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, Reflection);

  /// \brief Assigns the given plugin name to every wdRTTI instance that has no plugin assigned yet.
  static void AssignPlugin(const char* szPluginName);

  static void SanityCheckType(wdRTTI* pType);

  /// \brief Handles events by wdPlugin, to figure out which types were provided by which plugin
  static void PluginEventHandler(const wdPluginEvent& EventData);
};


// ***********************************
// ***** Object Allocator Struct *****


/// \brief The interface for an allocator that creates instances of reflected types.
struct WD_FOUNDATION_DLL wdRTTIAllocator
{
  /// \brief Returns whether the type that is represented by this allocator, can be dynamically allocated at runtime.
  virtual bool CanAllocate() const { return true; } // [tested]

  /// \brief Allocates one instance.
  template <typename T>
  wdInternal::NewInstance<T> Allocate(wdAllocatorBase* pAllocator = nullptr)
  {
    return AllocateInternal(pAllocator).Cast<T>();
  }

  /// \brief Clones the given instance.
  template <typename T>
  wdInternal::NewInstance<T> Clone(const void* pObject, wdAllocatorBase* pAllocator = nullptr)
  {
    return CloneInternal(pObject, pAllocator).Cast<T>();
  }

  /// \brief Deallocates the given instance.
  virtual void Deallocate(void* pObject, wdAllocatorBase* pAllocator = nullptr) = 0; // [tested]

private:
  virtual wdInternal::NewInstance<void> AllocateInternal(wdAllocatorBase* pAllocator) = 0;
  virtual wdInternal::NewInstance<void> CloneInternal(const void* pObject, wdAllocatorBase* pAllocator)
  {
    WD_REPORT_FAILURE("Cloning is not supported by this allocator.");
    return wdInternal::NewInstance<void>(nullptr, pAllocator);
  }
};

/// \brief Dummy Allocator for types that should not be allocatable through the reflection system.
struct WD_FOUNDATION_DLL wdRTTINoAllocator : public wdRTTIAllocator
{
  /// \brief Returns false, because this type of allocator is used for classes that shall not be allocated dynamically.
  virtual bool CanAllocate() const override { return false; } // [tested]

  /// \brief Will trigger an assert.
  virtual wdInternal::NewInstance<void> AllocateInternal(wdAllocatorBase* pAllocator) override // [tested]
  {
    WD_REPORT_FAILURE("This function should never be called.");
    return wdInternal::NewInstance<void>(nullptr, pAllocator);
  }

  /// \brief Will trigger an assert.
  virtual void Deallocate(void* pObject, wdAllocatorBase* pAllocator) override // [tested]
  {
    WD_REPORT_FAILURE("This function should never be called.");
  }
};

/// \brief Default implementation of wdRTTIAllocator that allocates instances via the given allocator.
template <typename CLASS, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
struct wdRTTIDefaultAllocator : public wdRTTIAllocator
{
  /// \brief Returns a new instance that was allocated with the given allocator.
  virtual wdInternal::NewInstance<void> AllocateInternal(wdAllocatorBase* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    return WD_NEW(pAllocator, CLASS);
  }

  /// \brief Clones the given instance with the given allocator.
  virtual wdInternal::NewInstance<void> CloneInternal(const void* pObject, wdAllocatorBase* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    return CloneImpl(pObject, pAllocator, wdTraitInt<std::is_copy_constructible<CLASS>::value>());
  }

  /// \brief Deletes the given instance with the given allocator.
  virtual void Deallocate(void* pObject, wdAllocatorBase* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    CLASS* pPointer = static_cast<CLASS*>(pObject);
    WD_DELETE(pAllocator, pPointer);
  }

private:
  wdInternal::NewInstance<void> CloneImpl(const void* pObject, wdAllocatorBase* pAllocator, wdTraitInt<0>)
  {
    WD_REPORT_FAILURE("Clone failed since the type is not copy constructible");
    return wdInternal::NewInstance<void>(nullptr, pAllocator);
  }

  wdInternal::NewInstance<void> CloneImpl(const void* pObject, wdAllocatorBase* pAllocator, wdTraitInt<1>)
  {
    return WD_NEW(pAllocator, CLASS, *static_cast<const CLASS*>(pObject));
  }
};
