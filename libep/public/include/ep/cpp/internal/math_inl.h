#include <float.h>
#include <math.h>

namespace ep {

template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector2<F> &v, const VarArg *pArgs);
template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector3<F> &v, const VarArg *pArgs);
template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Vector4<F> &v, const VarArg *pArgs);
template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Quaternion<F> &q, const VarArg *pArgs);
template<typename F>
ptrdiff_t epStringify(Slice<char> buffer, String format, const Matrix4x4<F> &m, const VarArg *pArgs);

epforceinline float Pow(float f, float n) { return powf(f, n); }
epforceinline double Pow(double d, double n) { return pow(d, n); }
epforceinline float LogN(float f) { return logf(f); }
epforceinline double LogN(double d) { return log(d); }
epforceinline float Log2(float f) { return log2f(f); }
epforceinline double Log2(double d) { return log2(d); }
epforceinline float Log10(float f) { return log10f(f); }
epforceinline double Log10(double d) { return log10(d); }
epforceinline float RSqrt(float f) { return 1.f/sqrtf(f); }
epforceinline double RSqrt(double d) { return 1.0/sqrt(d); }
epforceinline float Sqrt(float f) { return sqrtf(f); }
epforceinline double Sqrt(double d) { return sqrt(d); }
epforceinline float Sin(float f) { return sinf(f); }
epforceinline double Sin(double d) { return sin(d); }
epforceinline float Cos(float f) { return cosf(f); }
epforceinline double Cos(double d) { return cos(d); }
epforceinline float Tan(float f) { return tanf(f); }
epforceinline double Tan(double d) { return tan(d); }
epforceinline float ASin(float f) { return asinf(f); }
epforceinline double ASin(double d) { return asin(d); }
epforceinline float ACos(float f) { return acosf(f); }
epforceinline double ACos(double d) { return acos(d); }
epforceinline float ATan(float f) { return atanf(f); }
epforceinline double ATan(double d) { return atan(d); }
epforceinline float ATan2(float y, float x) { return atan2f(y, x); }
epforceinline double ATan2(double y, double x) { return atan2(y, x); }

epforceinline float Round(float f) { return f >= 0.0f ? floorf(f + 0.5f) : ceilf(f - 0.5f); }
epforceinline double Round(double d) { return d >= 0.0 ? floor(d + 0.5) : ceil(d - 0.5); }
epforceinline float Floor(float f) { return floorf(f); }
epforceinline double Floor(double d) { return floor(d); }
epforceinline float Ceil(float f) { return ceilf(f); }
epforceinline double Ceil(double d) { return ceil(d); }

template <typename T> T RoundEven(T t)
{
  int integer = (int)t;
  T integerPart = Floor(t);
  T fractionalPart = t - integer;

  if (fractionalPart > T(0.5) || fractionalPart < T(0.5))
    return Round(t);
  else if ((integer % 2) == 0)
    return integerPart;
  else if (integer < 0)
    return integerPart - T(1.0); // Negative values should be +1 negative
  else
    return integerPart + T(1.0);
}

template <typename T> T            Abs(T v) { return v < T(0) ? -v : v; }
template <typename T> Vector2<T> Abs(const Vector2<T> &v) { Vector2<T> r = { v.x<T(0) ? -v.x : v.x, v.y<T(0) ? -v.y : v.y }; return r; }
template <typename T> Vector3<T> Abs(const Vector3<T> &v) { Vector3<T> r = { v.x<T(0) ? -v.x : v.x, v.y<T(0) ? -v.y : v.y, v.z<T(0) ? -v.z : v.z }; return r; }
template <typename T> Vector4<T> Abs(const Vector4<T> &v) { Vector4<T> r = { v.x<T(0) ? -v.x : v.x, v.y<T(0) ? -v.y : v.y, v.z<T(0) ? -v.z : v.z, v.w<T(0) ? -v.w : v.w }; return r; }

