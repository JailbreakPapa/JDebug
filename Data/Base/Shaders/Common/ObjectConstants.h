#pragma once

#include "ConstantBufferMacros.h"
#include "Platforms.h"

struct WD_SHADER_STRUCT wdPerInstanceData
{
  TRANSFORM(ObjectToWorld);
  TRANSFORM(ObjectToWorldNormal);
  FLOAT1(BoundingSphereRadius);
  UINT1(GameObjectID);
  UINT1(VertexColorAccessData);

  INT1(Reserved);
  COLOR4F(Color);
};

#if WD_ENABLED(PLATFORM_SHADER)
StructuredBuffer<wdPerInstanceData> perInstanceData;

#  if defined(USE_SKINNING)
StructuredBuffer<Transform> skinningTransforms;
#  endif

Buffer<uint> perInstanceVertexColors;

#else // C++

WD_DEFINE_AS_POD_TYPE(wdPerInstanceData);

WD_CHECK_AT_COMPILETIME(sizeof(wdPerInstanceData) == 128);
#endif

CONSTANT_BUFFER(wdObjectConstants, 2)
{
  UINT1(InstanceDataOffset);
};



#if WD_ENABLED(PLATFORM_SHADER)

// Access to instance should usually go through this macro!
// It's a macro so it can work with arbitrary input structs (for VS/GS/PS...)
#  if defined(CAMERA_MODE) && CAMERA_MODE == CAMERA_MODE_STEREO
#    define GetInstanceData() perInstanceData[G.Input.InstanceID / 2 + InstanceDataOffset]
#  else
#    define GetInstanceData() perInstanceData[G.Input.InstanceID + InstanceDataOffset]
#  endif

#  define VERTEX_COLOR_ACCESS_OFFSET_BITS 28
#  define VERTEX_COLOR_ACCESS_OFFSET_MASK ((1 << VERTEX_COLOR_ACCESS_OFFSET_BITS) - 1)

uint GetNumInstanceVertexColorsHelper(uint accessData)
{
  return accessData >> VERTEX_COLOR_ACCESS_OFFSET_BITS;
}

uint GetInstanceVertexColorsHelper(uint accessData, uint vertexID, uint colorIndex)
{
  uint numColorsPerVertex = GetNumInstanceVertexColorsHelper(accessData);
  uint offset = (accessData & VERTEX_COLOR_ACCESS_OFFSET_MASK) + (vertexID * numColorsPerVertex + colorIndex);
  return colorIndex < numColorsPerVertex ? perInstanceVertexColors[offset] : 0;
}

#  define GetNumInstanceVertexColors() GetNumInstanceVertexColorsHelper(GetInstanceData().VertexColorAccessData)
#  define GetInstanceVertexColors(colorIndex) GetInstanceVertexColorsHelper(GetInstanceData().VertexColorAccessData, G.Input.VertexID, colorIndex)

#endif
