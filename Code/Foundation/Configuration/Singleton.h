#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>

#include <typeinfo>

/// \file


/// \brief nsSingletonRegistry knows about all singleton instances of classes that use NS_DECLARE_SINGLETON.
///
/// It allows to query for a specific interface implementation by type name only, which makes it possible to
/// get rid of unwanted library dependencies and use pure virtual interface classes, without singleton code
/// (and thus link dependencies).
///
/// See NS_DECLARE_SINGLETON and NS_DECLARE_SINGLETON_OF_INTERFACE for details.
class NS_FOUNDATION_DLL nsSingletonRegistry
{
public:
  struct SingletonEntry
  {
    nsString m_sName;
    void* m_pInstance = nullptr;
  };

  /// \todo Events for new/deleted singletons -> nsInspector integration

  /// \brief Retrieves a singleton instance by type name. Returns nullptr if no singleton instance is available.
  template <typename Interface>
  inline static Interface* GetSingletonInstance() // [tested]
  {
    return static_cast<Interface*>(s_Singletons.GetValueOrDefault(GetHash<Interface>(), {"", nullptr}).m_pInstance);
  }

  /// \brief Retrieves a singleton instance by type name. Asserts if no singleton instance is available.
  template <typename Interface>
  inline static Interface* GetRequiredSingletonInstance() // [tested]
  {
    auto value = GetSingletonInstance<Interface>();
    NS_ASSERT_ALWAYS(value, "No instance of singleton type \"{0}\" has been registered!", typeid(Interface).name());
    return value;
  }

  /// \brief Allows to inspect all known singletons
  static const nsMap<size_t, SingletonEntry>& GetAllRegisteredSingletons();

  /// \brief Registers a singleton instance under a given type name. This is automatically called by nsSingletonRegistrar.
  template <typename Interface>
  inline static void Register(Interface* pSingletonInstance) // [tested]
  {
    NS_ASSERT_DEV(pSingletonInstance != nullptr, "Invalid singleton instance pointer");
    NS_ASSERT_DEV(
      s_Singletons[GetHash<Interface>()].m_pInstance == nullptr, "Singleton for type '{0}' has already been registered", typeid(Interface).name());

    s_Singletons[GetHash<Interface>()] = {typeid(Interface).name(), pSingletonInstance};
  }

  /// \brief Unregisters a singleton instance. This is automatically called by nsSingletonRegistrar.
  template <typename Interface>
  inline static void Unregister() // [tested]
  {
    NS_ASSERT_DEV(
      s_Singletons[GetHash<Interface>()].m_pInstance != nullptr, "Singleton for type '{0}' is currently not registered", typeid(Interface).name());

    s_Singletons.Remove(GetHash<Interface>());
  }

private:
  template <typename>
  friend class nsSingletonRegistrar;

  template <typename Interface>
  inline static size_t GetHash()
  {
    static const size_t hash = typeid(Interface).hash_code();
    return hash;
  }

  static nsMap<size_t, SingletonEntry> s_Singletons;
};


/// \brief Insert this into a class declaration to turn the class into a singleton.
///
///        You can access the singleton instance in two ways.
///        By calling the static GetSingleton() function on the specific type.
///        By querying the instance through nsSingletonRegistry giving the class type as a string.
///        The latter allows to get the implementation of an interface that is only declared through a simple header
///        but was not linked against.
///
///        Use NS_DECLARE_SINGLETON for a typical singleton class.
///        Use NS_DECLARE_SINGLETON_OF_INTERFACE for a singleton class that implements a specific interface,
///        which is itself not declared as a singleton and thus does not support to get to the interface implementation
///        through GetSingleton(). This is necessary, if you want to decouple library link dependencies and thus not put
///        any singleton code into the interface declaration, to keep it a pure virtual interface.
///        You can then query that class pointer also through the name of the interface using nsSingletonRegistry.
#define NS_DECLARE_SINGLETON(self)                 \
public:                                            \
  NS_ALWAYS_INLINE static self* GetSingleton()     \
  {                                                \
    return s_pSingleton;                           \
  }                                                \
                                                   \
private:                                           \
  NS_DISALLOW_COPY_AND_ASSIGN(self);               \
  void RegisterSingleton()                         \
  {                                                \
    s_pSingleton = this;                           \
    nsSingletonRegistry::Register<self>(this);     \
  }                                                \
  static void UnregisterSingleton()                \
  {                                                \
    if (s_pSingleton)                              \
    {                                              \
      nsSingletonRegistry::Unregister<self>();     \
      s_pSingleton = nullptr;                      \
    }                                              \
  }                                                \
  friend class nsSingletonRegistrar<self>;         \
  nsSingletonRegistrar<self> m_SingletonRegistrar; \
  static self* s_pSingleton

/// \brief Insert this into a class declaration to turn the class into a singleton.
///
///        You can access the singleton instance in two ways.
///        By calling the static GetSingleton() function on the specific type.
///        By querying the instance through nsSingletonRegistry giving the class type as a string.
///        The latter allows to get the implementation of an interface that is only declared through a simple header
///        but was not linked against.
///
///        Use NS_DECLARE_SINGLETON for a typical singleton class.
///        Use NS_DECLARE_SINGLETON_OF_INTERFACE for a singleton class that implements a specific interface,
///        which is itself not declared as a singleton and thus does not support to get to the interface implementation
///        through GetSingleton(). This is necessary, if you want to decouple library link dependencies and thus not put
///        any singleton code into the interface declaration, to keep it a pure virtual interface.
///        You can then query that class pointer also through the name of the interface using nsSingletonRegistry.
#define NS_DECLARE_SINGLETON_OF_INTERFACE(self, interface) \
public:                                                    \
  NS_ALWAYS_INLINE static self* GetSingleton()             \
  {                                                        \
    return s_pSingleton;                                   \
  }                                                        \
                                                           \
private:                                                   \
  NS_DISALLOW_COPY_AND_ASSIGN(self);                       \
  void RegisterSingleton()                                 \
  {                                                        \
    s_pSingleton = this;                                   \
    nsSingletonRegistry::Register<self>(this);             \
    nsSingletonRegistry::Register<interface>(this);        \
  }                                                        \
  static void UnregisterSingleton()                        \
  {                                                        \
    if (s_pSingleton)                                      \
    {                                                      \
      nsSingletonRegistry::Unregister<interface>();        \
      nsSingletonRegistry::Unregister<self>();             \
      s_pSingleton = nullptr;                              \
    }                                                      \
  }                                                        \
  friend class nsSingletonRegistrar<self>;                 \
  nsSingletonRegistrar<self> m_SingletonRegistrar;         \
  static self* s_pSingleton


/// \brief Put this into the cpp of a singleton class
#define NS_IMPLEMENT_SINGLETON(self) self* self::s_pSingleton = nullptr



/// \brief [internal] Helper class to implement nsSingletonRegistry and NS_DECLARE_SINGLETON
///
/// Classes that use NS_DECLARE_SINGLETON must pass their this pointer to their m_SingletonRegistrar member
/// during construction.
template <class TYPE>
class nsSingletonRegistrar
{
public:
  NS_ALWAYS_INLINE nsSingletonRegistrar(TYPE* pType) // [tested]
  {
    pType->RegisterSingleton();
  }

  NS_ALWAYS_INLINE ~nsSingletonRegistrar() // [tested]
  {
    TYPE::UnregisterSingleton();
  }
};