template <typename T> T            Max(T a, T b) { return (a > b) ? a : b; }
template <typename T> Vector2<T> Min(const Vector2<T> &v1, const Vector2<T> &v2) { Vector2<T> r = { v1.x<v2.x ? v1.x : v2.x, v1.y<v2.y ? v1.y : v2.y }; return r; }
template <typename T> Vector3<T> Min(const Vector3<T> &v1, const Vector3<T> &v2) { Vector3<T> r = { v1.x<v2.x ? v1.x : v2.x, v1.y<v2.y ? v1.y : v2.y, v1.z<v2.z ? v1.z : v2.z }; return r; }
template <typename T> Vector4<T> Min(const Vector4<T> &v1, const Vector4<T> &v2) { Vector4<T> r = { v1.x<v2.x ? v1.x : v2.x, v1.y<v2.y ? v1.y : v2.y, v1.z<v2.z ? v1.z : v2.z, v1.w<v2.w ? v1.w : v2.w }; return r; }
template <typename T> T            Min(T a, T b) { return (a < b) ? a : b; }
template <typename T> Vector2<T> Max(const Vector2<T> &v1, const Vector2<T> &v2) { Vector2<T> r = { v1.x>v2.x ? v1.x : v2.x, v1.y>v2.y ? v1.y : v2.y }; return r; }
template <typename T> Vector3<T> Max(const Vector3<T> &v1, const Vector3<T> &v2) { Vector3<T> r = { v1.x>v2.x ? v1.x : v2.x, v1.y>v2.y ? v1.y : v2.y, v1.z>v2.z ? v1.z : v2.z }; return r; }
template <typename T> Vector4<T> Max(const Vector4<T> &v1, const Vector4<T> &v2) { Vector4<T> r = { v1.x>v2.x ? v1.x : v2.x, v1.y>v2.y ? v1.y : v2.y, v1.z>v2.z ? v1.z : v2.z, v1.w>v2.w ? v1.w : v2.w }; return r; }
template <typename T> T            Clamp(T v, T _min, T _max) { return v<_min ? _min : (v>_max ? _max : v); }
template <typename T> Vector2<T> Clamp(const Vector2<T> &v, const Vector2<T> &_min, const Vector2<T> &_max) { Vector2<T> r = { v.x<_min.x ? _min.x : (v.x>_max.x ? _max.x : v.x), v.y<_min.y ? _min.y : (v.y>_max.y ? _max.y : v.y) }; return r; }
template <typename T> Vector3<T> Clamp(const Vector3<T> &v, const Vector3<T> &_min, const Vector3<T> &_max) { Vector3<T> r = { v.x<_min.x ? _min.x : (v.x>_max.x ? _max.x : v.x), v.y<_min.y ? _min.y : (v.y>_max.y ? _max.y : v.y), v.z<_min.z ? _min.z : (v.z>_max.z ? _max.z : v.z) }; return r; }
template <typename T> Vector4<T> Clamp(const Vector4<T> &v, const Vector4<T> &_min, const Vector4<T> &_max) { Vector4<T> r = { v.x<_min.x ? _min.x : (v.x>_max.x ? _max.x : v.x), v.y<_min.y ? _min.y : (v.y>_max.y ? _max.y : v.y), v.z<_min.z ? _min.z : (v.z>_max.z ? _max.z : v.z), v.w<_min.w ? _min.w : (v.w>_max.w ? _max.w : v.w) }; return r; }

template <typename T> T Dot2(const Vector2<T> &v1, const Vector2<T> &v2) { return v1.x*v2.x + v1.y*v2.y; }
template <typename T> T Dot2(const Vector3<T> &v1, const Vector3<T> &v2) { return v1.x*v2.x + v1.y*v2.y; }
template <typename T> T Dot2(const Vector4<T> &v1, const Vector4<T> &v2) { return v1.x*v2.x + v1.y*v2.y; }
template <typename T> T Dot3(const Vector3<T> &v1, const Vector3<T> &v2) { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
template <typename T> T Dot3(const Vector4<T> &v1, const Vector4<T> &v2) { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
template <typename T> T Dot4(const Vector4<T> &v1, const Vector4<T> &v2) { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }
template <typename T> T Doth(const Vector3<T> &v3, const Vector4<T> &v4) { return v3.x*v4.x + v3.y*v4.y + v3.z*v4.z + v4.w; }
template <typename T> T DotQ(const Quaternion<T> &q1, const Quaternion<T> &q2) { return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w; }

template <typename T> T MagSq2(const Vector2<T> &v) { return v.x*v.x + v.y*v.y; }
template <typename T> T MagSq2(const Vector3<T> &v) { return v.x*v.x + v.y*v.y; }
template <typename T> T MagSq2(const Vector4<T> &v) { return v.x*v.x + v.y*v.y; }
template <typename T> T MagSq3(const Vector3<T> &v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
template <typename T> T MagSq3(const Vector4<T> &v) { return v.x*v.x + v.y*v.y + v.z*v.z; }
template <typename T> T MagSq4(const Vector4<T> &v) { return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; }

template <typename T> T Mag2(const Vector2<T> &v) { return Sqrt(v.x*v.x + v.y*v.y); }
template <typename T> T Mag2(const Vector3<T> &v) { return Sqrt(v.x*v.x + v.y*v.y); }
template <typename T> T Mag2(const Vector4<T> &v) { return Sqrt(v.x*v.x + v.y*v.y); }
template <typename T> T Mag3(const Vector3<T> &v) { return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
template <typename T> T Mag3(const Vector4<T> &v) { return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
template <typename T> T Mag4(const Vector4<T> &v) { return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }

template <typename T> T Cross2(const Vector2<T> &v1, const Vector2<T> &v2) { return v1.x*v2.y - v1.y*v2.x; }
template <typename T> T Cross2(const Vector3<T> &v1, const Vector3<T> &v2) { return v1.x*v2.y - v1.y*v2.x; }
template <typename T> T Cross2(const Vector4<T> &v1, const Vector4<T> &v2) { return v1.x*v2.y - v1.y*v2.x; }

template <typename T> Vector3<T> Cross3(const Vector3<T> &v1, const Vector3<T> &v2) { Vector3<T> r = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x }; return r; }
template <typename T> Vector3<T> Cross3(const Vector4<T> &v1, const Vector4<T> &v2) { Vector3<T> r = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x }; return r; }

template <typename T> Vector2<T> Normalize2(const Vector2<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y); Vector2<T> r = { v.x*s, v.y*s }; return r; }
template <typename T> Vector3<T> Normalize2(const Vector3<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y); Vector3<T> r = { v.x*s, v.y*s, v.z }; return r; }
template <typename T> Vector4<T> Normalize2(const Vector4<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y); Vector4<T> r = { v.x*s, v.y*s, v.z, v.w }; return r; }
template <typename T> Vector3<T> Normalize3(const Vector3<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y + v.z*v.z); Vector3<T> r = { v.x*s, v.y*s, v.z*s }; return r; }
template <typename T> Vector4<T> Normalize3(const Vector4<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y + v.z*v.z); Vector4<T> r = { v.x*s, v.y*s, v.z*s, v.w }; return r; }
template <typename T> Vector4<T> Normalize4(const Vector4<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); Vector4<T> r = { v.x*s, v.y*s, v.z*s, v.w*s }; return r; }
template <typename T> Quaternion<T> NormalizeQ(const Quaternion<T> &v) { T s = RSqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); Quaternion<T> r = { v.x*s, v.y*s, v.z*s, v.w*s }; return r; }


