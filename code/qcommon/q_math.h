#ifndef _Q_MATH_H_
#define _Q_MATH_H_

#ifndef BSPC
# include "q_platform.h"
#else
#  ifdef _MSC_VER
#    define ID_INLINE __inline
#  else
#    define ID_INLINE inline
#  endif
#  if !defined(QDECL)
#    define QDECL
#  endif
#define LittleLong
#define LittleShort
#define LittleFloat
//FIXME 0xA5EA: this is crap
#endif

#ifdef Q3_VM
  typedef int intptr_t;
#else
# include <math.h>
#endif
/*
#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880f
#endif
*/
//#define DEG2RAD( a ) ( ( (a) * Q_PI ) / 180.0F )
//#define RAD2DEG( a ) ( ( (a) * 180.0f ) / Q_PI )

typedef unsigned char 		byte;

#ifdef __cplusplus
//#define qboolean int
	typedef int qboolean;
//#define qboolean bool
#define qtrue ((int)1)
#define qfalse ((int)0)
#else
typedef enum {qfalse, qtrue}	qboolean;
#endif


typedef union
{
  float f;
  int i;
  unsigned int ui;
} floatint_t;

typedef union
{
  unsigned char b[4];
  int i;
  unsigned int ui;
} byte4int_t;

typedef union
{
  unsigned char b[2];
  short         s;
} shortbytes_t;

typedef union
{
  char c[4];
  int i;
  unsigned int ui;
} char4int_t;

/*
#ifdef KPQ3_DOUBLE_VEC
typedef double vec_t;
#else
typedef float vec_t;
#endif

typedef vec_t  vec2_t[2];
typedef vec_t  vec3_t[3];
typedef vec_t  vec4_t[4];
typedef vec_t  vec5_t[5];
typedef vec_t axis_t[3][3];
typedef vec_t  quat_t[4];		// | x y z w |
typedef vec_t  matrix_t[16];
typedef vec_t  matrix3x3_t[9];
typedef double dvec_t;
typedef dvec_t  dvec3_t[3];*/

//qboolean VectorCompare( const vec3_t v1, const vec3_t v2 );

//#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
//#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];}
//#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
//#define VectorCopy(a,b) {b[0]=a[0];b[1]=a[1];b[2]=a[2];}
//#define VectorScale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}
//#define VectorClear(x) {x[0] = x[1] = x[2] = 0;}
//#define	VectorNegate(x) {x[0]=-x[0];x[1]=-x[1];x[2]=-x[2];}
//void Vec10Copy( vec_t *in, vec_t *out );

//vec_t _DotProduct (vec3_t v1, vec3_t v2);
//void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
//void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
//void _VectorCopy (vec3_t in, vec3_t out);
//void _VectorScale (vec3_t v, vec_t scale, vec3_t out);
/*
double _VectorLength( const vec3_t v );

void VectorMA( const vec3_t va, double scale, const vec3_t vb, vec3_t vc );

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
vec_t VectorNormalize( const vec3_t in, vec3_t out );
vec_t ColorNormalize( const vec3_t in, vec3_t out );
void VectorInverse (vec3_t v);

void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );

void NormalToLatLong( const vec3_t normal, byte bytes[2] );

int	PlaneTypeForNormal (vec3_t normal);

*/


#ifdef __cplusplus
//#define KPQ3_DOUBLE_VEC

//template<typename T> extern force_inline T Max3(T x, T y, T z) { return x > y ? x > z ? x : z : y > z ? y : z; }
//template<typename T> extern force_inline T Min3(T x, T y, T z) { return x < y ? x < z ? x : z : y < z ? y : z; }


//extern force_inline void Vec4_Negate(vec4_t const  src, vec4_t tgt)    { tgt[0] = -src[0]; tgt[1] = -src[1]; tgt[2] =- src[2]; tgt[3]=-src[3]; }
/////extern force_inline void Vec4_Add(vec4_t const  a, vec4_t const  b, vec4_t c)   { c[0] = a[0] + b[0]; c[1] = a[1] + b[1]; c[2] = a[2] + b[2]; c[3] = a[3] + b[3]; }
//////extern force_inline void Vec4_Subtract(vec4_t const  a, vec4_t const  b, vec4_t c)  { c[0]=a[0]-b[0]; c[1]=a[1]-b[1]; c[2]=a[2]-b[2]; c[3]=a[3]-b[3]; }

