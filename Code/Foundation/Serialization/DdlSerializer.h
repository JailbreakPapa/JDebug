#pragma once

/// \file

#include <Foundation/Basics.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Types/UniquePtr.h>

class nsOpenDdlReaderElement;

struct NS_FOUNDATION_DLL nsSerializedBlock
{
  nsString m_Name;
  nsUniquePtr<nsAbstractObjectGraph> m_Graph;
};

class NS_FOUNDATION_DLL nsAbstractGraphDdlSerializer
{
public:
  static void Write(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph = nullptr, bool bCompactMmode = true, nsOpenDdlWriter::TypeStringMode typeMode = nsOpenDdlWriter::TypeStringMode::Shortest);
  static nsResult Read(nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = true);

  static void Write(nsOpenDdlWriter& inout_stream, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypesGraph = nullptr);
  static nsResult Read(const nsOpenDdlReaderElement* pRootElement, nsAbstractObjectGraph* pGraph, nsAbstractObjectGraph* pTypesGraph = nullptr, bool bApplyPatches = true);

  static void WriteDocument(nsStreamWriter& inout_stream, const nsAbstractObjectGraph* pHeader, const nsAbstractObjectGraph* pGraph, const nsAbstractObjectGraph* pTypes, bool bCompactMode = true, nsOpenDdlWriter::TypeStringMode typeMode = nsOpenDdlWriter::TypeStringMode::Shortest);
  static nsResult ReadDocument(nsStreamReader& inout_stream, nsUniquePtr<nsAbstractObjectGraph>& ref_pHeader, nsUniquePtr<nsAbstractObjectGraph>& ref_pGraph, nsUniquePtr<nsAbstractObjectGraph>& ref_pTypes, bool bApplyPatches = true);

  static nsResult ReadHeader(nsStreamReader& inout_stream, nsAbstractObjectGraph* pGraph);

private:
  static nsResult ReadBlocks(nsStreamReader& stream, nsHybridArray<nsSerializedBlock, 3>& blocks);
};
