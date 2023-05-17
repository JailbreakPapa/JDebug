#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Utilities/EnumerableClass.h>

class wdStreamWriter;
class wdStreamReader;
class wdVariantTypeInfo;

/// \brief Variant type registry allows for custom variant type infos to be accessed.
///
/// Custom variant types are defined via the WD_DECLARE_CUSTOM_VARIANT_TYPE and WD_DEFINE_CUSTOM_VARIANT_TYPE macros.
/// \sa WD_DECLARE_CUSTOM_VARIANT_TYPE, WD_DEFINE_CUSTOM_VARIANT_TYPE
class WD_FOUNDATION_DLL wdVariantTypeRegistry
{
  WD_DECLARE_SINGLETON(wdVariantTypeRegistry);

public:
  /// \brief Find the variant type info for the given wdRTTI type.
  /// \return wdVariantTypeInfo if one exits for the given type, otherwise nullptr.
  const wdVariantTypeInfo* FindVariantTypeInfo(const wdRTTI* pType) const;
  ~wdVariantTypeRegistry();

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, VariantTypeRegistry);
  wdVariantTypeRegistry();

  void PluginEventHandler(const wdPluginEvent& EventData);
  void UpdateTypes();

  wdHashTable<const wdRTTI*, const wdVariantTypeInfo*> m_TypeInfos;
};

/// \brief Defines functions to allow the full feature set of wdVariant to be used.
/// \sa WD_DEFINE_CUSTOM_VARIANT_TYPE, wdVariantTypeRegistry
class WD_FOUNDATION_DLL wdVariantTypeInfo : public wdEnumerable<wdVariantTypeInfo>
{
public:
  wdVariantTypeInfo();
  virtual const wdRTTI* GetType() const = 0;
  virtual wdUInt32 Hash(const void* pObject) const = 0;
  virtual bool Equal(const void* pObjectA, const void* pObjectB) const = 0;
  virtual void Serialize(wdStreamWriter& ref_writer, const void* pObject) const = 0;
  virtual void Deserialize(wdStreamReader& ref_reader, void* pObject) const = 0;

  WD_DECLARE_ENUMERABLE_CLASS(wdVariantTypeInfo);
};

/// \brief Helper template used by WD_DEFINE_CUSTOM_VARIANT_TYPE.
/// \sa WD_DEFINE_CUSTOM_VARIANT_TYPE
template <typename T>
class wdVariantTypeInfoT : public wdVariantTypeInfo
{
  const wdRTTI* GetType() const override
  {
    return wdGetStaticRTTI<T>();
  }
  wdUInt32 Hash(const void* pObject) const override
  {
    return wdHashHelper<T>::Hash(*static_cast<const T*>(pObject));
  }
  bool Equal(const void* pObjectA, const void* pObjectB) const override
  {
    return wdHashHelper<T>::Equal(*static_cast<const T*>(pObjectA), *static_cast<const T*>(pObjectB));
  }
  void Serialize(wdStreamWriter& writer, const void* pObject) const override
  {
    writer << *static_cast<const T*>(pObject);
  }
  void Deserialize(wdStreamReader& reader, void* pObject) const override
  {
    reader >> *static_cast<T*>(pObject);
  }
};

/// \brief Defines a custom variant type, allowing it to be serialized and compared. The type needs to be declared first before using this macro.
///
/// The given type must implement wdHashHelper and wdStreamWriter / wdStreamReader operators.
/// Macros should be placed in any cpp. Note that once a custom type is defined, it is considered a value type and will be passed by value. It must be linked into every editor and engine dll to allow serialization. Thus it should only be used for common types in base libraries.
/// Limitations: Currently only member variables are supported on custom types, no arrays, set, maps etc. For best performance, any custom type smaller than 16 bytes should be POD so it can be inlined into the wdVariant.
/// \sa WD_DECLARE_CUSTOM_VARIANT_TYPE, wdVariantTypeRegistry, wdVariant
#define WD_DEFINE_CUSTOM_VARIANT_TYPE(TYPE)                                                                                                                                       \
  WD_CHECK_AT_COMPILETIME_MSG(wdVariantTypeDeduction<TYPE>::value == wdVariantType::TypedObject, "WD_DECLARE_CUSTOM_VARIANT_TYPE needs to be added to the header defining TYPE"); \
  wdVariantTypeInfoT<TYPE> g_wdVariantTypeInfoT_##TYPE;
