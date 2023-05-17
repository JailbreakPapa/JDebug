#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Reflection/Reflection.h>

/// \brief Base class of all attributes can be used to decorate a RTTI property.
class WD_FOUNDATION_DLL wdPropertyAttribute : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdPropertyAttribute, wdReflectedClass);
};

/// \brief A property attribute that indicates that the property may not be modified through the UI
class WD_FOUNDATION_DLL wdReadOnlyAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdReadOnlyAttribute, wdPropertyAttribute);
};

/// \brief A property attribute that indicates that the property is not to be shown in the UI
class WD_FOUNDATION_DLL wdHiddenAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdHiddenAttribute, wdPropertyAttribute);
};

/// \brief A property attribute that indicates that the property is not to be serialized
/// and whatever it points to only exists temporarily while running or in editor.
class WD_FOUNDATION_DLL wdTemporaryAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdTemporaryAttribute, wdPropertyAttribute);
};

/// \brief Used to categorize types (e.g. add component menu)
class WD_FOUNDATION_DLL wdCategoryAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCategoryAttribute, wdPropertyAttribute);

public:
  wdCategoryAttribute() = default;
  wdCategoryAttribute(const char* szCategory) { m_sCategory = szCategory; }

  const char* GetCategory() const { return m_sCategory; }

private:
  wdUntrackedString m_sCategory;
};

/// \brief A property attribute that indicates that this feature is still in development and should not be shown to all users.
class WD_FOUNDATION_DLL wdInDevelopmentAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdInDevelopmentAttribute, wdPropertyAttribute);

public:
  enum Phase
  {
    Alpha,
    Beta
  };

  wdInDevelopmentAttribute() = default;
  wdInDevelopmentAttribute(wdInt32 iPhase) { m_Phase = iPhase; }

  const char* GetString() const;

  wdInt32 m_Phase = Phase::Beta;
};


/// \brief Used for dynamic titles of visual script nodes.
/// E.g. "Set Bool Property '{Name}'" will allow the title to by dynamic
/// by reading the current value of the 'Name' property.
class WD_FOUNDATION_DLL wdTitleAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdTitleAttribute, wdPropertyAttribute);

public:
  wdTitleAttribute() = default;
  wdTitleAttribute(const char* szTitle) { m_sTitle = szTitle; }

  const char* GetTitle() const { return m_sTitle; }

private:
  wdUntrackedString m_sTitle;
};

/// \brief Used to colorize types
class WD_FOUNDATION_DLL wdColorAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdColorAttribute, wdPropertyAttribute);

public:
  wdColorAttribute() = default;
  wdColorAttribute(const wdColor& color) { m_Color = color; }

  const wdColor& GetColor() const { return m_Color; }

private:
  wdColor m_Color;
};

/// \brief A property attribute that indicates that the alpha channel of an wdColorGammaUB or wdColor should be exposed in the UI.
class WD_FOUNDATION_DLL wdExposeColorAlphaAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdExposeColorAlphaAttribute, wdPropertyAttribute);
};

/// \brief Used for any property shown as a line edit (int, float, vector etc).
class WD_FOUNDATION_DLL wdSuffixAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdSuffixAttribute, wdPropertyAttribute);

public:
  wdSuffixAttribute() = default;
  wdSuffixAttribute(const char* szSuffix) { m_sSuffix = szSuffix; }

  const char* GetSuffix() const { return m_sSuffix; }

private:
  wdUntrackedString m_sSuffix;
};

/// \brief Used to show a text instead of the minimum value of a property.
class WD_FOUNDATION_DLL wdMinValueTextAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdMinValueTextAttribute, wdPropertyAttribute);

public:
  wdMinValueTextAttribute() = default;
  wdMinValueTextAttribute(const char* szText) { m_sText = szText; }

  const char* GetText() const { return m_sText; }

private:
  wdUntrackedString m_sText;
};

/// \brief Sets the default value of the property.
class WD_FOUNDATION_DLL wdDefaultValueAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdDefaultValueAttribute, wdPropertyAttribute);

public:
  wdDefaultValueAttribute() = default;
  wdDefaultValueAttribute(const wdVariant& value) { m_Value = value; }

  const wdVariant& GetValue() const { return m_Value; }

private:
  wdVariant m_Value;
};

/// \brief A property attribute that allows to define min and max values for the UI. Min or max may be set to an invalid variant to indicate
/// unbounded values in one direction.
class WD_FOUNDATION_DLL wdClampValueAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdClampValueAttribute, wdPropertyAttribute);

