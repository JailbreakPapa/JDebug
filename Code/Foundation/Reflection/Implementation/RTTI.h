#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>

// *****************************************
// ***** Runtime Type Information Data *****

struct nsRTTIAllocator;
class nsAbstractProperty;
class nsAbstractFunctionProperty;
class nsAbstractMessageHandler;
struct nsMessageSenderInfo;
class nsPropertyAttribute;
class nsMessage;
using nsMessageId = nsUInt16;

/// \brief This class holds information about reflected types. Each instance represents one type that is known to the reflection
/// system.
///
/// Instances of this class are typically created through the macros from the StaticRTTI.h header.
/// Each instance represents one type. This class holds information about derivation hierarchies and exposed properties. You can thus find
/// out whether a type is derived from some base class and what properties of which types are available. Properties can then be read and
/// modified on instances of this type.
class NS_FOUNDATION_DLL nsRTTI
{
public:
  /// \brief The constructor requires all the information about the type that this object represents.
  nsRTTI(nsStringView sName, const nsRTTI* pParentType, nsUInt32 uiTypeSize, nsUInt32 uiTypeVersion, nsUInt8 uiVariantType,
    nsBitflags<nsTypeFlags> flags, nsRTTIAllocator* pAllocator, nsArrayPtr<const nsAbstractProperty*> properties, nsArrayPtr<const nsAbstractFunctionProperty*> functions,
    nsArrayPtr<const nsPropertyAttribute*> attributes, nsArrayPtr<nsAbstractMessageHandler*> messageHandlers,
    nsArrayPtr<nsMessageSenderInfo> messageSenders, const nsRTTI* (*fnVerifyParent)());


  ~nsRTTI();

  /// \brief Can be called in debug builds to check that all reflected objects are correctly set up.
  void VerifyCorrectness() const;

  /// \brief Calls VerifyCorrectness() on all nsRTTI objects.
  static void VerifyCorrectnessForAllTypes();

  /// \brief Returns the name of this type.
  NS_ALWAYS_INLINE nsStringView GetTypeName() const { return m_sTypeName; } // [tested]

  /// \brief Returns the hash of the name of this type.
  NS_ALWAYS_INLINE nsUInt64 GetTypeNameHash() const { return m_uiTypeNameHash; } // [tested]

  /// \brief Returns the type that is the base class of this type. May be nullptr if this type has no base class.
  NS_ALWAYS_INLINE const nsRTTI* GetParentType() const { return m_pParentType; } // [tested]

  /// \brief Returns the corresponding variant type for this type or Invalid if there is none.
  NS_ALWAYS_INLINE nsVariantType::Enum GetVariantType() const { return static_cast<nsVariantType::Enum>(m_uiVariantType); }

  /// \brief Returns true if this type is derived from the given type (or of the same type).
  NS_ALWAYS_INLINE bool IsDerivedFrom(const nsRTTI* pBaseType) const // [tested]
  {
    const nsUInt32 thisGeneration = m_ParentHierarchy.GetCount();
    const nsUInt32 baseGeneration = pBaseType->m_ParentHierarchy.GetCount();
    NS_ASSERT_DEBUG(thisGeneration > 0 && baseGeneration > 0, "SetupParentHierarchy() has not been called");
    return thisGeneration >= baseGeneration && m_ParentHierarchy.GetData()[thisGeneration - baseGeneration] == pBaseType;
  }

  /// \brief Returns true if this type is derived from or identical to the given type.
  template <typename BASE>
  NS_ALWAYS_INLINE bool IsDerivedFrom() const // [tested]
  {
    return IsDerivedFrom(nsGetStaticRTTI<BASE>());
  }

  /// \brief Returns the object through which instances of this type can be allocated.
  NS_ALWAYS_INLINE nsRTTIAllocator* GetAllocator() const { return m_pAllocator; } // [tested]

