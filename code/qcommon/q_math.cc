/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-20xy Lars '0xA5EA' Kandler
Copyright (C) 2011-2012 Ari 'KRYPTYK' Mirles

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
//
// q_math.c -- stateless support routines that are included in each code module

// Some of the vector functions are static inline in q_shared.h. q3asm
// doesn't understand static functions though, so we only want them in
// one file. That's what this is about.

#ifdef Q3_VM
#define __Q3_VM_MATH
#define force_inline ID_INLINE
#endif

#ifdef Q3_VM
#define sqrtf sqrt
#endif

#include "q_shared.h"
//#include <cfloat>

#ifdef BSPC
#include <math.h>
#endif

vec3_t vec3_origin = {0, 0, 0};
vec3_t vec3_up = {0, 0, 1};
vec3_t   axisDefault[ 3 ] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
matrix_t matrixIdentity = {     1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1
                          };
quat_t   quatIdentity = { 0, 0, 0, 1 };


#if 0 //ndef NO_DEVELOPER_CMDS
//vec_t **I4x4;
const matrix_t ALIGN16(Hilbert4) =
{
  1.f,     1.f/2.f, 1.f/3.f, 1.f/4.f,
  1.f/2.f, 1.f/3.f, 1.f/4.f, 1.f/5.f,
  1.f/3.f, 1.f/4.f, 1.f/5.f, 1.f/6.f,
  1.f/4.f, 1.f/5.f, 1.f/6.f, 1.f/7.f
};
const matrix_t ALIGN16(ASymetric4) =
{
  5.421681495984457f, 2.429831063577000f, 0.191850872377119f, 3.955013293040301f,
  0.926318880309393f, 4.527109636465218f, 3.360293084127580f, 2.851749608619593f,
  0.171047680971528f, 2.233699693410323f, 3.982594705078037f, 4.299861890859538f,
  4.901325698004463f, 2.838426238603861f, 3.189206114476794f, 4.430513347547178f
};
#endif

vec4_t		colorBlack = {0, 0, 0, 1};
vec4_t		colorRed = {1, 0, 0, 1};
vec4_t		colorGreen = {0, 1, 0, 1};
vec4_t		colorBlue = {0, 0, 1, 1};
vec4_t		colorYellow = {1, 1, 0, 1};
vec4_t		colorMagenta = {1, 0, 1, 1};
vec4_t		colorCyan = {0, 1, 1, 1};
vec4_t		colorWhite = {1, 1, 1, 1};
vec4_t		colorLtGrey = {0.75, 0.75, 0.75, 1};
vec4_t		colorMdGrey = {0.5, 0.5, 0.5, 1};
vec4_t		colorDkGrey = {0.25, 0.25, 0.25, 1};
vec4_t		colortmbBrown =  {0.7f,  0.5f,  0.2f, 1.f};		/* 0xA5EA */
vec4_t		colortmBlue	= {0.5f, 0.62f, 0.71f, 1.f};		/* 0xA5EA */
vec4_t		colortmGreen	= {103.f/255.f, 103.f/255.f, 52.f/255.f, 1.f};
vec4_t		colortmgreenyellow = {154.f/255.f, 154.f/255.f, 77.f/255.f, 1.00f};
vec4_t		colortmRed	= {6.0f/10.0f, 2.0f/10.0f, 2.0f/10.0f, 1};

vec4_t		colorDkGreenConsole = {0.1f, 0.135f, 0.1f,1 }; //console background


const vec4_t g_color_table[8] =
{
  {0.00000f, 0.00000f, 0.00000f, 1.00000f},	// 0 standard colors
  {1.00000f, 0.00000f, 0.00000f, 1.00000f},	// 1 |
  {0.00000f, 1.00000f, 0.00000f, 1.00000f},	// 2 v
  {1.00000f, 1.00000f, 0.00000f, 1.00000f},	// 3
  {0.00000f, 0.00000f, 1.00000f, 1.00000f},	// 4
  {0.00000f, 1.00000f, 1.00000f, 1.00000f},	// 5
  {1.00000f, 0.00000f, 1.00000f, 1.00000f},	// 6 ^
  {1.00000f, 1.00000f, 1.00000f, 1.00000f},	// 7 |
};

const vec3_t	bytedirs[NUMVERTEXNORMALS] =
{
  {-0.525731f, 0.000000f, 0.850651f}, {-0.442863f, 0.238856f, 0.864188f},
  {-0.295242f, 0.000000f, 0.955423f}, {-0.309017f, 0.500000f, 0.809017f},
  {-0.162460f, 0.262866f, 0.951056f}, {0.000000f, 0.000000f, 1.000000f},
  {0.000000f, 0.850651f, 0.525731f}, {-0.147621f, 0.716567f, 0.681718f},
  {0.147621f, 0.716567f, 0.681718f}, {0.000000f, 0.525731f, 0.850651f},
  {0.309017f, 0.500000f, 0.809017f}, {0.525731f, 0.000000f, 0.850651f},
  {0.295242f, 0.000000f, 0.955423f}, {0.442863f, 0.238856f, 0.864188f},
  {0.162460f, 0.262866f, 0.951056f}, {-0.681718f, 0.147621f, 0.716567f},
  {-0.809017f, 0.309017f, 0.500000f},{-0.587785f, 0.425325f, 0.688191f},
  {-0.850651f, 0.525731f, 0.000000f},{-0.864188f, 0.442863f, 0.238856f},
  {-0.716567f, 0.681718f, 0.147621f},{-0.688191f, 0.587785f, 0.425325f},
  {-0.500000f, 0.809017f, 0.309017f}, {-0.238856f, 0.864188f, 0.442863f},
  {-0.425325f, 0.688191f, 0.587785f}, {-0.716567f, 0.681718f, -0.147621f},
  {-0.500000f, 0.809017f, -0.309017f}, {-0.525731f, 0.850651f, 0.000000f},
  {0.000000f, 0.850651f, -0.525731f}, {-0.238856f, 0.864188f, -0.442863f},
  {0.000000f, 0.955423f, -0.295242f}, {-0.262866f, 0.951056f, -0.162460f},
  {0.000000f, 1.000000f, 0.000000f}, {0.000000f, 0.955423f, 0.295242f},
  {-0.262866f, 0.951056f, 0.162460f}, {0.238856f, 0.864188f, 0.442863f},
  {0.262866f, 0.951056f, 0.162460f}, {0.500000f, 0.809017f, 0.309017f},
  {0.238856f, 0.864188f, -0.442863f},{0.262866f, 0.951056f, -0.162460f},
  {0.500000f, 0.809017f, -0.309017f},{0.850651f, 0.525731f, 0.000000f},
  {0.716567f, 0.681718f, 0.147621f}, {0.716567f, 0.681718f, -0.147621f},
  {0.525731f, 0.850651f, 0.000000f}, {0.425325f, 0.688191f, 0.587785f},
  {0.864188f, 0.442863f, 0.238856f}, {0.688191f, 0.587785f, 0.425325f},
  {0.809017f, 0.309017f, 0.500000f}, {0.681718f, 0.147621f, 0.716567f},
  {0.587785f, 0.425325f, 0.688191f}, {0.955423f, 0.295242f, 0.000000f},
  {1.000000f, 0.000000f, 0.000000f}, {0.951056f, 0.162460f, 0.262866f},
  {0.850651f, -0.525731f, 0.000000f},{0.955423f, -0.295242f, 0.000000f},
  {0.864188f, -0.442863f, 0.238856f}, {0.951056f, -0.162460f, 0.262866f},
  {0.809017f, -0.309017f, 0.500000f}, {0.681718f, -0.147621f, 0.716567f},
  {0.850651f, 0.000000f, 0.525731f}, {0.864188f, 0.442863f, -0.238856f},
  {0.809017f, 0.309017f, -0.500000f}, {0.951056f, 0.162460f, -0.262866f},
  {0.525731f, 0.000000f, -0.850651f}, {0.681718f, 0.147621f, -0.716567f},
  {0.681718f, -0.147621f, -0.716567f},{0.850651f, 0.000000f, -0.525731f},
  {0.809017f, -0.309017f, -0.500000f}, {0.864188f, -0.442863f, -0.238856f},
  {0.951056f, -0.162460f, -0.262866f}, {0.147621f, 0.716567f, -0.681718f},
  {0.309017f, 0.500000f, -0.809017f}, {0.425325f, 0.688191f, -0.587785f},
  {0.442863f, 0.238856f, -0.864188f}, {0.587785f, 0.425325f, -0.688191f},
  {0.688191f, 0.587785f, -0.425325f}, {-0.147621f, 0.716567f, -0.681718f},
  {-0.309017f, 0.500000f, -0.809017f}, {0.000000f, 0.525731f, -0.850651f},
  {-0.525731f, 0.000000f, -0.850651f}, {-0.442863f, 0.238856f, -0.864188f},
  {-0.295242f, 0.000000f, -0.955423f}, {-0.162460f, 0.262866f, -0.951056f},
  {0.000000f, 0.000000f, -1.000000f}, {0.295242f, 0.000000f, -0.955423f},
  {0.162460f, 0.262866f, -0.951056f}, {-0.442863f, -0.238856f, -0.864188f},
  {-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f},
  {0.000000f, -0.850651f, -0.525731f}, {-0.147621f, -0.716567f, -0.681718f},
  {0.147621f, -0.716567f, -0.681718f}, {0.000000f, -0.525731f, -0.850651f},
  {0.309017f, -0.500000f, -0.809017f}, {0.442863f, -0.238856f, -0.864188f},
  {0.162460f, -0.262866f, -0.951056f}, {0.238856f, -0.864188f, -0.442863f},
  {0.500000f, -0.809017f, -0.309017f}, {0.425325f, -0.688191f, -0.587785f},
  {0.716567f, -0.681718f, -0.147621f}, {0.688191f, -0.587785f, -0.425325f},
  {0.587785f, -0.425325f, -0.688191f}, {0.000000f, -0.955423f, -0.295242f},
  {0.000000f, -1.000000f, 0.000000f}, {0.262866f, -0.951056f, -0.162460f},
  {0.000000f, -0.850651f, 0.525731f}, {0.000000f, -0.955423f, 0.295242f},
  {0.238856f, -0.864188f, 0.442863f}, {0.262866f, -0.951056f, 0.162460f},
  {0.500000f, -0.809017f, 0.309017f}, {0.716567f, -0.681718f, 0.147621f},
  {0.525731f, -0.850651f, 0.000000f}, {-0.238856f, -0.864188f, -0.442863f},
  {-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f},
  {-0.850651f, -0.525731f, 0.000000f}, {-0.716567f, -0.681718f, -0.147621f},
  {-0.716567f, -0.681718f, 0.147621f}, {-0.525731f, -0.850651f, 0.000000f},
  {-0.500000f, -0.809017f, 0.309017f}, {-0.238856f, -0.864188f, 0.442863f},
  {-0.262866f, -0.951056f, 0.162460f}, {-0.864188f, -0.442863f, 0.238856f},
  {-0.809017f, -0.309017f, 0.500000f}, {-0.688191f, -0.587785f, 0.425325f},
  {-0.681718f, -0.147621f, 0.716567f}, {-0.442863f, -0.238856f, 0.864188f},
  {-0.587785f, -0.425325f, 0.688191f}, {-0.309017f, -0.500000f, 0.809017f},
  {-0.147621f, -0.716567f, 0.681718f}, {-0.425325f, -0.688191f, 0.587785f},
  {-0.162460f, -0.262866f, 0.951056f}, {0.442863f, -0.238856f, 0.864188f},
  {0.162460f, -0.262866f, 0.951056f}, {0.309017f, -0.500000f, 0.809017f},
  {0.147621f, -0.716567f, 0.681718f}, {0.000000f, -0.525731f, 0.850651f},
  {0.425325f, -0.688191f, 0.587785f}, {0.587785f, -0.425325f, 0.688191f},
  {0.688191f, -0.587785f, 0.425325f}, {-0.955423f, 0.295242f, 0.000000f},
  {-0.951056f, 0.162460f, 0.262866f}, {-1.000000f, 0.000000f, 0.000000f},
  {-0.850651f, 0.000000f, 0.525731f}, {-0.955423f, -0.295242f, 0.000000f},
  {-0.951056f, -0.162460f, 0.262866f}, {-0.864188f, 0.442863f, -0.238856f},
  {-0.951056f, 0.162460f, -0.262866f}, {-0.809017f, 0.309017f, -0.500000f},
  {-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f},
  {-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f},
  {-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f},
  {-0.688191f, 0.587785f, -0.425325f}, {-0.587785f, 0.425325f, -0.688191f},
  {-0.425325f, 0.688191f, -0.587785f}, {-0.425325f, -0.688191f, -0.587785f},
  {-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f}
};

//FIXME(0xA5EA): Q_cos and Q_sin do not work on qvm
#if 0 //def Q3_VM
//===============================
// Cordic algorithm
#define NUM_CORDIC_ITER   30
#define M_PI2             (M_PI+M_PI)
#define HALF_M_PI         (0.5f*M_PI)
#define CORDIC_1K         0x26DD3B6A
#define CORDIC_SCALE      (vec_t)(1 << 30)
static const int cordicLU[] =
{
  0x3243F6A8, 0x1DAC6705, 0x0FADBAFC, 0x07F56EA6, 0x03FEAB76, 0x01FFD55B, 0x00FFFAAA, 0x007FFF55,
  0x003FFFEA, 0x001FFFFD, 0x000FFFFF, 0x0007FFFF, 0x0003FFFF, 0x0001FFFF, 0x0000FFFF, 0x00007FFF,
  0x00003FFF, 0x00001FFF, 0x00000FFF, 0x000007FF, 0x000003FF, 0x000001FF, 0x000000FF, 0x0000007F,
  0x0000003F, 0x0000001F, 0x0000000F, 0x00000008, 0x00000004, 0x00000002, 0x00000001, 0x00000000
};

extern force_inline void _cordic(int alpha, int *s, int *c)
{
  int tx, ty, tz, signz, x = CORDIC_1K, y = 0, i = 0;
  int z = alpha;

  for(i = 0; i < NUM_CORDIC_ITER; i++)
  {
    signz = (z >= 0 ? 0 : -1);
    tx    = x - (((y>>i)^signz) - signz);
    ty    = y + (((x>>i)^signz) - signz);
    tz    = z - ((cordicLU[i]^signz) - signz);
    x     = tx;
    y     = ty;
    z     = tz;
  }
  *c = x;
  *s = y;
}
extern force_inline int TranslateCordicAlpha(vec_t *alpha)
{
  int n = 0;

  // map alpha to -2pi <= alpha <= 2pi
  if (*alpha > M_PI2 || *alpha < -M_PI2)
  {
    *alpha -= ((int)(*alpha/M_PI2))*M_PI2;
  }
  // map alpha to -1/2pi <= alpha <= 1/2pi
  if (*alpha > HALF_M_PI || *alpha < -HALF_M_PI)
  {
    n       = (int)(*alpha/HALF_M_PI);
    *alpha -= n*HALF_M_PI;
  }
  return n;
}
//extern force_inline int   TranslateCordicAlpha(vec_t *alpha);
//extern force_inline vec_t  Q_cos(vec_t alpha);
static  ID_INLINE vec_t  Q_cos(vec_t alpha)
{
  int c = 0, s = 0, n = 0;
  n = TranslateCordicAlpha(&alpha);
  _cordic((int)(alpha*CORDIC_SCALE), &s, &c);

  switch(n)
  {
  case -3: case 1:
    return (-s/CORDIC_SCALE);

  case -1: case 3:
    return (s/CORDIC_SCALE);

  case -2: case 2:
    return (-c/CORDIC_SCALE);

  case  0: default:
    return (c/CORDIC_SCALE);
  }
}
//float QQ_sin(float x);
static  ID_INLINE vec_t Q_sin(vec_t alpha)
{
  int c = 0, s = 0, n = 0;

  n = TranslateCordicAlpha(&alpha);

  _cordic((int)(alpha*CORDIC_SCALE), &s, &c);

  switch(n)
  {
  case -3: case 1:
    return (c/CORDIC_SCALE);

  case -1: case 3:
    return (-c/CORDIC_SCALE);

  case -2: case 2:
    return (-s/CORDIC_SCALE);

  case 0: default:
    return (s/CORDIC_SCALE);
  }
}
//void  Q_SinCos(vec_t alpha, vec_t *s, vec_t *c);
extern force_inline void Q_SinCos(vec_t alpha, vec_t *s, vec_t *c)
{
  int ic = 0, is = 0, n = 0;

  n = TranslateCordicAlpha(&alpha);

  _cordic((int)(alpha*CORDIC_SCALE), &is, &ic);

  switch(n)
  {
  case -3: case 1:
    *c = (-is/CORDIC_SCALE);
    *s = (ic/CORDIC_SCALE);
    return;

  case -1: case 3:
    *c = (is/CORDIC_SCALE);
    *s = (-ic/CORDIC_SCALE);
    return;

  case -2: case 2:
    *c = (-ic/CORDIC_SCALE);
    *s = (-is/CORDIC_SCALE);
    return;

  case  0: default:
    *c = (ic/CORDIC_SCALE);
    *s = (is/CORDIC_SCALE);
    return;
  }
}

