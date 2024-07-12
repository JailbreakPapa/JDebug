#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/ColorScheme.h>

nsColor nsColorScheme::s_Colors[Count][10] = {
  {
    nsColorGammaUB(201, 42, 42),   // oc-red-9
    nsColorGammaUB(224, 49, 49),   // oc-red-8
    nsColorGammaUB(240, 62, 62),   // oc-red-7
    nsColorGammaUB(250, 82, 82),   // oc-red-6
    nsColorGammaUB(255, 107, 107), // oc-red-5
    nsColorGammaUB(255, 135, 135), // oc-red-4
    nsColorGammaUB(255, 168, 168), // oc-red-3
    nsColorGammaUB(255, 201, 201), // oc-red-2
    nsColorGammaUB(255, 227, 227), // oc-red-1
    nsColorGammaUB(255, 245, 245), // oc-red-0
  },
  {
    nsColorGammaUB(166, 30, 77),   // oc-pink-9
    nsColorGammaUB(194, 37, 92),   // oc-pink-8
    nsColorGammaUB(214, 51, 108),  // oc-pink-7
    nsColorGammaUB(230, 73, 128),  // oc-pink-6
    nsColorGammaUB(240, 101, 149), // oc-pink-5
    nsColorGammaUB(247, 131, 172), // oc-pink-4
    nsColorGammaUB(250, 162, 193), // oc-pink-3
    nsColorGammaUB(252, 194, 215), // oc-pink-2
    nsColorGammaUB(255, 222, 235), // oc-pink-1
    nsColorGammaUB(255, 240, 246), // oc-pink-0
  },
  {
    nsColorGammaUB(134, 46, 156),  // oc-grape-9
    nsColorGammaUB(156, 54, 181),  // oc-grape-8
    nsColorGammaUB(174, 62, 201),  // oc-grape-7
    nsColorGammaUB(190, 75, 219),  // oc-grape-6
    nsColorGammaUB(204, 93, 232),  // oc-grape-5
    nsColorGammaUB(218, 119, 242), // oc-grape-4
    nsColorGammaUB(229, 153, 247), // oc-grape-3
    nsColorGammaUB(238, 190, 250), // oc-grape-2
    nsColorGammaUB(243, 217, 250), // oc-grape-1
    nsColorGammaUB(248, 240, 252), // oc-grape-0
  },
  {
    nsColorGammaUB(95, 61, 196),   // oc-violet-9
    nsColorGammaUB(103, 65, 217),  // oc-violet-8
    nsColorGammaUB(112, 72, 232),  // oc-violet-7
    nsColorGammaUB(121, 80, 242),  // oc-violet-6
    nsColorGammaUB(132, 94, 247),  // oc-violet-5
    nsColorGammaUB(151, 117, 250), // oc-violet-4
    nsColorGammaUB(177, 151, 252), // oc-violet-3
    nsColorGammaUB(208, 191, 255), // oc-violet-2
    nsColorGammaUB(229, 219, 255), // oc-violet-1
    nsColorGammaUB(243, 240, 255), // oc-violet-0
  },
  {
    nsColorGammaUB(54, 79, 199),   // oc-indigo-9
    nsColorGammaUB(59, 91, 219),   // oc-indigo-8
    nsColorGammaUB(66, 99, 235),   // oc-indigo-7
    nsColorGammaUB(76, 110, 245),  // oc-indigo-6
    nsColorGammaUB(92, 124, 250),  // oc-indigo-5
    nsColorGammaUB(116, 143, 252), // oc-indigo-4
    nsColorGammaUB(145, 167, 255), // oc-indigo-3
    nsColorGammaUB(186, 200, 255), // oc-indigo-2
    nsColorGammaUB(219, 228, 255), // oc-indigo-1
    nsColorGammaUB(237, 242, 255), // oc-indigo-0
  },
  {
    nsColorGammaUB(24, 100, 171),  // oc-blue-9
    nsColorGammaUB(25, 113, 194),  // oc-blue-8
    nsColorGammaUB(28, 126, 214),  // oc-blue-7
    nsColorGammaUB(34, 139, 230),  // oc-blue-6
    nsColorGammaUB(51, 154, 240),  // oc-blue-5
    nsColorGammaUB(77, 171, 247),  // oc-blue-4
    nsColorGammaUB(116, 192, 252), // oc-blue-3
    nsColorGammaUB(165, 216, 255), // oc-blue-2
    nsColorGammaUB(208, 235, 255), // oc-blue-1
    nsColorGammaUB(231, 245, 255), // oc-blue-0
  },
  {
    nsColorGammaUB(11, 114, 133),  // oc-cyan-9
    nsColorGammaUB(12, 133, 153),  // oc-cyan-8
    nsColorGammaUB(16, 152, 173),  // oc-cyan-7
    nsColorGammaUB(21, 170, 191),  // oc-cyan-6
    nsColorGammaUB(34, 184, 207),  // oc-cyan-5
    nsColorGammaUB(59, 201, 219),  // oc-cyan-4
    nsColorGammaUB(102, 217, 232), // oc-cyan-3
    nsColorGammaUB(153, 233, 242), // oc-cyan-2
    nsColorGammaUB(197, 246, 250), // oc-cyan-1
    nsColorGammaUB(227, 250, 252), // oc-cyan-0
  },
  {
    nsColorGammaUB(8, 127, 91),    // oc-teal-9
    nsColorGammaUB(9, 146, 104),   // oc-teal-8
    nsColorGammaUB(12, 166, 120),  // oc-teal-7
    nsColorGammaUB(18, 184, 134),  // oc-teal-6
    nsColorGammaUB(32, 201, 151),  // oc-teal-5
    nsColorGammaUB(56, 217, 169),  // oc-teal-4
    nsColorGammaUB(99, 230, 190),  // oc-teal-3
    nsColorGammaUB(150, 242, 215), // oc-teal-2
    nsColorGammaUB(195, 250, 232), // oc-teal-1
    nsColorGammaUB(230, 252, 245), // oc-teal-0
  },
  {
    nsColorGammaUB(43, 138, 62),   // oc-green-9
    nsColorGammaUB(47, 158, 68),   // oc-green-8
    nsColorGammaUB(55, 178, 77),   // oc-green-7
    nsColorGammaUB(64, 192, 87),   // oc-green-6
    nsColorGammaUB(81, 207, 102),  // oc-green-5
    nsColorGammaUB(105, 219, 124), // oc-green-4
    nsColorGammaUB(140, 233, 154), // oc-green-3
    nsColorGammaUB(178, 242, 187), // oc-green-2
    nsColorGammaUB(211, 249, 216), // oc-green-1
    nsColorGammaUB(235, 251, 238), // oc-green-0
  },
  {
    nsColorGammaUB(92, 148, 13),   // oc-lime-9
    nsColorGammaUB(102, 168, 15),  // oc-lime-8
    nsColorGammaUB(116, 184, 22),  // oc-lime-7
    nsColorGammaUB(130, 201, 30),  // oc-lime-6
    nsColorGammaUB(148, 216, 45),  // oc-lime-5
    nsColorGammaUB(169, 227, 75),  // oc-lime-4
    nsColorGammaUB(192, 235, 117), // oc-lime-3
    nsColorGammaUB(216, 245, 162), // oc-lime-2
    nsColorGammaUB(233, 250, 200), // oc-lime-1
    nsColorGammaUB(244, 252, 227), // oc-lime-0
  },
  {
    nsColorGammaUB(230, 119, 0),   // oc-yellow-9
    nsColorGammaUB(240, 140, 0),   // oc-yellow-8
    nsColorGammaUB(245, 159, 0),   // oc-yellow-7
    nsColorGammaUB(250, 176, 5),   // oc-yellow-6
    nsColorGammaUB(252, 196, 25),  // oc-yellow-5
    nsColorGammaUB(255, 212, 59),  // oc-yellow-4
    nsColorGammaUB(255, 224, 102), // oc-yellow-3
    nsColorGammaUB(255, 236, 153), // oc-yellow-2
    nsColorGammaUB(255, 243, 191), // oc-yellow-1
    nsColorGammaUB(255, 249, 219), // oc-yellow-0
  },
  {
    nsColorGammaUB(217, 72, 15),   // oc-orange-9
    nsColorGammaUB(232, 89, 12),   // oc-orange-8
    nsColorGammaUB(247, 103, 7),   // oc-orange-7
    nsColorGammaUB(253, 126, 20),  // oc-orange-6
    nsColorGammaUB(255, 146, 43),  // oc-orange-5
    nsColorGammaUB(255, 169, 77),  // oc-orange-4
    nsColorGammaUB(255, 192, 120), // oc-orange-3
    nsColorGammaUB(255, 216, 168), // oc-orange-2
    nsColorGammaUB(255, 232, 204), // oc-orange-1
    nsColorGammaUB(255, 244, 230), // oc-orange-0
  },
  {
    nsColorGammaUB(33, 37, 41),    // oc-gray-9
    nsColorGammaUB(52, 58, 64),    // oc-gray-8
    nsColorGammaUB(73, 80, 87),    // oc-gray-7
    nsColorGammaUB(134, 142, 150), // oc-gray-6
    nsColorGammaUB(173, 181, 189), // oc-gray-5
    nsColorGammaUB(206, 212, 218), // oc-gray-4
    nsColorGammaUB(222, 226, 230), // oc-gray-3
    nsColorGammaUB(233, 236, 239), // oc-gray-2
    nsColorGammaUB(241, 243, 245), // oc-gray-1
    nsColorGammaUB(248, 249, 250), // oc-gray-0
  },
};

