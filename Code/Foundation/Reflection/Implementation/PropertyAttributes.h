#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/Math/ColorScheme.h>
#include <Foundation/Reflection/Reflection.h>

/// \brief Base class of all attributes can be used to decorate a RTTI property.
class NS_FOUNDATION_DLL nsPropertyAttribute : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsPropertyAttribute, nsReflectedClass);
};

/// \brief A property attribute that indicates that the property may not be modified through the UI
class NS_FOUNDATION_DLL nsReadOnlyAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsReadOnlyAttribute, nsPropertyAttribute);
};

/// \brief A property attribute that indicates that the property is not to be shown in the UI
class NS_FOUNDATION_DLL nsHiddenAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsHiddenAttribute, nsPropertyAttribute);
};

/// \brief A property attribute that indicates that the property is not to be serialized
/// and whatever it points to only exists temporarily while running or in editor.
class NS_FOUNDATION_DLL nsTemporaryAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsTemporaryAttribute, nsPropertyAttribute);
};

/// \brief Used to categorize types (e.g. add component menu)
class NS_FOUNDATION_DLL nsCategoryAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCategoryAttribute, nsPropertyAttribute);

public:
  nsCategoryAttribute() = default;
  nsCategoryAttribute(const char* szCategory)
    : m_sCategory(szCategory)
  {
  }

  const char* GetCategory() const { return m_sCategory; }

private:
  nsUntrackedString m_sCategory;
};

/// \brief A property attribute that indicates that this feature is still in development and should not be shown to all users.
class NS_FOUNDATION_DLL nsInDevelopmentAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsInDevelopmentAttribute, nsPropertyAttribute);

public:
  enum Phase
  {
    Alpha,
    Beta
  };

  nsInDevelopmentAttribute() = default;
  nsInDevelopmentAttribute(nsInt32 iPhase) { m_Phase = iPhase; }

  const char* GetString() const;

  nsInt32 m_Phase = Phase::Beta;
};


/// \brief Used for dynamic titles of visual script nodes.
/// E.g. "Set Bool Property '{Name}'" will allow the title to by dynamic
/// by reading the current value of the 'Name' property.
class NS_FOUNDATION_DLL nsTitleAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsTitleAttribute, nsPropertyAttribute);

public:
  nsTitleAttribute() = default;
  nsTitleAttribute(const char* szTitle)
    : m_sTitle(szTitle)
  {
  }

  const char* GetTitle() const { return m_sTitle; }

private:
  nsUntrackedString m_sTitle;
};

/// \brief Used to colorize types
class NS_FOUNDATION_DLL nsColorAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsColorAttribute, nsPropertyAttribute);

public:
  nsColorAttribute() = default;
  nsColorAttribute(const nsColor& color)
    : m_Color(color)
  {
  }
  const nsColor& GetColor() const { return m_Color; }

private:
  nsColor m_Color;
};

/// \brief A property attribute that indicates that the alpha channel of an nsColorGammaUB or nsColor should be exposed in the UI.
class NS_FOUNDATION_DLL nsExposeColorAlphaAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsExposeColorAlphaAttribute, nsPropertyAttribute);
};

/// \brief Used for any property shown as a line edit (int, float, vector etc).
class NS_FOUNDATION_DLL nsSuffixAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsSuffixAttribute, nsPropertyAttribute);

public:
  nsSuffixAttribute() = default;
  nsSuffixAttribute(const char* szSuffix)
    : m_sSuffix(szSuffix)
  {
  }

  const char* GetSuffix() const { return m_sSuffix; }

private:
  nsUntrackedString m_sSuffix;
};

/// \brief Used to show a text instead of the minimum value of a property.
class NS_FOUNDATION_DLL nsMinValueTextAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsMinValueTextAttribute, nsPropertyAttribute);

public:
  nsMinValueTextAttribute() = default;
  nsMinValueTextAttribute(const char* szText)
    : m_sText(szText)
  {
  }

  const char* GetText() const { return m_sText; }

private:
  nsUntrackedString m_sText;
};

/// \brief Sets the default value of the property.
class NS_FOUNDATION_DLL nsDefaultValueAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDefaultValueAttribute, nsPropertyAttribute);

public:
  nsDefaultValueAttribute() = default;

  nsDefaultValueAttribute(const nsVariant& value)
    : m_Value(value)
  {
  }

  nsDefaultValueAttribute(nsInt32 value)
    : m_Value(value)
  {
  }

  nsDefaultValueAttribute(float value)
    : m_Value(value)
  {
  }

  nsDefaultValueAttribute(double value)
    : m_Value(value)
  {
  }

  nsDefaultValueAttribute(nsStringView value)
    : m_Value(nsVariant(value, false))
  {
  }

  nsDefaultValueAttribute(const char* value)
    : m_Value(nsVariant(nsStringView(value), false))
  {
  }

  const nsVariant& GetValue() const { return m_Value; }

