/* -------------------------------------------------------------------------------

Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

----------------------------------------------------------------------------------
This code has been altered significantly from its original form, to support
several games based on the Quake III Arena engine, in the form of "Q3Map2."
------------------------------------------------------------------------------- */
/* marker */
#ifndef GAME_KINGPINQ3_H_
#define GAME_KINGPINQ3_H_

/* -------------------------------------------------------------------------------
content and surface flags
------------------------------------------------------------------------------- */

/* game flags */
#define	Q_CONT_SOLID				0x00000001	/*1*/	// an eye is never valid in a solid
#define Q_CONT_OPAQUE				0x00000002	/*2*/	//  ydnar flags
#define Q_CONT_LIGHTGRID			0x00000004	/*4*/	//  ydnar flags
#define	Q_CONT_LAVA					0x00000008	/*8*/
#define	Q_CONT_SLIME				0x00000010	/*16*/
#define	Q_CONT_WATER				0x00000020	/*32*/
#define	Q_CONT_FOG					0x00000040	/*64*/
#define Q_CONT_NOTTEAM1				0x00000080	/*128*/
#define Q_CONT_NOTTEAM2				0x00000100	/*256*/
#define Q_CONT_NOBOTCLIP			0x00000200	/*512*/
#define Q_CONT_LADDER				0x00000400	/*1024*/	//1024	//0x100000000  //FIXME: 0xA5EA, added for bsp build
//#define Q_CONT_ITEM				0x00000800	/*2048*/	//unused
//#define Q_CONT_AI_NOSIGHT			0x00001000	/*4096*/	//unused
//#define Q_CONT_CLIPSHOT			0x00002000	/*8192*/	//unused
#define	Q_CONT_MISSILECLIP 			0x00004000	/*16384*/	// add hypov8
#define	Q_CONT_AREAPORTAL			0x00008000	/*32768*/
#define	Q_CONT_PLAYERCLIP			0x00010000	/*65536*/
#define	Q_CONT_MONSTERCLIP			0x00020000	/*131072*/
#define	Q_CONT_TELEPORTER			0x00040000	/*262144*/ //bot specific contents types
#define	Q_CONT_JUMPPAD				0x00080000	/*524288*/	
#define Q_CONT_CLUSTERPORTAL		0x00100000	/*1048576*/
#define Q_CONT_DONOTENTER			0x00200000	/*2097152*/
#define Q_CONT_BOTCLIP				0x00400000	/*4194304*/
#define Q_CONT_MOVER				0x00800000	/*8388608*/
#define	Q_CONT_ORIGIN				0x01000000	/*16777216*/	// removed before bsping an entity
#define	Q_CONT_BODY					0x02000000	/*33554432*/	// should never be on a brush, only in game
#define	Q_CONT_CORPSE				0x04000000	/*67108864*/
#define	Q_CONT_DETAIL				0x08000000	/*134217728*/	// brushes not used for the bsp
#define	Q_CONT_STRUCTURAL			0x10000000	/*268435456*/	// brushes used for the bsp
#define	Q_CONT_TRANSLUCENT			0x20000000	/*536870912*/	// don't consume surface fragments inside
#define	Q_CONT_TRIGGER				0x40000000	/*1073741824*/
#define	Q_CONT_NODROP				0x80000000	/*2147483648*/	// don't leave bodies or items (death fog, lava)

#define	Q_SURF_NODAMAGE				0x00000001	// never give falling damage
#define	Q_SURF_SLICK				0x00000002	// effects game physics
#define	Q_SURF_SKY					0x00000004	// lighting from environment map
#define	Q_SURF_LADDER				0x00000008
#define	Q_SURF_NOIMPACT				0x00000010	// don't make missile explosions
#define	Q_SURF_NOMARKS				0x00000020	// don't leave missile marks
#define	Q_SURF_FLESH				0x00000040	// make flesh sounds and effects
#define	Q_SURF_NODRAW				0x00000080	// don't generate a drawsurface at all
#define	Q_SURF_HINT					0x00000100	// make a primary bsp splitter
#define	Q_SURF_SKIP					0x00000200	// completely ignore, allowing non-closed brushes
#define	Q_SURF_NOLIGHTMAP			0x00000400	// surface doesn't need a lightmap
#define	Q_SURF_POINTLIGHT			0x00000800	// generate lighting info at vertexes
#define	Q_SURF_METALSTEPS			0x00001000	// clanking footsteps
#define	Q_SURF_NOSTEPS				0x00002000	// no footstep sounds
#define	Q_SURF_NONSOLID				0x00004000	// don't collide against curves with this set
#define	Q_SURF_LIGHTFILTER			0x00008000	// act as a light filter during q3map -light
#define	Q_SURF_ALPHASHADOW			0x00010000	// do per-pixel light shadow casting in q3map
#define	K_SURF_COLLISION			0x00020000	// don't draw but use for per polygon collision detection
#define Q_SURF_DUST					0x00040000 // leave a dust trail when walking on this surface
#define Q_SURF_GRAVEL				0x00080000	// footsteps steps gravel
#define Q_SURF_METALLIGHT			0x00100000	// footsteps metal light
#define Q_SURF_RUG 					0x00200000	// footsteps steps rug
#define Q_SURF_SNOW					0x00400000	// footsteps steps snow
#define Q_SURF_WOOD					0x00800000	// footsteps steps wood
#define Q_SURF_ICE					0x01000000	// footsteps steps ice
#define Q_SURF_GRASS				0x02000000	// footsteps steps crass
#define Q_SURF_MARBLE				0x04000000	// footsteps steps marble
#define Q_SURF_TIN				    0x08000000	// footsteps steps tin
#define Q_SURF_GLASS				0x10000000	// glass damage marks
//#define	Q_SURF_					0x20000000	// unused
//#define	Q_SURF_					0x40000000	//unused
//#define	Q_SURF_					0x80000000	// unused