vec_t K_tan(vec_t alpha)
{
  int c = 0, s = 0, n = 0;
  vec_t Si, Co;

  n = TranslateCordicAlpha(&alpha);

  _cordic((int)(alpha*CORDIC_SCALE), &s, &c);

  switch(n)
  {
  case -3: case 1:
    Si = (c/CORDIC_SCALE);
    Co = (-s/CORDIC_SCALE);
    break;

  case -1: case 3:
    Si = (-c/CORDIC_SCALE);
    Co = (s/CORDIC_SCALE);
    break;

  case -2: case 2:
    Si = (-s/CORDIC_SCALE);
    Co = (-c/CORDIC_SCALE);
    break;

  case  0: default:
    Si = (s/CORDIC_SCALE);
    Co = (c/CORDIC_SCALE);
    break;
  }

  if (Q_fabs(Co) > .0f)
  {
    return (Si/Co);
  }
  else
  {
    return .0f;     // FIXME: 0 ok ?
  }
}
#endif
//==============================================================

int Q_rand(int *seed)
{
  *seed = (69069 * *seed + 1);
  return *seed;
}

// Range of [0,1]
float Q_random( int *seed )
{
  return ( Q_rand( seed ) & 0xffff ) / ( float ) 0x10000;
}

// Range of [-1,1]
float Q_crandom( int *seed )
{
  return 2.0 * ( Q_random( seed ) - 0.5 );
}

//=======================================================

byte ClampByte( int i )
{
  if ( i < 0 )
  {
    return 0;
  }

  if ( i > 255 )
  {
    return 255;
  }

  return i;
}

signed char ClampChar(int i)
{
  if (i < -128)
    return -128;
  if (i > 127)
    return 127;
  return i;
}

signed short ClampShort(int i)
{
  if (i < -32768)
    return -32768;
  if (i > 0x7fff)
    return 0x7fff;
  return i;
}


// this isn't a real cheap function to call!
int DirToByte(vec3_t dir)
{
  int		i, best;
  vec_t	d, bestd;

  if (!dir)
    return 0;

  bestd = 0;
  best = 0;
  for (i=0 ; i<NUMVERTEXNORMALS ; i++)
  {
    d = DotProduct (dir, bytedirs[i]);
    if (d > bestd)
    {
      bestd = d;
      best = i;
    }
  }
  return best;
}

void ByteToDir(int b, vec3_t dir)
{
  if (b < 0 || b >= NUMVERTEXNORMALS)
  {
    VectorCopy(vec3_origin, dir);
    return;
  }
  VectorCopy (bytedirs[b], dir);
}


unsigned ColorBytes3 (vec_t r, vec_t g, vec_t b)
{
  unsigned	i;

  ((byte *)&i)[0] = r * 255;
  ((byte *)&i)[1] = g * 255;
  ((byte *)&i)[2] = b * 255;

  return i;
}

unsigned ColorBytes4 (vec_t r, vec_t g, vec_t b, vec_t a)
{
  unsigned	i;

  ((byte *)&i)[0] = r * 255;
  ((byte *)&i)[1] = g * 255;
  ((byte *)&i)[2] = b * 255;
  ((byte *)&i)[3] = a * 255;

  return i;
}


float NormalizeColor( const vec3_t in, vec3_t out )
{
  float max;

  max = in[ 0 ];

  if ( in[ 1 ] > max )
  {
    max = in[ 1 ];
  }

  if ( in[ 2 ] > max )
  {
    max = in[ 2 ];
  }

  if ( !max )
  {
    VectorClear( out );
  }

  else
  {
    out[ 0 ] = in[ 0 ] / max;
    out[ 1 ] = in[ 1 ] / max;
    out[ 2 ] = in[ 2 ] / max;
  }

  return max;
}

void ClampColor(vec4_t color)
{
  int i;
  for(i = 0; i < 4; i++)
  {
    if (color[i] < 0)
      color[i] = 0;

    if (color[i] > 1)
      color[i] = 1;
  }
}

#ifdef USE_FLAMEGUN

#endif // USE_FLAMEGUN


/*
=====================
   PlaneNormalize
=====================
*/
vec_t PlaneNormalize(vec4_t plane)
{
  vec_t length, ilength;

  length = sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);

  if (length == 0)
  {
    VectorClear(plane);
    return 0;
  }

  ilength = 1.0 / length;
  plane[0] = plane[0] * ilength;
  plane[1] = plane[1] * ilength;
  plane[2] = plane[2] * ilength;
  plane[3] = plane[3] * ilength;

  return length;
}

/*
* =====================
* PlaneFromPoints
*
* Returns false if the triangle is degenerate.
* The normal will point out of the clock for clockwise ordered points
* =====================
*/
qboolean PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c)
{
  vec3_t d1, d2;

  VectorSubtract(b, a, d1);
  VectorSubtract(c, a, d2);
  CrossProduct(d2, d1, plane);

  if (VectorNormalize(plane) == 0)
  {
    return qfalse;
  }

  plane[3] = DotProduct(a, plane);
  return qtrue;
}

/*
* =====================
* PlaneFromPoints
* Returns false if the triangle is degenerate.
* =====================
*/
qboolean PlaneFromPointsOrder(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c, qboolean cw)
{
  vec3_t d1, d2;

  VectorSubtract(b, a, d1);
  VectorSubtract(c, a, d2);

  if (cw)
  {
    CrossProduct(d2, d1, plane);
  }

  else
  {
    CrossProduct(d1, d2, plane);
  }

  if (VectorNormalize(plane) == 0)
  {
    return qfalse;
  }

  plane[3] = DotProduct(a, plane);
  return qtrue;
}

/*
=====================
PlanesGetIntersectionPoint
=====================
*/
qboolean PlanesGetIntersectionPoint(const vec4_t plane1, const vec4_t plane2, const vec4_t plane3, vec3_t out)
{
  // http://www.cgafaq.info/wiki/Intersection_of_three_planes
  vec3_t	n1, n2, n3;
  vec3_t	n1n2, n2n3, n3n1;
  vec_t	denom;

  VectorNormalize2(plane1, n1);
  VectorNormalize2(plane2, n2);
  VectorNormalize2(plane3, n3);

  CrossProduct(n1, n2, n1n2);
  CrossProduct(n2, n3, n2n3);
  CrossProduct(n3, n1, n3n1);

  denom = DotProduct(n1, n2n3);

  // check if the denominator is zero (which would mean that no intersection is to be found
  if (denom == 0)
  {
    // no intersection could be found, return <0,0,0>
    VectorClear(out);
    return qfalse;
  }

  VectorClear(out);

  VectorMA(out, plane1[3], n2n3, out);
  VectorMA(out, plane2[3], n3n1, out);
  VectorMA(out, plane3[3], n1n2, out);

  VectorScale(out, 1.0f / denom, out);

  return qtrue;
}
void PlaneIntersectRay( const vec3_t rayPos, const vec3_t rayDir, const vec4_t plane, vec3_t res )
{
  vec3_t dir;
  float  sect;

  VectorNormalize2( rayDir, dir );

  sect = - ( DotProduct( plane, rayPos ) - plane[ 3 ] ) / DotProduct( plane, rayDir );
  VectorScale( dir, sect, dir );
  VectorAdd( rayPos, dir, res );
}
/*
 * ===============
 * RotatePointAroundVector
 * ===============
 */
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees )
{
  float sind, cosd, expr;
  vec3_t dxp;

  degrees = DEG2RAD( degrees );
  sind = sin( degrees );
  cosd = cos( degrees );
  expr = ( 1 - cosd ) * DotProduct( dir, point );
  CrossProduct( dir, point, dxp );

  dst[ 0 ] = expr * dir[ 0 ] + cosd * point[ 0 ] + sind * dxp[ 0 ];
  dst[ 1 ] = expr * dir[ 1 ] + cosd * point[ 1 ] + sind * dxp[ 1 ];
  dst[ 2 ] = expr * dir[ 2 ] + cosd * point[ 2 ] + sind * dxp[ 2 ];
}

/*
 * ===============
 * RotateAroundDirection
 * ===============
 */
void RotateAroundDirection( vec3_t axis[ 3 ], float yaw )
{
  // create an arbitrary axis[1]
  PerpendicularVector(axis[1], axis[0]);

  // rotate it around axis[0] by yaw
  if (yaw)
  {
    vec3_t	temp;
    VectorCopy(axis[1], temp);
    RotatePointAroundVector(axis[1], axis[0], temp, yaw);
  }

  // cross to get axis[2]
  CrossProduct(axis[0], axis[1], axis[2]);
}

/*
 * ================
 * Q_isnan
 *
 * Don't pass doubles to this
 * ================
 */
int Q_isnan( float x )
{
  return ( Q_floatBitsToUint( x ) & 0x7fffffff ) > 0x7f800000;
}
void VectorToAngles(const vec3_t value1, vec3_t angles)
{
  vec_t	forward;
  vec_t	yaw, pitch;

  if (value1[1] == 0 && value1[0] == 0)
  {
    yaw = 0;
    if (value1[2] > 0)
      pitch = 90;
    else
      pitch = 270;
  }
  else
  {
    if (value1[0])
      yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
    else if (value1[1] > 0)
      yaw = 90;
    else
      yaw = 270;

    if (yaw < 0)
      yaw += 360;

    forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
    pitch = (atan2(value1[2], forward) * 180 / M_PI);
    if (pitch < 0)
      pitch += 360;
  }

  angles[PITCH] = -pitch;
  angles[YAW] = yaw;
  angles[ROLL] = 0;
}


/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis(const vec3_t angles, vec3_t axis[3])
{
  vec3_t	right;
  // angle vectors returns "right" instead of "y axis"
  AngleVectors(angles, axis[0], right, axis[2]);
  VectorSubtract(vec3_origin, right, axis[1]);
}

#if 0
//0xA5EA, use macros instead of functions, clean this up some time

void AxisClear(vec3_t axis[3]) {
  axis[0][0] = 1;
  axis[0][1] = 0;
  axis[0][2] = 0;
  axis[1][0] = 0;
  axis[1][1] = 1;
  axis[1][2] = 0;
  axis[2][0] = 0;
  axis[2][1] = 0;
  axis[2][2] = 1;
}

void AxisCopy(vec3_t in[3], vec3_t out[3]) {
  VectorCopy(in[0], out[0]);
  VectorCopy(in[1], out[1]);
  VectorCopy(in[2], out[2]);
}
#endif

void ProjectPointOnPlane( vec3_t dst, const vec3_t point, const vec3_t normal )
{
  float d = -DotProduct( point, normal );
  VectorMA( point, d, normal, dst );
}

/*
 * ================
 * MakeNormalVectors
 *
 * Given a normalized forward vector, create two
 * other perpendicular vectors
 * ================
 */
void MakeNormalVectors(const vec3_t forward, vec3_t right, vec3_t up)
{
  vec_t	d;
  // this rotate and negate guarantees a vector
  // not colinear with the original
  right[1] = -forward[0];
  right[2] =  forward[1];
  right[0] =  forward[2];

  d = DotProduct (right, forward);
  VectorMA (right, -d, forward, right);
  VectorNormalize (right);
  CrossProduct (right, forward, up);
}

void VectorRotate(vec3_t in, vec3_t matrix[3], vec3_t out)
{
  out[0] = DotProduct(in, matrix[0]);
  out[1] = DotProduct(in, matrix[1]);
  out[2] = DotProduct(in, matrix[2]);
}

//============================================================================
#ifdef Q3_VM
#if !idppc
/*
** vec_t q_rsqrt(vec_t number)
*/
vec_t Q_rsqrt(vec_t number)
{
  floatint_t t;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  t.f  = number;
  t.i  = 0x5f3759df - (t.i >> 1);     // what the fuck?
  y  = t.f;
  y  = y * (threehalfs - (x2 * y * y));   // 1st iteration
  y  = y * (threehalfs - (x2 * y * y));   // 2nd iteration, this can be removed
  return y;
}

vec_t K_sqrt(vec_t x)
{
  return x*Q_rsqrt(x);
}
vec_t Q_fabs(vec_t f)
{
  floatint_t fi;
  fi.f = f;
  fi.i &= 0x7FFFFFFF;
  return fi.f;
}
#endif
#endif

//============================================================

/*
 * ===============
 * LerpAngle
 *
 * ===============
 */
float LerpAngle( float from, float to, float frac )
{
  if ( to - from > 180 )
  {
    to -= 360;
  }

  if ( to - from < -180 )
  {
    to += 360;
  }

  return ( from + frac * ( to - from ) );
}

/*
 * =================
 * LerpPosition
 *
 * =================
 */

void LerpPosition( vec3_t start, vec3_t end, float frac, vec3_t out )
{
  vec3_t dist;

  VectorSubtract( end, start, dist );
  VectorMA( start, frac, dist, out );
}

/*
 * =================
 * AngleSubtract
 *
 * Always returns a value from -180 to 180
 * =================
 */
float AngleSubtract( float a1, float a2 )
{
  float a = a1 - a2;


  return a - 360.0f * floor( ( a + 180.0f ) / 360.0f );
}

void AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 )
{
  v3[ 0 ] = AngleSubtract( v1[ 0 ], v2[ 0 ] );
  v3[ 1 ] = AngleSubtract( v1[ 1 ], v2[ 1 ] );
  v3[ 2 ] = AngleSubtract( v1[ 2 ], v2[ 2 ] );
}

float AngleMod( float a )
{
  return ( ( 360.0 / 65536 ) * ( ( int )( a * ( 65536 / 360.0 ) ) & 65535 ) );
}

/*
 * =================
 * AngleNormalize2Pi
 *
 * returns angle normalized to the range [0 <= angle < 2*M_PI]
 * =================
 */
float AngleNormalize2Pi( float angle )
{
  return DEG2RAD( AngleNormalize360( RAD2DEG( angle ) ) );
}
/*
 * =================
 * AngleNormalize360
 *
 * returns angle normalized to the range [0 <= angle < 360]
 * =================
 */
float AngleNormalize360( float angle )
{
  return ( 360.0 / 65536 ) * ( ( int )( angle * ( 65536 / 360.0 ) ) & 65535 );
}

/*
 * =================
 * AngleNormalize180
 *
 * returns angle normalized to the range [-180 < angle <= 180]
 * =================
 */
vec_t AngleNormalize180(vec_t angle)
{
  angle = AngleNormalize360(angle);
  if (angle > 180.0)
    angle -= 360.0;
  return angle;
}

/*
=================
AngleDelta
returns the normalized delta from angle1 to angle2
=================
*/
vec_t AngleDelta(vec_t angle1, vec_t angle2)
{
  return AngleNormalize180(angle1 - angle2);
}

/*
 * =================
 * AngleBetweenVectors
 *
 * returns the angle between two vectors normalized to the range [0 <= angle <= 180]
 * =================
 */
float AngleBetweenVectors( const vec3_t a, const vec3_t b )
{
  vec_t alen, blen;

  alen = VectorLength( a );
  blen = VectorLength( b );

  if ( !alen || !blen )
  {
    return 0;
  }

  // complete dot product of two vectors a, b is |a| * |b| * cos(angle)
  // this results in:
  //
  // angle = acos( (a * b) / (|a| * |b|) )
  return RAD2DEG( acos( DotProduct( a, b ) / ( alen * blen ) ) );
}

