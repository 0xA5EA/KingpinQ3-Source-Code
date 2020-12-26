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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "../../ui/menudef.h" // bk001205 - for Q3_ui as well

typedef struct
{
  const char *order;
  int taskNum;
} orderTask_t;

static const orderTask_t validOrders[] = {
  {VOICECHAT_GETFLAG,            TEAMTASK_OFFENSE},
  {VOICECHAT_OFFENSE,            TEAMTASK_OFFENSE},
  {VOICECHAT_DEFEND,             TEAMTASK_DEFENSE},
  {VOICECHAT_DEFENDFLAG,         TEAMTASK_DEFENSE},
  {VOICECHAT_PATROL,             TEAMTASK_PATROL},
  {VOICECHAT_CAMP,               TEAMTASK_CAMP},
  {VOICECHAT_FOLLOWME,           TEAMTASK_FOLLOW},
  {VOICECHAT_RETURNFLAG,         TEAMTASK_RETRIEVE},
  {VOICECHAT_FOLLOWFLAGCARRIER,  TEAMTASK_ESCORT}
};

static const int numValidOrders = sizeof(validOrders) / sizeof(orderTask_t);

/*
=================
CG_ValidOrder
=================
*/
static int CG_ValidOrder(const char *p)
{
  int i;
  for (i = 0; i < numValidOrders; i++)
  {
    if (Q_stricmp(p, validOrders[i].order) == 0)
      return validOrders[i].taskNum;
  }
  return -1;
}

/*
=================
CG_ParseScores
=================
*/
static void CG_ParseScores(void)
{
  int i, powerups;

  cg.numScores = atoi(CG_Argv(1));
  if (cg.numScores > MAX_CLIENTS)
    cg.numScores = MAX_CLIENTS;

  cg.teamScores[0] = atoi(CG_Argv(2));
  cg.teamScores[1] = atoi(CG_Argv(3));

  Com_Memset(cg.scores, 0, sizeof(cg.scores));
  for (i = 0; i < cg.numScores; i++)
  {
    cg.scores[i].client     = atoi(CG_Argv(i * 14 + 4));
    cg.scores[i].score      = atoi(CG_Argv(i * 14 + 5));
    cg.scores[i].ping       = atoi(CG_Argv(i * 14 + 6));
    cg.scores[i].time       = atoi(CG_Argv(i * 14 + 7));
    cg.scores[i].scoreFlags = atoi(CG_Argv(i * 14 + 8));
    powerups                = atoi(CG_Argv(i * 14 + 9));
    cg.scores[i].accuracy   = atoi(CG_Argv(i * 14 + 10));
    //cg.scores[i].impressiveCount = atoi(CG_Argv(i * 14 + 11));
    cg.scores[i].frags      = atoi(CG_Argv(i * 14 + 11));
    cg.scores[i].excellentCount = atoi(CG_Argv(i * 14 + 12));
    //FIXME(0xA5EA): is something missing here ?
   // cg.scores[i].guantletCount   = atoi(CG_Argv(i * 14 + 13));
    cg.scores[i].deaths  = atoi(CG_Argv(i * 14 + 13));       // PERS_KILLED

    cg.scores[i].defendCount     = atoi(CG_Argv(i * 14 + 14));
    cg.scores[i].assistCount     = atoi(CG_Argv(i * 14 + 15));
    cg.scores[i].perfect         = atoi(CG_Argv(i * 14 + 16));

    cg.scores[i].captures        = atoi(CG_Argv(i * 14 + 17));

    if (cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS)
      cg.scores[i].client = 0;

    cgs.clientinfo[cg.scores[i].client].score    = cg.scores[i].score;
    cgs.clientinfo[cg.scores[i].client].powerups = powerups;

    cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
  }
  CG_SetScoreSelection(NULL);
}

/*
=================
CG_ParseTeamInfo
=================
*/
static void CG_ParseTeamInfo(void)
{
  int i;
  int client;

  numSortedTeamPlayers = atoi(CG_Argv(1));
  if (numSortedTeamPlayers < 0 || numSortedTeamPlayers > TEAM_MAXOVERLAY)
  {
    CG_Error( "CG_ParseTeamInfo: numSortedTeamPlayers out of range (%d)", numSortedTeamPlayers);
    return;
  }

  for (i = 0; i < numSortedTeamPlayers; i++)
  {
    client = atoi(CG_Argv(i * 6 + 2));
    if ( client < 0 || client >= MAX_CLIENTS )
    {
      CG_Error( "CG_ParseTeamInfo: bad client number: %d", client );
      return;
    }

    sortedTeamPlayers[i] = client;

    cgs.clientinfo[client].location  = atoi(CG_Argv(i * 6 + 3));
    cgs.clientinfo[client].health    = atoi(CG_Argv(i * 6 + 4));
    cgs.clientinfo[client].armor     = atoi(CG_Argv(i * 6 + 5));
    cgs.clientinfo[client].curWeapon = atoi(CG_Argv(i * 6 + 6));
    cgs.clientinfo[client].powerups  = atoi(CG_Argv(i * 6 + 7));
  }
}