public:
  wdClampValueAttribute() = default;
  wdClampValueAttribute(const wdVariant& min, const wdVariant& max)
  {
    m_MinValue = min;
    m_MaxValue = max;
  }

  const wdVariant& GetMinValue() const { return m_MinValue; }
  const wdVariant& GetMaxValue() const { return m_MaxValue; }

private:
  wdVariant m_MinValue;
  wdVariant m_MaxValue;
};

/// \brief Used to categorize properties into groups
class WD_FOUNDATION_DLL wdGroupAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdGroupAttribute, wdPropertyAttribute);

public:
  wdGroupAttribute();
  wdGroupAttribute(const char* szGroup, float fOrder = -1.0f);
  wdGroupAttribute(const char* szGroup, const char* szIconName, float fOrder = -1.0f);

  const char* GetGroup() const { return m_sGroup; }
  const char* GetIconName() const { return m_sIconName; }
  float GetOrder() const { return m_fOrder; }

private:
  wdUntrackedString m_sGroup;
  wdUntrackedString m_sIconName;
  float m_fOrder = -1.0f;
};

/// \brief Derive from this class if you want to define an attribute that replaces the property type widget.
///
/// Using this attribute affects both member properties as well as elements in a container but not the container widget.
/// When creating a property widget, the property grid will look for an attribute of this type and use
/// its type to look for a factory creator in wdRttiMappedObjectFactory<wdQtPropertyWidget>.
/// E.g. wdRttiMappedObjectFactory<wdQtPropertyWidget>::RegisterCreator(wdGetStaticRTTI<wdFileBrowserAttribute>(), FileBrowserCreator);
/// will replace the property widget for all properties that use wdFileBrowserAttribute.
class WD_FOUNDATION_DLL wdTypeWidgetAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdTypeWidgetAttribute, wdPropertyAttribute);
};

/// \brief Derive from this class if you want to define an attribute that replaces the property widget of containers.
///
/// Using this attribute affects the container widget but not container elements.
/// Only derive from this class if you want to replace the container widget itself, in every other case
/// prefer to use wdTypeWidgetAttribute.
class WD_FOUNDATION_DLL wdContainerWidgetAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdContainerWidgetAttribute, wdPropertyAttribute);
};

/// \brief Add this attribute to a tag set member property to make it use the tag set editor
/// and define the categories it will use as a ; separated list of category names.
///
/// Usage: WD_SET_MEMBER_PROPERTY("Tags", m_Tags)->AddAttributes(new wdTagSetWidgetAttribute("Category1;Category2")),
class WD_FOUNDATION_DLL wdTagSetWidgetAttribute : public wdContainerWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdTagSetWidgetAttribute, wdContainerWidgetAttribute);

public:
  wdTagSetWidgetAttribute() = default;
  wdTagSetWidgetAttribute(const char* szTagFilter) { m_sTagFilter = szTagFilter; }

  const char* GetTagFilter() const { return m_sTagFilter; }

private:
  wdUntrackedString m_sTagFilter;
};

/// \brief Add this attribute to a variant map property to make it map to the exposed parameters
/// of an asset. For this, the member property name of the asset reference needs to be passed in.
/// The exposed parameters of the currently set asset on that property will be used as the source.
///
/// Usage:
/// WD_ACCESSOR_PROPERTY("Effect", GetParticleEffectFile, SetParticleEffectFile)->AddAttributes(new wdAssetBrowserAttribute("Particle
/// Effect")), WD_MAP_ACCESSOR_PROPERTY("Parameters",...)->AddAttributes(new wdExposedParametersAttribute("Effect")),
class WD_FOUNDATION_DLL wdExposedParametersAttribute : public wdContainerWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdExposedParametersAttribute, wdContainerWidgetAttribute);

public:
  wdExposedParametersAttribute() = default;
  wdExposedParametersAttribute(const char* szParametersSource) { m_sParametersSource = szParametersSource; }

  const char* GetParametersSource() const { return m_sParametersSource; }

private:
  wdUntrackedString m_sParametersSource;
};

