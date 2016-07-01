#include "eptest.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/component/component.h"
#include <random>

using namespace ep;

// TODO: these tests are old and should be deprecated!!
TEST(EPVariant, Deprecated)
{
  Variant t;
  EPASSERT(t.type() == Variant::Type::Void, "!");
  Variant t0(nullptr);
  EPASSERT(t0.type() == Variant::Type::Null, "!");
  Variant t1(true);
  EPASSERT(t1.type() == Variant::Type::Bool && t1.asBool() == true, "!");
  Variant t2(10);
  EPASSERT(t2.type() == Variant::Type::Int && t2.asInt() == 10, "!");
  Variant t3(10.0);
  EPASSERT(t3.type() == Variant::Type::Float && t3.asFloat() == 10.0, "!");

  // string constructions
  Variant t4("hello");
  EPASSERT(t4.type() == Variant::Type::String, "!");
  Variant t5(String("hello"));
  EPASSERT(t5.type() == Variant::Type::String, "!");
  Variant t6(MutableString<0>("hello"));
  EPASSERT(t6.type() == Variant::Type::String, "!");
  Variant t7(MutableString<16>("hello"));
  EPASSERT(t7.type() == Variant::Type::String, "!");
  Variant t8(MutableString<0>("really long string that will claim allocation"));
  EPASSERT(t8.type() == Variant::Type::String, "!");
  Variant t9(SharedString("test"));
  EPASSERT(t9.type() == Variant::Type::String, "!");

  MutableString<0> x = "123";
  MutableString<15> y = "123";
  SharedString z = "123";

  Variant t10(x);
  EPASSERT(t10.type() == Variant::Type::String, "!");
  Variant t11(y);
  EPASSERT(t11.type() == Variant::Type::String, "!");
  Variant t12(z);
  EPASSERT(t12.type() == Variant::Type::String, "!");

  ComponentRef spC;
  Variant t15(spC);
  Variant t16(ComponentRef(nullptr));

  const Variant arr[] = { 1, 2, "3" };
  Variant t17(arr);
  auto a1 = t17.as<Array<float>>();

  Variant t18("[1,  2  ,  \"3\" ] ");
  auto a2 = t18.as<Array<SharedString>>();

  Variant t19;
  EPASSERT(!t19.isValid(), "!");

  SharedString::format("{0} {1} {2}", Variant(true), Variant(10), Variant(20.0));
}


TEST(EPVariant, EPVariantNull)
{
  Variant var(nullptr);

  EXPECT_EQ(var.type(), Variant::Type::Null);
  EXPECT_TRUE(var.is(Variant::Type::Null));

  EXPECT_EQ(0, var.stringify().cmp(String("nil")));

  EXPECT_FALSE(var.asBool());
  EXPECT_EQ(0, var.asInt());
  EXPECT_EQ(0.0, var.asFloat());

  EXPECT_FALSE(var.asEnum(nullptr));
  EXPECT_FALSE(var.asComponent());
  EXPECT_FALSE(var.asDelegate());

  EXPECT_TRUE(var.asString().empty());

  EXPECT_TRUE(var.asArray().empty());
  EXPECT_TRUE(var.asAssocArray().empty());
//  EXPECT_TRUE(var.asAssocArraySeries().empty());

  EXPECT_EQ(0, var.arrayLen());
  EXPECT_EQ(0, var.assocArraySeriesLen());

  EXPECT_THROW(var[0], EPException);
  EXPECT_THROW(var["dummy"], EPException);

}

TEST(EPVariant, EPVariantBool)
{
  Variant varTrue(true);
  Variant varFalse(false);

  EXPECT_EQ(varTrue.type(), Variant::Type::Bool);
  EXPECT_TRUE(varTrue.is(Variant::Type::Bool));

  EXPECT_EQ(0, varTrue.stringify().cmp(String("true")));
  EXPECT_EQ(0, varFalse.stringify().cmp(String("false")));

  EXPECT_TRUE(varTrue.asBool());
  EXPECT_FALSE(varFalse.asBool());

  EXPECT_NE(0, varTrue.asInt());
  EXPECT_EQ(0, varFalse.asInt());

  EXPECT_EQ(1.0, varTrue.asFloat());
  EXPECT_EQ(0.0, varFalse.asFloat());

  EXPECT_FALSE(varTrue.asEnum(nullptr));

  EXPECT_THROW(varTrue.asComponent(), EPException);
  EXPECT_THROW(varTrue.asDelegate(), EPException);
  EXPECT_THROW(varTrue.asString().empty(), EPException);

  EXPECT_THROW(varTrue.asArray().empty(), EPException);
  EXPECT_THROW(varTrue.asAssocArray().empty(), EPException);
//  EXPECT_THROW(varTrue.asAssocArraySeries().empty(), EPException);

  EXPECT_EQ(0, varTrue.arrayLen());
  EXPECT_EQ(0, varTrue.assocArraySeriesLen());

  EXPECT_THROW(varTrue[0], EPException);
  EXPECT_THROW(varTrue["dummy"], EPException);
}

