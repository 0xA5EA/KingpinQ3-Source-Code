/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// This file must be identical in the quake and utils directories
// contents flags are seperate bits
// a given brush can contribute multiple content bits
// these definitions also need to be in q_shared.h!

#define	CONTENTS_SOLID				0x00000001	/*1*/	// an eye is never valid in a solid
#define CONTENTS_OPAQUE				0x00000002	/*2*/	//  ydnar flags
#define CONTENTS_LIGHTGRID			0x00000004	/*4*/	//  ydnar flags
#define	CONTENTS_LAVA				0x00000008	/*8*/
#define	CONTENTS_SLIME				0x00000010	/*16*/
#define	CONTENTS_WATER				0x00000020	/*32*/
#define	CONTENTS_FOG				0x00000040	/*64*/
#define CONTENTS_NOTTEAM1			0x00000080	/*128*/
#define CONTENTS_NOTTEAM2			0x00000100	/*256*/
#define CONTENTS_NOBOTCLIP			0x00000200	/*512*/
#define CONTENTS_LADDER				0x00000400	/*1024*/	//1024	//0x100000000  //FIXME: 0xA5EA, added for bsp build
//#define CONTENTS_ITEM				0x00000800	/*2048*/	//unused
//#define CONTENTS_AI_NOSIGHT		0x00001000	/*4096*/	//unused
//#define CONTENTS_CLIPSHOT			0x00002000	/*8192*/	//unused
#define	CONTENTS_MISSILECLIP 		0x00004000	/*16384*/	// add hypov8
#define	CONTENTS_AREAPORTAL			0x00008000	/*32768*/
#define	CONTENTS_PLAYERCLIP			0x00010000	/*65536*/
#define	CONTENTS_MONSTERCLIP		0x00020000	/*131072*/
#define	CONTENTS_TELEPORTER			0x00040000	/*262144*/ //bot specific contents types
#define	CONTENTS_JUMPPAD			0x00080000	/*524288*/	
#define CONTENTS_CLUSTERPORTAL		0x00100000	/*1048576*/
#define CONTENTS_DONOTENTER			0x00200000	/*2097152*/
#define CONTENTS_BOTCLIP			0x00400000	/*4194304*/
#define CONTENTS_MOVER				0x00800000	/*8388608*/
#define	CONTENTS_ORIGIN				0x01000000	/*16777216*/	// removed before bsping an entity
#define	CONTENTS_BODY				0x02000000	/*33554432*/	// should never be on a brush, only in game
#define	CONTENTS_CORPSE				0x04000000	/*67108864*/
#define	CONTENTS_DETAIL				0x08000000	/*134217728*/	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL			0x10000000	/*268435456*/	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT		0x20000000	/*536870912*/	// don't consume surface fragments inside
#define	CONTENTS_TRIGGER			0x40000000	/*1073741824*/
#define	CONTENTS_NODROP				0x80000000	/*2147483648*/	// don't leave bodies or items (death fog, lava)

#define	SURF_NODAMAGE		       0x00000001	// never give falling damage
#define	SURF_SLICK			       0x00000002	// effects game physics
#define	SURF_SKY				       0x00000004	// lighting from environment map
#define	SURF_LADDER			       0x00000008
#define	SURF_NOIMPACT		       0x00000010	// don't make missile explosions
#define	SURF_NOMARKS		       0x00000020	// don't leave missile marks
#define	SURF_FLESH			       0x00000040	// make flesh sounds and effects
#define	SURF_NODRAW			       0x00000080	// don't generate a drawsurface at all
#define	SURF_HINT				       0x00000100	// make a primary bsp splitter
#define	SURF_SKIP				       0x00000200	// completely ignore, allowing non-closed brushes
#define	SURF_NOLIGHTMAP	       0x00000400	// surface doesn't need a lightmap
#define	SURF_POINTLIGHT	       0x00000800	// generate lighting info at vertexes
#define	SURF_METALSTEPS				0x00001000	// footsteps clanking 
#define	SURF_NOSTEPS				0x00002000	// footstep NO sounds
#define	SURF_NONSOLID		       0x00004000	// don't collide against curves with this set
#define	SURF_LIGHTFILTER       0x00008000	// act as a light filter during q3map -light
#define	SURF_ALPHASHADOW       0x00010000	// do per-pixel light shadow casting in q3map
#define	SURF_COLLISION	       0x00020000	// don't draw but use for per polygon collision detection
#define SURF_DUST				       0x00040000 // leave a dust trail when walking on this surface
#define SURF_GRAVEL				     0x00080000	// footsteps steps gravel
#define SURF_METALLIGHT		     0x00100000	// footsteps metal light
#define SURF_RUG 				       0x00200000	// footsteps steps rug
#define SURF_SNOW				       0x00400000	// footsteps steps snow
#define SURF_WOOD				       0x00800000	// footsteps steps wood
#define SURF_ICE				       0x01000000	// footsteps steps ice
#define SURF_GRASS				     0x02000000	// footsteps steps grass
#define SURF_MARBLE				     0x04000000	// footsteps steps marble
#define SURF_TIN				       0x08000000	// footsteps steps tin
#define SURF_GLASS				     0x10000000	// glass damage marks
//#define	SURF_					0x20000000	// unused
//#define	SURF_					0x40000000	//unused
//#define	SURF_					0x80000000	// unused

//add hypov8. kmap has this combined, choose either vertex or lightmap
//not sure if some objects wont need either, like flairs??
#define SURF_VERTEXLIT			(SURF_POINTLIGHT | SURF_NOLIGHTMAP)

#ifdef BSPC
#define	LAST_VISIBLE_CONTENTS	64
#endif

