#pragma once

#include <Foundation/Math/Math.h>

namespace nsMath
{
  NS_ALWAYS_INLINE double GetCurveValue_Linear(double t)
  {
    return t;
  }

  NS_ALWAYS_INLINE double GetCurveValue_ConstantZero(double t)
  {
    return 0.0;
  }

  NS_ALWAYS_INLINE double GetCurveValue_ConstantOne(double t)
  {
    return 1.0;
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInSine(double t)
  {
    return 1.0 - cos((t * nsMath::Pi<double>()) / 2.0);
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseOutSine(double t)
  {
    return sin((t * nsMath::Pi<double>()) / 2.0);
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutSine(double t)
  {
    return -(cos(nsMath::Pi<double>() * t) - 1.0) / 2.0;
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInQuad(double t)
  {
    return t * t;
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseOutQuad(double t)
  {
    return 1.0 - (1.0 - t) * (1.0 - t);
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutQuad(double t)
  {
    return t < 0.5 ? 2.0 * t * t : 1.0 - nsMath::Pow(-2.0 * t + 2, 2.0) / 2;
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInCubic(double t)
  {
    return t * t * t;
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseOutCubic(double t)
  {
    return 1.0 - pow(1 - t, 3.0);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutCubic(double t)
  {
    return t < 0.5 ? 4.0 * t * t * t : 1.0 - nsMath::Pow(-2.0 * t + 2.0, 3.0) / 2.0;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInQuartic(double t)
  {
    return t * t * t * t;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutQuartic(double t)
  {
    return 1.0 - nsMath::Pow(1.0 - t, 4.0);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutQuartic(double t)
  {
    return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - nsMath::Pow(-2.0 * t + 2.0, 4.0) / 2.0;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInQuintic(double t)
  {
    return t * t * t * t * t;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutQuintic(double t)
  {
    return 1.0 - nsMath::Pow(1.0 - t, 5.0);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutQuintic(double t)
  {
    return t < 0.5 ? 16.0 * t * t * t * t * t : 1.0 - nsMath::Pow(-2.0 * t + 2.0, 5.0) / 2.0;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInExpo(double t)
  {
    return t == 0 ? 0 : nsMath::Pow(2.0, 10.0 * t - 10.0);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutExpo(double t)
  {
    return t == 1.0 ? 1.0 : 1.0 - nsMath::Pow(2.0, -10.0 * t);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutExpo(double t)
  {
    if (t == 0.0)
    {
      return 0.0;
    }
    else if (t == 1.0)
    {
      return 1.0;
    }
    else
    {
      return t < 0.5 ? nsMath::Pow(2.0, 20.0 * t - 10.0) / 2.0
                     : (2.0 - nsMath::Pow(2.0, -20.0 * t + 10.0)) / 2.0;
    }
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInCirc(double t)
  {
    return 1.0 - nsMath::Sqrt(1.0 - nsMath::Pow(t, 2.0));
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutCirc(double t)
  {
    return nsMath::Sqrt(1.0 - nsMath::Pow(t - 1.0, 2.0));
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutCirc(double t)
  {
    return t < 0.5
             ? (1.0 - nsMath::Sqrt(1.0 - nsMath::Pow(2.0 * t, 2.0))) / 2.0
             : (nsMath::Sqrt(1.0 - nsMath::Pow(-2.0 * t + 2.0, 2.0)) + 1.0) / 2.0;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInBack(double t)
  {
    return 2.70158 * t * t * t - 1.70158 * t * t;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutBack(double t)
  {
    return 1 + 2.70158 * nsMath::Pow(t - 1.0, 3.0) + 1.70158 * nsMath::Pow(t - 1.0, 2.0);
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutBack(double t)
  {
    return t < 0.5
             ? (nsMath::Pow(2.0 * t, 2.0) * (((1.70158 * 1.525) + 1.0) * 2 * t - (1.70158 * 1.525))) / 2.0
             : (nsMath::Pow(2.0 * t - 2.0, 2.0) * (((1.70158 * 1.525) + 1.0) * (t * 2.0 - 2.0) + (1.70158 * 1.525)) + 2.0) / 2.0;
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseInElastic(double t)
  {
    if (t == 0.0)
    {
      return 0.0;
    }
    else if (t == 1.0)
    {
      return 1.0;
    }
    else
    {
      return -nsMath::Pow(2.0, 10.0 * t - 10.0) * sin((t * 10.0 - 10.75) * ((2.0 * nsMath::Pi<double>()) / 3.0));
    }
  }


  NS_ALWAYS_INLINE double GetCurveValue_EaseOutElastic(double t)
  {
    if (t == 0.0)
    {
      return 0.0;
    }
    else if (t == 1.0)
    {
      return 1.0;
    }
    else
    {
      return nsMath::Pow(2.0, -10.0 * t) * sin((t * 10.0 - 0.75) * ((2.0 * nsMath::Pi<double>()) / 3.0)) + 1.0;
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutElastic(double t)
  {
    if (t == 0.0)
    {
      return 0.0;
    }
    else if (t == 1.0)
    {
      return 1.0;
    }
    else
    {
      return t < 0.5
               ? -(nsMath::Pow(2.0, 20.0 * t - 10.0) * sin((20.0 * t - 11.125) * ((2 * nsMath::Pi<double>()) / 4.5))) / 2.0
               : (nsMath::Pow(2.0, -20.0 * t + 10.0) * sin((20.0 * t - 11.125) * ((2 * nsMath::Pi<double>()) / 4.5))) / 2.0 + 1.0;
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInBounce(double t)
  {
    return 1.0 - GetCurveValue_EaseOutBounce(1.0 - t);
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseOutBounce(double t)
  {
    if (t < 1.0 / 2.75)
    {
      return 7.5625 * t * t;
    }
    else if (t < 2.0 / 2.75)
    {
      t -= 1.5 / 2.75;
      return 7.5625 * t * t + 0.75;
    }
    else if (t < 2.5 / 2.75)
    {
      t -= 2.25 / 2.75;
      return 7.5625 * t * t + 0.9375;
    }
    else
    {
      t -= 2.625 / 2.75;
      return 7.5625 * t * t + 0.984375;
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_EaseInOutBounce(double t)
  {
    return t < 0.5
             ? (1.0 - GetCurveValue_EaseOutBounce(1.0 - 2.0 * t)) / 2.0
             : (1.0 + GetCurveValue_EaseOutBounce(2.0 * t - 1.0)) / 2.0;
  }

  NS_ALWAYS_INLINE double GetCurveValue_Conical(double t)
  {
    if (t < 0.2)
    {
      return 1.0f - nsMath::Pow(1.0 - (t * 5.0), 4.0);
    }
    else
    {
      t = (t - 0.2) / 0.8; // normalize to 0-1 range

      return 1.0 - nsMath::Pow(t, 2.0);
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_FadeInHoldFadeOut(double t)
  {
    if (t < 0.2)
    {
      return 1.0f - nsMath::Pow(1.0 - (t * 5.0), 3.0);
    }
    else if (t > 0.8)
    {
      return 1.0 - nsMath::Pow((t - 0.8) * 5.0, 3.0);
    }
    else
    {
      return 1.0;
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_FadeInFadeOut(double t)
  {
    if (t < 0.5)
    {
      return 1.0f - nsMath::Pow(1.0 - (t * 2.0), 3.0);
    }
    else
    {
      return 1.0 - nsMath::Pow((t - 0.5) * 2.0, 3.0);
    }
  }

  NS_ALWAYS_INLINE double GetCurveValue_Bell(double t)
  {
    if (t < 0.25)
    {
      return (nsMath::Pow((t * 4.0), 3.0)) * 0.5;
    }
    else if (t < 0.5)
    {
      return (1.0f - nsMath::Pow(1.0 - ((t - 0.25) * 4.0), 3.0)) * 0.5 + 0.5;
    }
    else if (t < 0.75)
    {
      return (1.0f - nsMath::Pow(((t - 0.5) * 4.0), 3.0)) * 0.5 + 0.5;
    }
    else
    {
      return (nsMath::Pow(1.0 - ((t - 0.75) * 4.0), 3.0)) * 0.5;
    }
  }
} // namespace nsMath

// static
inline double nsCurveFunction::GetValue(Enum function, double x)
{
  switch (function)
  {
    case Linear:
      return nsMath::GetCurveValue_Linear(x);
    case ConstantZero:
      return nsMath::GetCurveValue_ConstantZero(x);
    case ConstantOne:
      return nsMath::GetCurveValue_ConstantOne(x);
    case EaseInSine:
      return nsMath::GetCurveValue_EaseInSine(x);
    case EaseOutSine:
      return nsMath::GetCurveValue_EaseOutSine(x);
    case EaseInOutSine:
      return nsMath::GetCurveValue_EaseInOutSine(x);
    case EaseInQuad:
      return nsMath::GetCurveValue_EaseInQuad(x);
    case EaseOutQuad:
      return nsMath::GetCurveValue_EaseOutQuad(x);
    case EaseInOutQuad:
      return nsMath::GetCurveValue_EaseInOutQuad(x);
    case EaseInCubic:
      return nsMath::GetCurveValue_EaseInCubic(x);
    case EaseOutCubic:
      return nsMath::GetCurveValue_EaseOutCubic(x);
    case EaseInOutCubic:
      return nsMath::GetCurveValue_EaseInOutCubic(x);
    case EaseInQuartic:
      return nsMath::GetCurveValue_EaseInQuartic(x);
    case EaseOutQuartic:
      return nsMath::GetCurveValue_EaseOutQuartic(x);
    case EaseInOutQuartic:
      return nsMath::GetCurveValue_EaseInOutQuartic(x);
    case EaseInQuintic:
      return nsMath::GetCurveValue_EaseInQuintic(x);
    case EaseOutQuintic:
      return nsMath::GetCurveValue_EaseOutQuintic(x);
    case EaseInOutQuintic:
      return nsMath::GetCurveValue_EaseInOutQuintic(x);
    case EaseInExpo:
      return nsMath::GetCurveValue_EaseInExpo(x);
    case EaseOutExpo:
      return nsMath::GetCurveValue_EaseOutExpo(x);
    case EaseInOutExpo:
      return nsMath::GetCurveValue_EaseInOutExpo(x);
    case EaseInCirc:
      return nsMath::GetCurveValue_EaseInCirc(x);
    case EaseOutCirc:
      return nsMath::GetCurveValue_EaseOutCirc(x);
    case EaseInOutCirc:
      return nsMath::GetCurveValue_EaseInOutCirc(x);
    case EaseInBack:
      return nsMath::GetCurveValue_EaseInBack(x);
    case EaseOutBack:
      return nsMath::GetCurveValue_EaseOutBack(x);
    case EaseInOutBack:
      return nsMath::GetCurveValue_EaseInOutBack(x);
    case EaseInElastic:
      return nsMath::GetCurveValue_EaseInElastic(x);
    case EaseOutElastic:
      return nsMath::GetCurveValue_EaseOutElastic(x);
    case EaseInOutElastic:
      return nsMath::GetCurveValue_EaseInOutElastic(x);
    case EaseInBounce:
      return nsMath::GetCurveValue_EaseInBounce(x);
    case EaseOutBounce:
      return nsMath::GetCurveValue_EaseOutBounce(x);
    case EaseInOutBounce:
      return nsMath::GetCurveValue_EaseInOutBounce(x);
    case Conical:
      return nsMath::GetCurveValue_Conical(x);
    case FadeInHoldFadeOut:
      return nsMath::GetCurveValue_FadeInHoldFadeOut(x);
    case FadeInFadeOut:
      return nsMath::GetCurveValue_FadeInFadeOut(x);
    case Bell:
      return nsMath::GetCurveValue_Bell(x);

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return 0.0;
}

// static
inline double nsCurveFunction::GetValue(Enum function, double x, bool bInverse)
{
  double value = GetValue(function, x);

  return bInverse ? (1.0 - value) : value;
}