//============================================================

/*
 * =================
 * SetPlaneSignbits
 * =================
 */
void SetPlaneSignbits( cplane_t *out )
{
  int bits, j;

  // for fast box on planeside test
  bits = 0;

  for ( j = 0; j < 3; j++ )
  {
    if ( out->normal[ j ] < 0 )
    {
      bits |= 1 << j;
    }
  }

  out->signbits = bits;
}

/*
 * ==================
 * BoxOnPlaneSide
 *
 * Returns 1, 2, or 1 + 2
 * ==================
 */
#ifndef BSPC //hypov8 defined in q_shared.h
#if defined(Q3_VM)
int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, struct cplane_s *p)
{
  float	dist[2];
  int		sides, b, i;

// fast axial cases
  if (p->type < 3)
  {
    if (p->dist <= emins[p->type])
      return 1;
    if (p->dist >= emaxs[p->type])
      return 2;
    return 3;
  }

// general case
  dist[0] = dist[1] = 0;
  if (p->signbits < 8) // >= 8: default case is original code (dist[0]=dist[1]=0)
  {
    for (i=0 ; i<3 ; i++)
    {
      b = (p->signbits >> i) & 1;
      dist[ b] += p->normal[i]*emaxs[i];
      dist[!b] += p->normal[i]*emins[i];
    }
  }

  sides = 0;
  if (dist[0] >= p->dist)
    sides = 1;
  if (dist[1] < p->dist)
    sides |= 2;

  return sides;
}
#endif
#endif
/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs )
{
  int    i;
  vec3_t corner;
  float  a, b;

  for (i=0 ; i<3 ; i++)
  {
    a = Q_fabs( mins[ i ] );
    b = Q_fabs( maxs[ i ] );
    corner[i] = a > b ? a : b;
  }

  return VectorLength (corner);
}

void ZeroBounds( vec3_t mins, vec3_t maxs )
{
  mins[ 0 ] = mins[ 1 ] = mins[ 2 ] = 0;
  maxs[ 0 ] = maxs[ 1 ] = maxs[ 2 ] = 0;
}
void ClearBounds(vec3_t mins, vec3_t maxs)
{
  mins[0] = mins[1] = mins[2] = 99999;
  maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds(const vec3_t v, vec3_t mins, vec3_t maxs)
{
  if (v[0] < mins[0])
    mins[0] = v[0];
  if (v[0] > maxs[0])
    maxs[0] = v[0];

  if (v[1] < mins[1])
    mins[1] = v[1];
  if (v[1] > maxs[1])
    maxs[1] = v[1];

  if (v[2] < mins[2])
    mins[2] = v[2];
  if (v[2] > maxs[2])
    maxs[2] = v[2];
}

qboolean PointInBounds( const vec3_t v, const vec3_t mins, const vec3_t maxs )
{
  if ( v[ 0 ] < mins[ 0 ] )
  {
    return qfalse;
  }

  if ( v[ 0 ] > maxs[ 0 ] )
  {
    return qfalse;
  }

  if ( v[ 1 ] < mins[ 1 ] )
  {
    return qfalse;
  }

  if ( v[ 1 ] > maxs[ 1 ] )
  {
    return qfalse;
  }

  if ( v[ 2 ] < mins[ 2 ] )
  {
    return qfalse;
  }

  if ( v[ 2 ] > maxs[ 2 ] )
  {
    return qfalse;
  }

  return qtrue;
}

void BoundsAdd(vec3_t mins, vec3_t maxs, const vec3_t mins2, const vec3_t maxs2)
{
  if (mins2[0] < mins[0])
    mins[0] = mins2[0];
  if (mins2[1] < mins[1])
    mins[1] = mins2[1];
  if (mins2[2] < mins[2])
    mins[2] = mins2[2];

  if (maxs2[0] > maxs[0])
    maxs[0] = maxs2[0];
  if (maxs2[1] > maxs[1])
    maxs[1] = maxs2[1];
  if (maxs2[2] > maxs[2])
    maxs[2] = maxs2[2];
}

qboolean BoundsIntersect( const vec3_t mins, const vec3_t maxs, const vec3_t mins2, const vec3_t maxs2 )
{
  if ( maxs[ 0 ] < mins2[ 0 ] ||
          maxs[ 1 ] < mins2[ 1 ] || maxs[ 2 ] < mins2[ 2 ] || mins[ 0 ] > maxs2[ 0 ] || mins[ 1 ] > maxs2[ 1 ] || mins[ 2 ] > maxs2[ 2 ] )
  {
    return qfalse;
  }

  return qtrue;
}

qboolean BoundsIntersectSphere( const vec3_t mins, const vec3_t maxs, const vec3_t origin, vec_t radius )
{
  if ( origin[ 0 ] - radius > maxs[ 0 ] ||
          origin[ 0 ] + radius < mins[ 0 ] ||
          origin[ 1 ] - radius > maxs[ 1 ] ||
          origin[ 1 ] + radius < mins[ 1 ] || origin[ 2 ] - radius > maxs[ 2 ] || origin[ 2 ] + radius < mins[ 2 ] )
  {
    return qfalse;
  }

  return qtrue;
}

qboolean BoundsIntersectPoint( const vec3_t mins, const vec3_t maxs, const vec3_t origin )
{
  if ( origin[ 0 ] > maxs[ 0 ] ||
          origin[ 0 ] < mins[ 0 ] || origin[ 1 ] > maxs[ 1 ] || origin[ 1 ] < mins[ 1 ] || origin[ 2 ] > maxs[ 2 ] || origin[ 2 ] < mins[ 2 ] )
  {
    return qfalse;
  }

  return qtrue;
}
//hypov8 merge: unvan .50

float BoundsMaxExtent( const vec3_t mins, const vec3_t maxs ) {
  float result = Q_fabs( mins[0] );

  result = Q_max( result, Q_fabs( mins[ 1 ] ) );
  result = Q_max( result, Q_fabs( mins[ 2 ] ) );
  result = Q_max( result, Q_fabs( maxs[ 0 ] ) );
  result = Q_max( result, Q_fabs( maxs[ 1 ] ) );
  result = Q_max( result, Q_fabs( maxs[ 2 ] ) );
  return result;
}

int VectorCompare( const vec3_t v1, const vec3_t v2 )
{
  if ( v1[ 0 ] != v2[ 0 ] || v1[ 1 ] != v2[ 1 ] || v1[ 2 ] != v2[ 2 ] )
  {
    return 0;
  }

  return 1;
}

vec_t VectorNormalize( vec3_t v )
{
  float length, ilength;

  length = DotProduct( v, v );

  if ( length != 0.0f )
  {
    ilength = Q_rsqrt( length );
    /* sqrt(length) = length * (1 / sqrt(length)) */
    length *= ilength;
    VectorScale( v, ilength, v );
  }

  return length;
}
// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length
//
void VectorNormalizeFast( vec3_t v )
{
  float ilength;

  ilength = Q_rsqrt( DotProduct( v, v ) );

  VectorScale( v, ilength, v );
}

vec_t VectorNormalize2( const vec3_t v, vec3_t out )
{
  float length, ilength;

  length = v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ];

  if ( length )
  {
    ilength = Q_rsqrt( length );
    /* sqrt(length) = length * (1 / sqrt(length)) */
    length *= ilength;
    VectorScale( v, ilength, out );
  }

  else
  {
    VectorClear( out );
  }

  return length;
}
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc )
{
  vecc[ 0 ] = veca[ 0 ] + scale * vecb[ 0 ];
  vecc[ 1 ] = veca[ 1 ] + scale * vecb[ 1 ];
  vecc[ 2 ] = veca[ 2 ] + scale * vecb[ 2 ];
}

vec_t _DotProduct( const vec3_t v1, const vec3_t v2 )
{
  return v1[ 0 ] * v2[ 0 ] + v1[ 1 ] * v2[ 1 ] + v1[ 2 ] * v2[ 2 ];
}

void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out )
{
  out[ 0 ] = veca[ 0 ] - vecb[ 0 ];
  out[ 1 ] = veca[ 1 ] - vecb[ 1 ];
  out[ 2 ] = veca[ 2 ] - vecb[ 2 ];
}

void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out )
{
  out[ 0 ] = veca[ 0 ] + vecb[ 0 ];
  out[ 1 ] = veca[ 1 ] + vecb[ 1 ];
  out[ 2 ] = veca[ 2 ] + vecb[ 2 ];
}

void _VectorCopy( const vec3_t in, vec3_t out )
{
  out[ 0 ] = in[ 0 ];
  out[ 1 ] = in[ 1 ];
  out[ 2 ] = in[ 2 ];
}

void _VectorScale( const vec3_t in, vec_t scale, vec3_t out )
{
  out[ 0 ] = in[ 0 ] * scale;
  out[ 1 ] = in[ 1 ] * scale;
  out[ 2 ] = in[ 2 ] * scale;
}
//#if 1 //ndef BSPC
//FIXME (0xA5EA):
#ifndef KMAP2
void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross)
{
  cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
  cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
  cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}
#endif

#ifndef KMAP2
vec_t VectorLength(vec3_t const v)
{
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}
#endif
vec_t VectorLengthSquared(vec3_t const v)
{
  return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}
//FIXME 0xA5EA: inlining probleme
vec_t  Distance(vec3_t const p1, vec3_t const p2)
{
  vec3_t v;
  VectorSubtract (p2, p1, v);
  return VectorLength(v);
}

 vec_t DistanceSquared(vec3_t const  p1, vec3_t const  p2)
{
  vec3_t	v;
  VectorSubtract (p2, p1, v);
  return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}
#if 0 //macro
void VectorInverse( vec3_t v )
{
  v[ 0 ] = -v[ 0 ];
  v[ 1 ] = -v[ 1 ];
  v[ 2 ] = -v[ 2 ];
}
#endif
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out )
{
  out[ 0 ] = in[ 0 ] * scale;
  out[ 1 ] = in[ 1 ] * scale;
  out[ 2 ] = in[ 2 ] * scale;
  out[ 3 ] = in[ 3 ] * scale;
}

int NearestPowerOfTwo( int val )
{
  int answer;

  for ( answer = 1; answer < val; answer <<= 1 )
  {
    ;
  }

  return answer;
}

int Q_log2(int val)
{
  int answer = 0;
  while ((val >>= 1) != 0) answer++;
  return answer;
}

void VectorReflect(vec3_t const v, vec3_t const normal, vec3_t out)
{
  float d;
  d = 2.0 * (v[0] * normal[0] + v[1] * normal[1] + v[2] * normal[2]);
  out[0] = v[0] - normal[0] * d;
  out[1] = v[1] - normal[1] * d;
  out[2] = v[2] - normal[2] * d;
}

/*
 * ================
 * AxisMultiply
 * ================
 */
void AxisMultiply( float in1[ 3 ][ 3 ], float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] )
{
  out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
  out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
  out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
  out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
  out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
  out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
  out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
  out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
  out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] + in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
}

void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up )
{
  float        angle;
  static float sr, sp, sy, cr, cp, cy;

  // static to help MS compiler fp bugs

  angle = angles[ YAW ] * ( M_PI * 2 / 360 );
  sy = sin( angle );
  cy = cos( angle );

  angle = angles[ PITCH ] * ( M_PI * 2 / 360 );
  sp = sin( angle );
  cp = cos( angle );

  angle = angles[ ROLL ] * ( M_PI * 2 / 360 );
  sr = sin( angle );
  cr = cos( angle );

  if ( forward )
  {
    forward[ 0 ] = cp * cy;
    forward[ 1 ] = cp * sy;
    forward[ 2 ] = -sp;
  }

  if ( right )
  {
    right[ 0 ] = ( -1 * sr * sp * cy + -1 * cr * -sy );
    right[ 1 ] = ( -1 * sr * sp * sy + -1 * cr * cy );
    right[ 2 ] = -1 * sr * cp;
  }

  if ( up )
  {
    up[ 0 ] = ( cr * sp * cy + -sr * -sy );
    up[ 1 ] = ( cr * sp * sy + -sr * cy );
    up[ 2 ] = cr * cp;
  }
}

/*
 * =================
 * PerpendicularVector
 *
 * assumes "src" is normalized
 * =================
 */
void PerpendicularVector( vec3_t dst, const vec3_t src )
{
  int    pos;
  int    i;
  float  minelem = 1.0F;
  vec3_t tempvec;

  /*
   * * find the smallest magnitude axially aligned vector
   */
  for ( pos = 0, i = 0; i < 3; i++ )
  {
    if ( Q_fabs( src[ i ] ) < minelem )
    {
      pos = i;
      minelem = Q_fabs( src[ i ] );
    }
  }

  tempvec[ 0 ] = tempvec[ 1 ] = tempvec[ 2 ] = 0.0F;
  tempvec[ pos ] = 1.0F;

  /*
   * * project the point onto the plane defined by src
   */
  ProjectPointOnPlane( dst, tempvec, src );

  /*
   * * normalize the result
   */
  VectorNormalize( dst );
}

// Ridah

/*
 * =================
 * GetPerpendicularViewVector
 *
 *  Used to find an "up" vector for drawing a sprite so that it always faces the view as best as possible
 * =================
 */
void GetPerpendicularViewVector( const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up )
{
  vec3_t v1, v2;

  VectorSubtract( point, p1, v1 );
  VectorNormalize( v1 );

  VectorSubtract( point, p2, v2 );
  VectorNormalize( v2 );

  CrossProduct( v1, v2, up );
  VectorNormalize( up );
}

/*
 * ================
 * ProjectPointOntoVector
 * ================
 */
void ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj )
{
  vec3_t pVec, vec;

  VectorSubtract( point, vStart, pVec );
  VectorSubtract( vEnd, vStart, vec );
  VectorNormalize( vec );
  // project onto the directional vector for this segment
  VectorMA( vStart, DotProduct( pVec, vec ), vec, vProj );
}

#define LINE_DISTANCE_EPSILON 1e-05f

/*
 * ================
 * DistanceBetweenLineSegmentsSquared
 * Return the smallest distance between two line segments, squared
 * ================
 */

vec_t DistanceBetweenLineSegmentsSquared( const vec3_t sP0, const vec3_t sP1,
        const vec3_t tP0, const vec3_t tP1, float *s, float *t )
{
  vec3_t sMag, tMag, diff;
  float  a, b, c, d, e;
  float  D;
  float  sN, sD;
  float  tN, tD;
  vec3_t separation;

  VectorSubtract( sP1, sP0, sMag );
  VectorSubtract( tP1, tP0, tMag );
  VectorSubtract( sP0, tP0, diff );
  a = DotProduct( sMag, sMag );
  b = DotProduct( sMag, tMag );
  c = DotProduct( tMag, tMag );
  d = DotProduct( sMag, diff );
  e = DotProduct( tMag, diff );
  sD = tD = D = a * c - b * b;

  if ( D < LINE_DISTANCE_EPSILON )
  {
    // the lines are almost parallel
    sN = 0.0; // force using point P0 on segment S1
    sD = 1.0; // to prevent possible division by 0.0 later
    tN = e;
    tD = c;
  }

  else
  {
    // get the closest points on the infinite  lines
    sN = ( b * e - c * d );
    tN = ( a * e - b * d );

    if ( sN < 0.0 )
    {
      // sN < 0 => the s=0 edge is visible
      sN = 0.0;
      tN = e;
      tD = c;
    }

    else if ( sN > sD )
    {
      // sN > sD => the s=1 edge is visible
      sN = sD;
      tN = e + b;
      tD = c;
    }
  }

  if ( tN < 0.0 )
  {
    // tN < 0 => the t=0 edge is visible
    tN = 0.0;

    // recompute sN for this edge
    if ( -d < 0.0 )
    {
      sN = 0.0;
    }

    else if ( -d > a )
    {
      sN = sD;
    }

    else
    {
      sN = -d;
      sD = a;
    }
  }

  else if ( tN > tD )
  {
    // tN > tD => the t=1 edge is visible
    tN = tD;

    // recompute sN for this edge
    if ( ( -d + b ) < 0.0 )
    {
      sN = 0;
    }

    else if ( ( -d + b ) > a )
    {
      sN = sD;
    }

    else
    {
      sN = ( -d + b );
      sD = a;
    }
  }

  // finally do the division to get *s and *t
  *s = ( fabs( sN ) < LINE_DISTANCE_EPSILON ? 0.0 : sN / sD );
  *t = ( fabs( tN ) < LINE_DISTANCE_EPSILON ? 0.0 : tN / tD );

  // get the difference of the two closest points
  VectorScale( sMag, *s, sMag );
  VectorScale( tMag, *t, tMag );
  VectorAdd( diff, sMag, separation );
  VectorSubtract( separation, tMag, separation );

  return VectorLengthSquared( separation );
}