/// \brief Add this attribute to an embedded class or container property to make it retrieve its default values from a dynamic meta info object on an asset.
///
/// The default values are retrieved from the asset meta data of the currently set asset on that property.
///
/// Usage:
/// WD_ACCESSOR_PROPERTY("Skeleton", GetSkeletonFile, SetSkeletonFile)->AddAttributes(new wdAssetBrowserAttribute("Skeleton")),
///
/// // Use this if the embedded class m_SkeletonMetaData is of type wdSkeletonMetaData.
/// WD_MEMBER_PROPERTY("SkeletonMetaData", m_SkeletonMetaData)->AddAttributes(new wdDynamicDefaultValueAttribute("Skeleton", "wdSkeletonMetaData")),
///
/// // Use this if you don't want embed the entire meta object but just some container of it. In this case the LocalBones container must match in type to the property 'BonesArrayNameInMetaData' in the meta data type 'wdSkeletonMetaData'.
/// WD_MAP_MEMBER_PROPERTY("LocalBones", m_Bones)->AddAttributes(new wdDynamicDefaultValueAttribute("Skeleton", "wdSkeletonMetaData", "BonesArrayNameInMetaData")),
class WD_FOUNDATION_DLL wdDynamicDefaultValueAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdDynamicDefaultValueAttribute, wdTypeWidgetAttribute);

public:
  wdDynamicDefaultValueAttribute() = default;
  wdDynamicDefaultValueAttribute(const char* szClassSource, const char* szClassType, const char* szClassProperty = nullptr)
  {
    m_sClassSource = szClassSource;
    m_sClassType = szClassType;
    m_sClassProperty = szClassProperty;
  }

  const char* GetClassSource() const { return m_sClassSource; }
  const char* GetClassType() const { return m_sClassType; }
  const char* GetClassProperty() const { return m_sClassProperty; }

private:
  wdUntrackedString m_sClassSource;
  wdUntrackedString m_sClassType;
  wdUntrackedString m_sClassProperty;
};


/// \brief Sets the allowed actions on a container.
class WD_FOUNDATION_DLL wdContainerAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdContainerAttribute, wdPropertyAttribute);

public:
  wdContainerAttribute() = default;
  wdContainerAttribute(bool bCanAdd, bool bCanDelete, bool bCanMove)
  {
    m_bCanAdd = bCanAdd;
    m_bCanDelete = bCanDelete;
    m_bCanMove = bCanMove;
  }

  bool CanAdd() const { return m_bCanAdd; }
  bool CanDelete() const { return m_bCanDelete; }
  bool CanMove() const { return m_bCanMove; }

private:
  bool m_bCanAdd = false;
  bool m_bCanDelete = false;
  bool m_bCanMove = false;
};

/// \brief Limits setting of pointer properties to derived types that have the given constant property and value
///
/// The szConstantValueProperty is a sibling property of the property this attribute is assigned to,
class WD_FOUNDATION_DLL wdConstrainPointerAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdConstrainPointerAttribute, wdPropertyAttribute);

public:
  wdConstrainPointerAttribute() = default;
  wdConstrainPointerAttribute(const char* szConstantName, const char* szConstantValueProperty)
  {
    m_sConstantName = szConstantName;
    m_sConstantValueProperty = szConstantValueProperty;
  }

  const wdUntrackedString& GetConstantName() const { return m_sConstantName; }
  const wdUntrackedString& GetConstantValueProperty() const { return m_sConstantValueProperty; }

private:
  wdUntrackedString m_sConstantName;
  wdUntrackedString m_sConstantValueProperty;
};

/// \brief Defines how a reference set by wdFileBrowserAttribute and wdAssetBrowserAttribute is treated.
///
/// A few examples to explain the flags:
/// ## Input for a mesh: **Transform | Thumbnail**
/// * The input (e.g. fbx) is obviously needed for transforming the asset.
/// * We also can't generate a thumbnail without it.
/// * But we don't need to package it with the final game as it is not used by the runtime.
///
/// ## Material on a mesh: **Thumbnail | Package**
/// * The default material on a mesh asset is not needed to transform the mesh. As only the material reference is stored in the mesh asset, any changes to the material do not affect the transform output of the mesh.
/// * It is obviously needed for the thumbnail as that is what is displayed in it.
/// * We also need to package this reference as otherwise the runtime would fail to instantiate the mesh without errors.
///
/// ## Surface on hit prefab: **Package**
/// * Transforming a surface is not affected if the prefab it spawns on impact changes. Only the reference is stored.
/// * The set prefab does not show up in the thumbnail so it is not needed.
/// * We do however need to package it or otherwise the runtime would fail to spawn the prefab on impact.
///
/// As a rule of thumb (also the default for each):
/// * wdFileBrowserAttribute are mostly Transform and Thumbnail.
/// * wdAssetBrowserAttribute are mostly Thumbnail and Package.
struct wdDependencyFlags
{
  typedef wdUInt8 StorageType;

