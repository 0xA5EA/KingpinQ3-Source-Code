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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef CG_PUBLIC_H_
#define CG_PUBLIC_H_

#define CMD_BACKUP 64
#define CMD_MASK (CMD_BACKUP - 1)
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP

#define MAX_ENTITIES_IN_SNAPSHOT 512                  // was 256 in vanilla Q3A

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct
{
	int snapFlags;                                    // SNAPFLAG_RATE_DELAYED, etc
	int ping;

	int serverTime;                                   // server time the message is valid for (in msec)

	byte areamask[MAX_MAP_AREA_BYTES];                // portalarea visibility bits

	playerState_t ps;                                 // complete information about the current player at this time

	int numEntities;                                  // all of the entities that need to be presented
	entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT]; // at the time of this snapshot

	int numServerCommands;                            // text based server commands to execute when this
	int serverCommandSequence;                        // snapshot becomes current
} snapshot_t;

enum
{
	CGAME_EVENT_NONE,
	CGAME_EVENT_TEAMMENU,
	CGAME_EVENT_SCOREBOARD,
	CGAME_EVENT_EDITHUD
};


/*
==================================================================

functions imported from the main executable

==================================================================
*/

#define CGAME_IMPORT_API_VERSION 7

typedef enum
{
  CG_PRINT,                        // 1
  CG_ERROR,
  CG_MILLISECONDS,
  CG_CVAR_REGISTER,
  CG_CVAR_UPDATE,
  CG_CVAR_SET,
  CG_CVAR_VARIABLESTRINGBUFFER,
  CG_ARGC,
  CG_ARGV,
  CG_ARGS,                        // 10
  CG_FS_FOPENFILE,
  CG_FS_READ,
  CG_FS_WRITE,
  CG_FS_FCLOSEFILE,
  CG_SENDCONSOLECOMMAND,
  CG_ADDCOMMAND,
  CG_SENDCLIENTCOMMAND,
  CG_UPDATESCREEN,
  CG_CM_LOADMAP,
  CG_CM_NUMINLINEMODELS,         // 20
  CG_CM_INLINEMODEL,
  CG_CM_LOADMODEL,
  CG_CM_TEMPBOXMODEL,
  CG_CM_POINTCONTENTS,
  CG_CM_TRANSFORMEDPOINTCONTENTS,
  CG_CM_BOXTRACE,
  CG_CM_TRANSFORMEDBOXTRACE,
  CG_CM_MARKFRAGMENTS,
  CG_S_STARTSOUND,
  CG_S_STARTLOCALSOUND,         // 30
  CG_S_CLEARLOOPINGSOUNDS,
  CG_S_ADDLOOPINGSOUND,
  CG_S_UPDATEENTITYPOSITION,
  CG_S_RESPATIALIZE,
  CG_S_REGISTERSOUND,
  CG_S_STARTBACKGROUNDTRACK,
  CG_R_LOADWORLDMAP,
  CG_R_REGISTERMODEL,
  CG_R_REGISTERSKIN,
  CG_R_REGISTERSHADER,          // 40
  CG_R_REGISTERSHADERNOMIP,
	CG_R_CLEARSCENE,
	CG_R_ADDREFENTITYTOSCENE,
	CG_R_ADDPOLYTOSCENE,
	CG_R_ADDLIGHTTOSCENE,
	CG_R_RENDERSCENE,
	CG_R_SETCOLOR,
	CG_R_DRAWSTRETCHPIC,
	CG_R_MODELBOUNDS,
	CG_R_LERPTAG,
	CG_GETGLCONFIG,               // 50
	CG_GETGAMESTATE,
	CG_GETCURRENTSNAPSHOTNUMBER,
	CG_GETSNAPSHOT,
	CG_GETSERVERCOMMAND,
	CG_GETCURRENTCMDNUMBER,
	CG_GETUSERCMD,
	CG_SETUSERCMDVALUE,
	CG_MEMORY_REMAINING,
	CG_R_REGISTERFONT,            // 60
	CG_KEY_ISDOWN,
	CG_KEY_GETCATCHER,
	CG_KEY_SETCATCHER,
	CG_KEY_GETKEY,
	CG_PC_ADD_GLOBAL_DEFINE,
	CG_PC_LOAD_SOURCE,
	CG_PC_FREE_SOURCE,
	CG_PC_READ_TOKEN,
	CG_PC_SOURCE_FILE_AND_LINE,
	CG_S_STOPBACKGROUNDTRACK,     // 70
	CG_REAL_TIME,
	CG_SNAPVECTOR,
	CG_REMOVECOMMAND,
	CG_R_LIGHTFORPOINT,
	CG_CIN_PLAYCINEMATIC,
	CG_CIN_STOPCINEMATIC,
	CG_CIN_RUNCINEMATIC,
	CG_CIN_DRAWCINEMATIC,
	CG_CIN_SETEXTENTS,
	CG_R_REMAP_SHADER,          // 80
	CG_S_ADDREALLOOPINGSOUND,
	CG_S_STOPLOOPINGSOUND,
	CG_CM_TEMPCAPSULEMODEL,
	CG_CM_CAPSULETRACE,
	CG_CM_TRANSFORMEDCAPSULETRACE,
	CG_GET_ENTITY_TOKEN,
	CG_R_ADDPOLYSTOSCENE,
	CG_R_INPVS,
  CG_FS_SEEK,
	CG_R_REGISTERANIMATION, // 90
  CG_R_REGISTERSHADERLIGHTATTENUATION,
	CG_R_ADDREFLIGHTSTOSCENE,
	CG_R_CHECKSKELETON,
	CG_R_BUILDSKELETON,
	CG_R_BLENDSKELETON,
	CG_R_BONEINDEX,
	CG_R_ANIMNUMFRAMES,
  CG_R_ANIMFRAMERATE,  // 98
  CG_R_ADDADDITIVELIGHTTOSCENE,  // 99
	CG_MEMSET = 100, // 101
	CG_MEMCPY,
	CG_STRNCPY,
	CG_SIN,
	CG_COS,
	CG_ATAN2,
	CG_SQRT,
	CG_FLOOR,
	CG_CEIL,
	CG_TESTPRINTINT,       // 110
	CG_TESTPRINTFLOAT,
	CG_ACOS,
  CG_CM_BISPHERETRACE ,
  CG_CM_TRANSFORMEDBISPHERETRACE,
  CG_FS_GETFILELIST,
  CG_R_DRAWROTATEDPIC,           //FIXME(0xA5EA): unimplemented on cg yet
  CG_R_ADDPOLYBUFFERTOSCENE,    // Added - KRYPTYK

} cgameImport_t;


