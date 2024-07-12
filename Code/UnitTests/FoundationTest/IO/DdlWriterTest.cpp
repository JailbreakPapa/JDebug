#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <FoundationTest/IO/JSONTestHelpers.h>


NS_CREATE_SIMPLE_TEST(IO, DdlWriter)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Object")
  {
    StreamComparer sc("TestObject{}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("TestObject");
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Named Object (global)")
  {
    StreamComparer sc("TestObject $ObjName{}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("TestObject", "ObjName", true);
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Named Object (local)")
  {
    StreamComparer sc("TestObject %ObjName{}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("TestObject", "ObjName", false);
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Object Hierarchy")
  {
    StreamComparer sc("obj1\n\
{\n\
	obj11 $a\n\
	{\n\
		obj111 %b{}\n\
		obj112{}\n\
	}\n\
	obj12 %c{}\n\
}\n\
obj2{}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("obj1");
    {
      js.BeginObject("obj11", "a", true);
      {
        {
          js.BeginObject("obj111", "b", false);
          js.EndObject();
        }
        {
          js.BeginObject("obj112");
          js.EndObject();
        }
      }
      js.EndObject();
      js.BeginObject("obj12", "c");
      {
      }
      js.EndObject();
    }
    js.EndObject();
    js.BeginObject("obj2");
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Empty Primitive List")
  {
    StreamComparer sc("Data\n\
{\n\
	bool{}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool);
      {
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Named Primitive List (global)")
  {
    StreamComparer sc("Data\n\
{\n\
	bool $values{}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, "values", true);
      {
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (bool)")
  {
    StreamComparer sc("Data\n\
{\n\
	bool $values{true,false}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, "values", true);
      {
        bool val[] = {true, false, true};
        js.WriteBool(val, 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (bool)")
  {
    StreamComparer sc("Data\n\
{\n\
	bool %values{true,true,false}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, "values", false);
      {
        bool val[] = {true, false, true};
        js.WriteBool(val, 1);
        js.WriteBool(val, 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (int8)")
  {
    StreamComparer sc("Data\n\
{\n\
	int8 $values{0,127,-128}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int8, "values", true);
      {
        nsInt8 val[] = {0, 127, -128};
        js.WriteInt8(val, 3);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (int16)")
  {
    StreamComparer sc("Data\n\
{\n\
	int16 $values{1,32767,-32768}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int16, "values", true);
      {
        nsInt16 val[] = {1, 32767, -32768};
        js.WriteInt16(&val[0], 1);
        js.WriteInt16(&val[1], 1);
        js.WriteInt16(&val[2], 1);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (int32)")
  {
    StreamComparer sc("Data\n\
{\n\
	int32{-2147483647,2147483647}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int32);
      {
        nsInt32 val[] = {-2147483647, 2147483647};
        js.WriteInt32(&val[0], 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (int64)")
  {
    StreamComparer sc("Data\n\
{\n\
	int64{-9223372036854775807,9223372036854775807}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Int64);
      {
        nsInt64 val[] = {-9223372036854775807LL, 9223372036854775807LL};
        js.WriteInt64(&val[0], 1);
        js.WriteInt64(&val[1], 1);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (uint8)")
  {
    StreamComparer sc("Data\n\
{\n\
	unsigned_int8{0,255}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt8);
      {
        nsUInt8 val[] = {0, 255, 27};
        js.WriteUInt8(val, 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (uint16)")
  {
    StreamComparer sc("Data\n\
{\n\
	uint16 %values{0,32767,65535}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::ShortenedUnsignedInt);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt16, "values", false);
      {
        nsUInt16 val[] = {0, 32767, 65535};
        js.WriteUInt16(&val[0], 1);
        js.WriteUInt16(&val[1], 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (uint32)")
  {
    StreamComparer sc("Data\n\
{\n\
	u3{4294967295}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Shortest);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt32);
      {
        nsUInt32 val[] = {4294967295, 0};
        js.WriteUInt32(&val[0], 1);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (uint64)")
  {
    StreamComparer sc("Data\n\
{\n\
	unsigned_int64{18446744073709551615,18446744073709551615}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Compliant);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::UInt64);
      {
        nsUInt64 val[] = {18446744073709551615ULL};
        js.WriteUInt64(&val[0], 1);
        js.WriteUInt64(&val[0], 1);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (float)")
  {
    StreamComparer sc("Data\n\
{\n\
	float{0}\n\
	float{0.1,0.00001}\n\
	float{0.00001,230000,-42.23,0,0.1}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);

    js.BeginObject("Data");
    {
      float val[] = {0, 0.1f, 1e-5f, 23e4f, -42.23f};

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
      {
        js.WriteFloat(&val[0], 1);
      }
      js.EndPrimitiveList();

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
      {
        js.WriteFloat(&val[1], 1);
        js.WriteFloat(&val[2], 1);
      }
      js.EndPrimitiveList();

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float);
      {
        js.WriteFloat(&val[2], 3);
        js.WriteFloat(&val[0], 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (double)")
  {
    StreamComparer sc("Data\n\
{\n\
	double{0}\n\
	double{0.1,0.00001}\n\
	double{0.00001,230000,-42.23,0,0.1}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);

    js.BeginObject("Data");
    {
      double val[] = {0, 0.1, 1e-5, 23e4, -42.23};

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Double);
      {
        js.WriteDouble(&val[0], 1);
      }
      js.EndPrimitiveList();

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Double);
      {
        js.WriteDouble(&val[1], 1);
        js.WriteDouble(&val[2], 1);
      }
      js.EndPrimitiveList();

      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Double);
      {
        js.WriteDouble(&val[2], 3);
        js.WriteDouble(&val[0], 2);
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Primitive List (string)")
  {
    StreamComparer sc("Data\n\
{\n\
	string{\"bla\"}\n\
	string{\"bla2\",\"blub\"}\n\
	string{\"A\\nwo\\rld\\\"\"}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    js.BeginObject("Data");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
      {
        js.WriteString("bla");
      }
      js.EndPrimitiveList();
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
      {
        js.WriteString("bla2");
        js.WriteString("blub");
      }
      js.EndPrimitiveList();
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
      {
        js.WriteString("A\nwo\rld\"");
      }
      js.EndPrimitiveList();
    }
    js.EndObject();
  }

  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LessIndentation")
  {
    StreamComparer sc("bool $balue{true}\n\
obj1\n\
{\n\
	string{\"bla\"}\n\
	obj11\n\
	{\n\
		float %falue{23.42}\n\
	}\n\
}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);

    js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, "balue", true);
    {
      bool b = true;
      js.WriteBool(&b, 1);
    }
    js.EndPrimitiveList();

    js.BeginObject("obj1");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
      {
        js.WriteString("bla");
      }
      js.EndPrimitiveList();

      js.BeginObject("obj11");
      {
        js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float, "falue");
        {
          float f = 23.42f;
          js.WriteFloat(&f);
        }
        js.EndPrimitiveList();
      }
      js.EndObject();
    }
    js.EndObject();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "None")
  {
    StreamComparer sc("bool$balue{1}obj1{string{\"bla\"}obj11{float%falue{23.42}}}");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetCompactMode(true);

    js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Bool, "balue", true);
    {
      bool b = true;
      js.WriteBool(&b, 1);
    }
    js.EndPrimitiveList();

    js.BeginObject("obj1");
    {
      js.BeginPrimitiveList(nsOpenDdlPrimitiveType::String);
      {
        js.WriteString("bla");
      }
      js.EndPrimitiveList();

      js.BeginObject("obj11");
      {
        js.BeginPrimitiveList(nsOpenDdlPrimitiveType::Float, "falue");
        {
          float f = 23.42f;
          js.WriteFloat(&f);
        }
        js.EndPrimitiveList();
      }
      js.EndObject();
    }
    js.EndObject();
  }
}
