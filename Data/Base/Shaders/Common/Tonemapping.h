#pragma once

#include "Common.h"

//==========================================================================================
// LINEAR
//==========================================================================================
float3 ToneMapping_Linear(float3 color)
{
  return color;
}

//==========================================================================================
// UNCHARTED 2
//==========================================================================================
float3 ToneMapping_Uncharted2(float3 x)
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;

  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

//==========================================================================================
// REINHARD
//==========================================================================================
float3 ToneMapping_Reinhard(float3 hdr, float k = 1.0f)
{
  return hdr / (hdr + k);
}

float3 ToneMapping_Reinhard_Inverse(float3 sdr, float k = 1.0f)
{
  return k * sdr / (k - sdr);
}

//==========================================================================================
// ACES
//==========================================================================================

//  Baking Lab
//  by MJP and David Neubelt
//  http://mynameismjp.wordpress.com/
//  All code licensed under the MIT license

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 aces_mat_input =
  {
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 aces_mat_output =
  {
    {1.60475, -0.53108, -0.07367},
    {-0.10208, 1.10813, -0.00605},
    {-0.00327, -0.07276, 1.07602}};

float3 RRTAndODTFit(float3 v)
{
  float3 a = v * (v + 0.0245786f) - 0.000090537f;
  float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
  return a / b;
}

float3 ToneMapping_ACES(float3 color)
{
  color = mul(aces_mat_input, color);

  // Apply RRT and ODT
  color = RRTAndODTFit(color);

  color = mul(aces_mat_output, color);

  // Clamp to [0, 1]
  color = saturate(color);

  return color;
}

//==========================================================================================
// AMD
//==========================================================================================

// General tonemapping operator, build 'b' term.
float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
  return -((-pow(midIn, contrast) + (midOut * (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) -
                                                pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut)) /
                                      (pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut)) /
           (pow(midIn, contrast * shoulder) * midOut));
}

// General tonemapping operator, build 'c' term.
float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
  return (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) - pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut) /
         (pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut);
}

// General tonemapping operator, p := {contrast,shoulder,b,c}.
float ColTone(float x, float4 p)
{
  float z = pow(x, p.r);
  return z / (pow(z, p.g) * p.b + p.a);
}

float3 ToneMapping_AMD(float3 color)
{
  const float hdrMax = 16.0;        // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
  const float contrast = 2.0;       // Use as a baseline to tune the amount of contrast the tonemapper has.
  const float shoulder = 1.0;       // Likely don't need to mess with this factor, unless matching existing tonemapper is not working well..
  const float midIn = 0.18;         // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
  const float midOut = 0.18 / 16.0; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.

  float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
  float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

  float peak = max(color.r, max(color.g, color.b));
  peak = max(FLT_MIN, peak);

  float3 ratio = color / peak;
  peak = ColTone(peak, float4(contrast, shoulder, b, c));
  // then process ratio

  // probably want send these pre-computed (so send over saturation/crossSaturation as a constant)
  float crosstalk = 4.0;                   // controls amount of channel crosstalk
  float saturation = contrast;             // full tonal range saturation control
  float crossSaturation = contrast * 16.0; // crosstalk saturation

  float white = 1.0;

  // wrap crosstalk in transform
  float ratio_temp = saturation / crossSaturation;
  float pow_temp = pow(peak, crosstalk);
  ratio = pow(abs(ratio), float3(ratio_temp, ratio_temp, ratio_temp));
  ratio = lerp(ratio, float3(white, white, white), float3(pow_temp, pow_temp, pow_temp));
  ratio = pow(abs(ratio), float3(crossSaturation, crossSaturation, crossSaturation));

  // then apply ratio to peak
  color = peak * ratio;
  return color;
}