private:
  nsVariant m_Value;
};

/// \brief A property attribute that allows to define min and max values for the UI. Min or max may be set to an invalid variant to indicate
/// unbounded values in one direction.
class NS_FOUNDATION_DLL nsClampValueAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsClampValueAttribute, nsPropertyAttribute);

public:
  nsClampValueAttribute() = default;
  nsClampValueAttribute(const nsVariant& min, const nsVariant& max)
    : m_MinValue(min)
    , m_MaxValue(max)
  {
  }

  const nsVariant& GetMinValue() const { return m_MinValue; }
  const nsVariant& GetMaxValue() const { return m_MaxValue; }

protected:
  nsVariant m_MinValue;
  nsVariant m_MaxValue;
};

/// \brief Used to categorize properties into groups
class NS_FOUNDATION_DLL nsGroupAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsGroupAttribute, nsPropertyAttribute);

public:
  nsGroupAttribute();
  nsGroupAttribute(const char* szGroup, float fOrder = -1.0f);
  nsGroupAttribute(const char* szGroup, const char* szIconName, float fOrder = -1.0f);

  const char* GetGroup() const { return m_sGroup; }
  const char* GetIconName() const { return m_sIconName; }
  float GetOrder() const { return m_fOrder; }

private:
  nsUntrackedString m_sGroup;
  nsUntrackedString m_sIconName;
  float m_fOrder = -1.0f;
};

/// \brief Derive from this class if you want to define an attribute that replaces the property type widget.
///
/// Using this attribute affects both member properties as well as elements in a container but not the container widget.
/// When creating a property widget, the property grid will look for an attribute of this type and use
/// its type to look for a factory creator in nsRttiMappedObjectFactory<nsQtPropertyWidget>.
/// E.g. nsRttiMappedObjectFactory<nsQtPropertyWidget>::RegisterCreator(nsGetStaticRTTI<nsFileBrowserAttribute>(), FileBrowserCreator);
/// will replace the property widget for all properties that use nsFileBrowserAttribute.
class NS_FOUNDATION_DLL nsTypeWidgetAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsTypeWidgetAttribute, nsPropertyAttribute);
};

/// \brief Derive from this class if you want to define an attribute that replaces the property widget of containers.
///
/// Using this attribute affects the container widget but not container elements.
/// Only derive from this class if you want to replace the container widget itself, in every other case
/// prefer to use nsTypeWidgetAttribute.
class NS_FOUNDATION_DLL nsContainerWidgetAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsContainerWidgetAttribute, nsPropertyAttribute);
};

/// \brief Add this attribute to a tag set member property to make it use the tag set editor
/// and define the categories it will use as a ; separated list of category names.
///
/// Usage: NS_SET_MEMBER_PROPERTY("Tags", m_Tags)->AddAttributes(new nsTagSetWidgetAttribute("Category1;Category2")),
class NS_FOUNDATION_DLL nsTagSetWidgetAttribute : public nsContainerWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsTagSetWidgetAttribute, nsContainerWidgetAttribute);

public:
  nsTagSetWidgetAttribute() = default;
  nsTagSetWidgetAttribute(const char* szTagFilter)
    : m_sTagFilter(szTagFilter)
  {
  }

  const char* GetTagFilter() const { return m_sTagFilter; }

private:
  nsUntrackedString m_sTagFilter;
};

/// \brief This attribute indicates that a widget should not use temporary transactions when changing the value.
class NS_FOUNDATION_DLL nsNoTemporaryTransactionsAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsNoTemporaryTransactionsAttribute, nsPropertyAttribute);
};

/// \brief Add this attribute to a variant map property to make it map to the exposed parameters
/// of an asset. For this, the member property name of the asset reference needs to be passed in.
/// The exposed parameters of the currently set asset on that property will be used as the source.
///
/// Usage:
/// NS_ACCESSOR_PROPERTY("Effect", GetParticleEffectFile, SetParticleEffectFile)->AddAttributes(new nsAssetBrowserAttribute("Particle
/// Effect")), NS_MAP_ACCESSOR_PROPERTY("Parameters",...)->AddAttributes(new nsExposedParametersAttribute("Effect")),
class NS_FOUNDATION_DLL nsExposedParametersAttribute : public nsContainerWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsExposedParametersAttribute, nsContainerWidgetAttribute);