// many kinds of mul...
template <typename T>
Matrix4x4<T> Mul(const Matrix4x4<T> &m, T f)
{
  Matrix4x4<T> r = { {{ m.a[0]*f, m.a[1]*f, m.a[2]*f, m.a[3]*f,
                       m.a[4]*f, m.a[5]*f, m.a[6]*f, m.a[7]*f,
                       m.a[8]*f, m.a[9]*f, m.a[10]*f,m.a[11]*f,
                       m.a[12]*f,m.a[13]*f,m.a[14]*f,m.a[15]*f }} };
  return r;
}
template <typename T>
Vector2<T> Mul(const Matrix4x4<T> &m, const Vector2<T> &v)
{
  Vector2<T> r;
  r.x = m.m._00*v.x + m.m._01*v.y + m.m._03;
  r.y = m.m._10*v.x + m.m._11*v.y + m.m._13;
  return r;
}
template <typename T>
Vector3<T> Mul(const Matrix4x4<T> &m, const Vector3<T> &v)
{
  Vector3<T> r;
  r.x = m.m._00*v.x + m.m._01*v.y + m.m._02*v.z + m.m._03;
  r.y = m.m._10*v.x + m.m._11*v.y + m.m._12*v.z + m.m._13;
  r.z = m.m._20*v.x + m.m._21*v.y + m.m._22*v.z + m.m._23;
  return r;
}
template <typename T>
Vector4<T> Mul(const Matrix4x4<T> &m, const Vector4<T> &v)
{
  Vector4<T> r;
  r.x = m.m._00*v.x + m.m._01*v.y + m.m._02*v.z + m.m._03*v.w;
  r.y = m.m._10*v.x + m.m._11*v.y + m.m._12*v.z + m.m._13*v.w;
  r.z = m.m._20*v.x + m.m._21*v.y + m.m._22*v.z + m.m._23*v.w;
  r.w = m.m._30*v.x + m.m._31*v.y + m.m._32*v.z + m.m._33*v.w;
  return r;
}
template <typename T>
Matrix4x4<T> Mul(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
  Matrix4x4<T> r;
  r.m._00 = m1.m._00*m2.m._00 + m1.m._01*m2.m._10 + m1.m._02*m2.m._20 + m1.m._03*m2.m._30;
  r.m._01 = m1.m._00*m2.m._01 + m1.m._01*m2.m._11 + m1.m._02*m2.m._21 + m1.m._03*m2.m._31;
  r.m._02 = m1.m._00*m2.m._02 + m1.m._01*m2.m._12 + m1.m._02*m2.m._22 + m1.m._03*m2.m._32;
  r.m._03 = m1.m._00*m2.m._03 + m1.m._01*m2.m._13 + m1.m._02*m2.m._23 + m1.m._03*m2.m._33;
  r.m._10 = m1.m._10*m2.m._00 + m1.m._11*m2.m._10 + m1.m._12*m2.m._20 + m1.m._13*m2.m._30;
  r.m._11 = m1.m._10*m2.m._01 + m1.m._11*m2.m._11 + m1.m._12*m2.m._21 + m1.m._13*m2.m._31;
  r.m._12 = m1.m._10*m2.m._02 + m1.m._11*m2.m._12 + m1.m._12*m2.m._22 + m1.m._13*m2.m._32;
  r.m._13 = m1.m._10*m2.m._03 + m1.m._11*m2.m._13 + m1.m._12*m2.m._23 + m1.m._13*m2.m._33;
  r.m._20 = m1.m._20*m2.m._00 + m1.m._21*m2.m._10 + m1.m._22*m2.m._20 + m1.m._23*m2.m._30;
  r.m._21 = m1.m._20*m2.m._01 + m1.m._21*m2.m._11 + m1.m._22*m2.m._21 + m1.m._23*m2.m._31;
  r.m._22 = m1.m._20*m2.m._02 + m1.m._21*m2.m._12 + m1.m._22*m2.m._22 + m1.m._23*m2.m._32;
  r.m._23 = m1.m._20*m2.m._03 + m1.m._21*m2.m._13 + m1.m._22*m2.m._23 + m1.m._23*m2.m._33;
  r.m._30 = m1.m._30*m2.m._00 + m1.m._31*m2.m._10 + m1.m._32*m2.m._20 + m1.m._33*m2.m._30;
  r.m._31 = m1.m._30*m2.m._01 + m1.m._31*m2.m._11 + m1.m._32*m2.m._21 + m1.m._33*m2.m._31;
  r.m._32 = m1.m._30*m2.m._02 + m1.m._31*m2.m._12 + m1.m._32*m2.m._22 + m1.m._33*m2.m._32;
  r.m._33 = m1.m._30*m2.m._03 + m1.m._31*m2.m._13 + m1.m._32*m2.m._23 + m1.m._33*m2.m._33;
  return r;
}
template <typename T>
Matrix4x4<T> Add(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
  Matrix4x4<T> r;
  r.m._00 = m1.m._00 + m2.m._00;
  r.m._01 = m1.m._01 + m2.m._01;
  r.m._02 = m1.m._02 + m2.m._02;
  r.m._03 = m1.m._03 + m2.m._03;
  r.m._10 = m1.m._10 + m2.m._10;
  r.m._11 = m1.m._11 + m2.m._11;
  r.m._12 = m1.m._12 + m2.m._12;
  r.m._13 = m1.m._13 + m2.m._13;
  r.m._20 = m1.m._20 + m2.m._20;
  r.m._21 = m1.m._21 + m2.m._21;
  r.m._22 = m1.m._22 + m2.m._22;
  r.m._23 = m1.m._23 + m2.m._23;
  r.m._30 = m1.m._30 + m2.m._30;
  r.m._31 = m1.m._31 + m2.m._31;
  r.m._32 = m1.m._32 + m2.m._32;
  r.m._33 = m1.m._33 + m2.m._33;
  return r;
}
template <typename T>
Matrix4x4<T> Sub(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2)
{
  Matrix4x4<T> r;
  r.m._00 = m1.m._00 - m2.m._00;
  r.m._01 = m1.m._01 - m2.m._01;
  r.m._02 = m1.m._02 - m2.m._02;
  r.m._03 = m1.m._03 - m2.m._03;
  r.m._10 = m1.m._10 - m2.m._10;
  r.m._11 = m1.m._11 - m2.m._11;
  r.m._12 = m1.m._12 - m2.m._12;
  r.m._13 = m1.m._13 - m2.m._13;
  r.m._20 = m1.m._20 - m2.m._20;
  r.m._21 = m1.m._21 - m2.m._21;
  r.m._22 = m1.m._22 - m2.m._22;
  r.m._23 = m1.m._23 - m2.m._23;
  r.m._30 = m1.m._30 - m2.m._30;
  r.m._31 = m1.m._31 - m2.m._31;
  r.m._32 = m1.m._32 - m2.m._32;
  r.m._33 = m1.m._33 - m2.m._33;
  return r;
}
template <typename T>
Quaternion<T> Mul(const Quaternion<T> &q1, const Quaternion<T> &q2)
{
  Quaternion<T> r;
  r.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
  r.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y;
  r.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
  r.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
  return r;
}

