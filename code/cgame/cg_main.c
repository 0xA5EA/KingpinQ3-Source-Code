/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"
#include "../ui/ui_shared.h"

//#define USE_MAP_THREAD

#ifdef USE_MAP_THREAD
#include <boost/thread/thread.hpp>
#endif
// display context for new ui stuff
displayContextDef_t cgDC;

int forceModelModificationCount = -1;
void CG_Shutdown(void);
/*
================
vmMain
This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
#ifdef __cplusplus
extern "C" Q_EXPORT intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
#else
Q_EXPORT intptr_t vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
#endif
{
  switch (command)
  {
  case CG_INIT:
    CG_Init(arg0, arg1, arg2, arg3);
    return 0;

  case CG_SHUTDOWN:
    CG_Shutdown();
    return 0;

  case CG_CONSOLE_COMMAND:
    return CG_ConsoleCommand();

  case CG_DRAW_ACTIVE_FRAME:
    CG_DrawActiveFrame(arg0, (stereoFrame_t)arg1, arg2);
    return 0;

  case CG_CROSSHAIR_PLAYER:
    return CG_CrosshairPlayer();

  case CG_LAST_ATTACKER:
    return CG_LastAttacker();

  case CG_KEY_EVENT:
    CG_KeyEvent(arg0, arg1);
    return 0;

  case CG_MOUSE_EVENT:
    cgDC.cursorx = cgs.cursorX;
    cgDC.cursory = cgs.cursorY;
    CG_MouseEvent(arg0, arg1);
    return 0;

  case CG_EVENT_HANDLING:
    CG_EventHandling(arg0);
    return 0;

  default:
    CG_Error("vmMain: unknown command %i", command);
    break;
  }
  return -1;
}

cg_t cg;
cgs_t cgs;
centity_t cg_entities[MAX_GENTITIES];
weaponInfo_t cg_weapons[MAX_WEAPONS];
itemInfo_t cg_items[MAX_ITEMS];

vmCvar_t cg_railTrailTime;
vmCvar_t cg_centertime;
vmCvar_t cg_runpitch;
vmCvar_t cg_runroll;
vmCvar_t cg_bobup;
vmCvar_t cg_bobpitch;
vmCvar_t cg_bobroll;
vmCvar_t cg_swingSpeed;
vmCvar_t cg_shadows;
vmCvar_t cg_playerShadows; //hypov8 unvan
vmCvar_t cg_gibs;
vmCvar_t cg_drawTimer;
vmCvar_t cg_drawFPS;
vmCvar_t cg_drawSnapshot;
vmCvar_t cg_draw3dIcons;
vmCvar_t cg_drawIcons;
vmCvar_t cg_drawAmmoWarning;
vmCvar_t cg_drawCrosshair;
vmCvar_t cg_drawCrosshairNames;
vmCvar_t cg_drawRewards;
vmCvar_t cg_crosshairSize;
vmCvar_t cg_crosshairX;
vmCvar_t cg_crosshairY;
vmCvar_t cg_crosshairHealth;
vmCvar_t cg_draw2D;
vmCvar_t cg_drawStatus;
vmCvar_t cg_animSpeed;
vmCvar_t cg_debugAnim;
vmCvar_t cg_debugPosition;
vmCvar_t cg_debugEvents;
vmCvar_t cg_errorDecay;
vmCvar_t cg_nopredict;
vmCvar_t cg_noPlayerAnims;
vmCvar_t cg_showmiss;
vmCvar_t cg_footsteps;
vmCvar_t cg_addMarks;
vmCvar_t cg_brassTime;
vmCvar_t cg_viewsize;
vmCvar_t cg_drawGun;
vmCvar_t cg_gun_frame;
vmCvar_t cg_gun_x;
vmCvar_t cg_gun_y;
vmCvar_t cg_gun_z;
vmCvar_t cg_tracerChance;
vmCvar_t cg_tracerWidth;
vmCvar_t cg_tracerLength;
vmCvar_t cg_autoswitch;
vmCvar_t cg_ignore;
vmCvar_t cg_fov;
vmCvar_t cg_zoomFov;
vmCvar_t cg_thirdPerson;
vmCvar_t cg_thirdPersonFixed; //add hypov8 allow camera orbit
vmCvar_t cg_thirdPersonRange;
vmCvar_t cg_thirdPersonAngle;
vmCvar_t cg_stereoSeparation;
vmCvar_t cg_lagometer;
vmCvar_t cg_drawAttacker;
vmCvar_t cg_synchronousClients;
vmCvar_t cg_ChatTime;
vmCvar_t cg_ChatHeight;
vmCvar_t cg_stats;
vmCvar_t cg_buildScript;
vmCvar_t cg_forceModel;
vmCvar_t cg_paused;
vmCvar_t cg_blood;
vmCvar_t cg_predictItems;
vmCvar_t cg_deferPlayers;
vmCvar_t cg_drawTeamOverlay;
vmCvar_t cg_teamOverlayUserinfo;
vmCvar_t cg_drawFriend;
vmCvar_t cg_teamChatsOnly;
vmCvar_t cg_noVoiceChats;
vmCvar_t cg_noVoiceText;
vmCvar_t cg_hudFiles;
vmCvar_t cg_scorePlum;
//unlagged - smooth clients #2
// this is done server-side now
vmCvar_t cg_smoothClients;
//unlagged - smooth clients #2
vmCvar_t pmove_fixed;
vmCvar_t pmove_msec;
vmCvar_t cg_pmove_msec;
vmCvar_t cg_cameraMode;
vmCvar_t cg_cameraOrbit;
vmCvar_t cg_cameraOrbitDelay;
vmCvar_t cg_timescaleFadeEnd;
vmCvar_t cg_timescaleFadeSpeed;
vmCvar_t cg_timescale;
vmCvar_t cg_fontTiny;
vmCvar_t cg_fontSmall;
vmCvar_t cg_fontBig;
vmCvar_t cg_fontHuge;
vmCvar_t cg_noTaunt;
vmCvar_t cg_noProjectileTrail;
vmCvar_t cg_trueLightning;
vmCvar_t cg_larshelp;   /* 0xA5EA */
vmCvar_t cg_larshelp2;  /* 0xA5EA */
vmCvar_t cg_larshelp3;  /* 0xA5EA */
//FIXME(0xA5EA): remove them again

vmCvar_t cg_particles;
vmCvar_t cg_particleCollision;
vmCvar_t cg_DragonTeamName;
vmCvar_t cg_NikkiTeamName;
vmCvar_t cg_currentSelectedPlayer;
vmCvar_t cg_currentSelectedPlayerName;
vmCvar_t cg_enableDust;
vmCvar_t cg_enableBreath;
#ifdef USE_GT_SINGLEPLAYER
vmCvar_t cg_singlePlayer;
vmCvar_t cg_singlePlayerActive;
vmCvar_t cg_recordSPDemo;
vmCvar_t cg_recordSPDemoName;
#endif
//vmCvar_t	cg_obeliskRespawnDelay;
vmCvar_t cg_precomputedLighting;
vmCvar_t cg_debugPlayerAnim;
vmCvar_t cg_debugWeaponAnim;
vmCvar_t cg_animBlend;
vmCvar_t cg_developer;
//unlagged - client options
vmCvar_t	cg_delag;
vmCvar_t	cg_debugDelag;
vmCvar_t	cg_drawBBox;
vmCvar_t	sv_fps;
vmCvar_t	cg_projectileNudge;
vmCvar_t	cg_optimizePrediction;
vmCvar_t	cl_timeNudge;
vmCvar_t	cg_latentCmds;
vmCvar_t	cg_plOut;
//unlagged - client options