  enum Enum
  {
    None = 0,              ///< The reference is not needed for anything in production. An example of this is editor references that are only used at edit time, e.g. a default animation clip for a skeleton.
    Thumbnail = WD_BIT(0), ///< This reference is a dependency to generating a thumbnail. The material references of a mesh for example.
    Transform = WD_BIT(1), ///< This reference is a dependency to transforming this asset. The input model of a mesh for example.
    Package = WD_BIT(2),   ///< This reference is needs to be packaged as it is used at runtime by this asset. All sounds or debris generated on impact of a surface are common examples of this.
    Default = 0
  };

  struct Bits
  {
    StorageType Thumbnail : 1;
    StorageType Transform : 1;
    StorageType Package : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdDependencyFlags);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdDependencyFlags);

/// \brief A property attribute that indicates that the string property should display a file browsing button.
///
/// Allows to specify the title for the browse dialog and the allowed file types.
/// Usage: WD_MEMBER_PROPERTY("File", m_sFilePath)->AddAttributes(new wdFileBrowserAttribute("Choose a File", "*.txt")),
class WD_FOUNDATION_DLL wdFileBrowserAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdFileBrowserAttribute, wdTypeWidgetAttribute);

public:
  // Predefined common type filters
  static constexpr const char* Meshes = "*.obj;*.fbx;*.gltf;*.glb";
  static constexpr const char* MeshesWithAnimations = "*.fbx;*.gltf;*.glb";
  static constexpr const char* ImagesLdrOnly = "*.dds;*.tga;*.png;*.jpg;*.jpeg";
  static constexpr const char* ImagesHdrOnly = "*.hdr;*.exr";
  static constexpr const char* ImagesLdrAndHdr = "*.dds;*.tga;*.png;*.jpg;*.jpeg;*.hdr;*.exr";
  static constexpr const char* CubemapsLdrAndHdr = "*.dds;*.hdr";

  wdFileBrowserAttribute() = default;
  wdFileBrowserAttribute(const char* szDialogTitle, const char* szTypeFilter, const char* szCustomAction = nullptr, wdBitflags<wdDependencyFlags> depencyFlags = wdDependencyFlags::Transform | wdDependencyFlags::Thumbnail)
  {
    m_sDialogTitle = szDialogTitle;
    m_sTypeFilter = szTypeFilter;
    m_sCustomAction = szCustomAction;
    m_DependencyFlags = depencyFlags;
  }

  const char* GetDialogTitle() const { return m_sDialogTitle; }
  const char* GetTypeFilter() const { return m_sTypeFilter; }
  const char* GetCustomAction() const { return m_sCustomAction; }
  wdBitflags<wdDependencyFlags> GetDependencyFlags() const { return m_DependencyFlags; }

private:
  wdUntrackedString m_sDialogTitle;
  wdUntrackedString m_sTypeFilter;
  wdUntrackedString m_sCustomAction;
  wdBitflags<wdDependencyFlags> m_DependencyFlags;
};

/// \brief A property attribute that indicates that the string property is actually an asset reference.
///
/// Allows to specify the allowed asset types, separated with ;
/// Usage: WD_MEMBER_PROPERTY("Texture", m_sTexture)->AddAttributes(new wdAssetBrowserAttribute("Texture 2D;Texture 3D")),
class WD_FOUNDATION_DLL wdAssetBrowserAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdAssetBrowserAttribute, wdTypeWidgetAttribute);

public:
  wdAssetBrowserAttribute() = default;
  wdAssetBrowserAttribute(const char* szTypeFilter, wdBitflags<wdDependencyFlags> depencyFlags = wdDependencyFlags::Thumbnail | wdDependencyFlags::Package)
  {
    m_DependencyFlags = depencyFlags;
    SetTypeFilter(szTypeFilter);
  }

  void SetTypeFilter(const char* szTypeFilter)
  {
    wdStringBuilder sTemp(";", szTypeFilter, ";");
    m_sTypeFilter = sTemp;
  }
  const char* GetTypeFilter() const { return m_sTypeFilter; }
  wdBitflags<wdDependencyFlags> GetDependencyFlags() const { return m_DependencyFlags; }

private:
  wdUntrackedString m_sTypeFilter;
  wdBitflags<wdDependencyFlags> m_DependencyFlags;
};

/// \brief Can be used on integer properties to display them as enums. The valid enum values and their names may change at runtime.
///
/// See wdDynamicEnum for details.
class WD_FOUNDATION_DLL wdDynamicEnumAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdDynamicEnumAttribute, wdTypeWidgetAttribute);