#define Q_SURF_VERTEXLIT			(Q_SURF_POINTLIGHT | Q_SURF_NOLIGHTMAP)

/* -------------------------------------------------------------------------------
game_t struct
------------------------------------------------------------------------------- */

{
	"kingpinq3",        /* -game x */
	"basekpq3",         /* default base game data dir */
	".KingpinQ3",       /* unix home sub-dir */
	"kingpinq3",        /* magic path word */
	"materials",        /* shader directory */
	1024,               /* max lightmapped surface verts */
	1024,               /* max surface verts */
	6144,               /* max surface indexes */
	qfalse,             /* flares */
	"flareshader",      /* default flare shader */
	qfalse,             /* wolf lighting model? */
	1024,               /* lightmap width/height */
	1.0f,               /* lightmap gamma */
	1.0f,               /* lightmap exposure */
	1.0f,               /* lightmap compensate */
	1.0f,               /* lightgrid scale */
	1.0f,               /* lightgrid ambient scale */
	qfalse,             /* light angle attenuation uses half-lambert curve */
	qtrue,              /* disable shader lightstyles hack */
	qfalse,              /* keep light entities on bsp */ /* hypov8 lights will be loaded from .map*/
	8,                  /* default patchMeta subdivisions tolerance */
	qtrue,              /* patch casting enabled */
	qtrue,              /* compile deluxemaps */
	0,                  /* deluxemaps default mode */
	512,                /* minimap size */
	1.0f,               /* minimap sharpener */
	0.0f,               /* minimap border */
	qtrue,              /* minimap keep aspect */
	MINIMAP_MODE_BLACK, /* minimap mode */
	"%s.png",           /* minimap name format */
	"XBSP",             /* bsp file prefix */
	48,                 /* bsp file version */
	qfalse,             /* cod-style lump len/ofs order */
	LoadXBSPFile,       /* bsp load function */
	WriteXBSPFile,      /* bsp write function */

	{
		/* name				contentFlags				contentFlagsClear			surfaceFlags				surfaceFlagsClear			compileFlags				compileFlagsClear */
		/* default */
		{ "default",		  Q_CONT_SOLID,				-1,							0,							-1,							C_SOLID,					-1 },

		/* ydnar */
		{ "lightgrid",		0,							0,							0,							0,							C_LIGHTGRID,				0 },
		{ "antiportal",		0,							0,							0,							0,							C_ANTIPORTAL,				0 },
		{ "skip",			    0,							0,							0,							0,							C_SKIP,						0 },

		/* compiler */
		{ "origin",			  Q_CONT_ORIGIN,				Q_CONT_SOLID,				0,							0,							C_ORIGIN | C_TRANSLUCENT,	C_SOLID },
		{ "areaportal",		Q_CONT_AREAPORTAL,			Q_CONT_SOLID,				0,							0,							C_AREAPORTAL | C_TRANSLUCENT,	C_SOLID },
		{ "trans",			  Q_CONT_TRANSLUCENT,			0,							0,							0,							C_TRANSLUCENT,				0 },
		{ "translucent",	Q_CONT_TRANSLUCENT,			0,							0,							0,							C_TRANSLUCENT,				0 },
		{ "detail",			  Q_CONT_DETAIL,				0,							0,							0,							C_DETAIL,					0 },
		{ "discrete",		Q_CONT_DETAIL,				0,							0,							0,					C_DETAIL,							0 },		
		{ "structural",		Q_CONT_STRUCTURAL,			0,							0,							0,							C_STRUCTURAL,				0 },
		{ "hint",			    0,							0,							Q_SURF_HINT,				0,							C_HINT,						0 },
		{ "nodraw",			  0,							0,							Q_SURF_NODRAW,				0,							C_NODRAW,					0 },

		{ "alphashadow",	0,							0,							Q_SURF_ALPHASHADOW,			0,							C_ALPHASHADOW | C_TRANSLUCENT,	0 },
		{ "lightfilter",	0,							0,							Q_SURF_LIGHTFILTER,			0,							C_LIGHTFILTER | C_TRANSLUCENT,	0 },
		{ "nolightmap",		0,							0,							Q_SURF_VERTEXLIT,			0,							C_VERTEXLIT,				0 },
		{ "pointlight",		0,							0,							Q_SURF_VERTEXLIT,			0,							C_VERTEXLIT,				0 },


		/* game */
		{ "nonsolid",		0,							Q_CONT_SOLID,				Q_SURF_NONSOLID,			0,							0,							C_SOLID },

		{ "trigger",		Q_CONT_TRIGGER,				Q_CONT_SOLID,				0,							0,							C_TRANSLUCENT,				C_SOLID },

		{ "water",			Q_CONT_WATER,				Q_CONT_SOLID,				0,							0,							C_LIQUID | C_TRANSLUCENT,	C_SOLID },
		{ "slime",			Q_CONT_SLIME,				Q_CONT_SOLID,				0,							0,							C_LIQUID | C_TRANSLUCENT,	C_SOLID },
		{ "lava",			Q_CONT_LAVA,				Q_CONT_SOLID,				0,							0,					C_LIQUID | C_TRANSLUCENT,			C_SOLID },

		{ "missileclip",	Q_CONT_MISSILECLIP,			Q_CONT_SOLID,				0,							0,					C_TRANSLUCENT,						C_SOLID },
		{ "playerclip",		Q_CONT_PLAYERCLIP,			Q_CONT_SOLID,				0,							0,					C_TRANSLUCENT,						C_SOLID },
		{ "monsterclip",	Q_CONT_MONSTERCLIP,			Q_CONT_SOLID,				0,							0,					C_TRANSLUCENT,						C_SOLID },
		{ "nodrop",			Q_CONT_NODROP,				Q_CONT_SOLID,				0,							0,							C_TRANSLUCENT,				C_SOLID },

		{ "clusterportal",	Q_CONT_CLUSTERPORTAL,		Q_CONT_SOLID,				0,							0,							C_TRANSLUCENT,				C_SOLID },
		{ "donotenter",		Q_CONT_DONOTENTER,			Q_CONT_SOLID,				0,							0,							C_TRANSLUCENT,				C_SOLID },
		{ "botclip",		Q_CONT_BOTCLIP,				Q_CONT_SOLID,				0,							0,							C_TRANSLUCENT,				C_SOLID },

		{ "fog",			Q_CONT_FOG,					Q_CONT_SOLID,				0,							0,							C_FOG,						C_SOLID },
		{ "sky",			0,							0,							Q_SURF_SKY,					0,							C_SKY,						0 },

		{ "slick",			0,							0,							Q_SURF_SLICK,				0,							0,							0 },

		{ "noimpact",		0,							0,							Q_SURF_NOIMPACT,			0,							0,							0 },
		{ "nomarks",		0,							0,							Q_SURF_NOMARKS,				0,							C_NOMARKS,					0 },
		{ "ladder",			Q_CONT_LADDER,			Q_CONT_SOLID,							Q_SURF_LADDER,				0,							C_TRANSLUCENT,							C_SOLID },
		{ "nodamage",		0,							0,							Q_SURF_NODAMAGE,			0,							0,							0 },
		{ "metalsteps",		0,							0,							Q_SURF_METALSTEPS,			0,							0,							0 },
		{ "flesh",			0,							0,							Q_SURF_FLESH,				0,							0,							0 },
		{ "nosteps",		0,							0,							Q_SURF_NOSTEPS,				0,							0,							0 },
		{ "collision",		0,							0,							K_SURF_COLLISION,			0,							C_SOLID | C_COLLISION,		0 },
		{ "dust",			0,							0,							Q_SURF_DUST,				0,							0,							0 },


		{ "gravelsteps",	0,							0,							Q_SURF_GRAVEL,				0,							0,							0 },
		{ "metallightsteps",0,							0,							Q_SURF_METALLIGHT,			0,							0,							0 },
		{ "rugsteps",		0,							0,							Q_SURF_RUG,					0,							0,							0 },
		{ "snowsteps",		0,							0,							Q_SURF_SNOW,				0,							0,							0 },
		{ "woodsteps",		0,							0,							Q_SURF_WOOD,				0,							0,							0 },
		{ "icesteps",		0,							0,							Q_SURF_ICE,					0,					0,									0 }, /*enabled hypov8*/
		{ "grassteps",		0,							0,							Q_SURF_GRASS,				0,					0,									0 },
		{ "marblesteps",		0,							0,						Q_SURF_MARBLE,				0,							0,							0 },
		{ "metaltinsteps",		0,							0,						Q_SURF_TIN,					0,							0,							0 },
		{ "glassteps",		0,							0,							Q_SURF_GLASS,				0,							0,							0 },


		/* null */
		{ NULL, 0, 0, 0, 0, 0, 0 }
	}
}



/* end marker */
#endif

