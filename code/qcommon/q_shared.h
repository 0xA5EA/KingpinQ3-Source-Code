/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
Copyright (C) 2011 Ari 'KRYPTYK' Mirles

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
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

//#ifndef BSPC //hypov8 add. below not in wolf
#include "q_mathsse.h"
//#endif
#define __STDC_FORMAT_MACROS
#ifndef Q_BIT
#define Q_BIT(x) (1 << (x))
#endif

//#define COMPAT_Q3A 1
//#define COMPAT_ET 1
//#defin COMPAT_KPQ3 1 //in build config

// this enables gametype single player
// for kpq3 disabled
//#define USE_GT_SINGLEPLAYER

// this enables weapon flamegun
// Now enabled -KRYPTYK
#define USE_FLAMEGUN //todo: repace with COMPAT_KPQ3

#define NO_SPEEDUP_MACROS

// uncomment this to use Kamikaze (not yet implemented properly)
//#define USE_KAMIKAZE

// comment out to enable bagman
//#define GT_BAGMAN_DISABLED

// enables harvestor and obelisk
//#define GT_USE_TA_TYPES


//#define USE_DDS_IMAGE
//#define COMPAT_Q3A 1

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#define PRODUCT_NAME            "KingpinQ3"
#define BASEGAME                "basekpq3"
#define CLIENT_WINDOW_TITLE     "KingpinQ3"
#define CLIENT_WINDOW_MIN_TITLE "Kpq3"
//#define HEARTBEAT_GAME          PRODUCT_NAME"-1"
#define CON_LOGFILE_NAME        "console.log"

//#define HEARTBEAT_FOR_MASTER    PRODUCT_NAME"-1"  //PORT_MASTER
#define HEARTBEAT_FOR_MASTER	"DarkPlaces" //using ioq3 new protocol. non Q3 game

#define GAMENAME_FOR_MASTER		PRODUCT_NAME "-1"		//"KingpinQ3-1" //HEARTBEAT_FOR_MASTER

//#define FLATLINE_FOR_MASTER		"KPQ3Flatline-1"	// HEARTBEAT_FOR_MASTER
#define FLATLINE_FOR_MASTER		HEARTBEAT_FOR_MASTER //hypo us DP name. more compatable

//#define HOMEPATH_NAME_WIN       PRODUCT_NAME
#define HOMEPATH_NAME_WIN       PRODUCT_NAME "\\_Beta2_0"//PATH_SEP
  //hypov8 move to new folder. beta1 config corrupting game.
  //todo: set config to check version number. reset if outdated.

#define KPQ3_BUILD_NUM "0.9.7.4" //hypov8 was PRODUCT_VERSION, conflict other libs
#define KPQ3_VERSION PRODUCT_NAME " " KPQ3_BUILD_NUM
//#define KPQ3_VERSION      "Wolf 1.41b-MP"


#define MAX_TEAMNAME 32

#define Q_UNUSED(x) (void)(x)

#ifdef _MSC_VER
#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic(memset, memcpy)
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#if (defined _MSC_VER)
#define Q_EXPORT __declspec(dllexport)
#elif (defined __SUNPRO_C)
#define Q_EXPORT __global
#elif ((__GNUC__ >= 3) && (!__EMX__) && (!sun))
#define Q_EXPORT __attribute__((visibility("default")))
#else
#define Q_EXPORT
#endif
/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifdef Q3_VM
# include "../game/bg_lib.h"
//typedef int intptr_t;
#else
# include <assert.h>
# include <math.h>
# include <stdio.h>
# include <stdarg.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>
# include <ctype.h>
# include <limits.h>

# ifdef  _MSC_VER
#   include <io.h>
#   ifndef _SDL_config_win32_h
      typedef __int64 int64_t;
      typedef __int32 int32_t;
      typedef __int16 int16_t;
      //typedef __int8 int8_t; //FIXME(0xA5EA): compile error under msvc
      typedef unsigned __int64 uint64_t;
      typedef unsigned __int32 uint32_t;
      typedef unsigned __int16 uint16_t;
      typedef unsigned __int8 uint8_t;
#   endif
  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
    size_t Q_vsnprintf(char *str, size_t size, const char *format, va_list ap);
# else
# include <stdint.h>
# define Q_vsnprintf vsnprintf
#endif
#endif

#ifdef _WIN32
#include <stdio.h>
#define Q_snprintf  _snprintf
#else
#define Q_snprintf  snprintf
#endif

#include "q_platform.h" //hypov8 linked elsewhere


//=============================================================

typedef int	qhandle_t;
typedef int	sfxHandle_t;
typedef int	fileHandle_t;
typedef int	clipHandle_t;

#define PAD(base, alignment)  (((base)+(alignment)-1) & ~((alignment)-1))
#define PADP(base, alignment) ((void *) PAD((intptr_t) (base), (alignment)))

#ifndef NULL
# define NULL                ((void *)0)
#endif
#define STRING(s)			       #s
// expand constants before stringifying them
#define XSTRING(s)		       STRING(s)
#define	MAX_QINT		         0x7fffffff
#define	MIN_QINT		         (-MAX_QINT-1)

// angle indexes
#define	PITCH				         0		// up / down
#define	YAW					         1		// left / right
#define	ROLL				         2		// fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	   1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	   1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		   1024	// max length of an individual token

#define	MAX_INFO_STRING		   1024
#define	MAX_INFO_KEY		     1024
#define	MAX_INFO_VALUE		   1024

#define	BIG_INFO_STRING		   8192  // used for system info key only
#define	BIG_INFO_KEY		     8192
#define	BIG_INFO_VALUE		   8192

#define	MAX_QPATH			       128 // 256	 ergibt funny lumpsize fehler
#ifdef PATH_MAX
# define MAX_OSPATH		  	   PATH_MAX
#else
# define	MAX_OSPATH			   256		// max length of a filesystem pathname
#endif
#define DEMOEXT              "dm_"        // standard demo extension
#define	MAX_NAME_LENGTH		    32		// max length of a client name
#define MAX_MASTER_SERVERS	     6		// 3 defined, 3 free
#define	MAX_SAY_TEXT	       150

//#define ARRAY_LEN(x)     (sizeof(x) / sizeof(*x))

// paramters for command buffer stuffing
typedef enum
{
  EXEC_NOW,     // don't return until completed, a VM should NEVER use this,
                // because some commands might cause the VM to be unloaded...
  EXEC_INSERT,  // insert at current position, but don't run yet
  EXEC_APPEND   // add to end of the command buffer (normal case)
} cbufExec_t;


// these aren't needed by any of the VMs.  put in another header?
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility

// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum
{
  PRINT_ALL,
  PRINT_DEVELOPER,		// only print when "developer 1"
  PRINT_WARNING,
  PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum
{
  ERR_FATAL,					// exit the entire game with a popup window
  ERR_DROP,					// print to console and disconnect from game
  ERR_SERVERDISCONNECT,		// don't kill server
  ERR_DISCONNECT,				// client disconnected from the server
  ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			     3
#define PROP_SPACE_WIDTH		     8
#define PROP_HEIGHT			      	27
#define PROP_SMALL_SIZE_SCALE	0.75

#define BLINK_DIVISOR		     	200
#define PULSE_DIVISOR			    75.f

#define UI_LEFT			  0x00000000	// default
#define UI_CENTER		  0x00000001
#define UI_RIGHT		  0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		  0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		  0x00004000

#if defined(_DEBUG) && !defined(BSPC)
#  define HUNK_DEBUG
#endif

typedef enum {
  h_high,
  h_low,
  h_dontcare
} ha_pref;

#ifdef HUNK_DEBUG
#  define Hunk_Alloc(size, preference)				Hunk_AllocDebug(size, preference, #size, __FILE__, __LINE__)
   void *Hunk_AllocDebug(int size, ha_pref preference, char const *label, char const *file, int line);
#else
#  ifndef BSPC //hypov8 
   void *Hunk_Alloc(int size, ha_pref preference);
#  else
   void *Hunk_Alloc(size_t size, ha_pref preference);
#  endif
#endif

#ifdef USE_ASM_LIB
# include "../asmlib/asmlib.h"
#endif

#ifndef Q3_VM
//FXIME: alligned alloc
# define Com_Allocate malloc
# define Com_Dealloc  free
#endif

#define CIN_system	  1
#define CIN_loop	    2
#define	CIN_hold    	4
#define CIN_silent	  8
#define CIN_shader	 16

// this is only here so the functions in q_shared.c and bg_*.c can link
void  QDECL Com_Error(int level, const char *error, ...) __attribute__ ((format (printf, 2, 3)));
void  QDECL Com_Printf(const char *msg, ...) __attribute__ ((format (printf, 1, 2)));


/*
==============================================================
MATHLIB
==============================================================
*/
#include "q_mathsse.h"
#ifdef __GNUC__
# define ALIGN(x) __attribute__((aligned(x)))
#else
# define ALIGN(x)
#endif




  //extern const matrix_t ALIGN16(matrixIdentity);

//#define NO_DEVELOPER_CMDS
#if 0 //ndef NO_DEVELOPER_CMDS
extern ALIGN16(const matrix_t Hilbert4); //hypov8 merge: const
extern ALIGN16(const matrix_t ASymetric4); //hypov8 merge: const
#endif

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#define MATRIX_INVERSE_EPSILON 1e-13f
#ifndef M_PI
#define M_PI 3.14159265358979323846f // matches value in gcc v2 math.h
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.414213562f
#endif

#ifndef M_ROOT3
#define M_ROOT3 1.732050808f
#endif

#define ARRAY_LEN(x) ( sizeof( x ) / sizeof( *( x ) ) )

// angle indexes
#define PITCH 0 // up / down
#define YAW   1 // left / right
#define ROLL  2 // fall over

#define NUMVERTEXNORMALS	162
extern	const vec3_t bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		  640
#define	SCREEN_HEIGHT		  480

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define BIGCHAR_WIDTH		  16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

extern	vec4_t	colorBlack;
extern	vec4_t	colorRed;
extern	vec4_t	colorGreen;
extern	vec4_t	colorBlue;
extern	vec4_t	colorYellow;
extern	vec4_t	colorMagenta;
extern	vec4_t	colorCyan;
extern	vec4_t	colorWhite;
extern	vec4_t	colorLtGrey;
extern	vec4_t	colorMdGrey;
extern	vec4_t	colorDkGrey;

extern	vec4_t	colortmbBrown;
extern	vec4_t	colortmBlue;
extern	vec4_t	colortmGreen;
extern  vec4_t	colortmgreenyellow;
extern  vec4_t	colortmRed;

#define Q_COLOR_ESCAPE	'^'
//#define Q_IsColorString(p)	((p) && *(p) == Q_COLOR_ESCAPE && *((p)+1) && isalnum(*((p)+1))) // ^[0-9a-zA-Z]

#define COLOR_BLACK		   '0'
#define COLOR_RED		     '1'
#define COLOR_GREEN		   '2'
#define COLOR_YELLOW	   '3'
#define COLOR_BLUE		   '4'
#define COLOR_CYAN		   '5'
#define COLOR_MAGENTA	   '6'
#define COLOR_WHITE		   '7'
#define ColorIndex(c)	   (((c) - '0') & 0x07)

#define S_COLOR_BLACK   "^0"
#define S_COLOR_RED		  "^1"
#define S_COLOR_GREEN   "^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	  "^4"
#define S_COLOR_CYAN	  "^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	  "^7"


STATIC_INLINE qboolean Q_IsColorString( const char *p ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
{
  return ( p[0] == Q_COLOR_ESCAPE &&
           ( /*p[1] == COLOR_NULL ||*/ ( p[1] >= '0' && p[1] != Q_COLOR_ESCAPE && p[1] < 'p' ) ) //hypov8 merge: null??
         ) ? qtrue : qfalse;
}
#endif

#define INDENT_MARKER    '\v'
extern const vec4_t g_color_table[8];

#define	MAKERGB(v, r, g, b) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA(v, r, g, b, a) v[0]=r;v[1]=g;v[2]=b;v[3]=a
// Hex Color string support
#define gethex( ch )                  ( ( ch ) > '9' ? ( ( ch ) >= 'a' ? ( ( ch ) - 'a' + 10 ) : ( ( ch ) - '7' ) ) : ( ( ch ) - '0' ) )
#define ishex( ch )                   ( ( ch ) && ( ( ( ch ) >= '0' && ( ch ) <= '9' ) || ( ( ch ) >= 'A' && ( ch ) <= 'F' ) || ( ( ch ) >= 'a' && ( ch ) <= 'f' ) ) )
// check whether in the rrggbb format, r,g,b e {0,...,9} U {A,...,F}
#define Q_IsHexColorString( p )       ( ishex( *( p ) ) && ishex( *( ( p ) + 1 ) ) && ishex( *( ( p ) + 2 ) ) && ishex( *( ( p ) + 3 ) ) && ishex( *( ( p ) + 4 ) ) && ishex( *( ( p ) + 5 ) ) )
#define Q_HexColorStringHasAlpha( p ) ( ishex( *( ( p ) + 6 ) ) && ishex( *( ( p ) + 7 ) ) )

//#ifdef __cplusplus)
#define DEG2RAD( a )                  ( ( ( a ) * M_PI ) / 180.0F )
#define RAD2DEG( a )                  ( ( ( a ) * 180.0f ) / M_PI )
//#else
#define M_DEG2RAD	(M_PI/180.f)
#define M_RAD2DEG	(180.f/M_PI)
//#endif

#ifdef Q3_VM
#define Q_clamp( a, b, c )            ( ( b ) >= ( c ) ? ( b ) : ( a ) < ( b ) ? ( b ) : ( a ) > ( c ) ? ( c ) : ( a ) )
#else
#define Q_clamp( a, b, c )            Maths::clamp( (a), (b), (c) )
#endif

#define Q_FLT_EPSILON 1.19209290e-7
int Com_HashKey(char *string, int maxlength);
struct cplane_s;

extern vec3_t vec3_origin;
extern vec3_t axisDefault[3];
extern matrix_t matrixIdentity;
extern quat_t   quatIdentity;

#define	nanmask (255<<23)

#define IS_NAN( x ) ( ( ( *(int *)&( x ) ) & nanmask ) == nanmask )

int Q_isnan(float x);

void Q_SnapVectorStd(vec3_t vec);

#if idx64
#if defined (__cplusplus)
extern "C" {
#endif
  extern  long QDECL qftolsse(float f);
  extern int QDECL qvmftolsse(void);
  extern  void QDECL qsnapvectorsse(vec3_t vec);
#if defined (__cplusplus)
}
#endif
  #define Q_ftol qftolsse
#define Q_SnapVector Q_SnapVectorStd
//FIXME(0xA5EA): this should be qsnapvectorsse but for unknown reason it leads to segmentation fault
  extern int (*Q_VMftol)(void);
#elif id386
#if defined (__cplusplus)
extern "C" {
#endif
  extern long QDECL qftolx87(float f);
  extern long QDECL qftolsse(float f);
  extern int QDECL qvmftolx87(void);
  extern int QDECL qvmftolsse(void);
  extern void QDECL qsnapvectorx87(vec3_t vec);
  extern void QDECL qsnapvectorsse(vec3_t vec);
#if defined (__cplusplus)
}
#endif

  extern long (QDECL *Q_ftol)(float f);
  extern int (QDECL *Q_VMftol)(void);
  extern void (QDECL *Q_SnapVector)(vec3_t vec);
#else
  #define Q_ftol(f) lrintf((f))
  #define Q_SnapVector(vec)\
  do\
  {\
    vec3_t *temp = (vec);\
    \
    (*temp)[0] = round((*temp)[0]);\
    (*temp)[1] = round((*temp)[1]);\
    (*temp)[2] = round((*temp)[2]);\
  } while(0)
#endif
#if defined (__cplusplus) && !defined(BSPC) //hypov8 :: error
  template <typename T> static ID_INLINE T Q_max(T a, T b) { return ((a) > (b) ? (a) : (b)) ; }
  template <typename T> static ID_INLINE T Q_min(T a, T b) { return ((a) < (b) ? (a) : (b)) ; }
 // template <typename T> static ID_INLINE T Q_bound(T a, T b, T c) { return (Q_max(a, Q_min(b, c))); }

#include <algorithm>
  namespace Maths
  {
    template<typename T> static inline T clamp(T value, T min, T max)
    {
      // if min > max, use min instead of max
      return std::max(min, std::min(std::max(min, max), value));
    }

    static inline float clampFraction(float value)
    {
      return clamp(value, 0.0f, 1.0f);
    }

    static inline double clampFraction(double value)
    {
      return clamp(value, 0.0, 1.0);
    }
  }
#else
#define Q_min(a, b)      ((a) < (b) ? (a) : (b))
#define Q_max(a, b)      ((a) > (b) ? (a) : (b))
//#define Q_bound(a, b, c) (Q_max(a, Q_min(b, c)))

#endif
  STATIC_INLINE long XreaL_Q_ftol( float f ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    return ( long ) f;
  }
#endif
  STATIC_INLINE unsigned int Q_floatBitsToUint( float number ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    floatint_t t;

    t.f = number;
    return t.ui;
  }
#endif

  STATIC_INLINE float Q_uintBitsToFloat( unsigned int number ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    floatint_t t;

    t.ui = number;
    return t.f;
  }
#endif

  STATIC_INLINE float Q_rsqrt( float number ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    float x = 0.5f * number;
    float y;

    Q_UNUSED(x);

    // compute approximate inverse square root
#if defined( idx86_sse )
    _mm_store_ss( &y, _mm_rsqrt_ss( _mm_load_ss( &number ) ) );
#elif idppc

#ifdef __GNUC__
    asm( "frsqrte %0, %1" : "=f"( y ) : "f"( number ) );
#else
    y = __frsqrte( number );
#endif
#else
    y = Q_uintBitsToFloat( 0x5f3759df - (Q_floatBitsToUint( number ) >> 1) );
    y *= ( 1.5f - ( x * y * y ) ); // initial iteration
#endif
#ifdef RSQRT_PRECISE
    y *= ( 1.5f - ( x * y * y ) ); // second iteration for higher precision
#endif
    return y;
  }
#endif

STATIC_INLINE float Q_fabs( float x ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
{
#ifndef Q3_VM
  return fabsf( x );
#else
  floatint_t tmp;

  tmp.f = x;
  tmp.i &= 0x7FFFFFFF;
  return tmp.f;
#endif
}
#endif

#define Q_recip(x) ( 1.0f / (x) )

byte         ClampByte( int i );
signed char ClampChar(int i);
signed short ClampShort(int i);

//#endif // Q3_VM

// this isn't a real cheap function to call!
int DirToByte(vec3_t dir);
void ByteToDir(int b, vec3_t dir);

#if 1

#define DotProduct( x,y )            ( ( x )[ 0 ] * ( y )[ 0 ] + ( x )[ 1 ] * ( y )[ 1 ] + ( x )[ 2 ] * ( y )[ 2 ] )
#define VectorSubtract( a,b,c )      ( ( c )[ 0 ] = ( a )[ 0 ] - ( b )[ 0 ],( c )[ 1 ] = ( a )[ 1 ] - ( b )[ 1 ],( c )[ 2 ] = ( a )[ 2 ] - ( b )[ 2 ] )
#define VectorAdd( a,b,c )           ( ( c )[ 0 ] = ( a )[ 0 ] + ( b )[ 0 ],( c )[ 1 ] = ( a )[ 1 ] + ( b )[ 1 ],( c )[ 2 ] = ( a )[ 2 ] + ( b )[ 2 ] )
#define VectorCopy( a,b )            ( ( b )[ 0 ] = ( a )[ 0 ],( b )[ 1 ] = ( a )[ 1 ],( b )[ 2 ] = ( a )[ 2 ] )
#define VectorScale( v, s, o )       ( ( o )[ 0 ] = ( v )[ 0 ] * ( s ),( o )[ 1 ] = ( v )[ 1 ] * ( s ),( o )[ 2 ] = ( v )[ 2 ] * ( s ) )
#define VectorMA( v, s, b, o )       (( o )[ 0 ] = ( v )[ 0 ] + ( b )[ 0 ] * ( s ), \
                                      ( o )[ 1 ] = ( v )[ 1 ] + ( b )[ 1 ] * ( s ), \
                                      ( o )[ 2 ] = ( v )[ 2 ] + ( b )[ 2 ] * ( s ))
#define VectorLerpTrem( f, s, e, r ) (( r )[ 0 ] = ( s )[ 0 ] + ( f ) * (( e )[ 0 ] - ( s )[ 0 ] ), \
                                      ( r )[ 1 ] = ( s )[ 1 ] + ( f ) * (( e )[ 1 ] - ( s )[ 1 ] ), \
                                      ( r )[ 2 ] = ( s )[ 2 ] + ( f ) * (( e )[ 2 ] - ( s )[ 2 ] ))

#else

#define DotProduct( x,y )       _DotProduct( x,y )
#define VectorSubtract( a,b,c ) _VectorSubtract( a,b,c )
#define VectorAdd( a,b,c )      _VectorAdd( a,b,c )
#define VectorCopy( a,b )       _VectorCopy( a,b )
#define VectorScale( v, s, o )  _VectorScale( v,s,o )
#define VectorMA( v, s, b, o )  _VectorMA( v,s,b,o )

#endif

#define VectorClear( a )             ( ( a )[ 0 ] = ( a )[ 1 ] = ( a )[ 2 ] = 0 )
#define VectorNegate( a,b )          ( ( b )[ 0 ] = -( a )[ 0 ],( b )[ 1 ] = -( a )[ 1 ],( b )[ 2 ] = -( a )[ 2 ] )
#define VectorSet( v, x, y, z )      ( ( v )[ 0 ] = ( x ), ( v )[ 1 ] = ( y ), ( v )[ 2 ] = ( z ) )

#define Vector2Set( v, x, y )        ( ( v )[ 0 ] = ( x ),( v )[ 1 ] = ( y ) )
#define Vector2Copy( a,b )           ( ( b )[ 0 ] = ( a )[ 0 ],( b )[ 1 ] = ( a )[ 1 ] )
#define Vector2Subtract( a,b,c )     ( ( c )[ 0 ] = ( a )[ 0 ] - ( b )[ 0 ],( c )[ 1 ] = ( a )[ 1 ] - ( b )[ 1 ] )

//hypov8 kpq3
#define Vec4_Add(a,b,c)			    ((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define Vec4_Subtract(a,b,c)	  ((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
//

#define Vector4Set( v, x, y, z, n )  ( ( v )[ 0 ] = ( x ),( v )[ 1 ] = ( y ),( v )[ 2 ] = ( z ),( v )[ 3 ] = ( n ) )
#define Vector4Copy( a,b )           ( ( b )[ 0 ] = ( a )[ 0 ],( b )[ 1 ] = ( a )[ 1 ],( b )[ 2 ] = ( a )[ 2 ],( b )[ 3 ] = ( a )[ 3 ] )
#define Vector4MA( v, s, b, o )      ( ( o )[ 0 ] = ( v )[ 0 ] + ( b )[ 0 ] * ( s ),( o )[ 1 ] = ( v )[ 1 ] + ( b )[ 1 ] * ( s ),( o )[ 2 ] = ( v )[ 2 ] + ( b )[ 2 ] * ( s ),( o )[ 3 ] = ( v )[ 3 ] + ( b )[ 3 ] * ( s ) )
#define Vector4Average( v, b, s, o ) ( ( o )[ 0 ] = ( ( v )[ 0 ] * ( 1 - ( s ) ) ) + ( ( b )[ 0 ] * ( s ) ),( o )[ 1 ] = ( ( v )[ 1 ] * ( 1 - ( s ) ) ) + ( ( b )[ 1 ] * ( s ) ),( o )[ 2 ] = ( ( v )[ 2 ] * ( 1 - ( s ) ) ) + ( ( b )[ 2 ] * ( s ) ),( o )[ 3 ] = ( ( v )[ 3 ] * ( 1 - ( s ) ) ) + ( ( b )[ 3 ] * ( s ) ) )
#define Vector4Lerp( f, s, e, r )    (( r )[ 0 ] = ( s )[ 0 ] + ( f ) * (( e )[ 0 ] - ( s )[ 0 ] ), \
                                      ( r )[ 1 ] = ( s )[ 1 ] + ( f ) * (( e )[ 1 ] - ( s )[ 1 ] ), \
                                      ( r )[ 2 ] = ( s )[ 2 ] + ( f ) * (( e )[ 2 ] - ( s )[ 2 ] ), \
                                      ( r )[ 3 ] = ( s )[ 3 ] + ( f ) * (( e )[ 3 ] - ( s )[ 3 ] ))

#define DotProduct4(x, y)            (( x )[ 0 ] * ( y )[ 0 ] + ( x )[ 1 ] * ( y )[ 1 ] + ( x )[ 2 ] * ( y )[ 2 ] + ( x )[ 3 ] * ( y )[ 3 ] )

#define SnapVector( v )              do { ( v )[ 0 ] = ( floor( ( v )[ 0 ] + 0.5f ) ); ( v )[ 1 ] = ( floor( ( v )[ 1 ] + 0.5f ) ); ( v )[ 2 ] = ( floor( ( v )[ 2 ] + 0.5f ) ); } while ( 0 )


#define AxisClear(a)    ((a)[0][0] = 1, (a)[0][1] = 0, (a)[0][2] = 0,   \
                          (a)[1][0] = 0, (a)[1][1] = 1, (a)[1][2] = 0,   \
                          (a)[2][0] = 0, (a)[2][1] = 0, (a)[2][2] = 1    )

