#include <Shaders/Common/GlobalConstants.h>

CONSTANT_BUFFER(wdMaterialConstants, 0)
{
  MAT4(ViewMatrix);
};

#if WD_ENABLED(PLATFORM_SHADER)

struct VS_IN
{
  float3 Position : POSITION;
};

struct VS_OUT
{
  float4 Position : SV_Position;
  float2 FragCoord : TEXCOORD0;
};

typedef VS_OUT PS_IN;

#endif
