#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#include <Foundation/Types/Uuid.h>
#include <ToolsFoundation/Object/ObjectMetaData.h>


NS_CREATE_SIMPLE_TEST(DocumentObject, ObjectMetaData)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Pointers / int")
  {
    nsObjectMetaData<void*, nsInt32> meta;

    int a = 0, b = 1, c = 2, d = 3;

    NS_TEST_BOOL(!meta.HasMetaData(&a));
    NS_TEST_BOOL(!meta.HasMetaData(&b));
    NS_TEST_BOOL(!meta.HasMetaData(&c));
    NS_TEST_BOOL(!meta.HasMetaData(&d));

    {
      auto pData = meta.BeginModifyMetaData(&a);
      *pData = a;
      meta.EndModifyMetaData();

      pData = meta.BeginModifyMetaData(&b);
      *pData = b;
      meta.EndModifyMetaData();

      pData = meta.BeginModifyMetaData(&c);
      *pData = c;
      meta.EndModifyMetaData();
    }

    NS_TEST_BOOL(meta.HasMetaData(&a));
    NS_TEST_BOOL(meta.HasMetaData(&b));
    NS_TEST_BOOL(meta.HasMetaData(&c));
    NS_TEST_BOOL(!meta.HasMetaData(&d));

    {
      auto pDataR = meta.BeginReadMetaData(&a);
      NS_TEST_INT(*pDataR, a);
      meta.EndReadMetaData();

      pDataR = meta.BeginReadMetaData(&b);
      NS_TEST_INT(*pDataR, b);
      meta.EndReadMetaData();

      pDataR = meta.BeginReadMetaData(&c);
      NS_TEST_INT(*pDataR, c);
      meta.EndReadMetaData();

      pDataR = meta.BeginReadMetaData(&d);
      NS_TEST_INT(*pDataR, 0);
      meta.EndReadMetaData();
    }
  }

  struct md
  {
    md() { b = false; }

    nsString s;
    bool b;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UUID / struct")
  {
    nsObjectMetaData<nsUuid, md> meta;

    const int num = 100;

    nsDynamicArray<nsUuid> obj;
    obj.SetCount(num);

    for (nsUInt32 i = 0; i < num; ++i)
    {
      nsUuid& uid = obj[i];
      uid = nsUuid::MakeUuid();

      if (nsMath::IsEven(i))
      {
        auto d1 = meta.BeginModifyMetaData(uid);
        d1->b = true;
        d1->s = "test";

        meta.EndModifyMetaData();
      }

      NS_TEST_BOOL(meta.HasMetaData(uid) == nsMath::IsEven(i));
    }

    for (nsUInt32 i = 0; i < num; ++i)
    {
      const nsUuid& uid = obj[i];

      auto p = meta.BeginReadMetaData(uid);

      NS_TEST_BOOL(p->b == nsMath::IsEven(i));

      if (nsMath::IsEven(i))
      {
        NS_TEST_STRING(p->s, "test");
      }
      else
      {
        NS_TEST_BOOL(p->s.IsEmpty());
      }

      meta.EndReadMetaData();
      NS_TEST_BOOL(meta.HasMetaData(uid) == nsMath::IsEven(i));
    }
  }
}