#define AxisCopy(a, b)  (VectorCopy((a)[0], (b)[0]),                     \
                          VectorCopy((a)[1], (b)[1]),                     \
                          VectorCopy((a)[2], (b)[2])                      )

#define Mat2_Multiply(A, B, out) (  (out)[0] = (A)[0]*(B)[0] + (A)[1]*(B)[2], \
                                    (out)[1] = (A)[0]*(B)[1] + (A)[1]*(B)[3], \
                                    (out)[2] = (A)[2]*(B)[0] + (A)[3]*(B)[2], \
                                    (out)[3] = (A)[2]*(B)[1] + (A)[3]*(B)[3]  )

#define VectorInverse(v) ( (v)[0] = -(v)[0], \
                           (v)[1] = -(v)[1], \
                           (v)[2] = -(v)[2] )


//unlagged - attack prediction #3
// moved from g_weapon.c
  void SnapVectorTowards( vec3_t v, vec3_t to );
//unlagged - attack prediction #3


// just in case you don't want to use the macros
  vec_t    _DotProduct( const vec3_t v1, const vec3_t v2 );
  void     _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
  void     _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
  void     _VectorCopy( const vec3_t in, vec3_t out );
  void     _VectorScale( const vec3_t in, float scale, vec3_t out );
  void     _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );


  unsigned ColorBytes3 (float r, float g, float b);
  unsigned ColorBytes4 (float r, float g, float b, float a);


//FIXME (0xA5EA): brakes aas build ?
  float NormalizeColor(const vec3_t in, vec3_t out);

  void  ClampColor(vec4_t color);

  float RadiusFromBounds(const vec3_t mins, const vec3_t maxs);
  void     ZeroBounds( vec3_t mins, vec3_t maxs );
  void ClearBounds(vec3_t mins, vec3_t maxs);
  void     AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

  void VectorReflect(vec3_t const v, vec3_t const normal, vec3_t out);
// RB: same as BoundsIntersectPoint but kept for compatibility
  qboolean PointInBounds( const vec3_t v, const vec3_t mins, const vec3_t maxs );

  void     BoundsAdd( vec3_t mins, vec3_t maxs, const vec3_t mins2, const vec3_t maxs2 );
  qboolean BoundsIntersect( const vec3_t mins, const vec3_t maxs, const vec3_t mins2, const vec3_t maxs2 );
  qboolean BoundsIntersectSphere( const vec3_t mins, const vec3_t maxs, const vec3_t origin, vec_t radius );
  qboolean BoundsIntersectPoint( const vec3_t mins, const vec3_t maxs, const vec3_t origin );
  STATIC_INLINE void BoundsToCorners( const vec3_t mins, const vec3_t maxs, vec3_t corners[ 8 ] ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    VectorSet( corners[ 0 ], mins[ 0 ], maxs[ 1 ], maxs[ 2 ] );
    VectorSet( corners[ 1 ], maxs[ 0 ], maxs[ 1 ], maxs[ 2 ] );
    VectorSet( corners[ 2 ], maxs[ 0 ], mins[ 1 ], maxs[ 2 ] );
    VectorSet( corners[ 3 ], mins[ 0 ], mins[ 1 ], maxs[ 2 ] );
    VectorSet( corners[ 4 ], mins[ 0 ], maxs[ 1 ], mins[ 2 ] );
    VectorSet( corners[ 5 ], maxs[ 0 ], maxs[ 1 ], mins[ 2 ] );
    VectorSet( corners[ 6 ], maxs[ 0 ], mins[ 1 ], mins[ 2 ] );
    VectorSet( corners[ 7 ], mins[ 0 ], mins[ 1 ], mins[ 2 ] );
  }
#endif


  int VectorCompare( const vec3_t v1, const vec3_t v2 );
  STATIC_INLINE int Vector4Compare( const vec4_t v1, const vec4_t v2 ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    if ( v1[ 0 ] != v2[ 0 ] || v1[ 1 ] != v2[ 1 ] || v1[ 2 ] != v2[ 2 ] || v1[ 3 ] != v2[ 3 ] )
    {
      return 0;
    }

    return 1;
  }
#endif
  STATIC_INLINE void VectorLerp( const vec3_t from, const vec3_t to, float frac, vec3_t out ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    out[ 0 ] = from[ 0 ] + ( ( to[ 0 ] - from[ 0 ] ) * frac );
    out[ 1 ] = from[ 1 ] + ( ( to[ 1 ] - from[ 1 ] ) * frac );
    out[ 2 ] = from[ 2 ] + ( ( to[ 2 ] - from[ 2 ] ) * frac );
  }