/*
================
CG_ParseServerinfo
This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo(void)
{
  const char *info;
  char *mapname;

  info = CG_ConfigString(CS_SERVERINFO);
  cgs.gametype = (gametype_t)atoi(Info_ValueForKey(info, "g_gametype"));
  trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));

  // 0xA5EA, hitmen and realmode
  cgs.weaponmod = atoi(Info_ValueForKey(info, "g_weaponmod"));
  trap_Cvar_Set("g_weaponmod", va("%i", cgs.weaponmod));

  cgs.dmflags      = atoi(Info_ValueForKey(info, "g_dmflags")); //hypov8 "dmflags"
  cgs.teamflags    = atoi(Info_ValueForKey(info, "g_teamflags")); //hypov8 "teamflags"
  cgs.fraglimit    = atoi(Info_ValueForKey(info, "g_fraglimit")); //hypov8 "fraglimit"
  cgs.capturelimit = atoi(Info_ValueForKey(info, "g_capturelimit")); //hypov8 "capturelimit"
#ifdef WITH_BAGMAN_MOD
  cgs.cashlimit = atoi(Info_ValueForKey(info, "g_cashlimit"));
  CG_Printf("cgs.cashlimit = %i\n", cgs.cashlimit);

  cgs.playermaxcash = atoi(Info_ValueForKey(info, "g_cashcollectmax"));
  CG_Printf("cgs.playermaxcash = %i\n", cgs.playermaxcash);

  cgs.playermaxcashstolen = atoi(Info_ValueForKey(info, "g_cashstolenmax"));
  CG_Printf("cgs.playermaxcashstolen = %i\n", cgs.playermaxcashstolen);

  //FIXME(0xA5EA): bagman
#endif // WITH_BAGMAN_MOD
  cgs.timelimit    = atoi(Info_ValueForKey(info, "g_timelimit")); //hypov8 "timelimit"
  cgs.maxclients   = atoi(Info_ValueForKey(info, "sv_maxclients"));
  cgs.bunnyhop = atoi(Info_ValueForKey(info, "g_bunnyhop")); //hypov8 
  cgs.antiSpawnCamp = atoi(Info_ValueForKey(info, "g_antiSpawnCamp")); //hypov8 add: not used yet..

  mapname  = Info_ValueForKey(info, "mapname");
  Com_sprintf(cgs.mapname, sizeof(cgs.mapname), "maps/%s.bsp", mapname);
  Q_strncpyz(cgs.dragonTeam, Info_ValueForKey(info, "g_dragonTeam"), sizeof(cgs.dragonTeam));
  trap_Cvar_Set("g_dragonTeam", cgs.dragonTeam);
  Q_strncpyz(cgs.nikkiTeam, Info_ValueForKey(info, "g_nikkiTeam"), sizeof(cgs.nikkiTeam));
  trap_Cvar_Set("g_nikkiTeam", cgs.nikkiTeam);
//unlagged - server options
	// we'll need this for deciding whether or not to predict weapon effects
	cgs.delagHitscan = atoi( Info_ValueForKey( info, "g_delagHitscan" ) );
	trap_Cvar_Set("g_delagHitscan", va("%i", cgs.delagHitscan));
//unlagged - server options
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup(void)
{
  const char *info;
  int warmup;

  info = CG_ConfigString(CS_WARMUP);

  warmup = atoi(info);
  cg.warmupCount = -1;

  if (warmup == 0 && cg.warmup)
  {

  }
  else if (warmup > 0 && cg.warmup <= 0)
  {
#ifdef WITH_BAGMAN_MOD
    if (cgs.gametype >= GT_CTF && cgs.gametype <= GT_BAGMAN)
#else
    if (cgs.gametype >= GT_CTF)
#endif
      trap_S_StartLocalSound(cgs.media.countPrepareTeamSound, CHAN_ANNOUNCER);
    else
      trap_S_StartLocalSound(cgs.media.countPrepareSound, CHAN_ANNOUNCER);
  }
  cg.warmup = warmup;
}

/*
================
CG_SetConfigValues
Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues(void)
{
  const char *s;

  cgs.scores1 = atoi(CG_ConfigString(CS_SCORES1));
  cgs.scores2 = atoi(CG_ConfigString(CS_SCORES2));
  cgs.levelStartTime = atoi(CG_ConfigString(CS_LEVEL_START_TIME));
  if (cgs.gametype == GT_CTF)
  {
    s = CG_ConfigString(CS_FLAGSTATUS);
    cgs.redflag  = s[0] - '0';
    cgs.blueflag = s[1] - '0';
  }
  else if (cgs.gametype == GT_1FCTF)
  {
    s = CG_ConfigString(CS_FLAGSTATUS);
    cgs.flagStatus = s[0] - '0';
  }
  cg.warmup = atoi(CG_ConfigString(CS_WARMUP));
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void)
{
  char originalShader[MAX_QPATH];
  char newShader[MAX_QPATH];
  char timeOffset[16];
  const char *o;
  char *n, *t;

  o = CG_ConfigString(CS_SHADERSTATE);

  while (o && *o)
  {
    n = (char*)Q_strstr(o, "=");
    if (n && *n)
    {
      strncpy(originalShader, o, n - o);
      originalShader[n - o] = 0;
      n++;
      t = Q_strstr(n, ":");
      if (t && *t)
      {
        strncpy(newShader, n, t - n);
        newShader[t - n] = 0;
      }
      else
        break;

      t++;
      o = Q_strstr(t, "@");
      if (o)
      {
        strncpy(timeOffset, t, o - t);
        timeOffset[o - t] = 0;
        o++;
        trap_R_RemapShader(originalShader, newShader, timeOffset);
      }
    }
    else
      break;
  }
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified(void)
{
  const char *str;
  int num;

  num = atoi(CG_Argv(1));

  // get the gamestate from the client system, which will have the
  // new configstring already integrated
  trap_GetGameState(&cgs.gameState);

  // look up the individual string that was modified
  str = CG_ConfigString(num);

  // do something with it if necessary
  if (num == CS_MUSIC)
    CG_StartMusic();
  else if (num == CS_SERVERINFO)
    CG_ParseServerinfo();
  else if (num == CS_WARMUP)
    CG_ParseWarmup();
  else if (num == CS_SCORES1)
    cgs.scores1 = atoi(str);
  else if (num == CS_SCORES2)
    cgs.scores2 = atoi(str);
  else if (num == CS_LEVEL_START_TIME)
    cgs.levelStartTime = atoi(str);
  else if (num == CS_VOTE_TIME)
  {
    cgs.voteTime = atoi(str);
    cgs.voteModified = qtrue;
  }
  else if (num == CS_VOTE_YES)
  {
    cgs.voteYes = atoi(str);
    cgs.voteModified = qtrue;
  }
  else if (num == CS_VOTE_NO)
  {
    cgs.voteNo = atoi(str);
    cgs.voteModified = qtrue;
  }
  else if (num == CS_VOTE_STRING)
  {
    Q_strncpyz(cgs.voteString, str, sizeof(cgs.voteString));
    trap_S_StartLocalSound(cgs.media.voteNow, CHAN_ANNOUNCER);
  }
  else if (num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1)
  {
    cgs.teamVoteTime[num - CS_TEAMVOTE_TIME]     = atoi(str);
    cgs.teamVoteModified[num - CS_TEAMVOTE_TIME] = qtrue;
  }
  else if (num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1)
  {
    cgs.teamVoteYes[num - CS_TEAMVOTE_YES]      = atoi(str);
    cgs.teamVoteModified[num - CS_TEAMVOTE_YES] = qtrue;
  }
  else if (num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1)
  {
    cgs.teamVoteNo[num - CS_TEAMVOTE_NO]       = atoi(str);
    cgs.teamVoteModified[num - CS_TEAMVOTE_NO] = qtrue;
  }
  else if (num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1)
  {
    Q_strncpyz(cgs.teamVoteString[num - CS_TEAMVOTE_STRING], str, sizeof(cgs.teamVoteString));
    trap_S_StartLocalSound(cgs.media.voteNow, CHAN_ANNOUNCER);
  }
  else if (num == CS_INTERMISSION)
    cg.intermissionStarted = atoi(str);
  else if (num >= CS_MODELS && num < CS_MODELS + MAX_MODELS)
    cgs.gameModels[num - CS_MODELS] = trap_R_RegisterModel(str);
  else if (num >= CS_SOUNDS && num < CS_SOUNDS + MAX_SOUNDS)
  {
    if (str[0] != '*')   // player specific sounds don't register here
      cgs.gameSounds[num - CS_SOUNDS] = trap_S_RegisterSound(str, qfalse);
  }
  else if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS)
  {
    CG_NewClientInfo(num - CS_PLAYERS);
    CG_BuildSpectatorString();
  }
  else if (num == CS_FLAGSTATUS)
  {
    if (cgs.gametype == GT_CTF)
    {
      // format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
      cgs.redflag  = str[0] - '0';
      cgs.blueflag = str[1] - '0';
    }
    else if (cgs.gametype == GT_1FCTF)
      cgs.flagStatus = str[0] - '0';
  }
  else if (num == CS_SHADERSTATE)
    CG_ShaderStateChanged();
}

#if 0
/*
=======================
CG_AddChatText

Adds chat text to the chat chat buffer
=======================
*/
static void CG_AddChatText(int client, const char *str)
{
  int len;
  char *p;
  char *ls;
  int lastcolor;
  int chatHeight;
  float w;

  if (client >= 0)
  {
    cgs.clientinfo[client].mLastChatTime = cg.time;
  }

  // Grab the users chat height settings
  chatHeight = cg_chatHeight.integer;
  if (chatHeight > CHAT_HEIGHT)
  {
    chatHeight = CHAT_HEIGHT;
  }

  // Chats disabled?
  if (chatHeight <= 0 || cg_chatTime.integer <= 0)
  {
    cgs.chatPos = cgs.chatLastPos = 0;
    return;
  }

  len = 0;

  lastcolor = COLOR_WHITE;

  // Next position to write chat text too in the circular chat buffer
  p  = cgs.chatText[cgs.chatPos % chatHeight];
  *p = 0;

  ls = NULL;
  w  = 0;

  while (*str)
  {
    float cw = trap_R_GetTextWidth(va("%c", *str), cgs.media.hudFont, 0.43f, 0);

    if (w > 560)
    {
      w = 0;

      if (ls)
      {
        str -= (p - ls);
        str++;
        p -= (p - ls);
      }

      *p = 0;

      cgs.chatTime[cgs.chatPos % chatHeight] = cg.time;

      cgs.chatPos++;
      p    = cgs.chatText[cgs.chatPos % chatHeight];
      *p   = 0;
      *p++ = Q_COLOR_ESCAPE;
      *p++ = lastcolor;
      len  = 0;
      ls   = NULL;
    }

    if (Q_IsColorString(str))
    {
      *p++      = *str++;
      lastcolor = *str;
      *p++      = *str++;
      continue;
    }

    if (*str == ' ')
    {
      ls = p;
    }

    *p++ = *str++;
    len++;
    w += cw;
  }
  *p = 0;

  cgs.chatTime[cgs.chatPos % chatHeight] = cg.time;
  cgs.chatPos++;

  if (cgs.chatPos - cgs.chatLastPos > chatHeight)
  {
    cgs.chatLastPos = cgs.chatPos - chatHeight;
  }
}
#endif
/*
=======================
CG_AddToTeamChat
0xA5EA teamchat
=======================
*/
static void CG_AddToTeamChat(const char *str)
{
  int len;
  char *p, *ls;
  int lastcolor;
  int chatHeight;

  if (cg_ChatHeight.integer < TEAMCHAT_HEIGHT)
    chatHeight = cg_ChatHeight.integer;
  else
    chatHeight = TEAMCHAT_HEIGHT;

  if (chatHeight <= 0 || cg_ChatTime.integer <= 0)
  {
    // team chat disabled, dump into normal chat
    cgs.teamChatPos = cgs.teamLastChatPos = 0;
    return;
  }

  len = 0;

  p  = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
  *p = 0;

  lastcolor = '7';

  ls = NULL;
  while (*str)
  {
    if (len > TEAMCHAT_WIDTH - 1)
    {
      if (ls)
      {
        str -= (p - ls);
        str++;
        p -= (p - ls);
      }
      *p = 0;

      cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

      cgs.teamChatPos++;
      p    = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
      *p   = 0;
      *p++ = Q_COLOR_ESCAPE;
      *p++ = lastcolor;
      len  = 0;
      ls   = NULL;
    }
	if (!str){ return; } //add hypov8 'Dereferncing null pointer'
    if (Q_IsColorString(str))
    {
      *p++      = *str++;
      lastcolor = *str;
      *p++      = *str++;
      continue;
    }
    if (*str == ' ')
      ls = p;

    *p++ = *str++;
    len++;
  }
  *p = 0;

  cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
  cgs.teamChatPos++;

  if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
    cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}

