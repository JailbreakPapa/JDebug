#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/Deque.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OpenDdlReader.h>
#include <Foundation/IO/OpenDdlUtils.h>
#include <Foundation/IO/OpenDdlWriter.h>
#include <Foundation/Strings/StringUtils.h>
#include <FoundationTest/IO/JSONTestHelpers.h>

static nsVariant CreateVariant(nsVariant::Type::Enum t, const void* pData);

NS_CREATE_SIMPLE_TEST(IO, DdlUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToColor")
  {
    const char* szTestData = "\
Color $c1 { float { 1, 0, 0.5 } }\
Color $c2 { float { 2, 1, 1.5, 0.1 } }\
Color $c3 { unsigned_int8 { 128, 2, 32 } }\
Color $c4 { unsigned_int8 { 128, 0, 32, 64 } }\
float $c5 { 1, 0, 0.5 }\
float $c6 { 2, 1, 1.5, 0.1 }\
unsigned_int8 $c7 { 128, 2, 32 }\
unsigned_int8 $c8 { 128, 0, 32, 64 }\
Color $c9 { float { 1, 0 } }\
Color $c10 { float { 1, 0, 3, 4, 5 } }\
Color $c11 { float { } }\
Color $c12 { }\
Color $c13 { double { 1, 1, 1, 2 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsColor c1, c2, c3, c4, c5, c6, c7, c8, c0;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("t0"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c1"), c1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c2"), c2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c3"), c3).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c4"), c4).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c5"), c5).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c6"), c6).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c7"), c7).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c8"), c8).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c9"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c10"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c11"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c12"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColor(doc.FindElement("c13"), c0).Failed());

    NS_TEST_BOOL(c1 == nsColor(1, 0, 0.5f, 1.0f));
    NS_TEST_BOOL(c2 == nsColor(2, 1, 1.5f, 0.1f));
    NS_TEST_BOOL(c3 == nsColorGammaUB(128, 2, 32));
    NS_TEST_BOOL(c4 == nsColorGammaUB(128, 0, 32, 64));
    NS_TEST_BOOL(c5 == nsColor(1, 0, 0.5f, 1.0f));
    NS_TEST_BOOL(c6 == nsColor(2, 1, 1.5f, 0.1f));
    NS_TEST_BOOL(c7 == nsColorGammaUB(128, 2, 32));
    NS_TEST_BOOL(c8 == nsColorGammaUB(128, 0, 32, 64));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToColorGamma")
  {
    const char* szTestData = "\
Color $c1 { float { 1, 0, 0.5 } }\
Color $c2 { float { 2, 1, 1.5, 0.1 } }\
Color $c3 { unsigned_int8 { 128, 2, 32 } }\
Color $c4 { unsigned_int8 { 128, 0, 32, 64 } }\
float $c5 { 1, 0, 0.5 }\
float $c6 { 2, 1, 1.5, 0.1 }\
unsigned_int8 $c7 { 128, 2, 32 }\
unsigned_int8 $c8 { 128, 0, 32, 64 }\
Color $c9 { float { 1, 0 } }\
Color $c10 { float { 1, 0, 3, 4, 5 } }\
Color $c11 { float { } }\
Color $c12 { }\
Color $c13 { double { 1, 1, 1, 2 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsColorGammaUB c1, c2, c3, c4, c5, c6, c7, c8, c0;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("t0"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c1"), c1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c2"), c2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c3"), c3).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c4"), c4).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c5"), c5).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c6"), c6).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c7"), c7).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c8"), c8).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c9"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c10"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c11"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c12"), c0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToColorGamma(doc.FindElement("c13"), c0).Failed());

    NS_TEST_BOOL(c1 == nsColorGammaUB(nsColor(1, 0, 0.5f, 1.0f)));
    NS_TEST_BOOL(c2 == nsColorGammaUB(nsColor(2, 1, 1.5f, 0.1f)));
    NS_TEST_BOOL(c3 == nsColorGammaUB(128, 2, 32));
    NS_TEST_BOOL(c4 == nsColorGammaUB(128, 0, 32, 64));
    NS_TEST_BOOL(c5 == nsColorGammaUB(nsColor(1, 0, 0.5f, 1.0f)));
    NS_TEST_BOOL(c6 == nsColorGammaUB(nsColor(2, 1, 1.5f, 0.1f)));
    NS_TEST_BOOL(c7 == nsColorGammaUB(128, 2, 32));
    NS_TEST_BOOL(c8 == nsColorGammaUB(128, 0, 32, 64));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToTime")
  {
    const char* szTestData = "\
Time $t1 { float { 0.1 } }\
Time $t2 { double { 0.2 } }\
float $t3 { 0.3 }\
double $t4 { 0.4 }\
Time $t5 { double { 0.2, 2 } }\
Time $t6 { int8 { 0, 2 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsTime t1, t2, t3, t4, t0;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t0"), t0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t1"), t1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t2"), t2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t3"), t3).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t4"), t4).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t5"), t0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTime(doc.FindElement("t6"), t0).Failed());

    NS_TEST_FLOAT(t1.GetSeconds(), 0.1, 0.0001f);
    NS_TEST_FLOAT(t2.GetSeconds(), 0.2, 0.0001f);
    NS_TEST_FLOAT(t3.GetSeconds(), 0.3, 0.0001f);
    NS_TEST_FLOAT(t4.GetSeconds(), 0.4, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToVec2")
  {
    const char* szTestData = "\
Vector $v1 { float { 0.1, 2 } }\
float $v2 { 0.3, 3 }\
Vector $v3 { float { 0.1 } }\
Vector $v4 { float { 0.1, 2.2, 3.33 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsVec2 v0, v1, v2;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec2(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec2(doc.FindElement("v1"), v1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec2(doc.FindElement("v2"), v2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec2(doc.FindElement("v3"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec2(doc.FindElement("v4"), v0).Failed());

    NS_TEST_VEC2(v1, nsVec2(0.1f, 2.0f), 0.0001f);
    NS_TEST_VEC2(v2, nsVec2(0.3f, 3.0f), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToVec3")
  {
    const char* szTestData = "\
Vector $v1 { float { 0.1, 2, 3.2 } }\
float $v2 { 0.3, 3,0}\
Vector $v3 { float { 0.1,2 } }\
Vector $v4 { float { 0.1, 2.2, 3.33,44 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsVec3 v0, v1, v2;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec3(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec3(doc.FindElement("v1"), v1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec3(doc.FindElement("v2"), v2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec3(doc.FindElement("v3"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec3(doc.FindElement("v4"), v0).Failed());

    NS_TEST_VEC3(v1, nsVec3(0.1f, 2.0f, 3.2f), 0.0001f);
    NS_TEST_VEC3(v2, nsVec3(0.3f, 3.0f, 0.0f), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToVec4")
  {
    const char* szTestData = "\
Vector $v1 { float { 0.1, 2, 3.2, 44.5 } }\
float $v2 { 0.3, 3,0, 12.}\
Vector $v3 { float { 0.1,2 } }\
Vector $v4 { float { 0.1, 2.2, 3.33, 44, 67 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsVec4 v0, v1, v2;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec4(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec4(doc.FindElement("v1"), v1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec4(doc.FindElement("v2"), v2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec4(doc.FindElement("v3"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVec4(doc.FindElement("v4"), v0).Failed());

    NS_TEST_VEC4(v1, nsVec4(0.1f, 2.0f, 3.2f, 44.5f), 0.0001f);
    NS_TEST_VEC4(v2, nsVec4(0.3f, 3.0f, 0.0f, 12.0f), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToMat3")
  {
    const char* szTestData = "\
Group $v1 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsMat3 v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToMat3(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToMat3(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_BOOL(v1.IsEqual(nsMat3::MakeFromValues(1, 4, 7, 2, 5, 8, 3, 6, 9), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToMat4")
  {
    const char* szTestData = "\
Group $v1 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsMat4 v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToMat4(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToMat4(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_BOOL(v1.IsEqual(nsMat4T::MakeFromValues(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16), 0.0001f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToTransform")
  {
    const char* szTestData = "\
Group $v1 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsTransform v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTransform(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTransform(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_VEC3(v1.m_vPosition, nsVec3(1, 2, 3), 0.0001f);
    NS_TEST_BOOL(v1.m_qRotation == nsQuat(4, 5, 6, 7));
    NS_TEST_VEC3(v1.m_vScale, nsVec3(8, 9, 10), 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToQuat")
  {
    const char* szTestData = "\
Vector $v1 { float { 0.1, 2, 3.2, 44.5 } }\
float $v2 { 0.3, 3,0, 12.}\
Vector $v3 { float { 0.1,2 } }\
Vector $v4 { float { 0.1, 2.2, 3.33, 44, 67 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsQuat v0, v1, v2;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToQuat(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToQuat(doc.FindElement("v1"), v1).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToQuat(doc.FindElement("v2"), v2).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToQuat(doc.FindElement("v3"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToQuat(doc.FindElement("v4"), v0).Failed());

    NS_TEST_BOOL(v1 == nsQuat(0.1f, 2.0f, 3.2f, 44.5f));
    NS_TEST_BOOL(v2 == nsQuat(0.3f, 3.0f, 0.0f, 12.0f));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToUuid")
  {
    const char* szTestData = "\
Data $v1 { unsigned_int64 { 12345678910, 10987654321 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsUuid v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToUuid(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToUuid(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_BOOL(v1 == nsUuid(12345678910, 10987654321));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToAngle")
  {
    const char* szTestData = "\
Data $v1 { float { 45.23 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsAngle v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToAngle(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToAngle(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_FLOAT(v1.GetRadian(), 45.23f, 0.0001f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToHashedString")
  {
    const char* szTestData = "\
Data $v1 { string { \"Hello World\" } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsHashedString v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToHashedString(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToHashedString(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_STRING(v1.GetView(), "Hello World");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToTempHashedString")
  {
    const char* szTestData = "\
Data $v1 { uint64 { 2720389094277464445 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsTempHashedString v0, v1;

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTempHashedString(doc.FindElement("v0"), v0).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToTempHashedString(doc.FindElement("v1"), v1).Succeeded());

    NS_TEST_BOOL(v1 == nsTempHashedString("GHIJK"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsOpenDdlUtils::ConvertToVariant")
  {
    const char* szTestData = "\
Color $v1 { float { 1, 0, 0.5 } }\
ColorGamma $v2 { unsigned_int8 { 128, 0, 32, 64 } }\
Time $v3 { float { 0.1 } }\
Vec2 $v4 { float { 0.1, 2 } }\
Vec3 $v5 { float { 0.1, 2, 3.2 } }\
Vec4 $v6 { float { 0.1, 2, 3.2, 44.5 } }\
Mat3 $v7 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9 } }\
Mat4 $v8 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } }\
Transform $v9 { float { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 } }\
Quat $v10 { float { 0.1, 2, 3.2, 44.5 } }\
Uuid $v11 { unsigned_int64 { 12345678910, 10987654321 } }\
Angle $v12 { float { 45.23 } }\
HashedString $v13 { string { \"Soo much string\" } }\
TempHashedString $v14 { uint64 { 2720389094277464445 } }\
";

    StringStream stream(szTestData);
    nsOpenDdlReader doc;
    NS_TEST_BOOL(doc.ParseDocument(stream).Succeeded());

    nsVariant v[15];

    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v0"), v[0]).Failed());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v1"), v[1]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v2"), v[2]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v3"), v[3]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v4"), v[4]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v5"), v[5]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v6"), v[6]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v7"), v[7]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v8"), v[8]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v9"), v[9]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v10"), v[10]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v11"), v[11]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v12"), v[12]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v13"), v[13]).Succeeded());
    NS_TEST_BOOL(nsOpenDdlUtils::ConvertToVariant(doc.FindElement("v14"), v[14]).Succeeded());

    NS_TEST_BOOL(v[1].IsA<nsColor>());
    NS_TEST_BOOL(v[2].IsA<nsColorGammaUB>());
    NS_TEST_BOOL(v[3].IsA<nsTime>());
    NS_TEST_BOOL(v[4].IsA<nsVec2>());
    NS_TEST_BOOL(v[5].IsA<nsVec3>());
    NS_TEST_BOOL(v[6].IsA<nsVec4>());
    NS_TEST_BOOL(v[7].IsA<nsMat3>());
    NS_TEST_BOOL(v[8].IsA<nsMat4>());
    NS_TEST_BOOL(v[9].IsA<nsTransform>());
    NS_TEST_BOOL(v[10].IsA<nsQuat>());
    NS_TEST_BOOL(v[11].IsA<nsUuid>());
    NS_TEST_BOOL(v[12].IsA<nsAngle>());
    NS_TEST_BOOL(v[13].IsA<nsHashedString>());
    NS_TEST_BOOL(v[14].IsA<nsTempHashedString>());

    NS_TEST_BOOL(v[1].Get<nsColor>() == nsColor(1, 0, 0.5));
    NS_TEST_BOOL(v[2].Get<nsColorGammaUB>() == nsColorGammaUB(128, 0, 32, 64));
    NS_TEST_FLOAT(v[3].Get<nsTime>().GetSeconds(), 0.1, 0.0001f);
    NS_TEST_VEC2(v[4].Get<nsVec2>(), nsVec2(0.1f, 2.0f), 0.0001f);
    NS_TEST_VEC3(v[5].Get<nsVec3>(), nsVec3(0.1f, 2.0f, 3.2f), 0.0001f);
    NS_TEST_VEC4(v[6].Get<nsVec4>(), nsVec4(0.1f, 2.0f, 3.2f, 44.5f), 0.0001f);
    NS_TEST_BOOL(v[7].Get<nsMat3>().IsEqual(nsMat3::MakeFromValues(1, 4, 7, 2, 5, 8, 3, 6, 9), 0.0001f));
    NS_TEST_BOOL(v[8].Get<nsMat4>().IsEqual(nsMat4::MakeFromValues(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16), 0.0001f));
    NS_TEST_BOOL(v[9].Get<nsTransform>().m_qRotation == nsQuat(4, 5, 6, 7));
    NS_TEST_VEC3(v[9].Get<nsTransform>().m_vPosition, nsVec3(1, 2, 3), 0.0001f);
    NS_TEST_VEC3(v[9].Get<nsTransform>().m_vScale, nsVec3(8, 9, 10), 0.0001f);
    NS_TEST_BOOL(v[10].Get<nsQuat>() == nsQuat(0.1f, 2.0f, 3.2f, 44.5f));
    NS_TEST_BOOL(v[11].Get<nsUuid>() == nsUuid(12345678910, 10987654321));
    NS_TEST_FLOAT(v[12].Get<nsAngle>().GetRadian(), 45.23f, 0.0001f);
    NS_TEST_STRING(v[13].Get<nsHashedString>().GetView(), "Soo much string");
    NS_TEST_BOOL(v[14].Get<nsTempHashedString>() == nsTempHashedString("GHIJK"));


    /// \test Test primitive types in nsVariant
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreColor")
  {
    StreamComparer sc("Color $v1{float{1,2,3,4}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreColor(js, nsColor(1, 2, 3, 4), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreColorGamma")
  {
    StreamComparer sc("ColorGamma $v1{uint8{1,2,3,4}}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreColorGamma(js, nsColorGammaUB(1, 2, 3, 4), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreTime")
  {
    StreamComparer sc("Time $v1{double{2.3}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreTime(js, nsTime::MakeFromSeconds(2.3), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreVec2")
  {
    StreamComparer sc("Vec2 $v1{float{1,2}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreVec2(js, nsVec2(1, 2), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreVec3")
  {
    StreamComparer sc("Vec3 $v1{float{1,2,3}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreVec3(js, nsVec3(1, 2, 3), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreVec4")
  {
    StreamComparer sc("Vec4 $v1{float{1,2,3,4}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreVec4(js, nsVec4(1, 2, 3, 4), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreMat3")
  {
    StreamComparer sc("Mat3 $v1{float{1,4,7,2,5,8,3,6,9}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreMat3(js, nsMat3::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreMat4")
  {
    StreamComparer sc("Mat4 $v1{float{1,5,9,13,2,6,10,14,3,7,11,15,4,8,12,16}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreMat4(js, nsMat4::MakeFromValues(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), "v1", true);
  }

  // NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreTransform")
  //{
  //  StreamComparer sc("Transform $v1{float{1,4,7,2,5,8,3,6,9,10}}\n");

  //  nsOpenDdlWriter js;
  //  js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
  //  js.SetOutputStream(&sc);

  //  nsOpenDdlUtils::StoreTransform(js, nsTransform(nsVec3(10, 20, 30), nsMat3(1, 2, 3, 4, 5, 6, 7, 8, 9)), "v1", true);
  //}

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreQuat")
  {
    StreamComparer sc("Quat $v1{float{1,2,3,4}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreQuat(js, nsQuat(1, 2, 3, 4), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreUuid")
  {
    StreamComparer sc("Uuid $v1{u4{12345678910,10987654321}}\n");

    nsOpenDdlWriter js;
    js.SetPrimitiveTypeStringMode(nsOpenDdlWriter::TypeStringMode::Shortest);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreUuid(js, nsUuid(12345678910, 10987654321), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreAngle")
  {
    StreamComparer sc("Angle $v1{float{2.3}}\n");

    nsOpenDdlWriter js;
    js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Readable);
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreAngle(js, nsAngle::MakeFromRadian(2.3f), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreHashedString")
  {
    StreamComparer sc("HashedString $v1{string{\"ABCDE\"}}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreHashedString(js, nsMakeHashedString("ABCDE"), "v1", true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreTempHashedString")
  {
    StreamComparer sc("TempHashedString $v1{uint64{2720389094277464445}}\n");

    nsOpenDdlWriter js;
    js.SetOutputStream(&sc);

    nsOpenDdlUtils::StoreTempHashedString(js, nsTempHashedString("GHIJK"), "v1", true);
  }

  // this test also covers all the types that Variant supports
  NS_TEST_BLOCK(nsTestBlock::Enabled, "StoreVariant")
  {
    alignas(NS_ALIGNMENT_OF(float)) nsUInt8 rawData[sizeof(float) * 16]; // enough for mat4

    for (nsUInt8 i = 0; i < NS_ARRAY_SIZE(rawData); ++i)
    {
      rawData[i] = i + 33;
    }

    rawData[NS_ARRAY_SIZE(rawData) - 1] = 0; // string terminator

    for (nsUInt32 t = nsVariant::Type::FirstStandardType + 1; t < nsVariant::Type::LastStandardType; ++t)
    {
      const nsVariant var = CreateVariant((nsVariant::Type::Enum)t, rawData);

      nsDefaultMemoryStreamStorage storage;
      nsMemoryStreamWriter writer(&storage);
      nsMemoryStreamReader reader(&storage);

      nsOpenDdlWriter js;
      js.SetFloatPrecisionMode(nsOpenDdlWriter::FloatPrecisionMode::Exact);
      js.SetOutputStream(&writer);

      nsOpenDdlUtils::StoreVariant(js, var, "bla");

      nsOpenDdlReader doc;
      NS_TEST_BOOL(doc.ParseDocument(reader).Succeeded());

      const auto pVarElem = doc.GetRootElement()->FindChild("bla");

      nsVariant result;
      nsOpenDdlUtils::ConvertToVariant(pVarElem, result).IgnoreResult();

      NS_TEST_BOOL(var == result);
    }
  }
}

static nsVariant CreateVariant(nsVariant::Type::Enum t, const void* pData)
{
  switch (t)
  {
    case nsVariant::Type::Bool:
      return nsVariant(*(nsInt8*)pData != 0);
    case nsVariant::Type::Int8:
      return nsVariant(*((nsInt8*)pData));
    case nsVariant::Type::UInt8:
      return nsVariant(*((nsUInt8*)pData));
    case nsVariant::Type::Int16:
      return nsVariant(*((nsInt16*)pData));
    case nsVariant::Type::UInt16:
      return nsVariant(*((nsUInt16*)pData));
    case nsVariant::Type::Int32:
      return nsVariant(*((nsInt32*)pData));
    case nsVariant::Type::UInt32:
      return nsVariant(*((nsUInt32*)pData));
    case nsVariant::Type::Int64:
      return nsVariant(*((nsInt64*)pData));
    case nsVariant::Type::UInt64:
      return nsVariant(*((nsUInt64*)pData));
    case nsVariant::Type::Float:
      return nsVariant(*((float*)pData));
    case nsVariant::Type::Double:
      return nsVariant(*((double*)pData));
    case nsVariant::Type::Color:
      return nsVariant(*((nsColor*)pData));
    case nsVariant::Type::Vector2:
      return nsVariant(*((nsVec2*)pData));
    case nsVariant::Type::Vector3:
      return nsVariant(*((nsVec3*)pData));
    case nsVariant::Type::Vector4:
      return nsVariant(*((nsVec4*)pData));
    case nsVariant::Type::Vector2I:
      return nsVariant(*((nsVec2I32*)pData));
    case nsVariant::Type::Vector3I:
      return nsVariant(*((nsVec3I32*)pData));
    case nsVariant::Type::Vector4I:
      return nsVariant(*((nsVec4I32*)pData));
    case nsVariant::Type::Vector2U:
      return nsVariant(*((nsVec2U32*)pData));
    case nsVariant::Type::Vector3U:
      return nsVariant(*((nsVec3U32*)pData));
    case nsVariant::Type::Vector4U:
      return nsVariant(*((nsVec4U32*)pData));
    case nsVariant::Type::Quaternion:
      return nsVariant(*((nsQuat*)pData));
    case nsVariant::Type::Matrix3:
      return nsVariant(*((nsMat3*)pData));
    case nsVariant::Type::Matrix4:
      return nsVariant(*((nsMat4*)pData));
    case nsVariant::Type::Transform:
      return nsVariant(*((nsTransform*)pData));
    case nsVariant::Type::String:
    case nsVariant::Type::StringView: // string views are stored as full strings as well
      return nsVariant((const char*)pData);
    case nsVariant::Type::DataBuffer:
    {
      nsDataBuffer db;
      db.SetCountUninitialized(sizeof(float) * 16);
      for (nsUInt32 i = 0; i < db.GetCount(); ++i)
        db[i] = ((nsUInt8*)pData)[i];

      return nsVariant(db);
    }
    case nsVariant::Type::Time:
      return nsVariant(*((nsTime*)pData));
    case nsVariant::Type::Uuid:
      return nsVariant(*((nsUuid*)pData));
    case nsVariant::Type::Angle:
      return nsVariant(*((nsAngle*)pData));
    case nsVariant::Type::ColorGamma:
      return nsVariant(*((nsColorGammaUB*)pData));
    case nsVariant::Type::HashedString:
    {
      nsHashedString s;
      s.Assign((const char*)pData);
      return nsVariant(s);
    }
    case nsVariant::Type::TempHashedString:
      return nsVariant(nsTempHashedString((const char*)pData));

    default:
      NS_REPORT_FAILURE("Unknown type");
  }

  return nsVariant();
}
