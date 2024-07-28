#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/Variant.h>

class nsStreamReader;
class nsStreamWriter;

/// \brief Flags for entries in nsBlackboard.
struct NS_CORE_DLL nsBlackboardEntryFlags
{
  using StorageType = nsUInt16;

  enum Enum
  {
    None = 0,
    Save = NS_BIT(0),          ///< Include the entry during serialization
    OnChangeEvent = NS_BIT(1), ///< Broadcast the 'ValueChanged' event when this entry's value is modified

    UserFlag0 = NS_BIT(7),
    UserFlag1 = NS_BIT(8),
    UserFlag2 = NS_BIT(9),
    UserFlag3 = NS_BIT(10),
    UserFlag4 = NS_BIT(11),
    UserFlag5 = NS_BIT(12),
    UserFlag6 = NS_BIT(13),
    UserFlag7 = NS_BIT(14),

    Invalid = NS_BIT(15),

    Default = None
  };

  struct Bits
  {
    StorageType Save : 1;
    StorageType OnChangeEvent : 1;
    StorageType Reserved : 5;
    StorageType UserFlag0 : 1;
    StorageType UserFlag1 : 1;
    StorageType UserFlag2 : 1;
    StorageType UserFlag3 : 1;
    StorageType UserFlag4 : 1;
    StorageType UserFlag5 : 1;
    StorageType UserFlag6 : 1;
    StorageType UserFlag7 : 1;
    StorageType Invalid : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsBlackboardEntryFlags);
NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsBlackboardEntryFlags);


/// \brief A blackboard is a key/value store that provides OnChange events to be informed when a value changes.
///
/// Blackboards are used to gather typically small pieces of data. Some systems write the data, other systems read it.
/// Through the blackboard, arbitrary systems can interact.
///
/// For example this is commonly used in game AI, where some system gathers interesting pieces of data about the environment,
/// and then NPCs might use that information to make decisions.
class NS_CORE_DLL nsBlackboard : public nsRefCounted
{
private:
  nsBlackboard(bool bIsGlobal);

public:
  ~nsBlackboard();

  bool IsGlobalBlackboard() const { return m_bIsGlobal; }

  /// \brief Factory method to create a new blackboard.
  ///
  /// Since blackboards use shared ownership we need to make sure that blackboards are created in nsCore.dll.
  /// Some compilers (MSVC) create local v-tables which can become stale if a blackboard was registered as global but the DLL
  /// which created the blackboard is already unloaded.
  ///
  /// See https://groups.google.com/g/microsoft.public.vc.language/c/atSh_2VSc2w/m/EgJ3r_7OzVUJ?pli=1
  static nsSharedPtr<nsBlackboard> Create(nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());

  /// \brief Factory method to get access to a globally registered blackboard.
  ///
  /// If a blackboard with that name was already created globally before, its reference is returned.
  /// Otherwise it will be created and permanently registered under that name.
  /// Global blackboards cannot be removed. Although you can change their name via "SetName()",
  /// the name under which they are registered globally will not change.
  ///
  /// If at some point you want to "remove" a global blackboard, instead call UnregisterAllEntries() to
  /// clear all its values.
  static nsSharedPtr<nsBlackboard> GetOrCreateGlobal(const nsHashedString& sBlackboardName, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());

  /// \brief Finds a global blackboard with the given name.
  static nsSharedPtr<nsBlackboard> FindGlobal(const nsTempHashedString& sBlackboardName);

  /// \brief Changes the name of the blackboard.
  ///
  /// \note For global blackboards this has no effect under which name they are found. A global blackboard continues to
  /// be found by the name under which it was originally registered.
  void SetName(nsStringView sName);
  const char* GetName() const { return m_sName; }
  const nsHashedString& GetNameHashed() const { return m_sName; }

  struct Entry
  {
    nsVariant m_Value;
    nsBitflags<nsBlackboardEntryFlags> m_Flags;

    /// The change counter is increased every time the entry's value changes.
    /// Read this and compare it to a previous known value, to detect whether the value was changed since the last check.
    nsUInt32 m_uiChangeCounter = 0;
  };

  struct EntryEvent
  {
    nsHashedString m_sName;
    nsVariant m_OldValue;
    const Entry* m_pEntry;
  };

  /// \brief Removes the named entry. Does nothing, if no such entry exists.
  void RemoveEntry(const nsHashedString& sName);

  ///  \brief Removes all entries.
  void RemoveAllEntries();

  /// \brief Returns whether an entry with the given name already exists.
  bool HasEntry(const nsTempHashedString& sName) const;

  /// \brief Sets the value of the named entry. If the entry doesn't exist, yet, it will be created with default flags.
  ///
  /// If the 'OnChangeEvent' flag is set for this entry, OnEntryEvent() will be broadcast.
  /// However, if the new value is no different to the old, no event will be broadcast.
  ///
  /// For new entries, no OnEntryEvent() is sent.
  ///
  /// For best efficiency, cache the entry name in an nsHashedString and use the other overload of this function.
  /// DO NOT RECREATE the nsHashedString every time, though.
  void SetEntryValue(nsStringView sName, const nsVariant& value);