/*
===============
CG_MapRestart
The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.
A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart(void)
{
  if (cg_showmiss.integer)
    CG_Printf("CG_MapRestart\n");

  CG_InitLocalEntities();
  CG_InitMarkPolys();
//	CG_ClearParticles ();	// id way
  CG_InitParticles();   // 0xA5EA, xreal way

  // make sure the "3 frags left" warnings play again
  cg.fraglimitWarnings = 0;

  cg.timelimitWarnings = 0;

  cg.intermissionStarted = qfalse;

  cgs.voteTime = 0;

  cg.mapRestart = qtrue;

  cg.weaponSelect = WP_NONE; //match server. 0 will raise a weapon

  CG_StartMusic();

  trap_S_ClearLoopingSounds(qtrue);

  // JPW NERVE -- reset render flags
  cg.flameFxRotate = 0; //cg_fxflags = 0;

  // we really should clear more parts of cg here and stop sounds

  // play the "fight" sound if this is a restart without warmup
  if (cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT */)
  {
    trap_S_StartLocalSound(cgs.media.countFightSound, CHAN_ANNOUNCER);
    CG_CenterPrint("FIGHT!", 120, GIANTCHAR_WIDTH * 2);
  }
#ifdef USE_GT_SINGLEPLAYER
  if (cg_singlePlayerActive.integer)
  {
    trap_Cvar_Set("ui_matchStartTime", va("%i", cg.time));
    if (cg_recordSPDemo.integer && cg_recordSPDemoName.string && *cg_recordSPDemoName.string)
    {
      trap_SendConsoleCommand(va("set g_synchronousclients 1 ; record %s \n", cg_recordSPDemoName.string));
    }
  }
