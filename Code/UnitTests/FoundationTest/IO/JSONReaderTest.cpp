#include <FoundationTest/FoundationTestPCH.h>

// NOTE: always save as Unicode UTF-8 with signature

#include <Foundation/Containers/Deque.h>
#include <Foundation/IO/JSONReader.h>

namespace JSONReaderTestDetail
{

  class StringStream : public nsStreamReader
  {
  public:
    StringStream(const void* pData)
    {
      m_pData = pData;
      m_uiLength = nsStringUtils::GetStringElementCount((const char*)pData);
    }

    virtual nsUInt64 ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
    {
      uiBytesToRead = nsMath::Min(uiBytesToRead, m_uiLength);
      m_uiLength -= uiBytesToRead;

      if (uiBytesToRead > 0)
      {
        nsMemoryUtils::Copy((nsUInt8*)pReadBuffer, (nsUInt8*)m_pData, (size_t)uiBytesToRead);
        m_pData = nsMemoryUtils::AddByteOffset(m_pData, (ptrdiff_t)uiBytesToRead);
      }

      return uiBytesToRead;
    }

  private:
    const void* m_pData;
    nsUInt64 m_uiLength;
  };

  void TraverseTree(const nsVariant& var, nsDeque<nsString>& ref_compare)
  {
    if (ref_compare.IsEmpty())
      return;

    switch (var.GetType())
    {
      case nsVariant::Type::VariantDictionary:
      {
        // nsLog::Printf("Expect: %s - Is: %s\n", "<object>", Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), "<object>");
        ref_compare.PopFront();

        const nsVariantDictionary& vd = var.Get<nsVariantDictionary>();

        for (auto it = vd.GetIterator(); it.IsValid(); ++it)
        {
          if (ref_compare.IsEmpty())
            return;

          // nsLog::Printf("Expect: %s - Is: %s\n", it.Key().GetData(), Compare.PeekFront().GetData());
          NS_TEST_STRING(ref_compare.PeekFront().GetData(), it.Key().GetData());
          ref_compare.PopFront();

          TraverseTree(it.Value(), ref_compare);
        }

        if (ref_compare.IsEmpty())
          return;

        NS_TEST_STRING(ref_compare.PeekFront().GetData(), "</object>");
        // nsLog::Printf("Expect: %s - Is: %s\n", "</object>", Compare.PeekFront().GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::VariantArray:
      {
        // nsLog::Printf("Expect: %s - Is: %s\n", "<array>", Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), "<array>");
        ref_compare.PopFront();

        const nsVariantArray& va = var.Get<nsVariantArray>();

        for (nsUInt32 i = 0; i < va.GetCount(); ++i)
        {
          TraverseTree(va[i], ref_compare);
        }

        if (ref_compare.IsEmpty())
          return;

        // nsLog::Printf("Expect: %s - Is: %s\n", "</array>", Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), "</array>");
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Invalid:
        // nsLog::Printf("Expect: %s - Is: %s\n", "null", Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), "null");
        ref_compare.PopFront();
        break;

      case nsVariant::Type::Bool:
        // nsLog::Printf("Expect: %s - Is: %s\n", var.Get<bool>() ? "bool true" : "bool false", Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), var.Get<bool>() ? "bool true" : "bool false");
        ref_compare.PopFront();
        break;

      case nsVariant::Type::Int8:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("int8 {0}", var.Get<nsInt8>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::UInt8:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("uint8 {0}", var.Get<nsUInt8>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Int16:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("int16 {0}", var.Get<nsInt16>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::UInt16:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("uint16 {0}", var.Get<nsUInt16>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Int32:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("int32 {0}", var.Get<nsInt32>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::UInt32:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("uint32 {0}", var.Get<nsUInt32>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Int64:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("int64 {0}", var.Get<nsInt64>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::UInt64:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("uint64 {0}", var.Get<nsUInt64>());
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Float:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("float {0}", nsArgF(var.Get<float>(), 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Double:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("double {0}", nsArgF(var.Get<double>(), 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Time:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("time {0}", nsArgF(var.Get<nsTime>().GetSeconds(), 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Angle:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("angle {0}", nsArgF(var.Get<nsAngle>().GetDegree(), 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::String:
        // nsLog::Printf("Expect: %s - Is: %s\n", var.Get<nsString>().GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), var.Get<nsString>().GetData());
        ref_compare.PopFront();
        break;

      case nsVariant::Type::StringView:
        // nsLog::Printf("Expect: %s - Is: %s\n", var.Get<nsString>().GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront(), var.Get<nsStringView>());
        ref_compare.PopFront();
        break;

      case nsVariant::Type::Vector2:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec2 ({0}, {1})", nsArgF(var.Get<nsVec2>().x, 4), nsArgF(var.Get<nsVec2>().y, 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Vector3:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec3 ({0}, {1}, {2})", nsArgF(var.Get<nsVec3>().x, 4), nsArgF(var.Get<nsVec3>().y, 4), nsArgF(var.Get<nsVec3>().z, 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Vector4:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec4 ({0}, {1}, {2}, {3})", nsArgF(var.Get<nsVec4>().x, 4), nsArgF(var.Get<nsVec4>().y, 4), nsArgF(var.Get<nsVec4>().z, 4), nsArgF(var.Get<nsVec4>().w, 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Vector2I:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec2i ({0}, {1})", var.Get<nsVec2I32>().x, var.Get<nsVec2I32>().y);
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Vector3I:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec3i ({0}, {1}, {2})", var.Get<nsVec3I32>().x, var.Get<nsVec3I32>().y, var.Get<nsVec3I32>().z);
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Vector4I:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("vec4i ({0}, {1}, {2}, {3})", var.Get<nsVec4I32>().x, var.Get<nsVec4I32>().y, var.Get<nsVec4I32>().z, var.Get<nsVec4I32>().w);
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Color:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("color ({0}, {1}, {2}, {3})", nsArgF(var.Get<nsColor>().r, 4), nsArgF(var.Get<nsColor>().g, 4), nsArgF(var.Get<nsColor>().b, 4), nsArgF(var.Get<nsColor>().a, 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::ColorGamma:
      {
        nsStringBuilder sTemp;
        const nsColorGammaUB c = var.ConvertTo<nsColorGammaUB>();

        sTemp.SetFormat("gamma ({0}, {1}, {2}, {3})", c.r, c.g, c.b, c.a);
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Quaternion:
      {
        nsStringBuilder sTemp;
        sTemp.SetFormat("quat ({0}, {1}, {2}, {3})", nsArgF(var.Get<nsQuat>().x, 4), nsArgF(var.Get<nsQuat>().y, 4), nsArgF(var.Get<nsQuat>().z, 4), nsArgF(var.Get<nsQuat>().w, 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Matrix3:
      {
        nsMat3 m = var.Get<nsMat3>();

        nsStringBuilder sTemp;
        sTemp.SetFormat("mat3 ({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8})", nsArgF(m.m_fElementsCM[0], 4), nsArgF(m.m_fElementsCM[1], 4), nsArgF(m.m_fElementsCM[2], 4), nsArgF(m.m_fElementsCM[3], 4), nsArgF(m.m_fElementsCM[4], 4), nsArgF(m.m_fElementsCM[5], 4), nsArgF(m.m_fElementsCM[6], 4), nsArgF(m.m_fElementsCM[7], 4), nsArgF(m.m_fElementsCM[8], 4));
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Matrix4:
      {
        nsMat4 m = var.Get<nsMat4>();

        nsStringBuilder sTemp;
        sTemp.SetPrintf("mat4 (%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f)", m.m_fElementsCM[0], m.m_fElementsCM[1], m.m_fElementsCM[2], m.m_fElementsCM[3], m.m_fElementsCM[4], m.m_fElementsCM[5], m.m_fElementsCM[6], m.m_fElementsCM[7], m.m_fElementsCM[8], m.m_fElementsCM[9], m.m_fElementsCM[10], m.m_fElementsCM[11], m.m_fElementsCM[12], m.m_fElementsCM[13], m.m_fElementsCM[14], m.m_fElementsCM[15]);
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      case nsVariant::Type::Uuid:
      {
        nsUuid uuid = var.Get<nsUuid>();
        nsStringBuilder sTemp;
        nsConversionUtils::ToString(uuid, sTemp);
        sTemp.Prepend("uuid ");
        // nsLog::Printf("Expect: %s - Is: %s\n", sTemp.GetData(), Compare.PeekFront().GetData());
        NS_TEST_STRING(ref_compare.PeekFront().GetData(), sTemp.GetData());
        ref_compare.PopFront();
      }
      break;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
        break;
    }
  }
} // namespace JSONReaderTestDetail

NS_CREATE_SIMPLE_TEST(IO, JSONReader)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Test")
  {
    nsStringUtf8 sTD(L"{\n\
\"myarray2\":[\"\",2.2],\n\
\"myarray\" : [1, 2.2, 3.3, false, \"ende\" ],\n\
\"String\"/**/ : \"testvälue\",\n\
\"double\"/***/ : 43.56,//comment\n\
\"float\" :/**//*a*/ 64/*comment*/.720001,\n\
\"bool\" : tr/*asdf*/ue,\n\
\"int\" : 23,\n\
\"MyNüll\" : nu/*asdf*/ll,\n\
\"object\" :\n\
/* totally \n weird \t stuff \n\n\n going on here // thats a line comment \n */ \
// more line comments \n\n\n\n\
{\n\
  \"variable in object\" : \"bla\\\\\\\"\\/\",\n\
    \"Subobject\" :\n\
  {\n\
    \"variable in subobject\" : \"blub\\r\\f\\n\\b\\t\",\n\
      \"array in sub\" : [\n\
    {\n\
      \"obj var\" : 234\n\
            /*stuff ] */ \
    },\n\
    {\n\
      \"obj var 2\" : -235\n//breakingcomment\n\
    }, true, 4, false ]\n\
  }\n\
},\n\
\"test\" : \"text\"\n\
}");
    const char* szTestData = sTD.GetData();

    // NOTE: The way this test is implemented, it might break, if the HashMap uses another insertion algorithm.
    // nsVariantDictionary is an nsHashmap and this test currently relies on one exact order in of the result.
    // If this should ever change (or be arbitrary at runtime), the test needs to be implemented in a more robust way.

    JSONReaderTestDetail::StringStream stream(szTestData);

    nsJSONReader reader;
    NS_TEST_BOOL(reader.Parse(stream).Succeeded());

    nsDeque<nsString> sCompare;
    sCompare.PushBack("<object>");
    sCompare.PushBack("int");
    sCompare.PushBack("double 23.0000");
    sCompare.PushBack("String");
    sCompare.PushBack(nsStringUtf8(L"testvälue").GetData()); // unicode literal

    sCompare.PushBack("double");
    sCompare.PushBack("double 43.5600");

    sCompare.PushBack("myarray");
    sCompare.PushBack("<array>");
    sCompare.PushBack("double 1.0000");
    sCompare.PushBack("double 2.2000");
    sCompare.PushBack("double 3.3000");
    sCompare.PushBack("bool false");
    sCompare.PushBack("ende");
    sCompare.PushBack("</array>");

    sCompare.PushBack("object");
    sCompare.PushBack("<object>");

    sCompare.PushBack("Subobject");
    sCompare.PushBack("<object>");

    sCompare.PushBack("array in sub");
    sCompare.PushBack("<array>");

    sCompare.PushBack("<object>");
    sCompare.PushBack("obj var");
    sCompare.PushBack("double 234.0000");
    sCompare.PushBack("</object>");

    sCompare.PushBack("<object>");
    sCompare.PushBack("obj var 2");
    sCompare.PushBack("double -235.0000");
    sCompare.PushBack("</object>");

    sCompare.PushBack("bool true");
    sCompare.PushBack("double 4.0000");
    sCompare.PushBack("bool false");

    sCompare.PushBack("</array>");


    sCompare.PushBack("variable in subobject");
    sCompare.PushBack("blub\r\f\n\b\t"); // escaped special characters

    sCompare.PushBack("</object>");

    sCompare.PushBack("variable in object");
    sCompare.PushBack("bla\\\"/"); // escaped backslash, quotation mark, slash

    sCompare.PushBack("</object>");

    sCompare.PushBack("float");
    sCompare.PushBack("double 64.7200");

    sCompare.PushBack("myarray2");
    sCompare.PushBack("<array>");
    sCompare.PushBack("");
    sCompare.PushBack("double 2.2000");
    sCompare.PushBack("</array>");

    sCompare.PushBack(nsStringUtf8(L"MyNüll").GetData()); // unicode literal
    sCompare.PushBack("null");

    sCompare.PushBack("test");
    sCompare.PushBack("text");

    sCompare.PushBack("bool");
    sCompare.PushBack("bool true");

    sCompare.PushBack("</object>");

    if (NS_TEST_BOOL(reader.GetTopLevelElementType() == nsJSONReader::ElementType::Dictionary))
    {
      JSONReaderTestDetail::TraverseTree(reader.GetTopLevelObject(), sCompare);

      NS_TEST_BOOL(sCompare.IsEmpty());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Array document")
  {
    const char* szTestData = "[\"a\",\"b\"]";

    // NOTE: The way this test is implemented, it might break, if the HashMap uses another insertion algorithm.
    // nsVariantDictionary is an nsHashmap and this test currently relies on one exact order in of the result.
    // If this should ever change (or be arbitrary at runtime), the test needs to be implemented in a more robust way.

    JSONReaderTestDetail::StringStream stream(szTestData);

    nsJSONReader reader;
    NS_TEST_BOOL(reader.Parse(stream).Succeeded());

    nsDeque<nsString> sCompare;
    sCompare.PushBack("<array>");
    sCompare.PushBack("a");
    sCompare.PushBack("b");
    sCompare.PushBack("</array>");

    if (NS_TEST_BOOL(reader.GetTopLevelElementType() == nsJSONReader::ElementType::Array))
    {
      JSONReaderTestDetail::TraverseTree(reader.GetTopLevelArray(), sCompare);

      NS_TEST_BOOL(sCompare.IsEmpty());
    }
  }
}