typedef struct
{
  vmCvar_t *vmCvar;
  const char *cvarName;
  const char *defaultString;
  int cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] =
{
  {&cg_ignore,                "cg_ignore",               "0", 0},  // used for debugging
  {&cg_autoswitch,            "cg_autoswitch",           "2", CVAR_ARCHIVE}, // hypov8 was 0
  {&cg_drawGun,               "cg_drawGun",              "1", CVAR_ARCHIVE},
  {&cg_zoomFov,               "cg_zoomfov",           "22.5", CVAR_ARCHIVE},
  {&cg_fov,                   "cg_fov",                 "90", CVAR_ARCHIVE},
  {&cg_viewsize,              "cg_viewsize",           "100", CVAR_ARCHIVE},
  {&cg_shadows,               "cg_shadows",              "1", CVAR_ARCHIVE},
  {&cg_playerShadows,         "cg_playerShadows",        "0", CVAR_ARCHIVE}, //hypov8 unvan todo: fix shader tile bug
  {&cg_gibs,                  "cg_gibs",                 "1", CVAR_ARCHIVE},
  {&cg_draw2D,                "cg_draw2D",               "1", CVAR_ARCHIVE},
  {&cg_drawStatus,            "cg_drawStatus",           "1", CVAR_ARCHIVE},
  {&cg_drawTimer,             "cg_drawTimer",            "1", CVAR_ARCHIVE},
  {&cg_drawFPS,               "cg_drawFPS",              "1", CVAR_ARCHIVE},
  {&cg_drawSnapshot,          "cg_drawSnapshot",         "0", CVAR_ARCHIVE},
  {&cg_draw3dIcons,           "cg_draw3dIcons",          "0", CVAR_ARCHIVE}, //hypov8 not needed in kp?
  {&cg_drawIcons,             "cg_drawIcons",            "1", CVAR_ARCHIVE},
  {&cg_drawAmmoWarning,       "cg_drawAmmoWarning",      "1", CVAR_ARCHIVE},
  {&cg_drawAttacker,          "cg_drawAttacker",         "1", CVAR_ARCHIVE},
  {&cg_drawCrosshair,         "cg_drawCrosshair",        "4", CVAR_ARCHIVE},
  {&cg_drawCrosshairNames,    "cg_drawCrosshairNames",   "1", CVAR_ARCHIVE},
  {&cg_drawRewards,           "cg_drawRewards",          "0", CVAR_ARCHIVE},
  {&cg_crosshairSize,         "cg_crosshairSize",       "24", CVAR_ARCHIVE},
  {&cg_crosshairHealth,       "cg_crosshairHealth",      "1", CVAR_ARCHIVE},
  {&cg_crosshairX,            "cg_crosshairX",           "0", CVAR_ARCHIVE},
  {&cg_crosshairY,            "cg_crosshairY",           "0", CVAR_ARCHIVE},
  {&cg_brassTime,             "cg_brassTime",         "2500", CVAR_ARCHIVE},
  {&cg_addMarks,              "cg_marks",                "1", CVAR_ARCHIVE},
  {&cg_lagometer,             "cg_lagometer",            "1", CVAR_ARCHIVE},
  {&cg_railTrailTime,         "cg_railTrailTime",      "400", CVAR_ARCHIVE},
  {&cg_gun_x,                 "cg_gunX",                 "0", CVAR_CHEAT},
  {&cg_gun_y,                 "cg_gunY",                 "0", CVAR_CHEAT},
  {&cg_gun_z,                 "cg_gunZ",                 "0", CVAR_CHEAT},
  {&cg_gun_frame,             "cg_gun_frame",            "0", CVAR_CHEAT},
  {&cg_centertime,            "cg_centertime",           "3", CVAR_CHEAT},
  {&cg_runpitch,              "cg_runpitch",         "0.002", CVAR_ARCHIVE},
  {&cg_runroll,               "cg_runroll",          "0.005", CVAR_ARCHIVE},
  {&cg_bobup,                 "cg_bobup",            "0.005", CVAR_CHEAT},
  {&cg_bobpitch,              "cg_bobpitch",         "0.002", CVAR_ARCHIVE},
  {&cg_bobroll,               "cg_bobroll",          "0.002", CVAR_ARCHIVE},
  {&cg_swingSpeed,            "cg_swingSpeed",         "0.3", CVAR_CHEAT},
  {&cg_animSpeed,             "cg_animspeed",            "1", CVAR_CHEAT},
  {&cg_debugAnim,             "cg_debuganim",            "0", CVAR_CHEAT},
  {&cg_debugPosition,         "cg_debugposition",        "0", CVAR_CHEAT},
  {&cg_debugEvents,           "cg_debugevents",          "0", CVAR_CHEAT},
  {&cg_errorDecay,            "cg_errordecay",         "100",   0},
  {&cg_nopredict,             "cg_nopredict",           "0", 0},
  {&cg_noPlayerAnims,         "cg_noplayeranims",       "0", CVAR_CHEAT},
  {&cg_showmiss,              "cg_showmiss",            "0", 0},
  {&cg_footsteps,             "cg_footsteps",           "1", CVAR_CHEAT},
  {&cg_tracerChance,          "cg_tracerchance",      "0.4", CVAR_CHEAT},
  {&cg_tracerWidth,           "cg_tracerwidth",         "1", CVAR_CHEAT},
  {&cg_tracerLength,          "cg_tracerlength",      "100", CVAR_CHEAT},
  {&cg_thirdPersonFixed,      "cg_thirdPersonFixed",    "1", CVAR_CHEAT },	//hypov8 allow 3rd person to rotate camera freely
  {&cg_thirdPersonRange,      "cg_thirdPersonRange",   "40", CVAR_CHEAT},
  {&cg_thirdPersonAngle,      "cg_thirdPersonAngle",    "0", CVAR_CHEAT},
  {&cg_thirdPerson,           "cg_thirdPerson",         "0", 0},
  {&cg_ChatTime,              "cg_ChatTime",        "15000", CVAR_ARCHIVE},
  {&cg_ChatHeight,            "cg_ChatHeight",         "10", CVAR_ARCHIVE},
  {&cg_forceModel,            "cg_forceModel",          "0", CVAR_ARCHIVE},
  {&cg_predictItems,          "cg_predictItems",        "1", CVAR_ARCHIVE},
  {&cg_particles,             "cg_particles",           "1", CVAR_ARCHIVE},
  {&cg_particleCollision,     "cg_particleCollision",   "0", CVAR_ARCHIVE},
  {&cg_deferPlayers,          "cg_deferPlayers",          "0", CVAR_ARCHIVE},
  {&cg_drawTeamOverlay,       "cg_drawTeamOverlay",          "1", CVAR_ARCHIVE},
  {&cg_teamOverlayUserinfo,   "teamoverlay",          "0", CVAR_ROM | CVAR_USERINFO},
  {&cg_stats,                 "cg_stats",                "0", 0},
  {&cg_drawFriend,            "cg_drawFriend",          "1", CVAR_ARCHIVE},
  {&cg_teamChatsOnly,         "cg_teamChatsOnly",          "0", CVAR_ARCHIVE},
  {&cg_noVoiceChats,          "cg_noVoiceChats",          "0", CVAR_ARCHIVE},
  {&cg_noVoiceText,           "cg_noVoiceText",          "0", CVAR_ARCHIVE},
  // the following variables are created in other parts of the system,
  // but we also reference them here
  {&cg_buildScript,           "com_buildScript",          "0", 0},               // force loading of all possible data amd error on failures
  {&cg_paused,                "cl_paused",                "0", CVAR_ROM},
  {&cg_blood,                 "com_blood",                "1", CVAR_ARCHIVE},
  {&cg_synchronousClients,    "g_synchronousClients",          "0", CVAR_SYSTEMINFO},   // communicated by systeminfo
  {&cg_DragonTeamName,        "g_dragonTeam",         TEAM_NAME_DRAGONS, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO},
  {&cg_NikkiTeamName,         "g_nikkiTeam",          TEAM_NAME_NIKKIS, CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_USERINFO},
  {&cg_currentSelectedPlayer, "cg_currentSelectedPlayer",          "0", CVAR_ARCHIVE},
  {&cg_currentSelectedPlayerName, "cg_currentSelectedPlayerName",          "", CVAR_ARCHIVE},
  {&cg_enableDust,            "g_enableDust",               "0", CVAR_SERVERINFO},
  {&cg_enableBreath,          "g_enableBreath",             "0", CVAR_SERVERINFO},
  {&cg_hudFiles,              "cg_hudFiles",       DFLT_HUDFILE, CVAR_ARCHIVE},
  {&cg_cameraOrbit,           "cg_cameraOrbit",             "0", CVAR_CHEAT},
  {&cg_cameraOrbitDelay,      "cg_cameraOrbitDelay",       "50", CVAR_ARCHIVE},
  {&cg_timescaleFadeEnd,      "cg_timescaleFadeEnd",       "1", 0},
  {&cg_timescaleFadeSpeed,    "cg_timescaleFadeSpeed",     "0", 0},
  {&cg_timescale,             "timescale",                 "1", 0},
  {&cg_scorePlum,             "cg_scorePlums",             "1", CVAR_USERINFO | CVAR_ARCHIVE},
  {&cg_smoothClients,         "cg_smoothClients",          "0", CVAR_USERINFO | CVAR_ARCHIVE},
//unlagged - smooth clients #2
// this is done server-side now
  {&cg_smoothClients,         "cg_smoothClients",          "0", CVAR_USERINFO | CVAR_ARCHIVE},
//unlagged - smooth clients #2
  {&cg_cameraMode,            "com_cameraMode",            "0", CVAR_CHEAT},
  {&pmove_fixed,              "pmove_fixed",               "0", CVAR_SYSTEMINFO},
  {&pmove_msec,               "pmove_msec",                "8", CVAR_SYSTEMINFO},
  {&pmove_fixed,              "pmove_fixed",               "0", CVAR_SYSTEMINFO}, //unvan .52 
  {&cg_noTaunt,               "cg_noTaunt",                "0", CVAR_ARCHIVE},
  {&cg_noProjectileTrail,     "cg_noProjectileTrail",          "0", CVAR_ARCHIVE},
  {&cg_fontTiny,              "ui_fontTiny",             "0.19", CVAR_ARCHIVE},
  {&cg_fontSmall,             "ui_fontSmall",            "0.27", CVAR_ARCHIVE},
  {&cg_fontBig,               "ui_fontBig",              "0.38", CVAR_ARCHIVE},
  {&cg_fontHuge,              "ui_fontHuge",             "0.45", CVAR_ARCHIVE},
  {&cg_precomputedLighting,   "r_precomputedLighting",      "1", 0}, // hypov8 was 0 cheat? shouldent be a cvar!!!
  {&cg_debugPlayerAnim,       "cg_debugPlayerAnim",         "0", CVAR_CHEAT},
  {&cg_debugWeaponAnim,       "cg_debugWeaponAnim",         "0", CVAR_CHEAT},
  {&cg_animBlend,             "cg_animblend",             "5.0", CVAR_ARCHIVE},
  {&cg_trueLightning,         "cg_trueLightning",         "0.0", CVAR_ARCHIVE},
  {&cg_developer,             "developer",                  "0", 0},

//unlagged - client options
  { &cg_delag,               "cg_delag",                    "1", CVAR_ARCHIVE | CVAR_USERINFO },
  { &cg_debugDelag,          "cg_debugDelag",               "0", CVAR_USERINFO | CVAR_CHEAT },
  { &cg_drawBBox,            "cg_drawBBox",                 "0", CVAR_CHEAT },
  // this will be automagically copied from the server
  { &sv_fps,                 "sv_fps",                     "20", CVAR_SYSTEMINFO}, //hypov8 note: not used anymore
  { &cg_projectileNudge,     "cg_projectileNudge",          "0", CVAR_ARCHIVE },
  { &cg_optimizePrediction,  "cg_optimizePrediction",       "1", CVAR_ARCHIVE },
  { &cl_timeNudge,           "cl_timeNudge",                "0", CVAR_ARCHIVE },
  { &cg_latentCmds,          "cg_latentCmds",               "1", CVAR_USERINFO | CVAR_CHEAT }, //hypov8 was 0. lag simulation??
  { &cg_plOut,               "cg_plOut",                    "0", CVAR_USERINFO | CVAR_CHEAT },
//unlagged - client options

 {&cg_larshelp,              "cg_larshelp",                "0", CVAR_ARCHIVE},
 {&cg_larshelp2,             "cg_larshelp2",               "0", CVAR_ARCHIVE},
 {&cg_larshelp3,             "cg_larshelp3",               "0", CVAR_ARCHIVE},
  //FIXME(0xA5EA): removed this cvars again
#ifdef USE_GT_SINGLEPLAYER
  {&cg_recordSPDemo,          "ui_recordSPDemo",          "0", CVAR_ARCHIVE},
  {&cg_recordSPDemoName,      "ui_recordSPDemoName",          "", CVAR_ARCHIVE},
  {&cg_singlePlayer,          "ui_singlePlayerActive",          "0", CVAR_USERINFO},
  {&cg_singlePlayerActive,    "ui_singlePlayerActive",          "0", CVAR_USERINFO}
#endif
};

static const int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars(void)
{
  int i;
  cvarTable_t *cv;
  char var[MAX_TOKEN_CHARS];

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
  {
    trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags);
  }

  // see if we are also running the server on this machine
  trap_Cvar_VariableStringBuffer("sv_running", var, sizeof(var));
  cgs.localServer = atoi(var);

  forceModelModificationCount = cg_forceModel.modificationCount;

  trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE);
  trap_Cvar_Register(NULL, "headmodel", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE);
  trap_Cvar_Register(NULL, "team_model", DEFAULT_TEAM_MODEL, CVAR_USERINFO | CVAR_ARCHIVE);
  trap_Cvar_Register(NULL, "team_headmodel", DEFAULT_TEAM_HEAD, CVAR_USERINFO | CVAR_ARCHIVE);
}

/*
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange(void)
{
  int i;

  for (i = 0; i < MAX_CLIENTS; i++)
  {
    const char *clientInfo;

    clientInfo = CG_ConfigString(CS_PLAYERS + i);

    if (!clientInfo[0])
      continue;

    CG_NewClientInfo(i);
  }
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars(void)
{
  int i;
  cvarTable_t *cv;

  for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++)
  {
		if ( cv->vmCvar ) {
			trap_Cvar_Update(cv->vmCvar);
		}
  }

  // check for modications here
  // If team overlay is on, ask for updates from the server.  If its off,
  // let the server know so we don't receive it
  if (drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount)
  {
    drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

    if (cg_drawTeamOverlay.integer > 0)
      trap_Cvar_Set("teamoverlay", "1");
    else
      trap_Cvar_Set("teamoverlay", "0");

    // FIXME E3 HACK
    trap_Cvar_Set("teamoverlay", "1");
  }

  // if force model changed
  if (forceModelModificationCount != cg_forceModel.modificationCount)
  {
    forceModelModificationCount = cg_forceModel.modificationCount;
    CG_ForceModelChange();
  }
}

int CG_CrosshairPlayer(void)
{
  if (cg.time > (cg.crosshairClientTime + 1000))
    return -1;

  return cg.crosshairClientNum;
}

int CG_LastAttacker(void)
{
  shortbytes_t attacker;

  if (!cg.attackerTime)
    return -1;

  attacker.s = cg.snap->ps.persistant[PERS_ATTACKER_INFO];
  return attacker.b[PERS_ATTACKER_POS];
}

void QDECL CG_Printf(const char *msg, ...)
{
  va_list argptr;
  char text[1024] = { '\0' };

  va_start(argptr, msg);
  Q_vsnprintf(text, sizeof(text), msg, argptr);
  va_end(argptr);

  trap_Print(text);
}

void QDECL CG_Error(const char *msg, ...)
{
  va_list argptr;
  char text[1024] = { '\0' };

  va_start(argptr, msg);
  Q_vsnprintf(text, sizeof(text), msg, argptr);
  va_end(argptr);

  trap_Error(text);
}

void QDECL Com_Error(int level, const char *error, ...)
{
  va_list argptr;
  char text[1024] = { '\0' };

  va_start(argptr, error);
  Q_vsnprintf(text, sizeof(text), error, argptr);
  va_end(argptr);

  CG_Error("%s", text);
}

void QDECL Com_Printf(const char *msg, ...)
{
  va_list argptr;
  char text[1024];

  va_start(argptr, msg);
  Q_vsnprintf(text, sizeof(text), msg, argptr);
  va_end(argptr);

  CG_Printf("%s", text);
}

void QDECL Com_DPrintf(const char *msg, ...)
{
  if (cg_developer.integer)
  {
    va_list argptr;
    char text[1024];
    va_start(argptr, msg);
    Q_vsnprintf(text, sizeof(text), msg, argptr);
    va_end(argptr);
    CG_Printf("%s", text);
  }
}

/*
================
CG_Argv
================
*/
const char *CG_Argv(int arg)
{
  static char buffer[MAX_STRING_CHARS];
  trap_Argv(arg, buffer, sizeof(buffer));
  return buffer;
}

