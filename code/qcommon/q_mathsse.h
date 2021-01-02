/*
===========================================================================
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler

This file is part of KingpinQ3 source code.

KingpinQ3 source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

KingpinQ3 source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with KingpinQ3 source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#ifndef _MATH_SSE_H_
#define _MATH_SSE_H_
#
#include "q_math.h"

#ifdef USING_SSE_MATH
typedef union
{
  float128 m128;
  float f[4];
  unsigned int ui[4];
}float128_u;

extern force_inline void Vec4_ClearSSE(vec4_t v)
{ _mm_store_ps(&v[0], _mm_setzero_ps()); }

extern force_inline void Vec4_CopySSE(vec4_t const  in, vec4_t out)
{ 	_mm_store_ps(&out[0], _mm_load_ps(&in[0]));  }

extern force_inline void Vec4_AddSSE(vec4_t const  a, vec4_t const  b, vec4_t out)
{ _mm_store_ps(out, _mm_add_ps(_mm_load_ps(&a[0]), _mm_load_ps(&b[0]))); }

extern force_inline void Vec4_SubtractSSE(vec4_t const  a, vec4_t const  b, vec4_t out)
{ _mm_store_ps(out, _mm_sub_ps(_mm_load_ps(&a[0]), _mm_load_ps(&b[0]))); }

extern force_inline qboolean Vec4_CompSSE(vec4_t const  a, vec4_t const  b)
{
  float128 mask = _mm_cmpeq_ps( _mm_load_ps(&a[0]),  _mm_load_ps(&b[0]));
  return (qboolean)(_mm_movemask_ps( mask ) == 0x0F);
}

extern force_inline qboolean Vec4_AnyDistGtEpsilona(float128 const& v1, float128 const& v2, vec_t epsilon)
{
 float128 _c = _mm_and_ps(_mm_sub_ps(v1, v2), _mm_load_ps((float*)&mm_absmask_ps[0]));
 float128 mask = _mm_cmpgt_ps(_c , _mm_set_ps1(epsilon));
 int bits = _mm_movemask_ps( mask );
 return bits;
}

extern force_inline qboolean Vec4_AnyDistGtEpsilonua(vec4_t const v1, vec4_t const  v2, vec_t epsilon)
{
  return Vec4_AnyDistGtEpsilona(_mm_loadu_ps(&v1[0]), _mm_loadu_ps(&v2[0]), epsilon);
}

extern force_inline qboolean Vec4_AnyDistGtEpsilona(vec4_t const v1, vec4_t const  v2, vec_t epsilon)
{
  return Vec4_AnyDistGtEpsilona(_mm_load_ps(&v1[0]), _mm_load_ps(&v2[0]), epsilon);
}

void Mat4_ClearSSEa(matrix_t A);
void Mat4_ClearSSEua(matrix_t A);

extern force_inline void Mat4_ClearSSE(matrix_t A)
{
  if (IsAlligned16(A))
    Mat4_ClearSSEa(A);
  else
    Mat4_ClearSSEua(A);
}

qboolean Mat4_CompareSSEa(matrix_t const  A, matrix_t const  B);
qboolean Mat4_CompareSSEua(matrix_t const  A, matrix_t const  B);

extern force_inline qboolean Mat4_CompareSSE(matrix_t const  A, matrix_t const  B)
{
  if (IsAlligned16(A) && IsAlligned16(B))
    return Mat4_CompareSSEa(A, B);
  else
    return Mat4_CompareSSEua(A, B);
}

qboolean Mat4_CompEpsilonSSEa(matrix_t const  A, matrix_t const  B, vec_t epsilon);
qboolean Mat4_CompEpsilonSSEua(matrix_t const  A, matrix_t const  B, vec_t epsilon);

extern force_inline qboolean Mat4_CompEpsilonSSE(matrix_t const  A, matrix_t const  B, vec_t epsilon)
{
  if (IsAlligned16(A) && IsAlligned16(B))
    return Mat4_CompEpsilonSSEa(A, B, epsilon);
  else
    return Mat4_CompEpsilonSSEua(A, B, epsilon);
}

void Mat4_CopySSEa(matrix_t const s, matrix_t t);
void Mat4_CopySSEua(matrix_t const s, matrix_t t);

extern force_inline void Mat4_CopySSE(matrix_t const s, matrix_t t)
{
  if (IsAlligned16(s) && IsAlligned16(t))
    return Mat4_CopySSEa(s, t);
  else
    return Mat4_CopySSEua(s, t);
}

#if 0
static force_inline int _mm_any_eq(vec4_t const & a, vec4_t const & b)
{
    //test a==b for each float in a & b
    float128 _a = _mm_load_ps(&a[0]);
    float128 _b = _mm_load_ps(&b[0]);
    float128 mask = _mm_cmpeq_ps(_a, _b);

    //copy top bit of each result to maskbits
    int maskBits = _mm_movemask_ps( mask );

    //return maskBits != 0;
    return maskBits;
}
#endif

void Mat4_MultiplySSEa(matrix_t const _restrict a, matrix_t const _restrict b, matrix_t _restrict d);
void Mat4_MultiplySSEua(matrix_t const _restrict a, matrix_t const _restrict b, matrix_t _restrict d);

extern force_inline void Mat4_MultiplySSE(matrix_t const _restrict a, matrix_t const _restrict b, matrix_t _restrict d)
{
  if (IsAlligned16(a) && IsAlligned16(b) && IsAlligned16(d))
    return Mat4_MultiplySSEa(a, b, d);
  else
    return Mat4_MultiplySSEua(a, b, d);
}


//#define Vec4_ClearSSE(v) ( _mm_store_ps(v, _mm_setzero_ps()) )
//ungetestet
//#define Vec4_CopySSE(in, out) (	_mm_store_ps(out, _mm_load_ps(in)) )
//ungetestet

#define Vec4_MASSE(veca, scale, vecb, vecc) { \
  float128          _a, _b, _s, _c; \
  _a = _mm_load_ps(veca); \
  _b = _mm_load_ps(vecb); \
  _s = _mm_set1_ps(scale); \
  _c = _mm_mul_ps(_s, _b); \
  _c = _mm_add_ps(_a, _c); \
  _mm_store_ps(vecc, _c); \
}
//ungetestet
#define Vec4_ScaleSSE(v, s, o) { \
  float128          _in, _scale, _out; \
  _in = _mm_load_ps(in); \
  _scale = _mm_set1_ps(scale); \
  _out = _mm_mul_ps(_in, _scale); \
  _mm_store_ps(out, _out); \
}
//FIXME: nicht getestet, liest doch wahrscheinlich in coordinate 4 muell

#define Vec4_SetSSE(s,d) \
{ \
  float128 XMM0 = _mm_set_ps1((s)); \
  _mm_store_ps((d), XMM0); \
}

#define _horizontal_sum(r) {\
float128 rw = r; \
r = _mm_shuffle_ps(r, rw, _MM_SHUFFLE(1, 0, 3, 2)); \
r = _mm_add_ps(r, rw); \
rw = r; \
r = _mm_shuffle_ps(r, rw, _MM_SHUFFLE(2, 3, 0, 1)); \
r = _mm_add_ps(r, rw); }

#if 0
static force_inline float Q_rsqrt(float number)
{
   float128 half4 = _mm_set_ss(.5f);
   float128 three = _mm_set_ss(3.f);
   float128 val = _mm_set_ss(number);
   float128 y  = _mm_rsqrt_ss(val);
   float128 muls = _mm_mul_ss(_mm_mul_ss(val, y ), y );
  {
    float128_u ff;
    unsigned int ui;
    ff.m128 =_mm_mul_ss(_mm_mul_ss(half4, y), _mm_sub_ss( three, muls) );
    ui = ff.ui[0];
    ui &= 0x7FFFFFFF;
    ui = 0x7F800000 - ui;
    return ((ui & 0x80000000)? .0f : ff.f[0]);
  }
}
#endif

extern force_inline void SEE_Absu(vec4_t v1)
{
  _mm_storeu_ps(&v1[0], _mm_and_ps(_mm_loadu_ps(&v1[0]), _mm_load_ps((float*)&mm_absmask_ps[0])));
}

extern force_inline void SEE_Abs(vec4_t v1)
{
  _mm_store_ps(&v1[0], _mm_and_ps(_mm_load_ps(&v1[0]), _mm_load_ps((float*)&mm_absmask_ps[0])));
}

extern force_inline qboolean Vec3_AnyDistGtEpsilon(vec3_t const  v1, vec3_t const  v2, vec_t epsilon)
{
 float128 _c = _mm_and_ps(_mm_sub_ps(_mm_setr_ps(v1[0], v1[1], v1[2], .0f),
                          _mm_setr_ps(v2[0], v2[1], v2[2], .0f)),
                          _mm_load_ps((float*)&mm_absmask_ps[0]));
 float128 mask = _mm_cmpgt_ps(_c , _mm_set_ps1(epsilon));
 int maskBits = _mm_movemask_ps( mask );
 return maskBits;
}

extern force_inline qboolean Vec3_AllDistLtEpsilon(vec3_t const  v1, vec3_t const  v2, vec_t eps)
{
 float128 _c = _mm_and_ps(_mm_sub_ps(_mm_setr_ps(v1[0], v1[1], v1[2], .0f),
                                 _mm_setr_ps(v2[0], v2[1], v2[2], .0f)),
                      _mm_load_ps((float*)&mm_absmask_ps[0]));
 float128 mask = _mm_cmplt_ps(_c , _mm_set_ps1(eps));
// int xmask = _mm_movemask_ps(mask);
 //printf("mask = %d\n", xmask);
 return (_mm_movemask_ps( mask ) == 0xF ? qtrue : qfalse);

}

/* swizzles for _mm_shuffle_ps instruction */
#define SWZ_XXXX 0x00
#define SWZ_YXXX 0x01
#define SWZ_ZXXX 0x02
#define SWZ_WXXX 0x03
#define SWZ_XYXX 0x04
#define SWZ_YYXX 0x05
#define SWZ_ZYXX 0x06
#define SWZ_WYXX 0x07
#define SWZ_XZXX 0x08
#define SWZ_YZXX 0x09
#define SWZ_ZZXX 0x0a
#define SWZ_WZXX 0x0b
#define SWZ_XWXX 0x0c
#define SWZ_YWXX 0x0d
#define SWZ_ZWXX 0x0e
#define SWZ_WWXX 0x0f
#define SWZ_XXYX 0x10
#define SWZ_YXYX 0x11
#define SWZ_ZXYX 0x12
#define SWZ_WXYX 0x13
#define SWZ_XYYX 0x14
#define SWZ_YYYX 0x15
#define SWZ_ZYYX 0x16
#define SWZ_WYYX 0x17
#define SWZ_XZYX 0x18
#define SWZ_YZYX 0x19
#define SWZ_ZZYX 0x1a
#define SWZ_WZYX 0x1b
#define SWZ_XWYX 0x1c
#define SWZ_YWYX 0x1d
#define SWZ_ZWYX 0x1e
#define SWZ_WWYX 0x1f
#define SWZ_XXZX 0x20
#define SWZ_YXZX 0x21
#define SWZ_ZXZX 0x22
#define SWZ_WXZX 0x23
#define SWZ_XYZX 0x24
#define SWZ_YYZX 0x25
#define SWZ_ZYZX 0x26
#define SWZ_WYZX 0x27
#define SWZ_XZZX 0x28
#define SWZ_YZZX 0x29
#define SWZ_ZZZX 0x2a
#define SWZ_WZZX 0x2b
#define SWZ_XWZX 0x2c
#define SWZ_YWZX 0x2d
#define SWZ_ZWZX 0x2e
#define SWZ_WWZX 0x2f
#define SWZ_XXWX 0x30
#define SWZ_YXWX 0x31
#define SWZ_ZXWX 0x32
#define SWZ_WXWX 0x33
#define SWZ_XYWX 0x34
#define SWZ_YYWX 0x35
#define SWZ_ZYWX 0x36
#define SWZ_WYWX 0x37
#define SWZ_XZWX 0x38
#define SWZ_YZWX 0x39
#define SWZ_ZZWX 0x3a
#define SWZ_WZWX 0x3b
#define SWZ_XWWX 0x3c
#define SWZ_YWWX 0x3d
#define SWZ_ZWWX 0x3e
#define SWZ_WWWX 0x3f
#define SWZ_XXXY 0x40
#define SWZ_YXXY 0x41
#define SWZ_ZXXY 0x42
#define SWZ_WXXY 0x43
#define SWZ_XYXY 0x44
#define SWZ_YYXY 0x45
#define SWZ_ZYXY 0x46
#define SWZ_WYXY 0x47
#define SWZ_XZXY 0x48
#define SWZ_YZXY 0x49
#define SWZ_ZZXY 0x4a
#define SWZ_WZXY 0x4b
#define SWZ_XWXY 0x4c
#define SWZ_YWXY 0x4d
#define SWZ_ZWXY 0x4e
#define SWZ_WWXY 0x4f
#define SWZ_XXYY 0x50
#define SWZ_YXYY 0x51
#define SWZ_ZXYY 0x52
#define SWZ_WXYY 0x53
#define SWZ_XYYY 0x54
#define SWZ_YYYY 0x55
#define SWZ_ZYYY 0x56
#define SWZ_WYYY 0x57
#define SWZ_XZYY 0x58
#define SWZ_YZYY 0x59
#define SWZ_ZZYY 0x5a
#define SWZ_WZYY 0x5b
#define SWZ_XWYY 0x5c
#define SWZ_YWYY 0x5d
#define SWZ_ZWYY 0x5e
#define SWZ_WWYY 0x5f
#define SWZ_XXZY 0x60
#define SWZ_YXZY 0x61
#define SWZ_ZXZY 0x62
#define SWZ_WXZY 0x63
#define SWZ_XYZY 0x64
#define SWZ_YYZY 0x65
#define SWZ_ZYZY 0x66
#define SWZ_WYZY 0x67
#define SWZ_XZZY 0x68
#define SWZ_YZZY 0x69
#define SWZ_ZZZY 0x6a
#define SWZ_WZZY 0x6b
#define SWZ_XWZY 0x6c
#define SWZ_YWZY 0x6d
#define SWZ_ZWZY 0x6e
#define SWZ_WWZY 0x6f
#define SWZ_XXWY 0x70
#define SWZ_YXWY 0x71
#define SWZ_ZXWY 0x72
#define SWZ_WXWY 0x73
#define SWZ_XYWY 0x74
#define SWZ_YYWY 0x75
#define SWZ_ZYWY 0x76
#define SWZ_WYWY 0x77
#define SWZ_XZWY 0x78
#define SWZ_YZWY 0x79
#define SWZ_ZZWY 0x7a
#define SWZ_WZWY 0x7b
#define SWZ_XWWY 0x7c
#define SWZ_YWWY 0x7d
#define SWZ_ZWWY 0x7e
#define SWZ_WWWY 0x7f
#define SWZ_XXXZ 0x80
#define SWZ_YXXZ 0x81
#define SWZ_ZXXZ 0x82
#define SWZ_WXXZ 0x83
#define SWZ_XYXZ 0x84
#define SWZ_YYXZ 0x85
#define SWZ_ZYXZ 0x86
#define SWZ_WYXZ 0x87
#define SWZ_XZXZ 0x88
#define SWZ_YZXZ 0x89
#define SWZ_ZZXZ 0x8a
#define SWZ_WZXZ 0x8b
#define SWZ_XWXZ 0x8c
#define SWZ_YWXZ 0x8d
#define SWZ_ZWXZ 0x8e
#define SWZ_WWXZ 0x8f
#define SWZ_XXYZ 0x90
#define SWZ_YXYZ 0x91
#define SWZ_ZXYZ 0x92
#define SWZ_WXYZ 0x93
#define SWZ_XYYZ 0x94
#define SWZ_YYYZ 0x95
#define SWZ_ZYYZ 0x96
#define SWZ_WYYZ 0x97
#define SWZ_XZYZ 0x98
#define SWZ_YZYZ 0x99
#define SWZ_ZZYZ 0x9a
#define SWZ_WZYZ 0x9b
#define SWZ_XWYZ 0x9c
#define SWZ_YWYZ 0x9d
#define SWZ_ZWYZ 0x9e
#define SWZ_WWYZ 0x9f
#define SWZ_XXZZ 0xa0
#define SWZ_YXZZ 0xa1
#define SWZ_ZXZZ 0xa2
#define SWZ_WXZZ 0xa3
#define SWZ_XYZZ 0xa4
#define SWZ_YYZZ 0xa5
#define SWZ_ZYZZ 0xa6
#define SWZ_WYZZ 0xa7
#define SWZ_XZZZ 0xa8
#define SWZ_YZZZ 0xa9
#define SWZ_ZZZZ 0xaa
#define SWZ_WZZZ 0xab
#define SWZ_XWZZ 0xac
#define SWZ_YWZZ 0xad
#define SWZ_ZWZZ 0xae
#define SWZ_WWZZ 0xaf
#define SWZ_XXWZ 0xb0
#define SWZ_YXWZ 0xb1
#define SWZ_ZXWZ 0xb2
#define SWZ_WXWZ 0xb3
#define SWZ_XYWZ 0xb4
#define SWZ_YYWZ 0xb5
#define SWZ_ZYWZ 0xb6
#define SWZ_WYWZ 0xb7
#define SWZ_XZWZ 0xb8
#define SWZ_YZWZ 0xb9
#define SWZ_ZZWZ 0xba
#define SWZ_WZWZ 0xbb
#define SWZ_XWWZ 0xbc
#define SWZ_YWWZ 0xbd
#define SWZ_ZWWZ 0xbe
#define SWZ_WWWZ 0xbf
#define SWZ_XXXW 0xc0
#define SWZ_YXXW 0xc1
#define SWZ_ZXXW 0xc2
#define SWZ_WXXW 0xc3
#define SWZ_XYXW 0xc4
#define SWZ_YYXW 0xc5
#define SWZ_ZYXW 0xc6
#define SWZ_WYXW 0xc7
#define SWZ_XZXW 0xc8
#define SWZ_YZXW 0xc9
#define SWZ_ZZXW 0xca
#define SWZ_WZXW 0xcb
#define SWZ_XWXW 0xcc
#define SWZ_YWXW 0xcd
#define SWZ_ZWXW 0xce
#define SWZ_WWXW 0xcf
#define SWZ_XXYW 0xd0
#define SWZ_YXYW 0xd1
#define SWZ_ZXYW 0xd2
#define SWZ_WXYW 0xd3
#define SWZ_XYYW 0xd4
#define SWZ_YYYW 0xd5
#define SWZ_ZYYW 0xd6
#define SWZ_WYYW 0xd7
#define SWZ_XZYW 0xd8
#define SWZ_YZYW 0xd9
#define SWZ_ZZYW 0xda
#define SWZ_WZYW 0xdb
#define SWZ_XWYW 0xdc
#define SWZ_YWYW 0xdd
#define SWZ_ZWYW 0xde
#define SWZ_WWYW 0xdf
#define SWZ_XXZW 0xe0
#define SWZ_YXZW 0xe1
#define SWZ_ZXZW 0xe2
#define SWZ_WXZW 0xe3
#define SWZ_XYZW 0xe4
#define SWZ_YYZW 0xe5
#define SWZ_ZYZW 0xe6
#define SWZ_WYZW 0xe7
#define SWZ_XZZW 0xe8
#define SWZ_YZZW 0xe9
#define SWZ_ZZZW 0xea
#define SWZ_WZZW 0xeb
#define SWZ_XWZW 0xec
#define SWZ_YWZW 0xed
#define SWZ_ZWZW 0xee
#define SWZ_WWZW 0xef
#define SWZ_XXWW 0xf0
#define SWZ_YXWW 0xf1
#define SWZ_ZXWW 0xf2
#define SWZ_WXWW 0xf3
#define SWZ_XYWW 0xf4
#define SWZ_YYWW 0xf5
#define SWZ_ZYWW 0xf6
#define SWZ_WYWW 0xf7
#define SWZ_XZWW 0xf8
#define SWZ_YZWW 0xf9
#define SWZ_ZZWW 0xfa
#define SWZ_WZWW 0xfb
#define SWZ_XWWW 0xfc
#define SWZ_YWWW 0xfd
#define SWZ_ZWWW 0xfe
#define SWZ_WWWW 0xff
#define sseSwizzle( a, mask ) _mm_shuffle_ps( (a), (a), SWZ_##mask )

