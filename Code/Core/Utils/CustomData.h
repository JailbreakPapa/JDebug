#pragma once

#include <Core/CoreDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/World/Declarations.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Reflection/Reflection.h>


/// \brief A base class for user-defined data assets.
///
/// Allows users to define their own asset types that can be created, edited and referenced in the editor without writing an editor plugin.
///
/// In order to do that, subclass nsCustomData,
/// and put the macro NS_DECLARE_CUSTOM_DATA_RESOURCE(YourCustomData) into the header next to your custom type.
/// Also put the macro NS_DEFINE_CUSTOM_DATA_RESOURCE(YourCustomData) into the implementation file.
///
/// Those will also define resource and resource handle types, such as YourCustomDataResource and YourCustomDataResourceHandle.
///
/// For a full example see SampleCustomData in the SampleGamePlugin.
class NS_CORE_DLL nsCustomData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsCustomData, nsReflectedClass);

public:
  /// \brief Loads the serialized custom data using a robust serialization-based method.
  ///
  /// This function does not need to be overridden. It will work, even if the properties change.
  /// It is only virtual in case you want to hook into the deserialization process.
  virtual void Load(class nsAbstractObjectGraph& ref_graph, class nsRttiConverterContext& ref_context, const class nsAbstractObjectNode* pRootNode);
};

/// \brief Base class for resources that represent different implementations of nsCustomData
///
/// These resources are automatically generated using these macros:
///   NS_DECLARE_CUSTOM_DATA_RESOURCE(YourCustomData)
///   NS_DEFINE_CUSTOM_DATA_RESOURCE(YourCustomData)
///
/// Put the former into a header next to YourCustomData and the latter into a cpp file.
///
/// This builds these types:
///   YourCustomDataResource
///   YourCustomDataResourceHandle
///
/// You can then use these to reference this resource type for example in components.
/// For a full example search the SampleGamePlugin for SampleCustomDataResource and SampleCustomDataResourceHandle and see how they are used.
class NS_CORE_DLL nsCustomDataResourceBase : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsCustomDataResourceBase, nsResource);

public:
  nsCustomDataResourceBase();
  ~nsCustomDataResourceBase();

protected:
  virtual void CreateAndLoadData(nsAbstractObjectGraph& ref_graph, nsRttiConverterContext& ref_context, const nsAbstractObjectNode* pRootNode) = 0;
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  nsResourceLoadDesc UpdateContent_Internal(nsStreamReader* Stream, const nsRTTI& rtti);
};

/// \brief Template resource type for sub-classed nsCustomData types.
///
/// See nsCustomDataResourceBase for details.
template <typename T>
class nsCustomDataResource : public nsCustomDataResourceBase
{
public:
  nsCustomDataResource();
  ~nsCustomDataResource();

  /// \brief Provides read access to the custom data type.
  ///
  /// Returns nullptr, if the resource wasn't loaded successfully.
  const T* GetData() const { return GetLoadingState() == nsResourceState::Loaded ? reinterpret_cast<const T*>(m_Data) : nullptr; }

protected:
  virtual void CreateAndLoadData(nsAbstractObjectGraph& graph, nsRttiConverterContext& context, const nsAbstractObjectNode* pRootNode) override;

  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;

  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;

  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

private:
  struct alignas(NS_ALIGNMENT_OF(T))
  {
    nsUInt8 m_Data[sizeof(T)];
  };
};

/// \brief Helper macro to declare a nsCustomDataResource<T> and a matching resource handle
///
/// See nsCustomDataResourceBase for details.
#define NS_DECLARE_CUSTOM_DATA_RESOURCE(SELF)                              \
  class SELF##Resource : public nsCustomDataResource<SELF>                 \
  {                                                                        \
    NS_ADD_DYNAMIC_REFLECTION(SELF##Resource, nsCustomDataResource<SELF>); \
    NS_RESOURCE_DECLARE_COMMON_CODE(SELF##Resource);                       \
  };                                                                       \
                                                                           \
  using SELF##ResourceHandle = nsTypedResourceHandle<SELF##Resource>

/// \brief Helper macro to define a nsCustomDataResource<T>
///
/// See nsCustomDataResourceBase for details.
#define NS_DEFINE_CUSTOM_DATA_RESOURCE(SELF)                                                 \
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(SELF##Resource, 1, nsRTTIDefaultAllocator<SELF##Resource>) \
  NS_END_DYNAMIC_REFLECTED_TYPE;                                                             \
                                                                                             \
  NS_RESOURCE_IMPLEMENT_COMMON_CODE(SELF##Resource)


#include <Core/Utils/Implementation/CustomData_inl.h>
