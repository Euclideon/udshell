#include "eptest.h"
#include "ep/version.h"


// ------------------------------- Version Tests -------------------------------------------------

TEST(Version, Extract)
{
  const char *verAllZeros = "0.0.0";
  const char *verMaj1Min2Rev3 = "1.2.3";
  const char *verMaj15Min99Rev387 = "15;99;387";
  const char *verMissingComponents1 = "36.8";
  const char *verMissingComponents2 = "136";

  epVersion result = epVersionExtract(verAllZeros);
  EXPECT_EQ(0, result.major);
  EXPECT_EQ(0, result.minor);
  EXPECT_EQ(0, result.revision);

  result = epVersionExtract(verMaj1Min2Rev3);
  EXPECT_EQ(1, result.major);
  EXPECT_EQ(2, result.minor);
  EXPECT_EQ(3, result.revision);

  result = epVersionExtract(verMaj15Min99Rev387, ";");
  EXPECT_EQ(15, result.major);
  EXPECT_EQ(99, result.minor);
  EXPECT_EQ(387, result.revision);

  result = epVersionExtract(verMissingComponents1);
  EXPECT_EQ(36, result.major);
  EXPECT_EQ(8, result.minor);
  EXPECT_EQ(0, result.revision);

  result = epVersionExtract(verMissingComponents2);
  EXPECT_EQ(136, result.major);
  EXPECT_EQ(0, result.minor);
  EXPECT_EQ(0, result.revision);
}

TEST(Version, IsCompatible)
{
  // Versions are considered API compatible if they have matching major numbers
  // Pre-Release versions (major == 0) are considered less stable and require matching minor numbers
  EXPECT_TRUE(epVersionIsCompatible("0.0.0", "0.0.0"));
  EXPECT_TRUE(epVersionIsCompatible("0.0.10", "0.0.999"));
  EXPECT_TRUE(epVersionIsCompatible("0.1.0", "0.1.99"));
  EXPECT_TRUE(epVersionIsCompatible("1.5.7", "1.2.45"));
  EXPECT_TRUE(epVersionIsCompatible("1.5.7", "1.5.45"));
  EXPECT_FALSE(epVersionIsCompatible("0.1.0", "0.2.0"));
  EXPECT_FALSE(epVersionIsCompatible("0.2.0", "0.1.0"));
  EXPECT_FALSE(epVersionIsCompatible("1.2.0", "0.1.0"));
  EXPECT_FALSE(epVersionIsCompatible("1.2.88", "2.1.77"));
}