#if 0//hypov8 todo:merge
//#define KPQ3_DOUBLE_VEC
#ifdef KPQ3_DOUBLE_VEC
	using vec_t = double;
#else
	using vec_t = float;
#endif

	using vec2_t = vec_t[2];
	using vec3_t = vec_t[3];
	using vec4_t = vec_t[4];
	using axis_t = vec_t[3][3];
	using matrix3x3_t = vec_t[3 * 3];
	using matrix_t = vec_t[4 * 4];
	using quat_t = vec_t[4];
#endif
#endif

#if 0 //else
#define Max3(x, y, z) (((x) > (y)) ? (((x) > (z)) ? (x) : (z)) : (((y) > (z)) ? (y) : (z)))
#define Min3(x, y, z) (((x) < (y)) ? (((x) < (z)) ? (x) : (z)) : (((y) < (z)) ? (y) : (z)))

#define VectorClear(a)			      ((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		    ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorInverse(a)         ((a)[0]=-(a)[0],(a)[1]=-(a)[1],(a)[2]=-(a)[2])
#define VectorSet(v, x, y, z)  	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define VectorCompare(a,b)			    ((a)[0]==(b)[0]&&(a)[1]==(b)[1]&&(a)[2]==(b)[2])
#define	SnapVector(v)			      {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
#define DotProduct(x,y)  	((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)  	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)			    ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			    ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	  	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	  	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))


#define Vec4_Clear(a)			      ((a)[0]=(a)[1]=(a)[2]=(a)[3]=0)
#define Vec4_Negate(a,b)		    ((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2],(b)[3]=-(a)[3])
#define Vector4Copy(a,b)			    ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define Vector4Set(v,x,y,z,w)		  ((v)[0]=(x),(v)[1]=(y),(v)[2]=(z),(v)[3]=(w))
#define Vec4_Comp(a,b)			    ((a)[0]==(b)[0]&&(a)[1]==(b)[1]&&(a)[2]==(b)[2]&&(a)[3]==(b)[3])
#define Vec4_Add(a,b,c)			    ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define Vec4_Subtract(a,b,c)	  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define Vec4_Scale(a,scalar,b) 	((b)[0]=(a)[0]*(scalar),(b)[1]=(a)[1]*(scalar), \
                                 (b)[2]=(a)[2]*(scalar),(b)[3]=(a)[3]*(scalar))
#define DotProduct4(x,y)	  ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2]+(x)[3]*(y)[3])
#define	VectorMA(v, s, b, o)	    ((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s),(o)[3]=(v)[3]+(b)[3]*(s))


#define AxisClear(a)    ((a)[0][0] = 1, (a)[0][1] = 0, (a)[0][2] = 0,   \
                          (a)[1][0] = 0, (a)[1][1] = 1, (a)[1][2] = 0,   \
                          (a)[2][0] = 0, (a)[2][1] = 0, (a)[2][2] = 1    )

#define AxisCopy(a, b)  (VectorCopy((a)[0], (b)[0]),                     \
                          VectorCopy((a)[1], (b)[1]),                     \
                          VectorCopy((a)[2], (b)[2])                      )
#if 0
void AxisCopy( vec3_t in[3], vec3_t out[3] ) {
  VectorCopy( in[0], out[0] );
  VectorCopy( in[1], out[1] );
  VectorCopy( in[2], out[2] );
}
#endif

// Matrix marcos are significantly faster than the functions (about 6 times faster)
#define MatrixCopy(S, D) ((D)[0]=(S)[0], (D)[4]=(S)[4], (D)[8] =(S)[8], (D)[12]=(S)[12],                   	\
                         (D)[1]=(S)[1], (D)[5]=(S)[5], (D)[9] =(S)[9], (D)[13]=(S)[13],	                    \
                         (D)[2]=(S)[2], (D)[6]=(S)[6], (D)[10]=(S)[10], (D)[14]=(S)[14],                    \
                         (D)[3]=(S)[3], (D)[7]=(S)[7], (D)[11]=(S)[11], (D)[15]=(S)[15])

