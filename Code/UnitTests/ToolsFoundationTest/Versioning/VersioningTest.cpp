#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphVersioning.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>
#include <ToolsFoundation/Serialization/ToolsSerializationUtils.h>
#include <ToolsFoundationTest/Reflection/ReflectionTestClasses.h>

NS_CREATE_SIMPLE_TEST_GROUP(Versioning);

struct nsPatchTestBase
{
public:
  nsPatchTestBase()
  {
    m_string = "Base";
    m_string2 = "";
  }

  nsString m_string;
  nsString m_string2;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsPatchTestBase);

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsPatchTestBase, nsNoBase, 1, nsRTTIDefaultAllocator<nsPatchTestBase>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("String", m_string),
    NS_MEMBER_PROPERTY("String2", m_string2),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

struct nsPatchTest : public nsPatchTestBase
{
public:
  nsPatchTest() { m_iInt32 = 1; }

  nsInt32 m_iInt32;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsPatchTest);

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsPatchTest, nsPatchTestBase, 1, nsRTTIDefaultAllocator<nsPatchTest>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Int", m_iInt32),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

namespace
{
  /// Patch class
  class nsPatchTestP : public nsGraphPatch
  {
  public:
    nsPatchTestP()
      : nsGraphPatch("nsPatchTestP", 2)
    {
    }
    virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
    {
      pNode->RenameProperty("Int", "IntRenamed");
      pNode->ChangeProperty("IntRenamed", 2);
    }
  };
  nsPatchTestP g_nsPatchTestP;

  /// Patch base class
  class nsPatchTestBaseBP : public nsGraphPatch
  {
  public:
    nsPatchTestBaseBP()
      : nsGraphPatch("nsPatchTestBaseBP", 2)
    {
    }
    virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
    {
      pNode->ChangeProperty("String", "BaseClassPatched");
    }
  };
  nsPatchTestBaseBP g_nsPatchTestBaseBP;

  /// Rename class
  class nsPatchTestRN : public nsGraphPatch
  {
  public:
    nsPatchTestRN()
      : nsGraphPatch("nsPatchTestRN", 2)
    {
    }
    virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
    {
      ref_context.RenameClass("nsPatchTestRN2");
      pNode->ChangeProperty("String", "RenameExecuted");
    }
  };
  nsPatchTestRN g_nsPatchTestRN;

  /// Patch renamed class to v3
  class nsPatchTestRN2 : public nsGraphPatch
  {
  public:
    nsPatchTestRN2()
      : nsGraphPatch("nsPatchTestRN2", 3)
    {
    }
    virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
    {
      pNode->ChangeProperty("String2", "Patched");
    }
  };
  nsPatchTestRN2 g_nsPatchTestRN2;

  /// Change base class
  class nsPatchTestCB : public nsGraphPatch
  {
  public:
    nsPatchTestCB()
      : nsGraphPatch("nsPatchTestCB", 2)
    {
    }
    virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
    {
      nsVersionKey bases[] = {{"nsPatchTestBaseBP", 1}};
      ref_context.ChangeBaseClass(bases);
      pNode->ChangeProperty("String2", "ChangedBase");
    }
  };
  nsPatchTestCB g_nsPatchTestCB;

  void ReplaceTypeName(nsAbstractObjectGraph& ref_graph, nsAbstractObjectGraph& ref_typesGraph, const char* szOldName, const char* szNewName)
  {
    for (auto it : ref_graph.GetAllNodes())
    {
      auto* pNode = it.Value();

      if (szOldName == pNode->GetType())
        pNode->SetType(szNewName);
    }

    for (auto it : ref_typesGraph.GetAllNodes())
    {
      auto* pNode = it.Value();

      if ("nsReflectedTypeDescriptor" == pNode->GetType())
      {
        if (auto* pProp = pNode->FindProperty("TypeName"))
        {
          if (nsStringUtils::IsEqual(szOldName, pProp->m_Value.Get<nsString>()))
            pProp->m_Value = szNewName;
        }
        if (auto* pProp = pNode->FindProperty("ParentTypeName"))
        {
          if (nsStringUtils::IsEqual(szOldName, pProp->m_Value.Get<nsString>()))
            pProp->m_Value = szNewName;
        }
      }
    }
  }