#endif
  trap_Cvar_Set("cg_thirdPerson", "0");
}

#define MAX_VOICEFILESIZE 16384
#define MAX_VOICEFILES       16 /* 0xA5EA, changed 8 to 16 */
#define MAX_VOICECHATS       64
#define MAX_VOICESOUNDS      64
#define MAX_CHATSIZE         64
#define MAX_HEADMODELS       64

typedef struct voiceChat_s
{
  char id[64];
  int numSounds;
  sfxHandle_t sounds[MAX_VOICESOUNDS];
  char chats[MAX_VOICESOUNDS][MAX_CHATSIZE];
}
voiceChat_t;

typedef struct voiceChatList_s
{
  char name[64];
  int gender;
  int numVoiceChats;
  voiceChat_t voiceChats[MAX_VOICECHATS];
}
voiceChatList_t;

typedef struct headModelVoiceChat_s
{
  char headmodel[64];
  int voiceChatNum;
}
headModelVoiceChat_t;

voiceChatList_t voiceChatLists[MAX_VOICEFILES];
headModelVoiceChat_t headModelVoiceChat[MAX_HEADMODELS];

/*
=================
CG_ParseVoiceChats
=================
*/
int CG_ParseVoiceChats(const char *filename, voiceChatList_t *voiceChatList, int maxVoiceChats)
{
  int len, i;
  fileHandle_t f;
  char buf[MAX_VOICEFILESIZE];
  char **p, *ptr;
  char *token;
  voiceChat_t *voiceChats;
  qboolean compress;
  sfxHandle_t sound;

  compress = qtrue;
  if (cg_buildScript.integer)
    compress = qfalse;

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f)
  {
    trap_Print(va(S_COLOR_RED "voice chat file not found: %s\n", filename));
    return qfalse;
  }
  if (len >= MAX_VOICEFILESIZE)
  {
    trap_Print(va(S_COLOR_RED "voice chat file too large: %s is %i, max allowed is %i", filename, len, MAX_VOICEFILESIZE));
    trap_FS_FCloseFile(f);
    return qfalse;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  ptr = buf;
  p   = &ptr;

  Com_sprintf(voiceChatList->name, sizeof(voiceChatList->name), "%s", filename);
  voiceChats = voiceChatList->voiceChats;
  for (i = 0; i < maxVoiceChats; i++)
  {
    voiceChats[i].id[0] = 0;
  }
  token = COM_ParseExt(p, qtrue);
  if (!token || token[0] == 0)
    return qtrue;

  //FIXME(0xA5EA): gender remove
  if (!Q_stricmp(token, "female"))
    voiceChatList->gender = GENDER_FEMALE;
  else if (!Q_stricmp(token, "male"))
    voiceChatList->gender = GENDER_MALE;
  else if (!Q_stricmp(token, "neuter"))
    voiceChatList->gender = GENDER_NEUTER;
  else
  {
    trap_Print(va(S_COLOR_RED "expected gender not found in voice chat file: %s\n", filename));
    return qfalse;
  }

  voiceChatList->numVoiceChats = 0;
  while (1)
  {
    token = COM_ParseExt(p, qtrue);
    if (!token || token[0] == 0)
      return qtrue;

    Com_sprintf(voiceChats[voiceChatList->numVoiceChats].id, sizeof(voiceChats[voiceChatList->numVoiceChats].id), "%s", token);
    token = COM_ParseExt(p, qtrue);
    if (Q_stricmp(token, "{"))
    {
      trap_Print(va(S_COLOR_RED "expected { found %s in voice chat file: %s\n", token, filename));
      return qfalse;
    }
    voiceChats[voiceChatList->numVoiceChats].numSounds = 0;
    while (1)
    {
      token = COM_ParseExt(p, qtrue);

      if (!token || token[0] == 0)
        return qtrue;

      if (!Q_stricmp(token, "}"))
        break;

      sound = trap_S_RegisterSound(token, compress);
      voiceChats[voiceChatList->numVoiceChats].sounds[voiceChats[voiceChatList->numVoiceChats].numSounds] = sound;
      token = COM_ParseExt(p, qtrue);

      if (!token || token[0] == 0)
        return qtrue;

      Com_sprintf(voiceChats[voiceChatList->numVoiceChats].chats[voiceChats[voiceChatList->numVoiceChats].numSounds], MAX_CHATSIZE, "%s", token);

      if (sound)
        voiceChats[voiceChatList->numVoiceChats].numSounds++;

      if (voiceChats[voiceChatList->numVoiceChats].numSounds >= MAX_VOICESOUNDS)
        break;
    }
    voiceChatList->numVoiceChats++;
    if (voiceChatList->numVoiceChats >= maxVoiceChats)
      return qtrue;
  }
  return qtrue;
}