public:
  nsExposedParametersAttribute() = default;
  nsExposedParametersAttribute(const char* szParametersSource)
    : m_sParametersSource(szParametersSource)
  {
  }

  const char* GetParametersSource() const { return m_sParametersSource; }

private:
  nsUntrackedString m_sParametersSource;
};

/// \brief Add this attribute to an embedded class or container property to make it retrieve its default values from a dynamic meta info object on an asset.
///
/// The default values are retrieved from the asset meta data of the currently set asset on that property.
///
/// Usage:
/// NS_ACCESSOR_PROPERTY("Skeleton", GetSkeletonFile, SetSkeletonFile)->AddAttributes(new nsAssetBrowserAttribute("Skeleton")),
///
/// // Use this if the embedded class m_SkeletonMetaData is of type nsSkeletonMetaData.
/// NS_MEMBER_PROPERTY("SkeletonMetaData", m_SkeletonMetaData)->AddAttributes(new nsDynamicDefaultValueAttribute("Skeleton", "nsSkeletonMetaData")),
///
/// // Use this if you don't want embed the entire meta object but just some container of it. In this case the LocalBones container must match in type to the property 'BonesArrayNameInMetaData' in the meta data type 'nsSkeletonMetaData'.
/// NS_MAP_MEMBER_PROPERTY("LocalBones", m_Bones)->AddAttributes(new nsDynamicDefaultValueAttribute("Skeleton", "nsSkeletonMetaData", "BonesArrayNameInMetaData")),
class NS_FOUNDATION_DLL nsDynamicDefaultValueAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicDefaultValueAttribute, nsTypeWidgetAttribute);

public:
  nsDynamicDefaultValueAttribute() = default;
  nsDynamicDefaultValueAttribute(const char* szClassSource,
    const char* szClassType, const char* szClassProperty = nullptr)
    : m_sClassSource(szClassSource)
    , m_sClassType(szClassType)
    , m_sClassProperty(szClassProperty)
  {
  }

  const char* GetClassSource() const { return m_sClassSource; }
  const char* GetClassType() const { return m_sClassType; }
  const char* GetClassProperty() const { return m_sClassProperty; }

private:
  nsUntrackedString m_sClassSource;
  nsUntrackedString m_sClassType;
  nsUntrackedString m_sClassProperty;
};


/// \brief Sets the allowed actions on a container.
class NS_FOUNDATION_DLL nsContainerAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsContainerAttribute, nsPropertyAttribute);

public:
  nsContainerAttribute() = default;
  nsContainerAttribute(bool bCanAdd, bool bCanDelete, bool bCanMove)
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

/// \brief Defines how a reference set by nsFileBrowserAttribute and nsAssetBrowserAttribute is treated.
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
/// * We do, however, need to package it or otherwise the runtime would fail to spawn the prefab on impact.
///
/// As a rule of thumb (also the default for each):
/// * nsFileBrowserAttribute are mostly Transform and Thumbnail.
/// * nsAssetBrowserAttribute are mostly Thumbnail and Package.
struct nsDependencyFlags
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,              ///< The reference is not needed for anything in production. An example of this is editor references that are only used at edit time, e.g. a default animation clip for a skeleton.
    Thumbnail = NS_BIT(0), ///< This reference is a dependency to generating a thumbnail. The material references of a mesh for example.
    Transform = NS_BIT(1), ///< This reference is a dependency to transforming this asset. The input model of a mesh for example.
    Package = NS_BIT(2),   ///< This reference needs to be packaged as it is used at runtime by this asset. All sounds or debris generated on impact of a surface are common examples of this.
    Default = 0
  };

  struct Bits
  {
    StorageType Thumbnail : 1;
    StorageType Transform : 1;
    StorageType Package : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsDependencyFlags);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsDependencyFlags);

/// \brief A property attribute that indicates that the string property should display a file browsing button.
///
/// Allows to specify the title for the browse dialog and the allowed file types.
/// Usage: NS_MEMBER_PROPERTY("File", m_sFilePath)->AddAttributes(new nsFileBrowserAttribute("Choose a File", "*.txt")),
class NS_FOUNDATION_DLL nsFileBrowserAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsFileBrowserAttribute, nsTypeWidgetAttribute);