  /// \brief Overload of SetEntryValue() that takes an nsHashedString rather than an nsStringView.
  ///
  /// Using this function is more efficient, if you access the blackboard often, but you must ensure
  /// to only create the nsHashedString once and cache it for reuse.
  /// Assigning a value to an nsHashedString is an expensive operation, so if you do not cache the string,
  /// prefer to use the other overload.
  void SetEntryValue(const nsHashedString& sName, const nsVariant& value);

  /// \brief Returns a pointer to the named entry, or nullptr if no such entry was registered.
  const Entry* GetEntry(const nsTempHashedString& sName) const;

  /// \brief Returns the flags of the named entry, or nsBlackboardEntryFlags::Invalid, if no such entry was registered.
  nsBitflags<nsBlackboardEntryFlags> GetEntryFlags(const nsTempHashedString& sName) const;

  /// \brief Sets the flags of an existing entry. Returns NS_FAILURE, if it wasn't created via SetEntryValue() or SetEntryValue() before.
  nsResult SetEntryFlags(const nsTempHashedString& sName, nsBitflags<nsBlackboardEntryFlags> flags);

  /// \brief Returns the value of the named entry, or the fallback nsVariant, if no such entry was registered.
  nsVariant GetEntryValue(const nsTempHashedString& sName, const nsVariant& fallback = nsVariant()) const;

  /// \brief Increments the value of the named entry. Returns the incremented value or an invalid variant if the entry does not exist or is not a number type.
  nsVariant IncrementEntryValue(const nsTempHashedString& sName);

  /// \brief Decrements the value of the named entry. Returns the decremented value or an invalid variant if the entry does not exist or is not a number type.
  nsVariant DecrementEntryValue(const nsTempHashedString& sName);

  /// \brief Grants read access to the entire map of entries.
  const nsHashTable<nsHashedString, Entry>& GetAllEntries() const { return m_Entries; }

  /// \brief Allows you to register to the OnEntryEvent. This is broadcast whenever an entry is modified that has the flag nsBlackboardEntryFlags::OnChangeEvent.
  const nsEvent<const EntryEvent&>& OnEntryEvent() const { return m_EntryEvents; }

  /// \brief This counter is increased every time an entry is added or removed (but not when it is modified).
  ///
  /// Comparing this value to a previous known value allows to quickly detect whether the set of entries has changed.
  nsUInt32 GetBlackboardChangeCounter() const { return m_uiBlackboardChangeCounter; }

  /// \brief This counter is increased every time any entry's value is modified.
  ///
  /// Comparing this value to a previous known value allows to quickly detect whether any entry has changed recently.
  nsUInt32 GetBlackboardEntryChangeCounter() const { return m_uiBlackboardEntryChangeCounter; }

  /// \brief Stores all entries that have the 'Save' flag in the stream.
  nsResult Serialize(nsStreamWriter& inout_stream) const;

  /// \brief Restores entries from the stream.
  ///
  /// If the blackboard already contains entries, the deserialized data is ADDED to the blackboard.
  /// If deserialized entries overlap with existing ones, the deserialized entries will overwrite the existing ones (both values and flags).
  nsResult Deserialize(nsStreamReader& inout_stream);

private:
  NS_ALLOW_PRIVATE_PROPERTIES(nsBlackboard);

  static nsBlackboard* Reflection_GetOrCreateGlobal(const nsHashedString& sName);
  static nsBlackboard* Reflection_FindGlobal(nsTempHashedString sName);
  void Reflection_SetEntryValue(nsStringView sName, const nsVariant& value);

  void ImplSetEntryValue(const nsHashedString& sName, Entry& entry, const nsVariant& value);

  bool m_bIsGlobal = false;
  nsHashedString m_sName;
  nsEvent<const EntryEvent&> m_EntryEvents;
  nsUInt32 m_uiBlackboardChangeCounter = 0;
  nsUInt32 m_uiBlackboardEntryChangeCounter = 0;
  nsHashTable<nsHashedString, Entry> m_Entries;

  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, Blackboard);
  static nsMutex s_GlobalBlackboardsMutex;
  static nsHashTable<nsHashedString, nsSharedPtr<nsBlackboard>> s_GlobalBlackboards;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsBlackboard);

//////////////////////////////////////////////////////////////////////////

struct NS_CORE_DLL nsBlackboardCondition
{
  nsHashedString m_sEntryName;
  double m_fComparisonValue = 0.0;
  nsEnum<nsComparisonOperator> m_Operator;

  bool IsConditionMet(const nsBlackboard& blackboard) const;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsBlackboardCondition);
