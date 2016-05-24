#include "ep/cpp/platform.h"
#include "ep/cpp/math.h"

namespace ep {
template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector2<F> &v, const VarArg *pArgs)
{
  return ::epStringify(buffer, format, Slice<F>((F*)&v, 2), pArgs);
}
template ptrdiff_t epStringify<float>(Slice<char> buffer, String format, const Vector2<float> &v, const VarArg *pArgs);
template ptrdiff_t epStringify<double>(Slice<char> buffer, String format, const Vector2<double> &v, const VarArg *pArgs);

template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector3<F> &v, const VarArg *pArgs)
{
  return ::epStringify(buffer, format, Slice<F>((F*)&v, 3), pArgs);
}
template ptrdiff_t epStringify<float>(Slice<char> buffer, String format, const Vector3<float> &v, const VarArg *pArgs);
template ptrdiff_t epStringify<double>(Slice<char> buffer, String format, const Vector3<double> &v, const VarArg *pArgs);

template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector4<F> &v, const VarArg *pArgs)
{
  return ::epStringify(buffer, format, Slice<F>((F*)&v, 4), pArgs);
}
template ptrdiff_t epStringify<float>(Slice<char> buffer, String format, const Vector4<float> &v, const VarArg *pArgs);
template ptrdiff_t epStringify<double>(Slice<char> buffer, String format, const Vector4<double> &v, const VarArg *pArgs);

template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Quaternion<F> &q, const VarArg *pArgs)
{
  return ::epStringify(buffer, format, Slice<F>((F*)&q, 4), pArgs);
}
template ptrdiff_t epStringify<float>(Slice<char> buffer, String format, const Quaternion<float> &q, const VarArg *pArgs);
template ptrdiff_t epStringify<double>(Slice<char> buffer, String format, const Quaternion<double> &q, const VarArg *pArgs);

template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Matrix4x4<F> &m, const VarArg *pArgs)
{
  // TODO: change this to emit [,,,][,,,][,,,][,,,] instead of a 16 vector??
  return ::epStringify(buffer, format, Slice<F>((F*)&m, 16), pArgs);
}
template ptrdiff_t epStringify<float>(Slice<char> buffer, String format, const Matrix4x4<float> &m, const VarArg *pArgs);
template ptrdiff_t epStringify<double>(Slice<char> buffer, String format, const Matrix4x4<double> &m, const VarArg *pArgs);

} // namespace ep

ep::Result epMath_Test()
{
  using namespace ep;

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

  return Result::Success;
}

