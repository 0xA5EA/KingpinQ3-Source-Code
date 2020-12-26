/*
 * qmathsse.cc
 *
 *  Created on: Aug 19, 2012
 *      Author: area
 */

#include "q_mathsse.h"

#ifdef USING_SSE_MATH
void Mat4_ClearSSEa(matrix_t A)
{
  _mm_store_ps(&A[ 0], _mm_setzero_ps());
  _mm_store_ps(&A[ 4], _mm_setzero_ps());
  _mm_store_ps(&A[ 8], _mm_setzero_ps());
  _mm_store_ps(&A[12], _mm_setzero_ps());
}

void Mat4_ClearSSEua(matrix_t A)
{
  _mm_storeu_ps(&A[ 0], _mm_setzero_ps());
  _mm_storeu_ps(&A[ 4], _mm_setzero_ps());
  _mm_storeu_ps(&A[ 8], _mm_setzero_ps());
  _mm_storeu_ps(&A[12], _mm_setzero_ps());
}

qboolean Mat4_CompareSSEa(matrix_t const  A, matrix_t const  B)
{
  float128 mask =_mm_cmpeq_ps( _mm_load_ps(&A[0]),  _mm_load_ps(&B[0]));
  if (_mm_movemask_ps(mask) != 0x0F) return qfalse;
  mask =_mm_cmpeq_ps( _mm_load_ps(&A[4]),  _mm_load_ps(&B[4]));
  if (_mm_movemask_ps(mask) != 0x0F) return qfalse;
  mask =_mm_cmpeq_ps( _mm_load_ps(&A[8]),  _mm_load_ps(&B[8]));
  if (_mm_movemask_ps(mask) != 0x0F) return qfalse;
  mask =_mm_cmpeq_ps( _mm_load_ps(&A[12]),  _mm_load_ps(&B[12]));
  if (_mm_movemask_ps(mask) != 0x0F) return qfalse;

  return qtrue;
}

qboolean Mat4_CompareSSEua(matrix_t const  A, matrix_t const  B)
{
  float128 mask =_mm_cmpeq_ps(_mm_loadu_ps(&A[0]), _mm_loadu_ps(&B[0]));
  if (_mm_movemask_ps(mask) != 0x0F)
    return qfalse;
  mask =_mm_cmpeq_ps(_mm_loadu_ps(&A[4]), _mm_loadu_ps(&B[4]));
  if (_mm_movemask_ps(mask) != 0x0F)
    return qfalse;
  mask =_mm_cmpeq_ps(_mm_loadu_ps(&A[8]), _mm_loadu_ps(&B[8]));
  if (_mm_movemask_ps(mask) != 0x0F)
    return qfalse;
  mask =_mm_cmpeq_ps(_mm_loadu_ps(&A[12]), _mm_loadu_ps(&B[12]));
  if (_mm_movemask_ps(mask) != 0x0F)
    return qfalse;
  return qtrue;
}

qboolean Mat4_CompEpsilonSSEa(matrix_t const  A, matrix_t const  B, vec_t epsilon)
{
  if (Vec4_AnyDistGtEpsilona( _mm_load_ps(&A[0]), _mm_load_ps(&B[0]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona( _mm_load_ps(&A[4]), _mm_load_ps(&B[4]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona( _mm_load_ps(&A[8]), _mm_load_ps(&B[8]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona( _mm_load_ps(&A[12]), _mm_load_ps(&B[12]), epsilon))
      return qfalse;
  return qtrue;
}

qboolean Mat4_CompEpsilonSSEua(matrix_t const  A, matrix_t const  B, vec_t epsilon)
{
  if (Vec4_AnyDistGtEpsilona(_mm_loadu_ps(&A[0]), _mm_loadu_ps(&B[0]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona(_mm_loadu_ps(&A[4]), _mm_loadu_ps(&B[4]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona(_mm_loadu_ps(&A[8]), _mm_loadu_ps(&B[8]), epsilon))
      return qfalse;
  if (Vec4_AnyDistGtEpsilona(_mm_loadu_ps(&A[12]), _mm_loadu_ps(&B[12]), epsilon))
      return qfalse;
  return qtrue;
}

void Mat4_CopySSEa(matrix_t const s, matrix_t t)
{
  _mm_store_ps(&t[ 0], _mm_load_ps(&s[ 0]));
  _mm_store_ps(&t[ 4], _mm_load_ps(&s[ 4]));
  _mm_store_ps(&t[ 8], _mm_load_ps(&s[ 8]));
  _mm_store_ps(&t[12], _mm_load_ps(&s[12]));
}

void Mat4_CopySSEua(matrix_t const s, matrix_t t)
{
  _mm_storeu_ps(&t[ 0], _mm_loadu_ps(&s[ 0]));
  _mm_storeu_ps(&t[ 4], _mm_loadu_ps(&s[ 4]));
  _mm_storeu_ps(&t[ 8], _mm_loadu_ps(&s[ 8]));
  _mm_storeu_ps(&t[12], _mm_loadu_ps(&s[12]));
}

void Mat4_MultiplySSEa(matrix_t const _restrict a, matrix_t const _restrict b, matrix_t _restrict d)
{
  float128 xmm0, xmm1, xmm2, xmm3;
  float128 xmm4 = _mm_load_ps(&a[0]);
  float128 xmm5 = _mm_load_ps(&a[4]);
  float128 xmm6 = _mm_load_ps(&a[8]);
  float128 xmm7 = _mm_load_ps(&a[12]);
  xmm0 = _mm_load1_ps(&b[0]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[1]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[2]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[3]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_store_ps(&d[0], xmm3);
  xmm0 = _mm_load1_ps(&b[4]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[5]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[6]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[7]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_store_ps(&d[4], xmm3);
  xmm0 = _mm_load1_ps(&b[8]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[9]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[10]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[11]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_store_ps(&d[8], xmm3);
  xmm0 = _mm_load1_ps(&b[12]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[13]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[14]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[15]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_store_ps(&d[12], xmm3);
}


void Mat4_MultiplySSEua(matrix_t const _restrict a, matrix_t const _restrict b, matrix_t _restrict d)
{
  float128 xmm0, xmm1, xmm2, xmm3;
  float128 xmm4 = _mm_loadu_ps(&a[0]);
  float128 xmm5 = _mm_loadu_ps(&a[4]);
  float128 xmm6 = _mm_loadu_ps(&a[8]);
  float128 xmm7 = _mm_loadu_ps(&a[12]);
  xmm0 = _mm_load1_ps(&b[0]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[1]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[2]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[3]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_storeu_ps(&d[0], xmm3);
  xmm0 = _mm_load1_ps(&b[4]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[5]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[6]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[7]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_storeu_ps(&d[4], xmm3);
  xmm0 = _mm_load1_ps(&b[8]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[9]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[10]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[11]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_storeu_ps(&d[8], xmm3);
  xmm0 = _mm_load1_ps(&b[12]);
  xmm0 = _mm_mul_ps(xmm4, xmm0);
  xmm1 = _mm_load1_ps(&b[13]);
  xmm1 = _mm_mul_ps(xmm5, xmm1);
  xmm2 = _mm_load1_ps(&b[14]);
  xmm2 = _mm_mul_ps(xmm6, xmm2);
  xmm3 = _mm_load1_ps(&b[15]);
  xmm3 = _mm_mul_ps(xmm7, xmm3);
  xmm1 = _mm_add_ps(xmm0, xmm1);
  xmm2 = _mm_add_ps(xmm1, xmm2);
  xmm3 = _mm_add_ps(xmm2, xmm3);
  _mm_storeu_ps(&d[12], xmm3);
}
#endif