  /// \brief Returns the array of properties that this type has. Does NOT include properties from base classes.
  NS_ALWAYS_INLINE nsArrayPtr<const nsAbstractProperty* const> GetProperties() const { return m_Properties; } // [tested]

  NS_ALWAYS_INLINE nsArrayPtr<const nsAbstractFunctionProperty* const> GetFunctions() const { return m_Functions; }

  NS_ALWAYS_INLINE nsArrayPtr<const nsPropertyAttribute* const> GetAttributes() const { return m_Attributes; }

  /// \brief Returns the first attribute that derives from the given type, or nullptr if nothing is found.
  template <typename Type>
  const Type* GetAttributeByType() const;

  /// \brief Returns the list of properties that this type has, including derived properties from all base classes.
  void GetAllProperties(nsDynamicArray<const nsAbstractProperty*>& out_properties) const; // [tested]

  /// \brief Returns the size (in bytes) of an instance of this type.
  NS_ALWAYS_INLINE nsUInt32 GetTypeSize() const { return m_uiTypeSize; } // [tested]

  /// \brief Returns the version number of this type.
  NS_ALWAYS_INLINE nsUInt32 GetTypeVersion() const { return m_uiTypeVersion; }

  /// \brief Returns the type flags.
  NS_ALWAYS_INLINE const nsBitflags<nsTypeFlags>& GetTypeFlags() const { return m_TypeFlags; } // [tested]

  /// \brief Searches all nsRTTI instances for the one with the given name, or nullptr if no such type exists.
  static const nsRTTI* FindTypeByName(nsStringView sName); // [tested]

  /// \brief Searches all nsRTTI instances for the one with the given hashed name, or nullptr if no such type exists.
  static const nsRTTI* FindTypeByNameHash(nsUInt64 uiNameHash); // [tested]
  static const nsRTTI* FindTypeByNameHash32(nsUInt32 uiNameHash);

  using PredicateFunc = nsDelegate<bool(const nsRTTI*), 48>;
  /// \brief Searches all nsRTTI instances for one where the given predicate function returns true
  static const nsRTTI* FindTypeIf(PredicateFunc func);

  /// \brief Will iterate over all properties of this type and (optionally) the base types to search for a property with the given name.
  const nsAbstractProperty* FindPropertyByName(nsStringView sName, bool bSearchBaseTypes = true) const; // [tested]

  /// \brief Returns the name of the plugin which this type is declared in.
  NS_ALWAYS_INLINE nsStringView GetPluginName() const { return m_sPluginName; } // [tested]

  /// \brief Returns the array of message handlers that this type has.
  NS_ALWAYS_INLINE const nsArrayPtr<nsAbstractMessageHandler*>& GetMessageHandlers() const { return m_MessageHandlers; }

  /// \brief Dispatches the given message to the proper message handler, if there is one available. Returns true if so, false if no message
  /// handler for this type exists.
  bool DispatchMessage(void* pInstance, nsMessage& ref_msg) const;

  /// \brief Dispatches the given message to the proper message handler, if there is one available. Returns true if so, false if no message
  /// handler for this type exists.
  bool DispatchMessage(const void* pInstance, nsMessage& ref_msg) const;

  /// \brief Returns whether this type can handle the given message type.
  template <typename MessageType>
  NS_ALWAYS_INLINE bool CanHandleMessage() const
  {
    return CanHandleMessage(MessageType::GetTypeMsgId());
  }

  /// \brief Returns whether this type can handle the message type with the given id.
  inline bool CanHandleMessage(nsMessageId id) const
  {
    NS_ASSERT_DEBUG(m_uiMsgIdOffset != nsSmallInvalidIndex, "Message handler table should have been gathered at this point.\n"
                                                            "If this assert is triggered for a type loaded from a dynamic plugin,\n"
                                                            "you may have forgotten to instantiate an nsPlugin object inside your plugin DLL.");

    const nsUInt32 uiIndex = id - m_uiMsgIdOffset;
    return uiIndex < m_DynamicMessageHandlers.GetCount() && m_DynamicMessageHandlers.GetData()[uiIndex] != nullptr;
  }