template <typename T>
T Lerp(T a, T b, T t)
{
  return a + (b-a)*t;
}
template <typename T>
Vector2<T> Lerp(const Vector2<T> &v1, const Vector2<T> &v2, T t)
{
  Vector2<T> r;
  T invT = T(1)-t;
  r.x = v1.x*invT + v2.x*t;
  r.y = v1.y*invT + v2.y*t;
  return r;
}
template <typename T>
Vector3<T> Lerp(const Vector3<T> &v1, const Vector3<T> &v2, T t)
{
  Vector3<T> r;
  T invT = T(1)-t;
  r.x = v1.x*invT + v2.x*t;
  r.y = v1.y*invT + v2.y*t;
  r.z = v1.z*invT + v2.z*t;
  return r;
}
template <typename T>
Vector4<T> Lerp(const Vector4<T> &v1, const Vector4<T> &v2, T t)
{
  Vector4<T> r;
  T invT = T(1)-t;
  r.x = v1.x*invT + v2.x*t;
  r.y = v1.y*invT + v2.y*t;
  r.z = v1.z*invT + v2.z*t;
  r.w = v1.w*invT + v2.w*t;
  return r;
}
template <typename T>
Matrix4x4<T> Lerp(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2, T t)
{
  return (T(1)-t)*m1 + t*m2;
}
template <typename T>
Quaternion<T> Lerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T t)
{
  Quaternion<T> r;
  T invT = T(1)-t;
  r.x = q1.x*invT + q2.x*t;
  r.y = q1.y*invT + q2.y*t;
  r.z = q1.z*invT + q2.z*t;
  r.w = q1.w*invT + q2.w*t;
  return r;
}

template <typename T>
Quaternion<T> Slerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T t)
{
  //http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/

  // quaternion to return
  Quaternion<T> r;

  // Calculate angle between them.
  T cosHalfTheta = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;

  // if q1=q2 or q1=-q2 then theta = 0 and we can return q1
  if (udAbs(cosHalfTheta) >= T(1))
  {
    r.w = q1.w;
    r.x = q1.x;
    r.y = q1.y;
    r.z = q1.z;
    return r;
  }

  // Calculate temporary values.
  T halfTheta = ACos(cosHalfTheta);
  T sinHalfTheta = Sqrt(T(1) - cosHalfTheta*cosHalfTheta);

  // if theta = 180 degrees then result is not fully defined
  // we could rotate around any axis normal to qa or qb
  if (fabs(sinHalfTheta) < T(0.001)) // fabs is floating point absolute
  {
    r.w = (q1.w * T(0.5) + q2.w * T(0.5));
    r.x = (q1.x * T(0.5) + q2.x * T(0.5));
    r.y = (q1.y * T(0.5) + q2.y * T(0.5));
    r.z = (q1.z * T(0.5) + q2.z * T(0.5));
    return r;
  }

  //calculate Quaternion.
  T ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
  T ratioB = sin(t * halfTheta) / sinHalfTheta;

  r.w = (q1.w * ratioA + q2.w * ratioB);
  r.x = (q1.x * ratioA + q2.x * ratioB);
  r.y = (q1.y * ratioA + q2.y * ratioB);
  r.z = (q1.z * ratioA + q2.z * ratioB);

  return r;
}

