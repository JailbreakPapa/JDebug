#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/JSONWriter.h>
#include <Foundation/IO/OSFile.h>
#include <FoundationTest/IO/JSONTestHelpers.h>


NS_CREATE_SIMPLE_TEST(IO, StandardJSONWriter)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Object")
  {
    StreamComparer sc("\"TestObject\" : {\n\
  \n\
}");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("TestObject");
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Anonymous Object")
  {
    StreamComparer sc("{\n\
  \n\
}");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject();
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableBool")
  {
    StreamComparer sc("\"var1\" : true,\n\"var2\" : false");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableBool("var1", true);
    js.AddVariableBool("var2", false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableInt32")
  {
    StreamComparer sc("\"var1\" : 23,\n\"var2\" : -42");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableInt32("var1", 23);
    js.AddVariableInt32("var2", -42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableUInt32")
  {
    StreamComparer sc("\"var1\" : 23,\n\"var2\" : 42");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableUInt32("var1", 23);
    js.AddVariableUInt32("var2", 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableInt64")
  {
    StreamComparer sc("\"var1\" : 23,\n\"var2\" : -42");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableInt64("var1", 23);
    js.AddVariableInt64("var2", -42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableUInt64")
  {
    StreamComparer sc("\"var1\" : 23,\n\"var2\" : 42");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableUInt64("var1", 23);
    js.AddVariableUInt64("var2", 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableFloat")
  {
    StreamComparer sc("\"var1\" : -65.5,\n\"var2\" : 2621.25");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableFloat("var1", -65.5f);
    js.AddVariableFloat("var2", 2621.25f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableDouble")
  {
    StreamComparer sc("\"var1\" : -65.125,\n\"var2\" : 2621.0625");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableDouble("var1", -65.125f);
    js.AddVariableDouble("var2", 2621.0625f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableString")
  {
    StreamComparer sc("\"var1\" : \"bla\",\n\"var2\" : \"blub\",\n\"special\" : \"I\\\\m\\t\\\"s\\bec/al\\\" \\f\\n//\\\\\\r\"");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableString("var1", "bla");
    js.AddVariableString("var2", "blub");

    js.AddVariableString("special", "I\\m\t\"s\bec/al\" \f\n//\\\r");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableNULL")
  {
    StreamComparer sc("\"var1\" : null");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableNULL("var1");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableTime")
  {
    StreamComparer sc("\"var1\" : 0.5,\n\"var2\" : 2.25");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableTime("var1", nsTime::MakeFromSeconds(0.5));
    js.AddVariableTime("var2", nsTime::MakeFromSeconds(2.25));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableUuid")
  {
    nsUuid guid;
    nsUInt64 val[2];
    val[0] = 0x1122334455667788;
    val[1] = 0x99AABBCCDDEEFF00;
    nsMemoryUtils::Copy(reinterpret_cast<nsUInt64*>(&guid), val, 2);

    StreamComparer sc("\"uuid_var\" : { \"$t\" : \"uuid\", \"$b\" : \"0x887766554433221100FFEEDDCCBBAA99\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableUuid("uuid_var", guid);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableAngle")
  {
    StreamComparer sc("\"var1\" : 90,\n\"var2\" : 180");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    // vs2019 is so imprecise, that the degree->radian conversion introduces differences in the final output
    js.AddVariableAngle("var1", nsAngle::MakeFromRadian(1.5707963267f));
    js.AddVariableAngle("var2", nsAngle::MakeFromRadian(1.0f * nsMath::Pi<float>()));
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableColor")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"color\", \"$v\" : \"(1.0000, 2.0000, 3.0000, 4.0000)\", \"$b\" : "
                      "\"0x0000803F000000400000404000008040\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableColor("var1", nsColor(1, 2, 3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec2")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"vec2\", \"$v\" : \"(1.0000, 2.0000)\", \"$b\" : \"0x0000803F00000040\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec2("var1", nsVec2(1, 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec3")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"vec3\", \"$v\" : \"(1.0000, 2.0000, 3.0000)\", \"$b\" : \"0x0000803F0000004000004040\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec3("var1", nsVec3(1, 2, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec4")
  {
    StreamComparer sc(
      "\"var1\" : { \"$t\" : \"vec4\", \"$v\" : \"(1.0000, 2.0000, 3.0000, 4.0000)\", \"$b\" : \"0x0000803F000000400000404000008040\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec4("var1", nsVec4(1, 2, 3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec2I32")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"vec2i\", \"$v\" : \"(1, 2)\", \"$b\" : \"0x0100000002000000\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec2I32("var1", nsVec2I32(1, 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec3I32")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"vec3i\", \"$v\" : \"(1, 2, 3)\", \"$b\" : \"0x010000000200000003000000\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec3I32("var1", nsVec3I32(1, 2, 3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVec4I32")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"vec4i\", \"$v\" : \"(1, 2, 3, 4)\", \"$b\" : \"0x01000000020000000300000004000000\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVec4I32("var1", nsVec4I32(1, 2, 3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableDataBuffer")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"data\", \"$b\" : \"0xFF00DA\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    nsDataBuffer db;
    db.PushBack(0xFF);
    db.PushBack(0x00);
    db.PushBack(0xDA);
    js.AddVariableDataBuffer("var1", db);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableQuat")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"quat\", \"$b\" : \"0x0000803F000000400000404000008040\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableQuat("var1", nsQuat(1, 2, 3, 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableMat3")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"mat3\", \"$b\" : \"0x0000803F000080400000E040000000400000A04000000041000040400000C04000001041\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableMat3("var1", nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableMat4")
  {
    StreamComparer sc("\"var1\" : { \"$t\" : \"mat4\", \"$b\" : "
                      "\"0x0000803F0000A0400000104100005041000000400000C0400000204100006041000040400000E04000003041000070410000804000000041"
                      "0000404100008041\" }");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableMat4("var1", nsMat4T::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AddVariableVariant")
  {
    StreamComparer sc("\
\"var1\" : 23,\n\
\"var2\" : 42.5,\n\
\"var3\" : 21.25,\n\
\"var4\" : true,\n\
\"var5\" : \"pups\"");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.AddVariableVariant("var1", nsVariant(23));
    js.AddVariableVariant("var2", nsVariant(42.5f));
    js.AddVariableVariant("var3", nsVariant(21.25));
    js.AddVariableVariant("var4", nsVariant(true));
    js.AddVariableVariant("var5", nsVariant("pups"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Arrays")
  {
    StreamComparer sc("\
{\n\
  \"EmptyArray\" : [  ],\n\
  \"NamedArray\" : [ 13 ],\n\
  \"NamedArray2\" : [ 1337, -4996 ],\n\
  \"Nested\" : [ null, [ 1, 2, 3 ], [ 4, 5, 6 ], [  ], \"That was an empty array\" ]\n\
}");

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject();
    js.BeginArray("EmptyArray");
    js.EndArray();

    js.BeginArray("NamedArray");
    js.WriteInt32(13);
    js.EndArray();

    js.BeginArray("NamedArray2");
    js.WriteInt32(1337);
    js.WriteInt32(-4996);
    js.EndArray();

    js.BeginVariable("Nested");
    js.BeginArray();
    js.WriteNULL();

    js.BeginArray();
    js.WriteInt32(1);
    js.WriteInt32(2);
    js.WriteInt32(3);
    js.EndArray();

    js.BeginArray();
    js.WriteInt32(4);
    js.WriteInt32(5);
    js.WriteInt32(6);
    js.EndArray();

    js.BeginArray();
    js.EndArray();

    js.WriteString("That was an empty array");
    js.EndArray();
    js.EndVariable();

    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Complex Objects")
  {
    nsStringUtf8 sExp(L"\
{\n\
  \"String\" : \"testvälue\",\n\
  \"double\" : 43.56,\n\
  \"float\" : 64.720001,\n\
  \"bööl\" : true,\n\
  \"int\" : 23,\n\
  \"myarray\" : [ 1, 2.2, 3.3, false, \"ende\" ],\n\
  \"object\" : {\n\
    \"variable in object\" : \"bla/*asdf*/ //tuff\",\n\
    \"Subobject\" : {\n\
      \"variable in subobject\" : \"bla\\\\\",\n\
      \"array in sub\" : [ {\n\
          \"obj var\" : 234\n\
        },\n\
        {\n\
          \"obj var 2\" : -235\n\
        }, true, 4, false ]\n\
    }\n\
  },\n\
  \"test\" : \"text\"\n\
}");

    StreamComparer sc(sExp.GetData());

    nsStandardJSONWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject();

    js.AddVariableString("String", nsStringUtf8(L"testvälue").GetData()); // Unicode / Utf-8 test (in string)
    js.AddVariableDouble("double", 43.56);
    js.AddVariableFloat("float", 64.72f);
    js.AddVariableBool(nsStringUtf8(L"bööl").GetData(), true);            // Unicode / Utf-8 test (identifier)
    js.AddVariableInt32("int", 23);

    js.BeginArray("myarray");
    js.WriteInt32(1);
    js.WriteFloat(2.2f);
    js.WriteDouble(3.3);
    js.WriteBool(false);
    js.WriteString("ende");
    js.EndArray();

    js.BeginObject("object");
    js.AddVariableString("variable in object", "bla/*asdf*/ //tuff"); // 'comment' in string
    js.BeginObject("Subobject");
    js.AddVariableString("variable in subobject", "bla\\");           // character to be escaped

    js.BeginArray("array in sub");
    js.BeginObject();
    js.AddVariableUInt64("obj var", 234);
    js.EndObject();
    js.BeginObject();
    js.AddVariableInt64("obj var 2", -235);
    js.EndObject();
    js.WriteBool(true);
    js.WriteInt32(4);
    js.WriteBool(false);
    js.EndArray();
    js.EndObject();
    js.EndObject();

    js.AddVariableString("test", "text");

    js.EndObject();
  }
}