/*
 * ================
 * DistanceBetweenLineSegments
 *
 * Return the smallest distance between two line segments
 * ================
 */

vec_t DistanceBetweenLineSegments( const vec3_t sP0, const vec3_t sP1, const vec3_t tP0, const vec3_t tP1, float *s, float *t )
{
  return ( vec_t ) sqrt( DistanceBetweenLineSegmentsSquared( sP0, sP1, tP0, tP1, s, t ) );
}

/*
 * ================
 * ProjectPointOntoVectorBounded
 * ================
 */
void ProjectPointOntoVectorBounded( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj )
{
  vec3_t pVec, vec;
  int    j;

  VectorSubtract( point, vStart, pVec );
  VectorSubtract( vEnd, vStart, vec );
  VectorNormalize( vec );
  // project onto the directional vector for this segment
  VectorMA( vStart, DotProduct( pVec, vec ), vec, vProj );

  // check bounds
  for ( j = 0; j < 3; j++ )
  {
    if ( ( vProj[ j ] > vStart[ j ] && vProj[ j ] > vEnd[ j ] ) || ( vProj[ j ] < vStart[ j ] && vProj[ j ] < vEnd[ j ] ) )
    {
      break;
    }
  }

  if ( j < 3 )
  {
    if ( Q_fabs( vProj[ j ] - vStart[ j ] ) < Q_fabs( vProj[ j ] - vEnd[ j ] ) )
    {
      VectorCopy( vStart, vProj );
    }

    else
    {
      VectorCopy( vEnd, vProj );
    }
  }
}

/*
 * ================
 * DistanceFromLineSquared
 * ================
 */
float DistanceFromLineSquared( vec3_t p, vec3_t lp1, vec3_t lp2 )
{
  vec3_t proj, t;
  int    j;

  ProjectPointOntoVector( p, lp1, lp2, proj );

  for ( j = 0; j < 3; j++ )
  {
    if ( ( proj[ j ] > lp1[ j ] && proj[ j ] > lp2[ j ] ) || ( proj[ j ] < lp1[ j ] && proj[ j ] < lp2[ j ] ) )
    {
      break;
    }
  }

  if ( j < 3 )
  {
    if ( Q_fabs( proj[ j ] - lp1[ j ] ) < Q_fabs( proj[ j ] - lp2[ j ] ) )
    {
      VectorSubtract( p, lp1, t );
    }

    else
    {
      VectorSubtract( p, lp2, t );
    }

    return VectorLengthSquared( t );
  }

  VectorSubtract( p, proj, t );
  return VectorLengthSquared( t );
}

/*
 * ================
 * DistanceFromVectorSquared
 * ================
 */
float DistanceFromVectorSquared( vec3_t p, vec3_t lp1, vec3_t lp2 )
{
  vec3_t proj, t;

  ProjectPointOntoVector( p, lp1, lp2, proj );
  VectorSubtract( p, proj, t );
  return VectorLengthSquared( t );
}
 //hypov8 merge: game dll
/*
float vectoyaw( const vec3_t vec )
{
  float yaw;

  if ( vec[ YAW ] == 0 && vec[ PITCH ] == 0 )
  {
    yaw = 0;
  }

  else
  {
    if ( vec[ PITCH ] )
    {
      yaw = ( atan2( vec[ YAW ], vec[ PITCH ] ) * 180 / M_PI );
    }

    else if ( vec[ YAW ] > 0 )
    {
      yaw = 90;
    }

    else
    {
      yaw = 270;
    }

    if ( yaw < 0 )
    {
      yaw += 360;
    }
  }

  return yaw;
}
*/
/*
 * =================
 * AxisToAngles
 *
 *  Used to convert the MD3 tag axis to MDC tag angles, which are much smaller
 *
 *  This doesn't have to be fast, since it's only used for conversion in utils, try to avoid
 *  using this during gameplay
 * =================
 */
void AxisToAngles( /*const*/ vec3_t axis[ 3 ], vec3_t angles )
{
  float length1;
  float yaw, pitch, roll = 0.0f;

  if ( axis[ 0 ][ 1 ] == 0 && axis[ 0 ][ 0 ] == 0 )
  {
    yaw = 0;

    if ( axis[ 0 ][ 2 ] > 0 )
    {
      pitch = 90;
    }

    else
    {
      pitch = 270;
    }
  }

  else
  {
    if ( axis[ 0 ][ 0 ] )
    {
      yaw = ( atan2( axis[ 0 ][ 1 ], axis[ 0 ][ 0 ] ) * 180 / M_PI );
    }

    else if ( axis[ 0 ][ 1 ] > 0 )
    {
      yaw = 90;
    }

    else
    {
      yaw = 270;
    }

    if ( yaw < 0 )
    {
      yaw += 360;
    }

    length1 = sqrt( axis[ 0 ][ 0 ] * axis[ 0 ][ 0 ] + axis[ 0 ][ 1 ] * axis[ 0 ][ 1 ] );
    pitch = ( atan2( axis[ 0 ][ 2 ], length1 ) * 180 / M_PI );

    if ( pitch < 0 )
    {
      pitch += 360;
    }

    roll = ( atan2( axis[ 1 ][ 2 ], axis[ 2 ][ 2 ] ) * 180 / M_PI );

    if ( roll < 0 )
    {
      roll += 360;
    }
  }

  angles[ PITCH ] = -pitch;
  angles[ YAW ] = yaw;
  angles[ ROLL ] = roll;
}

float VectorDistance( vec3_t v1, vec3_t v2 )
{
  vec3_t dir;

  VectorSubtract( v2, v1, dir );
  return VectorLength( dir );
}

float VectorDistanceSquared( vec3_t v1, vec3_t v2 )
{
  vec3_t dir;

  VectorSubtract( v2, v1, dir );
  return VectorLengthSquared( dir );
}

// done.

/*
 * ================
 * VectorMaxComponent
 *
 * Return the biggest component of some vector
 * ================
 */
float VectorMaxComponent( vec3_t v )
{
  float biggest = v[ 0 ];

  if ( v[ 1 ] > biggest )
  {
    biggest = v[ 1 ];
  }

  if ( v[ 2 ] > biggest )
  {
    biggest = v[ 2 ];
  }

  return biggest;
}

/*
 * ================
 * VectorMinComponent
 *
 * Return the smallest component of some vector
 * ================
 */
float VectorMinComponent( vec3_t v )
{
  float smallest = v[ 0 ];

  if ( v[ 1 ] < smallest )
  {
    smallest = v[ 1 ];
  }

  if ( v[ 2 ] < smallest )
  {
    smallest = v[ 2 ];
  }

  return smallest;
}

//=============================================

// RB: XreaL matrix math functions