/*
=================
CG_RegisterItemSounds
The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds(int itemNum)
{
  const gitem_t *item = 0;
  char data[MAX_QPATH];
  char const *s, *start;
  int len;

  item = &bg_itemlist[itemNum];

  if (item->pickup_sound)
    trap_S_RegisterSound(item->pickup_sound, qfalse);

  // parse the space seperated precache string for other media
  s = item->sounds;

  if (!s || !s[0])
    return;

  while (*s)
  {
    start = s;
    while (*s && *s != ' ')
      s++;

    len = s - start;
    if (len >= MAX_QPATH || len < 5)
    {
      CG_Error("PrecacheItem: %s has bad precache string", item->classname);
      return;
    }

    Com_Memcpy(data, start, len);

    data[len] = 0;

    if (*s)
      s++;

    if (!qstrcmp(data + len - 3, "wav") || !qstrcmp(data + len - 3, "ogg"))
      trap_S_RegisterSound(data, qfalse);

  }
}

/*
=================
CG_RegisterSounds
called during a precache command
=================
*/
static void CG_RegisterSounds(void)
{
  int i;
  char items[MAX_ITEMS + 1];
  char name[MAX_QPATH];
  const char *soundName;

  // voice commands
  CG_LoadVoiceChats();
  CG_LoadingString("feedback", qfalse);
  cgs.media.oneMinuteSound    = trap_S_RegisterSound("sound/feedback/1_minute.ogg", qtrue);
  cgs.media.fiveMinuteSound   = trap_S_RegisterSound("sound/feedback/5_minute.ogg", qtrue);
  cgs.media.suddenDeathSound  = trap_S_RegisterSound("sound/feedback/sudden_death.ogg", qtrue);
  cgs.media.oneFragSound      = trap_S_RegisterSound("sound/feedback/1_frag.ogg", qtrue);
  cgs.media.twoFragSound      = trap_S_RegisterSound("sound/feedback/2_frags.ogg", qtrue);
  cgs.media.threeFragSound    = trap_S_RegisterSound("sound/feedback/3_frags.ogg", qtrue);
  cgs.media.count3Sound       = trap_S_RegisterSound("sound/feedback/three.ogg", qtrue);
  cgs.media.count2Sound       = trap_S_RegisterSound("sound/feedback/two.ogg", qtrue);
  cgs.media.count1Sound       = trap_S_RegisterSound("sound/feedback/one.ogg", qtrue);
  cgs.media.countFightSound   = trap_S_RegisterSound("sound/feedback/fight.ogg", qtrue);
  cgs.media.countPrepareSound = trap_S_RegisterSound("sound/feedback/prepare.ogg", qtrue);
  //FIXME(0xA5EA): this is still same sound
  cgs.media.countPrepareTeamSound = trap_S_RegisterSound("sound/feedback/prepare.ogg", qtrue);
  CG_LoadingString("team sounds", qfalse);
  if (cgs.gametype >= GT_TEAM || cg_buildScript.integer)
  {
    cgs.media.captureAwardSound = trap_S_RegisterSound("sound/teamplay/flagcapture_yourteam.ogg", qtrue);
    cgs.media.redLeadsSound     = trap_S_RegisterSound("sound/feedback/redleads.ogg", qtrue);
    cgs.media.blueLeadsSound    = trap_S_RegisterSound("sound/feedback/blueleads.ogg", qtrue);
    cgs.media.teamsTiedSound    = trap_S_RegisterSound("sound/feedback/teamstied.ogg", qtrue);
#ifdef USE_EXTENTED_HIT_SOUNDS
    cgs.media.hitTeamSound      = trap_S_RegisterSound("sound/feedback/hit_teammate.ogg", qtrue);
#endif
    cgs.media.redScoredSound  = trap_S_RegisterSound("sound/teamplay/voc_red_scores.ogg", qtrue);
    cgs.media.blueScoredSound = trap_S_RegisterSound("sound/teamplay/voc_blue_scores.ogg", qtrue);

    cgs.media.captureYourTeamSound = trap_S_RegisterSound("sound/teamplay/SMALL_BE.ogg", qtrue);
    cgs.media.captureOpponentSound = trap_S_RegisterSound("sound/teamplay/ering.ogg", qtrue);

    cgs.media.returnYourTeamSound = trap_S_RegisterSound("sound/teamplay/clochedeglise.ogg", qtrue);
    cgs.media.returnOpponentSound = trap_S_RegisterSound("sound/teamplay/clochedeglise.ogg", qtrue);

    cgs.media.takenYourTeamSound = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);
    cgs.media.takenOpponentSound = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);

    if (cgs.gametype == GT_CTF || cg_buildScript.integer)
    {
      cgs.media.redFlagReturnedSound       = trap_S_RegisterSound("sound/teamplay/voc_red_returned.ogg", qtrue);
      cgs.media.blueFlagReturnedSound      = trap_S_RegisterSound("sound/teamplay/voc_blue_returned.ogg", qtrue);
      cgs.media.enemyTookYourFlagSound     = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);
      cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);
    }

    if (cgs.gametype == GT_1FCTF || cg_buildScript.integer)
    {
      // FIXME: get a replacement for this sound ?
      cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/flagreturn_opponent.ogg", qtrue);
      cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_1flag.ogg", qtrue);
      cgs.media.enemyTookTheFlagSound    = trap_S_RegisterSound("sound/teamplay/voc_enemy_1flag.ogg", qtrue);
    }
    //FIXME (0xA5EA): cleanup
    if (cgs.gametype == GT_1FCTF || cgs.gametype == GT_CTF || cg_buildScript.integer)
    {
      cgs.media.youHaveFlagSound = trap_S_RegisterSound("sound/teamplay/voc_you_flag.ogg", qtrue);
      cgs.media.holyShitSound    = trap_S_RegisterSound("sound/feedback/voc_holyshit.ogg", qtrue);
    }
#ifdef WITH_BAGMAN_MOD
	if (cgs.gametype == GT_BAGMAN)
	{
		cgs.media.bmDepositSound = trap_S_RegisterSound("sound/teamplay/SMALL_BE.ogg", qtrue);
		cgs.media.bmStolenSound = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);
		//cgs.media.yourTeamTookEnemyBagSound = trap_S_RegisterSound("sound/teamplay/cashmatch_alarm3.ogg", qtrue);
		//cgs.media.captureYourTeamSound = trap_S_RegisterSound("sound/teamplay/SMALL_BE.ogg", qtrue);
	}
#endif


#ifdef GT_USE_TA_TYPES
    if (cgs.gametype == GT_OBELISK || cg_buildScript.integer)
    {
      cgs.media.yourBaseIsUnderAttackSound = trap_S_RegisterSound("sound/teamplay/voc_base_attack.ogg", qtrue);
    }
#endif
    cgs.media.youHaveFlagSound         = trap_S_RegisterSound("sound/teamplay/voc_you_flag.ogg", qtrue);
    cgs.media.holyShitSound            = trap_S_RegisterSound("sound/feedback/voc_holyshit.ogg", qtrue);
    cgs.media.neutralFlagReturnedSound = trap_S_RegisterSound("sound/teamplay/ering.ogg", qtrue);
    cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound("sound/teamplay/voc_team_1flag.ogg", qtrue);
    cgs.media.enemyTookTheFlagSound    = trap_S_RegisterSound("sound/teamplay/voc_enemy_1flag.ogg", qtrue);

  }
  CG_LoadingString("ambient sounds", qfalse);
  cgs.media.tracerSound = trap_S_RegisterSound("sound/weapons/ric8.ogg", qfalse);
//	cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.ogg", qfalse ); /* 0xA5EA, removed weaponchange sound */
  cgs.media.wearOffSound    = trap_S_RegisterSound("sound/items/wearoff.ogg", qfalse);
  cgs.media.useNothingSound = trap_S_RegisterSound("sound/items/use_nothing.ogg", qfalse);
  //cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.ogg", qfalse );		 /* 0xA5EA */
  cgs.media.gibSound = trap_S_RegisterSound("sound/actors/player/bodyfalls/jibs.ogg", qfalse);      /* 0xA5EA */