public:
  // Predefined common type filters
  static constexpr nsStringView Meshes = "*.obj;*.fbx;*.gltf;*.glb"_nssv;
  static constexpr nsStringView MeshesWithAnimations = "*.fbx;*.gltf;*.glb"_nssv;
  static constexpr nsStringView ImagesLdrOnly = "*.dds;*.tga;*.png;*.jpg;*.jpeg"_nssv;
  static constexpr nsStringView ImagesHdrOnly = "*.hdr;*.exr"_nssv;
  static constexpr nsStringView ImagesLdrAndHdr = "*.dds;*.tga;*.png;*.jpg;*.jpeg;*.hdr;*.exr"_nssv;
  static constexpr nsStringView CubemapsLdrAndHdr = "*.dds;*.hdr"_nssv;

  nsFileBrowserAttribute() = default;
  nsFileBrowserAttribute(nsStringView sDialogTitle, nsStringView sTypeFilter, nsStringView sCustomAction = {}, nsBitflags<nsDependencyFlags> depencyFlags = nsDependencyFlags::Transform | nsDependencyFlags::Thumbnail)
    : m_sDialogTitle(sDialogTitle)
    , m_sTypeFilter(sTypeFilter)
    , m_sCustomAction(sCustomAction)
    , m_DependencyFlags(depencyFlags)
  {
  }

  nsStringView GetDialogTitle() const { return m_sDialogTitle; }
  nsStringView GetTypeFilter() const { return m_sTypeFilter; }
  nsStringView GetCustomAction() const { return m_sCustomAction; }
  nsBitflags<nsDependencyFlags> GetDependencyFlags() const { return m_DependencyFlags; }

private:
  nsUntrackedString m_sDialogTitle;
  nsUntrackedString m_sTypeFilter;
  nsUntrackedString m_sCustomAction;
  nsBitflags<nsDependencyFlags> m_DependencyFlags;
};

/// \brief Indicates that the string property should allow to browse for an file (or programs) outside the project directories.
///
/// Allows to specify the title for the browse dialog and the allowed file types.
/// Usage: NS_MEMBER_PROPERTY("File", m_sFilePath)->AddAttributes(new nsFileBrowserAttribute("Choose a File", "*.exe")),
class NS_FOUNDATION_DLL nsExternalFileBrowserAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsExternalFileBrowserAttribute, nsTypeWidgetAttribute);

public:
  nsExternalFileBrowserAttribute() = default;
  nsExternalFileBrowserAttribute(nsStringView sDialogTitle, nsStringView sTypeFilter)
    : m_sDialogTitle(sDialogTitle)
    , m_sTypeFilter(sTypeFilter)
  {
  }

  nsStringView GetDialogTitle() const { return m_sDialogTitle; }
  nsStringView GetTypeFilter() const { return m_sTypeFilter; }

private:
  nsUntrackedString m_sDialogTitle;
  nsUntrackedString m_sTypeFilter;
};

/// \brief A property attribute that indicates that the string property is actually an asset reference.
///
/// Allows to specify the allowed asset types, separated with ;
/// Usage: NS_MEMBER_PROPERTY("Texture", m_sTexture)->AddAttributes(new nsAssetBrowserAttribute("Texture 2D;Texture 3D")),
class NS_FOUNDATION_DLL nsAssetBrowserAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsAssetBrowserAttribute, nsTypeWidgetAttribute);

public:
  nsAssetBrowserAttribute() = default;
  nsAssetBrowserAttribute(const char* szTypeFilter, nsBitflags<nsDependencyFlags> depencyFlags = nsDependencyFlags::Thumbnail | nsDependencyFlags::Package)
    : m_DependencyFlags(depencyFlags)
  {
    SetTypeFilter(szTypeFilter);
  }

  nsAssetBrowserAttribute(const char* szTypeFilter, const char* szRequiredTag, nsBitflags<nsDependencyFlags> depencyFlags = nsDependencyFlags::Thumbnail | nsDependencyFlags::Package)
    : m_DependencyFlags(depencyFlags)
  {
    SetTypeFilter(szTypeFilter);
    m_sRequiredTag = szRequiredTag;
  }

  void SetTypeFilter(const char* szTypeFilter)
  {
    nsStringBuilder sTemp(";", szTypeFilter, ";");
    m_sTypeFilter = sTemp;
  }

  const char* GetTypeFilter() const { return m_sTypeFilter; }
  nsBitflags<nsDependencyFlags> GetDependencyFlags() const { return m_DependencyFlags; }

  const char* GetRequiredTag() const { return m_sRequiredTag; }

private:
  nsUntrackedString m_sTypeFilter;
  nsUntrackedString m_sRequiredTag;
  nsBitflags<nsDependencyFlags> m_DependencyFlags;
};