public:
  wdDynamicEnumAttribute() = default;
  wdDynamicEnumAttribute(const char* szDynamicEnumName) { m_sDynamicEnumName = szDynamicEnumName; }

  const char* GetDynamicEnumName() const { return m_sDynamicEnumName; }

private:
  wdUntrackedString m_sDynamicEnumName;
};

/// \brief Can be used on string properties to display them as enums. The valid enum values and their names may change at runtime.
///
/// See wdDynamicStringEnum for details.
class WD_FOUNDATION_DLL wdDynamicStringEnumAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdDynamicStringEnumAttribute, wdTypeWidgetAttribute);

public:
  wdDynamicStringEnumAttribute() = default;
  wdDynamicStringEnumAttribute(const char* szDynamicEnumName) { m_sDynamicEnumName = szDynamicEnumName; }

  const char* GetDynamicEnumName() const { return m_sDynamicEnumName; }

private:
  wdUntrackedString m_sDynamicEnumName;
};


//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdManipulatorAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdManipulatorAttribute, wdPropertyAttribute);

public:
  wdManipulatorAttribute(const char* szProperty1, const char* szProperty2 = nullptr, const char* szProperty3 = nullptr,
    const char* szProperty4 = nullptr, const char* szProperty5 = nullptr, const char* szProperty6 = nullptr);

  wdUntrackedString m_sProperty1;
  wdUntrackedString m_sProperty2;
  wdUntrackedString m_sProperty3;
  wdUntrackedString m_sProperty4;
  wdUntrackedString m_sProperty5;
  wdUntrackedString m_sProperty6;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdSphereManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdSphereManipulatorAttribute, wdManipulatorAttribute);

public:
  wdSphereManipulatorAttribute();
  wdSphereManipulatorAttribute(const char* szOuterRadiusProperty, const char* szInnerRadiusProperty = nullptr);

  const wdUntrackedString& GetOuterRadiusProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetInnerRadiusProperty() const { return m_sProperty2; }
};


//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdCapsuleManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCapsuleManipulatorAttribute, wdManipulatorAttribute);

public:
  wdCapsuleManipulatorAttribute();
  wdCapsuleManipulatorAttribute(const char* szHeightProperty, const char* szRadiusProperty);

  const wdUntrackedString& GetLengthProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
};


//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdBoxManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdBoxManipulatorAttribute, wdManipulatorAttribute);

public:
  wdBoxManipulatorAttribute();
  wdBoxManipulatorAttribute(const char* szSizeProperty, float fSizeScale, bool bRecenterParent, const char* szOffsetProperty = nullptr, const char* szRotationProperty = nullptr);

  bool m_bRecenterParent = false;
  float m_fSizeScale = 1.0f;

  const wdUntrackedString& GetSizeProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetOffsetProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetRotationProperty() const { return m_sProperty3; }
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdNonUniformBoxManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdNonUniformBoxManipulatorAttribute, wdManipulatorAttribute);

public:
  wdNonUniformBoxManipulatorAttribute();
  wdNonUniformBoxManipulatorAttribute(
    const char* szNegXProp, const char* szPosXProp, const char* szNegYProp, const char* szPosYProp, const char* szNegZProp, const char* szPosZProp);
  wdNonUniformBoxManipulatorAttribute(const char* szSizeX, const char* szSizeY, const char* szSizeZ);

  bool HasSixAxis() const { return !m_sProperty4.IsEmpty(); }

  const wdUntrackedString& GetNegXProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetPosXProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetNegYProperty() const { return m_sProperty3; }
  const wdUntrackedString& GetPosYProperty() const { return m_sProperty4; }
  const wdUntrackedString& GetNegZProperty() const { return m_sProperty5; }
  const wdUntrackedString& GetPosZProperty() const { return m_sProperty6; }

  const wdUntrackedString& GetSizeXProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetSizeYProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetSizeZProperty() const { return m_sProperty3; }
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdConeLengthManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdConeLengthManipulatorAttribute, wdManipulatorAttribute);

public:
  wdConeLengthManipulatorAttribute();
  wdConeLengthManipulatorAttribute(const char* szRadiusProperty);

  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty1; }
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdConeAngleManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdConeAngleManipulatorAttribute, wdManipulatorAttribute);