//	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/actors/player/bodyfalls/jibs.ogg", qfalse );	/* 0xA5EA */
  cgs.media.gibBounce1Sound = trap_S_RegisterSound("sound/player/gibimp1.ogg", qfalse);             /* 0xA5EA */
  cgs.media.gibBounce2Sound = trap_S_RegisterSound("sound/player/gibimp2.ogg", qfalse);
  cgs.media.gibBounce3Sound = trap_S_RegisterSound("sound/player/gibimp3.ogg", qfalse);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              // 0xA5EA
  //cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.ogg", qfalse );
  cgs.media.teleInSound  = trap_S_RegisterSound("sound/misc/tele1.ogg", qfalse);
  cgs.media.teleOutSound = trap_S_RegisterSound("sound/world/teleout.ogg", qfalse);
  cgs.media.respawnSound = trap_S_RegisterSound("sound/items/respawn1.ogg", qfalse);                /* itemrespawnsound */
  cgs.media.noAmmoSound = trap_S_RegisterSound("sound/weapons/noammo.ogg", qfalse);
  cgs.media.talkSound = trap_S_RegisterSound("sound/misc/talk.ogg", qfalse);                        /* 0xA5EA */
  cgs.media.hitSound = trap_S_RegisterSound("sound/feedback/hit.ogg", qfalse);                      /* seltsames beep aus q3 */
  CG_LoadingString("medal sounds", qfalse);
  cgs.media.impressiveSound  = trap_S_RegisterSound("sound/feedback/impressive.ogg", qtrue);
  cgs.media.excellentSound   = trap_S_RegisterSound("sound/feedback/excellent.ogg", qtrue);
  cgs.media.deniedSound      = trap_S_RegisterSound("sound/feedback/denied.ogg", qtrue);
  cgs.media.humiliationSound = trap_S_RegisterSound("sound/feedback/humiliation.ogg", qtrue);
  cgs.media.assistSound      = trap_S_RegisterSound("sound/feedback/assist.ogg", qtrue);
  cgs.media.defendSound      = trap_S_RegisterSound("sound/feedback/defense.ogg", qtrue);
                                                                                                                                                                                                                                                                                                                                                               // 0xA5EA
  cgs.media.takenLeadSound = trap_S_RegisterSound("sound/feedback/takenlead.ogg", qtrue);
  cgs.media.tiedLeadSound  = trap_S_RegisterSound("sound/feedback/tiedlead.ogg", qtrue);
  cgs.media.lostLeadSound  = trap_S_RegisterSound("sound/feedback/lostlead.ogg", qtrue);

  //FIXME(0xA5EA): this sounds are still missing
  cgs.media.voteNow = trap_S_RegisterSound( "sound/feedback/vote_now.ogg", qtrue);
  cgs.media.votePassed = trap_S_RegisterSound( "sound/feedback/vote_passed.ogg", qtrue);
  cgs.media.voteFailed = trap_S_RegisterSound( "sound/feedback/vote_failed.ogg", qtrue);

  cgs.media.watrInSound  = trap_S_RegisterSound("sound/actors/player/step water/water_in.ogg", qfalse);
  cgs.media.watrOutSound = trap_S_RegisterSound("sound/actors/player/step water/water_out.ogg", qfalse);
  cgs.media.watrUnSound  = trap_S_RegisterSound("sound/actors/player/male/water_under.ogg", qfalse); //hypov8 merge: melee??
  cgs.media.jumpPadSound = trap_S_RegisterSound("sound/world/jumppad.ogg", qfalse);

  cgs.media.sfx_Pipehbody = trap_S_RegisterSound("sound/weapons/melee/pipehitbody.ogg", qfalse);
  cgs.media.sfx_Pipehcboard = trap_S_RegisterSound("sound/weapons/melee/pipehitcboard.ogg", qfalse);
  cgs.media.sfx_Pipehcement = trap_S_RegisterSound("sound/weapons/melee/pipehitcement.ogg", qfalse);
  cgs.media.sfx_Pipehgravel = trap_S_RegisterSound("sound/weapons/melee/pipehitgravel.ogg", qfalse);
  cgs.media.sfx_Pipehwood = trap_S_RegisterSound("sound/weapons/melee/pipehitwood.ogg", qfalse);
  cgs.media.sfx_Pipehhead = trap_S_RegisterSound("sound/weapons/melee/pipehithead.ogg", qfalse);
  cgs.media.sfx_Pipehtile = trap_S_RegisterSound("sound/weapons/melee/pipehittile.ogg", qfalse);
  cgs.media.sfx_Pipehtin = trap_S_RegisterSound("sound/weapons/melee/pipehittin.ogg", qfalse);

  cgs.media.sfx_HookHitBrick = trap_S_RegisterSound("sound/weapons/melee/pipehitcement.ogg", qfalse); //hypov8 todo: WP_GRAPPLING_HOOK

  CG_LoadingString("footsteps", qfalse);
  for (i = 0; i < 4; i++)
  {
    Com_sprintf(name, sizeof(name), "sound/actors/player/step pavement/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step water/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound(name, qfalse);         /* water */

    Com_sprintf(name, sizeof(name), "sound/actors/player/step metal heavy/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step metal light/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_METALLIGHT][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step gravel/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_GRAVEL][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step rug/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_RUG][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step marble/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_MARBLE][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step puddle/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_PUDDLE][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step wood/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_WOOD][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step tin/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_TIN][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step snow/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_SNOW][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step crass/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_CRASS][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step ladder/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_LADDER][i] = trap_S_RegisterSound(name, qfalse);

    Com_sprintf(name, sizeof(name), "sound/actors/player/step glass/step%i.ogg", i + 1);
    cgs.media.footsteps[FOOTSTEP_GLASS][i] = trap_S_RegisterSound(name, qfalse);
  }
	//ToDo: duplicate sounds
	//also hypov8 why is there spaces in file names?
  cgs.media.landSounds[FOOTSTEP_NORMAL] = trap_S_RegisterSound("sound/actors/player/step pavement/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_BOOT] = cgs.media.landSounds[FOOTSTEP_NORMAL];
  cgs.media.landSounds[FOOTSTEP_FLESH] = cgs.media.landSounds[FOOTSTEP_NORMAL];
  cgs.media.landSounds[FOOTSTEP_MECH] = cgs.media.landSounds[FOOTSTEP_NORMAL];
  cgs.media.landSounds[FOOTSTEP_ENERGY] = cgs.media.landSounds[FOOTSTEP_NORMAL];
  cgs.media.landSounds[FOOTSTEP_METAL] = trap_S_RegisterSound("sound/actors/player/step metal heavy/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_SPLASH] = trap_S_RegisterSound("sound/actors/player/step water/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_METAL] = trap_S_RegisterSound("sound/actors/player/step metal heavy/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_METALLIGHT] = trap_S_RegisterSound("sound/actors/player/step metal light/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_PUDDLE] = trap_S_RegisterSound("sound/actors/player/step puddle/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_RUG] = trap_S_RegisterSound("sound/actors/player/step rug/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_SNOW] = trap_S_RegisterSound("sound/actors/player/step snow/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_TIN] = trap_S_RegisterSound("sound/actors/player/step tin/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_MARBLE] = trap_S_RegisterSound("sound/actors/player/step marble/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_WOOD] = trap_S_RegisterSound("sound/actors/player/step wood/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_GLASS] = trap_S_RegisterSound("sound/actors/player/step glass/land1.ogg", qfalse);
  cgs.media.landSounds[FOOTSTEP_LADDER] = cgs.media.landSounds[FOOTSTEP_TIN]; //hypov8 tin

  CG_LoadingString("item sounds", qfalse);
  // only register the items that the server says we need
  qstrcpy(items, CG_ConfigString(CS_ITEMS));

  for (i = 1; i < bg_numItems; i++)
  {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
    CG_RegisterItemSounds(i);
//		}
    //FIXME(0xA5EA): wtf is this ?
  }

  for (i = 1; i < MAX_SOUNDS; i++)
  {
    soundName = CG_ConfigString(CS_SOUNDS + i);
    if (!soundName[0])
      break;

    if (soundName[0] == '*')
      continue;   // custom sound

    cgs.gameSounds[i] = trap_S_RegisterSound(soundName, qfalse);
  }

  // FIXME: only needed with item
#ifdef GT_USE_TA_TYPES
  cgs.media.flightSound = trap_S_RegisterSound("sound/items/flight.ogg", qfalse);
#endif
  cgs.media.medkitSound = trap_S_RegisterSound("sound/items/use_medkit.ogg", qfalse);

  cgs.media.sfx_ric[0] = trap_S_RegisterSound("sound/weapons/ric1.ogg", qfalse);
  cgs.media.sfx_ric[1] = trap_S_RegisterSound("sound/weapons/ric2.ogg", qfalse);
  cgs.media.sfx_ric[2] = trap_S_RegisterSound("sound/weapons/ric3.ogg", qfalse);
  cgs.media.sfx_ric[3] = trap_S_RegisterSound("sound/weapons/ric4.ogg", qfalse);
  cgs.media.sfx_ric[4] = trap_S_RegisterSound("sound/weapons/ric5.ogg", qfalse);
  cgs.media.sfx_ric[5] = trap_S_RegisterSound("sound/weapons/ric6.ogg", qfalse);
  cgs.media.sfx_ric[6] = trap_S_RegisterSound("sound/weapons/ric7.ogg", qfalse);
  cgs.media.sfx_ric[7] = trap_S_RegisterSound("sound/weapons/ric8.ogg", qfalse);

  //cgs.media.sfx_BullethWood[0] = trap_S_RegisterSound ("sound/weapons/bullethit_wood1.ogg", qfalse);
  cgs.media.sfx_BullethWood[0] = trap_S_RegisterSound("sound/weapons/impactwood1.ogg", qfalse);
  cgs.media.sfx_BullethWood[1] = trap_S_RegisterSound("sound/weapons/impactwood2.ogg", qfalse);
  cgs.media.sfx_BullethWood[2] = trap_S_RegisterSound("sound/weapons/impactwood3.ogg", qfalse);
  cgs.media.sfx_BullethWood[3] = trap_S_RegisterSound("sound/weapons/impactwood4.ogg", qfalse);
  cgs.media.sfx_BullethWood[4] = trap_S_RegisterSound("sound/weapons/impactwood5.ogg", qfalse);
  cgs.media.sfx_BullethWood[5] = trap_S_RegisterSound("sound/weapons/impactwood6.ogg", qfalse);
  cgs.media.sfx_BullethWood[6] = trap_S_RegisterSound("sound/weapons/impactwood7.ogg", qfalse);
  //cgs.media.sfx_BullethWood[1] = trap_S_RegisterSound ("sound/weapons/bullethit_wood2.ogg", qfalse);
  //cgs.media.sfx_BullethWood[2] = trap_S_RegisterSound ("sound/weapons/bullethit_wood3.ogg", qfalse);
  //cgs.media.sfx_BullethWood[3] = trap_S_RegisterSound ("sound/weapons/impactMetal1.ogg", qfalse);
#if 0
  cgs.media.sfx_smallBullethWood[0] = trap_S_RegisterSound("sound/weapons/smallBullethitWood1.ogg", qfalse);
  cgs.media.sfx_smallBullethWood[1] = trap_S_RegisterSound("sound/weapons/smallBullethitWood0.ogg", qfalse);
#endif
  cgs.media.sfx_BullethIce[0] = trap_S_RegisterSound("sound/weapons/bullethit_ice1.ogg", qfalse);
  cgs.media.sfx_BullethIce[1] = trap_S_RegisterSound("sound/weapons/bullethit_ice2.ogg", qfalse);

  cgs.media.sfx_BullethMetal[0] = trap_S_RegisterSound("sound/weapons/bullethit_tin.ogg", qfalse);
  cgs.media.sfx_BullethMetal[1] = trap_S_RegisterSound("sound/weapons/bullethit_tin2.ogg", qfalse);
  cgs.media.sfx_BullethMetal[2] = trap_S_RegisterSound("sound/weapons/bullethit_tin3.ogg", qfalse);
  cgs.media.sfx_BullethMetal[3] = trap_S_RegisterSound("sound/weapons/bullethit_tin4.ogg", qfalse);
  cgs.media.sfx_BullethMetal[4] = trap_S_RegisterSound("sound/weapons/impactMetal2.ogg",  qfalse);
  cgs.media.sfx_BullethMetal[5] = trap_S_RegisterSound("sound/weapons/impactMetal3.ogg",  qfalse);
  cgs.media.sfx_BullethMetal[6] = trap_S_RegisterSound("sound/weapons/impactMetal4.ogg",  qfalse);
//	cgs.media.sfx_BullethMetal[4] = trap_S_RegisterSound ("sound/weapons/bullethit_tin5.ogg", qfalse);
//	cgs.media.sfx_BullethMetal[5] = trap_S_RegisterSound ("sound/weapons/bigBullethitMetal1.ogg", qfalse);

  cgs.media.sfx_BullethEarth[0] = trap_S_RegisterSound("sound/weapons/bullethit_earth1.ogg", qfalse);
  cgs.media.sfx_BullethEarth[1] = trap_S_RegisterSound("sound/weapons/bullethit_earth2.ogg", qfalse);
  cgs.media.sfx_BullethEarth[2] = trap_S_RegisterSound("sound/weapons/bullethit_earth3.ogg", qfalse);

  cgs.media.sfx_ShottyhitMetal[2] = trap_S_RegisterSound("sound/weapons/bullethit_tin.ogg", qfalse);                   /* 0xA5EA */
  cgs.media.sfx_ShottyhitMetal[1] = trap_S_RegisterSound("sound/weapons/bullethit_tin2.ogg", qfalse);                  /* 0xA5EA */
  cgs.media.sfx_ShottyhitMetal[0] = trap_S_RegisterSound("sound/weapons/bullethit_tin3.ogg", qfalse);                  /* 0xA5EA */
  //cgs.media.sfx_ShottyhitMetal[2] = trap_S_RegisterSound ("sound/weapons/bullethit_tin4.ogg", qfalse);	/* 0xA5EA */

  cgs.media.sfx_Shottyhit[0] = trap_S_RegisterSound("sound/weapons/shottyric1.ogg", qfalse);                           /* 0xA5EA */
  cgs.media.sfx_Shottyhit[1] = trap_S_RegisterSound("sound/weapons/shottyric2.ogg", qfalse);                           /* 0xA5EA */
  cgs.media.sfx_Shottyhit[2] = trap_S_RegisterSound("sound/weapons/shottyric3.ogg", qfalse);                           /* 0xA5EA */
  cgs.media.sfx_Shottyhit[3] = trap_S_RegisterSound("sound/weapons/shottyric4.ogg", qfalse);
  cgs.media.sfx_Shottyhit[4] = trap_S_RegisterSound("sound/weapons/shottyric5.ogg", qfalse);
  cgs.media.sfx_Shottyhit[5] = trap_S_RegisterSound("sound/weapons/shottyric6.ogg", qfalse);
  cgs.media.sfx_Shottyhit[6] = trap_S_RegisterSound("sound/weapons/shottyric7.ogg", qfalse);
  cgs.media.sfx_Shottyhit[7] = trap_S_RegisterSound("sound/weapons/shottyric8.ogg", qfalse);
#if 0

  cgs.media.sfx_ShottyhitWood[0] = trap_S_RegisterSound("sound/weapons/bullethit_wood1.ogg", qfalse);                  /* 0xA5EA */
  //cgs.media.sfx_ShottyhitWood[1] = trap_S_RegisterSound ("sound/weapons/shottyhwood2.ogg", qfalse);	/* 0xA5EA */
  cgs.media.sfx_ShottyhitWood[1] = trap_S_RegisterSound("sound/weapons/bullethit_wood2.ogg", qfalse);
  cgs.media.sfx_ShottyhitWood[2] = trap_S_RegisterSound("sound/weapons/bullethit_wood3.ogg",  qfalse);
  //cgs.media.sfx_ShottyhitWood[3] = trap_S_RegisterSound ("sound/weapons/ric1.ogg",  qfalse);
#endif
  cgs.media.sfx_ShottyhitIce[0] = trap_S_RegisterSound("sound/weapons/shottyhice0.ogg", qfalse);                       /* 0xA5EA */
  cgs.media.sfx_ShottyhitIce[1] = trap_S_RegisterSound("sound/weapons/shottyhice0.ogg", qfalse);                       /* 0xA5EA */

  cgs.media.sfx_ShottyhitEarth[0] = trap_S_RegisterSound("sound/weapons/shottyhearth1.ogg", qfalse);                   /* 0xA5EA */
  cgs.media.sfx_ShottyhitEarth[1] = trap_S_RegisterSound("sound/weapons/shottyhearth0.ogg", qfalse);                   /* 0xA5EA */

  cgs.media.sfx_rockexp[0] = trap_S_RegisterSound("sound/world/explosion1.ogg", qfalse);
  cgs.media.sfx_rockexp[1] = trap_S_RegisterSound("sound/world/explosion2.ogg", qfalse);

  cgs.media.sfx_GrenExpl[0] = trap_S_RegisterSound("sound/weapons/grenade_launcher/grExpl1.ogg", qfalse);
  cgs.media.sfx_GrenExpl[1] = trap_S_RegisterSound("sound/weapons/grenade_launcher/grExpl2.ogg", qfalse);
  cgs.media.sfx_GrenExpl[2] = trap_S_RegisterSound("sound/weapons/grenade_launcher/grExpl3.ogg", qfalse);
  /*
 //  #ifdef MISSIONPACK
      cgs.media.sfx_proxexp = trap_S_RegisterSound( "sound/weapons/proxmine/wstbexpl.ogg" , qfalse);
      cgs.media.sfx_nghit = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpd.ogg" , qfalse);
      cgs.media.sfx_nghitflesh = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpl.ogg" , qfalse);
      cgs.media.sfx_nghitmetal = trap_S_RegisterSound( "sound/weapons/nailgun/wnalimpm.ogg", qfalse );
      cgs.media.sfx_chghit = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpd.ogg", qfalse );
      cgs.media.sfx_chghitflesh = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpl.ogg", qfalse );
      cgs.media.sfx_chghitmetal = trap_S_RegisterSound( "sound/weapons/vulcan/wvulimpm.ogg", qfalse );
      cgs.media.weaponHoverSound = trap_S_RegisterSound( "sound/weapons/weapon_hover.ogg", qfalse );

      cgs.media.winnerSound = trap_S_RegisterSound( "sound/feedback/voc_youwin.ogg", qfalse );
      cgs.media.loserSound = trap_S_RegisterSound( "sound/feedback/voc_youlose.ogg", qfalse );
      cgs.media.youSuckSound = trap_S_RegisterSound( "sound/misc/yousuck.ogg", qfalse );

      cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.ogg", qfalse);
      cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.ogg", qfalse);
      cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.ogg", qfalse);
      cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.ogg", qfalse);
  #endif
  */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // 0xA5EA
  /* 0xA5EA */
#ifdef USE_KAMIKAZE
  cgs.media.kamikazeExplodeSound = trap_S_RegisterSound("sound/items/kam_explode.ogg", qfalse);
  cgs.media.kamikazeImplodeSound = trap_S_RegisterSound("sound/items/kam_implode.ogg", qfalse);
  cgs.media.kamikazeFarSound     = trap_S_RegisterSound("sound/items/kam_explode_far.ogg", qfalse);
#endif
  //cgs.media.regenSound   = trap_S_RegisterSound("sound/items/regen.ogg", qfalse);
  //cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.ogg", qfalse);

  //cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.ogg", qfalse );
  cgs.media.n_healthSound = trap_S_RegisterSound("sound/world/pickups/health.ogg", qfalse);
  // Grenadelauncer bounce sounds
  CG_LoadingString("misc sounds", qfalse);
  cgs.media.hgrenb1aSound     = trap_S_RegisterSound("sound/weapons/grenade_launcher/grenade bounce1.ogg", qfalse);
  cgs.media.hgrenb2aSound     = trap_S_RegisterSound("sound/weapons/grenade_launcher/grenade bounce2.ogg", qfalse);
  cgs.media.hgrenb3aSound     = trap_S_RegisterSound("sound/weapons/grenade_launcher/grenade bounce3.ogg", qfalse);    /* 0xA5EA */
  cgs.media.hgrenbwood1Sound  = trap_S_RegisterSound("sound/weapons/grenade_launcher/g_wood0.ogg", qfalse);            /* 0xA5EA */
  cgs.media.hgrenbwood2Sound  = trap_S_RegisterSound("sound/weapons/grenade_launcher/g_wood1.ogg", qfalse);            /* 0xA5EA */
  cgs.media.hgrenbmetal1Sound = trap_S_RegisterSound("sound/weapons/grenade_launcher/g_default0.ogg", qfalse);         /* 0xA5EA */
  cgs.media.hgrenbmetal2Sound = trap_S_RegisterSound("sound/weapons/grenade_launcher/g_default1.ogg", qfalse);         /* 0xA5EA */

#ifdef USE_FLAMEGUN
  cgs.media.flameSound = trap_S_RegisterSound( "sound/weapons/flame_thrower/flame_burn.ogg", qfalse );
  cgs.media.flameScreenSound = trap_S_RegisterSound( "sound/weapons/flame_thrower/toasty.ogg", qfalse );
#if 0
  //cgs.media.flameBlowSound = trap_S_RegisterSound( "sound/weapons/flame_thrower/flame_pilot.ogg", qfalse );
  //cgs.media.flameStartSound = trap_S_RegisterSound( "sound/weapons/flame_thrower/flame_up.ogg", qfalse );
  //cgs.media.flameStreamSound = trap_S_RegisterSound( "sound/weapons/flame_thrower/flame_fire.ogg", qfalse );
#endif 
  //FIXME (0xA5EA): sounds dont exist yet //hypov8 note: typo. but not used anyway
  cgs.media.flameCrackSound =     0; // -trap_S_RegisterSound( "sound/world/firecrack1.wav", qfalse );

  cgs.media.flamethrowerFireStream = trap_R_RegisterShader( "flamethrowerFireStream" );
  cgs.media.flamethrowerBlueStream = trap_R_RegisterShader( "flamethrowerBlueStream" );

  cgs.media.onFireShader1 = trap_R_RegisterShader( "entityOnFire1" );
  cgs.media.onFireShader2 = trap_R_RegisterShader( "entityOnFire3" );

  cgs.media.sparkFlareShader = trap_R_RegisterShader( "sparkFlareParticle" );

  cgs.media.nerveTestShader = trap_R_RegisterShader( "jpwtest1" );
  cgs.media.idTestShader = trap_R_RegisterShader( "jpwtest2" );

#endif
  //FIXME (0xA5EA): shader dont exist yet
}

//===================================================================================
/*
=================
CG_RegisterGraphics
This function may execute for a couple of minutes with a slow disk.
=================
*/
#ifdef USE_MAP_THREAD

static void CG_LoadMap()
{
  trap_R_LoadWorldMap(cgs.mapname);
}

#endif
static void CG_RegisterGraphics(void)
{
  int i;
  char items[MAX_ITEMS + 1];
  static const char *sb_nums[11] =
  {
    "gfx/2d/numbers/zero_32b",
    "gfx/2d/numbers/one_32b",
    "gfx/2d/numbers/two_32b",
    "gfx/2d/numbers/three_32b",
    "gfx/2d/numbers/four_32b",
    "gfx/2d/numbers/five_32b",
    "gfx/2d/numbers/six_32b",
    "gfx/2d/numbers/seven_32b",
    "gfx/2d/numbers/eight_32b",
    "gfx/2d/numbers/nine_32b",
    "gfx/2d/numbers/minus_32b",
  };
  /* 0xA5EA ============> */
  /*	static char *sb_numskp[] = {
  		"gfx/2d/numbers/yhud_7",
  		"gfx/2d/numbers/yhud_0",
  		"gfx/2d/numbers/yhud_1",
  		"gfx/2d/numbers/yhud_2",
  		"gfx/2d/numbers/yhud_3",
  		"gfx/2d/numbers/yhud_4",
  		"gfx/2d/numbers/yhud_5",
  		"gfx/2d/numbers/yhud_6",
  		"gfx/2d/numbers/yhud_7",
  		"gfx/2d/numbers/yhud_8",
  		"gfx/2d/numbers/yhud_9",
  		"gfx/2d/numbers/yhud_-",
  	};*/

  /*  <=============== 0xA5EA */
#ifndef USE_MAP_THREAD
  // clear any references to old media
  Com_Memset(&cg.refdef, 0, sizeof(cg.refdef));
  trap_R_ClearScene();

  CG_LoadingString(cgs.mapname, qfalse);

  trap_R_LoadWorldMap(cgs.mapname);
#else
  Com_Memset(&cg.refdef, 0, sizeof(cg.refdef));
  trap_R_ClearScene();

  CG_LoadingString(cgs.mapname, qfalse);
  boost::thread thd(&CG_LoadMap);
  thd.join();
#endif

  // precache status bar pics
  CG_LoadingString("precaching", qfalse);

  //FIXME(0xA5EA): number shaders still needed ??
  for (i = 0; i < 11; i++)
  {
    cgs.media.numberShaders[i] = trap_R_RegisterShader(sb_nums[i]);
    //cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_numskp [i] );  /*0xA5EA */
  }
  CG_LoadingString("interface", qfalse);
  cgs.media.botSkillShaders[0] = trap_R_RegisterShader("ui/assets/skill1.png");
  cgs.media.botSkillShaders[1] = trap_R_RegisterShader("ui/assets/skill2.png");
  cgs.media.botSkillShaders[2] = trap_R_RegisterShader("ui/assets/skill3.png");
  cgs.media.botSkillShaders[3] = trap_R_RegisterShader("ui/assets/skill4.png");
  cgs.media.botSkillShaders[4] = trap_R_RegisterShader("ui/assets/skill5.png");

  CG_LoadingString("rankings", qfalse);
  cgs.media.deferShader = trap_R_RegisterShaderNoMip("gfx/2d/defer.png");

  //add hypov8 animated gl exp thats in syc with its release
  cgs.media.smokeGLShader[0] = trap_R_RegisterShader("smokeGL01");
  cgs.media.smokeGLShader[1] = trap_R_RegisterShader("smokeGL02");
  cgs.media.smokeGLShader[2] = trap_R_RegisterShader("smokeGL03");
  cgs.media.smokeGLShader[3] = trap_R_RegisterShader("smokeGL04");
  cgs.media.smokeGLShader[4] = trap_R_RegisterShader("smokeGL05");
  cgs.media.smokeGLShader[5] = trap_R_RegisterShader("smokeGL06");

  cgs.media.smokePuffShader = trap_R_RegisterShader("smokePuff");
  cgs.media.smokePuffRlShader = trap_R_RegisterShader( "smokePuffRocketlauncher" );
  cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader("shotgunSmokePuff");
  cgs.media.lightningShader = trap_R_RegisterShader("lightningBolt");

//#ifdef MISSIONPACK
//	cgs.media.nailPuffShader = trap_R_RegisterShader( "nailtrail" );
//	cgs.media.blueProxMine = trap_R_RegisterModel( "models/weaphits/proxmineb.md3" );
//#endif
#if 0
  cgs.media.plasmaBallShader = trap_R_RegisterShader("sprites/plasma1");
#endif

  //cg.testLight.attenuationShader = trap_R_RegisterShaderLightAttenuation(cg.testLightName);
  //FIXME(0xA5EA): wtf

  cgs.media.lagometerShader  = trap_R_RegisterShader("lagometer");
  cgs.media.connectionShader = trap_R_RegisterShader("disconnected");

  cgs.media.waterBubbleShader = trap_R_RegisterShader("waterBubble");

  cgs.media.tracerShader = trap_R_RegisterShader("gfx/misc/tracer");
  cgs.media.selectShader = trap_R_RegisterShader("gfx/2d/select");

  for (i = 0; i < NUM_CROSSHAIRS; i++)
  {
    cgs.media.crosshairShader[i] = trap_R_RegisterShader(va("gfx/2d/crosshair%c", 'a' + i));
  }

  cgs.media.backTileShader = trap_R_RegisterShader("gfx/2d/backtile");
  cgs.media.noammoShader   = trap_R_RegisterShader("gfx/icons/noammo");
  CG_LoadingString("effects", qfalse);
  // powerup shaders
#if 0
  cgs.media.quadShader         = trap_R_RegisterShader("powerups/quad");
  cgs.media.quadWeaponShader   = trap_R_RegisterShader("powerups/quadWeapon");
  cgs.media.battleSuitShader   = trap_R_RegisterShader("powerups/battleSuit");
  cgs.media.battleWeaponShader = trap_R_RegisterShader("powerups/battleWeapon");
  cgs.media.invisShader        = trap_R_RegisterShader("powerups/invisibility");
  cgs.media.regenShader        = trap_R_RegisterShader("powerups/regen");
#endif
  cgs.media.hastePuffShader = trap_R_RegisterShader("hasteSmokePuff");
#ifdef GT_USE_TA_TYPES
  if (cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF || cgs.gametype == GT_HARVESTER || cg_buildScript.integer)
  {
    cgs.media.redCubeModel  = trap_R_RegisterModel("models/powerups/orb/r_orb.md3");
    cgs.media.blueCubeModel = trap_R_RegisterModel("models/powerups/orb/b_orb.md3");
    cgs.media.redCubeIcon  = trap_R_RegisterShader("gfx/icons/skull_red");
    cgs.media.blueCubeIcon = trap_R_RegisterShader("gfx/icons/skull_blue");
  }
#endif

	if (cgs.gametype == GT_CTF || cgs.gametype == GT_1FCTF
#ifdef GT_USE_TA_TYPES
      || cgs.gametype == GT_HARVESTER
#endif
     || cg_buildScript.integer)
	{
		CG_LoadingString("visuals", qfalse);
		//dragon flag
		cgs.media.dragonFlagModel  = trap_R_RegisterModel("models/flags/drag_flag.md3");
		cgs.media.dragonFlagShader[0]  = trap_R_RegisterShaderNoMip("gfx/icons/iconf_red1");
		cgs.media.dragonFlagShader[1]  = trap_R_RegisterShaderNoMip("gfx/icons/iconf_red2");
		cgs.media.dragonFlagShader[2]  = trap_R_RegisterShaderNoMip("gfx/icons/iconf_red3");
		//niki flag
		cgs.media.nikkiFlagModel = trap_R_RegisterModel("models/flags/nikki_flag.md3");
		cgs.media.nikkiFlagShader[0] = trap_R_RegisterShaderNoMip("gfx/icons/iconf_blu1");
		cgs.media.nikkiFlagShader[1] = trap_R_RegisterShaderNoMip("gfx/icons/iconf_blu2");
		cgs.media.nikkiFlagShader[2] = trap_R_RegisterShaderNoMip("gfx/icons/iconf_blu3");

		//cgs.media.dragonFlagModel  = trap_R_RegisterModel("models/flags/world_drag.md5mesh"); // hypov8 flag models
		//cgs.media.nikkiFlagModel = trap_R_RegisterModel("models/flags/flag_w_nikki.md3"); 

		//hypov8 todo: sort out names once n 4 all
		//should only be 1 model and multiple skin files

#if 0
		cgs.media.flagPoleModel = trap_R_RegisterModel("models/flag2/flagpole.md3"); // 0xA5EA qfalse stimmt ?
		cgs.media.flagFlapModel = trap_R_RegisterModel("models/flag2/flagflap3.md3");

		cgs.media.dragonFlagBaseModel     = trap_R_RegisterModel("models/mapobjects/flagbase/red_base.md3");
		cgs.media.nikkiFlagBaseModel    = trap_R_RegisterModel("models/mapobjects/flagbase/blue_base.md3");
		cgs.media.neutralFlagBaseModel = trap_R_RegisterModel("models/mapobjects/flagbase/ntrl_base.md3");

		cgs.media.dragonFlagFlapSkin     = trap_R_RegisterSkin("models/flag2/dragon.skin");
		cgs.media.nikkiFlagFlapSkin    = trap_R_RegisterSkin("models/flag2/blue.skin");
		cgs.media.neutralFlagFlapSkin = trap_R_RegisterSkin("models/flag2/white.skin");
#else
		// not used / missing
		cgs.media.flagPoleModel = 0;
		cgs.media.flagFlapModel = 0;

		cgs.media.dragonFlagBaseModel = 0;
		cgs.media.nikkiFlagBaseModel = 0;
		cgs.media.neutralFlagBaseModel = 0;

		cgs.media.dragonFlagFlapSkin     = 0;
		cgs.media.nikkiFlagFlapSkin    = 0;
		cgs.media.neutralFlagFlapSkin = 0;
#endif

	}

	if (cgs.gametype == GT_1FCTF || cg_buildScript.integer)
	{
		cgs.media.neutralFlagModel = trap_R_RegisterModel("models/flags/n_flag.md3");
		cgs.media.flagShader[0]    = trap_R_RegisterShaderNoMip("gfx/icons/iconf_neut"); //hud icons
		cgs.media.flagShader[1]    = trap_R_RegisterShaderNoMip("gfx/icons/iconf_red");
		cgs.media.flagShader[2]    = trap_R_RegisterShaderNoMip("gfx/icons/iconf_blu");
		cgs.media.flagShader[3]    = trap_R_RegisterShaderNoMip("gfx/icons/iconf_neut2");
	}

#ifdef WITH_BAGMAN_MOD
  else if (cgs.gametype == GT_BAGMAN)
  {
     //cgs.media.dragonsSafeBaseModel =  trap_R_RegisterModel("models/al0/cashsafe_al.md5mesh");
     //cgs.media.nikkisSafeBaseModel =  trap_R_RegisterModel("models/al0/cashsafe_al.md5mesh"); //hypov8 disabled
	 cgs.media.dragonsSafeBaseModel = trap_R_RegisterModel("models/qpin_gameobj/moneybag.md3");  //hypov8 ToDo: make each team differnt
	 cgs.media.nikkisSafeBaseModel = trap_R_RegisterModel("models/qpin_gameobj/moneybag.md3");

	 cgs.media.cashStolenModel = trap_R_RegisterModel("models/qpin_gameobj/moneybag.md3"); // hypov8

	 //cgs.media.cashStolenSkin = trap_R_RegisterSkin("models/flag2/white.skin"); // tmp hypov8
  }
#endif

	#ifdef GT_USE_TA_TYPES
	if (cgs.gametype == GT_OBELISK || cg_buildScript.integer)
	{
		cgs.media.overloadBaseModel   = trap_R_RegisterModel("models/powerups/overload_base.md3");
		cgs.media.overloadTargetModel = trap_R_RegisterModel("models/powerups/overload_target.md3");
		cgs.media.overloadLightsModel = trap_R_RegisterModel("models/powerups/overload_lights.md3");
		cgs.media.overloadEnergyModel = trap_R_RegisterModel("models/powerups/overload_energy.md3");
	}

	if (cgs.gametype == GT_HARVESTER || cg_buildScript.integer)
	{
		cgs.media.harvesterModel        = trap_R_RegisterModel("models/powerups/harvester/harvester.md3");
		cgs.media.harvesterRedSkin      = trap_R_RegisterSkin("models/powerups/harvester/red.skin");
		cgs.media.harvesterBlueSkin     = trap_R_RegisterSkin("models/powerups/harvester/blue.skin");
		cgs.media.harvesterNeutralModel = trap_R_RegisterModel("models/powerups/obelisk/obelisk.md3");
	}
#endif
#ifdef USE_KAMIKAZE
	cgs.media.redKamikazeShader = trap_R_RegisterShader("models/weaphits/kamikred");
#endif
	cgs.media.dustPuffShader    = trap_R_RegisterShader("hasteSmokePuff");

	if (cgs.gametype >= GT_TEAM || cg_buildScript.integer)
	{
		//cgs.media.friendShader = trap_R_RegisterShader("sprites/foe");
		//hypov8 show team icons
		cgs.media.teamDragon = trap_R_RegisterShader("sprites/teamdragon");
		cgs.media.teamNikki = trap_R_RegisterShader("sprites/teamnikki");

		cgs.media.teamStatusBar = trap_R_RegisterShader("gfx/2d/colorbar.jpg");
#ifdef USE_KAMIKAZE
		cgs.media.blueKamikazeShader = trap_R_RegisterShader("models/weaphits/kamikblu");
#endif
	}
	cgs.media.armorModel[0] = trap_R_RegisterModel("models/pu_icon/armor_head/armor_head_hd.md3");
	cgs.media.armorModel[1] = trap_R_RegisterModel("models/pu_icon/armor/armorhdtop.md3");
	cgs.media.armorModel[2] = trap_R_RegisterModel("models/pu_icon/armor_lo/armor_lo_hd.md3");

	cgs.media.armorIcon[0]  =  trap_R_RegisterShaderNoMip("gfx/icons/h_a_head");
	cgs.media.armorIcon[1]  = trap_R_RegisterShaderNoMip("gfx/icons/h_a_body");
	cgs.media.armorIcon[2]  =  trap_R_RegisterShaderNoMip("gfx/icons/h_a_leg");
	cgs.media.armorIcon[3]  =  trap_R_RegisterShaderNoMip("gfx/icons/h_a_head2");
	cgs.media.armorIcon[4]  = trap_R_RegisterShaderNoMip("gfx/icons/h_a_body2");
	cgs.media.armorIcon[5]  =  trap_R_RegisterShaderNoMip("gfx/icons/h_a_leg2");
	cgs.media.armorIcon[6]  =  trap_R_RegisterShaderNoMip("gfx/icons/h_a_man");//hypov8 player icon

	cgs.media.machinegunBrassModel = trap_R_RegisterModel("models/weapons/shells/m_shell.md3");
	cgs.media.shotgunBrassModel    = trap_R_RegisterModel("models/weapons/shells/s_shell.md3");
	cgs.media.gibArm       = trap_R_RegisterModel("models/gibs/arm.md3");
	cgs.media.gibChest     = trap_R_RegisterModel("models/gibs/chest.md3");
	cgs.media.gibFist      = trap_R_RegisterModel("models/gibs/leg1.md3");
	cgs.media.gibFoot      = trap_R_RegisterModel("models/gibs/leg2.md3");
	cgs.media.gibForearm   = trap_R_RegisterModel("models/gibs/gib1.md3");
	cgs.media.gibIntestine = trap_R_RegisterModel("models/gibs/smallchest.md3");
	cgs.media.gibLeg       = trap_R_RegisterModel("models/gibs/gib5.md3");
	cgs.media.gibSkull     = trap_R_RegisterModel("models/gibs/gib6.md3");

	cgs.media.smoke2 = trap_R_RegisterModel("models/weapons/shells/s_shell.md3");  //FIXME: was hat das mit smoke zu tun ?

	CG_LoadingString("notifications", qfalse);
	cgs.media.balloonShader = trap_R_RegisterShader("gfx/sprites/balloon");

	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/fbullet.md3"); //note hypov8 remove??, old particle shader
	//	cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3", qtrue );
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");

	//	cgs.media.teleportEffectModel = trap_R_RegisterModel( "models/misc/telep.md3" );
	//	cgs.media.teleportEffectShader = trap_R_RegisterShader( "teleportEffect" );

	CG_LoadingString("powerups", qfalse);
	#ifdef USE_KAMIKAZE
	cgs.media.kamikazeEffectModel = trap_R_RegisterModel("models/weaphits/kamboom2.md3");
	cgs.media.kamikazeShockWave   = trap_R_RegisterModel("models/weaphits/kamwave.md3");
	cgs.media.kamikazeHeadModel   = trap_R_RegisterModel("models/powerups/kamikazi.md3");
	cgs.media.kamikazeHeadTrail   = trap_R_RegisterModel("models/powerups/trailtest.md3");
	#endif

	cgs.media.shadowProjectedLightShader = trap_R_RegisterShaderLightAttenuation("lights/shadowProjectedLight");
	cgs.media.flashLightShader = trap_R_RegisterShaderLightAttenuation("flashlightshader");
	//cgs.media.flashLightModel = trap_R_RegisterModel("models/weaphits/bullet.md3"); //ToDo: hypov8 flashlight model

	CG_LoadingString("awards", qfalse);
	cgs.media.medalImpressive = trap_R_RegisterShader("medal_impressive");
	cgs.media.medalExcellent  = trap_R_RegisterShaderNoMip("medal_excellent");
	cgs.media.medalGauntlet   = trap_R_RegisterShaderNoMip("medal_gauntlet");
	cgs.media.medalDefend     = trap_R_RegisterShaderNoMip("medal_defend");
	cgs.media.medalAssist     = trap_R_RegisterShaderNoMip("medal_assist");
	cgs.media.medalCapture    = trap_R_RegisterShaderNoMip("medal_capture");

	//effects
	cgs.media.unlinkEffect = trap_R_RegisterShader("player/unlink_effect");

	CG_LoadingString("teams", qfalse);
	Com_Memset(cg_items, 0, sizeof(cg_items));
	Com_Memset(cg_weapons, 0, sizeof(cg_weapons));

	// only register the items that the server says we need
	qstrcpy(items, CG_ConfigString(CS_ITEMS)); // CS_ITEMS is a bitstring
	for (i = 1; i < bg_numItems; i++)
	{
		if (items[i] == '1' || cg_buildScript.integer)
			CG_RegisterItemVisuals(i);
	}

	CG_LoadingString("marks", qfalse);

	// wall marks
	cgs.media.bulletMark[0] = trap_R_RegisterShader("gfx/damage/bullet_mrk1");
	cgs.media.bulletMark[1] = trap_R_RegisterShader("gfx/damage/bullet_mrk2");
	cgs.media.bulletMarkmetal[0] = trap_R_RegisterShader("gfx/damage/bullet_mrk_metal1");
	cgs.media.bulletMarkmetal[1] = trap_R_RegisterShader("gfx/damage/bullet_mrk_metal2");
	cgs.media.bulletMarkwood[0] = trap_R_RegisterShader("gfx/damage/bullet_mrk_wood1");
	cgs.media.bulletMarkwood[1] = trap_R_RegisterShader("gfx/damage/bullet_mrk_wood2");
	cgs.media.bulletMarkglas[0] = trap_R_RegisterShader("gfx/damage/bullet_mrk_glas1");
	cgs.media.bulletMarkglas[1] = trap_R_RegisterShader("gfx/damage/bullet_mrk_glas2");

	cgs.media.burnMarkShader       = trap_R_RegisterShader("burnMark");
	cgs.media.burnMarkShaderRl       = trap_R_RegisterShader("rocketExplodeMark");

#if 1 //hypov8 todo crowbar/pipe decals
	cgs.media.meleeMarkBrick[0]		= trap_R_RegisterShader("gfx/damage/melee_brick_1");
	cgs.media.meleeMarkBrick[1]		= trap_R_RegisterShader("gfx/damage/melee_brick_2");
	cgs.media.meleeMarkMetal[0]		= trap_R_RegisterShader("gfx/damage/melee_metal_1");
	cgs.media.meleeMarkMetal[1]		= trap_R_RegisterShader("gfx/damage/melee_metal_2");
	cgs.media.meleeMarkWood[0]		= trap_R_RegisterShader("gfx/damage/melee_wood_1");
	cgs.media.meleeMarkWood[1]		= trap_R_RegisterShader("gfx/damage/melee_wood_2");
	cgs.media.meleeMarkGlass[0]		= trap_R_RegisterShader("gfx/damage/melee_glass_1");
	cgs.media.meleeMarkGlass[1]		= trap_R_RegisterShader("gfx/damage/melee_glass_2");
#endif

	cgs.media.holeMarkShader       = trap_R_RegisterShader("gfx/damage/hole_lg_mrk");
	cgs.media.energyMarkShader     = trap_R_RegisterShader("gfx/damage/burn_med_mrk");
	cgs.media.shadowMarkShader     = trap_R_RegisterShader("markShadow");
	cgs.media.wakeMarkShader       = trap_R_RegisterShader("wake");
	cgs.media.bloodMarkShader[0]   = trap_R_RegisterShader("bloodmark");
	cgs.media.bloodMarkShader[1]   = trap_R_RegisterShader("bloodmark");
	cgs.media.bloodMarkShader[2]   = trap_R_RegisterShader("bloodmark");
	cgs.media.bloodTrailShader     = trap_R_RegisterShader("bloodTrail");
	cgs.media.bloodExplosionShader = trap_R_RegisterShader("bloodExplosion");
	cgs.media.viewBloodShader      = trap_R_RegisterShader("viewBloodBlend"); //hypov8 todo: direction

	cgs.media.bloodSpurtShader[0] = trap_R_RegisterShader("gfx/particles/blood_spurt");
	cgs.media.bloodSpurtShader[1] = trap_R_RegisterShader("gfx/particles/blood_spurt2");
	cgs.media.bloodSpurtShader[2] = trap_R_RegisterShader("gfx/particles/blood_spurt3");

	cgs.media.rocketFirePartShader = trap_R_RegisterShader("gfx/particles/flare2");
	//FIXME(0xA5EA):redundant
	cgs.media.teleportFlareShader = trap_R_RegisterShader("gfx/particles/flare2");

	cgs.media.sparkShader = trap_R_RegisterShader("impactSparks");

	// register all weapons
	//FIXME (0xA5EA): should this be handled with precache in bg_itemlist ?
	/*
	CG_LoadingString("weapons", qfalse);
	for (i = 1; i < bg_numItems; i++)
	{
	if (bg_itemlist[i].giType != IT_WEAPON)
		continue;

	CG_RegisterWeapon(bg_itemlist[i].giTag);
	}*/

	CG_LoadingString("model icons", qfalse);
	// register the inline models
	CG_LoadingString("models", qfalse);

	cgs.numInlineModels = trap_CM_NumInlineModels();
	for (i = 1; i < cgs.numInlineModels; i++)
	{
		char name[10];
		vec3_t mins, maxs;
		int j;

		Com_sprintf(name, sizeof(name), "*%i", i);
		cgs.inlineDrawModel[i] = trap_R_RegisterModel(name);

		trap_R_ModelBounds(cgs.inlineDrawModel[i], mins, maxs);
		for (j = 0; j < 3; j++)
		{
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * (maxs[j] - mins[j]);
		}
	}

	// register all the server specified models
	for (i = 1; i < MAX_MODELS; i++)
	{
		const char *modelName;
		modelName = CG_ConfigString(CS_MODELS + i);

		if (!modelName[0])
			break;

		cgs.gameModels[i] = trap_R_RegisterModel(modelName);
	}

#ifdef GT_USE_TA_TYPES
    // new stuff
	cgs.media.patrolShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/patrol.tga");
	cgs.media.assaultShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/assault.tga");
	cgs.media.campShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/camp.tga");
	cgs.media.followShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/follow.tga");
	cgs.media.defendShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/defend.tga");
	cgs.media.teamLeaderShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/team_leader.tga");
	cgs.media.retrieveShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/retrieve.tga");
	cgs.media.escortShader = trap_R_RegisterShaderNoMip("ui/assets/statusbar/escort.tga");
#endif
	cgs.media.cursor         = trap_R_RegisterShaderNoMip("ui/cursor");
	cgs.media.sizeCursor     = trap_R_RegisterShaderNoMip("gfx/2d/select");
	cgs.media.selectCursor   = trap_R_RegisterShaderNoMip("gfx/2d/select");
	cgs.media.flagShaders[0] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_in_base.tga");
	cgs.media.flagShaders[1] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_capture.tga");
	cgs.media.flagShaders[2] = trap_R_RegisterShaderNoMip("ui/assets/statusbar/flag_missing.tga");

	CG_LoadingString("particles", qfalse);
	CG_InitParticles();
}

/*
=======================
CG_BuildSpectatorString
=======================
*/
void CG_BuildSpectatorString(void)
{
  int i;

  cg.spectatorList[0] = 0;
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR)
      Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
  }
  i = qstrlen(cg.spectatorList);
  if (i != cg.spectatorLen)
  {
    cg.spectatorLen   = (int)i;
    cg.spectatorWidth = -1;
  }
}

