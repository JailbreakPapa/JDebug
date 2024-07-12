#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/DeduplicationReadContext.h>
#include <Foundation/IO/DeduplicationWriteContext.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Types/UniquePtr.h>

namespace
{
  struct RefCountedVec3 : public nsRefCounted
  {
    RefCountedVec3() = default;
    RefCountedVec3(const nsVec3& v)
      : m_v(v)
    {
    }

    nsResult Serialize(nsStreamWriter& inout_stream) const
    {
      inout_stream << m_v;
      return NS_SUCCESS;
    }

    nsResult Deserialize(nsStreamReader& inout_stream)
    {
      inout_stream >> m_v;
      return NS_SUCCESS;
    }

    nsVec3 m_v;
  };

  struct ComplexComponent
  {
    nsTransform* m_pTransform = nullptr;
    nsVec3* m_pPosition = nullptr;
    nsSharedPtr<RefCountedVec3> m_pScale;
    nsUInt32 m_uiIndex = nsInvalidIndex;

    nsResult Serialize(nsStreamWriter& inout_stream) const
    {
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteObject(inout_stream, m_pTransform));
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteObject(inout_stream, m_pPosition));
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteObject(inout_stream, m_pScale));

      inout_stream << m_uiIndex;
      return NS_SUCCESS;
    }

    nsResult Deserialize(nsStreamReader& inout_stream)
    {
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadObject(inout_stream, m_pTransform));
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadObject(inout_stream, m_pPosition));
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadObject(inout_stream, m_pScale));

      inout_stream >> m_uiIndex;
      return NS_SUCCESS;
    }
  };

  struct ComplexObject
  {
    nsDynamicArray<nsUniquePtr<nsTransform>> m_Transforms;
    nsDynamicArray<nsVec3> m_Positions;
    nsDynamicArray<nsSharedPtr<RefCountedVec3>> m_Scales;

    nsDynamicArray<ComplexComponent> m_Components;

    nsMap<nsUInt32, nsTransform*> m_TransformMap;
    nsSet<nsVec3*> m_UniquePositions;

    nsResult Serialize(nsStreamWriter& inout_stream) const
    {
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteArray(inout_stream, m_Transforms));
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteArray(inout_stream, m_Positions));
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteArray(inout_stream, m_Scales));
      NS_SUCCEED_OR_RETURN(
        nsDeduplicationWriteContext::GetContext()->WriteMap(inout_stream, m_TransformMap, nsDeduplicationWriteContext::WriteMapMode::DedupValue));
      NS_SUCCEED_OR_RETURN(nsDeduplicationWriteContext::GetContext()->WriteSet(inout_stream, m_UniquePositions));
      NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_Components));
      return NS_SUCCESS;
    }

    nsResult Deserialize(nsStreamReader& inout_stream)
    {
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadArray(inout_stream, m_Transforms));
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadArray(inout_stream, m_Positions,
        nullptr));                                                                                                       // should not allocate anything
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadArray(inout_stream, m_Scales));
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadMap(
        inout_stream, m_TransformMap, nsDeduplicationReadContext::ReadMapMode::DedupValue, nullptr, nullptr));           // should not allocate anything
      NS_SUCCEED_OR_RETURN(nsDeduplicationReadContext::GetContext()->ReadSet(inout_stream, m_UniquePositions, nullptr)); // should not allocate anything
      NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_Components));
      return NS_SUCCESS;
    }
  };
} // namespace

NS_CREATE_SIMPLE_TEST(IO, DeduplicationContext)
{
  nsDefaultMemoryStreamStorage streamStorage;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Writer")
  {
    nsMemoryStreamWriter writer(&streamStorage);

    nsDeduplicationWriteContext dedupWriteContext;

    ComplexObject obj;
    for (nsUInt32 i = 0; i < 20; ++i)
    {
      obj.m_Transforms.ExpandAndGetRef() = NS_DEFAULT_NEW(nsTransform, nsVec3(static_cast<float>(i), 0, 0));
      obj.m_Positions.ExpandAndGetRef() = nsVec3(1, 2, static_cast<float>(i));
      obj.m_Scales.ExpandAndGetRef() = NS_DEFAULT_NEW(RefCountedVec3, nsVec3(0, static_cast<float>(i), 0));
    }

    for (nsUInt32 i = 0; i < 10; ++i)
    {
      auto& component = obj.m_Components.ExpandAndGetRef();
      component.m_uiIndex = i * 2;
      component.m_pTransform = obj.m_Transforms[component.m_uiIndex].Borrow();
      component.m_pPosition = &obj.m_Positions[component.m_uiIndex];
      component.m_pScale = obj.m_Scales[component.m_uiIndex];

      obj.m_TransformMap.Insert(i, obj.m_Transforms[i].Borrow());
      obj.m_UniquePositions.Insert(&obj.m_Positions[i]);
    }



    NS_TEST_BOOL(obj.Serialize(writer).Succeeded());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reader")
  {
    nsMemoryStreamReader reader(&streamStorage);

    nsDeduplicationReadContext dedupReadContext;

    ComplexObject obj;
    NS_TEST_BOOL(obj.Deserialize(reader).Succeeded());

    NS_TEST_INT(obj.m_Transforms.GetCount(), 20);
    NS_TEST_INT(obj.m_Positions.GetCount(), 20);
    NS_TEST_INT(obj.m_Scales.GetCount(), 20);
    NS_TEST_INT(obj.m_TransformMap.GetCount(), 10);
    NS_TEST_INT(obj.m_UniquePositions.GetCount(), 10);
    NS_TEST_INT(obj.m_Components.GetCount(), 10);

    for (nsUInt32 i = 0; i < obj.m_Components.GetCount(); ++i)
    {
      auto& component = obj.m_Components[i];

      NS_TEST_BOOL(component.m_pTransform == obj.m_Transforms[component.m_uiIndex].Borrow());
      NS_TEST_BOOL(component.m_pPosition == &obj.m_Positions[component.m_uiIndex]);
      NS_TEST_BOOL(component.m_pScale == obj.m_Scales[component.m_uiIndex]);

      NS_TEST_BOOL(component.m_pTransform->m_vPosition == nsVec3(static_cast<float>(i) * 2, 0, 0));
      NS_TEST_BOOL(*component.m_pPosition == nsVec3(1, 2, static_cast<float>(i) * 2));
      NS_TEST_BOOL(component.m_pScale->m_v == nsVec3(0, static_cast<float>(i) * 2, 0));
    }

    for (nsUInt32 i = 0; i < 10; ++i)
    {
      if (NS_TEST_BOOL(obj.m_TransformMap.GetValue(i) != nullptr))
      {
        NS_TEST_BOOL(*obj.m_TransformMap.GetValue(i) == obj.m_Transforms[i].Borrow());
      }

      NS_TEST_BOOL(obj.m_UniquePositions.Contains(&obj.m_Positions[i]));
    }
  }
}