  NS_ALWAYS_INLINE const nsArrayPtr<nsMessageSenderInfo>& GetMessageSender() const { return m_MessageSenders; }

  struct ForEachOptions
  {
    using StorageType = nsUInt8;

    enum Enum
    {
      None = 0,
      ExcludeNonAllocatable = NS_BIT(0), ///< Excludes all types that cannot be allocated through nsRTTI. They may still be creatable through regular C++, though.
      ExcludeAbstract = NS_BIT(1),       ///< Excludes all types that are marked as 'abstract'. They may not be abstract in the C++ sense, though.
      ExcludeNotConcrete = ExcludeNonAllocatable | ExcludeAbstract,

      Default = None
    };

    struct Bits
    {
      nsUInt8 ExcludeNonAllocatable : 1;
      nsUInt8 ExcludeAbstract : 1;
    };
  };

  using VisitorFunc = nsDelegate<void(const nsRTTI*), 48>;
  static void ForEachType(VisitorFunc func, nsBitflags<ForEachOptions> options = ForEachOptions::Default); // [tested]

  static void ForEachDerivedType(const nsRTTI* pBaseType, VisitorFunc func, nsBitflags<ForEachOptions> options = ForEachOptions::Default);

  template <typename T>
  static NS_ALWAYS_INLINE void ForEachDerivedType(VisitorFunc func, nsBitflags<ForEachOptions> options = ForEachOptions::Default)
  {
    ForEachDerivedType(nsGetStaticRTTI<T>(), func, options);
  }

protected:
  nsStringView m_sPluginName;
  nsStringView m_sTypeName;
  nsArrayPtr<const nsAbstractProperty* const> m_Properties;
  nsArrayPtr<const nsAbstractFunctionProperty* const> m_Functions;
  nsArrayPtr<const nsPropertyAttribute* const> m_Attributes;
  void UpdateType(const nsRTTI* pParentType, nsUInt32 uiTypeSize, nsUInt32 uiTypeVersion, nsUInt8 uiVariantType, nsBitflags<nsTypeFlags> flags);
  void RegisterType();
  void UnregisterType();

  void GatherDynamicMessageHandlers();
  void SetupParentHierarchy();

  const nsRTTI* m_pParentType = nullptr;
  nsRTTIAllocator* m_pAllocator = nullptr;

  nsUInt32 m_uiTypeSize = 0;
  nsUInt32 m_uiTypeVersion = 0;
  nsUInt64 m_uiTypeNameHash = 0;
  nsUInt32 m_uiTypeIndex = 0;
  nsBitflags<nsTypeFlags> m_TypeFlags;
  nsUInt8 m_uiVariantType = 0;
  nsUInt16 m_uiMsgIdOffset = nsSmallInvalidIndex;

  const nsRTTI* (*m_VerifyParent)();

  nsArrayPtr<nsAbstractMessageHandler*> m_MessageHandlers;
  nsSmallArray<nsAbstractMessageHandler*, 1, nsStaticsAllocatorWrapper> m_DynamicMessageHandlers; // do not track this data, it won't be deallocated before shutdown

  nsArrayPtr<nsMessageSenderInfo> m_MessageSenders;
  nsSmallArray<const nsRTTI*, 7, nsStaticsAllocatorWrapper> m_ParentHierarchy;

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, Reflection);

  /// \brief Assigns the given plugin name to every nsRTTI instance that has no plugin assigned yet.
  static void AssignPlugin(nsStringView sPluginName);

  static void SanityCheckType(nsRTTI* pType);

  /// \brief Handles events by nsPlugin, to figure out which types were provided by which plugin
  static void PluginEventHandler(const nsPluginEvent& EventData);
};

NS_DECLARE_FLAGS_OPERATORS(nsRTTI::ForEachOptions);


// ***********************************
// ***** Object Allocator Struct *****