#endif
  STATIC_INLINE int VectorCompareEpsilon( const vec3_t v1, const vec3_t v2, float epsilon ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    vec3_t d;

    VectorSubtract( v1, v2, d );
    d[ 0 ] = fabs( d[ 0 ] );
    d[ 1 ] = fabs( d[ 1 ] );
    d[ 2 ] = fabs( d[ 2 ] );

    if ( d[ 0 ] > epsilon || d[ 1 ] > epsilon || d[ 2 ] > epsilon )
    {
      return 0;
    }

    return 1;
  }
#endif

//used for unvan md5
  STATIC_INLINE void VectorMin(const vec3_t a, const vec3_t b, vec3_t out) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    out[0] = a[0] < b[0] ? a[0] : b[0];
    out[1] = a[1] < b[1] ? a[1] : b[1];
    out[2] = a[2] < b[2] ? a[2] : b[2];
  }
#endif
  STATIC_INLINE void VectorMax(const vec3_t a, const vec3_t b, vec3_t out) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    out[0] = a[0] > b[0] ? a[0] : b[0];
    out[1] = a[1] > b[1] ? a[1] : b[1];
    out[2] = a[2] > b[2] ? a[2] : b[2];
  }
#endif


//int VectorCompare(const vec3_t v1, const vec3_t v2);
vec_t VectorLength(const vec3_t v);
vec_t VectorLengthSquared(const vec3_t v);
vec_t Distance(const vec3_t p1, const vec3_t p2);
vec_t DistanceSquared(const vec3_t p1, const vec3_t p2);
void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);
vec_t VectorNormalizeSelfstd(vec3_t v);		// returns vector length
void VectorNormalizeFaststd(vec3_t v);
vec_t VectorNormalize2std(const vec3_t v, vec3_t out);
///////////////void VectorInverse(vec3_t v); //hypo

vec_t	K_tan(vec_t alpha);




// FIXME: 0xA5EA, passt das mit dem QVm define ?

#if 0 //def USING_SSE_MATH
  #define VectorNormalize     VectorNormalizeSelfSSE
  #define VectorNormalize2    VectorNormalize2SSE
  #define VectorNormalize3    VectorNormalizeSelf2SSE
  #define VectorNormalizeFast VectorNormalizeSelfSSE
#else
  vec_t VectorNormalize( vec3_t v );       // returns vector length
  void VectorNormalizeFast( vec3_t v );     // does NOT return vector length, uses rsqrt approximation
  vec_t VectorNormalize2( const vec3_t v, vec3_t out );
  //#  define VectorNormalize     VectorNormalizeSelfstd
  //#  define VectorNormalize2    VectorNormalize2std
  #define VectorNormalize3     VectorNormalize //VectorNormalizeSelfstd
  //#  define VectorNormalizeFast VectorNormalizeFaststd
#endif

  void Vector4Scale(const vec4_t in, vec_t scale, vec4_t out);
  void VectorRotate(vec3_t in, vec3_t matrix[3], vec3_t out);

  int   NearestPowerOfTwo( int val );
  int   Q_log2(int val);
  float Q_acos(float c);
  int   Q_rand(int *seed);
  float Q_random(int *seed);
  float Q_crandom(int *seed);

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

  void VectorToAngles(const vec3_t value1, vec3_t angles);
  //float vectoyaw( const vec3_t vec ); //hypov8 merge: game dll
  void AnglesToAxis(const vec3_t angles, vec3_t axis[3]);
// TTimo: const vec_t ** would require explicit casts for ANSI C conformance
// see unix/const-arg.c
  void  AxisToAngles( /*const*/ vec3_t axis[ 3 ], vec3_t angles );
  //void AxisToAngles ( const vec3_t axis[3], vec3_t angles );
  float VectorDistance( vec3_t v1, vec3_t v2 );
  float VectorDistanceSquared( vec3_t v1, vec3_t v2 );

  float VectorMinComponent( vec3_t v );
  float VectorMaxComponent( vec3_t v );

#if 0
//0xA5EA, use macros instead of functions, clean this up some time
  void AxisClear(vec3_t axis[3]);
  void AxisCopy(vec3_t in[3], vec3_t out[3]);
#endif
  void SetPlaneSignbits(struct cplane_s *out);

  float BoundsMaxExtent( const vec3_t mins, const vec3_t maxs ); //hypov8 merge: unvan .50

  float	AngleMod(float a);
  float	LerpAngle(float from, float to, float frac);
  void  LerpPosition( vec3_t start, vec3_t end, float frac, vec3_t out );
  float AngleSubtract( float a1, float a2 );
  void  AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );
  
  float AngleNormalize2Pi( float angle );
  float AngleNormalize360(float angle);
  float AngleNormalize180(float angle);
  float AngleDelta(float angle1, float angle2);
  float AngleBetweenVectors( const vec3_t a, const vec3_t b );
  void  AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );

vec_t PlaneNormalize( vec4_t plane );  // returns normal length

/* greebo: This calculates the intersection point of three planes.
 * Returns <0,0,0> if no intersection point could be found, otherwise returns the coordinates of the intersection point
 * (this may also be 0,0,0) 
*/
  qboolean PlanesGetIntersectionPoint( const vec4_t plane1, const vec4_t plane2, const vec4_t plane3, vec3_t out );
  void     PlaneIntersectRay( const vec3_t rayPos, const vec3_t rayDir, const vec4_t plane, vec3_t res );


  qboolean PlaneFromPoints(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c);
  qboolean PlaneFromPointsOrder(vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c, qboolean cw);
  void     ProjectPointOnPlane(vec3_t dst, const vec3_t point, const vec3_t normal);
  void     RotatePointAroundVector(vec3_t dst, const vec3_t dir, const vec3_t point, float degrees);
  void     RotatePointAroundVertex( vec3_t pnt, float rot_x, float rot_y, float rot_z, const vec3_t origin );
  void     RotateAroundDirection(vec3_t axis[3], float yaw);
  void     MakeNormalVectors(const vec3_t forward, vec3_t right, vec3_t up);

// perpendicular vector could be replaced by this

  void VectorMatrixMultiply( const vec3_t p, vec3_t m[ 3 ], vec3_t out );
// RB: NOTE renamed MatrixMultiply to AxisMultiply because it conflicts with most new matrix functions
// It is important for mod developers to do this change as well or they risk a memory corruption by using
// the other MatrixMultiply function.
  void AxisMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
  void  PerpendicularVector( vec3_t dst, const vec3_t src );

// Ridah
  void  GetPerpendicularViewVector( const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up );
  void  ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj );
  void  ProjectPointOntoVectorBounded( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj );
  float DistanceFromLineSquared( vec3_t p, vec3_t lp1, vec3_t lp2 );
  float DistanceFromVectorSquared( vec3_t p, vec3_t lp1, vec3_t lp2 );

// done.

  vec_t DistanceBetweenLineSegmentsSquared( const vec3_t sP0, const vec3_t sP1,
      const vec3_t tP0, const vec3_t tP1, float *s, float *t );
  vec_t DistanceBetweenLineSegments( const vec3_t sP0, const vec3_t sP1,
                                     const vec3_t tP0, const vec3_t tP1, float *s, float *t );

//=============================================

// RB: XreaL matrix math functions required by the renderer

  void     MatrixIdentity( matrix_t m );
  void     MatrixClear( matrix_t m );
  void     MatrixCopy( const matrix_t in, matrix_t out );
  qboolean MatrixCompare( const matrix_t a, const matrix_t b );
  void     MatrixTransposeIntoXMM( const matrix_t m );
  void     MatrixTranspose( const matrix_t in, matrix_t out );

// invert any m4x4 using Kramer's rule.. return qtrue if matrix is singular, else return qfalse
  qboolean MatrixInverse( matrix_t m );
  void     MatrixSetupXRotation( matrix_t m, vec_t degrees );
  void     MatrixSetupYRotation( matrix_t m, vec_t degrees );
  void     MatrixSetupZRotation( matrix_t m, vec_t degrees );
  void     MatrixSetupTranslation( matrix_t m, vec_t x, vec_t y, vec_t z );
  void     MatrixSetupScale( matrix_t m, vec_t x, vec_t y, vec_t z );
  void     MatrixSetupShear( matrix_t m, vec_t x, vec_t y );
  void     MatrixMultiply( const matrix_t a, const matrix_t b, matrix_t out );
  void     MatrixMultiply2( matrix_t m, const matrix_t m2 );
  void     MatrixMultiplyRotation( matrix_t m, vec_t pitch, vec_t yaw, vec_t roll );
  void     MatrixMultiplyZRotation( matrix_t m, vec_t degrees );
  void     MatrixMultiplyTranslation( matrix_t m, vec_t x, vec_t y, vec_t z );
  void     MatrixMultiplyScale( matrix_t m, vec_t x, vec_t y, vec_t z );
  void     MatrixMultiplyShear( matrix_t m, vec_t x, vec_t y );
  void     MatrixToAngles( const matrix_t m, vec3_t angles );
  void     MatrixFromAngles( matrix_t m, vec_t pitch, vec_t yaw, vec_t roll );
  void     MatrixFromVectorsFLU( matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up );
  void     MatrixFromVectorsFRU( matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up );
  void     MatrixFromQuat( matrix_t m, const quat_t q );
  void     MatrixFromPlanes( matrix_t m, const vec4_t left, const vec4_t right, const vec4_t bottom, const vec4_t top,
                             const vec4_t near, const vec4_t far );
  void     MatrixToVectorsFLU( const matrix_t m, vec3_t forward, vec3_t left, vec3_t up );
  void     MatrixToVectorsFRU( const matrix_t m, vec3_t forward, vec3_t right, vec3_t up );
  void     MatrixSetupTransformFromVectorsFLU( matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin );
  void     MatrixSetupTransformFromVectorsFRU( matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up, const vec3_t origin );
  void     MatrixSetupTransformFromRotation( matrix_t m, const matrix_t rot, const vec3_t origin );
  void     MatrixSetupTransformFromQuat( matrix_t m, const quat_t quat, const vec3_t origin );
  void     MatrixAffineInverse( const matrix_t in, matrix_t out );
  void     MatrixTransformNormal( const matrix_t m, const vec3_t in, vec3_t out );
  void     MatrixTransformNormal2( const matrix_t m, vec3_t inout );
  void     MatrixTransformPoint( const matrix_t m, const vec3_t in, vec3_t out );
  void     MatrixTransformPoint2( const matrix_t m, vec3_t inout );
  void     MatrixTransform4( const matrix_t m, const vec4_t in, vec4_t out );
  void     MatrixTransformPlane( const matrix_t m, const vec4_t in, vec4_t out );
  void     MatrixTransformPlane2( const matrix_t m, vec4_t inout );
  void     MatrixTransformBounds( const matrix_t m, const vec3_t mins, const vec3_t maxs, vec3_t omins, vec3_t omaxs );
  void     MatrixPerspectiveProjection( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionLH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionRH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionFovYAspectLH( matrix_t m, vec_t fov, vec_t aspect, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionFovXYLH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionFovXYRH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far );
  void     MatrixPerspectiveProjectionFovXYInfiniteRH( matrix_t m, vec_t fovX, vec_t fovY, vec_t near );
  void     MatrixOrthogonalProjection( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );

  void     MatrixOrthogonalProjectionLH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );
  void     MatrixOrthogonalProjectionRH( matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far );

  void     MatrixPlaneReflection( matrix_t m, const vec4_t plane );

  void     MatrixLookAtLH( matrix_t output, const vec3_t pos, const vec3_t dir, const vec3_t up );
  void     MatrixLookAtRH( matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up );
  void     MatrixScaleTranslateToUnitCube( matrix_t m, const vec3_t mins, const vec3_t maxs );
  void     MatrixCrop( matrix_t m, const vec3_t mins, const vec3_t maxs );

  STATIC_INLINE void AnglesToMatrix( const vec3_t angles, matrix_t m ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    MatrixFromAngles( m, angles[ PITCH ], angles[ YAW ], angles[ ROLL ] );
  }