template <typename T>
Matrix4x4<T> Transpose(const Matrix4x4<T> &m)
{
  Matrix4x4<T> r = { {{ m.a[0], m.a[4], m.a[8], m.a[12],
                       m.a[1], m.a[5], m.a[9], m.a[13],
                       m.a[2], m.a[6], m.a[10], m.a[14],
                       m.a[3], m.a[7], m.a[11], m.a[15] }} };
  return r;
}

template <typename T>
T Determinant(const Matrix4x4<T> &m)
{
  return m.m._03*m.m._12*m.m._21*m.m._30 - m.m._02*m.m._13*m.m._21*m.m._30 - m.m._03*m.m._11*m.m._22*m.m._30 + m.m._01*m.m._13*m.m._22*m.m._30 +
    m.m._02*m.m._11*m.m._23*m.m._30 - m.m._01*m.m._12*m.m._23*m.m._30 - m.m._03*m.m._12*m.m._20*m.m._31 + m.m._02*m.m._13*m.m._20*m.m._31 +
    m.m._03*m.m._10*m.m._22*m.m._31 - m.m._00*m.m._13*m.m._22*m.m._31 - m.m._02*m.m._10*m.m._23*m.m._31 + m.m._00*m.m._12*m.m._23*m.m._31 +
    m.m._03*m.m._11*m.m._20*m.m._32 - m.m._01*m.m._13*m.m._20*m.m._32 - m.m._03*m.m._10*m.m._21*m.m._32 + m.m._00*m.m._13*m.m._21*m.m._32 +
    m.m._01*m.m._10*m.m._23*m.m._32 - m.m._00*m.m._11*m.m._23*m.m._32 - m.m._02*m.m._11*m.m._20*m.m._33 + m.m._01*m.m._12*m.m._20*m.m._33 +
    m.m._02*m.m._10*m.m._21*m.m._33 - m.m._00*m.m._12*m.m._21*m.m._33 - m.m._01*m.m._10*m.m._22*m.m._33 + m.m._00*m.m._11*m.m._22*m.m._33;
}