  nsAbstractObjectNode* SerializeObject(nsAbstractObjectGraph& ref_graph, nsAbstractObjectGraph& ref_typesGraph, const nsRTTI* pRtti, void* pObject)
  {
    nsAbstractObjectNode* pNode = nullptr;
    {
      // Object
      nsRttiConverterContext context;
      nsRttiConverterWriter rttiConverter(&ref_graph, &context, true, true);
      context.RegisterObject(nsUuid::MakeStableUuidFromString(pRtti->GetTypeName()), pRtti, pObject);
      pNode = rttiConverter.AddObjectToGraph(pRtti, pObject, "ROOT");
    }
    {
      // Types
      nsSet<const nsRTTI*> types;
      types.Insert(pRtti);
      nsReflectionUtils::GatherDependentTypes(pRtti, types);
      nsToolsSerializationUtils::SerializeTypes(types, ref_typesGraph);
    }
    return pNode;
  }

  void PatchGraph(nsAbstractObjectGraph& ref_graph, nsAbstractObjectGraph& ref_typesGraph)
  {
    nsGraphVersioning::GetSingleton()->PatchGraph(&ref_typesGraph);
    nsGraphVersioning::GetSingleton()->PatchGraph(&ref_graph, &ref_typesGraph);
  }
} // namespace

NS_CREATE_SIMPLE_TEST(Versioning, GraphPatch)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "PatchClass")
  {
    nsAbstractObjectGraph graph;
    nsAbstractObjectGraph typesGraph;

    nsPatchTest data;
    data.m_iInt32 = 5;
    nsAbstractObjectNode* pNode = SerializeObject(graph, typesGraph, nsGetStaticRTTI<nsPatchTest>(), &data);
    ReplaceTypeName(graph, typesGraph, "nsPatchTest", "nsPatchTestP");
    PatchGraph(graph, typesGraph);

    nsAbstractObjectNode::Property* pInt = pNode->FindProperty("IntRenamed");
    NS_TEST_INT(2, pInt->m_Value.Get<nsInt32>());
    NS_TEST_BOOL(pNode->FindProperty("Int") == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PatchBaseClass")
  {
    nsAbstractObjectGraph graph;
    nsAbstractObjectGraph typesGraph;

    nsPatchTest data;
    data.m_string = "Unpatched";
    nsAbstractObjectNode* pNode = SerializeObject(graph, typesGraph, nsGetStaticRTTI<nsPatchTest>(), &data);
    ReplaceTypeName(graph, typesGraph, "nsPatchTestBase", "nsPatchTestBaseBP");
    PatchGraph(graph, typesGraph);

    nsAbstractObjectNode::Property* pString = pNode->FindProperty("String");
    NS_TEST_STRING(pString->m_Value.Get<nsString>(), "BaseClassPatched");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RenameClass")
  {
    nsAbstractObjectGraph graph;
    nsAbstractObjectGraph typesGraph;

    nsPatchTest data;
    data.m_string = "NotRenamed";
    nsAbstractObjectNode* pNode = SerializeObject(graph, typesGraph, nsGetStaticRTTI<nsPatchTest>(), &data);
    ReplaceTypeName(graph, typesGraph, "nsPatchTest", "nsPatchTestRN");
    PatchGraph(graph, typesGraph);

    nsAbstractObjectNode::Property* pString = pNode->FindProperty("String");
    NS_TEST_BOOL(pString->m_Value.Get<nsString>() == "RenameExecuted");
    NS_TEST_STRING(pNode->GetType(), "nsPatchTestRN2");
    NS_TEST_INT(pNode->GetTypeVersion(), 3);
    nsAbstractObjectNode::Property* pString2 = pNode->FindProperty("String2");
    NS_TEST_BOOL(pString2->m_Value.Get<nsString>() == "Patched");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeBaseClass")
  {
    nsAbstractObjectGraph graph;
    nsAbstractObjectGraph typesGraph;

    nsPatchTest data;
    data.m_string = "NotPatched";
    nsAbstractObjectNode* pNode = SerializeObject(graph, typesGraph, nsGetStaticRTTI<nsPatchTest>(), &data);
    ReplaceTypeName(graph, typesGraph, "nsPatchTest", "nsPatchTestCB");
    PatchGraph(graph, typesGraph);

    nsAbstractObjectNode::Property* pString = pNode->FindProperty("String");
    NS_TEST_STRING(pString->m_Value.Get<nsString>(), "BaseClassPatched");
    NS_TEST_INT(pNode->GetTypeVersion(), 2);
    nsAbstractObjectNode::Property* pString2 = pNode->FindProperty("String2");
    NS_TEST_STRING(pString2->m_Value.Get<nsString>(), "ChangedBase");
  }
}