#endif


//void VectorToAngles(const vec3_t value1, vec3_t angles); //hypov8 not used?


//=============================================
#if 1 //hypov8 merge:
//=============================================

// RB: XreaL quaternion math functions required by the renderer

#define QuatSet(q,x,y,z,w) (( q )[ 0 ] = ( x ),( q )[ 1 ] = ( y ),( q )[ 2 ] = ( z ),( q )[ 3 ] = ( w ))
#define QuatCopy(a,b)      (( b )[ 0 ] = ( a )[ 0 ],( b )[ 1 ] = ( a )[ 1 ],( b )[ 2 ] = ( a )[ 2 ],( b )[ 3 ] = ( a )[ 3 ] )

#define QuatCompare(a,b)   (( a )[ 0 ] == ( b )[ 0 ] && ( a )[ 1 ] == ( b )[ 1 ] && ( a )[ 2 ] == ( b )[ 2 ] && ( a )[ 3 ] == ( b )[ 3 ] )

  STATIC_INLINE void QuatClear( quat_t q ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    q[ 0 ] = 0;
    q[ 1 ] = 0;
    q[ 2 ] = 0;
    q[ 3 ] = 1;
  }
#endif

  STATIC_INLINE void QuatZero( quat_t o ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    o[ 0 ] = 0.0f;
    o[ 1 ] = 0.0f;
    o[ 2 ] = 0.0f;
    o[ 3 ] = 0.0f;
  }
#endif

  STATIC_INLINE void QuatAdd( const quat_t p, const quat_t q,
            quat_t o ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    o[ 0 ] = p[ 0 ] + q[ 0 ];
    o[ 1 ] = p[ 1 ] + q[ 1 ];
    o[ 2 ] = p[ 2 ] + q[ 2 ];
    o[ 3 ] = p[ 3 ] + q[ 3 ];
  }
#endif

  STATIC_INLINE void QuatMA( const quat_t p, float f, const quat_t q,
           quat_t o ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    o[ 0 ] = p[ 0 ] + f * q[ 0 ];
    o[ 1 ] = p[ 1 ] + f * q[ 1 ];
    o[ 2 ] = p[ 2 ] + f * q[ 2 ];
    o[ 3 ] = p[ 3 ] + f * q[ 3 ];
  }
#endif

  /*
  STATIC_INLINE int QuatCompare(const quat_t a, const quat_t b) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
          if(a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3])
          {
                  return 0;
          }
          return 1;
  }
#endif
  */

  STATIC_INLINE void QuatCalcW( quat_t q ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
#if 1
    vec_t term = 1.0f - ( q[ 0 ] * q[ 0 ] + q[ 1 ] * q[ 1 ] + q[ 2 ] * q[ 2 ] );

    if ( term < 0.0 )
    {
      q[ 3 ] = 0.0;
    }
    else
    {
      q[ 3 ] = -sqrt( term );
    }

#else
    q[ 3 ] = sqrt( fabs( 1.0f - ( q[ 0 ] * q[ 0 ] + q[ 1 ] * q[ 1 ] + q[ 2 ] * q[ 2 ] ) ) );
#endif
  }
#endif

  STATIC_INLINE void QuatInverse( quat_t q ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    q[ 0 ] = -q[ 0 ];
    q[ 1 ] = -q[ 1 ];
    q[ 2 ] = -q[ 2 ];
  }
#endif

  STATIC_INLINE void QuatAntipodal( quat_t q ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    q[ 0 ] = -q[ 0 ];
    q[ 1 ] = -q[ 1 ];
    q[ 2 ] = -q[ 2 ];
    q[ 3 ] = -q[ 3 ];
  }
#endif

  STATIC_INLINE vec_t QuatLength( const quat_t q ) IFDECLARE
#ifdef Q3_VM_INSTANTIATE
  {
    return ( vec_t ) sqrt( q[ 0 ] * q[ 0 ] + q[ 1 ] * q[ 1 ] + q[ 2 ] * q[ 2 ] + q[ 3 ] * q[ 3 ] );
  }
#endif

  vec_t QuatNormalize( quat_t q );

  void  QuatFromAngles( quat_t q, vec_t pitch, vec_t yaw, vec_t roll );

  STATIC_INLINE void AnglesToQuat( const vec3_t angles, quat_t q )
#ifdef Q3_VM_INSTANTIATE
  {
    QuatFromAngles( q, angles[ PITCH ], angles[ YAW ], angles[ ROLL ] );
  }
#endif

  void QuatFromMatrix( quat_t q, const matrix_t m );
  void QuatToVectorsFLU( const quat_t quat, vec3_t forward, vec3_t left, vec3_t up );
  void QuatToVectorsFRU( const quat_t quat, vec3_t forward, vec3_t right, vec3_t up );

  void QuatToAxis( const quat_t q, vec3_t axis[ 3 ] );
  void QuatToAngles( const quat_t q, vec3_t angles );

// Quaternion multiplication, analogous to the matrix multiplication routines.

// qa = rotate by qa, then qb
  void QuatMultiply0( quat_t qa, const quat_t qb );

// qc = rotate by qa, then qb
  void QuatMultiply1( const quat_t qa, const quat_t qb, quat_t qc );

// qc = rotate by qa, then by inverse of qb
  void QuatMultiply2( const quat_t qa, const quat_t qb, quat_t qc );

// qc = rotate by inverse of qa, then by qb
  void QuatMultiply3( const quat_t qa, const quat_t qb, quat_t qc );

// qc = rotate by inverse of qa, then by inverse of qb
  void QuatMultiply4( const quat_t qa, const quat_t qb, quat_t qc );

  void QuatSlerp( const quat_t from, const quat_t to, float frac, quat_t out );
  void QuatTransformVector( const quat_t q, const vec3_t in, vec3_t out );
  void QuatTransformVectorInverse( const quat_t q, const vec3_t in, vec3_t out );

//=============================================
// combining Transformations

#ifndef	USING_SSE_MATH
  void TransInit( transform_t *t );
  void TransCopy( const transform_t *in, transform_t *out );

  void TransformPoint( const transform_t *t, const vec3_t in, vec3_t out );
  void TransformPointInverse( const transform_t *t, const vec3_t in, vec3_t out );
  void TransformNormalVector( const transform_t *t, const vec3_t in, vec3_t out );
  void TransformNormalVectorInverse( const transform_t *t, const vec3_t in, vec3_t out );

  void TransInitRotationQuat( const quat_t quat, transform_t *t );
  void TransInitRotation( const vec3_t axis, float angle,
        transform_t *t );
  void TransInitTranslation( const vec3_t vec, transform_t *t );
  void TransInitScale( float factor, transform_t *t );

  void TransInsRotationQuat( const quat_t quat, transform_t *t );
  void TransInsRotation( const vec3_t axis, float angle, transform_t *t );
  void TransAddRotationQuat( const quat_t quat, transform_t *t );
  void TransAddRotation( const vec3_t axis, float angle, transform_t *t );
  void TransInsScale( float factor, transform_t *t );
  void TransAddScale( float factor, transform_t *t );
  void TransInsTranslation( const vec3_t vec, transform_t *t );
  void TransAddTranslation( const vec3_t vec, transform_t *t );

  void TransCombine( const transform_t *a, const transform_t *b,
         transform_t *c );
  void TransInverse( const transform_t *in, transform_t *out );

  void TransStartLerp( transform_t *t );
  void TransAddWeight( float weight, const transform_t *a, transform_t *t );
  void TransEndLerp( transform_t *t );
#endif

#endif //hypov8 end merge:




float Com_Clamp(float min, float max, float value);
void COM_FixPath(char *pathname);
char* COM_SkipPath(char *pathname);
const char	*COM_GetExtension(const char *name);
void COM_StripExtension(const char *in, char *out, size_t destsize);
void COM_StripExtension2(const char *in, char *out, int destsize);
void COM_StripExtension3(const char *src, char *dest, size_t destsize);
void COM_DefaultExtension(char *path, int maxSize, const char *extension);
void Com_SkipRestOfLine(char **data);
void Com_SkipBracedSection(char **program);
void COM_BeginParseSession(const char *name);
int	COM_GetCurrentParseLine(void);
qboolean COM_CompareExtension(const char *in, const char *ext);
char *COM_Parse(char **data_p);
char *COM_ParseExt(char **data_p, qboolean allowLineBreak);
char       *COM_ParseExt2(const char **data_p, qboolean allowLineBreak); //add hypov8 unvan
char *Com_Parse(char **data_p);
char *Com_ParseExt(char **data_p, qboolean allowLineBreaks);
int COM_Compress(char *data_p);
void COM_ParseError(char *format, ...) __attribute__ ((format (printf, 1, 2)));
void COM_ParseWarning(char *format, ...) __attribute__ ((format (printf, 1, 2)));
int Com_HexStrToInt(const char *str);

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL				2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION		5			// punctuation
#define TT_FORMULA				6			// 0xA5EA
#endif

