#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Utilities/EnumerableClass.h>

class nsStreamWriter;
class nsStreamReader;
class nsVariantTypeInfo;

/// \brief Variant type registry allows for custom variant type infos to be accessed.
///
/// Custom variant types are defined via the NS_DECLARE_CUSTOM_VARIANT_TYPE and NS_DEFINE_CUSTOM_VARIANT_TYPE macros.
/// \sa NS_DECLARE_CUSTOM_VARIANT_TYPE, NS_DEFINE_CUSTOM_VARIANT_TYPE
class NS_FOUNDATION_DLL nsVariantTypeRegistry
{
  NS_DECLARE_SINGLETON(nsVariantTypeRegistry);

public:
  /// \brief Find the variant type info for the given nsRTTI type.
  /// \return nsVariantTypeInfo if one exits for the given type, otherwise nullptr.
  const nsVariantTypeInfo* FindVariantTypeInfo(const nsRTTI* pType) const;
  ~nsVariantTypeRegistry();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, VariantTypeRegistry);
  nsVariantTypeRegistry();

  void PluginEventHandler(const nsPluginEvent& EventData);
  void UpdateTypes();

  nsHashTable<const nsRTTI*, const nsVariantTypeInfo*> m_TypeInfos;
};

/// \brief Defines functions to allow the full feature set of nsVariant to be used.
/// \sa NS_DEFINE_CUSTOM_VARIANT_TYPE, nsVariantTypeRegistry
class NS_FOUNDATION_DLL nsVariantTypeInfo : public nsEnumerable<nsVariantTypeInfo>
{
public:
  nsVariantTypeInfo();
  virtual const nsRTTI* GetType() const = 0;
  virtual nsUInt32 Hash(const void* pObject) const = 0;
  virtual bool Equal(const void* pObjectA, const void* pObjectB) const = 0;
  virtual void Serialize(nsStreamWriter& ref_writer, const void* pObject) const = 0;
  virtual void Deserialize(nsStreamReader& ref_reader, void* pObject) const = 0;

  NS_DECLARE_ENUMERABLE_CLASS(nsVariantTypeInfo);
};

/// \brief Helper template used by NS_DEFINE_CUSTOM_VARIANT_TYPE.
/// \sa NS_DEFINE_CUSTOM_VARIANT_TYPE
template <typename T>
class nsVariantTypeInfoT : public nsVariantTypeInfo
{
  const nsRTTI* GetType() const override
  {
    return nsGetStaticRTTI<T>();
  }
  nsUInt32 Hash(const void* pObject) const override
  {
    return nsHashHelper<T>::Hash(*static_cast<const T*>(pObject));
  }
  bool Equal(const void* pObjectA, const void* pObjectB) const override
  {
    return nsHashHelper<T>::Equal(*static_cast<const T*>(pObjectA), *static_cast<const T*>(pObjectB));
  }
  void Serialize(nsStreamWriter& writer, const void* pObject) const override
  {
    writer << *static_cast<const T*>(pObject);
  }
  void Deserialize(nsStreamReader& reader, void* pObject) const override
  {
    reader >> *static_cast<T*>(pObject);
  }
};

/// \brief Defines a custom variant type, allowing it to be serialized and compared. The type needs to be declared first before using this macro.
///
/// The given type must implement nsHashHelper and nsStreamWriter / nsStreamReader operators.
/// Macros should be placed in any cpp. Note that once a custom type is defined, it is considered a value type and will be passed by value. It must be linked into every editor and engine dll to allow serialization. Thus it should only be used for common types in base libraries.
/// Limitations: Currently only member variables are supported on custom types, no arrays, set, maps etc. For best performance, any custom type smaller than 16 bytes should be POD so it can be inlined into the nsVariant.
/// \sa NS_DECLARE_CUSTOM_VARIANT_TYPE, nsVariantTypeRegistry, nsVariant
#define NS_DEFINE_CUSTOM_VARIANT_TYPE(TYPE)                                                                                                                                       \
  NS_CHECK_AT_COMPILETIME_MSG(nsVariantTypeDeduction<TYPE>::value == nsVariantType::TypedObject, "NS_DECLARE_CUSTOM_VARIANT_TYPE needs to be added to the header defining TYPE"); \
  nsVariantTypeInfoT<TYPE> g_nsVariantTypeInfoT_##TYPE;
