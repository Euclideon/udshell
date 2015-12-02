#include "gtest/gtest.h"
#include "ep/cpp/platform.h"
// TODO: fill out these tests

TEST(EPStringTest, EmptyString) {
  String nullStr;
  EXPECT_TRUE(nullStr.empty());
  EXPECT_FALSE(nullStr);  // operator bool
  EXPECT_TRUE(!nullStr);  // operator bool
  EXPECT_EQ(0, nullStr.length);
  EXPECT_EQ(nullptr, nullStr.ptr);
}

TEST(EPStringTest, NotEmptyString) {
  String nullStr("blah");
  EXPECT_FALSE(nullStr.empty());
  EXPECT_FALSE(!nullStr);   // operator bool
  EXPECT_TRUE(nullStr);     // operator bool
  EXPECT_EQ(4, nullStr.length);
  EXPECT_TRUE(nullStr.ptr != nullptr);
}

TEST(EPStringTest, GetLeftRightAt) {
  String s("Blah ab AB Whee AB ab Boo");

  // all the permutations
  EXPECT_TRUE(s.getLeftAtFirst("AB", false).eq("Blah ab "));
  EXPECT_TRUE(s.getLeftAtFirst("AB", true).eq("Blah ab AB"));
  EXPECT_TRUE(s.getLeftAtLast("AB", false).eq("Blah ab AB Whee "));
  EXPECT_TRUE(s.getLeftAtLast("AB", true).eq("Blah ab AB Whee AB"));
  EXPECT_TRUE(s.getRightAtFirst("AB", false).eq(" Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtFirst("AB", true).eq("AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtLast("AB", false).eq(" ab Boo"));
  EXPECT_TRUE(s.getRightAtLast("AB", true).eq("AB ab Boo"));

  // if the substr doesn't exist
  EXPECT_TRUE(s.getLeftAtFirst("bl", false).eq("Blah ab AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getLeftAtLast("bl", false).eq("Blah ab AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtFirst("bl", false).length == 0);
  EXPECT_TRUE(s.getRightAtLast("bl", false).length == 0);

  // case insensitive versions
  EXPECT_TRUE(s.getLeftAtFirstIC("AB", false).eq("Blah "));
  EXPECT_TRUE(s.getLeftAtFirstIC("AB", true).eq("Blah ab"));
  EXPECT_TRUE(s.getLeftAtLastIC("AB", false).eq("Blah ab AB Whee AB "));
  EXPECT_TRUE(s.getLeftAtLastIC("AB", true).eq("Blah ab AB Whee AB ab"));
  EXPECT_TRUE(s.getRightAtFirstIC("AB", false).eq(" AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtFirstIC("AB", true).eq("ab AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtLastIC("AB", false).eq(" Boo"));
  EXPECT_TRUE(s.getRightAtLastIC("AB", true).eq("ab Boo"));

  // if the substr doesn't exist
  EXPECT_TRUE(s.getLeftAtFirstIC("xx", false).eq("Blah ab AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getLeftAtLastIC("xx", false).eq("Blah ab AB Whee AB ab Boo"));
  EXPECT_TRUE(s.getRightAtFirstIC("xx", false).length == 0);
  EXPECT_TRUE(s.getRightAtLastIC("xx", false).length == 0);
}

TEST(EPMutableStringTest, URLDecode) {
  MutableString<0> spacesUrl0; spacesUrl0.urlDecode("This%20has%20spaces%20");
  String spacesString0("This has spaces ");

  MutableString<0> spacesUrl1; spacesUrl1.urlDecode("This+also+has+spaces+");
  String spacesString1("This also has spaces ");

  String safeString("The quick brown fox jumps over the lazy dog 0123456789 THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG - _ . ~");
  MutableString<0> safeUrl; safeUrl.urlDecode(safeString);

  MutableString<0> reservedUrl; reservedUrl.urlDecode("!*%27()%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D");
  String reservedString("!*'();:@&=+$,/?#[]");

  MutableString<0> unsafeUrl; unsafeUrl.urlDecode("%01%02%03%04%05%06%07%1B");
  String unsafeString("\x1\x2\x3\x4\x5\x6\x7\x1B");

  EXPECT_TRUE(spacesString0.eq(spacesUrl0));
  EXPECT_TRUE(spacesString1.eq(spacesUrl1));
  EXPECT_TRUE(safeString.eq(safeUrl));
  EXPECT_TRUE(reservedString.eq(reservedUrl));
  EXPECT_TRUE(unsafeString.eq(unsafeUrl));
}