public:
  wdConeAngleManipulatorAttribute();
  wdConeAngleManipulatorAttribute(const char* szAngleProperty, float fScale = 1.0f, const char* szRadiusProperty = nullptr);

  const wdUntrackedString& GetAngleProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty2; }

  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdTransformManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdTransformManipulatorAttribute, wdManipulatorAttribute);

public:
  wdTransformManipulatorAttribute();
  wdTransformManipulatorAttribute(const char* szTranslateProperty, const char* szRotateProperty = nullptr, const char* szScaleProperty = nullptr, const char* szOffsetTranslation = nullptr, const char* szOffsetRotation = nullptr);

  const wdUntrackedString& GetTranslateProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRotateProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetScaleProperty() const { return m_sProperty3; }
  const wdUntrackedString& GetGetOffsetTranslationProperty() const { return m_sProperty4; }
  const wdUntrackedString& GetGetOffsetRotationProperty() const { return m_sProperty5; }
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdBoneManipulatorAttribute : public wdManipulatorAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdBoneManipulatorAttribute, wdManipulatorAttribute);

public:
  wdBoneManipulatorAttribute();
  wdBoneManipulatorAttribute(const char* szTransformProperty, const char* szBindTo);

  const wdUntrackedString& GetTransformProperty() const { return m_sProperty1; }
};

//////////////////////////////////////////////////////////////////////////

struct wdVisualizerAnchor
{
  using StorageType = wdUInt8;

  enum Enum
  {
    Center = 0,
    PosX = WD_BIT(0),
    NegX = WD_BIT(1),
    PosY = WD_BIT(2),
    NegY = WD_BIT(3),
    PosZ = WD_BIT(4),
    NegZ = WD_BIT(5),

    Default = Center
  };

  struct Bits
  {
    StorageType PosX : 1;
    StorageType NegX : 1;
    StorageType PosY : 1;
    StorageType NegY : 1;
    StorageType PosZ : 1;
    StorageType NegZ : 1;
  };
};

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdVisualizerAnchor);

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdVisualizerAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdVisualizerAttribute, wdPropertyAttribute);

public:
  wdVisualizerAttribute(const char* szProperty1, const char* szProperty2 = nullptr, const char* szProperty3 = nullptr,
    const char* szProperty4 = nullptr, const char* szProperty5 = nullptr);

  wdUntrackedString m_sProperty1;
  wdUntrackedString m_sProperty2;
  wdUntrackedString m_sProperty3;
  wdUntrackedString m_sProperty4;
  wdUntrackedString m_sProperty5;
  wdBitflags<wdVisualizerAnchor> m_Anchor;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdBoxVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdBoxVisualizerAttribute, wdVisualizerAttribute);

public:
  wdBoxVisualizerAttribute();
  wdBoxVisualizerAttribute(const char* szSizeProperty, float fSizeScale = 1.0f, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, wdBitflags<wdVisualizerAnchor> anchor = wdVisualizerAnchor::Center, wdVec3 vOffsetOrScale = wdVec3::ZeroVector(), const char* szOffsetProperty = nullptr, const char* szRotationProperty = nullptr);

  const wdUntrackedString& GetSizeProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetColorProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetOffsetProperty() const { return m_sProperty3; }
  const wdUntrackedString& GetRotationProperty() const { return m_sProperty4; }

  float m_fSizeScale = 1.0f;
  wdColor m_Color;
  wdVec3 m_vOffsetOrScale;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdSphereVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdSphereVisualizerAttribute, wdVisualizerAttribute);

public:
  wdSphereVisualizerAttribute();
  wdSphereVisualizerAttribute(const char* szRadiusProperty, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, wdBitflags<wdVisualizerAnchor> anchor = wdVisualizerAnchor::Center, wdVec3 vOffsetOrScale = wdVec3::ZeroVector(), const char* szOffsetProperty = nullptr);

  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetColorProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetOffsetProperty() const { return m_sProperty3; }

  wdColor m_Color;
  wdVec3 m_vOffsetOrScale;
};


//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdCapsuleVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCapsuleVisualizerAttribute, wdVisualizerAttribute);

public:
  wdCapsuleVisualizerAttribute();
  wdCapsuleVisualizerAttribute(const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, wdBitflags<wdVisualizerAnchor> anchor = wdVisualizerAnchor::Center);

  const wdUntrackedString& GetHeightProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetColorProperty() const { return m_sProperty3; }

  wdColor m_Color;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdCylinderVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCylinderVisualizerAttribute, wdVisualizerAttribute);