/// \brief The interface for an allocator that creates instances of reflected types.
struct NS_FOUNDATION_DLL nsRTTIAllocator
{
  virtual ~nsRTTIAllocator();

  /// \brief Returns whether the type that is represented by this allocator, can be dynamically allocated at runtime.
  virtual bool CanAllocate() const { return true; } // [tested]

  /// \brief Allocates one instance.
  template <typename T>
  nsInternal::NewInstance<T> Allocate(nsAllocator* pAllocator = nullptr)
  {
    return AllocateInternal(pAllocator).Cast<T>();
  }

  /// \brief Clones the given instance.
  template <typename T>
  nsInternal::NewInstance<T> Clone(const void* pObject, nsAllocator* pAllocator = nullptr)
  {
    return CloneInternal(pObject, pAllocator).Cast<T>();
  }

  /// \brief Deallocates the given instance.
  virtual void Deallocate(void* pObject, nsAllocator* pAllocator = nullptr) = 0; // [tested]

private:
  virtual nsInternal::NewInstance<void> AllocateInternal(nsAllocator* pAllocator) = 0;
  virtual nsInternal::NewInstance<void> CloneInternal(const void* pObject, nsAllocator* pAllocator)
  {
    NS_REPORT_FAILURE("Cloning is not supported by this allocator.");
    return nsInternal::NewInstance<void>(nullptr, pAllocator);
  }
};

/// \brief Dummy Allocator for types that should not be allocatable through the reflection system.
struct NS_FOUNDATION_DLL nsRTTINoAllocator : public nsRTTIAllocator
{
  /// \brief Returns false, because this type of allocator is used for classes that shall not be allocated dynamically.
  virtual bool CanAllocate() const override { return false; } // [tested]

  /// \brief Will trigger an assert.
  virtual nsInternal::NewInstance<void> AllocateInternal(nsAllocator* pAllocator) override // [tested]
  {
    NS_REPORT_FAILURE("This function should never be called.");
    return nsInternal::NewInstance<void>(nullptr, pAllocator);
  }

  /// \brief Will trigger an assert.
  virtual void Deallocate(void* pObject, nsAllocator* pAllocator) override // [tested]
  {
    NS_REPORT_FAILURE("This function should never be called.");
  }
};

/// \brief Default implementation of nsRTTIAllocator that allocates instances via the given allocator.
template <typename CLASS, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
struct nsRTTIDefaultAllocator : public nsRTTIAllocator
{
  /// \brief Returns a new instance that was allocated with the given allocator.
  virtual nsInternal::NewInstance<void> AllocateInternal(nsAllocator* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    return NS_NEW(pAllocator, CLASS);
  }

  /// \brief Clones the given instance with the given allocator.
  virtual nsInternal::NewInstance<void> CloneInternal(const void* pObject, nsAllocator* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    return CloneImpl(pObject, pAllocator, nsTraitInt<std::is_copy_constructible<CLASS>::value>());
  }

  /// \brief Deletes the given instance with the given allocator.
  virtual void Deallocate(void* pObject, nsAllocator* pAllocator) override // [tested]
  {
    if (pAllocator == nullptr)
    {
      pAllocator = AllocatorWrapper::GetAllocator();
    }

    CLASS* pPointer = static_cast<CLASS*>(pObject);
    NS_DELETE(pAllocator, pPointer);
  }

private:
  nsInternal::NewInstance<void> CloneImpl(const void* pObject, nsAllocator* pAllocator, nsTraitInt<0>)
  {
    NS_REPORT_FAILURE("Clone failed since the type is not copy constructible");
    return nsInternal::NewInstance<void>(nullptr, pAllocator);
  }

  nsInternal::NewInstance<void> CloneImpl(const void* pObject, nsAllocator* pAllocator, nsTraitInt<1>)
  {
    return NS_NEW(pAllocator, CLASS, *static_cast<const CLASS*>(pObject));
  }
};