// We could use a lower brightness here for our dark UI but the colors looks much nicer at higher brightness so we just apply a scale factor instead.
static constexpr nsUInt8 DarkUIBrightness = 3;
static constexpr nsUInt8 DarkUIGrayBrightness = 4; // gray is too dark at UIBrightness
static constexpr float DarkUISaturation = 0.95f;
static constexpr nsColor DarkUIFactor = nsColor(0.5f, 0.5f, 0.5f, 1.0f);
nsColor nsColorScheme::s_DarkUIColors[Count] = {
  GetColor(nsColorScheme::Red, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Pink, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Grape, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Violet, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Indigo, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Blue, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Cyan, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Teal, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Green, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Lime, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Yellow, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Orange, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(nsColorScheme::Gray, DarkUIGrayBrightness, DarkUISaturation) * DarkUIFactor,
};

static constexpr nsUInt8 LightUIBrightness = 4;
static constexpr nsUInt8 LightUIGrayBrightness = 5; // gray is too dark at UIBrightness
static constexpr float LightUISaturation = 1.0f;
nsColor nsColorScheme::s_LightUIColors[Count] = {
  GetColor(nsColorScheme::Red, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Pink, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Grape, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Violet, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Indigo, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Blue, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Cyan, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Teal, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Green, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Lime, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Yellow, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Orange, LightUIBrightness, LightUISaturation),
  GetColor(nsColorScheme::Gray, LightUIGrayBrightness, LightUISaturation),
};

// static
nsColor nsColorScheme::GetColor(float fIndex, nsUInt8 uiBrightness, float fSaturation /*= 1.0f*/, float fAlpha /*= 1.0f*/)
{
  nsUInt32 uiIndexA, uiIndexB;
  float fFrac;
  GetInterpolation(fIndex, uiIndexA, uiIndexB, fFrac);

  const nsColor a = s_Colors[uiIndexA][uiBrightness];
  const nsColor b = s_Colors[uiIndexB][uiBrightness];
  const nsColor c = nsMath::Lerp(a, b, fFrac);
  const float l = c.GetLuminance();
  return nsMath::Lerp(nsColor(l, l, l), c, fSaturation).WithAlpha(fAlpha);
}

nsColorScheme::CategoryColorFunc nsColorScheme::s_CategoryColorFunc = nullptr;

nsColor nsColorScheme::GetCategoryColor(nsStringView sCategory, CategoryColorUsage usage)
{
  if (s_CategoryColorFunc != nullptr)
  {
    return s_CategoryColorFunc(sCategory, usage);
  }

  nsInt8 iBrightnessOffset = -3;
  nsUInt8 uiSaturationStep = 0;

  if (usage == nsColorScheme::CategoryColorUsage::BorderIconColor)
  {
    // don't color these icons at all
    return nsColor::MakeZero();
  }

  if (usage == nsColorScheme::CategoryColorUsage::MenuEntryIcon || usage == nsColorScheme::CategoryColorUsage::AssetMenuIcon)
  {
    iBrightnessOffset = 2;
    uiSaturationStep = 0;
  }
  else if (usage == nsColorScheme::CategoryColorUsage::ViewportIcon)
  {
    iBrightnessOffset = 2;
    uiSaturationStep = 2;
  }
  else if (usage == nsColorScheme::CategoryColorUsage::OverlayIcon)
  {
    iBrightnessOffset = 2;
    uiSaturationStep = 0;
  }
  else if (usage == nsColorScheme::CategoryColorUsage::SceneTreeIcon)
  {
    iBrightnessOffset = 2;
    uiSaturationStep = 0;
  }
  else if (usage == nsColorScheme::CategoryColorUsage::BorderColor)
  {
    iBrightnessOffset = -3;
    uiSaturationStep = 0;
  }

  const nsUInt8 uiBrightness = (nsUInt8)nsMath::Clamp<nsInt32>(DarkUIBrightness + iBrightnessOffset, 0, 9);
  const float fSaturation = DarkUISaturation - (uiSaturationStep * 0.2f);

  if (const char* sep = sCategory.FindSubString("/"))
  {
    // chop off everything behind the first separator
    sCategory = nsStringView(sCategory.GetStartPointer(), sep);
  }

  if (sCategory.IsEqual_NoCase("AI"))
    return nsColorScheme::GetColor(nsColorScheme::Cyan, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Animation"))
    return nsColorScheme::GetColor(nsColorScheme::Pink, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Construction"))
    return nsColorScheme::GetColor(nsColorScheme::Orange, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Custom") || sCategory.IsEqual_NoCase("Game"))
    return nsColorScheme::GetColor(nsColorScheme::Red, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Effects"))
    return nsColorScheme::GetColor(nsColorScheme::Grape, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Gameplay"))
    return nsColorScheme::GetColor(nsColorScheme::Indigo, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Input"))
    return nsColorScheme::GetColor(nsColorScheme::Red, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Lighting"))
    return nsColorScheme::GetColor(nsColorScheme::Violet, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Logic"))
    return nsColorScheme::GetColor(nsColorScheme::Teal, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Physics"))
    return nsColorScheme::GetColor(nsColorScheme::Blue, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Prefabs"))
    return nsColorScheme::GetColor(nsColorScheme::Orange, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Rendering"))
    return nsColorScheme::GetColor(nsColorScheme::Lime, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Terrain"))
    return nsColorScheme::GetColor(nsColorScheme::Lime, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Scripting"))
    return nsColorScheme::GetColor(nsColorScheme::Green, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Sound"))
    return nsColorScheme::GetColor(nsColorScheme::Blue, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("Utilities") || sCategory.IsEqual_NoCase("Editing"))
    return nsColorScheme::GetColor(nsColorScheme::Gray, uiBrightness, fSaturation) * DarkUIFactor;

  if (sCategory.IsEqual_NoCase("XR"))
    return nsColorScheme::GetColor(nsColorScheme::Cyan, uiBrightness, fSaturation) * DarkUIFactor;

  nsLog::Warning("Color for category '{}' is undefined.", sCategory);
  return nsColor::MakeZero();
}
