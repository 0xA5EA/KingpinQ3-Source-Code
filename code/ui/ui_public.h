/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-xyyz Lars '0xA5EA' Kandler

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
#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__


#define UI_API_VERSION 8

typedef struct
{
  connstate_t connState;
  int connectPacketCount;
  int clientNum;
  char servername[MAX_STRING_CHARS];
  char updateInfoString[MAX_STRING_CHARS];
  char messageString[MAX_STRING_CHARS];
} uiClientState_t;

typedef enum
{
  UI_ERROR,                     // 1
  UI_PRINT,
  UI_MILLISECONDS,
  UI_CVAR_SET,
  UI_CVAR_VARIABLEVALUE,
  UI_CVAR_VARIABLESTRINGBUFFER,
  UI_CVAR_SETVALUE,
  UI_CVAR_RESET,
  UI_CVAR_CREATE,
  UI_CVAR_INFOSTRINGBUFFER,   // 10
  UI_ARGC,
  UI_ARGV,
  UI_CMD_EXECUTETEXT,
  UI_FS_FOPENFILE,
  UI_FS_READ,
  UI_FS_WRITE,
  UI_FS_FCLOSEFILE,
  UI_FS_GETFILELIST,
	UI_R_REGISTERMODEL,
	UI_R_REGISTERSKIN,        // 20
  UI_R_REGISTERSHADERNOMIP,
  UI_R_CLEARSCENE,
  UI_R_ADDREFENTITYTOSCENE,
  UI_R_ADDPOLYTOSCENE,
  UI_R_ADDLIGHTTOSCENE,
  UI_R_RENDERSCENE,
  UI_R_SETCOLOR,
  UI_R_DRAWSTRETCHPIC,
  UI_UPDATESCREEN,
  UI_CM_LERPTAG,            // 30
  UI_CM_LOADMODEL,
  UI_S_REGISTERSOUND,
  UI_S_STARTLOCALSOUND,
  UI_KEY_KEYNUMTOSTRINGBUF,
  UI_KEY_GETBINDINGBUF,
  UI_KEY_SETBINDING,
  UI_KEY_ISDOWN,
  UI_KEY_GETOVERSTRIKEMODE,
  UI_KEY_SETOVERSTRIKEMODE,
  UI_KEY_CLEARSTATES,       // 40
  UI_KEY_GETCATCHER,
  UI_KEY_SETCATCHER,
  UI_GETCLIPBOARDDATA,
  UI_GETGLCONFIG,
  UI_GETCLIENTSTATE,
  UI_GETCONFIGSTRING,
  UI_LAN_GETPINGQUEUECOUNT,
  UI_LAN_CLEARPING,
  UI_LAN_GETPING,
  UI_LAN_GETPINGINFO,       // 50
  UI_CVAR_REGISTER,
  UI_CVAR_UPDATE,
  UI_MEMORY_REMAINING,

//#ifndef STANDALONE		// 0xA5EA^, bah, hat total verr�ckte bugs verursacht !!!! h�ngt evtl. mit syscalls.asm zusammen
  UI_GET_CDKEY,
  UI_SET_CDKEY,
//#endif
  UI_R_REGISTERFONT,
  UI_R_MODELBOUNDS,
  UI_R_LERPTAG, //add hypov8
  UI_PC_ADD_GLOBAL_DEFINE,
  UI_PC_LOAD_SOURCE,
  UI_PC_FREE_SOURCE,        // 60
  UI_PC_READ_TOKEN,
  UI_PC_SOURCE_FILE_AND_LINE,
  UI_S_STOPBACKGROUNDTRACK,
  UI_S_STARTBACKGROUNDTRACK,
  UI_REAL_TIME,
  UI_LAN_GETSERVERCOUNT,
  UI_LAN_GETSERVERADDRESSSTRING,
  UI_LAN_GETSERVERINFO,
  UI_LAN_MARKSERVERVISIBLE,
  UI_LAN_UPDATEVISIBLEPINGS, // 70
  UI_LAN_RESETPINGS,
  UI_LAN_LOADCACHEDSERVERS,
  UI_LAN_SAVECACHEDSERVERS,
  UI_LAN_ADDSERVER,
  UI_LAN_REMOVESERVER,
  UI_CIN_PLAYCINEMATIC,
  UI_CIN_STOPCINEMATIC,
  UI_CIN_RUNCINEMATIC,
  UI_CIN_DRAWCINEMATIC,
  UI_CIN_SETEXTENTS,        // 80
  UI_R_REMAP_SHADER,
//#ifndef STANDALONE		// 0xA5EA
  UI_VERIFY_CDKEY,
//#endif
  UI_LAN_SERVERSTATUS,
  UI_LAN_GETSERVERPING,
  UI_LAN_SERVERISVISIBLE,
  UI_LAN_COMPARESERVERS,
  // 1.32
  UI_FS_SEEK,  // 87
  UI_R_ADDADDITIVELIGHTTOSCENE, //88

  UI_MEMSET = 100,
  UI_MEMCPY,
  UI_STRNCPY,
  UI_SIN,
  UI_COS,
  UI_ATAN2,
  UI_SQRT,
  UI_FLOOR,
  UI_CEIL,

  UI_GETCPUCONFIG, // 110
  UI_GETGLCONFIG2,
  UI_R_DRAWROTATEDPIC,

  UI_R_REGISTERANIMATION,
  UI_R_BUILDSKELETON,
  UI_R_ANIMNUMFRAMES,
  UI_R_ANIMFRAMERATE,
  UI_R_BONEINDEX, //fixme uneimplemented
  UI_R_RESETSKELETON,
  UI_R_BLENDSKELETON,

  UI_ACOS = 130,
} uiImport_t;

typedef enum
{
  UIMENU_NONE,
  UIMENU_MAIN,
  UIMENU_INGAME,
  UIMENU_NEED_CD,
  UIMENU_BAD_CD_KEY,
  UIMENU_TEAM,
  UIMENU_POSTGAME
} uiMenuCommand_t;

#define SORT_HOST 0
#define SORT_MAP 1
#define SORT_CLIENTS 2
#define SORT_GAME 3
#define SORT_PING 4
#define SORT_PUNKBUSTER 5

typedef enum
{
  UI_GETAPIVERSION = 0,   // system reserved

  UI_INIT,
//	void	UI_Init( void );

  UI_SHUTDOWN,
//	void	UI_Shutdown( void );

  UI_KEY_EVENT,
//	void	UI_KeyEvent( int key );

  UI_MOUSE_EVENT,
//	void	UI_MouseEvent( int dx, int dy );

  UI_REFRESH,
//	void	UI_Refresh( int time );

  UI_IS_FULLSCREEN,
//	qboolean UI_IsFullscreen( void );

  UI_SET_ACTIVE_MENU,
//	void	UI_SetActiveMenu( uiMenuCommand_t menu );

  UI_CONSOLE_COMMAND,
//	qboolean UI_ConsoleCommand( int realTime );
#ifndef STANDALONE // 0xA5EA
  UI_DRAW_CONNECT_SCREEN,
//	void	UI_DrawConnectScreen( qboolean overlay );
  UI_HASUNIQUECDKEY
#else
  UI_DRAW_CONNECT_SCREEN
#endif
// if !overlay, the background will be drawn, otherwise it will be
// overlayed over whatever the cgame has drawn.
// a GetClientState syscall will be made to get the current strings
} uiExport_t;

#endif
