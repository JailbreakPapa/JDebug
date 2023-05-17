#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

class WD_FOUNDATION_DLL wdAbstractGraphBinarySerializer
{
public:
  static void Write(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph = nullptr);                // [tested]
  static void Read(wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = false); // [tested]

private:
};