/*
==================================================================

functions exported to the main executable

==================================================================
*/

typedef enum
{
	CG_INIT,
//	void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum )
	// called when the level loads or when the renderer is restarted
	// all media should be registered at this time
	// cgame will display loading status by calling SCR_Update, which
	// will call CG_DrawInformation during the loading process
	// reliableCommandSequence will be 0 on fresh loads, but higher for
	// demos, tourney restarts, or vid_restarts

	CG_SHUTDOWN,
//	void (*CG_Shutdown)( void );
	// oportunity to flush and close any open files

	CG_CONSOLE_COMMAND,
//	qboolean (*CG_ConsoleCommand)( void );
	// a console command has been issued locally that is not recognized by the
	// main game system.
	// use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the
	// command is not known to the game

	CG_DRAW_ACTIVE_FRAME,
//	void (*CG_DrawActiveFrame)( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );
	// Generates and draws a game scene and status information at the given time.
	// If demoPlayback is set, local movement prediction will not be enabled

	CG_CROSSHAIR_PLAYER,
//	int (*CG_CrosshairPlayer)( void );

	CG_LAST_ATTACKER,
//	int (*CG_LastAttacker)( void );

	CG_KEY_EVENT,
//	void	(*CG_KeyEvent)( int key, qboolean down );

	CG_MOUSE_EVENT,
//	void	(*CG_MouseEvent)( int dx, int dy );
	CG_EVENT_HANDLING
//	void (*CG_EventHandling)(int type);
} cgameExport_t;

//----------------------------------------------
#endif // CG_PUBLIC_H_