/*
=================
CG_LoadVoiceChats
=================
*/
void CG_LoadVoiceChats(void)
{
  int size;

  size = trap_MemoryRemaining();
  CG_ParseVoiceChats("scripts/thug.voice", &voiceChatLists[0], MAX_VOICECHATS);
  //FIXME (0xA5EA):
#if 0
  CG_ParseVoiceChats("scripts/female1.voice", &voiceChatLists[0], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/female2.voice", &voiceChatLists[1], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/female3.voice", &voiceChatLists[2], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/male1.voice", &voiceChatLists[3], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/male2.voice", &voiceChatLists[4], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/male3.voice", &voiceChatLists[5], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/male4.voice", &voiceChatLists[6], MAX_VOICECHATS);
  CG_ParseVoiceChats("scripts/male5.voice", &voiceChatLists[7], MAX_VOICECHATS);
#endif
  CG_Printf("voice chat memory size = %d\n", size - trap_MemoryRemaining());
}

/*
=================
CG_HeadModelVoiceChats
=================
*/
int CG_HeadModelVoiceChats(char *filename)
{
  int len, i;
  fileHandle_t f;
  char buf[MAX_VOICEFILESIZE];
  char **p, *ptr;
  char *token;

  len = trap_FS_FOpenFile(filename, &f, FS_READ);
  if (!f)
    //trap_Print( va( "voice chat file not found: %s\n", filename ) );
    return -1;

  if (len >= MAX_VOICEFILESIZE)
  {
    trap_Print(va(S_COLOR_RED "voice chat file too large: %s is %i, max allowed is %i", filename, len, MAX_VOICEFILESIZE));
    trap_FS_FCloseFile(f);
    return -1;
  }

  trap_FS_Read(buf, len, f);
  buf[len] = 0;
  trap_FS_FCloseFile(f);

  ptr = buf;
  p   = &ptr;

  token = COM_ParseExt(p, qtrue);
  if (!token || token[0] == 0)
    return -1;

  for (i = 0; i < MAX_VOICEFILES; i++)
  {
    if (!Q_stricmp(token, voiceChatLists[i].name))
      return i;
  }

  //FIXME: maybe try to load the .voice file which name is stored in token?
  return -1;
}

