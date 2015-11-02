#include "ep/cpp/platform.h"
#include "ep/cpp/math.h"

epResult epMath_Test()
{
  Float4 x;
  Float4x4 m;
//  Double4x4 dm;

  Float3 v;
  Float4 v4;

  float f = Dot3(v, v);
  Pow(f, (float)EP_PI);

  v = 2.f*v;
  v = v*v+v;

  m = Float4x4::create(x, x, x, x);

  m.transpose();
  m.determinant();
  m.inverse();

  Cross3(v, v4.toVector3());

  v += v;
  v /= 2.f;

  v *= v.one();

  Mul(m, 1.f);
  Mul(m, v);
  Mul(m, v4);
  Mul(m, m);

  Abs(-1.f);
  Abs(-1.0);
  Abs(-1);
  Abs(v);

//  Float4x4 r = Float4x4::rotationYPR(4, 1, 1);
//  Float3 ypr = r.extractYPR();
//  Float4x4 r2 = Float4x4::rotationYPR(ypr);

  return epR_Success;
}