/// \brief Can be used on integer properties to display them as enums. The valid enum values and their names may change at runtime.
///
/// See nsDynamicEnum for details.
class NS_FOUNDATION_DLL nsDynamicEnumAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicEnumAttribute, nsTypeWidgetAttribute);

public:
  nsDynamicEnumAttribute() = default;
  nsDynamicEnumAttribute(const char* szDynamicEnumName)
    : m_sDynamicEnumName(szDynamicEnumName)
  {
  }

  const char* GetDynamicEnumName() const { return m_sDynamicEnumName; }

private:
  nsUntrackedString m_sDynamicEnumName;
};

/// \brief Can be used on string properties to display them as enums. The valid enum values and their names may change at runtime.
///
/// See nsDynamicStringEnum for details.
class NS_FOUNDATION_DLL nsDynamicStringEnumAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicStringEnumAttribute, nsTypeWidgetAttribute);

public:
  nsDynamicStringEnumAttribute() = default;
  nsDynamicStringEnumAttribute(const char* szDynamicEnumName)
    : m_sDynamicEnumName(szDynamicEnumName)
  {
  }

  const char* GetDynamicEnumName() const { return m_sDynamicEnumName; }

private:
  nsUntrackedString m_sDynamicEnumName;
};

/// \brief Can be used on integer properties to display them as bitflags. The valid bitflags and their names may change at runtime.
class NS_FOUNDATION_DLL nsDynamicBitflagsAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicBitflagsAttribute, nsTypeWidgetAttribute);

public:
  nsDynamicBitflagsAttribute() = default;
  nsDynamicBitflagsAttribute(nsStringView sDynamicName)
    : m_sDynamicBitflagsName(sDynamicName)
  {
  }

  nsStringView GetDynamicBitflagsName() const { return m_sDynamicBitflagsName; }

private:
  nsUntrackedString m_sDynamicBitflagsName;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsManipulatorAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsManipulatorAttribute, nsPropertyAttribute);

public:
  nsManipulatorAttribute(const char* szProperty1, const char* szProperty2 = nullptr, const char* szProperty3 = nullptr,
    const char* szProperty4 = nullptr, const char* szProperty5 = nullptr, const char* szProperty6 = nullptr);

  nsUntrackedString m_sProperty1;
  nsUntrackedString m_sProperty2;
  nsUntrackedString m_sProperty3;
  nsUntrackedString m_sProperty4;
  nsUntrackedString m_sProperty5;
  nsUntrackedString m_sProperty6;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsSphereManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsSphereManipulatorAttribute, nsManipulatorAttribute);

public:
  nsSphereManipulatorAttribute();
  nsSphereManipulatorAttribute(const char* szOuterRadiusProperty, const char* szInnerRadiusProperty = nullptr);

  const nsUntrackedString& GetOuterRadiusProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetInnerRadiusProperty() const { return m_sProperty2; }
};


//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsCapsuleManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCapsuleManipulatorAttribute, nsManipulatorAttribute);

public:
  nsCapsuleManipulatorAttribute();
  nsCapsuleManipulatorAttribute(const char* szHeightProperty, const char* szRadiusProperty);

  const nsUntrackedString& GetLengthProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
};


//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsBoxManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoxManipulatorAttribute, nsManipulatorAttribute);

public:
  nsBoxManipulatorAttribute();
  nsBoxManipulatorAttribute(const char* szSizeProperty, float fSizeScale, bool bRecenterParent, const char* szOffsetProperty = nullptr, const char* szRotationProperty = nullptr);

  bool m_bRecenterParent = false;
  float m_fSizeScale = 1.0f;

  const nsUntrackedString& GetSizeProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetOffsetProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetRotationProperty() const { return m_sProperty3; }
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsNonUniformBoxManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsNonUniformBoxManipulatorAttribute, nsManipulatorAttribute);

public:
  nsNonUniformBoxManipulatorAttribute();
  nsNonUniformBoxManipulatorAttribute(
    const char* szNegXProp, const char* szPosXProp, const char* szNegYProp, const char* szPosYProp, const char* szNegZProp, const char* szPosZProp);
  nsNonUniformBoxManipulatorAttribute(const char* szSizeX, const char* szSizeY, const char* szSizeZ);

  bool HasSixAxis() const { return !m_sProperty4.IsEmpty(); }

  const nsUntrackedString& GetNegXProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetPosXProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetNegYProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetPosYProperty() const { return m_sProperty4; }
  const nsUntrackedString& GetNegZProperty() const { return m_sProperty5; }
  const nsUntrackedString& GetPosZProperty() const { return m_sProperty6; }

  const nsUntrackedString& GetSizeXProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetSizeYProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetSizeZProperty() const { return m_sProperty3; }
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsConeLengthManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsConeLengthManipulatorAttribute, nsManipulatorAttribute);