static ID_INLINE __m128 unitQuat() {
  return _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f); // order is reversed
}
static ID_INLINE __m128 sseLoadInts(const int vec[4]) {
  return *(__m128 *)vec;
}
static ID_INLINE __m128 mask_0000() {
  static const ALIGN16( int vec[4]) = { 0, 0, 0, 0 };
  return sseLoadInts(vec);
}
static ID_INLINE __m128 mask_000W() {
  static const ALIGN16( int vec[4]) = { 0, 0, 0, -1 };
  return sseLoadInts(vec);
}
static ID_INLINE __m128 mask_XYZ0() {
  static const ALIGN16( int vec[4]) = { -1, -1, -1, 0 };
  return sseLoadInts(vec);
}

static ID_INLINE __m128 sign_000W() {
  static const ALIGN16( int vec[4]) = { 0, 0, 0, 1 << 31 };
  return sseLoadInts(vec);
}
static ID_INLINE __m128 sign_XYZ0() {
  static const ALIGN16( int vec[4]) = { 1 << 31, 1 << 31, 1 << 31, 0 };
  return sseLoadInts(vec);
}
static ID_INLINE __m128 sign_XYZW() {
  static const ALIGN16( int vec[4]) = { 1 << 31, 1 << 31, 1 << 31, 1 << 31 };
  return sseLoadInts(vec);
}