TEST(EPVariant, EPVariantInt)
{
  std::default_random_engine eng(0);
  std::uniform_int_distribution<int64_t> dist(INT64_MIN, INT64_MAX);

  for (int64_t i = 0; i < 1000; ++i)
  {
    int64_t rInt = dist(eng);
    Variant varInt(rInt);

    EXPECT_EQ(!!rInt, varInt.asBool());
    EXPECT_EQ(rInt, varInt.asInt());
    EXPECT_EQ((double)rInt, varInt.asFloat());
  }

  int64_t rInt = dist(eng);
  Variant varInt(rInt);

  EXPECT_EQ(varInt.type(), Variant::Type::Int);
  EXPECT_TRUE(varInt.is(Variant::Type::Int));
  EXPECT_TRUE(varInt.stringify().empty());

  EXPECT_FALSE(varInt.asEnum(nullptr));

  EXPECT_THROW(varInt.asComponent(), EPException);
  EXPECT_THROW(varInt.asDelegate(), EPException);
  EXPECT_THROW(varInt.asString().empty(), EPException);

  EXPECT_THROW(varInt.asArray().empty(), EPException);
  EXPECT_THROW(varInt.asAssocArray().empty(), EPException);
//  EXPECT_THROW(varInt.asAssocArraySeries().empty(), EPException);

  EXPECT_EQ(0, varInt.arrayLen());
  EXPECT_EQ(0, varInt.assocArraySeriesLen());

  EXPECT_THROW(varInt[0], EPException);
  EXPECT_THROW(varInt["dummy"], EPException);
}


TEST(EPVariant, EPVariantDbl)
{
  std::mt19937 eng(0);
  std::uniform_real_distribution<> dist(-FLT_MAX, FLT_MAX);

  for (int64_t i = 0; i < 1000; ++i)
  {
    double rDbl = dist(eng);
    Variant varDbl(rDbl);

    EXPECT_EQ(!!rDbl, varDbl.asBool());
    EXPECT_EQ((int64_t)rDbl, varDbl.asInt());
    EXPECT_EQ(rDbl, varDbl.asFloat());
  }

  double rDbl = dist(eng);
  Variant varDbl(rDbl);

  EXPECT_EQ(varDbl.type(), Variant::Type::Float);
  EXPECT_TRUE(varDbl.is(Variant::Type::Float));
  EXPECT_TRUE(varDbl.stringify().empty());

  EXPECT_FALSE(varDbl.asEnum(nullptr));

  EXPECT_THROW(varDbl.asComponent(), EPException);
  EXPECT_THROW(varDbl.asDelegate(), EPException);
  EXPECT_THROW(varDbl.asString().empty(), EPException);

  EXPECT_THROW(varDbl.asArray().empty(), EPException);
  EXPECT_THROW(varDbl.asAssocArray().empty(), EPException);
//  EXPECT_THROW(varDbl.asAssocArraySeries().empty(), EPException);

  EXPECT_EQ(0, varDbl.arrayLen());
  EXPECT_EQ(0, varDbl.assocArraySeriesLen());

  EXPECT_THROW(varDbl[0], EPException);
  EXPECT_THROW(varDbl["dummy"], EPException);
}


TEST(EPVariant, EPVariantSlice)
{
  Variant var0(0);
  Variant var1(1);
  Variant var2(2);
  Variant testArray[] = { var0, var1, var2 };
  Slice<Variant> testSlice(testArray, 3);

  Variant varSliceUnSafe(testSlice, true);
  Variant varSlice(testSlice);

  EXPECT_EQ(varSlice.type(), Variant::Type::Array);
  EXPECT_TRUE(varSlice.is(Variant::Type::Array));
  EXPECT_TRUE(varSlice.stringify().empty());

  EXPECT_THROW(varSlice.asBool(), EPException);
  EXPECT_THROW(varSlice.asInt(), EPException);
  EXPECT_THROW(varSlice.asFloat(), EPException);

  EXPECT_FALSE(varSlice.asEnum(nullptr));

  EXPECT_THROW(varSlice.asComponent(), EPException);
  EXPECT_THROW(varSlice.asDelegate(), EPException);
  EXPECT_THROW(varSlice.asString().empty(), EPException);

  EXPECT_TRUE(testSlice == varSliceUnSafe.asArray());
  EXPECT_TRUE(testSlice != varSlice.asArray());

  for (size_t i = 0; i < 3; ++i)
    EXPECT_TRUE(testSlice[i].asInt() == varSlice.asArray()[i].asInt());

  EXPECT_THROW(varSlice.asAssocArray().empty(), EPException);
//  EXPECT_THROW(varSlice.asAssocArraySeries().empty(), EPException);

  EXPECT_EQ(varSlice[0].asInt(), 0);
  EXPECT_EQ(varSlice[1].asInt(), 1);
  EXPECT_EQ(varSlice[2].asInt(), 2);

  EXPECT_ASSERT_THROW(varSlice[3]);
}