/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients(void)
{
  int i;

//	CG_LoadingClient(cg.clientNum);
  CG_NewClientInfo(cg.clientNum);

  for (i = 0; i < MAX_CLIENTS; i++)
  {
    const char *clientInfo;

    if (cg.clientNum == i)
      continue;

    clientInfo = CG_ConfigString(CS_PLAYERS + i);
    if (!clientInfo[0])
      continue;

    CG_LoadingString(va("precaching %i", i), qfalse);
    CG_NewClientInfo(i);
  }
  CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString(int index)
{
	char *out="";
	
  if (index < 0)
    CG_Error("CG_ConfigString: <0 bad index: %i ", index); //hypov8 re'enabled.

  if (index >= MAX_CONFIGSTRINGS-1)
	  CG_Error("CG_ConfigString: >%i bad index: %i", MAX_CONFIGSTRINGS, index);
  
  //hypov8 read overun
  if (!(index < 0 && index >= MAX_CONFIGSTRINGS - 1))
  {
	  out = cgs.gameState.stringData;
	  out += cgs.gameState.stringOffsets[index];
  }

  return out; 
}

//==================================================================

/*
======================
CG_StartMusic
======================
*/
void CG_StartMusic(void)
{
  char *s;
  char parm1[MAX_QPATH], parm2[MAX_QPATH];

  // start the background music
  s = (char *)CG_ConfigString(CS_MUSIC);
  Q_strncpyz(parm1, COM_Parse(&s), sizeof(parm1));
  Q_strncpyz(parm2, COM_Parse(&s), sizeof(parm2));

  trap_S_StartBackgroundTrack(parm1, parm2);
}

char *CG_GetMenuBuffer(const char *filename)
{
  int len;
  fileHandle_t f;
  static char buf[MAX_MENUFILE];

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f)
  {
    trap_Print(va(S_COLOR_RED "menu file not found: %s, using default\n", filename));
    return NULL;
  }
  if (len >= MAX_MENUFILE)
  {
    trap_Print(va(S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE));
    trap_FS_FCloseFile(f);
    return NULL;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle)
{
  pc_token_t token;
  const char *tempStr;

  if (!trap_PC_ReadToken(handle, &token))
    return qfalse;

  if (Q_stricmp(token.string, "{") != 0)
    return qfalse;

  while (1)
  {
    if (!trap_PC_ReadToken(handle, &token))
      return qfalse;

    if (Q_stricmp(token.string, "}") == 0)
      return qtrue;

    // font
    if (Q_stricmp(token.string, "font") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
      continue;
    }

    if (Q_stricmp(token.string, "tinyFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.tinyFont);
      continue;
    }

    // smallFont
    if (Q_stricmp(token.string, "smallFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
      continue;
    }

    // font
    if (Q_stricmp(token.string, "bigfont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
      continue;
    }

    //huge font
    if (Q_stricmp(token.string, "hugeFont") == 0)
    {
      int pointSize;

      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.hugeFont);
      continue;
    }
    //FIXME (0xA5EA):  chatfont
#if 0
    // chatFont
    if (Q_stricmp(token.string, "chatFont") == 0)
    {
      int pointSize;
      if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize))
        return qfalse;

      cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.chatFont);
      continue;
    }
#endif
    // gradientbar
    if (Q_stricmp(token.string, "gradientbar") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
      continue;
    }

    // enterMenuSound
    if (Q_stricmp(token.string, "menuEnterSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      cgDC.Assets.menuEnterSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // exitMenuSound
    if (Q_stricmp(token.string, "menuExitSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      cgDC.Assets.menuExitSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // itemFocusSound
    if (Q_stricmp(token.string, "itemFocusSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      cgDC.Assets.itemFocusSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    // menuBuzzSound
    if (Q_stricmp(token.string, "menuBuzzSound") == 0)
    {
      if (!PC_String_Parse(handle, &tempStr))
        return qfalse;

      cgDC.Assets.menuBuzzSound = trap_S_RegisterSound(tempStr, qfalse);
      continue;
    }

    if (Q_stricmp(token.string, "cursor") == 0)
    {
      if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr))
        return qfalse;

      cgDC.Assets.cursor = trap_R_RegisterShaderNoMip(cgDC.Assets.cursorStr);
      continue;
    }

    if (Q_stricmp(token.string, "fadeClamp") == 0)
    {
      if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "fadeCycle") == 0)
    {
      if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "fadeAmount") == 0)
    {
      if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "shadowX") == 0)
    {
      if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "shadowY") == 0)
    {
      if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY))
        return qfalse;

      continue;
    }

    if (Q_stricmp(token.string, "shadowColor") == 0)
    {
      if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor))
        return qfalse;

      cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
      continue;
    }
  }
  return qfalse; // bk001204 - why not?
}