/*
=================
CG_GetVoiceChat
=================
*/
int CG_GetVoiceChat(voiceChatList_t *voiceChatList, const char *id, sfxHandle_t *snd, char **chat)
{
  int i, rnd;

  for (i = 0; i < voiceChatList->numVoiceChats; i++)
  {
    if (!Q_stricmp(id, voiceChatList->voiceChats[i].id))
    {
      rnd   = random() * voiceChatList->voiceChats[i].numSounds;
      *snd  = voiceChatList->voiceChats[i].sounds[rnd];
      *chat = voiceChatList->voiceChats[i].chats[rnd];
      return qtrue;
    }
  }
  return qfalse;
}

/*
=================
CG_VoiceChatListForClient
=================
*/
voiceChatList_t *CG_VoiceChatListForClient(int clientNum)
{
  clientInfo_t *ci;
  int voiceChatNum, i, j, k, gender;
  char filename[MAX_QPATH], headModelName[MAX_QPATH];

  if (clientNum < 0 || clientNum >= MAX_CLIENTS)
    clientNum = 0;

  ci = &cgs.clientinfo[clientNum];

  for (k = 0; k < 2; k++)
  {
    if (k == 0)
    {
      if (ci->headModelName[0] == '*')
        Com_sprintf(headModelName, sizeof(headModelName), "%s/%s", ci->headModelName + 1, ci->headSkinName);
      else
        Com_sprintf(headModelName, sizeof(headModelName), "%s/%s", ci->headModelName, ci->headSkinName);
    }
    else
    {
      if (ci->headModelName[0] == '*')
        Com_sprintf(headModelName, sizeof(headModelName), "%s", ci->headModelName + 1);
      else
        Com_sprintf(headModelName, sizeof(headModelName), "%s", ci->headModelName);
    }
    // find the voice file for the head model the client uses
    for (i = 0; i < MAX_HEADMODELS; i++)
    {
      if (!Q_stricmp(headModelVoiceChat[i].headmodel, headModelName))
        break;
    }

    if (i < MAX_HEADMODELS)
      return &voiceChatLists[headModelVoiceChat[i].voiceChatNum];

    // find a <headmodelname>.vc file
    for (i = 0; i < MAX_HEADMODELS; i++)
    {
      if (!qstrlen(headModelVoiceChat[i].headmodel))
      {
        //FIXME(0xA5EA): headmodel vc files ?
        Com_sprintf(filename, sizeof(filename), "botfiles/bots/%s.vc", headModelName); //hypov8 was scripts/
        voiceChatNum = CG_HeadModelVoiceChats(filename);

        if (voiceChatNum == -1)
          break;

        Com_sprintf(headModelVoiceChat[i].headmodel, sizeof(headModelVoiceChat[i].headmodel), "%s", headModelName);
        headModelVoiceChat[i].voiceChatNum = voiceChatNum;
        return &voiceChatLists[headModelVoiceChat[i].voiceChatNum];
      }
    }
  }
  gender = ci->gender;
  for (k = 0; k < 2; k++)
  {
    // just pick the first with the right gender
    for (i = 0; i < MAX_VOICEFILES; i++)
    {
      if (qstrlen(voiceChatLists[i].name))
      {
        if (voiceChatLists[i].gender == gender)
        {
          // store this head model with voice chat for future reference
          for (j = 0; j < MAX_HEADMODELS; j++)
          {
            if (!qstrlen(headModelVoiceChat[j].headmodel))
            {
              Com_sprintf(headModelVoiceChat[j].headmodel, sizeof(headModelVoiceChat[j].headmodel), "%s", headModelName);
              headModelVoiceChat[j].voiceChatNum = i;
              break;
            }
          }
          return &voiceChatLists[i];
        }
      }
    }
    // fall back to male gender because we don't have neuter in the mission pack
    if (gender == GENDER_MALE)
      break;

    gender = GENDER_MALE;
  }
  // store this head model with voice chat for future reference
  for (j = 0; j < MAX_HEADMODELS; j++)
  {
    if (!qstrlen(headModelVoiceChat[j].headmodel))
    {
      Com_sprintf(headModelVoiceChat[j].headmodel, sizeof(headModelVoiceChat[j].headmodel), "%s", headModelName);
      headModelVoiceChat[j].voiceChatNum = 0;
      break;
    }
  }
  // just return the first voice chat list
  return &voiceChatLists[0];
}