static ID_INLINE __m128 sseDot4(__m128 a, __m128 b) {
  __m128 prod = _mm_mul_ps(a, b);
  __m128 sum1 = _mm_add_ps(prod, sseSwizzle(prod, YXWZ));
  __m128 sum2 = _mm_add_ps(sum1, sseSwizzle(sum1, ZWXY));
  return sum2;
}
static ID_INLINE __m128 sseCrossProduct(__m128 a, __m128 b) {
  __m128 a_yzx = sseSwizzle(a, YZXW);
  __m128 b_yzx = sseSwizzle(b, YZXW);
  __m128 c_zxy = _mm_sub_ps(_mm_mul_ps(a, b_yzx),
    _mm_mul_ps(a_yzx, b));
  return sseSwizzle(c_zxy, YZXW);
}
static ID_INLINE __m128 sseQuatMul(__m128 a, __m128 b) {
  __m128 a1 = sseSwizzle(a, WWWW);
  __m128 c1 = _mm_mul_ps(a1, b);
  __m128 a2 = sseSwizzle(a, XYZX);
  __m128 b2 = sseSwizzle(b, WWWX);
  __m128 c2 = _mm_xor_ps(_mm_mul_ps(a2, b2), sign_000W());
  __m128 a3 = sseSwizzle(a, YZXY);
  __m128 b3 = sseSwizzle(b, ZXYY);
  __m128 c3 = _mm_xor_ps(_mm_mul_ps(a3, b3), sign_000W());
  __m128 a4 = sseSwizzle(a, ZXYZ);
  __m128 b4 = sseSwizzle(b, YZXZ);
  __m128 c4 = _mm_mul_ps(a4, b4);
  return _mm_add_ps(_mm_add_ps(c1, c2), _mm_sub_ps(c3, c4));
}
#define RSQRT_PRECISE
static ID_INLINE __m128 sseQuatNormalize(__m128 q) {
  __m128 p = _mm_mul_ps(q, q);
  __m128 t, h;
  p = _mm_add_ps(sseSwizzle(p, XXZZ),
    sseSwizzle(p, YYWW));
  p = _mm_add_ps(sseSwizzle(p, XXXX),
    sseSwizzle(p, ZZZZ));
  t = _mm_rsqrt_ps(p);
#ifdef RSQRT_PRECISE
  h = _mm_mul_ps(_mm_set1_ps(0.5f), t);
  t = _mm_mul_ps(_mm_mul_ps(t, t), p);
  t = _mm_sub_ps(_mm_set1_ps(3.0f), t);
  t = _mm_mul_ps(h, t);
#endif
  return _mm_mul_ps(q, t);
}
static ID_INLINE __m128 sseQuatTransform(__m128 q, __m128 vec) {
  __m128 t, t2;
  t = sseCrossProduct(q, vec);
  t = _mm_add_ps(t, t);
  t2 = sseCrossProduct(q, t);
  t = _mm_mul_ps(sseSwizzle(q, WWWW), t);
  return _mm_add_ps(_mm_add_ps(vec, t2), t);
}
static ID_INLINE __m128 sseQuatTransformInverse(__m128 q, __m128 vec) {
  __m128 t, t2;
  t = sseCrossProduct(vec, q);
  t = _mm_add_ps(t, t);
  t2 = sseCrossProduct(t, q);
  t = _mm_mul_ps(sseSwizzle(q, WWWW), t);
  return _mm_add_ps(_mm_add_ps(vec, t2), t);
}
static ID_INLINE void sseStoreVec3(__m128 in, vec3_t out) {
  __m128 old = _mm_loadu_ps(out);
  old = _mm_or_ps(_mm_and_ps(in, mask_XYZ0()),
    _mm_and_ps(old, mask_000W()));
  _mm_storeu_ps(out, old);
}
static ID_INLINE void TransInit(transform_t *t) {
  __m128 u = unitQuat();
  t->sseRot = u;
  t->sseTransScale = u;
}
static ID_INLINE void TransCopy(const transform_t *in, transform_t *out) {
  out->sseRot = in->sseRot;
  out->sseTransScale = in->sseTransScale;
}
static ID_INLINE void TransformPoint(const transform_t *t,
  const vec3_t in, vec3_t out) {
  __m128 ts = t->sseTransScale;
  __m128 tmp = sseQuatTransform(t->sseRot, _mm_loadu_ps(in));
  tmp = _mm_mul_ps(tmp, sseSwizzle(ts, WWWW));
  tmp = _mm_add_ps(tmp, ts);
  sseStoreVec3(tmp, out);
}
static ID_INLINE void TransformPointInverse(const transform_t *t,
  const vec3_t in, vec3_t out) {
  __m128 ts = t->sseTransScale;
  __m128 v = _mm_sub_ps(_mm_loadu_ps(in), ts);
  v = _mm_mul_ps(v, _mm_rcp_ps(sseSwizzle(ts, WWWW)));
  v = sseQuatTransformInverse(t->sseRot, v);
  sseStoreVec3(v, out);
}
static ID_INLINE void TransformNormalVector(const transform_t *t,
  const vec3_t in, vec3_t out) {
  __m128 v = _mm_loadu_ps(in);
  v = sseQuatTransform(t->sseRot, v);
  sseStoreVec3(v, out);
}
static ID_INLINE void TransformNormalVectorInverse(const transform_t *t,
  const vec3_t in, vec3_t out) {
  __m128 v = _mm_loadu_ps(in);
  v = sseQuatTransformInverse(t->sseRot, v);
  sseStoreVec3(v, out);
}
static ID_INLINE __m128 sseAxisAngleToQuat(const vec3_t axis, float angle) {
  __m128 sa = _mm_set1_ps((float)sin(0.5f * angle));
  __m128 ca = _mm_set1_ps((float)cos(0.5f * angle));
  __m128 a = _mm_loadu_ps(axis);
  a = _mm_and_ps(a, mask_XYZ0());
  a = _mm_mul_ps(a, sa);
  return _mm_or_ps(a, _mm_and_ps(ca, mask_000W()));
}
static ID_INLINE void TransInitRotationQuat(const quat_t quat, transform_t *t) {
  t->sseRot = _mm_loadu_ps(quat);
  t->sseTransScale = unitQuat();
}
static ID_INLINE void TransInitRotation(const vec3_t axis, float angle,  transform_t *t) {
  t->sseRot = sseAxisAngleToQuat(axis, angle);
  t->sseTransScale = unitQuat();
}
static ID_INLINE void TransInitTranslation(const vec3_t vec, transform_t *t) {
  __m128 v = _mm_loadu_ps(vec);
  v = _mm_and_ps(v, mask_XYZ0());
  t->sseRot = unitQuat();
  t->sseTransScale = _mm_or_ps(v, unitQuat());
}
static ID_INLINE void TransInitScale(float factor, transform_t *t) {
  __m128 f = _mm_set1_ps(factor);
  f = _mm_and_ps(f, mask_000W());
  t->sseRot = unitQuat();
  t->sseTransScale = f;
}
static ID_INLINE void TransInsRotationQuat(const quat_t quat, transform_t *t) {
  __m128 q = _mm_loadu_ps(quat);
  t->sseRot = sseQuatMul(t->sseRot, q);
}
static ID_INLINE void TransInsRotation(const vec3_t axis, float angle,
  transform_t *t) {
  __m128 q = sseAxisAngleToQuat(axis, angle);
  t->sseRot = sseQuatMul(q, t->sseRot);
}
static ID_INLINE void TransAddRotationQuat(const quat_t quat, transform_t *t) {
  __m128 q = _mm_loadu_ps(quat);
  __m128 transformed = sseQuatTransform(q, t->sseTransScale);
  t->sseRot = sseQuatMul(q, t->sseRot);
  t->sseTransScale = _mm_or_ps(_mm_and_ps(transformed, mask_XYZ0()),
    _mm_and_ps(t->sseTransScale, mask_000W()));
}
static ID_INLINE void TransAddRotation(const vec3_t axis, float angle,
  transform_t *t) {
  __m128 q = sseAxisAngleToQuat(axis, angle);
  __m128 transformed = sseQuatTransform(q, t->sseTransScale);
  t->sseRot = sseQuatMul(t->sseRot, q);
  t->sseTransScale = _mm_or_ps(_mm_and_ps(transformed, mask_XYZ0()),
    _mm_and_ps(t->sseTransScale, mask_000W()));
}
static ID_INLINE void TransInsScale(float factor, transform_t *t) {
  t->scale *= factor;
}
static ID_INLINE void TransAddScale(float factor, transform_t *t) {
  __m128 f = _mm_set1_ps(factor);
  t->sseTransScale = _mm_mul_ps(f, t->sseTransScale);
}
static ID_INLINE void TransInsTranslation(const vec3_t vec,
  transform_t *t) {
  __m128 v = _mm_loadu_ps(vec);
  __m128 ts = t->sseTransScale;
  v = sseQuatTransform(t->sseRot, v);
  v = _mm_mul_ps(v, sseSwizzle(ts, WWWW));
  v = _mm_and_ps(v, mask_XYZ0());
  t->sseTransScale = _mm_add_ps(ts, v);
}
static ID_INLINE void TransAddTranslation(const vec3_t vec,
  transform_t *t) {
  __m128 v = _mm_loadu_ps(vec);
  v = _mm_and_ps(v, mask_XYZ0());
  t->sseTransScale = _mm_add_ps(t->sseTransScale, v);
}
static ID_INLINE void TransCombine(const transform_t *a,
  const transform_t *b,
  transform_t *out) {
  __m128 aRot = a->sseRot;
  __m128 aTS = a->sseTransScale;
  __m128 bRot = b->sseRot;
  __m128 bTS = b->sseTransScale;
  __m128 tmp = sseQuatTransform(bRot, aTS);
  tmp = _mm_or_ps(_mm_and_ps(tmp, mask_XYZ0()),
    _mm_and_ps(aTS, mask_000W()));
  tmp = _mm_mul_ps(tmp, sseSwizzle(bTS, WWWW));
  out->sseTransScale = _mm_add_ps(tmp, _mm_and_ps(bTS, mask_XYZ0()));
  out->sseRot = sseQuatMul(bRot, aRot);
}
static ID_INLINE void TransInverse(const transform_t *in,
  transform_t *out) {
  __m128 rot = in->sseRot;
  __m128 ts = in->sseTransScale;
  __m128 invS = _mm_rcp_ps(sseSwizzle(ts, WWWW));
  __m128 invRot = _mm_xor_ps(rot, sign_XYZ0());
  __m128 invT = _mm_xor_ps(ts, sign_XYZ0());
  __m128 tmp = sseQuatTransform(invRot, invT);
  tmp = _mm_mul_ps(tmp, invS);
  out->sseRot = invRot;
  out->sseTransScale = _mm_or_ps(_mm_and_ps(tmp, mask_XYZ0()),
    _mm_and_ps(invS, mask_000W()));
}
static ID_INLINE void TransStartLerp(transform_t *t) {
  t->sseRot = mask_0000();
  t->sseTransScale = mask_0000();
}
static ID_INLINE void TransAddWeight(float weight, const transform_t *a,
  transform_t *out) {
  __m128 w = _mm_set1_ps(weight);
  __m128 d = sseDot4(a->sseRot, out->sseRot);
  out->sseTransScale = _mm_add_ps(out->sseTransScale,
    _mm_mul_ps(w, a->sseTransScale));
  w = _mm_xor_ps(w, _mm_and_ps(d, sign_XYZW()));
  out->sseRot = _mm_add_ps(out->sseRot,
    _mm_mul_ps(w, a->sseRot));
}
static ID_INLINE void TransEndLerp(transform_t *t) {
  t->sseRot = sseQuatNormalize(t->sseRot);
}