public:
  wdCylinderVisualizerAttribute();
  wdCylinderVisualizerAttribute(wdEnum<wdBasisAxis> axis, const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, wdBitflags<wdVisualizerAnchor> anchor = wdVisualizerAnchor::Center, wdVec3 vOffsetOrScale = wdVec3::ZeroVector(), const char* szOffsetProperty = nullptr);
  wdCylinderVisualizerAttribute(const char* szAxisProperty, const char* szHeightProperty, const char* szRadiusProperty, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, wdBitflags<wdVisualizerAnchor> anchor = wdVisualizerAnchor::Center, wdVec3 vOffsetOrScale = wdVec3::ZeroVector(), const char* szOffsetProperty = nullptr);

  const wdUntrackedString& GetAxisProperty() const { return m_sProperty5; }
  const wdUntrackedString& GetHeightProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetColorProperty() const { return m_sProperty3; }
  const wdUntrackedString& GetOffsetProperty() const { return m_sProperty4; }

  wdColor m_Color;
  wdVec3 m_vOffsetOrScale;
  wdEnum<wdBasisAxis> m_Axis;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdDirectionVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdDirectionVisualizerAttribute, wdVisualizerAttribute);

public:
  wdDirectionVisualizerAttribute();
  wdDirectionVisualizerAttribute(wdEnum<wdBasisAxis> axis, float fScale, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, const char* szLengthProperty = nullptr);
  wdDirectionVisualizerAttribute(const char* szAxisProperty, float fScale, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr, const char* szLengthProperty = nullptr);

  const wdUntrackedString& GetColorProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetLengthProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetAxisProperty() const { return m_sProperty3; }

  wdEnum<wdBasisAxis> m_Axis;
  wdColor m_Color;
  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdConeVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdConeVisualizerAttribute, wdVisualizerAttribute);

public:
  wdConeVisualizerAttribute();

  /// \brief Attribute to add on an RTTI type to add a cone visualizer for specific properties.
  ///
  /// szRadiusProperty may be nullptr, in which case it is assumed to be 1
  /// fScale will be multiplied with value of szRadiusProperty to determine the size of the cone
  /// szColorProperty may be nullptr. In this case it is ignored and fixedColor is used instead.
  /// fixedColor is ignored if szColorProperty is valid.
  wdConeVisualizerAttribute(wdEnum<wdBasisAxis> axis, const char* szAngleProperty, float fScale, const char* szRadiusProperty, const wdColor& fixedColor = wdColorScheme::LightUI(wdColorScheme::Grape), const char* szColorProperty = nullptr);

  const wdUntrackedString& GetAngleProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetColorProperty() const { return m_sProperty3; }

  wdEnum<wdBasisAxis> m_Axis;
  wdColor m_Color;
  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class WD_FOUNDATION_DLL wdCameraVisualizerAttribute : public wdVisualizerAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdCameraVisualizerAttribute, wdVisualizerAttribute);

public:
  wdCameraVisualizerAttribute();

  /// \brief Attribute to add on an RTTI type to add a camera cone visualizer.
  wdCameraVisualizerAttribute(const char* szModeProperty, const char* szFovProperty, const char* szOrthoDimProperty, const char* szNearPlaneProperty, const char* szFarPlaneProperty);

  const wdUntrackedString& GetModeProperty() const { return m_sProperty1; }
  const wdUntrackedString& GetFovProperty() const { return m_sProperty2; }
  const wdUntrackedString& GetOrthoDimProperty() const { return m_sProperty3; }
  const wdUntrackedString& GetNearPlaneProperty() const { return m_sProperty4; }
  const wdUntrackedString& GetFarPlaneProperty() const { return m_sProperty5; }
};

//////////////////////////////////////////////////////////////////////////

// Implementation moved here as it requires wdPropertyAttribute to be fully defined.
template <typename Type>
const Type* wdRTTI::GetAttributeByType() const
{
  for (const auto* pAttr : m_Attributes)
  {
    if (pAttr->GetDynamicRTTI()->IsDerivedFrom<Type>())
      return static_cast<const Type*>(pAttr);
  }
  if (GetParentType() != nullptr)
    return GetParentType()->GetAttributeByType<Type>();
  else
    return nullptr;
}