#define MAX_VOICECHATBUFFER 32

typedef struct bufferedVoiceChat_s
{
  int clientNum;
  sfxHandle_t snd;
  int voiceOnly;
  char cmd[MAX_SAY_TEXT];
  char message[MAX_SAY_TEXT];
} bufferedVoiceChat_t;

bufferedVoiceChat_t voiceChatBuffer[MAX_VOICECHATBUFFER];

/*
=================
CG_PlayVoiceChat
=================
*/
void CG_PlayVoiceChat(bufferedVoiceChat_t *vchat)
{
  // if we are going into the intermission, don't start any voices
  if (cg.intermissionStarted)
    return;

  if (!cg_noVoiceChats.integer)
  {
    trap_S_StartLocalSound(vchat->snd, CHAN_VOICE);
    if (vchat->clientNum != cg.snap->ps.clientNum)
    {
      int orderTask = CG_ValidOrder(vchat->cmd);
      if (orderTask > 0)
      {
        cgs.acceptOrderTime = cg.time + 5000;
        Q_strncpyz(cgs.acceptVoice, vchat->cmd, sizeof(cgs.acceptVoice));
        cgs.acceptTask   = orderTask;
        cgs.acceptLeader = vchat->clientNum;
      }
      // see if this was an order
      //CG_ShowResponseHead();  /*0xA5EA FIXME: gives an undefined error in q3lcc */
    }
  }
  if (!vchat->voiceOnly && !cg_noVoiceText.integer)
    CG_AddToTeamChat(vchat->message);

  voiceChatBuffer[cg.voiceChatBufferOut].snd = 0;
}

/*
=====================
CG_PlayBufferedVoieChats
=====================
*/
void CG_PlayBufferedVoiceChats(void)
{
  if (cg.voiceChatTime < cg.time)
  {
    if (cg.voiceChatBufferOut != cg.voiceChatBufferIn && voiceChatBuffer[cg.voiceChatBufferOut].snd)
    {
      CG_PlayVoiceChat(&voiceChatBuffer[cg.voiceChatBufferOut]);
      cg.voiceChatBufferOut = (cg.voiceChatBufferOut + 1) % MAX_VOICECHATBUFFER;
      cg.voiceChatTime = cg.time + 1000;
    }
  }
}

/*
=====================
CG_AddBufferedVoiceChat
=====================
*/
void CG_AddBufferedVoiceChat(bufferedVoiceChat_t *vchat)
{
  // if we are going into the intermission, don't start any voices
  if (cg.intermissionStarted)
    return;

  Com_Memcpy(&voiceChatBuffer[cg.voiceChatBufferIn], vchat, sizeof(bufferedVoiceChat_t));
  cg.voiceChatBufferIn = (cg.voiceChatBufferIn + 1) % MAX_VOICECHATBUFFER;
  if (cg.voiceChatBufferIn == cg.voiceChatBufferOut)
  {
    CG_PlayVoiceChat(&voiceChatBuffer[cg.voiceChatBufferOut]);
    cg.voiceChatBufferOut++;
  }
}

