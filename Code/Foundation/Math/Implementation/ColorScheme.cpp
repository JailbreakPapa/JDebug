#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/ColorScheme.h>

wdColor wdColorScheme::s_Colors[Count][10] = {
  {
    wdColorGammaUB(201, 42, 42),   // oc-red-9
    wdColorGammaUB(224, 49, 49),   // oc-red-8
    wdColorGammaUB(240, 62, 62),   // oc-red-7
    wdColorGammaUB(250, 82, 82),   // oc-red-6
    wdColorGammaUB(255, 107, 107), // oc-red-5
    wdColorGammaUB(255, 135, 135), // oc-red-4
    wdColorGammaUB(255, 168, 168), // oc-red-3
    wdColorGammaUB(255, 201, 201), // oc-red-2
    wdColorGammaUB(255, 227, 227), // oc-red-1
    wdColorGammaUB(255, 245, 245), // oc-red-0
  },
  {
    wdColorGammaUB(166, 30, 77),   // oc-pink-9
    wdColorGammaUB(194, 37, 92),   // oc-pink-8
    wdColorGammaUB(214, 51, 108),  // oc-pink-7
    wdColorGammaUB(230, 73, 128),  // oc-pink-6
    wdColorGammaUB(240, 101, 149), // oc-pink-5
    wdColorGammaUB(247, 131, 172), // oc-pink-4
    wdColorGammaUB(250, 162, 193), // oc-pink-3
    wdColorGammaUB(252, 194, 215), // oc-pink-2
    wdColorGammaUB(255, 222, 235), // oc-pink-1
    wdColorGammaUB(255, 240, 246), // oc-pink-0
  },
  {
    wdColorGammaUB(134, 46, 156),  // oc-grape-9
    wdColorGammaUB(156, 54, 181),  // oc-grape-8
    wdColorGammaUB(174, 62, 201),  // oc-grape-7
    wdColorGammaUB(190, 75, 219),  // oc-grape-6
    wdColorGammaUB(204, 93, 232),  // oc-grape-5
    wdColorGammaUB(218, 119, 242), // oc-grape-4
    wdColorGammaUB(229, 153, 247), // oc-grape-3
    wdColorGammaUB(238, 190, 250), // oc-grape-2
    wdColorGammaUB(243, 217, 250), // oc-grape-1
    wdColorGammaUB(248, 240, 252), // oc-grape-0
  },
  {
    wdColorGammaUB(95, 61, 196),   // oc-violet-9
    wdColorGammaUB(103, 65, 217),  // oc-violet-8
    wdColorGammaUB(112, 72, 232),  // oc-violet-7
    wdColorGammaUB(121, 80, 242),  // oc-violet-6
    wdColorGammaUB(132, 94, 247),  // oc-violet-5
    wdColorGammaUB(151, 117, 250), // oc-violet-4
    wdColorGammaUB(177, 151, 252), // oc-violet-3
    wdColorGammaUB(208, 191, 255), // oc-violet-2
    wdColorGammaUB(229, 219, 255), // oc-violet-1
    wdColorGammaUB(243, 240, 255), // oc-violet-0
  },
  {
    wdColorGammaUB(54, 79, 199),   // oc-indigo-9
    wdColorGammaUB(59, 91, 219),   // oc-indigo-8
    wdColorGammaUB(66, 99, 235),   // oc-indigo-7
    wdColorGammaUB(76, 110, 245),  // oc-indigo-6
    wdColorGammaUB(92, 124, 250),  // oc-indigo-5
    wdColorGammaUB(116, 143, 252), // oc-indigo-4
    wdColorGammaUB(145, 167, 255), // oc-indigo-3
    wdColorGammaUB(186, 200, 255), // oc-indigo-2
    wdColorGammaUB(219, 228, 255), // oc-indigo-1
    wdColorGammaUB(237, 242, 255), // oc-indigo-0
  },
  {
    wdColorGammaUB(24, 100, 171),  // oc-blue-9
    wdColorGammaUB(25, 113, 194),  // oc-blue-8
    wdColorGammaUB(28, 126, 214),  // oc-blue-7
    wdColorGammaUB(34, 139, 230),  // oc-blue-6
    wdColorGammaUB(51, 154, 240),  // oc-blue-5
    wdColorGammaUB(77, 171, 247),  // oc-blue-4
    wdColorGammaUB(116, 192, 252), // oc-blue-3
    wdColorGammaUB(165, 216, 255), // oc-blue-2
    wdColorGammaUB(208, 235, 255), // oc-blue-1
    wdColorGammaUB(231, 245, 255), // oc-blue-0
  },
  {
    wdColorGammaUB(11, 114, 133),  // oc-cyan-9
    wdColorGammaUB(12, 133, 153),  // oc-cyan-8
    wdColorGammaUB(16, 152, 173),  // oc-cyan-7
    wdColorGammaUB(21, 170, 191),  // oc-cyan-6
    wdColorGammaUB(34, 184, 207),  // oc-cyan-5
    wdColorGammaUB(59, 201, 219),  // oc-cyan-4
    wdColorGammaUB(102, 217, 232), // oc-cyan-3
    wdColorGammaUB(153, 233, 242), // oc-cyan-2
    wdColorGammaUB(197, 246, 250), // oc-cyan-1
    wdColorGammaUB(227, 250, 252), // oc-cyan-0
  },
  {
    wdColorGammaUB(8, 127, 91),    // oc-teal-9
    wdColorGammaUB(9, 146, 104),   // oc-teal-8
    wdColorGammaUB(12, 166, 120),  // oc-teal-7
    wdColorGammaUB(18, 184, 134),  // oc-teal-6
    wdColorGammaUB(32, 201, 151),  // oc-teal-5
    wdColorGammaUB(56, 217, 169),  // oc-teal-4
    wdColorGammaUB(99, 230, 190),  // oc-teal-3
    wdColorGammaUB(150, 242, 215), // oc-teal-2
    wdColorGammaUB(195, 250, 232), // oc-teal-1
    wdColorGammaUB(230, 252, 245), // oc-teal-0
  },
  {
    wdColorGammaUB(43, 138, 62),   // oc-green-9
    wdColorGammaUB(47, 158, 68),   // oc-green-8
    wdColorGammaUB(55, 178, 77),   // oc-green-7
    wdColorGammaUB(64, 192, 87),   // oc-green-6
    wdColorGammaUB(81, 207, 102),  // oc-green-5
    wdColorGammaUB(105, 219, 124), // oc-green-4
    wdColorGammaUB(140, 233, 154), // oc-green-3
    wdColorGammaUB(178, 242, 187), // oc-green-2
    wdColorGammaUB(211, 249, 216), // oc-green-1
    wdColorGammaUB(235, 251, 238), // oc-green-0
  },
  {
    wdColorGammaUB(92, 148, 13),   // oc-lime-9
    wdColorGammaUB(102, 168, 15),  // oc-lime-8
    wdColorGammaUB(116, 184, 22),  // oc-lime-7
    wdColorGammaUB(130, 201, 30),  // oc-lime-6
    wdColorGammaUB(148, 216, 45),  // oc-lime-5
    wdColorGammaUB(169, 227, 75),  // oc-lime-4
    wdColorGammaUB(192, 235, 117), // oc-lime-3
    wdColorGammaUB(216, 245, 162), // oc-lime-2
    wdColorGammaUB(233, 250, 200), // oc-lime-1
    wdColorGammaUB(244, 252, 227), // oc-lime-0
  },
  {
    wdColorGammaUB(230, 119, 0),   // oc-yellow-9
    wdColorGammaUB(240, 140, 0),   // oc-yellow-8
    wdColorGammaUB(245, 159, 0),   // oc-yellow-7
    wdColorGammaUB(250, 176, 5),   // oc-yellow-6
    wdColorGammaUB(252, 196, 25),  // oc-yellow-5
    wdColorGammaUB(255, 212, 59),  // oc-yellow-4
    wdColorGammaUB(255, 224, 102), // oc-yellow-3
    wdColorGammaUB(255, 236, 153), // oc-yellow-2
    wdColorGammaUB(255, 243, 191), // oc-yellow-1
    wdColorGammaUB(255, 249, 219), // oc-yellow-0
  },
  {
    wdColorGammaUB(217, 72, 15),   // oc-orange-9
    wdColorGammaUB(232, 89, 12),   // oc-orange-8
    wdColorGammaUB(247, 103, 7),   // oc-orange-7
    wdColorGammaUB(253, 126, 20),  // oc-orange-6
    wdColorGammaUB(255, 146, 43),  // oc-orange-5
    wdColorGammaUB(255, 169, 77),  // oc-orange-4
    wdColorGammaUB(255, 192, 120), // oc-orange-3
    wdColorGammaUB(255, 216, 168), // oc-orange-2
    wdColorGammaUB(255, 232, 204), // oc-orange-1
    wdColorGammaUB(255, 244, 230), // oc-orange-0
  },
  {
    wdColorGammaUB(33, 37, 41),    // oc-gray-9
    wdColorGammaUB(52, 58, 64),    // oc-gray-8
    wdColorGammaUB(73, 80, 87),    // oc-gray-7
    wdColorGammaUB(134, 142, 150), // oc-gray-6
    wdColorGammaUB(173, 181, 189), // oc-gray-5
    wdColorGammaUB(206, 212, 218), // oc-gray-4
    wdColorGammaUB(222, 226, 230), // oc-gray-3
    wdColorGammaUB(233, 236, 239), // oc-gray-2
    wdColorGammaUB(241, 243, 245), // oc-gray-1
    wdColorGammaUB(248, 249, 250), // oc-gray-0
  },
};

