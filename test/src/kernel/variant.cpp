#include "gtest/gtest.h"
#include "ep/cpp/variant.h"
#include <random>

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
  EXPECT_TRUE(var.asAssocArraySeries().empty());

  EXPECT_EQ(0, var.arrayLen());
  EXPECT_EQ(0, var.assocArraySeriesLen());

  EXPECT_TRUE(var[0].is(Variant::Type::Null));
  EXPECT_TRUE(var["dummy"].is(Variant::Type::Null));

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

  EXPECT_DEATH(varTrue.asComponent(), "Wrong type!");
  EXPECT_DEATH(varTrue.asDelegate(), "Wrong type!");
  EXPECT_DEATH(varTrue.asString().empty(), "Wrong type!");

  EXPECT_DEATH(varTrue.asArray().empty(), "Wrong type!");
  EXPECT_DEATH(varTrue.asAssocArray().empty(), "Wrong type!");
  EXPECT_DEATH(varTrue.asAssocArraySeries().empty(), "Wrong type!");

  EXPECT_EQ(0, varTrue.arrayLen());
  EXPECT_EQ(0, varTrue.assocArraySeriesLen());

  EXPECT_TRUE(varTrue[0].is(Variant::Type::Null));
  EXPECT_TRUE(varTrue["dummy"].is(Variant::Type::Null));
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

  EXPECT_DEATH(varInt.asComponent(), "Wrong type!");
  EXPECT_DEATH(varInt.asDelegate(), "Wrong type!");
  EXPECT_DEATH(varInt.asString().empty(), "Wrong type!");

  EXPECT_DEATH(varInt.asArray().empty(), "Wrong type!");
  EXPECT_DEATH(varInt.asAssocArray().empty(), "Wrong type!");
  EXPECT_DEATH(varInt.asAssocArraySeries().empty(), "Wrong type!");

  EXPECT_EQ(0, varInt.arrayLen());
  EXPECT_EQ(0, varInt.assocArraySeriesLen());

  EXPECT_TRUE(varInt[0].is(Variant::Type::Null));
  EXPECT_TRUE(varInt["dummy"].is(Variant::Type::Null));
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

  EXPECT_DEATH(varDbl.asComponent(), "Wrong type!");
  EXPECT_DEATH(varDbl.asDelegate(), "Wrong type!");
  EXPECT_DEATH(varDbl.asString().empty(), "Wrong type!");

  EXPECT_DEATH(varDbl.asArray().empty(), "Wrong type!");
  EXPECT_DEATH(varDbl.asAssocArray().empty(), "Wrong type!");
  EXPECT_DEATH(varDbl.asAssocArraySeries().empty(), "Wrong type!");

  EXPECT_EQ(0, varDbl.arrayLen());
  EXPECT_EQ(0, varDbl.assocArraySeriesLen());

  EXPECT_TRUE(varDbl[0].is(Variant::Type::Null));
  EXPECT_TRUE(varDbl["dummy"].is(Variant::Type::Null));
}


TEST(EPVariant, EPVariantSlice)
{
  Variant var0(0);
  Variant var1(1);
  Variant var2(2);
  Variant testArray[] = { var0, var1, var2 };
  Slice<Variant> testSlice(testArray, 3);

  Variant varSlice(testSlice);

  EXPECT_EQ(varSlice.type(), Variant::Type::Array);
  EXPECT_TRUE(varSlice.is(Variant::Type::Array));
  EXPECT_TRUE(varSlice.stringify().empty());

  EXPECT_DEATH(varSlice.asBool(), "Wrong type!");
  EXPECT_DEATH(varSlice.asInt(), "Wrong type!");
  EXPECT_DEATH(varSlice.asFloat(), "Wrong type!");

  EXPECT_FALSE(varSlice.asEnum(nullptr));

  EXPECT_DEATH(varSlice.asComponent(), "Wrong type!");
  EXPECT_DEATH(varSlice.asDelegate(), "Wrong type!");
  EXPECT_DEATH(varSlice.asString().empty(), "Wrong type!");

  EXPECT_TRUE(testSlice == varSlice.asArray());

  EXPECT_DEATH(varSlice.asAssocArray().empty(), "Wrong type!");
  EXPECT_DEATH(varSlice.asAssocArraySeries().empty(), "Wrong type!");

  EXPECT_EQ(varSlice[0].asInt(), 0);
  EXPECT_EQ(varSlice[1].asInt(), 1);
  EXPECT_EQ(varSlice[2].asInt(), 2);
  EXPECT_DEATH(varSlice[3], "Index out of range!");
}


// TODO : Implement Enums, ComponentRefs, Delegates, strings and KeyValuePair Arrays