void CG_ParseMenu(const char *menuFile)
{
  pc_token_t token;
  int handle;

  handle = trap_PC_LoadSource(menuFile);
  if (!handle)
    handle = trap_PC_LoadSource("ui/testhud.menu");

  if (!handle)
    return;

  while (1)
  {
    if (!trap_PC_ReadToken(handle, &token))
      break;

    //if ( Q_stricmp( token, "{" ) ) {
    //	Com_Printf( "Missing { in menu file\n" );
    //	break;
    //}

    //if ( menuCount == MAX_MENUS ) {
    //	Com_Printf( "Too many menus!\n" );
    //	break;
    //}

    if (token.string[0] == '}')
      break;

    if (Q_stricmp(token.string, "assetGlobalDef") == 0)
    {
      if (CG_Asset_Parse(handle))
        continue;
      else
        break;
    }

    if (Q_stricmp(token.string, "menudef") == 0)
    {
      // start a new menu
      Menu_New(handle);
    }
  }
  trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p)
{
  char *token;

  token = COM_ParseExt(p, qtrue);

  if (token[0] != '{')
    return qfalse;

  while (1)
  {
    token = COM_ParseExt(p, qtrue);

    if (Q_stricmp(token, "}") == 0)
      return qtrue;

    if (!token || token[0] == 0)
      return qfalse;

    CG_ParseMenu(token);
  }
  return qfalse;
}



