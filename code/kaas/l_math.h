/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#ifndef __MATHLIB__
#define __MATHLIB__

// mathlib.h

#include <math.h>

#define USE_Q_SHARED
#ifdef USE_Q_SHARED
#include "../qcommon/q_platform.h" //hypov8 re-enable
#include "../qcommon/q_shared.h" //hypov8 re-enable
#include "../qcommon/q_math.h"

#define BSPC_SIDE_FRONT   0
#define BSPC_SIDE_ON      2
#define BSPC_SIDE_BACK    1
#define BSPC_SIDE_CROSS   3 //-2

#define PITCH       0
#define YAW         1
#define ROLL        2

#ifndef Q_PI
#define Q_PI  3.14159265358979323846
#endif
STATIC_INLINE vec_t Q_rint (vec_t in) //static inline
{
  return floor(in + 0.5);
}


#define LEQUAL_EPSILON 0.001
STATIC_INLINE qboolean lVectorCompare (vec3_t v1, vec3_t v2) //static inline
{
  int   i;

  for (i=0 ; i<3 ; i++)
    if (fabs(v1[i]-v2[i]) > LEQUAL_EPSILON)
      return qfalse;

  return qtrue;
}

void RotatePoint(vec3_t point, float matrix[3][3]); //hypov8
//void CreateRotationMatrix(const vec3_t angles, vec3_t matrix[3]);
//void CreateRotationMatrix(vec3_t angles, float matrix[3][3]); //hypov8
#else

#if 0
// 0xA5EA
#ifdef DOUBLEVEC_T
typedef double vec_t;
#else
typedef float vec_t;
#endif
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
#endif
#if 1	// 0xA5EA
#define	BSPC_SIDE_FRONT		0
#define	BSPC_SIDE_ON			2
#define	BSPC_SIDE_BACK		1
#define	BSPC_SIDE_CROSS		3 //-2
#endif

#define PITCH		0
#define YAW			1
#define ROLL		2

#define	Q_PI	3.14159265358979323846

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

extern vec3_t vec3_origin;
extern vec3_t vec3_up;

#define	EQUAL_EPSILON	0.001

qboolean lVectorCompare (vec3_t v1, vec3_t v2);

#define DotProduct(x,y)			(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define VectorSubtract(a,b,c)	{c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
#define VectorAdd(a,b,c)		{c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#ifndef VectorCopy
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#endif
#define Vector4Copy(a,b)		{b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];}
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
//#define VectorClear(x)			{x[0] = x[1] = x[2] = 0;}
#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(x, y)		{y[0]=-x[0];y[1]=-x[1];y[2]=-x[2];}
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))

vec_t Q_rint (vec_t in);
//vec_t _DotProduct (vec3_t v1, vec3_t v2);
//void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
//void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
//void _VectorCopy (vec3_t in, vec3_t out);
//void _VectorScale (vec3_t v, vec_t scale, vec3_t out);
//void _VectorMA(vec3_t va, double scale, vec3_t vb, vec3_t vc);

double Vector_Length(vec3_t v);
void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t VectorNormalize(vec3_t inout);
vec_t ColorNormalize(vec3_t in, vec3_t out);
vec_t VectorNormalize2(const vec3_t v, vec3_t out);


void ClearBounds (vec3_t mins, vec3_t maxs);


void VectorInverse (vec3_t v);
void AddPointToBounds (const vec3_t v, vec3_t mins, vec3_t maxs);
#if 0
// 0xA5EA: this is now a macro in q_shared.h FIXME: check if bsp version is same as engine version

#endif
void AngleVectors (const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3]);
void RotatePoint(vec3_t point, float matrix[3][3]);
//void CreateRotationMatrix(vec3_t angles, float matrix[3][3]);
#endif // USE_QSHARED
#endif