template <typename T>
Matrix4x4<T> Inverse(const Matrix4x4<T> &m)
{
  Matrix4x4<T> r;
  r.m._00 = m.m._12*m.m._23*m.m._31 - m.m._13*m.m._22*m.m._31 + m.m._13*m.m._21*m.m._32 - m.m._11*m.m._23*m.m._32 - m.m._12*m.m._21*m.m._33 + m.m._11*m.m._22*m.m._33;
  r.m._01 = m.m._03*m.m._22*m.m._31 - m.m._02*m.m._23*m.m._31 - m.m._03*m.m._21*m.m._32 + m.m._01*m.m._23*m.m._32 + m.m._02*m.m._21*m.m._33 - m.m._01*m.m._22*m.m._33;
  r.m._02 = m.m._02*m.m._13*m.m._31 - m.m._03*m.m._12*m.m._31 + m.m._03*m.m._11*m.m._32 - m.m._01*m.m._13*m.m._32 - m.m._02*m.m._11*m.m._33 + m.m._01*m.m._12*m.m._33;
  r.m._03 = m.m._03*m.m._12*m.m._21 - m.m._02*m.m._13*m.m._21 - m.m._03*m.m._11*m.m._22 + m.m._01*m.m._13*m.m._22 + m.m._02*m.m._11*m.m._23 - m.m._01*m.m._12*m.m._23;
  r.m._10 = m.m._13*m.m._22*m.m._30 - m.m._12*m.m._23*m.m._30 - m.m._13*m.m._20*m.m._32 + m.m._10*m.m._23*m.m._32 + m.m._12*m.m._20*m.m._33 - m.m._10*m.m._22*m.m._33;
  r.m._11 = m.m._02*m.m._23*m.m._30 - m.m._03*m.m._22*m.m._30 + m.m._03*m.m._20*m.m._32 - m.m._00*m.m._23*m.m._32 - m.m._02*m.m._20*m.m._33 + m.m._00*m.m._22*m.m._33;
  r.m._12 = m.m._03*m.m._12*m.m._30 - m.m._02*m.m._13*m.m._30 - m.m._03*m.m._10*m.m._32 + m.m._00*m.m._13*m.m._32 + m.m._02*m.m._10*m.m._33 - m.m._00*m.m._12*m.m._33;
  r.m._13 = m.m._02*m.m._13*m.m._20 - m.m._03*m.m._12*m.m._20 + m.m._03*m.m._10*m.m._22 - m.m._00*m.m._13*m.m._22 - m.m._02*m.m._10*m.m._23 + m.m._00*m.m._12*m.m._23;
  r.m._20 = m.m._11*m.m._23*m.m._30 - m.m._13*m.m._21*m.m._30 + m.m._13*m.m._20*m.m._31 - m.m._10*m.m._23*m.m._31 - m.m._11*m.m._20*m.m._33 + m.m._10*m.m._21*m.m._33;
  r.m._21 = m.m._03*m.m._21*m.m._30 - m.m._01*m.m._23*m.m._30 - m.m._03*m.m._20*m.m._31 + m.m._00*m.m._23*m.m._31 + m.m._01*m.m._20*m.m._33 - m.m._00*m.m._21*m.m._33;
  r.m._22 = m.m._01*m.m._13*m.m._30 - m.m._03*m.m._11*m.m._30 + m.m._03*m.m._10*m.m._31 - m.m._00*m.m._13*m.m._31 - m.m._01*m.m._10*m.m._33 + m.m._00*m.m._11*m.m._33;
  r.m._23 = m.m._03*m.m._11*m.m._20 - m.m._01*m.m._13*m.m._20 - m.m._03*m.m._10*m.m._21 + m.m._00*m.m._13*m.m._21 + m.m._01*m.m._10*m.m._23 - m.m._00*m.m._11*m.m._23;
  r.m._30 = m.m._12*m.m._21*m.m._30 - m.m._11*m.m._22*m.m._30 - m.m._12*m.m._20*m.m._31 + m.m._10*m.m._22*m.m._31 + m.m._11*m.m._20*m.m._32 - m.m._10*m.m._21*m.m._32;
  r.m._31 = m.m._01*m.m._22*m.m._30 - m.m._02*m.m._21*m.m._30 + m.m._02*m.m._20*m.m._31 - m.m._00*m.m._22*m.m._31 - m.m._01*m.m._20*m.m._32 + m.m._00*m.m._21*m.m._32;
  r.m._32 = m.m._02*m.m._11*m.m._30 - m.m._01*m.m._12*m.m._30 - m.m._02*m.m._10*m.m._31 + m.m._00*m.m._12*m.m._31 + m.m._01*m.m._10*m.m._32 - m.m._00*m.m._11*m.m._32;
  r.m._33 = m.m._01*m.m._12*m.m._20 - m.m._02*m.m._11*m.m._20 + m.m._02*m.m._10*m.m._21 - m.m._00*m.m._12*m.m._21 - m.m._01*m.m._10*m.m._22 + m.m._00*m.m._11*m.m._22;
  return r*(T(1)/Determinant(m));
}
template <typename T>
Quaternion<T> Inverse(const Quaternion<T> &q)
{
  T s = T(1)/(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
  Quaternion<T> r = { -q.x*s, -q.y*s, -q.z*s, q.w*s };
  return r;
}

template <typename T>
Quaternion<T> Conjugate(const Quaternion<T> &q)
{
  Quaternion<T> r = { -q.x, -q.y, -q.z, q.w };
  return r;
}

// Matrix4x4 members
template <typename T>
Matrix4x4<T>& Matrix4x4<T>::transpose()
{
  *this = Transpose(*this);
  return *this;
}

template <typename T>
T Matrix4x4<T>::determinant()
{
  return Determinant(*this);
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::inverse()
{
  *this = Inverse(*this);
  return *this;
}

template <typename T>
Vector3<T> Matrix4x4<T>::extractYPR() const
{
  Vector3<T> mx = Normalize3(axis.x.toVector3());
  Vector3<T> my = Normalize3(axis.y.toVector3());
  Vector3<T> mz = Normalize3(axis.z.toVector3());

  T Cy, Sy; // yaw
  T Cp, Sp; // pitch
  T Cr, Sr; // roll

  // our YPR matrix looks like this:
  // [ CyCr-SySpSr, -SyCp, CySr+CrSySp ] ** Remember, axiis are columns!
  // [ CrSy+CySpSr,  CyCp, SySr-CyCrSp ]
  // [    -SrCp,      Sp,     CrCp     ]

  // we know sin(pitch) without doing any work!
  Sp = my.z;

  // use trig identity (cos(p))^2 + (sin(p))^2 = 1, to find cos(p)
  Cp = Sqrt(1.0f - Sp*Sp);

  // There's an edge case when
  if (Cp >= 0.00001f)
  {
    // find the rest from other cells
    T factor = T(1)/Cp;
    Sy = -my.x*factor;
    Cy = my.y*factor;
    Sr = -mx.z*factor;
    Cr = mz.z*factor;
  }
  else
  {
    // if Cp = 0 and Sp = ±1 (pitch straight up or down), the above matrix becomes:
    // [ CyCr-SySr,  0,  CySr+CrSySp ] ** Remember, axiis are columns!
    // [ CrSy+CySr,  0,  SySr-CyCrSp ]
    // [     0,      Sp,     0       ]
    // we can't distinguish yaw from roll at this point,
    // we'll assume zero roll and take the rotation as yaw:
    Sr = 0.0f; //sin(0) = 0
    Cr = 1.0f; //cos(0) = 1

    // so we can now get the yaw:
    // [ Cy, 0,  SySp ] ** Remember, axiis are columns!
    // [ Sy, 0, -CySp ]
    // [ 0,  Sp,  0   ]
    Sy = mx.y;
    Cy = mx.x;
  }

  // use tan(y) = sin(y) / cos(y)
  // -> y = atan(sin(y) / cos(y))
  // -> y = atan2(sin(y) , cos(y))
  T y = atan2(Sy, Cy);
  T p = atan2(Sp, Cp);
  T r = atan2(Sr, Cr);
  return Vector3<T>::create(y < T(0) ? y + T(EP_2PI) : y, p < T(0) ? p + T(EP_2PI) : p, r < T(0) ? r + T(EP_2PI) : r);
}


// Quaternion members
template <typename T>
Quaternion<T>& Quaternion<T>::inverse()
{
  *this = Inverse(*this);
  return *this;
}

template <typename T>
Quaternion<T>& Quaternion<T>::conjugate()
{
  *this = Conjugate(*this);
  return *this;
}

template <typename T>
Vector3<T> Quaternion<T>::apply(const Vector3<T> &v)
{
  Vector3<T> r;

  float vMult = 2.0f*(x*v.x + y*v.y + z*v.z);
  float crossMult = 2.0f*w;
  float pMult = crossMult*w - 1.0f;

  r.x = pMult*v.x + vMult*x + crossMult*(y*v.z - z*v.y);
  r.y = pMult*v.y + vMult*y + crossMult*(z*v.x - x*v.z);
  r.z = pMult*v.z + vMult*z + crossMult*(x*v.y - y*v.x);

  return r;
}

template <typename T>
Vector3<T> Quaternion<T>::eulerAngles()
{
  Vector3<T> r = {
    ATan2(T(2) * y*w - T(2) * x*z, T(1) - T(2) * y*y - T(2) * z*z),
    ASin(T(2) * x*y + T(2) * z*w),
    ATan2(T(2) * x*w - T(2) * y*z, T(1) - T(2) * x*x - T(2) * z*z),
  };

  return r;
}

// Matrix4x4 initialisers
template <typename T>
Matrix4x4<T> Matrix4x4<T>::identity()
{
  Matrix4x4<T> r = { {{ T(1),T(0),T(0),T(0),
                       T(0),T(1),T(0),T(0),
                       T(0),T(0),T(1),T(0),
                       T(0),T(0),T(0),T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::create(const T m[16])
{
  Matrix4x4<T> r = { {{ m[0], m[1], m[2], m[3],
                       m[4], m[5], m[6], m[7],
                       m[8], m[9], m[10],m[11],
                       m[12],m[13],m[14],m[15] }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::create(T _00, T _10, T _20, T _30, T _01, T _11, T _21, T _31, T _02, T _12, T _22, T _32, T _03, T _13, T _23, T _33)
{
  Matrix4x4<T> r = { {{ _00, _10, _20, _30,  // NOTE: remember, this looks a bit funny because we store columns (axiis) contiguous!
                       _01, _11, _21, _31,
                       _02, _12, _22, _32,
                       _03, _13, _23, _33 }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::create(const Vector4<T> &xColumn, const Vector4<T> &yColumn, const Vector4<T> &zColumn, const Vector4<T> &wColumn)
{
  Matrix4x4<T> r = { {{ xColumn.x, xColumn.y, xColumn.z, xColumn.w,
                       yColumn.x, yColumn.y, yColumn.z, yColumn.w,
                       zColumn.x, zColumn.y, zColumn.z, zColumn.w,
                       wColumn.x, wColumn.y, wColumn.z, wColumn.w }} };
  return r;
}

template <typename T>
template <typename U> // OMG, nested templates... I didn't even know this was a thing!
Matrix4x4<T> Matrix4x4<T>::create(const Matrix4x4<U> &_m)
{
  Matrix4x4<T> r = { {{ T(_m.m._00), T(_m.m._10), T(_m.m._20), T(_m.m._30),
                       T(_m.m._01), T(_m.m._11), T(_m.m._21), T(_m.m._31),
                       T(_m.m._02), T(_m.m._12), T(_m.m._22), T(_m.m._32),
                       T(_m.m._03), T(_m.m._13), T(_m.m._23), T(_m.m._33) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationX(T rad, const Vector3<T> &t)
{
  T c = Cos(rad);
  T s = Sin(rad);
  Matrix4x4<T> r = { {{ T(1),T(0),T(0),T(0),
                       T(0),  c ,  s ,T(0),
                       T(0), -s ,  c ,T(0),
                       t.x, t.y, t.z, T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationY(T rad, const Vector3<T> &t)
{
  T c = Cos(rad);
  T s = Sin(rad);
  Matrix4x4<T> r = { {{   c ,T(0), -s, T(0),
                       T(0),T(1),T(0),T(0),
                         s ,T(0),  c ,T(0),
                       t.x, t.y, t.z, T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationZ(T rad, const Vector3<T> &t)
{
  T c = Cos(rad);
  T s = Sin(rad);
  Matrix4x4<T> r = { {{   c ,  s ,T(0),T(0),
                        -s ,  c ,T(0),T(0),
                       T(0),T(0),T(1),T(0),
                       t.x, t.y, t.z, T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationAxis(const Vector3<T> &axis, T rad, const Vector3<T> &t)
{
  T c = Cos(rad);
  T s = Sin(rad);
  Vector3<T> n = axis;
  Vector3<T> a = (T(1) - c) * axis;
  Matrix4x4<T> r = { {{ a.x*n.x + c,     a.x*n.y + s*n.z, a.x*n.z - s*n.y, T(0),
                       a.y*n.x - s*n.z, a.y*n.y + c,     a.y*n.z + s*n.x, T(0),
                       a.z*n.x + s*n.y, a.z*n.y - s*n.x, a.z*n.z + c,     T(0),
                       t.x,             t.y,             t.z,             T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationYPR(T y, T p, T r, const Vector3<T> &t)
{
  T c1 = Cos(y), c2 = Cos(p), c3 = Cos(r);
  T s1 = Sin(y), s2 = Sin(p), s3 = Sin(r);
  Matrix4x4<T> result = { {{  c1*c3-s1*s2*s3, c3*s1+c1*s2*s3, -c2*s3, T(0),
                            -c2*s1,          c1*c2,           s2,    T(0),
                             c1*s3+c3*s1*s2, s1*s3-c1*c3*s2,  c2*c3, T(0),
                             t.x,            t.y,             t.z,   T(1) }} };
  return result;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::rotationQuat(const Quaternion<T> &q, const Vector3<T> &t)
{
  Matrix4x4<T> r = translation(t);
  //EPASSERT(false, "TODO");

  T sqw = q.w * q.w;
  T sqx = q.x * q.x;
  T sqy = q.y * q.y;
  T sqz = q.z * q.z;

  // invs (inverse square length) is only required if quaternion is not already normalised
  T invs = T(1) / (sqx + sqy + sqz + sqw);

  r.m._11 = (+sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
  r.m._22 = (-sqx + sqy - sqz + sqw)*invs;
  r.m._00 = (-sqx - sqy + sqz + sqw)*invs;

  T tmp1 = q.x*q.y;
  T tmp2 = q.z*q.w;
  r.m._21 = T(2) * (tmp1 + tmp2)*invs;
  r.m._12 = T(2) * (tmp1 - tmp2)*invs;

  tmp1 = q.x*q.z;
  tmp2 = q.y*q.w;
  r.m._01 = T(2) * (tmp1 - tmp2)*invs;
  r.m._10 = T(2) * (tmp1 + tmp2)*invs;

  tmp1 = q.y*q.z;
  tmp2 = q.x*q.w;
  r.m._02 = T(2) * (tmp1 + tmp2)*invs;
  r.m._20 = T(2) * (tmp1 - tmp2)*invs;

  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::translation(T x, T y, T z)
{
  Matrix4x4<T> r = { {{ T(1),T(0),T(0),T(0),
                       T(0),T(1),T(0),T(0),
                       T(0),T(0),T(1),T(0),
                       x,   y,   z, T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::scaleNonUniform(T x, T y, T z, const Vector3<T> &t)
{
  Matrix4x4<T> r = { {{   x,  T(0), T(0), T(0),
                       T(0),   y,  T(0), T(0),
                       T(0), T(0),   z,  T(0),
                       t.x,  t.y,  t.z,  T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::perspective(T fovY, T aspectRatio, T znear, T zfar)
{
  T fov = Tan(fovY / T(2));
  Matrix4x4<T> r = { {{ T(1)/(aspectRatio*fov), T(0),         T(0),                                T(0),
                       T(0),                   T(0),         (zfar + znear) / (zfar - znear),     T(1),
                       T(0),                   T(1)/fov,     T(0),                                T(0),
                       T(0),                   T(0),         -(T(2)*zfar*znear) / (zfar - znear), T(0) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::ortho(T left, T right, T bottom, T top, T znear, T zfar)
{
  Matrix4x4<T> r = { {{ T(2) / (right - left),            T(0),                             T(0),                             T(0),
                       T(0),                             T(0),                             T(2) / (zfar - znear),            T(0),
                       T(0),                             T(2) / (top - bottom),            T(0),                             T(0),
                       -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zfar + znear) / (zfar - znear), T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::orthoForScreeen(T width, T height, T znear, T zfar)
{
  Matrix4x4<T> r = { {{ T(2) / width, T(0),           T(0),                            T(0),
                       T(0),         -T(2) / height, T(0),                            T(0),
                       T(0),         T(0),           T(2) / (zfar - znear),           T(0),
                       T(-1),        T(1),          -(zfar + znear) / (zfar - znear), T(1) }} };
  return r;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::lookAt(const Vector3<T> &from, const Vector3<T> &at, const Vector3<T> &up)
{
  Vector3<T> y = Normalize3(at - from);
  Vector3<T> x = Normalize3(Cross3(y, up));
  Vector3<T> z = Cross3(x, y);
  Matrix4x4<T> r = { {{ x.x,    x.y,    x.z,    0,
                       y.x,    y.y,    y.z,    0,
                       z.x,    z.y,    z.z,    0,
                       from.x, from.y, from.z, 1 }} };
  return r;
}

template <typename T>
Quaternion<T> Quaternion<T>::create(const Vector3<T> &axis, T rad)
{
  T a = rad*T(0.5);
  T s = Sin(a);
  Quaternion<T> r = { axis.x*s,
                      axis.y*s,
                      axis.z*s,
                      Cos(a) };
  return r;
}

template <typename T>
Quaternion<T> Quaternion<T>::create(const T _y, const T _p, const T _r)
{
  Quaternion<T> r;

  // Assuming the angles are in radians.
  T c1 = Cos(_y / 2); //Yaw
  T s1 = Sin(_y / 2); //Yaw
  T c2 = Cos(_p / 2); //Pitch
  T s2 = Sin(_p / 2); //Pitch
  T c3 = Cos(_r / 2); //Roll
  T s3 = Sin(_r / 2); //Roll
  T c1c2 = c1 * c2;
  T s1s2 = s1 * s2;

  r.w = c1c2 * c3 - s1s2 * s3;
  r.x = c1c2 * s3 + s1s2 * c3;
  r.y = s1 * c2 * c3 + c1 * s2 * s3;
  r.z = c1 * s2 * c3 - s1 * c2 * s3;

  return r;
}

} //namespace ep