void CG_LoadMenus(const char *menuFile)
{
  char *token;
  char *p;
  int len, start;
  fileHandle_t f;
  static char buf[MAX_MENUDEFFILE];

  start = trap_Milliseconds();

  len = trap_FS_FOpenFile(menuFile, &f, FS_READ);
  if (!f)
  {
    trap_Error(va(S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile));
    len = trap_FS_FOpenFile(DFLT_HUDFILE, &f, FS_READ);
    if (!f)
      trap_Error(va(S_COLOR_RED "default menu file not found: " DFLT_HUDFILE ", unable to continue!\n"));
  }

  if (len >= MAX_MENUDEFFILE)
  {
    trap_Error(va(S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE));
    trap_FS_FCloseFile(f);
    return;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  COM_Compress(buf);

  Menu_Reset();

  p = buf;

  while (1)
  {
    token = COM_ParseExt(&p, qtrue);
    if (!token || token[0] == 0 || token[0] == '}')
      break;

    //if ( Q_stricmp( token, "{" ) ) {
    //	Com_Printf( "Missing { in menu file\n" );
    //	break;
    //}

    //if ( menuCount == MAX_MENUS ) {
    //	Com_Printf( "Too many menus!\n" );
    //	break;
    //}

    if (Q_stricmp(token, "}") == 0)
      break;

    if (Q_stricmp(token, "loadmenu") == 0)
    {
      if (CG_Load_Menu(&p))
        continue;
      else
        break;
    }
  }

  Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);
}

static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key)
{
  return qfalse;
}

static int CG_FeederCount(float feederID)
{
  int i, count;

  count = 0;
  if (feederID == FEEDER_DRAGONTEAM_LIST)
  {
    for (i = 0; i < cg.numScores; i++)
    {
      if (cg.scores[i].team == TEAM_DRAGONS)
        count++;
    }
  }
  else if (feederID == FEEDER_NIKKITEAM_LIST)
  {
    for (i = 0; i < cg.numScores; i++)
    {
      if (cg.scores[i].team == TEAM_NIKKIS)
        count++;
    }
  }
  else if (feederID == FEEDER_SCOREBOARD)
    return cg.numScores;

  return count;
}

void CG_SetScoreSelection(void *p)
{
  menuDef_t *menu   = (menuDef_t *)p;
  playerState_t *ps = &cg.snap->ps;
  int i, red, blue;

  red = blue = 0;
  for (i = 0; i < cg.numScores; i++)
  {
    if (cg.scores[i].team == TEAM_DRAGONS)
      red++;
    else if (cg.scores[i].team == TEAM_NIKKIS)
      blue++;

    if (ps->clientNum == cg.scores[i].client)
      cg.selectedScore = i;
  }

  if (menu == NULL)
    return;     // just interested in setting the selected score

  if (cgs.gametype >= GT_TEAM)
  {
    int feeder = FEEDER_DRAGONTEAM_LIST;
    i = red;
    if (cg.scores[cg.selectedScore].team == TEAM_NIKKIS)
    {
      feeder = FEEDER_NIKKITEAM_LIST;
      i = blue;
    }
    Menu_SetFeederSelection(menu, feeder, i, NULL);
  }
  else
  {
    Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
  }
}

// FIXME: might need to cache this info
static clientInfo_t *CG_InfoFromScoreIndex(int index, int team, int *scoreIndex)
{
  int i, count;

  if (cgs.gametype >= GT_TEAM)
  {
    count = 0;
    for (i = 0; i < cg.numScores; i++)
    {
      if (cg.scores[i].team == team)
      {
        if (count == index)
        {
          *scoreIndex = i;
          return &cgs.clientinfo[cg.scores[i].client];
        }
        count++;
      }
    }
  }
  *scoreIndex = index;
  return &cgs.clientinfo[cg.scores[index].client];
}