public:
  nsConeLengthManipulatorAttribute();
  nsConeLengthManipulatorAttribute(const char* szRadiusProperty);

  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty1; }
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsConeAngleManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsConeAngleManipulatorAttribute, nsManipulatorAttribute);

public:
  nsConeAngleManipulatorAttribute();
  nsConeAngleManipulatorAttribute(const char* szAngleProperty, float fScale = 1.0f, const char* szRadiusProperty = nullptr);

  const nsUntrackedString& GetAngleProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty2; }

  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsTransformManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsTransformManipulatorAttribute, nsManipulatorAttribute);

public:
  nsTransformManipulatorAttribute();
  nsTransformManipulatorAttribute(const char* szTranslateProperty, const char* szRotateProperty = nullptr, const char* szScaleProperty = nullptr, const char* szOffsetTranslation = nullptr, const char* szOffsetRotation = nullptr);

  const nsUntrackedString& GetTranslateProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRotateProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetScaleProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetGetOffsetTranslationProperty() const { return m_sProperty4; }
  const nsUntrackedString& GetGetOffsetRotationProperty() const { return m_sProperty5; }
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsBoneManipulatorAttribute : public nsManipulatorAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoneManipulatorAttribute, nsManipulatorAttribute);

public:
  nsBoneManipulatorAttribute();
  nsBoneManipulatorAttribute(const char* szTransformProperty, const char* szBindTo);

  const nsUntrackedString& GetTransformProperty() const { return m_sProperty1; }
};

//////////////////////////////////////////////////////////////////////////

struct nsVisualizerAnchor
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Center = 0,
    PosX = NS_BIT(0),
    NegX = NS_BIT(1),
    PosY = NS_BIT(2),
    NegY = NS_BIT(3),
    PosZ = NS_BIT(4),
    NegZ = NS_BIT(5),

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

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsVisualizerAnchor);

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsVisualizerAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsVisualizerAttribute, nsPropertyAttribute);

public:
  nsVisualizerAttribute(const char* szProperty1, const char* szProperty2 = nullptr, const char* szProperty3 = nullptr,
    const char* szProperty4 = nullptr, const char* szProperty5 = nullptr);

  nsUntrackedString m_sProperty1;
  nsUntrackedString m_sProperty2;
  nsUntrackedString m_sProperty3;
  nsUntrackedString m_sProperty4;
  nsUntrackedString m_sProperty5;
  nsBitflags<nsVisualizerAnchor> m_Anchor;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsBoxVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoxVisualizerAttribute, nsVisualizerAttribute);

public:
  nsBoxVisualizerAttribute();
  nsBoxVisualizerAttribute(const char* szSizeProperty, float fSizeScale = 1.0f, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, nsBitflags<nsVisualizerAnchor> anchor = nsVisualizerAnchor::Center, nsVec3 vOffsetOrScale = nsVec3::MakeZero(), const char* szOffsetProperty = nullptr, const char* szRotationProperty = nullptr);

  const nsUntrackedString& GetSizeProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetOffsetProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetRotationProperty() const { return m_sProperty4; }

  float m_fSizeScale = 1.0f;
  nsColor m_Color;
  nsVec3 m_vOffsetOrScale;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsSphereVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsSphereVisualizerAttribute, nsVisualizerAttribute);

public:
  nsSphereVisualizerAttribute();
  nsSphereVisualizerAttribute(const char* szRadiusProperty, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, nsBitflags<nsVisualizerAnchor> anchor = nsVisualizerAnchor::Center, nsVec3 vOffsetOrScale = nsVec3::MakeZero(), const char* szOffsetProperty = nullptr);

  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetOffsetProperty() const { return m_sProperty3; }

  nsColor m_Color;
  nsVec3 m_vOffsetOrScale;
};


//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsCapsuleVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCapsuleVisualizerAttribute, nsVisualizerAttribute);

public:
  nsCapsuleVisualizerAttribute();
  nsCapsuleVisualizerAttribute(const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, nsBitflags<nsVisualizerAnchor> anchor = nsVisualizerAnchor::Center);

  const nsUntrackedString& GetHeightProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty3; }

  nsColor m_Color;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsCylinderVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCylinderVisualizerAttribute, nsVisualizerAttribute);