typedef struct pc_token_s
{
  int type;
  int subtype;
  int intvalue;
  float floatvalue;
  char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token
void COM_MatchToken(char**buf_p, char *match);
void SkipBracedSection(char **program);
qboolean SkipBracedSection_Depth(char **program, int depth);
void SkipRestOfLine(char **data);
void Parse1DMatrix(char **buf_p, int x, float *m);
void Com_Parse1DMatrix(char **buf_p, int x, float *m, qboolean checkBrackets);
void Parse2DMatrix(char **buf_p, int y, int x, float *m);
void Parse3DMatrix(char **buf_p, int z, int y, int x, float *m);
int Com_HexStrToInt(const char *str);
size_t	QDECL Com_sprintf(char *dest, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
char *Com_SkipTokens(char *s, int numTokens, char *sep);
char *Com_SkipCharset(char *s, char *sep);
void Com_RandomBytes(byte *string, int len);

// mode parm for FS_FOpenFile
typedef enum
{
  FS_READ,
  FS_WRITE,
  FS_APPEND,
  FS_APPEND_SYNC
} fsMode_t;

typedef enum
{
  FS_SEEK_CUR,
  FS_SEEK_END,
  FS_SEEK_SET
} fsOrigin_t;

//=============================================
int Q_isprint(int c);
int Q_islower(int c);
int Q_isupper(int c);
int Q_isalpha(int c);
qboolean Q_isanumber(const char *s);
qboolean Q_isintegral(float f);

#ifdef USE_ASM_LIB
//FIXME: QVM build
# define Com_Memset   A_memset
# define Com_Memcpy   A_memcpy
# define Com_Memmove  A_memmove
# define qstrlen      A_strlen
# define qstrcpy      A_strcpy
# define qstrcat      A_strcat
# define qstrcmp      A_strcmp
#else
# define Com_Memset   memset
# define Com_Memcpy   memcpy //dest,src, size
# define Com_Memmove  memmove
# define qstrlen      strlen
# define qstrcpy      strcpy
# define qstrcat      strcat
# define qstrcmp      strcmp
#endif

// portable case insensitive compare
#if defined (USE_ASM_LIB) && !defined (Q3_VM)
#define Q_stricmp A_stricmp
#define Q_strlwr A_strtolower
#define Q_strupr A_strtoupper
extern force_inline char* Q_strrchr(const char* string, int c)
{
  char buff[2] = {(char)(c), '\0' };
  return A_strstr((char*)string, buff);
}
#define Q_strstr A_strstr
//extern force_inline const char * Q_stristr(const char * haystack, const char * needle)
//{ return A_strstr((char*)haystack, needle); }
#else

#if defined( WIN32) || defined( _WIN64) //hypov8 BSPC
#define Q_stricmp	_stricmp
#define access _access
#else
//#define Q_stricmp stricmp //hy
int Q_stricmp(const char *s1, const char *s2);
#endif
char *Q_strlwr(char *s1);
char *Q_strupr(char *s1);
char *Q_strrchr(const char* string, int c);
#define Q_strstr strstr
#endif

const char* Q_stristr(const char *s, const char *find);
void Q_strcat(char *dest, size_t size, const char *src);
int Q_strncmp(const char *s1, const char *s2, size_t n);
int Q_stricmpn(const char *s1, const char *s2, size_t n);
int Q_strnicmp(const char *s1, const char *s2, size_t n);

qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace);

// buffer size safe library replacements
void	Q_strncpyz(char *dest, const char *src, size_t destsize);
// strlen that discounts Quake color sequences
int Q_PrintStrlen(const char *string);
// removes color sequences from string
char *Q_CleanStr(char *string);
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=============================================
// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
  byte	b0;
  byte	b1;
  byte	b2;
  byte	b3;
  byte	b4;
  byte	b5;
  byte	b6;
  byte	b7;
} qint64;

