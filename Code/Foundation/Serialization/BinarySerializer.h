#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

class NS_FOUNDATION_DLL nsAbstractGraphBinarySerializer
{
public:
  static void Write(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph = nullptr);                // [tested]
  static void Read(nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = false); // [tested]

private:
};