#define MatrixCompare(A, B) ((A)[0]==(B)[0] && (A)[4]==(B)[4] && (A)[ 8]==(B)[ 8] && (A)[12]==(B)[12] &&     \
                            (A)[1]==(B)[1] && (A)[5]==(B)[5] && (A)[ 9]==(B)[ 9] && (A)[13]==(B)[13] &&     \
                            (A)[2]==(B)[2] && (A)[6]==(B)[6] && (A)[10]==(B)[10] && (A)[14]==(B)[14] &&     \
                            (A)[3]==(B)[3] && (A)[7]==(B)[7] && (A)[11]==(B)[11] && (A)[15]==(B)[15] )

#define Mat2_Multiply(A, B, out) (	(out)[0] = (A)[0]*(B)[0] + (A)[1]*(B)[2], \
                                    (out)[1] = (A)[0]*(B)[1] + (A)[1]*(B)[3], \
                                    (out)[2] = (A)[2]*(B)[0] + (A)[3]*(B)[2], \
                                    (out)[3] = (A)[2]*(B)[1] + (A)[3]*(B)[3]  )

#define MatrixMultiply(a, b, out) ( \
              (out)[ 0] = (b)[ 0]*(a)[ 0] + (b)[ 1]*(a)[ 4] + (b)[ 2]*(a)[ 8] + (b)[ 3]*(a)[12], \
              (out)[ 1] = (b)[ 0]*(a)[ 1] + (b)[ 1]*(a)[ 5] + (b)[ 2]*(a)[ 9] + (b)[ 3]*(a)[13], \
              (out)[ 2] = (b)[ 0]*(a)[ 2] + (b)[ 1]*(a)[ 6] + (b)[ 2]*(a)[10] + (b)[ 3]*(a)[14], \
              (out)[ 3] = (b)[ 0]*(a)[ 3] + (b)[ 1]*(a)[ 7] + (b)[ 2]*(a)[11] + (b)[ 3]*(a)[15], \
              (out)[ 4] = (b)[ 4]*(a)[ 0] + (b)[ 5]*(a)[ 4] + (b)[ 6]*(a)[ 8] + (b)[ 7]*(a)[12], \
              (out)[ 5] = (b)[ 4]*(a)[ 1] + (b)[ 5]*(a)[ 5] + (b)[ 6]*(a)[ 9] + (b)[ 7]*(a)[13], \
              (out)[ 6] = (b)[ 4]*(a)[ 2] + (b)[ 5]*(a)[ 6] + (b)[ 6]*(a)[10] + (b)[ 7]*(a)[14], \
              (out)[ 7] = (b)[ 4]*(a)[ 3] + (b)[ 5]*(a)[ 7] + (b)[ 6]*(a)[11] + (b)[ 7]*(a)[15], \
              (out)[ 8] = (b)[ 8]*(a)[ 0] + (b)[ 9]*(a)[ 4] + (b)[10]*(a)[ 8] + (b)[11]*(a)[12], \
              (out)[ 9] = (b)[ 8]*(a)[ 1] + (b)[ 9]*(a)[ 5] + (b)[10]*(a)[ 9] + (b)[11]*(a)[13], \
              (out)[10] = (b)[ 8]*(a)[ 2] + (b)[ 9]*(a)[ 6] + (b)[10]*(a)[10] + (b)[11]*(a)[14], \
              (out)[11] = (b)[ 8]*(a)[ 3] + (b)[ 9]*(a)[ 7] + (b)[10]*(a)[11] + (b)[11]*(a)[15], \
              (out)[12] = (b)[12]*(a)[ 0] + (b)[13]*(a)[ 4] + (b)[14]*(a)[ 8] + (b)[15]*(a)[12], \
              (out)[13] = (b)[12]*(a)[ 1] + (b)[13]*(a)[ 5] + (b)[14]*(a)[ 9] + (b)[15]*(a)[13], \
              (out)[14] = (b)[12]*(a)[ 2] + (b)[13]*(a)[ 6] + (b)[14]*(a)[10] + (b)[15]*(a)[14], \
              (out)[15] = (b)[12]*(a)[ 3] + (b)[13]*(a)[ 7] + (b)[14]*(a)[11] + (b)[15]*(a)[15]  )