char* QDECL va(char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define TRUNCATE_LENGTH	64
void Com_TruncateLongString(char *buffer, const char *s);

typedef enum
{
  TT_NONE,
  TT_AABB,
  TT_CAPSULE,
  TT_BISPHERE,
  TT_NUM_TRACE_TYPES
} traceType_t;

//=============================================
// key / value info strings
char *Info_ValueForKey(const char *s, const char *key);
void Info_RemoveKey(char *s, const char *key);
void Info_RemoveKey_big(char *s, const char *key);
void Info_SetValueForKey(char *s, const char *key, const char *value);
void Info_SetValueForKey_Big(char *s, const char *key, const char *value);
qboolean Info_Validate(const char *s);
void Info_NextPair(const char **s, char *key, char *value);

typedef struct
{
  qboolean        frameMemory;
  int             currentElements;
  int             maxElements;	// will reallocate and move when exceeded
  void          **elements;
} growList_t;
// you don't need to init the growlist if you don't mind it growing and moving
// the list as it expands
void Com_InitGrowList(growList_t * list, int maxElements);
void Com_DestroyGrowList(growList_t * list);
int  Com_AddToGrowList(growList_t * list, void *data);
void* Com_GrowListElement(const growList_t * list, int index);
int  Com_IndexForGrowListElement(const growList_t * list, const void *element);

enum
{
  MEMSTREAM_SEEK_SET,
  MEMSTREAM_SEEK_CUR,
  MEMSTREAM_SEEK_END
};

enum
{
  MEMSTREAM_FLAGS_EOF = Q_BIT( 0 ),
  MEMSTREAM_FLAGS_ERR = Q_BIT( 1 ),
};

// helper struct for reading binary file formats
typedef struct memStream_s
{
  byte *buffer;
  int  bufSize;
  byte *curPos;
  int  flags;
}

memStream_t;

memStream_t *AllocMemStream( byte *buffer, int bufSize );
void        FreeMemStream( memStream_t *s );
int         MemStreamRead( memStream_t *s, void *buffer, int len );
int         MemStreamGetC( memStream_t *s );
int         MemStreamGetLong( memStream_t *s );
int         MemStreamGetShort( memStream_t *s );
float       MemStreamGetFloat( memStream_t *s );

void BoundsAdd(vec3_t mins, vec3_t maxs, const vec3_t mins2, const vec3_t maxs2);

#ifdef USE_PSK_MODEL_LOADER
enum
{
  MEMSTREAM_SEEK_SET,
  MEMSTREAM_SEEK_CUR,
  MEMSTREAM_SEEK_END
};

enum
{
  MEMSTREAM_FLAGS_EOF = Q_BIT(0),
  MEMSTREAM_FLAGS_ERR = Q_BIT(1),
};

// helper struct for reading binary file formats
typedef struct memStream_s
{
  byte           *buffer;
  int				bufSize;
  byte           *curPos;
  int             flags;
}
memStream_t;

memStream_t    *AllocMemStream(byte *buffer, int bufSize);

void			FreeMemStream(memStream_t * s);
int				MemStreamRead(memStream_t *s, void *buffer, int len);
int				MemStreamGetC(memStream_t *s);
int				MemStreamGetLong(memStream_t * s);
int				MemStreamGetShort(memStream_t * s);
float			MemStreamGetFloat(memStream_t * s);
#endif

/*
==========================================================
CVARS (console variables)
Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/
#define CVAR_ARCHIVE         Q_BIT(0)	   // set to cause it to be saved to vars.rc
                                         // used for system variables, not for player
                                         // specific configurations
#define CVAR_USERINFO		     Q_BIT(1)	   // sent to server on connect or change
#define CVAR_SERVERINFO		   Q_BIT(2)	   // sent in response to front end requests
#define CVAR_SYSTEMINFO		   Q_BIT(3)	   // these cvars will be duplicated on all clients
#define CVAR_INIT			       Q_BIT(4)	   // don't allow change from console at all,
                                         // but can be set from the command line
#define CVAR_LATCH			     Q_BIT(5)	   // will only change when C code next does
                                         // a Cvar_Get(), so it can't be changed
                                         // without proper initialization.  modified
                                         // will be set, even though the value hasn't
                                         // changed yet
#define CVAR_ROM			       Q_BIT(6)	   // display only, cannot be set by user at all
#define CVAR_USER_CREATED	   Q_BIT(7)	   // created by a set command
#define CVAR_TEMP			       Q_BIT(8)	   // can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			     Q_BIT(9)	   // can not be changed if cheats are disabled
#define CVAR_NORESTART		   Q_BIT(10)   // do not clear when a cvar_restart is issued
#define CVAR_SERVER_CREATED	 Q_BIT(11)   // cvar was created by a server the client connected to
#define CVAR_VM_CREATED		   Q_BIT(12)   // cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED       Q_BIT(13)   // prevent modifying this var from VMs or the server
                                         // These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED        Q_BIT(14)   // Cvar was modified
#define CVAR_SHADER		  	   Q_BIT(15)   // tell renderer to recompile shaders.
#define CVAR_NONEXISTENT	   0xFFFFFFFF  // cvar doesn't exist

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s cvar_t;
struct cvar_s
{
  char			*name;
  char			*string;
  char			*resetString;		// cvar_restart will reset to this value
  char			*latchedString;		// for CVAR_LATCH vars
  int				flags;
  qboolean	modified;			// set each time the cvar is changed
  int				modificationCount;	// incremented each time the cvar is changed
  float			value;				// atof(string)
  int				integer;			// atoi(string)
  qboolean	validate;
  qboolean	integral;
  float			min;
  float			max;

 cvar_t *next;
 cvar_t *prev;
 cvar_t *hashNext;
 cvar_t *hashPrev; //hypov8 merge: add struct
  int			hashIndex;
} ;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct
{
  cvarHandle_t	handle;
  int			modificationCount;
  float		value;
  int			integer;
  char		string[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

VoIP
==============================================================
*/
// if you change the count of flags be sure to also change VOIP_FLAGNUM
#define VOIP_SPATIAL		0x01		// spatialized voip message
#define VOIP_DIRECT		0x02		// non-spatialized voip message
// number of flags voip knows. You will have to bump protocol version number if you
// change this.
#define VOIP_FLAGCNT		2
/*
==============================================================
COLLISION DETECTION
==============================================================
*/
#include "surfaceflags.h"			// shared with the q3map utility

#ifdef BSPC
// 3-5 are non-axial planes snapped to the nearest for kaas
#define	PLANE_ANYX		3
#define	PLANE_ANYY		4
#define	PLANE_ANYZ		5
#endif

// for bsp build only
typedef enum
{
  SIDE_FRONT = 0,
  SIDE_BACK = 1,
  SIDE_ON = 2,
  SIDE_CROSS = 3
} planeSide_t;
typedef enum
{
  PLANE_X = 0,
  PLANE_Y = 1,
  PLANE_Z = 2,
  PLANE_NON_AXIAL = 3,
  PLANE_NON_PLANAR = 4
} planeType_t;
/*
=================
PlaneTypeForNormal
=================
*/
#ifndef Q3_VM
//#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL)))
static ID_INLINE int PlaneTypeForNormal(vec3_t normal)
{
  if(normal[0] == 1.0)
    return PLANE_X;
  if(normal[1] == 1.0)
    return PLANE_Y;
  if(normal[2] == 1.0)
    return PLANE_Z;
  if(normal[0] == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
    return PLANE_NON_PLANAR;
  return PLANE_NON_AXIAL;
}
#else
int PlaneTypeForNormal(vec3_t normal);
#endif

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s
{
  vec3_t	normal;
  float	dist;
  byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
  byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
  byte	pad[2];
} cplane_t;
// a trace is returned when a box is swept through the world
typedef struct
{
  qboolean        allsolid;	// if true, plane is not valid
  qboolean        startsolid;	// if true, the initial point was in a solid area
  float           fraction;	// time completed, 1.0 = didn't hit anything
  vec3_t          endpos;		// final position
  cplane_t        plane;		// surface normal at impact, transformed to world space
  int             surfaceFlags;	// surface hit
  int             contents;	// contents on other side of surface hit
  int             entityNum;	// entity the contacted sirface is a part of
  float           lateralFraction;	// fraction of collision tangetially to the trace direction
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD

// markfragments are returned by CM_MarkFragments()
typedef struct
{
  int		firstPoint;
  int		numPoints;
} markFragment_t;

typedef struct
{
  vec3_t		origin;
  vec3_t		axis[3];
} orientation_t;

//=====================================================================
// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI					0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008

// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
typedef enum {
  CHAN_AUTO,
  CHAN_LOCAL,		// menu sounds, etc
  CHAN_WEAPON,
  CHAN_VOICE,
  CHAN_ITEM,
  CHAN_BODY,
  CHAN_LOCAL_SOUND,	// chat messages, etc
  CHAN_ANNOUNCER		// announcer voices, etc
} soundChannel_t;

#if 1 //ndef BSPC //hypov8 
#ifndef Q3_VM
extern force_inline int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, struct cplane_s * p)
{
  {
    // fast axial cases
    if (p->type < 3)
    {
      if (p->dist <= emins[p->type])
        return 1;
      if (p->dist >= emaxs[p->type])
        return 2;
      return 3;
    }
  }

  {
    float dist[2];
    int   sides = 0;
     // general case
    dist[0] = dist[1] = 0;
    if (p->signbits < 8) // >= 8: default case is original code (dist[0]=dist[1]=0)
    {
      int i = 0, b;
      for (; i<3 ; i++)
      {
        b = (p->signbits >> i) & 1;
        dist[ b] += p->normal[i]*emaxs[i];
        dist[!b] += p->normal[i]*emins[i];
      }
    }

    if (dist[0] >= p->dist)
      sides = 1;
    if (dist[1] < p->dist)
      sides |= 2;

    return sides;
  }
}
#else
int BoxOnPlaneSide (const vec3_t emins, const vec3_t emaxs, struct cplane_s *p);
#endif
#endif

/*
========================================================================
  ELEMENTS COMMUNICATED ACROSS THE NET
========================================================================
*/
#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

// per-level limits
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define	GENTITYNUM_BITS		11	// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE	    	(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		    (MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

// if you increase GMODELNUM_BITS then:
//  increase MAX_CONFIGSTRINGS to 2048 and double MAX_MSGLEN
#define	GMODELNUM_BITS		8	// don't need to send any more
#define	MAX_MODELS			(1 << GMODELNUM_BITS)	// 2^9 = 512 references entityState_t::modelindex
#define	MAX_SOUNDS			256		// so they cannot be blindly increased
#define MAX_EFFECTS			256

#define	MAX_CONFIGSTRINGS	(1024)

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	16000
typedef struct
{
  int			stringOffsets[MAX_CONFIGSTRINGS];
  char		stringData[MAX_GAMESTATE_CHARS];
  int			dataCount;
} gameState_t;

//=========================================================
#define REF_FORCE_DLIGHT       ( 1 << 31 ) // RF, passed in through overdraw parameter, force this dlight under all conditions
#define REF_JUNIOR_DLIGHT      ( 1 << 30 ) // (SA) this dlight does not light surfaces.  it only affects dynamic light grid
#define REF_DIRECTED_DLIGHT    ( 1 << 29 ) // ydnar: global directional light, origin should be interpreted as a normal vector
#define REF_RESTRICT_DLIGHT    ( 1 << 1 ) // dlight is restricted to following entities
#define REF_INVERSE_DLIGHT     ( 1 << 0 ) // inverse dlight for dynamic shadows


// bit field limits
#define	MAX_STATS			    16
#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16
#define	MAX_WEAPONS				16

#define	MAX_PS_EVENTS			4 // max events per frame before we drop events //daemon .5 (was 2)

#define PS_PMOVEFRAMECOUNTBITS	6

#define SQR(a) ((a) * (a))

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
typedef struct playerState_s
{
  int			commandTime;	// cmd->serverTime of last executed command
  int			pm_type;
  int			bobCycle;		// for view bobbing and footstep generation
  unsigned int	pm_flags;		// ducked, jump_held, etc
  int			pm_time;

  vec3_t		origin;
  vec3_t		velocity;
  int			weaponTime;
  int			gravity;

  int			speed;
  int			delta_angles[3];	// add to command angles to get view direction
                  // changed by spawns, rotating objects, and teleporters

  int			groundEntityNum;// ENTITYNUM_NONE = in air

  int			legsTimer;		// don't change low priority animations until this runs out
  int			legsAnim;		// mask off ANIM_TOGGLEBIT

  int			torsoTimer;		// don't change low priority animations until this runs out
  int			torsoAnim;		// mask off ANIM_TOGGLEBIT

  int			movementDir;	// a number 0 to 7 that represents the reletive angle
                // of movement to the view angle (axial and diagonals)
                // when at rest, the value will remain unchanged
                // used to twist the legs during strafing

  unsigned int	eFlags;			// copied to entityState_t->eFlags

  int			eventSequence;	// pmove generated events
  int			events[MAX_PS_EVENTS];
  int			eventParms[MAX_PS_EVENTS];

  int			externalEvent;	// events set on player from another source
  int			externalEventParm;
  int			externalEventTime;

  int			clientNum;		// ranges from 0 to MAX_CLIENTS-1

        // weapon info
  int			weapon;			// copied to entityState_t->weapon
  int			weaponstate;

  vec3_t		viewangles;		// for fixed views
  int			viewheight;


  // damage feedback
  int			damageEvent;	// when it changes, latch the other parms
  int			damageYaw;
  int			damagePitch;
  int			damageCount;

  int			stats[MAX_STATS];          // declared as int but sent over network as short !!!!
  int			persistant[MAX_PERSISTANT];	// stats that aren't cleared on death

  int			generic1;
  int			loopSound;

  vec3_t		grapplePoint;	// location of grapple to pull towards if PMF_GRAPPLE_PULL
  int			weaponAnim;		// mask off ANIM_TOGGLEBIT //daemon .50

  int			ammo_all[MAX_WEAPONS];	// bullets left on player total
  int			ammo_mag[MAX_WEAPONS];	// bullets left in gun/mag
  int			powerups[MAX_POWERUPS];	// level.time that the powerup runs out// //hypov8 todo: sent as short int. is level.time to long??

  int			hmgBulletNum;	// stored. count the 3 bullets
  int			viewShootBob;	// view recoil
  int			viewBurnCount;	// change red blood to orange flame
  int			bunnyHop;		// prevent bunyhop. cap speed on ground

  // ---------------------------------------------------------------------- //
  // not communicated over the net at all
  int			ping;			// server to game info for scoreboard
  int			pmove_framecount;	// FIXME: don't transmit over the network
  int			entityEventSequence;
  int			jumppad_frame;
  int			jumppad_ent;	// jumppad entity hit this frame
  int			safetime;		//hypov8 todo: move. kill safe campers
  // ---------------------------------------------------------------------- //

  // Added for fire damage -KRYPTYK
  //int		onFireStart; //hypov8 todo: not used??
  //int		onFireEnd;
} playerState_t;

//====================================================================
//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK	        	1
#define	BUTTON_TALK		        	2			  // displays talk balloon and disables actions
#define	BUTTON_USE_HOLDABLE	    4
#define	BUTTON_GESTURE	      	8
#define	BUTTON_WALKING	      	16			// walking can't just be infered from MOVE_RUN
                                        // because a key pressed late in the frame will
                                        // only generate a small move value for that frame
                                        // walking will use different animations and
                                        // won't generate footsteps
#define BUTTON_AFFIRMATIVE	    32
#define	BUTTON_NEGATIVE		      64
#define BUTTON_GETFLAG	      	128
#define BUTTON_GUARDBASE	      256
#define BUTTON_PATROL		        512
#define BUTTON_FOLLOWME		      1024
#define BUTTON_RELOAD           2048
#define	BUTTON_ANY			        4096 //2048			// any key whatsoever

#define	MOVE_RUN		           	120			// if forwardmove or rightmove are >= MOVE_RUN,
                    // then BUTTON_WALKING should be set

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s
{
  int				serverTime;
  int				angles[3];
  int 			buttons;
  byte			weapon;           // weapon
  signed char	forwardmove, rightmove, upmove;
} usercmd_t;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

typedef enum
{
  TR_STATIONARY,
  TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
  TR_LINEAR,
  TR_LINEAR_STOP,
  TR_SINE,					// value = base + sin(time / duration) * delta
  TR_GRAVITY,
  TR_BUOYANCY,
  TR_ACCELERATE,
  TR_DECCELERATE
} trType_t;

typedef struct
{
  trType_t	trType;
  int		trTime;
  int		trDuration;			// if non 0, trTime + trDuration = stop time
  float           trAcceleration;	// gravity factor, etc
  vec4_t          trBase;		// Tr3B: changed from vec3_t to vec4_t to support quaternions
  vec4_t          trDelta;	// velocity, etc - Tr3B: changed from vec3_t to vec4_t to support quaternions
} trajectory_t;

//void ZeroBounds(vec3_t mins, vec3_t maxs);
void Matrix4x4MultiplySelf(matrix_t m, const matrix_t m2);
void MatrixFromQuat(matrix_t m, const quat_t q);
void MatrixSetupTransformFromQuat(matrix_t m, const quat_t quat, const vec3_t origin);
void MatrixMultiplyTranslation(matrix_t m, vec_t x, vec_t y, vec_t z);
void MatrixFromAngles(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void MatrixTransformNormal2(const matrix_t m, vec3_t inout);
void MatrixTransform4(const matrix_t m, const vec4_t in, vec4_t out);
void MatrixSetupTransform(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin);
void MatrixMultiplyZRotation(matrix_t m, vec_t degrees);
void MatrixMultiplyRotation(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void MatrixMultiplyShear(matrix_t m, vec_t x, vec_t y);
qboolean _mat4x4_Inverse(const float *A, float *invA);
qboolean MatrixInverse(matrix_t A);

void QuatZero(quat_t o);
void QuatInverse(quat_t q);
void QuatMA(const quat_t p, float f, const quat_t q, quat_t o);
void QuatToVectorsFRU(const quat_t quat, vec3_t forward, vec3_t right, vec3_t up);
void QuatSlerp(const quat_t from, const quat_t to, float frac, quat_t out);
void QuatToAxis(const quat_t q, vec3_t axis[3]);
void QuatTransformVector(const quat_t q, const vec3_t in, vec3_t out);
void QuatTransformVectorInverse(const quat_t q, const vec3_t in, vec3_t out);
void QuatFromMatrix(quat_t q, const matrix_t m);
void PlaneIntersectRay(const vec3_t rayPos, const vec3_t rayDir, const vec4_t plane, vec3_t res);
void MatrixOrthogonalProjectionRH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);
void MatrixCrop(matrix_t m, const vec3_t mins, const vec3_t maxs);

vec_t QuatNormalize(quat_t q);
void QuatFromAngles(quat_t q, vec_t pitch, vec_t yaw, vec_t roll);
#ifndef Q3_VM
/*
extern force_inline void QuatCalcW(quat_t q)
{
  vec_t term = 1.0f - (q[0] * q[0] + q[1] * q[1] + q[2] * q[2]);
  if(term < 0.0)
    q[3] = 0.0;
  else
    q[3] = -sqrt(term);
  //q[3] = sqrt(fabs(1.0f - (q[0] * q[0] + q[1] * q[1] + q[2] * q[2])));
}
*/

#else // Q3_VM
void AnglesToQuat(const vec3_t angles, quat_t q);
#endif // Q3_VM

//#define QuatCopy(a,b)   ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
//#define QuatCompare(a,b)  ((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3])
//#define QuatClear(q) ((q)[0] = (q)[1] = (q)[2] = 0, (q)[3] = 1)
// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s
{
  int		number;			// entity index
  int		eType;			// entityType_t
  int		eFlags;

  trajectory_t	pos;	// for calculating position
  trajectory_t	apos;	// for calculating angles

  int		time;
  int		time2;

  vec3_t	origin;
  vec3_t	origin2;

  vec3_t	angles;
  vec3_t	angles2;

  int		otherEntityNum;	 // shotgun sources, etc
  int		otherEntityNum2;

  int		groundEntityNum;// -1 = in air

  int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
  int		loopSound;		  // constantly loop this sound

  int		modelindex;
  int		modelindex2;
  int		clientNum;	  	// 0 to (MAX_CLIENTS - 1), for players and corpses
  int		frame;

  int		solid;			    // for client side prediction, trap_linkentity sets this properly

    // old style events, in for compatibility only
  int		event;			    // impulse events -- muzzle flashes, footsteps, etc
  int		eventParm;

  // for players
  int		powerups;		    // bit flags
  int		weapon;			    // determines weapon and flash model, etc
  int		legsAnim;		    // mask off ANIM_TOGGLEBIT
  int		torsoAnim;	  	// mask off ANIM_TOGGLEBIT

  int		generic1;

  // Added for fire damage state -KRYPTYK 8/21/11
  int onFireStart;
  int onFireEnd;

  int weaponAnim; // mask off ANIM_TOGGLEBIT

  int	chaseCamMode;			//add hypov8 ToDo:
  //0xA5EA: beware, if you add new variables you have to edit
  //entityStateFields in msg.cc as well !!!
} entityState_t;

typedef enum
{
  CA_UNINITIALIZED,
  CA_DISCONNECTED, 	// not talking to a server
  CA_AUTHORIZING,		// not used any more, was checking cd key
  CA_CONNECTING,		// sending request packets to the server
  CA_CHALLENGING,		// sending challenge packets to the server
  CA_CONNECTED,		// netchan_t established, getting gamestate
  CA_LOADING,			// only during cgame initialization, never during main loop
  CA_PRIMED,			// got gamestate, waiting for first frame
  CA_ACTIVE,			// game views should be displayed
  CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

// font support

#define GLYPH_START       0
#define GLYPH_END         255
#define GLYPH_CHARSTART   32
#define GLYPH_CHAREND     127
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1

typedef struct
{
  int height;       // number of scan lines
  int top;          // top of glyph in buffer
  int bottom;       // bottom of glyph in buffer
  int pitch;        // width for copying
  int xSkip;        // x adjustment
  int imageWidth;   // width of actual image
  int imageHeight;  // height of actual image
  float s;          // x offset in image where glyph starts
  float t;          // y offset in image where glyph starts
  float s2;
  float t2;
  qhandle_t glyph;  // handle to the shader with the glyph
  char shaderName[32];
} glyphInfo_t;

#define NULL_FONT NULL
typedef struct
{
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
  float         glyphScale;
  char          name[MAX_QPATH];
} fontInfo_t;

#define Square(x) ((x)*(x))

// real time
//=============================================
typedef struct qtime_s
{
  int tm_sec;     /* seconds after the minute - [0,59] */
  int tm_min;     /* minutes after the hour - [0,59] */
  int tm_hour;    /* hours since midnight - [0,23] */
  int tm_mday;    /* day of the month - [1,31] */
  int tm_mon;     /* months since January - [0,11] */
  int tm_year;    /* years since 1900 */
  int tm_wday;    /* days since Sunday - [0,6] */
  int tm_yday;    /* days since January 1 - [0,365] */
  int tm_isdst;   /* daylight savings time flag */
} qtime_t;

// server browser sources
// TTimo: AS_MPLAYER is no longer used
#define AS_LOCAL			0
//#define AS_MPLAYER			1
#define AS_GLOBAL			1 //master0
#define AS_GLOBAL1			2 //master1
#define AS_GLOBAL2			3 //master2
#define AS_GLOBAL3			4 //master3
#define AS_GLOBAL4			5 //master4
#define AS_GLOBAL5			6 //master4
#define AS_FAVORITES		7

// cinematic states
typedef enum
{
  FMV_IDLE,
  FMV_PLAY,		// play
  FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
  FMV_ID_BLT,
  FMV_ID_IDLE,
  FMV_LOOPED,
  FMV_ID_WAIT
} e_status;

typedef enum _flag_status
{
  FLAG_ATBASE = 0,
  FLAG_TAKEN,			     // CTF
  FLAG_TAKEN_DRAGON, 	 // One Flag CTF
  FLAG_TAKEN_NIKKI,	   // One Flag CTF
  FLAG_DROPPED
} flagStatus_t;
// 0xA5EA

#define	MAX_GLOBAL_SERVERS				4096
#define	MAX_OTHER_SERVERS					128
#define MAX_PINGREQUESTS					32
#define MAX_SERVERSTATUSREQUESTS	16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

#ifndef STANDALONE		// 0xA5EA
#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2
#endif

typedef enum
{
  CF_UNDEF         = 0 <<  1,
  CF_RDTSC         = 1 <<  0,
  CF_MMX           = 1 <<  1,
  CF_MMX_EXT       = 1 <<  2,
  CF_3DNOW         = 1 <<  3,
  CF_3DNOW_EXT     = 1 <<  4,
  CF_SSE           = 1 <<  5,
  CF_SSE2          = 1 <<  6,
  CF_SSE3          = 1 <<  7,
  CF_SSE3_SUP      = 1 <<  9,
  CF_SSE41         = 1 << 10,
  CF_SSE41_POPCENT = 1 << 11,
  CF_SSE42         = 1 << 12,
  CF_AVXYMM        = 1 << 13,
  CF_AVX2          = 1 << 14,
  CF_PCLMUL_AES    = 1 << 15,
  CF_ALTIVEC       = 1 << 16,
} cpuFeatures_t;

typedef struct
{
  int numCpus;
  char cpuName[256];
  int features;
  int instructionSet;
  qboolean bigEndian;
  size_t cacheSizeL1;
  size_t cacheSizeL2;
  size_t cacheSizeL3;
}cpuConfig_t;

#ifndef Q3_VM
/*
extern force_inline void SetPlaneSignbits(cplane_t *out)
{
  int bits = 0, j;
  // for fast box on planeside test
  for (j = 0; j < 3; j++)
  {
    if (out->normal[j] < 0.f)
    {
      bits |= 1 << j;
    }
  }
  out->signbits = bits;
}*/
#endif
#endif	// __Q_SHARED_H
