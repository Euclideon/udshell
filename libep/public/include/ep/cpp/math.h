#pragma once
#if !defined(EPMATH_HPP)
#define EPMATH_HPP

#include "ep/cpp/platform.h"

#define EP_PI            3.1415926535897932384626433832795
#define EP_2PI           6.283185307179586476925286766559
#define EP_HALF_PI       1.5707963267948966192313216916398
#define EP_ROOT_2        1.4142135623730950488016887242097
#define EP_INV_ROOT_2    0.70710678118654752440084436210485
#define EP_RAD2DEG(rad)  ((rad)*57.295779513082320876798154814105)
#define EP_DEG2RAD(deg)  ((deg)*0.01745329251994329576923690768489)

#define EP_PIf           float(EP_PI)
#define EP_2PIf          float(EP_2PI)
#define EP_HALF_PIf      float(EP_HALF_PI)
#define EP_ROOT_2f       float(EP_ROOT_2)
#define EP_INV_ROOT_2f   float(EP_INV_ROOT_2)
#define EP_RAD2DEGf(rad) float(EP_RAD2DEG(rad))
#define EP_DEG2RADf(deg) float(EP_DEG2RAD(deg))

namespace ep {

// prototypes
template <typename T> struct Vector2;
template <typename T> struct Vector3;
template <typename T> struct Vector4;
template <typename T> struct Quaternion;
template <typename T> struct Matrix4x4;

// math functions
float Pow(float f, float n);
double Pow(double d, double n);
float LogN(float f);
double LogN(double d);
float Log2(float f);
double Log2(double d);
float Log10(float f);
double Log10(double d);
float RSqrt(float f);
double RSqrt(double d);
float Sqrt(float f);
double Sqrt(double d);
float Sin(float f);
double Sin(double d);
float Cos(float f);
double Cos(double d);
float Tan(float f);
double Tan(double d);
float ASin(float f);
double ASin(double d);
float ACos(float f);
double ACos(double d);
float ATan(float f);
double ATan(double d);
float ATan2(float y, float x);
double ATan2(double y, double x);

// rounding functions
float Round(float f);
double Round(double d);
float Floor(float f);
double Floor(double d);
float Ceil(float f);
double Ceil(double d);
template <typename T> T RoundEven(T t);


// typical linear algebra functions
template <typename T> T          Abs(T v);
template <typename T> Vector2<T> Abs(const Vector2<T> &v);
template <typename T> Vector3<T> Abs(const Vector3<T> &v);
template <typename T> Vector4<T> Abs(const Vector4<T> &v);

template <typename T> T          Max(T a, T b);
template <typename T> Vector2<T> Max(const Vector2<T> &v1, const Vector2<T> &v2);
template <typename T> Vector3<T> Max(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> Vector4<T> Max(const Vector4<T> &v1, const Vector4<T> &v2);
template <typename T> T          Min(T a, T b);
template <typename T> Vector2<T> Min(const Vector2<T> &v1, const Vector2<T> &v2);
template <typename T> Vector3<T> Min(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> Vector4<T> Min(const Vector4<T> &v1, const Vector4<T> &v2);
template <typename T> T          Clamp(T v, T _min, T _max);
template <typename T> Vector2<T> Clamp(const Vector2<T> &v, const Vector2<T> &_min, const Vector2<T> &_max);
template <typename T> Vector3<T> Clamp(const Vector3<T> &v, const Vector3<T> &_min, const Vector3<T> &_max);
template <typename T> Vector4<T> Clamp(const Vector4<T> &v, const Vector4<T> &_min, const Vector4<T> &_max);

template <typename T> T Dot2(const Vector2<T> &v1, const Vector2<T> &v2);
template <typename T> T Dot2(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> T Dot2(const Vector4<T> &v1, const Vector4<T> &v2);
template <typename T> T Dot3(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> T Dot3(const Vector4<T> &v1, const Vector4<T> &v2);
template <typename T> T Dot4(const Vector4<T> &v1, const Vector4<T> &v2);
template <typename T> T Doth(const Vector3<T> &v3, const Vector4<T> &v4);
template <typename T> T DotQ(const Quaternion<T> &q1, const Quaternion<T> &q2);

template <typename T> T MagSq2(const Vector2<T> &v);
template <typename T> T MagSq2(const Vector3<T> &v);
template <typename T> T MagSq2(const Vector4<T> &v);
template <typename T> T MagSq3(const Vector3<T> &v);
template <typename T> T MagSq3(const Vector4<T> &v);
template <typename T> T MagSq4(const Vector4<T> &v);

template <typename T> T Mag2(const Vector2<T> &v);
template <typename T> T Mag2(const Vector3<T> &v);
template <typename T> T Mag2(const Vector4<T> &v);
template <typename T> T Mag3(const Vector3<T> &v);
template <typename T> T Mag3(const Vector4<T> &v);
template <typename T> T Mag4(const Vector4<T> &v);

template <typename T> T Cross2(const Vector2<T> &v1, const Vector2<T> &v2);
template <typename T> T Cross2(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> T Cross2(const Vector4<T> &v1, const Vector4<T> &v2);

template <typename T> Vector3<T> Cross3(const Vector3<T> &v1, const Vector3<T> &v2);
template <typename T> Vector3<T> Cross3(const Vector4<T> &v1, const Vector4<T> &v2);

template <typename T> Vector2<T> Normalize2(const Vector2<T> &v);
template <typename T> Vector3<T> Normalize2(const Vector3<T> &v);
template <typename T> Vector4<T> Normalize2(const Vector4<T> &v);
template <typename T> Vector3<T> Normalize3(const Vector3<T> &v);
template <typename T> Vector4<T> Normalize3(const Vector4<T> &v);
template <typename T> Vector4<T> Normalize4(const Vector4<T> &v);
template <typename T> Quaternion<T> NormalizeQ(const Quaternion<T> &q);

// matrix and quat functions
template <typename T> Vector2<T> Mul(const Matrix4x4<T> &m, const Vector2<T> &v);
template <typename T> Vector3<T> Mul(const Matrix4x4<T> &m, const Vector3<T> &v);
template <typename T> Vector4<T> Mul(const Matrix4x4<T> &m, const Vector4<T> &v);
template <typename T> Matrix4x4<T> Mul(const Matrix4x4<T> &m, T f);
template <typename T> Matrix4x4<T> Mul(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2);
template <typename T> Quaternion<T> Mul(const Quaternion<T> &q1, const Quaternion<T> &q2);

template <typename T> T Lerp(T a, T b, T t);
template <typename T> Vector2<T> Lerp(const Vector2<T> &v1, const Vector2<T> &v2, T t);
template <typename T> Vector3<T> Lerp(const Vector3<T> &v1, const Vector3<T> &v2, T t);
template <typename T> Vector4<T> Lerp(const Vector4<T> &v1, const Vector4<T> &v2, T t);
template <typename T> Quaternion<T> Lerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T t);
template <typename T> Matrix4x4<T> Lerp(const Matrix4x4<T> &m1, const Matrix4x4<T> &m2, T t);

template <typename T> Matrix4x4<T> Transpose(const Matrix4x4<T> &m);

template <typename T> T Determinant(const Matrix4x4<T> &m);

template <typename T> Quaternion<T> Inverse(const Quaternion<T> &q);
template <typename T> Matrix4x4<T> Inverse(const Matrix4x4<T> &m);

template <typename T> Quaternion<T> Conjugate(const Quaternion<T> &q);

template <typename T> Quaternion<T> Slerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T t);


// types
template <typename T>
struct Vector2
{
  T x, y;

  Vector2<T> operator -() const { Vector2<T> r = { -x, -y }; return r; }

  Vector2<T> operator +(const Vector2<T> &v) const { Vector2<T> r = { x+v.x, y+v.y }; return r; }
  Vector2<T> operator -(const Vector2<T> &v) const { Vector2<T> r = { x-v.x, y-v.y }; return r; }
  Vector2<T> operator *(const Vector2<T> &v) const { Vector2<T> r = { x*v.x, y*v.y }; return r; }
  Vector2<T> operator *(T f) const { Vector2<T> r = { x*f,   y*f }; return r; }
  Vector2<T> operator /(const Vector2<T> &v) const { Vector2<T> r = { x/v.x, y/v.y }; return r; }
  Vector2<T> operator /(T f) const { Vector2<T> r = { x/f,   y/f }; return r; }

  bool       operator ==(const Vector2<T> &v) const { return x==v.x && y==v.y; }
  bool       operator !=(const Vector2<T> &v) const { return x!=v.x || y!=v.y; }

  Vector2<T>& operator +=(const Vector2<T> &v) { x += v.x; y += v.y; return *this; }
  Vector2<T>& operator -=(const Vector2<T> &v) { x -= v.x; y -= v.y; return *this; }
  Vector2<T>& operator *=(const Vector2<T> &v) { x *= v.x; y *= v.y; return *this; }
  Vector2<T>& operator *=(T f) { x *= f;   y *= f;   return *this; }
  Vector2<T>& operator /=(const Vector2<T> &v) { x /= v.x; y /= v.y; return *this; }
  Vector2<T>& operator /=(T f) { x /= f;   y /= f;   return *this; }

  // static members
  static Vector2<T> zero() { Vector2<T> r = { T(0), T(0) }; return r; }
  static Vector2<T> one() { Vector2<T> r = { T(1), T(1) }; return r; }

  static Vector2<T> xAxis() { Vector2<T> r = { T(1), T(0) }; return r; }
  static Vector2<T> yAxis() { Vector2<T> r = { T(0), T(1) }; return r; }

  static Vector2<T> create(T _f) { Vector2<T> r = { _f, _f }; return r; }
  static Vector2<T> create(T _x, T _y) { Vector2<T> r = { _x, _y }; return r; }
  template <typename U>
  static Vector2<T> create(const Vector2<U> &_v) { Vector2<T> r = { T(_v.x), T(_v.y) }; return r; }
};
template <typename T>
Vector2<T> operator *(T f, const Vector2<T> &v) { Vector2<T> r = { v.x*f, v.y*f }; return r; }

template <typename T>
struct Vector3
{
  T x, y, z;

  Vector3<T>& toVector2() { return *(Vector2<T>*)this; }
  const Vector3<T>& toVector2() const { return *(Vector2<T>*)this; }

  Vector3<T> operator -() const { Vector3<T> r = { -x, -y, -z }; return r; }

  Vector3<T> operator +(const Vector3<T> &v) const { Vector3<T> r = { x+v.x, y+v.y, z+v.z }; return r; }
  Vector3<T> operator -(const Vector3<T> &v) const { Vector3<T> r = { x-v.x, y-v.y, z-v.z }; return r; }
  Vector3<T> operator *(const Vector3<T> &v) const { Vector3<T> r = { x*v.x, y*v.y, z*v.z }; return r; }
  Vector3<T> operator *(T f) const { Vector3<T> r = { x*f,   y*f,   z*f }; return r; }
  Vector3<T> operator /(const Vector3<T> &v) const { Vector3<T> r = { x/v.x, y/v.y, z/v.z }; return r; }
  Vector3<T> operator /(T f) const { Vector3<T> r = { x/f,   y/f,   z/f }; return r; }

  bool       operator ==(const Vector3<T> &v) const { return x==v.x && y==v.y && z==v.z; }
  bool       operator !=(const Vector3<T> &v) const { return x!=v.x || y!=v.y || z!=v.z; }

  Vector3<T>& operator +=(const Vector3<T> &v) { x += v.x; y += v.y; z += v.z; return *this; }
  Vector3<T>& operator -=(const Vector3<T> &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
  Vector3<T>& operator *=(const Vector3<T> &v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
  Vector3<T>& operator *=(T f) { x *= f;   y *= f;   z *= f;   return *this; }
  Vector3<T>& operator /=(const Vector3<T> &v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
  Vector3<T>& operator /=(T f) { x /= f;   y /= f;   z /= f;   return *this; }

  // static members
  static Vector3<T> zero() { Vector3<T> r = { T(0), T(0), T(0) }; return r; }
  static Vector3<T> one() { Vector3<T> r = { T(1), T(1), T(1) }; return r; }

  static Vector3<T> create(T _f) { Vector3<T> r = { _f, _f, _f };     return r; }
  static Vector3<T> create(T _x, T _y, T _z) { Vector3<T> r = { _x, _y, _z };     return r; }
  static Vector3<T> create(const Vector2<T> _v, T _z) { Vector3<T> r = { _v.x, _v.y, _z }; return r; }
  template <typename U>
  static Vector3<T> create(const Vector3<U> &_v) { Vector3<T> r = { T(_v.x), T(_v.y), T(_v.z) }; return r; }
};
template <typename T>
Vector3<T> operator *(T f, const Vector3<T> &v) { Vector3<T> r = { v.x*f, v.y*f, v.z*f }; return r; }

template <typename T>
struct Vector4
{
  T x, y, z, w;

  Vector3<T>& toVector3() { return *(Vector3<T>*)this; }
  const Vector3<T>& toVector3() const { return *(Vector3<T>*)this; }
  Vector2<T>& toVector2() { return *(Vector2<T>*)this; }
  const Vector2<T>& toVector2() const { return *(Vector2<T>*)this; }

  Vector4<T> operator -() const { Vector4<T> r = { -x, -y, -z, -w }; return r; }

  Vector4<T> operator +(const Vector4<T> &v) const { Vector4<T> r = { x+v.x, y+v.y, z+v.z, w+v.w }; return r; }
  Vector4<T> operator -(const Vector4<T> &v) const { Vector4<T> r = { x-v.x, y-v.y, z-v.z, w-v.w }; return r; }
  Vector4<T> operator *(const Vector4<T> &v) const { Vector4<T> r = { x*v.x, y*v.y, z*v.z, w*v.w }; return r; }
  Vector4<T> operator *(T f) const { Vector4<T> r = { x*f,   y*f,   z*f,   w*f }; return r; }
  Vector4<T> operator /(const Vector4<T> &v) const { Vector4<T> r = { x/v.x, y/v.y, z/v.z, w/v.w }; return r; }
  Vector4<T> operator /(T f) const { Vector4<T> r = { x/f,   y/f,   z/f,   w/f }; return r; }

  bool       operator ==(const Vector4<T> &v) const { return x==v.x && y==v.y && z==v.z && w==v.w; }
  bool       operator !=(const Vector4<T> &v) const { return x!=v.x || y!=v.y || z!=v.z || w!=v.w; }

  Vector4<T>& operator +=(const Vector4<T> &v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
  Vector4<T>& operator -=(const Vector4<T> &v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
  Vector4<T>& operator *=(const Vector4<T> &v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
  Vector4<T>& operator *=(T f) { x *= f;   y *= f;   z *= f;   w *= f;   return *this; }
  Vector4<T>& operator /=(const Vector4<T> &v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
  Vector4<T>& operator /=(T f) { x /= f;   y /= f;   z /= f;   w /= f;   return *this; }

  // static members
  static Vector4<T> zero() { Vector4<T> r = { T(0), T(0), T(0), T(0) }; return r; }
  static Vector4<T> one() { Vector4<T> r = { T(1), T(1), T(1), T(1) }; return r; }
  static Vector4<T> identity() { Vector4<T> r = { T(0), T(0), T(0), T(1) }; return r; }

  static Vector4<T> create(T _f) { Vector4<T> r = { _f,   _f,   _f, _f }; return r; }
  static Vector4<T> create(T _x, T _y, T _z, T _w) { Vector4<T> r = { _x,   _y,   _z, _w }; return r; }
  static Vector4<T> create(const Vector3<T> &_v, T _w) { Vector4<T> r = { _v.x, _v.y, _v.z, _w }; return r; }
  static Vector4<T> create(const Vector2<T> &_v, T _z, T _w) { Vector4<T> r = { _v.x, _v.y,   _z, _w }; return r; }
  template <typename U>
  static Vector4<T> create(const Vector4<U> &_v) { Vector4<T> r = { T(_v.x), T(_v.y), T(_v.z), T(_v.w) }; return r; }
};
template <typename T>
Vector4<T> operator *(T f, const Vector4<T> &v) { Vector4<T> r = { v.x*f, v.y*f, v.z*f, v.w*f }; return r; }

template <typename T>
struct Quaternion
{
  T x, y, z, w;

  Quaternion<T> operator *(const Quaternion<T> &q) const { return Mul(*this, q); }
  Quaternion<T> operator *(T f) const { Quaternion<T> r = { x*f, y*f, z*f, w*f }; return r; }

  Quaternion<T>& operator *=(const Quaternion<T> &q) { *this = mul(*this, q); return *this; }
  Quaternion<T>& operator *=(T f) { x *= f; y *= f; z *= f; w *= f; return *this; }

  Quaternion<T>& inverse();
  Quaternion<T>& conjugate();

  Vector3<T> apply(const Vector3<T> &v);

  Vector3<T> eulerAngles();

  // static members
  static Quaternion<T> identity() { Quaternion<T> r = { T(0), T(0), T(0), T(1) }; return r; }

  static Quaternion<T> create(const Vector3<T> &axis, T rad);
  static Quaternion<T> create(const T _y, const T _p, const T _r);
  static Quaternion<T> create(const Vector3<T> &ypr) { return create(ypr.x, ypr.y, ypr.z); }

  template <typename U>
  static Quaternion<T> create(const Quaternion<U> &_q) { Quaternion<T> r = { T(_q.x), T(_q.y), T(_q.z), T(_q.w) }; return r; }
};
template <typename T>
Quaternion<T> operator *(T f, const Quaternion<T> &q) { Quaternion<T> r = { q.x*f, q.y*f, q.z*f, q.w*f }; return r; }

template <typename T>
struct Matrix4x4
{
  union
  {
    T a[16];
    struct
    {
      Vector4<T> c[4];
    };
    struct
    {
      Vector4<T> x;
      Vector4<T> y;
      Vector4<T> z;
      Vector4<T> t;
    } axis;
    struct
    {
      T // remember, we store columns (axiis) sequentially! (so appears transposed here)
        _00, _10, _20, _30,
        _01, _11, _21, _31,
        _02, _12, _22, _32,
        _03, _13, _23, _33;
    } m;
  };

  Matrix4x4<T> operator *(const Matrix4x4<T> &_m) const { return Mul(*this, _m); }
  Matrix4x4<T> operator *(T f) const { return Mul(*this, f); }
  Vector4<T> operator *(const Vector4<T> &v) const { return Mul(*this, v); }

  Matrix4x4<T>& operator *=(const Matrix4x4<T> &_m) { *this = Mul(*this, _m); return *this; }
  Matrix4x4<T>& operator *=(T f) { *this = Mul(*this, f); return *this; }

  bool operator ==(const Matrix4x4& rh) const { return memcmp(this, &rh, sizeof(*this)) == 0; }

  Matrix4x4<T>& transpose();
  T determinant();
  Matrix4x4<T>& inverse();

  Vector3<T> extractYPR() const;

  // static members
  static Matrix4x4<T> identity();

  static Matrix4x4<T> create(const T m[16]);
  static Matrix4x4<T> create(T _00, T _10, T _20, T _30, T _01, T _11, T _21, T _31, T _02, T _12, T _22, T _32, T _03, T _13, T _23, T _33);
  static Matrix4x4<T> create(const Vector4<T> &xColumn, const Vector4<T> &yColumn, const Vector4<T> &zColumn, const Vector4<T> &wColumn);
  template <typename U>
  static Matrix4x4<T> create(const Matrix4x4<U> &_m);

  static Matrix4x4<T> rotationX(T rad, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> rotationY(T rad, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> rotationZ(T rad, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> rotationAxis(const Vector3<T> &axis, T rad, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> rotationYPR(T y, T p, T r, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> rotationYPR(const Vector3<T> &ypr, const Vector3<T> &t = Vector3<T>::zero()) { return rotationYPR(ypr.x, ypr.y, ypr.z, t); }
  static Matrix4x4<T> rotationQuat(const Quaternion<T> &q, const Vector3<T> &t = Vector3<T>::zero());

  static Matrix4x4<T> translation(T x, T y, T z);
  static Matrix4x4<T> translation(const Vector3<T> &t) { return translation(t.x, t.y, t.z); }

  static Matrix4x4<T> scaleUniform(T s, const Vector3<T> &t = Vector3<T>::zero()) { return scaleNonUniform(s, s, s, t); }
  static Matrix4x4<T> scaleNonUniform(T x, T y, T z, const Vector3<T> &t = Vector3<T>::zero());
  static Matrix4x4<T> scaleNonUniform(const Vector3<T> &s, const Vector3<T> &t = Vector3<T>::zero()) { return scaleNonUniform(s.x, s.y, s.z, t); }

  static Matrix4x4<T> perspective(T fovY, T aspectRatio, T znear, T zfar);
  static Matrix4x4<T> ortho(T left, T right, T bottom, T top, T znear = T(0), T zfar = T(1));
  static Matrix4x4<T> orthoForScreeen(T width, T height, T znear = T(0), T zfar = T(1));

  static Matrix4x4<T> lookAt(const Vector3<T> &from, const Vector3<T> &at, const Vector3<T> &up = Vector3<T>::create(T(0), T(0), T(1)));
};
template <typename T>
Matrix4x4<T> operator *(T f, const Matrix4x4<T> &m) { return m*f; }


// typedef's for typed vectors/matices
typedef Vector4<float>  Float4;
typedef Vector3<float>  Float3;
typedef Vector2<float>  Float2;
typedef Vector4<double> Double4;
typedef Vector3<double> Double3;
typedef Vector2<double> Double2;

typedef Matrix4x4<float>  Float4x4;
typedef Matrix4x4<double> Double4x4;
//typedef Matrix3x4<float>  Float3x4;
//typedef Matrix3x4<double> Double3x4;
//typedef Matrix3x3<float>  Float3x3;
//typedef Matrix3x3<double> Double3x3;

typedef Quaternion<float>  FloatQuat;
typedef Quaternion<double>  DoubleQuat;

} // namespace ep

// unit tests
epResult epMath_Test();

#include "ep/cpp/internal/math_inl.h"

#endif // EPMATH_HPP