// We could use a lower brightness here for our dark UI but the colors looks much nicer at higher brightness so we just apply a scale factor instead.
static constexpr wdUInt8 DarkUIBrightness = 3;
static constexpr wdUInt8 DarkUIGrayBrightness = 4; // gray is too dark at UIBrightness
static constexpr float DarkUISaturation = 0.95f;
static constexpr wdColor DarkUIFactor = wdColor(0.5f, 0.5f, 0.5f, 1.0f);
wdColor wdColorScheme::s_DarkUIColors[Count] = {
  GetColor(wdColorScheme::Red, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Pink, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Grape, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Violet, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Indigo, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Blue, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Cyan, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Teal, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Green, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Lime, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Yellow, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Orange, DarkUIBrightness, DarkUISaturation) * DarkUIFactor,
  GetColor(wdColorScheme::Gray, DarkUIGrayBrightness, DarkUISaturation) * DarkUIFactor,
};

static constexpr wdUInt8 LightUIBrightness = 4;
static constexpr wdUInt8 LightUIGrayBrightness = 5; // gray is too dark at UIBrightness
static constexpr float LightUISaturation = 1.0f;
wdColor wdColorScheme::s_LightUIColors[Count] = {
  GetColor(wdColorScheme::Red, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Pink, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Grape, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Violet, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Indigo, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Blue, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Cyan, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Teal, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Green, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Lime, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Yellow, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Orange, LightUIBrightness, LightUISaturation),
  GetColor(wdColorScheme::Gray, LightUIGrayBrightness, LightUISaturation),
};

// static
wdColor wdColorScheme::GetColor(float fIndex, wdUInt8 uiBrightness, float fSaturation /*= 1.0f*/, float fAlpha /*= 1.0f*/)
{
  wdUInt32 uiIndexA, uiIndexB;
  float fFrac;
  GetInterpolation(fIndex, uiIndexA, uiIndexB, fFrac);

  const wdColor a = s_Colors[uiIndexA][uiBrightness];
  const wdColor b = s_Colors[uiIndexB][uiBrightness];
  const wdColor c = wdMath::Lerp(a, b, fFrac);
  const float l = c.GetLuminance();
  return wdMath::Lerp(wdColor(l, l, l), c, fSaturation).WithAlpha(fAlpha);
}


WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_ColorScheme);