static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle)
{
 //gitem_t *item;
  int scoreIndex = 0;
  clientInfo_t *info = NULL;
  int team = -1;
  score_t *sp = NULL;

  *handle = -1;

  if (feederID == FEEDER_DRAGONTEAM_LIST)
    team = TEAM_DRAGONS;
  else if (feederID == FEEDER_NIKKITEAM_LIST)
    team = TEAM_NIKKIS;

  info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
  sp   = &cg.scores[scoreIndex];
#if 0
  if (info && info->infoValid)
  {
    switch (column)
    {
    case 0:
      if (info->powerups & (1 << PW_NEUTRALFLAG))
      {
        item    = BG_FindItemForPowerup(PW_NEUTRALFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else if (info->powerups & (1 << PW_DRAGONFLAG))
      {
        item    = BG_FindItemForPowerup(PW_DRAGONFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else if (info->powerups & (1 << PW_NIKKIFLAG))
      {
        item    = BG_FindItemForPowerup(PW_NIKKIFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else
      {
        if (info->botSkill > 0 && info->botSkill <= 5)
        {
          *handle = cgs.media.botSkillShaders[info->botSkill - 1];
        }
        else if (info->handicap < 100)
        {
          return va("%i", info->handicap);
        }
      }
      break;
    case 1:
      if (team == -1)
      {
        return "";
      }
      else
      {
       ;// *handle = CG_StatusHandle(info->teamTask);
      }
      break;
    case 2:
      if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << sp->client))
        return "Ready";

      if (team == -1)
      {
        if (cgs.gametype == GT_TOURNAMENT)
        {
          return va("%i/%i", info->wins, info->losses);
        }
        else if (info->infoValid && info->team == TEAM_SPECTATOR)
        {
          return "Spectator";
        }
        else
        {
          return "";
        }
      }
      else
      {
        if (info->teamLeader)
          return "Leader";
      }
      break;
    case 3:
      return info->name;

      break;
    case 4:
      return va("%i", info->score);

      break;
    case 5:
      return va("%4i", sp->time);

      break;
    case 6:
      if (sp->ping == -1)
        return "connecting";

      return va("%4i", sp->ping);
      break;
    }
  }
#else
  if (info && info->infoValid)
  {
    switch (column)
    {
#if 0
    case 0:
      if (info->powerups & (1 << PW_NEUTRALFLAG))
      {
        item    = BG_FindItemForPowerup(PW_NEUTRALFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else if (info->powerups & (1 << PW_DRAGONFLAG))
      {
        item    = BG_FindItemForPowerup(PW_DRAGONFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else if (info->powerups & (1 << PW_NIKKIFLAG))
      {
        item    = BG_FindItemForPowerup(PW_NIKKIFLAG);
        *handle = cg_items[ITEM_INDEX(item)].icon;
      }
      else
      {
        if (info->botSkill > 0 && info->botSkill <= 5)
        {
          *handle = cgs.media.botSkillShaders[info->botSkill - 1];
        }
        else if (info->handicap < 100)
        {
          return va("%i", info->handicap);
        }
      }
      break;
    case 1:
      if (team == -1)
      {
        return "";
      }
      else
      {
        *handle = CG_StatusHandle(info->teamTask);
      }
      break;
    case 2:
      if (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << sp->client))
      {
        return "Ready";
      }
      if (team == -1)
      {
        if (cgs.gametype == GT_TOURNAMENT)
        {
          return va("%i/%i", info->wins, info->losses);
        }
        else if (info->infoValid && info->team == TEAM_SPECTATOR)
        {
          return "Spectator";
        }
        else
        {
          return "";
        }
      }
      else
      {
        if (info->teamLeader)
        {
          return "Leader";
        }
      }
      break;
#endif
    case 0:
      return info->name;
      break;

    case 3:
      return va("%i", info->score);
      break;

    case 2:
      return va("%4i", sp->time);
      break;

    case 1:
      if (sp->ping == -1)
      {
        return "n.a.";           // 0xA5EA
      }
      return va("%4i", sp->ping);
      break;

    case 4:
      return va("%i", sp->frags); //"4" sp->deaths (sp->score - sp->deaths) > 0 ? sp->score - sp->deaths : 0);
      break;

    case 5:
       return va("%i", sp->deaths); //return "5";   //deaths
      break;

    case 6:
      return va("%3i", sp->accuracy);
      break;
    }
  }
#endif
  return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index)
{
  return 0;
}

static void CG_FeederSelection(float feederID, int index)
{
  if (cgs.gametype >= GT_TEAM)
  {
    int i, count;
    int team = (feederID == FEEDER_DRAGONTEAM_LIST) ? TEAM_DRAGONS : TEAM_NIKKIS;
    count = 0;
    for (i = 0; i < cg.numScores; i++)
    {
      if (cg.scores[i].team == team)
      {
        if (index == count)
          cg.selectedScore = i;

        count++;
      }
    }
  }
  else
    cg.selectedScore = index;
}
//#endif


static float CG_Cvar_Get(const char *cvar)
{
  char buff[128];

  Com_Memset(buff, 0, sizeof(buff));
  trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
  return atof(buff);
}

void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style)
{
  CG_Text_Paint(x, y, scale, color, text, 0, limit, style, NULL_FONT);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale)
{
  switch (ownerDraw)
  {
  case CG_GAME_TYPE:
    return CG_Text_Width(CG_GameTypeString(), scale, 0, NULL_FONT);
  case CG_GAME_STATUS:
    return CG_Text_Width(CG_GetGameStatusText(), scale, 0, NULL_FONT);
  case CG_KILLER:
    return CG_Text_Width(CG_GetKillerText(), scale, 0, NULL_FONT);
  case CG_DRAGON_NAME:
    return CG_Text_Width(cg_DragonTeamName.string, scale, 0, NULL_FONT);
  case CG_NIKKI_NAME:
    return CG_Text_Width(cg_NikkiTeamName.string, scale, 0, NULL_FONT);
  }
  return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h)
{
  return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle)
{
  trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h)
{
  trap_CIN_SetExtents(handle, x, y, w, h);
  trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle)
{
  trap_CIN_RunCinematic(handle);
}

/*
=================
CG_LoadHudMenu();
=================
*/
void CG_LoadHudMenu(void)
{
  char buff[1024];
  const char *hudSet;

  cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
  cgDC.setColor            = &trap_R_SetColor;
  cgDC.drawHandlePic       = &CG_DrawPic;
  cgDC.drawStretchPic      = &trap_R_DrawStretchPic;
  cgDC.drawText            = &CG_Text_Paint;
  cgDC.textWidth           = &CG_Text_Width;
  cgDC.textHeight          = &CG_Text_Height;
  cgDC.registerModel       = &trap_R_RegisterModel;
  cgDC.modelBounds         = &trap_R_ModelBounds;
  cgDC.fillRect            = &CG_FillRect;
  cgDC.drawRect            = &CG_DrawRect;
  cgDC.drawSides           = &CG_DrawSides;
  cgDC.drawTopBottom       = &CG_DrawTopBottom;
  cgDC.clearScene          = &trap_R_ClearScene;
  cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
  cgDC.renderScene         = &trap_R_RenderScene;
  cgDC.registerFont        = &trap_R_RegisterFont;
  cgDC.ownerDrawItem       = &CG_OwnerDraw;
  cgDC.getValue            = &CG_GetValue;
  cgDC.ownerDrawVisible    = &CG_OwnerDrawVisible;
  cgDC.runScript           = &CG_RunMenuScript;
  cgDC.getTeamColor        = &CG_GetTeamColor;
  cgDC.setCVar             = trap_Cvar_Set;
  cgDC.getCVarString       = trap_Cvar_VariableStringBuffer;
  cgDC.getCVarValue        = CG_Cvar_Get;
  cgDC.drawTextWithCursor  = &CG_Text_PaintWithCursor;
  //cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
  //cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
  cgDC.startLocalSound    = &trap_S_StartLocalSound;
  cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
  cgDC.feederCount        = &CG_FeederCount;
  cgDC.feederItemImage    = &CG_FeederItemImage;
  cgDC.feederItemText     = &CG_FeederItemText;
  cgDC.feederSelection    = &CG_FeederSelection;
  //cgDC.setBinding = &trap_Key_SetBinding;
  //cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
  //cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
  //cgDC.executeText = &trap_Cmd_ExecuteText;
  cgDC.Error          = &Com_Error;
  cgDC.Print          = &Com_Printf;
  cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
  //cgDC.Pause = &CG_Pause;
  cgDC.registerSound        = &trap_S_RegisterSound;
  cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
  cgDC.stopBackgroundTrack  = &trap_S_StopBackgroundTrack;
  cgDC.playCinematic        = &CG_PlayCinematic;
  cgDC.stopCinematic        = &CG_StopCinematic;
  cgDC.drawCinematic        = &CG_DrawCinematic;
  cgDC.runCinematicFrame    = &CG_RunCinematicFrame;

  Init_Display(&cgDC);

  Menu_Reset();

  trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
  hudSet = buff;

  if (hudSet[0] == '\0')
    hudSet = DFLT_HUDFILE;

  CG_LoadMenus(hudSet);
}

void CG_AssetCache(void)
{
  //if (Assets.textFont == NULL) {
  //  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
  //}
  //Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
  //Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
  cgDC.Assets.gradientBar         = trap_R_RegisterShaderNoMip(ASSET_GRADIENTBAR);
  cgDC.Assets.fxBasePic           = trap_R_RegisterShaderNoMip(ART_FX_BASE);
  cgDC.Assets.fxPic[0]            = trap_R_RegisterShaderNoMip(ART_FX_RED);
  cgDC.Assets.fxPic[1]            = trap_R_RegisterShaderNoMip(ART_FX_YELLOW);
  cgDC.Assets.fxPic[2]            = trap_R_RegisterShaderNoMip(ART_FX_GREEN);
  cgDC.Assets.fxPic[3]            = trap_R_RegisterShaderNoMip(ART_FX_TEAL);
  cgDC.Assets.fxPic[4]            = trap_R_RegisterShaderNoMip(ART_FX_BLUE);
  cgDC.Assets.fxPic[5]            = trap_R_RegisterShaderNoMip(ART_FX_CYAN);
  cgDC.Assets.fxPic[6]            = trap_R_RegisterShaderNoMip(ART_FX_WHITE);
  cgDC.Assets.scrollBar           = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR);
  cgDC.Assets.scrollBarArrowDown  = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWDOWN);
  cgDC.Assets.scrollBarArrowUp    = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWUP);
  cgDC.Assets.scrollBarArrowLeft  = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWLEFT);
  cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip(ASSET_SCROLLBAR_ARROWRIGHT);
  cgDC.Assets.scrollBarThumb      = trap_R_RegisterShaderNoMip(ASSET_SCROLL_THUMB);
  cgDC.Assets.sliderBar           = trap_R_RegisterShaderNoMip(ASSET_SLIDER_BAR);
  cgDC.Assets.sliderThumb         = trap_R_RegisterShaderNoMip(ASSET_SLIDER_THUMB);
}

/*
=================
CG_Init
Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum, int randomSeed)
{
  const char *s;

  srand(randomSeed);
  // clear everything
  Com_Memset(&cgs, 0, sizeof(cgs));
  Com_Memset(&cg, 0, sizeof(cg));
  Com_Memset(cg_entities, 0, sizeof(cg_entities));
  Com_Memset(cg_weapons, 0, sizeof(cg_weapons));
  Com_Memset(cg_items, 0, sizeof(cg_items));

	// Set up the pmove params with sensible default values, the server params will
	// be communicated with the "pmove_params" server commands.
	// These values are the same as the default values of the servers to preserve
	cg.pmoveParams.fixed = cg.pmoveParams.synchronous = 0; //unvan .52
	cg.pmoveParams.accurate = 1; //unvan .52
	cg.pmoveParams.msec = 8; //unvan .52

  cg.clientNum = clientNum;
  cg.progress = 0;
  cgs.processedSnapshotNum  = serverMessageNum;
  cgs.serverCommandSequence = serverCommandSequence;
  cgs.media.menuback = trap_R_RegisterShader("ui/assets/backscreen");
  cgs.media.load0    = trap_R_RegisterShader("ui/load0");
  cgs.media.load1    = trap_R_RegisterShader("ui/load1");
  cgs.media.detail   = trap_R_RegisterShader("ui/maps_select");

  // load a few needed things before we do any screen updates
 // cgs.media.charsetShader = trap_R_RegisterShader("gfx/2d/bigchars");
  cgs.media.charsetShader = trap_R_RegisterShader("gfx/2d/charset-bezerk-plain-rc2");
  cgs.media.whiteShader = trap_R_RegisterShader("white");
  cgs.media.outlineShader = trap_R_RegisterShader("gfx/misc/outline"); //debug cg_drawBBOX

  // otty: register fonts here, otherwise CG_LoadingString wont work
  trap_R_RegisterFont("fonts/VeraBd.ttf", 22, &cgs.media.freeSansBoldFont);
  trap_R_RegisterFont("fonts/Vera.ttf", 22, &cgs.media.freeSansFont);
  trap_R_RegisterFont("fonts/LibMonoReg.ttf", 14, &cgs.media.consoleFont );
  CG_RegisterCvars();

  CG_InitConsoleCommands();

  //FIXME: 0xA5EA, realmode und hitmen
  cg.weaponSelect = WP_NONE; // note: this will spawn with gun lowered -> raise // WP_SHOTGUN;

  cgs.redflag    = cgs.blueflag = -1; // For compatibily, default to unset for
  cgs.flagStatus = -1;
  // old servers

  // get the rendering configuration from the client system
  trap_GetGlconfig(&cgs.glconfig);
  cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
  cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;
  // FIXME: 0xA5EA, widescreen

  // get the gamestate from the client system
  trap_GetGameState(&cgs.gameState);

  // check version
  s = CG_ConfigString(CS_GAME_VERSION);

  if (qstrcmp(s, GAME_VERSION))
    CG_Error("Client/Server game mismatch: %s/%s", GAME_VERSION, s);

  s = CG_ConfigString(CS_LEVEL_START_TIME);
  cgs.levelStartTime = atoi(s);

  String_Init();

  CG_AssetCache();
  CG_LoadHudMenu();      // load new hud stuff

  // load the new map
  CG_LoadingString("...", qfalse);

  CG_ParseServerinfo();

  // load the new map
  CG_LoadingString("collision map", qfalse);

  trap_CM_LoadMap(cgs.mapname);

  cg.loading = qtrue;         // force players to load instead of defer

  CG_LoadingString("Sounds", qtrue);

  CG_RegisterSounds();

  CG_LoadingString("Graphics", qtrue);

#if 0 //def USE_MAP_THREAD
  Com_Memset(&cg.refdef, 0, sizeof(cg.refdef));
  trap_R_ClearScene();

  CG_LoadingString(cgs.mapname, qfalse);
  boost::thread thd(&CG_LoadMap);
  thd.join();
#endif

  CG_RegisterGraphics();

  CG_LoadingString("weapons", qfalse);
  CG_InitWeapons();

  CG_LoadingString("Clients", qtrue);
  CG_RegisterClients();       // if low on memory, some clients will be deferred


  cg.loading = qfalse;        // future players will be deferred

  CG_LoadingString("Entities", qtrue);
  CG_InitLocalEntities();

  CG_LoadingString("Polys", qtrue);
  CG_InitMarkPolys();

#ifdef USE_FLAMEGUN
  CG_LoadingString("flamechunks", qtrue);
  CG_InitFlameChunks();       // RF, register and clear all flamethrower resources
#endif

  // remove the last loading update
//	cg.infoScreenText[0] = 0;

  // Make sure we have update values (scores)
  CG_SetConfigValues();

  CG_LoadingString("Music", qtrue);
  CG_StartMusic();

  // remove the last loading update
  cg.progress = 0;

  CG_InitTeamChat();

  CG_ShaderStateChanged();

  trap_S_ClearLoopingSounds(qtrue);
}

/*
=================
CG_Shutdown
Called before every level change or subsystem restart
=================
*/
void CG_Shutdown(void)
{
  // some mods may need to do cleanup work here,
  // like closing files or archiving session data
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor
*/
#if 0 //ndef MISSIONPACK
void CG_EventHandling(int type)
{
}

void CG_KeyEvent(int key, qboolean down)
{
}

void CG_MouseEvent(int x, int y)
{
}
#endif