public:
  nsCylinderVisualizerAttribute();
  nsCylinderVisualizerAttribute(nsEnum<nsBasisAxis> axis, const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, nsBitflags<nsVisualizerAnchor> anchor = nsVisualizerAnchor::Center, nsVec3 vOffsetOrScale = nsVec3::MakeZero(), const char* szOffsetProperty = nullptr);
  nsCylinderVisualizerAttribute(const char* szAxisProperty, const char* szHeightProperty, const char* szRadiusProperty, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, nsBitflags<nsVisualizerAnchor> anchor = nsVisualizerAnchor::Center, nsVec3 vOffsetOrScale = nsVec3::MakeZero(), const char* szOffsetProperty = nullptr);

  const nsUntrackedString& GetAxisProperty() const { return m_sProperty5; }
  const nsUntrackedString& GetHeightProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetOffsetProperty() const { return m_sProperty4; }

  nsColor m_Color;
  nsVec3 m_vOffsetOrScale;
  nsEnum<nsBasisAxis> m_Axis;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsDirectionVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDirectionVisualizerAttribute, nsVisualizerAttribute);

public:
  nsDirectionVisualizerAttribute();
  nsDirectionVisualizerAttribute(nsEnum<nsBasisAxis> axis, float fScale, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, const char* szLengthProperty = nullptr);
  nsDirectionVisualizerAttribute(const char* szAxisProperty, float fScale, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr, const char* szLengthProperty = nullptr);

  const nsUntrackedString& GetColorProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetLengthProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetAxisProperty() const { return m_sProperty3; }

  nsEnum<nsBasisAxis> m_Axis;
  nsColor m_Color;
  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsConeVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsConeVisualizerAttribute, nsVisualizerAttribute);

public:
  nsConeVisualizerAttribute();

  /// \brief Attribute to add on an RTTI type to add a cone visualizer for specific properties.
  ///
  /// szRadiusProperty may be nullptr, in which case it is assumed to be 1
  /// fScale will be multiplied with value of szRadiusProperty to determine the size of the cone
  /// szColorProperty may be nullptr. In this case it is ignored and fixedColor is used instead.
  /// fixedColor is ignored if szColorProperty is valid.
  nsConeVisualizerAttribute(nsEnum<nsBasisAxis> axis, const char* szAngleProperty, float fScale, const char* szRadiusProperty, const nsColor& fixedColor = nsColorScheme::LightUI(nsColorScheme::Grape), const char* szColorProperty = nullptr);

  const nsUntrackedString& GetAngleProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRadiusProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty3; }

  nsEnum<nsBasisAxis> m_Axis;
  nsColor m_Color;
  float m_fScale;
};

//////////////////////////////////////////////////////////////////////////

class NS_FOUNDATION_DLL nsCameraVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsCameraVisualizerAttribute, nsVisualizerAttribute);

public:
  nsCameraVisualizerAttribute();

  /// \brief Attribute to add on an RTTI type to add a camera cone visualizer.
  nsCameraVisualizerAttribute(const char* szModeProperty, const char* szFovProperty, const char* szOrthoDimProperty, const char* szNearPlaneProperty, const char* szFarPlaneProperty);

  const nsUntrackedString& GetModeProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetFovProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetOrthoDimProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetNearPlaneProperty() const { return m_sProperty4; }
  const nsUntrackedString& GetFarPlaneProperty() const { return m_sProperty5; }
};

//////////////////////////////////////////////////////////////////////////

// Implementation moved here as it requires nsPropertyAttribute to be fully defined.
template <typename Type>
const Type* nsRTTI::GetAttributeByType() const
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
const Type* nsAbstractProperty::GetAttributeByType() const
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
class NS_FOUNDATION_DLL nsMaxArraySizeAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsMaxArraySizeAttribute, nsPropertyAttribute);

public:
  nsMaxArraySizeAttribute() = default;
  nsMaxArraySizeAttribute(nsUInt32 uiMaxSize) { m_uiMaxSize = uiMaxSize; }

  const nsUInt32& GetMaxSize() const { return m_uiMaxSize; }

private:
  nsUInt32 m_uiMaxSize = 0;
};

//////////////////////////////////////////////////////////////////////////

/// \brief If this attribute is set, the UI is encouraged to prevent the user from creating duplicates of the same thing.
///
/// For arrays of objects this means that multiple objects of the same type are not allowed.
class NS_FOUNDATION_DLL nsPreventDuplicatesAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsPreventDuplicatesAttribute, nsPropertyAttribute);

public:
  nsPreventDuplicatesAttribute() = default;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Attribute for types that should not be exposed to the scripting framework
class NS_FOUNDATION_DLL nsExcludeFromScript : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsExcludeFromScript, nsPropertyAttribute);
};