#define MatrixAffineInverse(A, D) ( \
              (D)[ 0] = (A)[ 0], (D)[ 4] = (A)[ 1], (D)[ 8] = (A)[ 2], \
              (D)[ 1] = (A)[ 4], (D)[ 5] = (A)[ 5], (D)[ 9] = (A)[ 6], \
              (D)[ 2] = (A)[ 8], (D)[ 6] = (A)[ 9], (D)[10] = (A)[10], \
              (D)[ 3] = 0, (D)[ 7] = 0, (D)[11] = 0, (D)[15] = 1,   \
              (D)[12] = -((A)[12]*(D)[ 0] + (A)[13]*(D)[ 4] + (A)[14]*(D)[ 8] ), \
              (D)[13] = -((A)[12]*(D)[ 1] + (A)[13]*(D)[ 5] + (A)[14]*(D)[ 9] ), \
              (D)[14] = -((A)[12]*(D)[ 2] + (A)[13]*(D)[ 6] + (A)[14]*(D)[10] ) )

#define MatrixSetupTranslation(A, x, y, z) (	\
              (A)[0] = 1, (A)[4] = 0, (A)[ 8] = 0, (A)[12] = (x), \
              (A)[1] = 0, (A)[5] = 1, (A)[ 9] = 0, (A)[13] = (y), \
              (A)[2] = 0, (A)[6] = 0, (A)[10] = 1, (A)[14] = (z), \
              (A)[3] = 0, (A)[7] = 0, (A)[11] = 0, (A)[15] = 1    )

#define MatrixSetupScale(A, x, y, z) (	\
              (A)[0]=(x), (A)[4]= 0,  (A)[ 8]= 0,  (A)[12]=0,	\
              (A)[1]= 0,  (A)[5]=(y), (A)[ 9]= 0,  (A)[13]=0, \
              (A)[2]= 0,  (A)[6]= 0,  (A)[10]=(z), (A)[14]=0, \
              (A)[3]= 0,  (A)[7]= 0,  (A)[11]= 0,  (A)[15]=1	)

#define MatrixMultiplyScale(A, x, y, z) (	\
              (A)[0] *= (x), (A)[4] *= (y), (A)[ 8] *= (z), \
              (A)[1] *= (x), (A)[5] *= (y), (A)[ 9] *= (z), \
              (A)[2] *= (x), (A)[6] *= (y), (A)[10] *= (z), \
              (A)[3] *= (x), (A)[7] *= (y), (A)[11] *= (z)	)

#define Mat4_SetupShear(A, x, y) ( \
              (A)[ 0] = 1,   (A)[ 4] = (x), (A)[ 8] = 0, (A)[12] = 0, \
              (A)[ 1] = (y), (A)[ 5] = 1,   (A)[ 9] = 0, (A)[13] = 0, \
              (A)[ 2] = 0,   (A)[ 6] = 0,   (A)[10] = 1, (A)[14] = 0, \
              (A)[ 3] = 0,   (A)[ 7] = 0,   (A)[11] = 0, (A)[15] = 1 )

#define Mat4_SetupTransform(A, forward, left, up, origin) ( \
              (A)[ 0]=(forward)[0], (A)[ 4]=(left)[0],    (A)[ 8]=(up)[0],  (A)[12]=(origin)[0], \
              (A)[ 1]=(forward)[1], (A)[ 5]=(left)[1],    (A)[ 9]=(up)[1],  (A)[13]=(origin)[1], \
              (A)[ 2]=(forward)[2], (A)[ 6]=(left)[2],    (A)[10]=(up)[2],  (A)[14]=(origin)[2], \
              (A)[ 3]=0,			  (A)[ 7]=0,            (A)[11]=0,        (A)[15]=1 )

