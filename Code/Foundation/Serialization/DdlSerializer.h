#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/UniquePtr.h>

class wdOpenDdlReaderElement;

struct WD_FOUNDATION_DLL wdSerializedBlock
{
  wdString m_Name;
  wdUniquePtr<wdAbstractObjectGraph> m_Graph;
};

class WD_FOUNDATION_DLL wdAbstractGraphDdlSerializer
{
public:
  static void Write(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph = nullptr, bool bCompactMmode = true, wdOpenDdlWriter::TypeStringMode typeMode = wdOpenDdlWriter::TypeStringMode::Shortest);
  static wdResult Read(wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = true);

  static void Write(wdOpenDdlWriter& inout_stream, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypesGraph = nullptr);
  static wdResult Read(const wdOpenDdlReaderElement* pRootElement, wdAbstractObjectGraph* pGraph, wdAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = true);

  static void WriteDocument(wdStreamWriter& inout_stream, const wdAbstractObjectGraph* pHeader, const wdAbstractObjectGraph* pGraph, const wdAbstractObjectGraph* pTypes, bool bCompactMode = true, wdOpenDdlWriter::TypeStringMode typeMode = wdOpenDdlWriter::TypeStringMode::Shortest);
  static wdResult ReadDocument(wdStreamReader& inout_stream, wdUniquePtr<wdAbstractObjectGraph>& ref_pHeader, wdUniquePtr<wdAbstractObjectGraph>& ref_pGraph, wdUniquePtr<wdAbstractObjectGraph>& ref_pTypes, bool bApplyPatches = true);

  static wdResult ReadHeader(wdStreamReader& inout_stream, wdAbstractObjectGraph* pGraph);

private:
  static wdResult ReadBlocks(wdStreamReader& stream, wdHybridArray<wdSerializedBlock, 3>& blocks);
};