/*
=================
CG_VoiceChatLocal
=================
*/
void CG_VoiceChatLocal(int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd)
{
  char *chat;
  voiceChatList_t *voiceChatList;
  clientInfo_t *ci;
  sfxHandle_t snd;
  bufferedVoiceChat_t vchat;

  // if we are going into the intermission, don't start any voices
  if (cg.intermissionStarted)
    return;

  if (clientNum < 0 || clientNum >= MAX_CLIENTS)
    clientNum = 0;

  ci = &cgs.clientinfo[clientNum];

  cgs.currentVoiceClient = clientNum;

  voiceChatList = CG_VoiceChatListForClient(clientNum);

  if (CG_GetVoiceChat(voiceChatList, cmd, &snd, &chat))
  {
    if (mode == SAY_TEAM || !cg_teamChatsOnly.integer)
    {
      vchat.clientNum = clientNum;
      vchat.snd       = snd;
      vchat.voiceOnly = voiceOnly;
      Q_strncpyz(vchat.cmd, cmd, sizeof(vchat.cmd));
      if (mode == SAY_TELL)
        Com_sprintf(vchat.message, sizeof(vchat.message), "[%s]: %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);
      else if (mode == SAY_TEAM)
        Com_sprintf(vchat.message, sizeof(vchat.message), "(%s): %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);
      else
        Com_sprintf(vchat.message, sizeof(vchat.message),   "%s: %c%c%s", ci->name, Q_COLOR_ESCAPE, color, chat);

      CG_AddBufferedVoiceChat(&vchat);
    }
  }
}

/*
=================
CG_VoiceChat
=================
*/
void CG_VoiceChat(int mode)
{
  const char *cmd;
  int clientNum, color;
  qboolean voiceOnly;

  voiceOnly = atoi(CG_Argv(1));
  clientNum = atoi(CG_Argv(2));
  color     = atoi(CG_Argv(3));
  cmd       = CG_Argv(4);

  if (cg_noTaunt.integer != 0)
  {
    if (!qstrcmp(cmd, VOICECHAT_KILLINSULT)  || !qstrcmp(cmd, VOICECHAT_TAUNT) || \
        !qstrcmp(cmd, VOICECHAT_DEATHINSULT) || !qstrcmp(cmd, VOICECHAT_KILLGAUNTLET) || \
        !qstrcmp(cmd, VOICECHAT_PRAISE))
    {
      return;
    }
  }

  CG_VoiceChatLocal(mode, voiceOnly, clientNum, color, cmd);
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar(char *text)
{
  int i, l;

  l = 0;
  for (i = 0; text[i]; i++)
  {
    if (text[i] == '\x19')
      continue;

    text[l++] = text[i];
  }
  text[l] = '\0';
}

static void CG_PmoveParams_f() {
	char arg1[64], arg2[64], arg3[64], arg4[64];

	if (trap_Argc() != 5) {
		return;
	}

	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));
	trap_Argv(3, arg3, sizeof(arg3));
	trap_Argv(4, arg4, sizeof(arg4));

	cg.pmoveParams.synchronous = atoi(arg1);
	cg.pmoveParams.fixed = atoi(arg2);
	cg.pmoveParams.msec = atoi(arg3);
	cg.pmoveParams.accurate = atoi(arg4);

	if (cg.pmoveParams.msec < 8) {
		cg.pmoveParams.msec = 8;
	} else if (cg.pmoveParams.msec > 33) {
		cg.pmoveParams.msec = 33;
	}
}

/*
=================
CG_ServerCommand
The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand(void)
{
  const char *cmd;
  char text[MAX_SAY_TEXT];

  cmd = CG_Argv(0);

  if (!cmd[0])
    return;  // server claimed the command

  if (!qstrcmp(cmd, "cp"))
  {
    CG_CenterPrint(CG_Argv(1), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH);
    return;
  }

  if (!qstrcmp(cmd, "cs"))
  {
    CG_ConfigStringModified();
    return;
  }

  if (!qstrcmp(cmd, "print"))
  {
    CG_Printf("%s", CG_Argv(1));
    cmd = CG_Argv(1);           // yes, this is obviously a hack, but so is the way we hear about
    // votes passing or failing
    if (!Q_stricmpn(cmd, "vote failed", 11) || !Q_stricmpn(cmd, "team vote failed", 16))
      trap_S_StartLocalSound(cgs.media.voteFailed, CHAN_ANNOUNCER);
    else if (!Q_stricmpn(cmd, "vote passed", 11) || !Q_stricmpn(cmd, "team vote passed", 16))
      trap_S_StartLocalSound(cgs.media.votePassed, CHAN_ANNOUNCER);
    return;
  }

  if (!qstrcmp(cmd, "chat"))
  {
    if (!cg_teamChatsOnly.integer)
    {
      trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
      Q_strncpyz(text, CG_Argv(1), MAX_SAY_TEXT);
      CG_RemoveChatEscapeChar(text);
      CG_AddToTeamChat(text);         /* 0xA5EA */
      CG_Printf("%s\n", text);        /* 0xA5EA, adds chat to console */
    }
    return;
  }

  if (!qstrcmp(cmd, "tchat"))
  {
    trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
    Q_strncpyz(text, CG_Argv(1), MAX_SAY_TEXT);
    CG_RemoveChatEscapeChar(text);
    CG_AddToTeamChat(text);
    //CG_Printf( "%s hihi\n", text );
    Com_Printf("%s", text);
    return;
  }
  if (!qstrcmp(cmd, "vchat"))
  {
    CG_VoiceChat(SAY_ALL);
    return;
  }

  if (!qstrcmp(cmd, "vtchat"))
  {
    CG_VoiceChat(SAY_TEAM);
    return;
  }

  if (!qstrcmp(cmd, "vtell"))
  {
    CG_VoiceChat(SAY_TELL);
    return;
  }

  if (!qstrcmp(cmd, "scores"))
  {
    CG_ParseScores();
    return;
  }

  if (!qstrcmp(cmd, "tinfo"))
  {
    CG_ParseTeamInfo();
    return;
  }

  if (!qstrcmp(cmd, "map_restart"))
  {
    CG_MapRestart();
    return;
  }

  if (Q_stricmp(cmd, "remapShader") == 0)
  {
    if (trap_Argc() == 4)
      trap_R_RemapShader(CG_Argv(1), CG_Argv(2), CG_Argv(3));
  }

  // loaddeferred can be both a servercmd and a consolecmd
  if (!qstrcmp(cmd, "loaddefered"))   // FIXME: spelled wrong, but not changing for demo
  {
    CG_LoadDeferredPlayers();
    return;
  }

  // clientLevelShot is sent before taking a special screenshot for
  // the menu system during development
  if (!qstrcmp(cmd, "clientLevelShot"))
  {
    cg.levelShot = qtrue;
    return;
  }

  if (!qstrcmp(cmd, "pmove_params"))
  {
	CG_PmoveParams_f();
    return;
  }


  CG_Printf("Unknown client game command: %s\n", cmd);
}

/*
====================
CG_ExecuteNewServerCommands
Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands(int latestSequence)
{
  while (cgs.serverCommandSequence < latestSequence)
  {
    if (trap_GetServerCommand(++cgs.serverCommandSequence))
      CG_ServerCommand();
  }
}