#define MatrixSetupTransformFromRotation(A, rot, origin)  ( \
              (A)[ 0] = (rot)[ 0], (A)[ 4] = (rot)[ 4], (A)[ 8] = (rot)[ 8], (A)[12] = (origin)[0], \
              (A)[ 1] = (rot)[ 1], (A)[ 5] = (rot)[ 5], (A)[ 9] = (rot)[ 9], (A)[13] = (origin)[1], \
              (A)[ 2] = (rot)[ 2], (A)[ 6] = (rot)[ 6], (A)[10] = (rot)[10], (A)[14] = (origin)[2], \
              (A)[ 3] = 0,         (A)[ 7] = 0,         (A)[11] = 0,         (A)[15] = 1 )

#define Mat4_Quake2GlLeftMult(A, out) ( \
              (out)[ 0]=-(A)[ 8], (out)[ 1]=-(A)[ 9], (out)[2 ]=-(A)[10],  (out)[ 3]=-(A)[11], \
              (out)[ 4]=-(A)[ 0], (out)[ 5]=-(A)[ 1], (out)[6 ]=-(A)[ 2],  (out)[ 7]=-(A)[ 3], \
              (out)[ 8]= (A)[ 4], (out)[ 9]= (A)[ 5], (out)[10]= (A)[ 6],  (out)[11]= (A)[ 7], \
              (out)[12]= (A)[12], (out)[13]= (A)[13], (out)[14]= (A)[14],  (out)[15]= (A)[15] )

#define Mat4_Quake2GlRightMult(A, out) ( \
              (out)[ 0] = -(A)[ 1], (out)[ 1] =  (A)[ 2], (out)[2 ] = -(A)[ 0],  (out)[ 3] =  (A)[ 3], \
              (out)[ 4] = -(A)[ 5], (out)[ 5] =  (A)[ 6], (out)[6 ] = -(A)[ 4],  (out)[ 7] =  (A)[ 7], \
              (out)[ 8] = -(A)[ 9], (out)[ 9] =  (A)[10], (out)[10] = -(A)[ 8],  (out)[11] =  (A)[11], \
              (out)[12] = -(A)[13], (out)[13] =  (A)[14], (out)[14] = -(A)[12],  (out)[15] =  (A)[15]  )

#define Mat4_TransformNormal(A, in, out) ( \
              (out)[ 0] = (A)[ 0]*(in)[ 0] + (A)[ 4]*(in)[ 1] + (A)[ 8]*(in)[ 2], \
              (out)[ 1] = (A)[ 1]*(in)[ 0] + (A)[ 5]*(in)[ 1] + (A)[ 9]*(in)[ 2], \
              (out)[ 2] = (A)[ 2]*(in)[ 0] + (A)[ 6]*(in)[ 1] + (A)[10]*(in)[ 2] )

#define MatrixTransformPoint(A, in, out) ( \
              (out)[ 0] = (A)[ 0]*(in)[ 0] + (A)[ 4]*(in)[ 1] + (A)[ 8]*(in)[ 2] + (A)[12], \
              (out)[ 1] = (A)[ 1]*(in)[ 0] + (A)[ 5]*(in)[ 1] + (A)[ 9]*(in)[ 2] + (A)[13], \
              (out)[ 2] = (A)[ 2]*(in)[ 0] + (A)[ 6]*(in)[ 1] + (A)[10]*(in)[ 2] + (A)[14] )

#define MatrixSetupTransformFromVectorsFLU(A, forward, left, up, origin)   (                          \
              (A)[ 0] = (forward)[0], (A)[ 4] = (left)[0], (A)[ 8] = (up)[0], (A)[12] = (origin)[0], \
              (A)[ 1] = (forward)[1], (A)[ 5] = (left)[1], (A)[ 9] = (up)[1], (A)[13] = (origin)[1], \
              (A)[ 2] = (forward)[2], (A)[ 6] = (left)[2], (A)[10] = (up)[2], (A)[14] = (origin)[2], \
              (A)[ 3] = 0,            (A)[ 7] = 0,         (A)[11] = 0,       (A)[15] = 1 )
#endif

#endif //_Q_MATH_H_