#if 0
//FIXME (0xA5EA):  function returns nan if v = 0 0 0
static force_inline float128 _mm_rsqrt_(float128 const & v)
{
  const float128 approx = _mm_rsqrt_ps( v);
  return _mm_mul_ps(_mm_mul_ps(_mm_set_ps1(.5f), approx), _mm_sub_ps(  _mm_set_ps1(3.f), _mm_mul_ps(_mm_mul_ps(v, approx), approx)) );
}

extern void force_inline  _mm_rsqrt_2( float128 const & v, float128& tgt)  throw ()
{
  if (Vec4_AllDistLtEpsilon(v, zeros4, 10e-20f))
  {
    tgt = _mm_load_ps(zeros4);
  }
  else
  {
    const float128 approx = _mm_rsqrt_ps( v);
    tgt = _mm_mul_ps(_mm_mul_ps(_mm_set_ps1(.5f), approx), _mm_sub_ps(  _mm_set_ps1(3.f), _mm_mul_ps(_mm_mul_ps(v, approx), approx)) );
  }
}

static force_inline void VectorNormalizeSelfSSE( vec3_t v )
{
  vec4_t ALIGN16(v4);
  float128 rsq;
  float128 _a = _mm_setr_ps(v[0], v[1], v[2], .0f);
  float128 dot = _mm_mul_ps(_a, _a);
  _horizontal_sum(dot);
  rsq = _mm_rsqrt_( dot );
  rsq = _mm_mul_ps(_a, rsq);
  _mm_store_ps(v4, rsq);
  v [0] = v4[0]; v [1] = v4[1]; v[2] = v4[2];
}