// *INDENT-OFF*
void MatrixIdentity( matrix_t m )
{
  m[ 0 ] = 1;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixClear( matrix_t m )
{
  m[ 0 ] = 0;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 0;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 0;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 0;
}

void MatrixCopy( const matrix_t in, matrix_t out )
{
  out[ 0 ] = in[ 0 ];
  out[ 4 ] = in[ 4 ];
  out[ 8 ] = in[ 8 ];
  out[ 12 ] = in[ 12 ];
  out[ 1 ] = in[ 1 ];
  out[ 5 ] = in[ 5 ];
  out[ 9 ] = in[ 9 ];
  out[ 13 ] = in[ 13 ];
  out[ 2 ] = in[ 2 ];
  out[ 6 ] = in[ 6 ];
  out[ 10 ] = in[ 10 ];
  out[ 14 ] = in[ 14 ];
  out[ 3 ] = in[ 3 ];
  out[ 7 ] = in[ 7 ];
  out[ 11 ] = in[ 11 ];
  out[ 15 ] = in[ 15 ];
}

qboolean MatrixCompare( const matrix_t a, const matrix_t b )
{
  return ( a[ 0 ] == b[ 0 ] && a[ 4 ] == b[ 4 ] && a[ 8 ] == b[ 8 ] && a[ 12 ] == b[ 12 ] &&
           a[ 1 ] == b[ 1 ] && a[ 5 ] == b[ 5 ] && a[ 9 ] == b[ 9 ] && a[ 13 ] == b[ 13 ] &&
           a[ 2 ] == b[ 2 ] && a[ 6 ] == b[ 6 ] && a[ 10 ] == b[ 10 ] && a[ 14 ] == b[ 14 ] &&
           a[ 3 ] == b[ 3 ] && a[ 7 ] == b[ 7 ] && a[ 11 ] == b[ 11 ] && a[ 15 ] == b[ 15 ] );
}

void MatrixTranspose( const matrix_t in, matrix_t out )
{
  out[ 0 ] = in[ 0 ];
  out[ 1 ] = in[ 4 ];
  out[ 2 ] = in[ 8 ];
  out[ 3 ] = in[ 12 ];
  out[ 4 ] = in[ 1 ];
  out[ 5 ] = in[ 5 ];
  out[ 6 ] = in[ 9 ];
  out[ 7 ] = in[ 13 ];
  out[ 8 ] = in[ 2 ];
  out[ 9 ] = in[ 6 ];
  out[ 10 ] = in[ 10 ];
  out[ 11 ] = in[ 14 ];
  out[ 12 ] = in[ 3 ];
  out[ 13 ] = in[ 7 ];
  out[ 14 ] = in[ 11 ];
  out[ 15 ] = in[ 15 ];
}

// helper functions for MatrixInverse from GtkRadiant C mathlib
static float m3_det( matrix3x3_t mat )
{
  float det;

  det = mat[ 0 ] * ( mat[ 4 ] * mat[ 8 ] - mat[ 7 ] * mat[ 5 ] )
        - mat[ 1 ] * ( mat[ 3 ] * mat[ 8 ] - mat[ 6 ] * mat[ 5 ] )
        + mat[ 2 ] * ( mat[ 3 ] * mat[ 7 ] - mat[ 6 ] * mat[ 4 ] );

  return ( det );
}

/*static int m3_inverse( matrix3x3_t mr, matrix3x3_t ma )
 * {
 *  float det = m3_det( ma );
 *
 *  if (det == 0 )
 *  {
 *    return 1;
 *  }
 *
 *
 *  mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
 *  mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
 *  mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;
 *
 *  mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
 *  mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
 *  mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;
 *
 *  mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
 *  mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
 *  mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;
 *
 *  return 0;
 * }*/

static void m4_submat( matrix_t mr, matrix3x3_t mb, int i, int j )
{
  int ti, tj, idst = 0, jdst = 0;

  for ( ti = 0; ti < 4; ti++ )
  {
    if ( ti < i )
    {
      idst = ti;
    }

    else if ( ti > i )
    {
      idst = ti - 1;
    }

    for ( tj = 0; tj < 4; tj++ )
    {
      if ( tj < j )
      {
        jdst = tj;
      }

      else if ( tj > j )
      {
        jdst = tj - 1;
      }

      if ( ti != i && tj != j )
      {
        mb[ idst * 3 + jdst ] = mr[ ti * 4 + tj ];
      }
    }
  }
}

static float m4_det( matrix_t mr )
{
  float       det, result = 0, i = 1;
  matrix3x3_t msub3;
  int         n;

  for ( n = 0; n < 4; n++, i *= -1 )
  {
    m4_submat( mr, msub3, 0, n );

    det = m3_det( msub3 );
    result += mr[ n ] * det * i;
  }

  return result;
}

qboolean MatrixInverse( matrix_t matrix )
{
  float       mdet = m4_det( matrix );
  matrix3x3_t mtemp;
  int         i, j, sign;
  matrix_t    m4x4_temp;

#if 0

  if ( fabs( mdet ) < 0.0000000001 )
  {
    return qtrue;
  }

#endif

  MatrixCopy( matrix, m4x4_temp );

  for ( i = 0; i < 4; i++ )
  {
    for ( j = 0; j < 4; j++ )
    {
      sign = 1 - ( ( i + j ) % 2 ) * 2;

      m4_submat( m4x4_temp, mtemp, i, j );

      // FIXME: try using * inverse det and see if speed/accuracy are good enough
      matrix[ i + j * 4 ] = ( m3_det( mtemp ) * sign ) / mdet;
    }
  }

  return qfalse;
}

void MatrixSetupXRotation( matrix_t m, vec_t degrees )
{
  vec_t a = DEG2RAD( degrees );

  m[ 0 ] = 1;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = cos( a );
  m[ 9 ] = -sin( a );
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = sin( a );
  m[ 10 ] = cos( a );
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupYRotation( matrix_t m, vec_t degrees )
{
  vec_t a = DEG2RAD( degrees );

  m[ 0 ] = cos( a );
  m[ 4 ] = 0;
  m[ 8 ] = sin( a );
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = -sin( a );
  m[ 6 ] = 0;
  m[ 10 ] = cos( a );
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupZRotation( matrix_t m, vec_t degrees )
{
  vec_t a = DEG2RAD( degrees );

  m[ 0 ] = cos( a );
  m[ 4 ] = -sin( a );
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = sin( a );
  m[ 5 ] = cos( a );
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupTranslation( matrix_t m, vec_t x, vec_t y, vec_t z )
{
  m[ 0 ] = 1;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = x;
  m[ 1 ] = 0;
  m[ 5 ] = 1;
  m[ 9 ] = 0;
  m[ 13 ] = y;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1;
  m[ 14 ] = z;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupScale( matrix_t m, vec_t x, vec_t y, vec_t z )
{
  m[ 0 ] = x;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = y;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = z;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupShear( matrix_t m, vec_t x, vec_t y )
{
  m[ 0 ] = 1;
  m[ 4 ] = x;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = y;
  m[ 5 ] = 1;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixMultiply( const matrix_t a, const matrix_t b, matrix_t out )
{
#if idx86_sse
  //#error MatrixMultiply
  int    i;
  __m128 _t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;

  _t4 = _mm_loadu_ps( &a[ 0 ] );
  _t5 = _mm_loadu_ps( &a[ 4 ] );
  _t6 = _mm_loadu_ps( &a[ 8 ] );
  _t7 = _mm_loadu_ps( &a[ 12 ] );

  for ( i = 0; i < 4; i++ )
  {
    _t0 = _mm_load1_ps( &b[ i * 4 + 0 ] );
    _t0 = _mm_mul_ps( _t4, _t0 );

    _t1 = _mm_load1_ps( &b[ i * 4 + 1 ] );
    _t1 = _mm_mul_ps( _t5, _t1 );

    _t2 = _mm_load1_ps( &b[ i * 4 + 2 ] );
    _t2 = _mm_mul_ps( _t6, _t2 );

    _t3 = _mm_load1_ps( &b[ i * 4 + 3 ] );
    _t3 = _mm_mul_ps( _t7, _t3 );

    _t1 = _mm_add_ps( _t0, _t1 );
    _t2 = _mm_add_ps( _t1, _t2 );
    _t3 = _mm_add_ps( _t2, _t3 );

    _mm_storeu_ps( &out[ i * 4 ], _t3 );
  }

#else
  out[ 0 ] = b[ 0 ] * a[ 0 ] + b[ 1 ] * a[ 4 ] + b[ 2 ] * a[ 8 ] + b[ 3 ] * a[ 12 ];
  out[ 1 ] = b[ 0 ] * a[ 1 ] + b[ 1 ] * a[ 5 ] + b[ 2 ] * a[ 9 ] + b[ 3 ] * a[ 13 ];
  out[ 2 ] = b[ 0 ] * a[ 2 ] + b[ 1 ] * a[ 6 ] + b[ 2 ] * a[ 10 ] + b[ 3 ] * a[ 14 ];
  out[ 3 ] = b[ 0 ] * a[ 3 ] + b[ 1 ] * a[ 7 ] + b[ 2 ] * a[ 11 ] + b[ 3 ] * a[ 15 ];

  out[ 4 ] = b[ 4 ] * a[ 0 ] + b[ 5 ] * a[ 4 ] + b[ 6 ] * a[ 8 ] + b[ 7 ] * a[ 12 ];
  out[ 5 ] = b[ 4 ] * a[ 1 ] + b[ 5 ] * a[ 5 ] + b[ 6 ] * a[ 9 ] + b[ 7 ] * a[ 13 ];
  out[ 6 ] = b[ 4 ] * a[ 2 ] + b[ 5 ] * a[ 6 ] + b[ 6 ] * a[ 10 ] + b[ 7 ] * a[ 14 ];
  out[ 7 ] = b[ 4 ] * a[ 3 ] + b[ 5 ] * a[ 7 ] + b[ 6 ] * a[ 11 ] + b[ 7 ] * a[ 15 ];

  out[ 8 ] = b[ 8 ] * a[ 0 ] + b[ 9 ] * a[ 4 ] + b[ 10 ] * a[ 8 ] + b[ 11 ] * a[ 12 ];
  out[ 9 ] = b[ 8 ] * a[ 1 ] + b[ 9 ] * a[ 5 ] + b[ 10 ] * a[ 9 ] + b[ 11 ] * a[ 13 ];
  out[ 10 ] = b[ 8 ] * a[ 2 ] + b[ 9 ] * a[ 6 ] + b[ 10 ] * a[ 10 ] + b[ 11 ] * a[ 14 ];
  out[ 11 ] = b[ 8 ] * a[ 3 ] + b[ 9 ] * a[ 7 ] + b[ 10 ] * a[ 11 ] + b[ 11 ] * a[ 15 ];

  out[ 12 ] = b[ 12 ] * a[ 0 ] + b[ 13 ] * a[ 4 ] + b[ 14 ] * a[ 8 ] + b[ 15 ] * a[ 12 ];
  out[ 13 ] = b[ 12 ] * a[ 1 ] + b[ 13 ] * a[ 5 ] + b[ 14 ] * a[ 9 ] + b[ 15 ] * a[ 13 ];
  out[ 14 ] = b[ 12 ] * a[ 2 ] + b[ 13 ] * a[ 6 ] + b[ 14 ] * a[ 10 ] + b[ 15 ] * a[ 14 ];
  out[ 15 ] = b[ 12 ] * a[ 3 ] + b[ 13 ] * a[ 7 ] + b[ 14 ] * a[ 11 ] + b[ 15 ] * a[ 15 ];
#endif
}

void MatrixMultiply2( matrix_t m, const matrix_t m2 )
{
  matrix_t tmp;

  MatrixCopy( m, tmp );
  MatrixMultiply( tmp, m2, m );
}

void MatrixMultiplyRotation( matrix_t m, vec_t pitch, vec_t yaw, vec_t roll )
{
  matrix_t tmp, rot;

  MatrixCopy( m, tmp );
  MatrixFromAngles( rot, pitch, yaw, roll );

  MatrixMultiply( tmp, rot, m );
}

void MatrixMultiplyZRotation( matrix_t m, vec_t degrees )
{
  matrix_t tmp;
  float angle = DEG2RAD( degrees );
  float s = sin( angle );
  float c = cos( angle );

  MatrixCopy( m, tmp );

  m[ 0 ] = tmp[ 0 ] * c + tmp[ 4 ] * s;
  m[ 1 ] = tmp[ 1 ] * c + tmp[ 5 ] * s;
  m[ 2 ] = tmp[ 2 ] * c + tmp[ 6 ] * s;
  m[ 3 ] = tmp[ 3 ] * c + tmp[ 7 ] * s;

  m[ 4 ] = tmp[ 0 ] * -s + tmp[ 4 ] * c;
  m[ 5 ] = tmp[ 1 ] * -s + tmp[ 5 ] * c;
  m[ 6 ] = tmp[ 2 ] * -s + tmp[ 6 ] * c;
  m[ 7 ] = tmp[ 3 ] * -s + tmp[ 7 ] * c;
}

void MatrixMultiplyTranslation( matrix_t m, vec_t x, vec_t y, vec_t z )
{
  m[ 12 ] += m[ 0 ] * x + m[ 4 ] * y + m[ 8 ] * z;
  m[ 13 ] += m[ 1 ] * x + m[ 5 ] * y + m[ 9 ] * z;
  m[ 14 ] += m[ 2 ] * x + m[ 6 ] * y + m[ 10 ] * z;
  m[ 15 ] += m[ 3 ] * x + m[ 7 ] * y + m[ 11 ] * z;
}

void MatrixMultiplyScale( matrix_t m, vec_t x, vec_t y, vec_t z )
{
  m[ 0 ] *= x;
  m[ 4 ] *= y;
  m[ 8 ] *= z;
  m[ 1 ] *= x;
  m[ 5 ] *= y;
  m[ 9 ] *= z;
  m[ 2 ] *= x;
  m[ 6 ] *= y;
  m[ 10 ] *= z;
  m[ 3 ] *= x;
  m[ 7 ] *= y;
  m[ 11 ] *= z;
}

void MatrixMultiplyShear( matrix_t m, vec_t x, vec_t y )
{
  matrix_t tmp;

  MatrixCopy( m, tmp );

  m[ 0 ] += m[ 4 ] * y;
  m[ 1 ] += m[ 5 ] * y;
  m[ 2 ] += m[ 6 ] * y;
  m[ 3 ] += m[ 7 ] * y;

  m[ 4 ] += tmp[ 0 ] * x;
  m[ 5 ] += tmp[ 1 ] * x;
  m[ 6 ] += tmp[ 2 ] * x;
  m[ 7 ] += tmp[ 3 ] * x;
}

void MatrixToAngles( const matrix_t m, vec3_t angles )
{
#if 1
  float theta;
  float cp;
  float sp;

  sp = m[ 2 ];

  // cap off our sin value so that we don't get any NANs
  if ( sp > 1.0 )
  {
    sp = 1.0;
  }

  else if ( sp < -1.0 )
  {
    sp = -1.0;
  }

  theta = -asin( sp );
  cp = cos( theta );

  if ( cp > 8192 *  1.192092896e-07F) //FLT_EPSILON
  {
    angles[ PITCH ] = RAD2DEG( theta );
    angles[ YAW ] = RAD2DEG( atan2( m[ 1 ], m[ 0 ] ) );
    angles[ ROLL ] = RAD2DEG( atan2( m[ 6 ], m[ 10 ] ) );
  }

  else
  {
    angles[ PITCH ] = RAD2DEG( theta );
    angles[ YAW ] = RAD2DEG( -atan2( m[ 4 ], m[ 5 ] ) );
    angles[ ROLL ] = 0;
  }

#else
  float a;
  float ca;

  a = asin( -m[ 2 ] );
  ca = cos( a );

  if ( fabs( ca ) > 0.005 )  // Gimbal lock?
  {
    angles[ PITCH ] = RAD2DEG( atan2( m[ 6 ] / ca, m[ 10 ] / ca ) );
    angles[ YAW ] = RAD2DEG( a );
    angles[ ROLL ] = RAD2DEG( atan2( m[ 1 ] / ca, m[ 0 ] / ca ) );
  }

  else
  {
    // Gimbal lock has occurred
    angles[ PITCH ] = RAD2DEG( atan2( -m[ 9 ], m[ 5 ] ) );
    angles[ YAW ] = RAD2DEG( a );
    angles[ ROLL ] = 0;
  }

#endif
}

void MatrixFromAngles( matrix_t m, vec_t pitch, vec_t yaw, vec_t roll )
{
  static float sr, sp, sy, cr, cp, cy;

  // static to help MS compiler fp bugs
  sp = sin( DEG2RAD( pitch ) );
  cp = cos( DEG2RAD( pitch ) );

  sy = sin( DEG2RAD( yaw ) );
  cy = cos( DEG2RAD( yaw ) );

  sr = sin( DEG2RAD( roll ) );
  cr = cos( DEG2RAD( roll ) );

  m[ 0 ] = cp * cy;
  m[ 4 ] = ( sr * sp * cy + cr * -sy );
  m[ 8 ] = ( cr * sp * cy + -sr * -sy );
  m[ 12 ] = 0;
  m[ 1 ] = cp * sy;
  m[ 5 ] = ( sr * sp * sy + cr * cy );
  m[ 9 ] = ( cr * sp * sy + -sr * cy );
  m[ 13 ] = 0;
  m[ 2 ] = -sp;
  m[ 6 ] = sr * cp;
  m[ 10 ] = cr * cp;
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixFromVectorsFLU( matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up )
{
  m[ 0 ] = forward[ 0 ];
  m[ 4 ] = left[ 0 ];
  m[ 8 ] = up[ 0 ];
  m[ 12 ] = 0;
  m[ 1 ] = forward[ 1 ];
  m[ 5 ] = left[ 1 ];
  m[ 9 ] = up[ 1 ];
  m[ 13 ] = 0;
  m[ 2 ] = forward[ 2 ];
  m[ 6 ] = left[ 2 ];
  m[ 10 ] = up[ 2 ];
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixFromVectorsFRU( matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up )
{
  m[ 0 ] = forward[ 0 ];
  m[ 4 ] = -right[ 0 ];
  m[ 8 ] = up[ 0 ];
  m[ 12 ] = 0;
  m[ 1 ] = forward[ 1 ];
  m[ 5 ] = -right[ 1 ];
  m[ 9 ] = up[ 1 ];
  m[ 13 ] = 0;
  m[ 2 ] = forward[ 2 ];
  m[ 6 ] = -right[ 2 ];
  m[ 10 ] = up[ 2 ];
  m[ 14 ] = 0;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixFromQuat( matrix_t m, const quat_t q )
{
#if 1

  /*
   *	From Quaternion to Matrix and Back
   *	February 27th 2005
   *	J.M.P. van Waveren
   *
   *	http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
   */
  float x2, y2, z2 /*, w2*/;
  float yy2, xy2;
  float xz2, yz2, zz2;
  float wz2, wy2, wx2, xx2;

  x2 = q[ 0 ] + q[ 0 ];
  y2 = q[ 1 ] + q[ 1 ];
  z2 = q[ 2 ] + q[ 2 ];
  //w2 = q[3] + q[3]; //Is this used for some underlying optimization?

  yy2 = q[ 1 ] * y2;
  xy2 = q[ 0 ] * y2;

  xz2 = q[ 0 ] * z2;
  yz2 = q[ 1 ] * z2;
  zz2 = q[ 2 ] * z2;

  wz2 = q[ 3 ] * z2;
  wy2 = q[ 3 ] * y2;
  wx2 = q[ 3 ] * x2;
  xx2 = q[ 0 ] * x2;

  m[ 0 ] = -yy2 - zz2 + 1.0f;
  m[ 1 ] = xy2 + wz2;
  m[ 2 ] = xz2 - wy2;

  m[ 4 ] = xy2 - wz2;
  m[ 5 ] = -xx2 - zz2 + 1.0f;
  m[ 6 ] = yz2 + wx2;

  m[ 8 ] = xz2 + wy2;
  m[ 9 ] = yz2 - wx2;
  m[ 10 ] = -xx2 - yy2 + 1.0f;

  m[ 3 ] = m[ 7 ] = m[ 11 ] = m[ 12 ] = m[ 13 ] = m[ 14 ] = 0;
  m[ 15 ] = 1;

#else

  /*
   *	http://www.gamedev.net/reference/articles/article1691.asp#Q54
   *	Q54. How do I convert a quaternion to a rotation matrix?
   *
   *	Assuming that a quaternion has been created in the form:
   *
   *	Q = |X Y Z W|
   *
   *	Then the quaternion can then be converted into a 4x4 rotation
   *	matrix using the following expression (Warning: you might have to
   *	transpose this matrix if you (do not) follow the OpenGL order!):
   *
   *	 ?        2     2                                      ?
   *	 ? 1 - (2Y  + 2Z )   2XY - 2ZW         2XZ + 2YW       ?
   *	 ?                                                     ?
   *	 ?                          2     2                    ?
   *	M = ? 2XY + 2ZW         1 - (2X  + 2Z )   2YZ - 2XW       ?
   *	 ?                                                     ?
   *	 ?                                            2     2  ?
   *	 ? 2XZ - 2YW         2YZ + 2XW         1 - (2X  + 2Y ) ?
   *	 ?                                                     ?
   */

  // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

  float xx, xy, xz, xw, yy, yz, yw, zz, zw;

  xx = q[ 0 ] * q[ 0 ];
  xy = q[ 0 ] * q[ 1 ];
  xz = q[ 0 ] * q[ 2 ];
  xw = q[ 0 ] * q[ 3 ];
  yy = q[ 1 ] * q[ 1 ];
  yz = q[ 1 ] * q[ 2 ];
  yw = q[ 1 ] * q[ 3 ];
  zz = q[ 2 ] * q[ 2 ];
  zw = q[ 2 ] * q[ 3 ];

  m[ 0 ] = 1 - 2 * ( yy + zz );
  m[ 1 ] = 2 * ( xy + zw );
  m[ 2 ] = 2 * ( xz - yw );
  m[ 4 ] = 2 * ( xy - zw );
  m[ 5 ] = 1 - 2 * ( xx + zz );
  m[ 6 ] = 2 * ( yz + xw );
  m[ 8 ] = 2 * ( xz + yw );
  m[ 9 ] = 2 * ( yz - xw );
  m[ 10 ] = 1 - 2 * ( xx + yy );

  m[ 3 ] = m[ 7 ] = m[ 11 ] = m[ 12 ] = m[ 13 ] = m[ 14 ] = 0;
  m[ 15 ] = 1;
#endif
}

void MatrixFromPlanes(matrix_t m, const vec4_t left, const vec4_t right, const vec4_t bottom, const vec4_t top, const vec4_t near, const vec4_t far)
{
  m[ 0] = (right[0] - left[0]) / 2;
  m[ 1] = (top[0] - bottom[0]) / 2;
  m[ 2] = (far[0] - near[0]) / 2;
  m[ 3] = right[0] - (right[0] - left[0]) / 2;

  m[ 4] = (right[1] - left[1]) / 2;
  m[ 5] = (top[1] - bottom[1]) / 2;
  m[ 6] = (far[1] - near[1]) / 2;
  m[ 7] = right[1] - (right[1] - left[1]) / 2;

  m[ 8] = (right[2] - left[2]) / 2;
  m[ 9] = (top[2] - bottom[2]) / 2;
  m[10] = (far[2] - near[2]) / 2;
  m[11] = right[2] - (right[2] - left[2]) / 2;

#if 0
  m[12] = (right[3] - left[3]) / 2;
  m[13] = (top[3] - bottom[3]) / 2;
  m[14] = (far[3] - near[3]) / 2;
  m[15] = right[3] - (right[3] - left[3]) / 2;
#else
  m[12] = (-right[3] - -left[3]) / 2;
  m[13] = (-top[3] - -bottom[3]) / 2;
  m[14] = (-far[3] - -near[3]) / 2;
  m[15] = -right[3] - (-right[3] - -left[3]) / 2;
#endif
}


void MatrixToVectorsFLU( const matrix_t m, vec3_t forward, vec3_t left, vec3_t up )
{
  if ( forward )
  {
    forward[ 0 ] = m[ 0 ]; // cp*cy;
    forward[ 1 ] = m[ 1 ]; // cp*sy;
    forward[ 2 ] = m[ 2 ]; //-sp;
  }

  if ( left )
  {
    left[ 0 ] = m[ 4 ]; // sr*sp*cy+cr*-sy;
    left[ 1 ] = m[ 5 ]; // sr*sp*sy+cr*cy;
    left[ 2 ] = m[ 6 ]; // sr*cp;
  }

  if ( up )
  {
    up[ 0 ] = m[ 8 ]; // cr*sp*cy+-sr*-sy;
    up[ 1 ] = m[ 9 ]; // cr*sp*sy+-sr*cy;
    up[ 2 ] = m[ 10 ]; // cr*cp;
  }
}

void MatrixToVectorsFRU(const matrix_t m, vec3_t forward, vec3_t right, vec3_t up)
{
  if (forward)
  {
    forward[0] = m[ 0];
    forward[1] = m[ 1];
    forward[2] = m[ 2];
  }

  if (right)
  {
    right[0] =-m[ 4];
    right[1] =-m[ 5];
    right[2] =-m[ 6];
  }

  if (up)
  {
    up[0] = m[ 8];
    up[1] = m[ 9];
    up[2] = m[10];
  }
}
void MatrixSetupTransformFromVectorsFLU( matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin )
{
  m[ 0 ] = forward[ 0 ];
  m[ 4 ] = left[ 0 ];
  m[ 8 ] = up[ 0 ];
  m[ 12 ] = origin[ 0 ];
  m[ 1 ] = forward[ 1 ];
  m[ 5 ] = left[ 1 ];
  m[ 9 ] = up[ 1 ];
  m[ 13 ] = origin[ 1 ];
  m[ 2 ] = forward[ 2 ];
  m[ 6 ] = left[ 2 ];
  m[ 10 ] = up[ 2 ];
  m[ 14 ] = origin[ 2 ];
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupTransformFromVectorsFRU( matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up, const vec3_t origin )
{
  m[ 0 ] = forward[ 0 ];
  m[ 4 ] = -right[ 0 ];
  m[ 8 ] = up[ 0 ];
  m[ 12 ] = origin[ 0 ];
  m[ 1 ] = forward[ 1 ];
  m[ 5 ] = -right[ 1 ];
  m[ 9 ] = up[ 1 ];
  m[ 13 ] = origin[ 1 ];
  m[ 2 ] = forward[ 2 ];
  m[ 6 ] = -right[ 2 ];
  m[ 10 ] = up[ 2 ];
  m[ 14 ] = origin[ 2 ];
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupTransformFromRotation( matrix_t m, const matrix_t rot, const vec3_t origin )
{
  m[ 0 ] = rot[ 0 ];
  m[ 4 ] = rot[ 4 ];
  m[ 8 ] = rot[ 8 ];
  m[ 12 ] = origin[ 0 ];
  m[ 1 ] = rot[ 1 ];
  m[ 5 ] = rot[ 5 ];
  m[ 9 ] = rot[ 9 ];
  m[ 13 ] = origin[ 1 ];
  m[ 2 ] = rot[ 2 ];
  m[ 6 ] = rot[ 6 ];
  m[ 10 ] = rot[ 10 ];
  m[ 14 ] = origin[ 2 ];
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixSetupTransformFromQuat( matrix_t m, const quat_t quat, const vec3_t origin )
{
  matrix_t rot;

  MatrixFromQuat( rot, quat );

  m[ 0 ] = rot[ 0 ];
  m[ 4 ] = rot[ 4 ];
  m[ 8 ] = rot[ 8 ];
  m[ 12 ] = origin[ 0 ];
  m[ 1 ] = rot[ 1 ];
  m[ 5 ] = rot[ 5 ];
  m[ 9 ] = rot[ 9 ];
  m[ 13 ] = origin[ 1 ];
  m[ 2 ] = rot[ 2 ];
  m[ 6 ] = rot[ 6 ];
  m[ 10 ] = rot[ 10 ];
  m[ 14 ] = origin[ 2 ];
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}
#if 1
void MatrixAffineInverse(const matrix_t in, matrix_t out)
{
    // cleaned up
    out[ 0] = in[ 0];       out[ 4] = in[ 1];       out[ 8] = in[ 2];
    out[ 1] = in[ 4];       out[ 5] = in[ 5];       out[ 9] = in[ 6];
    out[ 2] = in[ 8];       out[ 6] = in[ 9];       out[10] = in[10];
    out[ 3] = 0;            out[ 7] = 0;            out[11] = 0;            out[15] = 1;

    out[12] = -(in[12] * out[ 0] + in[13] * out[ 4] + in[14] * out[ 8]);
    out[13] = -(in[12] * out[ 1] + in[13] * out[ 5] + in[14] * out[ 9]);
    out[14] = -(in[12] * out[ 2] + in[13] * out[ 6] + in[14] * out[10]);
}
#endif

#ifdef Q3_VM
void AnglesToQuat(const vec3_t angles, quat_t q)
{
  QuatFromAngles(q, angles[PITCH], angles[YAW], angles[ROLL]);
}
#endif




void MatrixTransformNormal( const matrix_t m, const vec3_t in, vec3_t out )
{
  out[ 0 ] = m[ 0 ] * in[ 0 ] + m[ 4 ] * in[ 1 ] + m[ 8 ] * in[ 2 ];
  out[ 1 ] = m[ 1 ] * in[ 0 ] + m[ 5 ] * in[ 1 ] + m[ 9 ] * in[ 2 ];
  out[ 2 ] = m[ 2 ] * in[ 0 ] + m[ 6 ] * in[ 1 ] + m[ 10 ] * in[ 2 ];
}


void MatrixTransformNormal2(const matrix_t m, vec3_t inout)
{
  vec3_t          tmp ;
  tmp[ 0] = m[ 0] * inout[ 0] + m[ 4] * inout[ 1] + m[ 8] * inout[ 2];
  tmp[ 1] = m[ 1] * inout[ 0] + m[ 5] * inout[ 1] + m[ 9] * inout[ 2];
  tmp[ 2] = m[ 2] * inout[ 0] + m[ 6] * inout[ 1] + m[10] * inout[ 2];
  VectorCopy(tmp, inout);
}

void MatrixTransformPoint( const matrix_t m, const vec3_t in, vec3_t out )
{
  out[ 0 ] = m[ 0 ] * in[ 0 ] + m[ 4 ] * in[ 1 ] + m[ 8 ] * in[ 2 ] + m[ 12 ];
  out[ 1 ] = m[ 1 ] * in[ 0 ] + m[ 5 ] * in[ 1 ] + m[ 9 ] * in[ 2 ] + m[ 13 ];
  out[ 2 ] = m[ 2 ] * in[ 0 ] + m[ 6 ] * in[ 1 ] + m[ 10 ] * in[ 2 ] + m[ 14 ];
}

void MatrixTransformPoint2( const matrix_t m, vec3_t inout )
{
  vec3_t tmp;

  tmp[ 0 ] = m[ 0 ] * inout[ 0 ] + m[ 4 ] * inout[ 1 ] + m[ 8 ] * inout[ 2 ] + m[ 12 ];
  tmp[ 1 ] = m[ 1 ] * inout[ 0 ] + m[ 5 ] * inout[ 1 ] + m[ 9 ] * inout[ 2 ] + m[ 13 ];
  tmp[ 2 ] = m[ 2 ] * inout[ 0 ] + m[ 6 ] * inout[ 1 ] + m[ 10 ] * inout[ 2 ] + m[ 14 ];

  VectorCopy( tmp, inout );
}
/*
================
MatrixTransform4
================
*/
void MatrixTransform4(const matrix_t m, const vec4_t in, vec4_t out)
{
#if id386_sse
//#error MatrixTransform4

  __m128			_t0, _t1, _t2, _x, _y, _z, _w, _m0, _m1, _m2, _m3;

  _m0 = _mm_loadu_ps(&m[0]);
  _m1 = _mm_loadu_ps(&m[4]);
  _m2 = _mm_loadu_ps(&m[8]);
  _m3 = _mm_loadu_ps(&m[12]);

  _t0 = _mm_loadu_ps(in);
  _x = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(0, 0, 0, 0));
  _y = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(1, 1, 1, 1));
  _z = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(2, 2, 2, 2));
  _w = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(3, 3, 3, 3));

  _t0 = _mm_mul_ps(_m3, _w);
  _t1 = _mm_mul_ps(_m2, _z);
  _t0 = _mm_add_ps(_t0, _t1);

  _t1 = _mm_mul_ps(_m1, _y);
  _t2 = _mm_mul_ps(_m0, _x);
  _t1 = _mm_add_ps(_t1, _t2);

  _t0 = _mm_add_ps(_t0, _t1);

  _mm_storeu_ps(out, _t0);
#else
  out[ 0] = m[ 0] * in[ 0] + m[ 4] * in[ 1] + m[ 8] * in[ 2] + m[12] * in[ 3];
  out[ 1] = m[ 1] * in[ 0] + m[ 5] * in[ 1] + m[ 9] * in[ 2] + m[13] * in[ 3];
  out[ 2] = m[ 2] * in[ 0] + m[ 6] * in[ 1] + m[10] * in[ 2] + m[14] * in[ 3];
  out[ 3] = m[ 3] * in[ 0] + m[ 7] * in[ 1] + m[11] * in[ 2] + m[15] * in[ 3];
#endif
}

/*
================
MatrixTransformPlane
================
*/
void MatrixTransformPlane(const matrix_t m, const vec4_t in, vec4_t out)
{
  vec3_t translation ;
  vec3_t planePos ;

  // rotate the plane normal
  MatrixTransformNormal(m, in, out);

  // add new position to current plane position
  VectorSet(translation,  m[12], m[13], m[14]);
  VectorMA(translation, in[3], out, planePos);

  out[3] = DotProduct(out, planePos);
}

/*
================
MatrixTransformPlane2
================
*/
void MatrixTransformPlane2(const matrix_t m, vec4_t inout)
{
  vec4_t			tmp ;

  MatrixTransformPlane(m, inout, tmp);
  Vector4Copy(tmp, inout);
}
#if 1 //md5
//unvan 0.52

/*
* =================
* MatrixTransformBounds
*
* Achieves the same result as:
*
*   BoundsClear(omins, omaxs);
*	for each corner c in bounds{imins, imaxs}
*   {
*	    vec3_t p;
*		MatrixTransformPoint(m, c, p);
*       AddPointToBounds(p, omins, omaxs);
*   }
*
* With fewer operations
*
* Pseudocode:
*	omins = min(mins.x*m.c1, maxs.x*m.c1) + min(mins.y*m.c2, maxs.y*m.c2) + min(mins.z*m.c3, maxs.z*m.c3) + c4
*	omaxs = max(mins.x*m.c1, maxs.x*m.c1) + max(mins.y*m.c2, maxs.y*m.c2) + max(mins.z*m.c3, maxs.z*m.c3) + c4
* =================
*/
void MatrixTransformBounds(const matrix_t m, const vec3_t mins, const vec3_t maxs, vec3_t omins, vec3_t omaxs)
{
	vec3_t minx, maxx;
	vec3_t miny, maxy;
	vec3_t minz, maxz;

	const float* c1 = m;
	const float* c2 = m + 4;
	const float* c3 = m + 8;
	const float* c4 = m + 12;

	VectorScale(c1, mins[0], minx);
	VectorScale(c1, maxs[0], maxx);

	VectorScale(c2, mins[1], miny);
	VectorScale(c2, maxs[1], maxy);

	VectorScale(c3, mins[2], minz);
	VectorScale(c3, maxs[2], maxz);

	vec3_t tmins, tmaxs;
	vec3_t tmp;

	VectorMin(minx, maxx, tmins);
	VectorMax(minx, maxx, tmaxs);
	VectorAdd(tmins, c4, tmins);
	VectorAdd(tmaxs, c4, tmaxs);

	VectorMin(miny, maxy, tmp);
	VectorAdd(tmp, tmins, tmins);

	VectorMax(miny, maxy, tmp);
	VectorAdd(tmp, tmaxs, tmaxs);

	VectorMin(minz, maxz, tmp);
	VectorAdd(tmp, tmins, omins);

	VectorMax(minz, maxz, tmp);
	VectorAdd(tmp, tmaxs, omaxs);
}


#endif
/*
================
MatrixSetupTransform
================
*/
void MatrixSetupTransform(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin)
{
  m[ 0] = forward[0];     m[ 4] = left[0];        m[ 8] = up[0];  m[12] = origin[0];
  m[ 1] = forward[1];     m[ 5] = left[1];        m[ 9] = up[1];  m[13] = origin[1];
  m[ 2] = forward[2];     m[ 6] = left[2];        m[10] = up[2];  m[14] = origin[2];
  m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}




/*
================
_mat2x2_Determinante
================
*/
static ID_INLINE vec_t _mat2x2_Determinante(vec_t *A)
{
  return (A[0] * A[3] - A[1] * A[2]);
}

/*
================
_mat2x2_Inverse
================
*/
static ID_INLINE qboolean _mat2x2_Inverse(vec_t *A, vec_t *invA)
{
  vec_t detA = _mat2x2_Determinante(A); // 2x mult, 1x add

  if (Q_fabs(detA) <= MATRIX_INVERSE_EPSILON)
    return qfalse;

  invA[0] =  A[3] / detA;  // 1x mult
  invA[1] = -A[1] / detA;  // 1x mult
  invA[2] = -A[2] / detA;  // 1x mult
  invA[3] =  A[0] / detA;  // 1x mult
                           //sum: 6x mult, 1x add
  return qtrue;
}

/*
================
_mat4x4_Inverse
================
*/
#if 1 //hypov8 merge:
qboolean _mat4x4_Inverse(const vec_t *A, vec_t *invA)
{
   //     A =	/ P Q \ ; invA = / P_ Q_
   //         \ R S /          \ R_ S_
   // det(A) = det(P)det(S - R*invP*Q)
   //        = det(S)det(P - Q*invS*R)
   // Strassen algorithm

  vec_t P[4], invP[4];
  qboolean hasInverse;

  P[0] = A[0]; P[1] = A[1];
  P[2] = A[4]; P[3] = A[5];

  hasInverse = _mat2x2_Inverse(P, invP);			// 6x mult, 1x add

  // is matrix singular ?
  if (!hasInverse)
    return qfalse;

  {
    vec_t Q[4], R[4], S[4], R2[4], R3[4], R4[4], R5[4], R6[4], R7[4];

    // get sub matrix
    Q[0] = A[2]; Q[1] = A[3];
    Q[2] = A[6]; Q[3] = A[7];

    R[0] = A[8]; R[1] = A[9];
    R[2] = A[12]; R[3] =A[13];

    S[0] = A[10];  S[1] = A[11];
    S[2] = A[14];  S[3] = A[15];

    Mat2_Multiply(R, invP, R2);				// 8x mulitply, 4x addition
    Mat2_Multiply(invP, Q, R3);				// 8x mulitply, 4x addition
    Mat2_Multiply(R, R3, R4);					// 8x mulitply, 4x addition

    R5[0] = R4[0] - S[0]; R5[1] = R4[1] - S[1];
    R5[2] = R4[2] - S[2]; R5[3] = R4[3] - S[3];		//				 4x addition

    hasInverse = _mat2x2_Inverse(R5, R6);		// 6x mulitply, 1x add

    if (!hasInverse)
    {
      return hasInverse;
    }
    else
    {
      vec_t Q_[4], R_[4];

      //Q_ = R3 x R6
      Mat2_Multiply(R3, R6, Q_);			// 8x mulitply, 4x addition

      // R_ = R6 * R2
      Mat2_Multiply(R6, R2, R_);			// 8x mulitply, 4x addition

      // R7 = R3 * R_
      Mat2_Multiply(R3, R_, R7);			// 8x mulitply, 4x addition

      // P_ = R1 - R7
      invA[0] = invP[0] - R7[0];				//			  , 4x additon
      invA[1] = invP[1] - R7[1];
      invA[4] = invP[2] - R7[2];
      invA[5] = invP[3] - R7[3];

      invA[2] = Q_[0];
      invA[3] = Q_[1];
      invA[6] = Q_[2];
      invA[7] = Q_[3];

      invA[8] = R_[0];
      invA[9] = R_[1];
      invA[12]= R_[2];
      invA[13]= R_[3];

      invA[10] = -R6[0];
      invA[11] = -R6[1];
      invA[14] = -R6[2];
      invA[15] = -R6[3];
    }
    return qtrue;                                    // 60x mulitply, 34 addition
  }
}
#endif //end

// helper functions for MatrixInverse from GtkRadiant C mathlib



/*
 * replacement for glFrustum
 * see glspec30.pdf chapter 2.12 Coordinate Transformations
 */
void MatrixPerspectiveProjection( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = ( 2 * near ) / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = ( right + left ) / ( right - left );
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = ( 2 * near ) / ( top - bottom );
  m[ 9 ] = ( top + bottom ) / ( top - bottom );
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = - ( far + near ) / ( far - near );
  m[ 14 ] = - ( 2 * far * near ) / ( far - near );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = -1;
  m[ 15 ] = 0;
}

/*
 * same as D3DXMatrixPerspectiveOffCenterLH
 *
 * http://msdn.microsoft.com/en-us/library/bb205353(VS.85).aspx
 */
void MatrixPerspectiveProjectionLH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = ( 2 * near ) / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = ( left + right ) / ( left - right );
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = ( 2 * near ) / ( top - bottom );
  m[ 9 ] = ( top + bottom ) / ( bottom - top );
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = far / ( far - near );
  m[ 14 ] = ( near * far ) / ( near - far );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 1;
  m[ 15 ] = 0;
}

/*
 * same as D3DXMatrixPerspectiveOffCenterRH
 *
 * http://msdn.microsoft.com/en-us/library/bb205354(VS.85).aspx
 */
void MatrixPerspectiveProjectionRH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = ( 2 * near ) / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = ( left + right ) / ( right - left );
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = ( 2 * near ) / ( top - bottom );
  m[ 9 ] = ( top + bottom ) / ( top - bottom );
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = far / ( near - far );
  m[ 14 ] = ( near * far ) / ( near - far );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = -1;
  m[ 15 ] = 0;
}

/*
 * same as D3DXMatrixPerspectiveFovLH
 *
 * http://msdn.microsoft.com/en-us/library/bb205350(VS.85).aspx
 */
void MatrixPerspectiveProjectionFovYAspectLH( matrix_t m, vec_t fov, vec_t aspect, vec_t near, vec_t far )
{
  vec_t width, height;

  width = tanf( DEG2RAD( fov * 0.5f ) );
  height = width / aspect;

  m[ 0 ] = 1 / width;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1 / height;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = far / ( far - near );
  m[ 14 ] = - ( near * far ) / ( far - near );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 1;
  m[ 15 ] = 0;
}

void MatrixPerspectiveProjectionFovXYLH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far )
{
  vec_t width, height;

  width = tanf( DEG2RAD( fovX * 0.5f ) );
  height = tanf( DEG2RAD( fovY * 0.5f ) );

  m[ 0 ] = 1 / width;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1 / height;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = far / ( far - near );
  m[ 14 ] = - ( near * far ) / ( far - near );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 1;
  m[ 15 ] = 0;
}

void MatrixPerspectiveProjectionFovXYRH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far )
{
  vec_t width, height;

  width = tanf( DEG2RAD( fovX * 0.5f ) );
  height = tanf( DEG2RAD( fovY * 0.5f ) );

  m[ 0 ] = 1 / width;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1 / height;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = far / ( near - far );
  m[ 14 ] = ( near * far ) / ( near - far );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = -1;
  m[ 15 ] = 0;
}

// Tr3B: far plane at infinity, see RobustShadowVolumes.pdf by Nvidia
void MatrixPerspectiveProjectionFovXYInfiniteRH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near )
{
  vec_t width, height;

  width = tanf( DEG2RAD( fovX * 0.5f ) );
  height = tanf( DEG2RAD( fovY * 0.5f ) );

  m[ 0 ] = 1 / width;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = 0;
  m[ 1 ] = 0;
  m[ 5 ] = 1 / height;
  m[ 9 ] = 0;
  m[ 13 ] = 0;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = -1;
  m[ 14 ] = -2 * near;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = -1;
  m[ 15 ] = 0;
}

/*
 * replacement for glOrtho
 * see glspec30.pdf chapter 2.12 Coordinate Transformations
 */
void MatrixOrthogonalProjection( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = 2 / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = - ( right + left ) / ( right - left );
  m[ 1 ] = 0;
  m[ 5 ] = 2 / ( top - bottom );
  m[ 9 ] = 0;
  m[ 13 ] = - ( top + bottom ) / ( top - bottom );
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = -2 / ( far - near );
  m[ 14 ] = - ( far + near ) / ( far - near );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

/*
 * same as D3DXMatrixOrthoOffCenterLH
 *
 * http://msdn.microsoft.com/en-us/library/bb205347(VS.85).aspx
 */
void MatrixOrthogonalProjectionLH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = 2 / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = ( left + right ) / ( left - right );
  m[ 1 ] = 0;
  m[ 5 ] = 2 / ( top - bottom );
  m[ 9 ] = 0;
  m[ 13 ] = ( top + bottom ) / ( bottom - top );
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1 / ( far - near );
  m[ 14 ] = near / ( near - far );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

/*
 * same as D3DXMatrixOrthoOffCenterRH
 *
 * http://msdn.microsoft.com/en-us/library/bb205348(VS.85).aspx
 */
void MatrixOrthogonalProjectionRH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far )
{
  m[ 0 ] = 2 / ( right - left );
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = ( left + right ) / ( left - right );
  m[ 1 ] = 0;
  m[ 5 ] = 2 / ( top - bottom );
  m[ 9 ] = 0;
  m[ 13 ] = ( top + bottom ) / ( bottom - top );
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 1 / ( near - far );
  m[ 14 ] = near / ( near - far );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

/*
 * same as D3DXMatrixReflect
 *
 * http://msdn.microsoft.com/en-us/library/bb205356%28v=VS.85%29.aspx
 */
void MatrixPlaneReflection( matrix_t m, const vec4_t plane )
{
  vec4_t P;
  Vector4Copy( plane, P );

  PlaneNormalize( P );

  /*
   *	-2 * P.a * P.a + 1  -2 * P.b * P.a      -2 * P.c * P.a        0
   *	-2 * P.a * P.b      -2 * P.b * P.b + 1  -2 * P.c * P.b        0
   *	-2 * P.a * P.c      -2 * P.b * P.c      -2 * P.c * P.c + 1    0
   *	-2 * P.a * P.d      -2 * P.b * P.d      -2 * P.c * P.d        1
   */

  // Quake uses a different plane equation
  m[ 0 ] = -2 * P[ 0 ] * P[ 0 ] + 1;
  m[ 4 ] = -2 * P[ 0 ] * P[ 1 ];
  m[ 8 ] = -2 * P[ 0 ] * P[ 2 ];
  m[ 12 ] = 2 * P[ 0 ] * P[ 3 ];
  m[ 1 ] = -2 * P[ 1 ] * P[ 0 ];
  m[ 5 ] = -2 * P[ 1 ] * P[ 1 ] + 1;
  m[ 9 ] = -2 * P[ 1 ] * P[ 2 ];
  m[ 13 ] = 2 * P[ 1 ] * P[ 3 ];
  m[ 2 ] = -2 * P[ 2 ] * P[ 0 ];
  m[ 6 ] = -2 * P[ 2 ] * P[ 1 ];
  m[ 10 ] = -2 * P[ 2 ] * P[ 2 ] + 1;
  m[ 14 ] = 2 * P[ 2 ] * P[ 3 ];
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;

#if 0
  matrix_t m2;
  MatrixCopy( m, m2 );
  MatrixTranspose( m2, m );
#endif
}

void MatrixLookAtLH( matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up )
{
  vec3_t dirN;
  vec3_t upN;
  vec3_t sideN;

#if 1
  CrossProduct( up, dir, sideN );
  VectorNormalize( sideN );

  CrossProduct( dir, sideN, upN );
  VectorNormalize( upN );
#else
  CrossProduct( dir, up, sideN );
  VectorNormalize( sideN );

  CrossProduct( sideN, dir, upN );
  VectorNormalize( upN );
#endif

  VectorNormalize2( dir, dirN );

  m[ 0 ] = sideN[ 0 ];
  m[ 4 ] = sideN[ 1 ];
  m[ 8 ] = sideN[ 2 ];
  m[ 12 ] = -DotProduct( sideN, eye );
  m[ 1 ] = upN[ 0 ];
  m[ 5 ] = upN[ 1 ];
  m[ 9 ] = upN[ 2 ];
  m[ 13 ] = -DotProduct( upN, eye );
  m[ 2 ] = dirN[ 0 ];
  m[ 6 ] = dirN[ 1 ];
  m[ 10 ] = dirN[ 2 ];
  m[ 14 ] = -DotProduct( dirN, eye );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixLookAtRH( matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up )
{
  vec3_t dirN;
  vec3_t upN;
  vec3_t sideN;

  CrossProduct( dir, up, sideN );
  VectorNormalize( sideN );

  CrossProduct( sideN, dir, upN );
  VectorNormalize( upN );

  VectorNormalize2( dir, dirN );

  m[ 0 ] = sideN[ 0 ];
  m[ 4 ] = sideN[ 1 ];
  m[ 8 ] = sideN[ 2 ];
  m[ 12 ] = -DotProduct( sideN, eye );
  m[ 1 ] = upN[ 0 ];
  m[ 5 ] = upN[ 1 ];
  m[ 9 ] = upN[ 2 ];
  m[ 13 ] = -DotProduct( upN, eye );
  m[ 2 ] = -dirN[ 0 ];
  m[ 6 ] = -dirN[ 1 ];
  m[ 10 ] = -dirN[ 2 ];
  m[ 14 ] = DotProduct( dirN, eye );
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixScaleTranslateToUnitCube( matrix_t m, const vec3_t mins, const vec3_t maxs )
{
  m[ 0 ] = 2 / ( maxs[ 0 ] - mins[ 0 ] );
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = - ( maxs[ 0 ] + mins[ 0 ] ) / ( maxs[ 0 ] - mins[ 0 ] );

  m[ 1 ] = 0;
  m[ 5 ] = 2 / ( maxs[ 1 ] - mins[ 1 ] );
  m[ 9 ] = 0;
  m[ 13 ] = - ( maxs[ 1 ] + mins[ 1 ] ) / ( maxs[ 1 ] - mins[ 1 ] );

  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = 2 / ( maxs[ 2 ] - mins[ 2 ] );
  m[ 14 ] = - ( maxs[ 2 ] + mins[ 2 ] ) / ( maxs[ 2 ] - mins[ 2 ] );

  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

void MatrixCrop( matrix_t m, const vec3_t mins, const vec3_t maxs )
{
  float scaleX, scaleY, scaleZ;
  float offsetX, offsetY, offsetZ;

  scaleX = 2.0f / ( maxs[ 0 ] - mins[ 0 ] );
  scaleY = 2.0f / ( maxs[ 1 ] - mins[ 1 ] );

  offsetX = -0.5f * ( maxs[ 0 ] + mins[ 0 ] ) * scaleX;
  offsetY = -0.5f * ( maxs[ 1 ] + mins[ 1 ] ) * scaleY;

  scaleZ = 1.0f / ( maxs[ 2 ] - mins[ 2 ] );
  offsetZ = -mins[ 2 ] * scaleZ;

  m[ 0 ] = scaleX;
  m[ 4 ] = 0;
  m[ 8 ] = 0;
  m[ 12 ] = offsetX;
  m[ 1 ] = 0;
  m[ 5 ] = scaleY;
  m[ 9 ] = 0;
  m[ 13 ] = offsetY;
  m[ 2 ] = 0;
  m[ 6 ] = 0;
  m[ 10 ] = scaleZ;
  m[ 14 ] = offsetZ;
  m[ 3 ] = 0;
  m[ 7 ] = 0;
  m[ 11 ] = 0;
  m[ 15 ] = 1;
}

//end hypov8 merge:




//=============================================

// RB: XreaL quaternion math functions

// *INDENT-ON*

vec_t QuatNormalize( quat_t q )
{
  float length, ilength;

  length = DotProduct4( q, q );

  if ( length )
  {
    ilength = Q_rsqrt( length );
    length *= ilength;
    q[ 0 ] *= ilength;
    q[ 1 ] *= ilength;
    q[ 2 ] *= ilength;
    q[ 3 ] *= ilength;
  }

  return length;
}




void QuatFromAngles( quat_t q, vec_t pitch, vec_t yaw, vec_t roll )
{
#if 1
  matrix_t tmp;

  MatrixFromAngles( tmp, pitch, yaw, roll );
  QuatFromMatrix( q, tmp );
#else
  static float sr, sp, sy, cr, cp, cy;

  // static to help MS compiler fp bugs
  sp = sin( DEG2RAD( pitch ) );
  cp = cos( DEG2RAD( pitch ) );

  sy = sin( DEG2RAD( yaw ) );
  cy = cos( DEG2RAD( yaw ) );

  sr = sin( DEG2RAD( roll ) );
  cr = cos( DEG2RAD( roll ) );

  q[ 0 ] = sr * cp * cy - cr * sp * sy; // x
  q[ 1 ] = cr * sp * cy + sr * cp * sy; // y
  q[ 2 ] = cr * cp * sy - sr * sp * cy; // z
  q[ 3 ] = cr * cp * cy + sr * sp * sy; // w
#endif
}

void QuatFromMatrix(quat_t q, const matrix_t m)
{
  /*
     From Quaternion to Matrix and Back
     February 27th 2005
     J.M.P. van Waveren

     http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
   */
  vec_t           t, s;

  if (m[0] + m[5] + m[10] > 0.0f)
  {
    t = m[0] + m[5] + m[10] + 1.0f;
    s = (1.0f / sqrtf(t)) * 0.5f;

    q[3] = s * t;
    q[2] = (m[1] - m[4]) * s;
    q[1] = (m[8] - m[2]) * s;
    q[0] = (m[6] - m[9]) * s;
  }
  else if (m[0] > m[5] && m[0] > m[10])
  {
    t = m[0] - m[5] - m[10] + 1.0f;
    s = (1.0f / sqrtf(t)) * 0.5f;

    q[0] = s * t;
    q[1] = (m[1] + m[4]) * s;
    q[2] = (m[8] + m[2]) * s;
    q[3] = (m[6] - m[9]) * s;
  }
  else if (m[5] > m[10])
  {
    t = -m[0] + m[5] - m[10] + 1.0f;
    s = (1.0f / sqrtf(t)) * 0.5f;

    q[1] = s * t;
    q[0] = (m[1] + m[4]) * s;
    q[3] = (m[8] - m[2]) * s;
    q[2] = (m[6] + m[9]) * s;
  }
  else
  {
    t = -m[0] - m[5] + m[10] + 1.0f;
    s = (1.0f / sqrtf(t)) * 0.5f;

    q[2] = s * t;
    q[3] = (m[1] - m[4]) * s;
    q[0] = (m[8] + m[2]) * s;
    q[1] = (m[6] + m[9]) * s;
  }
}


/*
================
MatrixScaleTranslateToFit
================
*/
void MatrixScaleTranslateToFit(matrix_t m, const vec3_t mins, const vec3_t maxs)
{
  m[ 0] = 2/(maxs[0]-mins[0]);
  m[ 4] = 0;
  m[ 8] = 0;
  m[12] = -(maxs[0]+mins[0])/(maxs[0]-mins[0]);

  m[ 1] = 0;
  m[ 5] = 2/(maxs[1]-mins[1]);
  m[ 9] = 0;
  m[13] = -(maxs[1]+mins[1])/(maxs[1]-mins[1]);

  m[ 2] = 0;
  m[ 6] = 0;
  m[10] = 2/(maxs[2]-mins[2]);
  m[14] = -(maxs[2]+mins[2])/(maxs[2]-mins[2]);

  m[ 3] = 0;
  m[ 7] = 0;
  m[11] = 0;
  m[15] = 1;
}

#ifndef Q3_VM

#endif // Q3_VM

/*
================
VectorToAngles
================
*/
#if 0
void VectorToAngles(const vec3_t value1, vec3_t angles)
{

  vec_t           forward;
  vec_t           yaw, pitch;

  if (value1[1] == 0 && value1[0] == 0)
  {
    yaw = 0;
    if (value1[2] > 0)
      pitch = 90;
    else
      pitch = 270;
  }
  else
  {
    if (value1[0])
      yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
    else if (value1[1] > 0)
      yaw = 90;
    else
      yaw = 270;

    if (yaw < 0)
      yaw += 360;

    forward = sqrt(value1[0] * value1[0] + value1[1] * value1[1]);
    pitch = (atan2(value1[2], forward) * 180 / M_PI);

    if (pitch < 0)
      pitch += 360;
  }

  angles[PITCH] = -pitch;
  angles[YAW] = yaw;
  angles[ROLL] = 0;
}
#endif


//#ifndef BSPC

/*
================
  QuatSlerp
================
*/


void QuatToVectorsFLU( const quat_t q, vec3_t forward, vec3_t left, vec3_t up )
{
  matrix_t tmp;

  MatrixFromQuat( tmp, q );
  MatrixToVectorsFRU( tmp, forward, left, up );
}

void QuatToVectorsFRU( const quat_t q, vec3_t forward, vec3_t right, vec3_t up )
{
  matrix_t tmp;

  MatrixFromQuat( tmp, q );
  MatrixToVectorsFRU( tmp, forward, right, up );
}

void QuatToAxis( const quat_t q, vec3_t axis[ 3 ] )
{
  matrix_t tmp;

  MatrixFromQuat( tmp, q );
  MatrixToVectorsFLU( tmp, axis[ 0 ], axis[ 1 ], axis[ 2 ] );
}

void QuatToAngles( const quat_t q, vec3_t angles )
{
  quat_t q2;

  q2[ 0 ] = q[ 0 ] * q[ 0 ];
  q2[ 1 ] = q[ 1 ] * q[ 1 ];
  q2[ 2 ] = q[ 2 ] * q[ 2 ];
  q2[ 3 ] = q[ 3 ] * q[ 3 ];

  angles[ PITCH ] = RAD2DEG( asin( -2 * ( q[ 2 ] * q[ 0 ] - q[ 3 ] * q[ 1 ] ) ) );
  angles[ YAW ] = RAD2DEG( atan2( 2 * ( q[ 2 ] * q[ 3 ] + q[ 0 ] * q[ 1 ] ), ( q2[ 2 ] - q2[ 3 ] - q2[ 0 ] + q2[ 1 ] ) ) );
  angles[ ROLL ] = RAD2DEG( atan2( 2 * ( q[ 3 ] * q[ 0 ] + q[ 2 ] * q[ 1 ] ), ( -q2[ 2 ] - q2[ 3 ] + q2[ 0 ] + q2[ 1 ] ) ) );
}

void QuatMultiply0( quat_t qa, const quat_t qb )
{
  quat_t tmp;

  QuatCopy( qa, tmp );
  QuatMultiply1( tmp, qb, qa );
}

void QuatMultiply1( const quat_t qa, const quat_t qb, quat_t qc )
{
  /*
   *	   from matrix and quaternion faq
   *	   x = w1x2 + x1w2 + y1z2 - z1y2
   *	   y = w1y2 + y1w2 + z1x2 - x1z2
   *	   z = w1z2 + z1w2 + x1y2 - y1x2
   *
   *	   w = w1w2 - x1x2 - y1y2 - z1z2
   */

  qc[ 0 ] = qa[ 3 ] * qb[ 0 ] + qa[ 0 ] * qb[ 3 ] + qa[ 1 ] * qb[ 2 ] - qa[ 2 ] * qb[ 1 ];
  qc[ 1 ] = qa[ 3 ] * qb[ 1 ] + qa[ 1 ] * qb[ 3 ] + qa[ 2 ] * qb[ 0 ] - qa[ 0 ] * qb[ 2 ];
  qc[ 2 ] = qa[ 3 ] * qb[ 2 ] + qa[ 2 ] * qb[ 3 ] + qa[ 0 ] * qb[ 1 ] - qa[ 1 ] * qb[ 0 ];
  qc[ 3 ] = qa[ 3 ] * qb[ 3 ] - qa[ 0 ] * qb[ 0 ] - qa[ 1 ] * qb[ 1 ] - qa[ 2 ] * qb[ 2 ];
}

void QuatMultiply2( const quat_t qa, const quat_t qb, quat_t qc )
{
  qc[ 0 ] = qa[ 3 ] * qb[ 0 ] + qa[ 0 ] * qb[ 3 ] + qa[ 1 ] * qb[ 2 ] + qa[ 2 ] * qb[ 1 ];
  qc[ 1 ] = qa[ 3 ] * qb[ 1 ] - qa[ 1 ] * qb[ 3 ] - qa[ 2 ] * qb[ 0 ] + qa[ 0 ] * qb[ 2 ];
  qc[ 2 ] = qa[ 3 ] * qb[ 2 ] - qa[ 2 ] * qb[ 3 ] - qa[ 0 ] * qb[ 1 ] + qa[ 1 ] * qb[ 0 ];
  qc[ 3 ] = qa[ 3 ] * qb[ 3 ] - qa[ 0 ] * qb[ 0 ] - qa[ 1 ] * qb[ 1 ] + qa[ 2 ] * qb[ 2 ];
}

void QuatMultiply3( const quat_t qa, const quat_t qb, quat_t qc )
{
  qc[ 0 ] = qa[ 3 ] * qb[ 0 ] + qa[ 0 ] * qb[ 3 ] + qa[ 1 ] * qb[ 2 ] + qa[ 2 ] * qb[ 1 ];
  qc[ 1 ] = -qa[ 3 ] * qb[ 1 ] + qa[ 1 ] * qb[ 3 ] - qa[ 2 ] * qb[ 0 ] + qa[ 0 ] * qb[ 2 ];
  qc[ 2 ] = -qa[ 3 ] * qb[ 2 ] + qa[ 2 ] * qb[ 3 ] - qa[ 0 ] * qb[ 1 ] + qa[ 1 ] * qb[ 0 ];
  qc[ 3 ] = -qa[ 3 ] * qb[ 3 ] + qa[ 0 ] * qb[ 0 ] - qa[ 1 ] * qb[ 1 ] + qa[ 2 ] * qb[ 2 ];
}

void QuatMultiply4( const quat_t qa, const quat_t qb, quat_t qc )
{
  qc[ 0 ] = qa[ 3 ] * qb[ 0 ] - qa[ 0 ] * qb[ 3 ] - qa[ 1 ] * qb[ 2 ] - qa[ 2 ] * qb[ 1 ];
  qc[ 1 ] = -qa[ 3 ] * qb[ 1 ] - qa[ 1 ] * qb[ 3 ] + qa[ 2 ] * qb[ 0 ] - qa[ 0 ] * qb[ 2 ];
  qc[ 2 ] = -qa[ 3 ] * qb[ 2 ] - qa[ 2 ] * qb[ 3 ] + qa[ 0 ] * qb[ 1 ] - qa[ 1 ] * qb[ 0 ];
  qc[ 3 ] = -qa[ 3 ] * qb[ 3 ] - qa[ 0 ] * qb[ 0 ] + qa[ 1 ] * qb[ 1 ] - qa[ 2 ] * qb[ 2 ];
}

void QuatSlerp( const quat_t from, const quat_t to, float frac, quat_t out )
{
#if 0
  quat_t to1;
  float omega, cosom, sinom, scale0, scale1;

  cosom = from[ 0 ] * to[ 0 ] + from[ 1 ] * to[ 1 ] + from[ 2 ] * to[ 2 ] + from[ 3 ] * to[ 3 ];

  if ( cosom < 0.0 )
  {
    cosom = -cosom;

    QuatCopy( to, to1 );
    QuatAntipodal( to1 );
  }

  else
  {
    QuatCopy( to, to1 );
  }

  if ( ( 1.0 - cosom ) > 0 )
  {
    omega = acos( cosom );
    sinom = sin( omega );
    scale0 = sin( ( 1.0 - frac ) * omega ) / sinom;
    scale1 = sin( frac * omega ) / sinom;
  }

  else
  {
    scale0 = 1.0 - frac;
    scale1 = frac;
  }

  out[ 0 ] = scale0 * from[ 0 ] + scale1 * to1[ 0 ];
  out[ 1 ] = scale0 * from[ 1 ] + scale1 * to1[ 1 ];
  out[ 2 ] = scale0 * from[ 2 ] + scale1 * to1[ 2 ];
  out[ 3 ] = scale0 * from[ 3 ] + scale1 * to1[ 3 ];
#else

  /*
   *	   Slerping Clock Cycles
   *	   February 27th 2005
   *	   J.M.P. van Waveren
   *
   *	   http://www.intel.com/cd/ids/developer/asmo-na/eng/293747.htm
   */
  float cosom, absCosom, sinom, sinSqr, omega, scale0, scale1;

  if ( frac <= 0.0f )
  {
    QuatCopy( from, out );
    return;
  }

  if ( frac >= 1.0f )
  {
    QuatCopy( to, out );
    return;
  }

  if ( QuatCompare( from, to ) )
  {
    QuatCopy( from, out );
    return;
  }

  cosom = from[ 0 ] * to[ 0 ] + from[ 1 ] * to[ 1 ] + from[ 2 ] * to[ 2 ] + from[ 3 ] * to[ 3 ];
  absCosom = fabs( cosom );

  if ( ( 1.0f - absCosom ) > 1e-6f )
  {
    sinSqr = 1.0f - absCosom * absCosom;
    sinom = 1.0f / sqrt( sinSqr );
    omega = atan2( sinSqr * sinom, absCosom );

    scale0 = sin( ( 1.0f - frac ) * omega ) * sinom;
    scale1 = sin( frac * omega ) * sinom;
  }

  else
  {
    scale0 = 1.0f - frac;
    scale1 = frac;
  }

  scale1 = ( cosom >= 0.0f ) ? scale1 : -scale1;

  out[ 0 ] = scale0 * from[ 0 ] + scale1 * to[ 0 ];
  out[ 1 ] = scale0 * from[ 1 ] + scale1 * to[ 1 ];
  out[ 2 ] = scale0 * from[ 2 ] + scale1 * to[ 2 ];
  out[ 3 ] = scale0 * from[ 3 ] + scale1 * to[ 3 ];
#endif
}


void QuatTransformVector(const quat_t q, const vec3_t in, vec3_t out)
{
#if 0 //
  matrix_t        ALIGN16(m);
  MatrixFromQuat(m, q);
  Mat4_TransformNormal(m, in, out);
 #else //hypov8 merge: unvan .50
  vec3_t tmp, tmp2;

  CrossProduct( q, in, tmp );
  VectorScale( tmp, 2.0f, tmp );
  CrossProduct( q, tmp, tmp2 );
  VectorMA( in, q[3], tmp, out );
  VectorAdd( out, tmp2, out );
#endif
}


void QuatTransformVectorInverse(const quat_t q, const vec3_t in, vec3_t out)
{
  vec3_t tmp, tmp2;

  // The inverse rotation is obtained by negating the vector
  // component of q, but that is mathematically the same as
  // swapping the arguments of the cross product.
  CrossProduct( in, q, tmp );
  VectorScale( tmp, 2.0f, tmp );
  CrossProduct( tmp, q, tmp2 );
  VectorMA( in, q[3], tmp, out );
  VectorAdd( out, tmp2, out );
}


#if !defined( idx86_sse) //#ifndef USING_SSE_MATH
// create an identity transform
void TransInit(transform_t *t)
{
  QuatClear(t->rot);
  VectorClear(t->trans);
  t->scale = 1.0f;
}

// copy a transform
void TransCopy(const transform_t *in, transform_t *out)
{
  Com_Memcpy(out, in, sizeof(transform_t));
}

// apply a transform to a point
void TransformPoint(const transform_t *t, const vec3_t in, vec3_t out)
{
  QuatTransformVector(t->rot, in, out);
  VectorScale(out, t->scale, out);
  VectorAdd(out, t->trans, out);
}

// apply the inverse of a transform to a point
void TransformPointInverse(const transform_t *t, const vec3_t in, vec3_t out)
{
  VectorSubtract(in, t->trans, out);
  VectorScale(out, 1.0f / t->scale, out);
  QuatTransformVectorInverse(t->rot, out, out);
}

// apply a transform to a normal vector (ignore scale and translation)
void TransformNormalVector(const transform_t *t, const vec3_t in, vec3_t out)
{
  QuatTransformVector(t->rot, in, out);
}

// apply the inverse of a transform to a normal vector (ignore scale
// and translation)
void TransformNormalVectorInverse(const transform_t *t, const vec3_t in,
  vec3_t out)
{
  QuatTransformVectorInverse(t->rot, in, out);
}
//hypov8 merge:
// initialize a transform with a pure rotation
void TransInitRotationQuat(const quat_t quat, transform_t *t)
{
  QuatCopy(quat, t->rot);
  VectorClear(t->trans);
  t->scale = 1.0f;
}

void TransInitRotation(const vec3_t axis, float angle, transform_t *t)
{
  float sa = sin(0.5f * angle);
  float ca = cos(0.5f * angle);
  quat_t q;

  VectorScale(axis, sa, q);
  q[3] = ca;
  TransInitRotationQuat(q, t);
}

// initialize a transform with a pure translation
void TransInitTranslation(const vec3_t vec, transform_t *t)
{
  QuatClear(t->rot);
  VectorCopy(vec, t->trans);
  t->scale = 1.0f;
}

// initialize a transform with a pure scale
void TransInitScale(float factor, transform_t *t)
{
  QuatClear(t->rot);
  VectorClear(t->trans);
  t->scale = factor;
}

// add a rotation to the start of an existing transform
void TransInsRotationQuat(const quat_t quat, transform_t *t)
{
  QuatMultiply0(t->rot, quat);
}
void TransInsRotation(const vec3_t axis, float angle, transform_t *t)
{
  float sa = sin(0.5f * angle);
  float ca = cos(0.5f * angle);
  quat_t q;

  VectorScale(axis, sa, q);
  q[3] = ca;
  TransInsRotationQuat(q, t);
}

// add a rotation to the end of an existing transform
void TransAddRotationQuat(const quat_t quat, transform_t *t)
{
  quat_t tmp;

  QuatTransformVector(quat, t->trans, t->trans);
  QuatCopy(quat, tmp);
  QuatMultiply0(tmp, t->rot);
  QuatCopy(tmp, t->rot);
}

void TransAddRotation(const vec3_t axis, float angle, transform_t *t)
{
  float sa = sin(0.5f * angle);
  float ca = cos(0.5f * angle);
  quat_t q;

  VectorScale(axis, sa, q);
  q[3] = ca;
  TransAddRotationQuat(q, t);
}

// add a scale to the start of an existing transform
void TransInsScale(float factor, transform_t *t)
{
  t->scale *= factor;
}

// add a scale to the end of an existing transform
void TransAddScale(float factor, transform_t *t)
{
  VectorScale(t->trans, factor, t->trans);
  t->scale *= factor;
}

// add a translation at the start of an existing transformation
void TransInsTranslation(const vec3_t vec, transform_t *t)
{
  vec3_t tmp;

  TransformPoint(t, vec, tmp);
  VectorAdd(t->trans, tmp, t->trans);
}

// add a translation at the end of an existing transformation
void TransAddTranslation(const vec3_t vec, transform_t *t)
{
  VectorAdd(t->trans, vec, t->trans);
}

// combine transform a and transform b into transform c
void TransCombine(const transform_t *a, const transform_t *b,
  transform_t *out)
{
  TransCopy(a, out);

  TransAddRotationQuat(b->rot, out);
  TransAddScale(b->scale, out);
  TransAddTranslation(b->trans, out);
}

// compute the inverse transform
void TransInverse(const transform_t *in, transform_t *out)
{
  quat_t inverse;
  transform_t tmp;

  TransInit(&tmp);
  VectorNegate(in->trans, tmp.trans);
  TransAddScale(1.0f / in->scale, &tmp);
  QuatCopy(in->rot, inverse);
  QuatInverse(inverse);
  TransAddRotationQuat(inverse, &tmp);
  TransCopy(&tmp, out);
}

// lerp between transforms
void TransStartLerp(transform_t *t)
{
  QuatZero(t->rot);
  VectorClear(t->trans);
  t->scale = 0.0f;
}
void TransAddWeight( float weight, const transform_t *a, transform_t *out )
{
  if ( DotProduct4( out->rot, a->rot ) < 0 )
  {
    QuatMA( out->rot, -weight, a->rot, out->rot );
  }
  else
  {
    QuatMA( out->rot, weight, a->rot, out->rot );
  }

  VectorMA( out->trans, weight, a->trans, out->trans );
  out->scale      += a->scale      * weight;
}
void TransEndLerp(transform_t *t)
{
  QuatNormalize(t->rot);
}
#endif


//#endif //BSPC