template <typename Type>
const Type* wdAbstractProperty::GetAttributeByType() const
{
  for (const auto* pAttr : m_Attributes)
  {
    if (pAttr->GetDynamicRTTI()->IsDerivedFrom<Type>())
      return static_cast<const Type*>(pAttr);
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////////

/// \brief A property attribute that specifies the max size of an array. If it is reached, no further elemets are allowed to be added.
class WD_FOUNDATION_DLL wdMaxArraySizeAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdMaxArraySizeAttribute, wdPropertyAttribute);

public:
  wdMaxArraySizeAttribute() = default;
  wdMaxArraySizeAttribute(wdUInt32 uiMaxSize) { m_uiMaxSize = uiMaxSize; }

  const wdUInt32& GetMaxSize() const { return m_uiMaxSize; }

private:
  wdUInt32 m_uiMaxSize = 0;
};

//////////////////////////////////////////////////////////////////////////

/// \brief If this attribute is set, the UI is encouraged to prevent the user from creating duplicates of the same thing.
///
/// For arrays of objects this means that multiple objects of the same type are not allowed.
class WD_FOUNDATION_DLL wdPreventDuplicatesAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdPreventDuplicatesAttribute, wdPropertyAttribute);

public:
  wdPreventDuplicatesAttribute() = default;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Attribute for wdMessages to instruct the visual script framework to automatically generate a node for sending this type of
/// message
class WD_FOUNDATION_DLL wdAutoGenVisScriptMsgSender : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdAutoGenVisScriptMsgSender, wdPropertyAttribute);
};

/// \brief Attribute for wdMessages to instruct the visual script framework to automatically generate a node for handling this type of
/// message
class WD_FOUNDATION_DLL wdAutoGenVisScriptMsgHandler : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdAutoGenVisScriptMsgHandler, wdPropertyAttribute);
};

/// \brief Attribute to mark a function up to be exposed to the scripting system. Arguments specify the names of the function parameters.
class WD_FOUNDATION_DLL wdScriptableFunctionAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdScriptableFunctionAttribute, wdPropertyAttribute);

  enum ArgType : wdUInt8
  {
    In,
    Out,
    Inout
  };

  wdScriptableFunctionAttribute(ArgType argType1 = In, const char* szArg1 = nullptr, ArgType argType2 = In, const char* szArg2 = nullptr,
    ArgType argType3 = In, const char* szArg3 = nullptr, ArgType argType4 = In, const char* szArg4 = nullptr, ArgType argType5 = In,
    const char* szArg5 = nullptr, ArgType argType6 = In, const char* szArg6 = nullptr);

  const char* GetArgumentName(wdUInt32 uiIndex) const;

  ArgType GetArgumentType(wdUInt32 uiIndex) const;

  wdUntrackedString m_sArg1;
  wdUntrackedString m_sArg2;
  wdUntrackedString m_sArg3;
  wdUntrackedString m_sArg4;
  wdUntrackedString m_sArg5;
  wdUntrackedString m_sArg6;

  wdUInt8 m_ArgType1;
  wdUInt8 m_ArgType2;
  wdUInt8 m_ArgType3;
  wdUInt8 m_ArgType4;
  wdUInt8 m_ArgType5;
  wdUInt8 m_ArgType6;
};

/// \brief Used to annotate properties to which pin or function parameter they belong (if necessary)
class WD_FOUNDATION_DLL wdVisScriptMappingAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdVisScriptMappingAttribute, wdPropertyAttribute);

  wdVisScriptMappingAttribute() = default;
  wdVisScriptMappingAttribute(wdInt32 iMapping)
    : m_iMapping(iMapping)
  {
  }

  wdInt32 m_iMapping = 0;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to mark that a component provides functionality that is executed with a long operation in the editor.
///
/// \a szOpTypeName must be the class name of a class derived from wdLongOpProxy.
/// Once a component is added to a scene with this attribute, the named long op will appear in the UI and can be executed.
///
/// The automatic registration is done by wdLongOpsAdapter
class WD_FOUNDATION_DLL wdLongOpAttribute : public wdPropertyAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdLongOpAttribute, wdPropertyAttribute);

public:
  wdLongOpAttribute() = default;
  wdLongOpAttribute(const char* szOpTypeName) { m_sOpTypeName = szOpTypeName; }

  wdUntrackedString m_sOpTypeName;
};

//////////////////////////////////////////////////////////////////////////

/// \brief A property attribute that indicates that the string property is actually a game object reference.
class WD_FOUNDATION_DLL wdGameObjectReferenceAttribute : public wdTypeWidgetAttribute
{
  WD_ADD_DYNAMIC_REFLECTION(wdGameObjectReferenceAttribute, wdTypeWidgetAttribute);

public:
  wdGameObjectReferenceAttribute() = default;
};