static force_inline vec_t VectorNormalizeSelf2SSE(vec3_t v)
{
  vec4_t ALIGN16(v4);
  float length;
  float128 recip;
  float128 _a = _mm_setr_ps(v[0], v[1], v[2], .0f);
  float128 dot = _mm_mul_ps(_a, _a);
  float128 rsq;
  _horizontal_sum(dot);
  rsq =_mm_rsqrt_(dot);
  length = (*(float*)&dot) * (*(float*)&rsq);
  if(length != .0f)
  {
    recip = _mm_mul_ps(_a, rsq);
     _mm_store_ps(v4, recip);
     v[0] = v4[0]; v[1] = v4[1];v[2] = v4[2];
  }
  else
  {
    VectorClear( v );
  }
  return length;
}

static force_inline vec_t VectorNormalize2SSE(vec3_t const v, vec3_t out)
{
  vec4_t ALIGN16(v4);
  float length;
  float128 recip, rsq;
  float128 _a = _mm_setr_ps(v[0], v[1], v[2], .0f);
  float128 dot = _mm_mul_ps(_a, _a);
  _horizontal_sum(dot);
   rsq =_mm_rsqrt_(dot);
   length = (*(float*)&dot) * (*(float*)&rsq);
   if(length != .0f)
  {
    recip = _mm_mul_ps(_a, rsq);
     _mm_store_ps(v4, recip);
     out[0] = v4[0]; out[1] = v4[1];out[2] = v4[2];
  }
  else
  {
    //FIXME (0xA5EA): was wenn v = out (selbe pointer
    VectorClear( out );
  }
  return length;
}
#endif
#endif // (USE_SSE) && (defined (__SSE__) || defined (__SSE2__))
#endif //_MATH_SSE_H_
