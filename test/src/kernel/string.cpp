#include "gtest/gtest.h"
#include "ep/cpp/platform.h"
// TODO: fill out these tests

void receivesString(String)
{
}
void receivesCString(const char*)
{
}

// TODO: these tests are old and should be deprecated!!
TEST(EPStringTest, Deprecated)
{
  // String
  char buffer[] = "world";

  String s1 = "hello";        // initialise from string
  String s2(buffer, 3);       // initialise to sub-string; ie, "wor"

  s1.eq(s2);                    // strcmp() == 0
  s1.eqIC(s2);                   // case insensitive; stricmp() == 0
  s1.eq("hello");               // compare with c-string

  SharedArray<char16_t> wcs(s1);   // init wchar string from c-string! yay unicode! (except we probably also want to decode utf-8...)

  wcs.eq(s1);                   // compare wide-char and ascii strings

  auto subStr = s1.slice(1, 4); // string slice; "ell"
  subStr.ptr = subStr.ptr;      // fixes gcc warnings

  s2.toStringz(buffer, sizeof(buffer)); // write String to c-string


                                        // MutableString
  MutableString<64> s_s1(s1);
  String s_slice = s_s1.slice(1, 4); // slices of Array are not owned; they die when the parent allocation dies
  s_slice.ptr = s_slice.ptr;     // fixes gcc warnings

  s_s1.eqIC("HELLO");            // string comparison against string literals

  receivesString(s_s1);         // pass to functions

  s_s1.reserve(100);            // reserve a big buffer
  EPASSERT(s_s1.eq(s1), "!");   // the existing contents is preserved

  s_s1.concat(s1, "!!", String("world"));


  // SharedString
  SharedString rcs1(s1);          // RC string initialised from some slice
  SharedString rcs2("string");    // also from literal
  SharedString rcs3(buffer, 4);   // also from c-string (and optionally a slice thereof)

  receivesString(rcs1);         // pass to functions

  MutableString<64> ss2 = rcs2; // stack string takes copy of a SharedString
  SharedString rcs4 = ss2;        // rc strings take copy of stack strings too

  rcs1 == s1;                   // compare SharedString and String pointers

  rcs1.eqIC(s1);                 // string comparison works too between SharedString and String

                                 //  SharedString::format("Format: %s", "hello");  // create from format string

                                 //  char temp[256];
                                 //  fopen(rcs1.toStringz(temp, sizeof(temp)), "ro"); // write SharedString to c-string, for passing to OS functions or C api's
                                 // unlike c_str(), user supplies buffer (saves allocations)

  SharedString r2 = SharedString::concat(s1, "!!", rcs1, String("world"), s_s1);
  //  SharedString::format("x{1}_{2}", "10", "200");

  receivesCString(r2.toStringz());

  const char *pName = "manu";
  SharedString cc = SharedString::concat("hello ", pName, 10);
  SharedString fmt = SharedString::format("{ 1 }, {2}, { 0 , hello }", "hello ", pName, 10);

  MutableString<0> ms(Concat, "hello ", pName, 10);
  ms.append("poop!");

  int arr[] = { 1, 2, 30 };
  ms.format("{1}, {'?',?7}{'!',!7}, {@6} {3}", "hello ", pName, 10, Slice<int>(arr, 3), "*5", 10, "!{0,@4}!", false);

  Slice<const void> poo;
  //  poo.slice(1, 3);
  //  poo[2];
  //  poo.alloc(10);

  WString wstr = (const char16_t*)L"xyz"; // TODO: HACK! should be: u"xyz" (utf16), NOT L"xyz" (wchar_t)
  ms.format("{0}", wstr);

  cc.parseInt();
}

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