/// \brief Attribute to mark a function up to be exposed to the scripting system. Arguments specify the names of the function parameters.
class NS_FOUNDATION_DLL nsScriptableFunctionAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsScriptableFunctionAttribute, nsPropertyAttribute);

  enum ArgType : nsUInt8
  {
    In,
    Out,
    Inout
  };

  nsScriptableFunctionAttribute(ArgType argType1 = In, const char* szArg1 = nullptr, ArgType argType2 = In, const char* szArg2 = nullptr,
    ArgType argType3 = In, const char* szArg3 = nullptr, ArgType argType4 = In, const char* szArg4 = nullptr, ArgType argType5 = In,
    const char* szArg5 = nullptr, ArgType argType6 = In, const char* szArg6 = nullptr);

  nsUInt32 GetArgumentCount() const { return m_ArgNames.GetCount(); }
  const char* GetArgumentName(nsUInt32 uiIndex) const { return m_ArgNames[uiIndex]; }

  ArgType GetArgumentType(nsUInt32 uiIndex) const { return static_cast<ArgType>(m_ArgTypes[uiIndex]); }

private:
  nsHybridArray<nsUntrackedString, 6> m_ArgNames;
  nsHybridArray<nsUInt8, 6> m_ArgTypes;
};

/// \brief Wrapper Attribute to add an attribute to a function argument
class NS_FOUNDATION_DLL nsFunctionArgumentAttributes : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsFunctionArgumentAttributes, nsPropertyAttribute);

  nsFunctionArgumentAttributes() = default;
  nsFunctionArgumentAttributes(nsUInt32 uiArgIndex, const nsPropertyAttribute* pAttribute1, const nsPropertyAttribute* pAttribute2 = nullptr, const nsPropertyAttribute* pAttribute3 = nullptr, const nsPropertyAttribute* pAttribute4 = nullptr);
  ~nsFunctionArgumentAttributes();

  nsUInt32 GetArgumentIndex() const { return m_uiArgIndex; }
  nsArrayPtr<const nsPropertyAttribute* const> GetArgumentAttributes() const { return m_ArgAttributes; }

private:
  nsUInt32 m_uiArgIndex = 0;
  nsHybridArray<const nsPropertyAttribute*, 4> m_ArgAttributes;
};

/// \brief Used to mark an array or (unsigned)int property as source for dynamic pin generation on nodes
class NS_FOUNDATION_DLL nsDynamicPinAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsDynamicPinAttribute, nsPropertyAttribute);

public:
  nsDynamicPinAttribute() = default;
  nsDynamicPinAttribute(const char* szProperty);

  const nsUntrackedString& GetProperty() const { return m_sProperty; }

private:
  nsUntrackedString m_sProperty;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Used to mark that a component provides functionality that is executed with a long operation in the editor.
///
/// \a szOpTypeName must be the class name of a class derived from nsLongOpProxy.
/// Once a component is added to a scene with this attribute, the named long op will appear in the UI and can be executed.
///
/// The automatic registration is done by nsLongOpsAdapter
class NS_FOUNDATION_DLL nsLongOpAttribute : public nsPropertyAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsLongOpAttribute, nsPropertyAttribute);

public:
  nsLongOpAttribute() = default;
  nsLongOpAttribute(const char* szOpTypeName)
    : m_sOpTypeName(szOpTypeName)
  {
  }

  nsUntrackedString m_sOpTypeName;
};

//////////////////////////////////////////////////////////////////////////

/// \brief A property attribute that indicates that the string property is actually a game object reference.
class NS_FOUNDATION_DLL nsGameObjectReferenceAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsGameObjectReferenceAttribute, nsTypeWidgetAttribute);

public:
  nsGameObjectReferenceAttribute() = default;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Displays the value range as an image, allowing users to pick a value like on a slider.
///
/// This attribute always has to be combined with an nsClampValueAttribute to define the min and max value range.
/// The constructor takes the name of an image generator. The generator is used to build the QImage used for the slider background.
///
/// Image generators are registered through nsQtImageSliderWidget::s_ImageGenerators. Search the codebase for that variable
/// to determine which types of image generators are available.
/// You can register custom generators as well.
class NS_FOUNDATION_DLL nsImageSliderUiAttribute : public nsTypeWidgetAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsImageSliderUiAttribute, nsTypeWidgetAttribute);

public:
  nsImageSliderUiAttribute() = default;
  nsImageSliderUiAttribute(const char* szImageGenerator)
  {
    m_sImageGenerator = szImageGenerator;
  }

  nsUntrackedString m_sImageGenerator;
};
