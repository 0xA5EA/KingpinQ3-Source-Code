/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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

/*****************************************************************************
 * name:		be_interface.h
 *
 * desc:		botlib interface
 *
 * $Archive: /source/code/botlib/be_interface.h $
 *
 *****************************************************************************/

//#define DEBUG			//debug code
#define RANDOMIZE		//randomize bot behaviour

//FIXME: get rid of this global structure
typedef struct botlib_globals_s
{
	int botlibsetup;						//true when the bot library has been setup
	int maxentities;						//maximum number of entities
	int maxclients;							//maximum number of clients
	float time;								//the global time
#ifdef DEBUG
	qboolean debug;							//true if debug is on
	int goalareanum;
	vec3_t goalorigin;
	int runai;
#endif
} botlib_globals_t;


extern botlib_globals_t botlibglobals;
extern botlib_import_t botimport;
extern int botDeveloper;					//true if developer is on

//
int Sys_MilliSeconds(void);