TEST(EPVariant, EPVariantAs)
{
  Variant v_null(nullptr);
  Variant v_bool(true);
  Variant v_int(10);
  Variant v_float(10.0);
  Variant v_shortString("false");
  Variant v_longString("The quick brown fox jumps over the lazy dog");
  Variant v_array(Variant::VarArray{ 1, 2.0, "3" });
  Variant v_map(Variant::VarMap{ { "x", 1 }, { "y", 2.0 }, { "z", "3" } });

  EXPECT_EQ(false, v_null.as<bool>());
  EXPECT_EQ(true, v_bool.as<bool>());
  EXPECT_EQ(true, v_int.as<bool>());
  EXPECT_EQ(true, v_float.as<bool>());
  EXPECT_EQ(false, v_shortString.as<bool>());
  EXPECT_EQ(true, v_longString.as<bool>());
  EXPECT_THROW(v_array.as<bool>(), EPException);
  EXPECT_THROW(v_map.as<bool>(), EPException);

  EXPECT_EQ(0, v_null.as<char>());
  EXPECT_EQ(1, v_bool.as<int8_t>());
  EXPECT_EQ(10, v_int.as<uint8_t>());
  EXPECT_EQ(10, v_float.as<char>());
  EXPECT_EQ(0, v_shortString.as<char>());
  EXPECT_EQ(0, v_longString.as<char>());
  EXPECT_THROW(v_array.as<char>(), EPException);
  EXPECT_THROW(v_map.as<char>(), EPException);

  EXPECT_EQ(0, v_null.as<int64_t>());
  EXPECT_EQ(1, v_bool.as<int64_t>());
  EXPECT_EQ(10, v_int.as<int64_t>());
  EXPECT_EQ(10, v_float.as<int64_t>());
  EXPECT_EQ(0, v_shortString.as<int64_t>());
  EXPECT_EQ(0, v_longString.as<int64_t>());
  EXPECT_THROW(v_array.as<int64_t>(), EPException);
  EXPECT_THROW(v_map.as<int64_t>(), EPException);

  EXPECT_EQ(0.0, v_null.as<float>());
  EXPECT_EQ(1.0, v_bool.as<float>());
  EXPECT_EQ(10.0, v_int.as<float>());
  EXPECT_EQ(10.0, v_float.as<float>());
  EXPECT_EQ(0.0, v_shortString.as<float>());
  EXPECT_EQ(0.0, v_longString.as<float>());
  EXPECT_THROW(v_array.as<float>(), EPException);
  EXPECT_THROW(v_map.as<float>(), EPException);

  EXPECT_EQ(nullptr, v_null.as<String>());
  EXPECT_THROW(v_bool.as<String>(), EPException);
  EXPECT_THROW(v_int.as<String>(), EPException);
  EXPECT_THROW(v_float.as<String>(), EPException);
  EXPECT_TRUE(v_shortString.as<String>().eq("false"));
  EXPECT_TRUE(v_longString.as<String>().eq("The quick brown fox jumps over the lazy dog"));
  EXPECT_THROW(v_array.as<String>(), EPException);
  EXPECT_THROW(v_map.as<String>(), EPException);

  EXPECT_EQ(nullptr, v_null.as<SharedString>());
  EXPECT_TRUE(v_bool.as<SharedString>().eq("true"));
  EXPECT_TRUE(v_int.as<SharedString>().eq("10"));
  EXPECT_TRUE(v_float.as<SharedString>().eq("10"));
  EXPECT_TRUE(v_shortString.as<SharedString>().eq("false"));
  EXPECT_TRUE(v_longString.as<SharedString>().eq("The quick brown fox jumps over the lazy dog"));
  EXPECT_TRUE(v_array.as<SharedString>().eq("[1, 2, 3]"));
  EXPECT_THROW(v_map.as<SharedString>(), EPException);

  EXPECT_EQ(nullptr, v_null.as<Array<int>>());
  EXPECT_THROW(v_bool.as<Array<int>>(), EPException);
  EXPECT_THROW(v_int.as<Array<int>>(), EPException);
  EXPECT_THROW(v_float.as<Array<int>>(), EPException);
  EXPECT_THROW(v_shortString.as<Array<int>>(), EPException);
  EXPECT_THROW(v_longString.as<Array<int>>(), EPException);
  EXPECT_TRUE(v_array.as<Array<int>>().eq(Slice<const int>{ 1, 2, 3 }));
  EXPECT_TRUE(v_map.as<Array<int>>().eq(Slice<const int>{}));

  EXPECT_EQ(Float2::create(1.f, 2.f), v_array.as<Float2>());
  EXPECT_EQ(Double3::create(1.0, 2.0, 3.0), v_array.as<Double3>());
  EXPECT_THROW(v_array.as<Double4>(), EPException);
}


// TODO : Implement Enums, ComponentRefs, Delegates, strings and KeyValuePair Arrays
