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

#include "g_local.h"
#include "g_team.h"

//static int num_cash_items;
typedef struct teamgame_s
{
  float last_flag_capture;
  int last_capture_team;
  flagStatus_t redStatus;  // CTF
  flagStatus_t blueStatus; // CTF
  flagStatus_t flagStatus; // One Flag CTF
  int redTakenTime;
  int blueTakenTime;
  int redObeliskAttackedTime;
  int blueObeliskAttackedTime;
#ifdef WITH_BAGMAN_MOD
  //int nikkicashcount;
  //int dragoncashcount;
  //int touchedNikkiSafeTime;
  //int touchedDragonSafeTime;
#endif
} teamgame_t;

teamgame_t teamgame;
gentity_t *neutralObelisk;

static const char ctfFlagStatusRemap[] = { '0', '1', '*', '*', '2' };
static const char oneFlagStatusRemap[] = { '0', '1', '2', '3', '4' };

static void Team_SetFlagStatus(int team, flagStatus_t status);
static const char *TeamName(int team);
//static const char *OtherTeamName(int team);
//static const char *TeamColorString(int team);
void BM_CashDie(gentity_t *ent);

/*
================
  Team_InitGame
================
*/
void Team_InitGame(void)
{
  Com_Memset(&teamgame, 0, sizeof teamgame);

  switch (g_gametype.integer)
  {
  case GT_CTF:
    teamgame.redStatus = teamgame.blueStatus = (flagStatus_t)-1; // Invalid to force update
    Team_SetFlagStatus(TEAM_DRAGONS, FLAG_ATBASE);
    Team_SetFlagStatus(TEAM_NIKKIS, FLAG_ATBASE);
    break;
#ifdef WITH_BAGMAN_MOD
  case GT_BAGMAN:
  //level.teamScores[TEAM_DRAGONS] = 0;
  //level.teamScores[TEAM_NIKKIS] = 0;
    //FIXME(0xA5EA): bagman, implement
    break;
#endif
  case GT_1FCTF:
    teamgame.flagStatus = (flagStatus_t)-1; // Invalid to force update
    Team_SetFlagStatus(TEAM_FREE, FLAG_ATBASE);
    break;
  default:
    break;
  }
}

/*
================
  Team_OtherTeam
================
*/
int Team_OtherTeam(int team)
{
  if (team == TEAM_DRAGONS)
    return TEAM_NIKKIS;
  else if (team == TEAM_NIKKIS)
    return TEAM_DRAGONS;
  return team;
}

/*
================
  TeamName
================
*/
static const char *TeamName(int team)
{
  if (team == TEAM_DRAGONS)
    return TEAM_NAME_DRAGONS;
  else if (team == TEAM_NIKKIS)
    return TEAM_NAME_NIKKIS;
  else if (team == TEAM_SPECTATOR)
    return "Spectator";

  return "FREE";
}

//(0xA5EA): currently unused
#if 0
/*
================
 OtherTeamName
================
*/
static const char *OtherTeamName(int team)
{
  if (team == TEAM_DRAGONS)
    return TEAM_NAME_NIKKIS;
  else if (team == TEAM_NIKKIS)
    return TEAM_NAME_DRAGONS;
  else if (team == TEAM_SPECTATOR)
    return "Spectator";
  return "FREE";
}

/*
================
 OtherTeamName
================
*/
static const char *TeamColorString(int team)
{
  if (team == TEAM_DRAGONS)
    return S_COLOR_RED;
  else if (team == TEAM_NIKKIS)
    return S_COLOR_YELLOW;
  else if (team == TEAM_SPECTATOR)
    return S_COLOR_BLUE;
  return S_COLOR_WHITE;
}
#endif
/*
================
 PrintMsg
================
*/
static void QDECL PrintMsg(gentity_t *ent, const char *fmt, ...)
{
  char msg[1024];
  va_list argptr;
  char *p;

  va_start(argptr, fmt);

  if (Q_vsnprintf(msg, sizeof(msg), fmt, argptr) > (int) sizeof(msg))
    G_Error("PrintMsg overrun");

  va_end(argptr);

  // double quotes are bad
  while ((p = strchr(msg, '"')) != NULL)
    *p = '\'';

  trap_SendServerCommand(((ent == NULL) ? -1 : ent - g_entities), va("print \"%s\"", msg));
}

/*
 ==============
 AddTeamScore
 used for gametype > GT_TEAM
 for gametype GT_TEAM the level.teamScores is updated in AddScore in g_combat.c
 ==============
 */
static void AddTeamScore(vec3_t origin, int team, int score)
{
  gentity_t *te;

  te = G_TempEntity(origin, EV_GLOBAL_TEAM_SOUND);
  te->r.svFlags |= SVF_BROADCAST;

  if (team == TEAM_DRAGONS)
  {
    if (level.teamScores[TEAM_DRAGONS] + score == level.teamScores[TEAM_NIKKIS])
      te->s.eventParm = GTS_TEAMS_ARE_TIED;       //teams are tied sound
    else if (level.teamScores[TEAM_DRAGONS] <= level.teamScores[TEAM_NIKKIS] && level.teamScores[TEAM_DRAGONS] + score > level.teamScores[TEAM_NIKKIS])
      te->s.eventParm = GTS_DRAGONTEAM_TOOK_LEAD; // red took the lead sound
    else
      te->s.eventParm = GTS_DRAGONTEAM_SCORED;    // red scored sound
  }
  else
  {
    if (level.teamScores[TEAM_NIKKIS] + score == level.teamScores[TEAM_DRAGONS])
      te->s.eventParm = GTS_TEAMS_ARE_TIED;  //teams are tied sound
    else if (level.teamScores[TEAM_NIKKIS] <= level.teamScores[TEAM_DRAGONS] && level.teamScores[TEAM_NIKKIS] + score > level.teamScores[TEAM_DRAGONS])
      te->s.eventParm = GTS_NIKKITEAM_TOOK_LEAD; // blue took the lead sound
    else
      te->s.eventParm = GTS_NIKKITEAM_SCORED; // blue scored sound
  }
  level.teamScores[team] += score;
}

#ifdef WITH_BAGMAN_MOD
/*
 ==============
 BM_AddTeamScore
 used for gametype > GT_TEAM
 for gametype GT_TEAM the level.teamScores is updated in AddScore in g_combat.c
 ==============
 */
static void BM_AddTeamScore(vec3_t origin, int team, int score)
{
  gentity_t *te;

  te = G_TempEntity(origin, EV_GLOBAL_TEAM_SOUND);
  te->r.svFlags |= SVF_BROADCAST;

  if (team == TEAM_DRAGONS)
    te->s.eventParm = GTS_DRAGON_DEPOSIT;
  else
    te->s.eventParm = GTS_NIKKI_DEPOSIT;

  level.teamScores[team] += score;
}
#endif

/*
==============
 OnSameTeam
==============
*/
qboolean OnSameTeam(gentity_t *ent1, gentity_t *ent2) //hypov8 same team, use to disable weapon hit decals
{
  if (!ent1->client || !ent2->client)
    return qfalse;

  if (g_gametype.integer < GT_TEAM)
    return qfalse;

  if (ent1->client->sess.sessionTeam == ent2->client->sess.sessionTeam)
    return qtrue;

  return qfalse;
}

/*
==============
 Team_SetFlagStatus
==============
*/
static void Team_SetFlagStatus(int team, flagStatus_t status)
{
  qboolean modified = qfalse;

  switch (team)
  {
  case TEAM_DRAGONS: // CTF
    if (teamgame.redStatus != status)
    {
      teamgame.redStatus = status;
      modified = qtrue;
    }
    break;
  case TEAM_NIKKIS: // CTF
    if (teamgame.blueStatus != status)
    {
      teamgame.blueStatus = status;
      modified = qtrue;
    }
    break;
  case TEAM_FREE: // One Flag CTF
    if (teamgame.flagStatus != status)
    {
      teamgame.flagStatus = status;
      modified = qtrue;
    }
    break;
  }

  if (modified)
  {
    char st[4];
    if (g_gametype.integer == GT_CTF)
    {
      if (team == TEAM_DRAGONS)
        st[0] = ctfFlagStatusRemap[teamgame.redStatus];
      else if (team == TEAM_NIKKIS)
        st[1] = ctfFlagStatusRemap[teamgame.blueStatus];

      st[2] = 0;
    }
    else if (g_gametype.integer == GT_1FCTF)
    {
      st[0] = oneFlagStatusRemap[teamgame.flagStatus];
      st[1] = 0;
    }
    //FIXME(0xA5EA): bagman ?
    trap_SetConfigstring(CS_FLAGSTATUS, st);
  }
}

/*
==============
 Team_CheckDroppedItem
==============
*/
void Team_CheckDroppedItem(gentity_t *dropped)
{
  switch (dropped->item->giTag)
  {
  case PW_DRAGONFLAG:
    Team_SetFlagStatus(TEAM_DRAGONS, FLAG_DROPPED);
    return;
  case PW_NIKKIFLAG:
    Team_SetFlagStatus(TEAM_NIKKIS, FLAG_DROPPED);
    return;
  case PW_NEUTRALFLAG:
    Team_SetFlagStatus(TEAM_FREE, FLAG_DROPPED);
    return;
  }
}

/*
================
 Team_ForceGesture
 hypov8 ctf only?
 will taunt for every team member
 should enforce a specific taunt?
================
*/
static void Team_ForceGesture(int team)
{
  int i;
  gentity_t *ent;

  for (i = 0; i < MAX_CLIENTS; i++)
  {
    ent = &g_entities[i];

    if (!ent->inuse)
      continue;

    if (!ent->client)
      continue;

    if ((int) ent->client->sess.sessionTeam != team)
      continue;

    ent->flags |= FL_FORCE_GESTURE;
  }
}

/*
 ================
 Team_FragBonuses
 Calculate the bonuses for flag defense, flag carrier defense, etc.
 Note that bonuses are not cumulative.  You get one, they are in importance
 order.
 ================
 */
void Team_FragBonuses(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker)
{
  int i;
  gentity_t *ent;
  int flag_pw, enemy_flag_pw;
  int otherteam;
  int tokens;
  gentity_t *flag, *carrier = NULL;
  char *c = 0;
  vec3_t v1, v2;
  int team;

  // no bonus for fragging yourself or team mates
  if (!targ->client || !attacker->client || targ == attacker || OnSameTeam(targ, attacker))
    return;

  team = targ->client->sess.sessionTeam;
  otherteam = Team_OtherTeam(targ->client->sess.sessionTeam);
  if (otherteam < 0)
    return; // whoever died isn't on a team

  // same team, if the flag at base, check to he has the enemy flag
  if (team == TEAM_DRAGONS)
  {
    flag_pw = PW_DRAGONFLAG;
    enemy_flag_pw = PW_NIKKIFLAG;
  }
  else
  {
    flag_pw = PW_NIKKIFLAG;
    enemy_flag_pw = PW_DRAGONFLAG;
  }
  if (g_gametype.integer == GT_1FCTF)
  {
    enemy_flag_pw = PW_NEUTRALFLAG;
  }
  // did the attacker frag the flag carrier?
  tokens = 0;

#ifdef GT_USE_TA_TYPES
  if (g_gametype.integer == GT_HARVESTER)
    tokens = targ->client->ps.generic1;
#endif

  if (targ->client->ps.powerups[enemy_flag_pw])
  {
    attacker->client->pers.teamState.lastfraggedcarrier = level.time;
    AddScore(attacker, targ->r.currentOrigin, CTF_FRAG_CARRIER_BONUS);
    attacker->client->pers.teamState.fragcarrier++;
    PrintMsg(NULL, "%s" S_COLOR_WHITE " fragged %s's flag carrier!\n",
        attacker->client->pers.netname, TeamName(team));

    // the target had the flag, clear the hurt carrier
    // field on the other team
    for (i = 0; i < g_maxclients.integer; i++)
    {
      ent = g_entities + i;
      if (ent->inuse && (int) ent->client->sess.sessionTeam == otherteam)
        ent->client->pers.teamState.lasthurtcarrier = 0;
    }
    return;
  }

  // did the attacker frag a head carrier? other->client->ps.generic1
  if (tokens)
  {
    attacker->client->pers.teamState.lastfraggedcarrier = level.time;
    AddScore(attacker, targ->r.currentOrigin, CTF_FRAG_CARRIER_BONUS * tokens
        * tokens);
    attacker->client->pers.teamState.fragcarrier++;
    PrintMsg(NULL, "%s" S_COLOR_WHITE " fragged %s's skull carrier!\n",
        attacker->client->pers.netname, TeamName(team));

    // the target had the flag, clear the hurt carrier
    // field on the other team
    for (i = 0; i < g_maxclients.integer; i++)
    {
      ent = g_entities + i;
      if (ent->inuse && (int) ent->client->sess.sessionTeam == otherteam)
        ent->client->pers.teamState.lasthurtcarrier = 0;
    }
    return;
  }

  if (targ->client->pers.teamState.lasthurtcarrier && level.time
      - targ->client->pers.teamState.lasthurtcarrier
      < CTF_CARRIER_DANGER_PROTECT_TIMEOUT
      && !attacker->client->ps.powerups[flag_pw])
  {
    // attacker is on the same team as the flag carrier and
    // fragged a guy who hurt our flag carrier
    AddScore(attacker, targ->r.currentOrigin, CTF_CARRIER_DANGER_PROTECT_BONUS);

    attacker->client->pers.teamState.carrierdefense++;
    targ->client->pers.teamState.lasthurtcarrier = 0;

    attacker->client->ps.persistant[PERS_DEFEND_COUNT]++;
    team = attacker->client->sess.sessionTeam;
    // add the sprite over the player's head
    attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST
        | EF_AWARD_DEFEND | EF_AWARD_CAP);
    attacker->client->ps.eFlags |= EF_AWARD_DEFEND;
    attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

    return;
  }

  if (targ->client->pers.teamState.lasthurtcarrier && level.time
      - targ->client->pers.teamState.lasthurtcarrier
      < CTF_CARRIER_DANGER_PROTECT_TIMEOUT)
  {
    // attacker is on the same team as the skull carrier and
    AddScore(attacker, targ->r.currentOrigin, CTF_CARRIER_DANGER_PROTECT_BONUS);

    attacker->client->pers.teamState.carrierdefense++;
    targ->client->pers.teamState.lasthurtcarrier = 0;

    attacker->client->ps.persistant[PERS_DEFEND_COUNT]++;
    team = attacker->client->sess.sessionTeam;
    // add the sprite over the player's head
    attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST
        | EF_AWARD_DEFEND | EF_AWARD_CAP);
    attacker->client->ps.eFlags |= EF_AWARD_DEFEND;
    attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

    return;
  }

  // flag and flag carrier area defense bonuses
  // we have to find the flag and carrier entities
#ifdef GT_USE_TA_TYPES
  if (g_gametype.integer == GT_OBELISK)
  {
    // find the team obelisk
    switch(attacker->client->sess.sessionTeam)
    {
      case TEAM_DRAGONS:
      c = "team_redobelisk";
      break;
      case TEAM_NIKKIS:
      c = "team_blueobelisk";
      break;
      default:
      return;
    }

  }
  else if (g_gametype.integer == GT_HARVESTER)
  {
    // find the center obelisk
    c = "team_neutralobelisk";
  }
  else
  {
#endif
  if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)
  {
    // find the flag
    switch (attacker->client->sess.sessionTeam)
    {
    case TEAM_DRAGONS:
      c = "team_ctf_dragonsflag";
      break;
    case TEAM_NIKKIS:
      c = "team_ctf_nikkisflag";
      break;
    default:
      c = " team_ctf_neutralflag";
      break;
    }
    // find attacker's team's flag carrier
    for (i = 0; i < g_maxclients.integer; i++)
    {
      carrier = g_entities + i;
      if (carrier->inuse && carrier->client->ps.powerups[flag_pw])
        break;
      carrier = NULL;
    }
  }
#ifdef  WITH_BAGMAN_MOD
  //FIXME(0xA5EA): bagman is this correct here
  else if ( g_gametype.integer == GT_BAGMAN )		// gilt doch nur fuer cash, nicht fuer den safe !!
  {
    switch (attacker->client->sess.sessionTeam)
    {
    case TEAM_DRAGONS:
      c = "team_bm_dragonssafe";
      break;
    case TEAM_NIKKIS:
      c = "team_bm_nikkissafe";
      break;
    default:
      return;
    }
  }
#endif
#ifdef GT_USE_TA_TYPES
}
#endif
  flag = NULL;
  while ((flag = G_Find(flag, FOFS(classname), c)) != NULL)
  {
    if (!(flag->flags & FL_DROPPED_ITEM))
      break;
  }

  if (!flag)
    return; // can't find attacker's flag

  // ok we have the attackers flag and a pointer to the carrier

  // check to see if we are defending the base's flag
  VectorSubtract(targ->r.currentOrigin, flag->r.currentOrigin, v1);
  VectorSubtract(attacker->r.currentOrigin, flag->r.currentOrigin, v2);

  if (((VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS && trap_InPVS(
      flag->r.currentOrigin, targ->r.currentOrigin)) || (VectorLength(v2)
      < CTF_TARGET_PROTECT_RADIUS && trap_InPVS(flag->r.currentOrigin,
      attacker->r.currentOrigin))) && attacker->client->sess.sessionTeam
      != targ->client->sess.sessionTeam)
  {

    // we defended the base flag
    AddScore(attacker, targ->r.currentOrigin, CTF_FLAG_DEFENSE_BONUS);
    attacker->client->pers.teamState.basedefense++;

    attacker->client->ps.persistant[PERS_DEFEND_COUNT]++;
    // add the sprite over the player's head
    attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST
        | EF_AWARD_DEFEND | EF_AWARD_CAP);
    attacker->client->ps.eFlags |= EF_AWARD_DEFEND;
    attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

    return;
  }

  if (carrier && carrier != attacker)
  {
    VectorSubtract(targ->r.currentOrigin, carrier->r.currentOrigin, v1);
    VectorSubtract(attacker->r.currentOrigin, carrier->r.currentOrigin, v1);

    if (((VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS && trap_InPVS(
        carrier->r.currentOrigin, targ->r.currentOrigin)) || (VectorLength(v2)
        < CTF_ATTACKER_PROTECT_RADIUS && trap_InPVS(carrier->r.currentOrigin,
        attacker->r.currentOrigin))) && attacker->client->sess.sessionTeam
        != targ->client->sess.sessionTeam)
    {
      AddScore(attacker, targ->r.currentOrigin, CTF_CARRIER_PROTECT_BONUS);
      attacker->client->pers.teamState.carrierdefense++;

      attacker->client->ps.persistant[PERS_DEFEND_COUNT]++;
      // add the sprite over the player's head
      attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST
          | EF_AWARD_DEFEND | EF_AWARD_CAP);
      attacker->client->ps.eFlags |= EF_AWARD_DEFEND;
      attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

      return;
    }
  }
}

/*
 ================
 Team_CheckHurtCarrier

 Check to see if attacker hurt the flag carrier.  Needed when handing out bonuses for assistance to flag
 carrier defense.
 ================
 */
void Team_CheckHurtCarrier(gentity_t *targ, gentity_t *attacker)
{
  int flag_pw;

  if (!targ->client || !attacker->client)
    return;

  if (targ->client->sess.sessionTeam == TEAM_DRAGONS)
    flag_pw = PW_NIKKIFLAG;
  else
    flag_pw = PW_DRAGONFLAG;

  // flags
  if (targ->client->ps.powerups[flag_pw] && targ->client->sess.sessionTeam
      != attacker->client->sess.sessionTeam)
    attacker->client->pers.teamState.lasthurtcarrier = level.time;

  // skulls
  if (targ->client->ps.generic1 && targ->client->sess.sessionTeam
      != attacker->client->sess.sessionTeam)
    attacker->client->pers.teamState.lasthurtcarrier = level.time;
}

/*
================
 Team_ResetFlag
================
*/
static gentity_t *Team_ResetFlag(int team)
{
  char *c;
  gentity_t *ent = NULL, *rent = NULL;

  switch (team)
  {
  case TEAM_DRAGONS: c = "team_ctf_dragonsflag"; break;
  case TEAM_NIKKIS:  c = "team_ctf_nikkisflag";  break;
  case TEAM_FREE:    c = "team_ctf_neutralflag"; break;
  default: return NULL;
  }

  while ((ent = G_Find(ent, FOFS(classname), c)) != NULL)
  {
    if (ent->flags & FL_DROPPED_ITEM)
      G_FreeEntity(ent);
    else
    {
      rent = ent;
      RespawnItem(ent);
    }
  }

  Team_SetFlagStatus(team, FLAG_ATBASE);

  return rent;
}



/*
================
 Team_ResetFlags
================
*/
static void Team_ResetFlags(void)
{
  if (g_gametype.integer == GT_CTF)
  {
    Team_ResetFlag(TEAM_DRAGONS);
    Team_ResetFlag(TEAM_NIKKIS);
  }
  else if (g_gametype.integer == GT_1FCTF)
  {
    Team_ResetFlag(TEAM_FREE);
  }
}

/*
================
 Team_ReturnFlagSound
================
*/
static void Team_ReturnFlagSound(gentity_t *ent, int team)
{
  gentity_t *te;

  if (ent == NULL)
  {
    G_Printf("Warning:  NULL passed to Team_ReturnFlagSound\n");
    return;
  }

  te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);

  if (team == TEAM_NIKKIS)
    te->s.eventParm = GTS_DRAGON_RETURN;
  else
    te->s.eventParm = GTS_NIKKI_RETURN;

  te->r.svFlags |= SVF_BROADCAST;
}

/*
================
 Team_TakeFlagSound
================
*/
static void Team_TakeFlagSound(gentity_t *ent, int team)
{
  gentity_t *te;

  if (ent == NULL)
  {
    G_Printf(S_COLOR_YELLOW"Warning:  NULL passed to Team_TakeFlagSound\n");
    return;
  }

  // only play sound when the flag was at the base
  // or not picked up the last 10 seconds
  switch (team)
  {
  case TEAM_DRAGONS:
    if (teamgame.blueStatus != FLAG_ATBASE)
    {
      if (teamgame.blueTakenTime > level.time - 10000)
        return;
    }
    teamgame.blueTakenTime = level.time;
    break;

  case TEAM_NIKKIS: // CTF
    if (teamgame.redStatus != FLAG_ATBASE)
    {
      if (teamgame.redTakenTime > level.time - 10000)
        return;
    }
    teamgame.redTakenTime = level.time;
    break;
  }

  te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);

  if (team == TEAM_NIKKIS)
    te->s.eventParm = GTS_DRAGON_TAKEN;
  else
    te->s.eventParm = GTS_NIKKI_TAKEN;

  te->r.svFlags |= SVF_BROADCAST;
}



/*
================
Team_TakeBagSound
 add hypov8
================
*/
static void BM_SafeStolenSound(gentity_t *ent, int team)
{
  gentity_t *te;

  te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);
  te->r.svFlags |= SVF_BROADCAST;
  if (team == TEAM_NIKKIS)
    te->s.eventParm = GTS_DRAGON_STOLEN;
  else
    te->s.eventParm = GTS_NIKKI_STOLEN;
}


/*
================
 Team_CaptureFlagSound
================
*/
static void Team_CaptureFlagSound(gentity_t *ent, int team)
{
  gentity_t *te;

  if (ent == NULL)
  {
    G_Printf(S_COLOR_YELLOW"Warning:  NULL passed to Team_CaptureFlagSound\n");
    return;
  }

  te = G_TempEntity(ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);

  if (team == TEAM_NIKKIS)
    te->s.eventParm = GTS_NIKKI_CAPTURE;
  else
    te->s.eventParm = GTS_DRAGON_CAPTURE;

  te->r.svFlags |= SVF_BROADCAST;
}

/*
================
 Team_ReturnFlag
================
*/
void Team_ReturnFlag(int team)
{
  Team_ReturnFlagSound(Team_ResetFlag(team), team);

  if (team == TEAM_FREE)
    PrintMsg(NULL, "The flag has returned!\n");
  else
    PrintMsg(NULL, "The %s flag has returned!\n", TeamName(team));
}

/*
================
 BM_FindGentity
================
*/
#ifdef WITH_BAGMAN_MOD
static gentity_t *BM_FindTeamsSafe(int team)
{
  char *c;
  gentity_t *ent = NULL;

  switch (team)
  {
  case TEAM_DRAGONS:
    c = "team_bm_dragonssafe";
    break;
  case TEAM_NIKKIS:
    c = "team_bm_nikkissafe";
    break;
  default:
    return NULL;
  }

  while ((ent = G_Find(ent, FOFS(classname), c)) != NULL)
  {
    return ent;
  }
  return NULL;
}

/*
================
 BM_ReturnCash
================
*/
void BM_ReturnCash(gentity_t *self)
{
  int cashTeam = Team_OtherTeam(self->client->sess.sessionTeam);
  int cash = self->client->ps.stats[STAT_STOLEN_CASH];

  if (cashTeam == 1)
    level.teamScores[TEAM_DRAGONS] += cash;
  else if (cashTeam == 2)
    level.teamScores[TEAM_NIKKIS] += cash;
  else
  {
    G_Printf(S_COLOR_YELLOW"Warning: wrong team passed to BM_ReturnCash\n");
    return;
  }
  //hypov8 need to update scorebord
  CalculateRanks();

  self->client->ps.powerups[PW_STOLENSCASH] = 0;
  self->client->ps.persistant[PERS_CASH_STOLEN] -= self->client->ps.stats[STAT_STOLEN_CASH];
  self->client->ps.stats[STAT_STOLEN_CASH] = 0;

  Team_ReturnFlagSound(BM_FindTeamsSafe(cashTeam), cashTeam);
  PrintMsg(NULL, "The %s cash %i has returned!\n", TeamName(cashTeam), cash);
  //FIXME(0xA5EA): is this ok ? something missing ?
}
#endif // WITH_BAGMAN_MOD
/*
================
 Team_FreeEntity
================
*/
void Team_FreeEntity(gentity_t *ent)
{
  switch (ent->item->giTag)
  {
  case PW_DRAGONFLAG:
    Team_ReturnFlag(TEAM_DRAGONS);
    return;
  case PW_NIKKIFLAG:
    Team_ReturnFlag(TEAM_NIKKIS);
    return;
  case PW_NEUTRALFLAG:
    Team_ReturnFlag(TEAM_FREE);
    return;
  }
}

/*
 ==============
 Team_DroppedFlagThink
 Automatically set in Launch_Item if the item is one of the flags
 Flags are unique in that if they are dropped, the base flag must be respawned when they time out
 ==============
 */
void Team_DroppedFlagThink(gentity_t *ent)
{
  int team = TEAM_FREE;
  switch (ent->item->giTag)
  {
  case PW_DRAGONFLAG:
    team = TEAM_DRAGONS;
    break;
  case PW_NIKKIFLAG:
    team = TEAM_NIKKIS;
    break;
  case PW_NEUTRALFLAG:
    team = TEAM_FREE;
    break;
  }

  Team_ReturnFlagSound(Team_ResetFlag(team), team);
}


/*
==============
Team_CaptureFlag
==============
*/
static int Team_CaptureFlag(gentity_t *ent, gentity_t *other, int team)
{
  int i;
  gentity_t *player;
  gclient_t *cl = other->client;
  int enemy_flag;

  if (g_gametype.integer == GT_1FCTF)
    enemy_flag = PW_NEUTRALFLAG;
  else
  {
    if (cl->sess.sessionTeam == TEAM_DRAGONS)
      enemy_flag = PW_NIKKIFLAG;
    else
      enemy_flag = PW_DRAGONFLAG;
  }

  // the flag is at home base.  if the player has the enemy
  // flag, he's just won!
  if (!cl->ps.powerups[enemy_flag])
    return 0; // We don't have the flag

  if (g_gametype.integer == GT_1FCTF)
    PrintMsg(NULL, "%s" S_COLOR_WHITE " captured the flag!\n", cl->pers.netname);
  else
    PrintMsg(NULL, "%s" S_COLOR_WHITE " captured the %s flag!\n", cl->pers.netname, TeamName(Team_OtherTeam(team)));

  cl->ps.powerups[enemy_flag] = 0;

  teamgame.last_flag_capture = level.time;
  teamgame.last_capture_team = team;

  // Increase the team's score
  AddTeamScore(ent->s.pos.trBase, other->client->sess.sessionTeam, 1);
  Team_ForceGesture(other->client->sess.sessionTeam);

  other->client->pers.teamState.captures++;
  // add the sprite over the player's head
  other->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
  other->client->ps.eFlags |= EF_AWARD_CAP;
  other->client->rewardTime = level.time + REWARD_SPRITE_TIME;
  other->client->ps.persistant[PERS_CAPTURES]++;

  // other gets another 10 frag bonus
  AddScore(other, ent->r.currentOrigin, CTF_CAPTURE_BONUS);

  Team_CaptureFlagSound(ent, team);

  // Ok, let's do the player loop, hand out the bonuses
  for (i = 0; i < g_maxclients.integer; i++)
  {
    player = &g_entities[i];

    if (!player->inuse)
      continue;

    if (player->client->sess.sessionTeam != cl->sess.sessionTeam)
    {
      player->client->pers.teamState.lasthurtcarrier = -5;
    }
    else if (player->client->sess.sessionTeam == cl->sess.sessionTeam)
    {
      if (player != other)
        AddScore(player, ent->r.currentOrigin, CTF_TEAM_BONUS);
      // award extra points for capture assists
      if (player->client->pers.teamState.lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
      {
        AddScore(player, ent->r.currentOrigin, CTF_RETURN_FLAG_ASSIST_BONUS);
        other->client->pers.teamState.assists++;

        BG_PersAddToAssistCount(1, &player->client->ps.persistant[PERS_EXCELLENT_ASSIST_COUNT]);
        // add the sprite over the player's head
        player->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
        player->client->ps.eFlags |= EF_AWARD_ASSIST;
        player->client->rewardTime = level.time + REWARD_SPRITE_TIME;

      }
      else if (player->client->pers.teamState.lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
      {
        AddScore(player, ent->r.currentOrigin, CTF_FRAG_CARRIER_ASSIST_BONUS);
        other->client->pers.teamState.assists++;
        BG_PersAddToAssistCount(1, &player->client->ps.persistant[PERS_EXCELLENT_ASSIST_COUNT]);
        // add the sprite over the player's head
        player->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
        player->client->ps.eFlags |= EF_AWARD_ASSIST;
        player->client->rewardTime = level.time + REWARD_SPRITE_TIME;
      }
    }
  }

  Team_ResetFlags();

  CalculateRanks();

  return 0; // Do not respawn this automatically
}

static int Team_TouchOurFlag(gentity_t * ent, gentity_t * other, int team)
{
  if (g_gametype.integer == GT_1FCTF || g_gametype.integer == GT_CTF)
  {
    if (ent->flags & FL_DROPPED_ITEM)
    {
      // hey, its not home.  return it by teleporting it back
      PrintMsg(NULL, "%s" S_COLOR_WHITE " returned the %s flag!\n", other->client->pers.netname, TeamName(team));
      AddScore(other, ent->r.currentOrigin, CTF_RECOVERY_BONUS);
      other->client->pers.teamState.flagrecovery++;
      other->client->pers.teamState.lastreturnedflag = level.time;
      //ResetFlag will remove this entity!  We must return zero
      Team_ReturnFlagSound(Team_ResetFlag(team), team);
      return 0;
    }
  }
  return Team_CaptureFlag(ent, other, team);
}


/*
================
 Team_TouchEnemyFlag
================
*/
static int Team_TouchEnemyFlag(gentity_t *ent, gentity_t *other, int team)
{
  gclient_t *cl = other->client;

  if (g_gametype.integer == GT_1FCTF)
  {
    PrintMsg(NULL, "%s" S_COLOR_WHITE " got the flag!\n", other->client->pers.netname);

    cl->ps.powerups[PW_NEUTRALFLAG] = 1; //INT_MAX; // flags never expire

    if (team == TEAM_DRAGONS)
      Team_SetFlagStatus(TEAM_FREE, FLAG_TAKEN_DRAGON);
    else
      Team_SetFlagStatus(TEAM_FREE, FLAG_TAKEN_NIKKI);
  }
  else
  {
    PrintMsg(NULL, "%s" S_COLOR_WHITE " got the %s flag!\n", other->client->pers.netname, TeamName(team));

    if (team == TEAM_DRAGONS)
      cl->ps.powerups[PW_DRAGONFLAG] = 1; //INT_MAX; // flags never expire
    else
      cl->ps.powerups[PW_NIKKIFLAG] = 1; //INT_MAX; // flags never expire

    Team_SetFlagStatus(team, FLAG_TAKEN);
  }

  AddScore(other, ent->r.currentOrigin, CTF_FLAG_BONUS);
  cl->pers.teamState.flagsince = level.time;
  Team_TakeFlagSound(ent, team);

  return -1; // Do not respawn this automatically, but do delete it if it was FL_DROPPED
}


#ifdef WITH_BAGMAN_MOD
/*
==============
BM_IsSafeEmpty
==============
*/
static qboolean BM_IsSafeEmpty(int team)
{
  switch(team)
  {
  case TEAM_DRAGONS:
    if (level.teamScores[TEAM_DRAGONS] <= 0)
      return qtrue;
    break;
  case TEAM_NIKKIS:
    if (level.teamScores[TEAM_NIKKIS] <= 0)
      return qtrue;
    break;
  }

  return qfalse;
}


/*
==============
BM_TouchOurSafe
==============
*/
static void BM_TouchOurSafe(gentity_t *ent, gentity_t *other)
{
  gclient_t *cl = other->client;
  int cash   = cl->ps.stats[STAT_CASH];
  int stolen = cl->ps.stats[STAT_STOLEN_CASH];
  int total = cash + stolen;
  int bonus = 0;

  if (!total)
    return;

  BM_AddTeamScore(ent->s.pos.trBase, cl->sess.sessionTeam, total);

  if (cash)
    bonus += BAGMAN_DEPOSIT_BONUS;

  if (stolen)
    bonus +=  BAGMAN_STEAL_DEPOSIT_BONUS;

  AddScore(other, ent->r.currentOrigin, bonus + total);

  cl->ps.persistant[PERS_CASH_DEPOSIT] += total;

  cl->ps.stats[STAT_CASH] = 0;
  cl->ps.stats[STAT_STOLEN_CASH] = 0;
  cl->ps.powerups[PW_STOLENSCASH] = 0;

  //FIXME(0xA5EA): bagman, was noch zurücksetzen ?
  //FIXME(0xA5EA): bagman only if stolen ??

  //BM_DepositCashSound(other);
}

/*
==============
BM_TouchEnemySafe
==============
*/
static void BM_TouchEnemySafe(gentity_t *ent, gentity_t *other)
{
  gclient_t *cl = other->client;
  int enemyTeam = Team_OtherTeam(cl->sess.sessionTeam);
  int pSafeAmount = 0;
  int take_amount  = 0;


  if (BM_IsSafeEmpty(enemyTeam))
    return;

  if (!BG_CanPickupCashStolen(g_cashstolenmax.integer, &cl->ps)) // hypov8 g_cashcollectmax
    return;

  //FIXME(0xA5EA):bagman cleanup
  PrintMsg(NULL, "%s" S_COLOR_WHITE " stole the %s cash!\n", cl->pers.netname, TeamName(enemyTeam));

  take_amount = g_cashstolenmax.integer - cl->ps.stats[STAT_STOLEN_CASH];
  if (take_amount <= 0)
  {
    G_Printf(S_COLOR_RED"Error: invalid take amount %i\n", take_amount);
    return;
  }

  //set safe values
  if (enemyTeam == TEAM_DRAGONS)
  {
    pSafeAmount = level.teamScores[TEAM_DRAGONS];
    take_amount = (take_amount < pSafeAmount) ? take_amount : pSafeAmount;
    pSafeAmount = level.teamScores[TEAM_DRAGONS] -= take_amount;
  }
  else
  {
    pSafeAmount = level.teamScores[TEAM_NIKKIS];
    take_amount = (take_amount < pSafeAmount) ? take_amount : pSafeAmount;
    level.teamScores[TEAM_NIKKIS] -= take_amount;
  }

  //set client scores
  cl->ps.stats[STAT_STOLEN_CASH] += take_amount;
  cl->ps.persistant[PERS_CASH_STOLEN] += take_amount;
  cl->ps.powerups[PW_STOLENSCASH] = 1; //INT_MAX; // cash never expire

  AddScore(other, ent->r.currentOrigin, BAGMAN_STEAL_BONUS);
  cl->pers.teamState.lasttouchedsafe = level.time;

  BM_SafeStolenSound(ent, enemyTeam);
}

/*
================
 BM_TouchSafe
================
*/
int BM_TouchSafe(gentity_t *ent, gentity_t *other)
{
  int team;
  gclient_t *cl = other->client;


  if (g_gametype.integer == GT_BAGMAN)
  {
    if (qstrcmp(ent->classname, "team_bm_dragonssafe") == 0)
      team = TEAM_DRAGONS;
    else if (qstrcmp(ent->classname, "team_bm_nikkissafe") == 0)
      team = TEAM_NIKKIS;
    else
    {
      PrintMsg(other, "Don't know what team the safe is on.\n");
      return 0;
    }

    //touch own safe
    if (team == cl->sess.sessionTeam)
      BM_TouchOurSafe(ent, other);
    //touch enemy safe
    else if (team != cl->sess.sessionTeam)
      BM_TouchEnemySafe(ent, other);
    }

  return 0;
}
#endif

/*
================
 Pickup_Team
================
*/
int Team_Pickup(gentity_t *ent, gentity_t *other)
{
  int team;
  gclient_t *cl = other->client;


  if (g_gametype.integer == GT_1FCTF || g_gametype.integer == GT_CTF)
  {
    // figure out what team this flag is
    if (qstrcmp(ent->classname, "team_ctf_dragonsflag") == 0)
      team = TEAM_DRAGONS;
    else if (qstrcmp(ent->classname, "team_ctf_nikkisflag") == 0)
      team = TEAM_NIKKIS;
    else if (qstrcmp(ent->classname, "team_ctf_neutralflag") == 0)
      team = TEAM_FREE;
    else
    {
      PrintMsg(other, "Don't know what team the flag is on.\n");
      return 0;
    }

    if (g_gametype.integer == GT_1FCTF)
    {
      if (team == TEAM_FREE)
        return Team_TouchEnemyFlag( ent, other, cl->sess.sessionTeam );

      //reached enemys flag
      if(team != cl->sess.sessionTeam)
        return Team_TouchOurFlag( ent, other, cl->sess.sessionTeam );

      return 0;
    }
    // GT_CTF
    if(team == cl->sess.sessionTeam)
      return Team_TouchOurFlag(ent, other, team);

    return Team_TouchEnemyFlag(ent, other, team);
  }

  return 0;
}

/*
===========
Team_GetLocation
Report a location for the player. Uses placed nearby target_location entities
============
*/
gentity_t *Team_GetLocation(gentity_t *ent)
{
  gentity_t *eloc, *best;
  float bestlen, len;
  vec3_t origin;

  best = NULL;
  bestlen = 3 * 8192.0 * 8192.0;

  VectorCopy(ent->r.currentOrigin, origin);

  for (eloc = level.locationHead; eloc; eloc = eloc->nextTrain)
  {
    len = (origin[0] - eloc->r.currentOrigin[0])*(origin[0] - eloc->r.currentOrigin[0])
        + (origin[1] - eloc->r.currentOrigin[1])*(origin[1] - eloc->r.currentOrigin[1])
        + (origin[2] - eloc->r.currentOrigin[2])*(origin[2] - eloc->r.currentOrigin[2]);

    if (len > bestlen)
      continue;

    if (!trap_InPVS(origin, eloc->r.currentOrigin))
      continue;

    bestlen = len;
    best = eloc;
  }

  return best;
}

/*
===========
 Team_GetLocation
 Report a location for the player. Uses placed nearby target_location entities
============
*/
qboolean Team_GetLocationMsg(gentity_t *ent, char *loc, int loclen)
{
  gentity_t *best;

  best = Team_GetLocation(ent);

  if (!best)
    return qfalse;

  if (best->count)
  {
    if (best->count < 0)
      best->count = 0;

    if (best->count > 7)
      best->count = 7;

    Com_sprintf(loc, loclen, "%c%c%s" S_COLOR_WHITE, Q_COLOR_ESCAPE, best->count + '0', best->message);
  }
  else
    Com_sprintf(loc, loclen, "%s", best->message);

  return qtrue;
}

/*
================
 SelectRandomDeathmatchSpawnPoint
 go to a random point that doesn't telefrag
================
*/
#define MAX_TEAM_SPAWN_POINTS 32
gentity_t *SelectRandomTeamSpawnPoint(int teamstate, team_t team)
{
  gentity_t *spot;
  int count;
  int selection;
  gentity_t *spots[MAX_TEAM_SPAWN_POINTS];
  char *classname;

  //classname2 = NULL;

  if (teamstate == 0) /* TEAM_BEGIN */
  {
    if (team == 1) /*TEAM_DRAGONS  */
    {
      classname = "team_dragonsspawn";
      {
        if (G_Find(NULL, FOFS(classname), "team_dragonsplayer") != NULL) //note hypov8 any faster way?
          classname = "team_dragonsplayer";
      }

    }
    else if (team == 2) /*TEAM_NIKKIS  */
    {
      classname = "team_nikkisspawn";
      {
        if (G_Find(NULL, FOFS(classname), "team_nikkisplayer") != NULL) //note hypov8 any faster way?
          classname = "team_nikkisplayer";
      }
    }
    else
    {
      return NULL; /* note hypov8 deathmatch starts??? */
    }
  }
  else
  {
    if (team == 1) //TEAM_DRAGONS
      classname = "team_dragonsspawn";
    else if (team == 2) //TEAM_NIKKIS
      classname = "team_nikkisspawn";
    else
      return NULL; /* note hypov8 deathmatch starts??? */
  }
  count = 0;

  spot = NULL;

  while ((spot = G_Find(spot, FOFS(classname), classname)) != NULL)
  {
    if (SpotWouldTelefrag(spot))
      continue;

    spots[count] = spot;

    if (++count == MAX_TEAM_SPAWN_POINTS)
      break;
  }

  if (!count) // no spots that won't telefrag
    return G_Find(NULL, FOFS(classname), classname);

  selection = rand() % count;
  return spots[selection];
}

/*
===========
 SelectCTFSpawnPoint
============
*/
gentity_t *SelectCTFSpawnPoint(team_t team, int teamstate, vec3_t origin, vec3_t angles)
{
  gentity_t *spot;

  spot = SelectRandomTeamSpawnPoint(teamstate, team);

  if (!spot)
    return SelectSpawnPoint(vec3_origin, origin, angles);

  VectorCopy(spot->s.origin, origin);
  origin[2] += 9;
  VectorCopy(spot->s.angles, angles);

  return spot;
}

/*---------------------------------------------------------------------------*/
/*
================
 SortClients
================
*/
static int QDECL SortClients(const void *a, const void *b)
{ return *(int *) a - *(int *) b; }

/*
 ==================
 TeamplayLocationsMessage
 Format:
 clientNum location health armor weapon powerups
 ==================
 */
static void TeamplayInfoMessage(gentity_t *ent)
{
  char entry[1024];
  char string[8192];
  size_t stringlength;
  int i, j;
  gentity_t *player;
  int cnt;
  int h, a;
  int totalarmor;
  int clients[TEAM_MAXOVERLAY];

  if (!ent->client->pers.teamInfo)
    return;

  // figure out what client should be on the display
  // we are limited to 8, but we want to use the top eight players
  // but in client order (so they don't keep changing position on the overlay)
  for (i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++)
  {
    player = g_entities + level.sortedClients[i];
    if (player->inuse && player->client->sess.sessionTeam == ent->client->sess.sessionTeam)
      clients[cnt++] = level.sortedClients[i];
  }

  // We have the top eight players, sort them by clientNum
  qsort(clients, cnt, sizeof(clients[0]), SortClients);

  // send the latest information on all clients
  string[0] = 0;
  stringlength = 0;

  for (i = 0, cnt = 0; i < g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++)
  {
    player = g_entities + i;

    if (player->inuse && player->client->sess.sessionTeam == ent->client->sess.sessionTeam)
    {
      h = player->client->ps.stats[STAT_HEALTH];
      a = 0;
      totalarmor  = player->client->ps.stats[STAT_ARMOR_LEGS];
      totalarmor += player->client->ps.stats[STAT_ARMOR_BODY];
      totalarmor += player->client->ps.stats[STAT_ARMOR_HEAD];
      a = totalarmor/3;

      if (h < 0)
        h = 0;
      if (a < 0)
        a = 0;

      Com_sprintf(entry, sizeof(entry), " %i %i %i %i %i %i",
          //				level.sortedClients[i], player->client->pers.teamState.location, h, a,
          (int)i, player->client->pers.teamState.location, h, a,
          player->client->ps.weapon, player->s.powerups);
      j = qstrlen(entry);
      if (stringlength + j > (int) sizeof(string))
        break;
      qstrcpy(string + stringlength, entry);
      stringlength += j;
      cnt++;
    }
  }

  trap_SendServerCommand(ent - g_entities, va("tinfo %i %s", cnt, string));
}

/*
================
 CheckTeamStatus
================
*/
void Team_CheckStatus(void)
{
  int i;
  gentity_t *loc, *ent;

  if (level.time - level.lastTeamLocationTime > TEAM_LOCATION_UPDATE_TIME)
  {
    level.lastTeamLocationTime = level.time;

    for (i = 0; i < g_maxclients.integer; i++)
    {
      ent = g_entities + i;

      if (ent->client->pers.connected != CON_CONNECTED)
        continue;

      if (ent->inuse && (ent->client->sess.sessionTeam == TEAM_DRAGONS || ent->client->sess.sessionTeam == TEAM_NIKKIS))
      {
        loc = Team_GetLocation(ent);
        if (loc)
          ent->client->pers.teamState.location = loc->health;
        else
          ent->client->pers.teamState.location = 0;
      }
    }

    for (i = 0; i < g_maxclients.integer; i++)
    {
      ent = g_entities + i;

      if (ent->client->pers.connected != CON_CONNECTED)
        continue;

      if (ent->inuse && (ent->client->sess.sessionTeam == TEAM_DRAGONS || ent->client->sess.sessionTeam == TEAM_NIKKIS))
        TeamplayInfoMessage(ent);
    }
  }
}

/*-----------------------------------------------------------------*/

/*QUAKED team_dragonsplayer (1 0 0) (-16 -16 -16) (16 16 32)
 Only in CTF games.  Red players spawn here at game start.
 */
void SP_team_Dragonsplayer(gentity_t *ent)
{}

/*QUAKED team_nikkisplayer (0 0 1) (-16 -16 -16) (16 16 32)
 Only in CTF games.  Blue players spawn here at game start.
 */
void SP_team_Nikkisplayer(gentity_t *ent)
{}

/*QUAKED team_dragonsspawn (1 0 0) (-16 -16 -24) (16 16 32)
 potential spawning position for red team in CTF games.
 Targets will be fired when someone spawns in on them.
 */
void SP_team_Dragonsspawn(gentity_t *ent)
{}

/*QUAKED team_nikkisspawn (0 0 1) (-16 -16 -24) (16 16 32)
 potential spawning position for blue team in CTF games.
 Targets will be fired when someone spawns in on them.
 */
void SP_team_Nikkisspawn(gentity_t *ent)
{}

#ifdef WITH_BAGMAN_MOD
//static gentity_t *BM_SpawnSafe(vec3_t origin, int team, int spawnflags);

#if 1

//waited 500ms
void BM_Safe_Think(gentity_t *ent)
{
  //ent->campcount = 0;
  ent->nextthink = 0;// level.time + 8.5 * 1000;
}

static void BM_Touch_Safe(gentity_t *self, gentity_t *other)
{
  int team = other->client->sess.sessionTeam;

  if (team == self->s.modelindex) //TEAM_DRAGONS 	TEAM_NIKKIS
    BM_TouchOurSafe(self, other);
  else
    BM_TouchEnemySafe(self, other);
}

//hypov8 add think for safecamp
void BM_Safe_Touch(gentity_t *safe, gentity_t *other, trace_t *trace)
{
  // can't retrigger until the wait is over
  if (safe->nextthink)
    return;

  //think every 500ms
  safe->think = BM_Safe_Think;
  safe->nextthink = level.time + 500;

  if (!other->client)
    return;

  BM_Touch_Safe(safe, other);

  //reset
  if (safe->enemy && safe->timestamp < level.time)
  {
    safe->bmSafe_campcount = 0;
    safe->enemy = NULL;
  }

  //same player??
  if (safe->enemy==NULL || safe->enemy != other)
  {
    safe->bmSafe_campcount = 0;
    safe->enemy = other;
    safe->timestamp = level.time + 10000;
  }
  else
  {
    safe->bmSafe_campcount += 1;

    if (safe->bmSafe_campcount <= 10 ) //5 seconds
      return;
    else if (safe->bmSafe_campcount == 12) //6 seconds
      PrintMsg(other, S_COLOR_RED"   **SAFE CAMPER**    YOU HAVE 3sec TO MOVE\n");
    else if (safe->bmSafe_campcount == 14) //7 seconds
      PrintMsg(other, S_COLOR_RED"   **SAFE CAMPER**    YOU HAVE 2sec TO MOVE\n");
    else if (safe->bmSafe_campcount == 16) //8 seconds
      PrintMsg(other, S_COLOR_RED"   **SAFE CAMPER**    YOU HAVE 1sec TO MOVE\n");
    else if (safe->bmSafe_campcount == 18) //9 seconds
    {
      PrintMsg(other, S_COLOR_RED"   **SAFE CAMPER**    YOU ARE DEAD \n");
      other->client->ps.stats[STAT_HEALTH] = other->health = -999;
      player_die(other, safe, NULL, 100000, MOD_SUICIDE);

      safe->bmSafe_campcount = 0;
      safe->enemy = NULL;
      return;
    }
  }
}

void BM_Safe_Setup(gentity_t *ent, int team)
{
  //ent->r.svFlags = SVF_NOCLIENT;
  ent->item				= BG_FindItem(ITEMNAME_CASHBAG);
  ent->s.pos.trTime		= level.time - 50;
  ent->s.modelindex		= team;
  ent->s.modelindex2		= 0;
  ent->clipmask		= SURF_NONSOLID;
  ent->r.contents		= CONTENTS_TRIGGER;
  //ent->s.pos.trType = TR_STATIONARY;
  ent->takedamage		= qfalse;
  ent->activator			= ent;
  ent->timestamp			= level.time + 5000;
  //ent->s.eType = ET_ITEM;// = ET_GENERAL; //ET_ITEM; //hypov8 item. dont works??
  ent->s.eType = ET_TEAM;
  ent->touch = BM_Safe_Touch;
  ent->wait = 0.5;
  //ent->s.modelindex = G_ModelIndex("models/qpin_gameobj/moneybag.md3");
  ent->think = BM_Safe_Think;
  ent->nextthink = level.time + 3000;

  //set bounding box for pickup
  VectorSet(ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS); // hypov8 adjusted model to suit
  VectorSet(ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);

  //G_SetOrigin(ent, ent->s.origin);

  trap_LinkEntity(ent);
}

/*
================
 SP_team_BM_Dragonssafe
================
*/
void SP_team_BM_Dragonssafe(gentity_t *ent)
{
  G_Printf(" SP_team_BM_Dragonssafe\n");
  if (g_gametype.integer != GT_BAGMAN)
  {
    G_FreeEntity(ent);
    return;
  }
  G_Printf("ent->classname %s\n", ent->classname);

  BM_Safe_Setup(ent, TEAM_DRAGONS);
}


/*
================
SP_team_BM_Nikkissafe
================
*/
void SP_team_BM_Nikkissafe(gentity_t *ent)
{
  G_Printf(" SP_team_BM_Nikkissafe\n");

  if (g_gametype.integer != GT_BAGMAN)
  {
    G_FreeEntity(ent);
    return;
  }
  G_Printf("ent->classname %s\n", ent->classname);

  BM_Safe_Setup(ent, TEAM_NIKKIS);
  ent->s.modelindex = TEAM_NIKKIS;
}
#endif

/*
 QUAKED SP_team_BM_cashspawn (0.5 0 1) (-16 -16 -16) (16 16 16)
 Spawn location for cash during "Grab da Loot" games
 angle - direction to project cash upon spawning
 speed - speed of projection
 type	- "cashroll" or "cashbag" (more money, longer delay)
 //hypov8
 cashspawnmultiply - 1 default, makes money spawn faster 2=twice as fast
*/
//#define cashroll "1"
//#define cashbag 2
void SP_team_BM_cashspawn(gentity_t *ent)
{
  char *type;
  float cashmultiplyer;

  G_Printf(" SP_team_BM_cashspawn\n");
  if (g_gametype.integer != GT_BAGMAN)
  {
    G_FreeEntity(ent);
    return;
  }

  G_SpawnFloat("angle",  "1", &ent->angle);
  if (!ent->angle)
    ent->angle = -1;

  //hypov8 needed??
  //if (ent->angle >= 180)
  //	ent->angle = ent->angle - 360;
  //hypov8 todo: angles?


  G_SpawnFloat("speed", "100", &ent->speed);
  if (!ent->speed)
    ent->speed = 100.0;

  //mapper option to change speed, default 1.0
  G_SpawnFloat("cashspawnmultiply", "1", &cashmultiplyer);
  if (!cashmultiplyer)
    cashmultiplyer = 1.0f;

  G_SpawnString("type", "cashroll", &type);
  if (strcmp(type, "cashbag"))
  {
    ent->wait = (g_cashspawndelay.value * 1000) / cashmultiplyer;
    ent->spawnflags = 1; //"cashroll"
  }
  else
  {    //spawn cashbags half as fast
    ent->wait = (g_cashspawndelay.value * 1000) * 2.0 / cashmultiplyer;
    ent->spawnflags = 2; //"cashbag"
  }

  ent->s.eType = ET_TEAM;
  ent->think = BM_CashSpawnThink;
  ent->nextthink = level.time + (int) ent->wait;
}

#if 0
/*
================
BM_Touch_Safe
================
*/
static void BM_Touch_Safe(gentity_t *self, gentity_t *other, trace_t *trace)
{
  int team;

  if (!other->client)
    return;

  team = other->client->sess.sessionTeam;

  if (team == self->spawnflags)
    BM_TouchOurSafe(self, other);
  else if (team == Team_OtherTeam(self->spawnflags))
    BM_TouchEnemySafe(self, other);
}


/*
================
BM_SpawnSafe
================
*/
static gentity_t *BM_SpawnSafe(vec3_t origin, int team, int spawnflags)
{
  gentity_t *ent;
  char *c;

  c = "team_bm_dragonssafe";
  switch (team)
  {
  case TEAM_DRAGONS:
    c = "team_bm_dragonssafe";
    break;
  case TEAM_NIKKIS:
    c = "team_bm_nikkissafe";
    break;
  }

  ent = G_Spawn();
  ent					= G_Find(ent, FOFS(classname), c);
  //ent->s.eType		= ET_TEAM;
  ent->s.eType		= ET_GENERAL; //ET_ITEM; //hypov8 item. dont works??
  ent->clipmask		= SURF_NONSOLID; // CONTENTS_SOLID;
  ent->r.contents		= CONTENTS_TRIGGER | SURF_NONSOLID;
  ent->s.pos.trType = TR_STATIONARY;
  ent->s.pos.trTime = 0;
  ent->s.pos.trDuration = 0;
  //ent->touch			= BM_Touch_Safe;

  VectorCopy(origin, ent->s.origin);
  VectorCopy(origin, ent->s.pos.trBase);
  VectorCopy(origin, ent->r.currentOrigin);
  VectorClear(ent->s.pos.trDelta);

  //safe bounding box
  VectorSet(ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
  VectorSet(ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);


   // suspended
  if (spawnflags & 1)
  {
    G_SetOrigin(ent, ent->s.origin);
  }
  else // drop to floor
  {
    vec3_t dest;
    trace_t tr;
    // mappers like to put them exactly on the floor, but being coplanar
    // will sometimes show up as starting in solid, so lif it up one pixel
    ent->s.origin[2] += 1;

    // drop to floor
    VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);

    trap_Trace(&tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID);

    if (tr.startsolid)
    {
      ent->s.origin[2] -= 1;
      G_Printf("SpawnSave: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
      ent->s.groundEntityNum = ENTITYNUM_NONE;
      G_SetOrigin(ent, ent->s.origin);
    }
    else
    {
      // allow to ride movers
      ent->s.groundEntityNum = tr.entityNum;
      G_SetOrigin(ent, tr.endpos);
    }
  }

  return ent;
}
#endif

void BM_CashSpawn_DropCash(gentity_t *ent)
{
  vec3_t angles3;
  vec3_t anglesOut[3];
  gentity_t *cash;

  //not needed anyway
  if (g_gametype.integer != GT_BAGMAN)
    return;

  //create a new item for spawned cash
  cash = G_Spawn();

  ent->bmSafe_numCashItems++;

  if (ent->spawnflags == 2)
    cash->classname = ITEMNAME_CASHBAG;
  else
    cash->classname = ITEMNAME_CASHROLL;


  //set up the new item to spawn
  cash->think				= BM_CashDie;
  cash->nextthink			= level.time + (60 * 1000);
  cash->s.eType			= ET_ITEM;
  cash->target_ent		= NULL;
  cash->s.pos.trType		= TR_GRAVITY;
  cash->s.pos.trTime		= level.time - 50;
  cash->r.contents		= CONTENTS_TRIGGER;
  cash->s.pos.trDuration	= 0;
  cash->item				= BG_FindItem(cash->classname);
  cash->s.modelindex		= cash->item - bg_itemlist;  // store item number in modelindex
  cash->s.modelindex2		= 1;
  cash->touch				= Touch_Item;
  cash->takedamage		= qfalse;
  cash->activator			= ent;
  cash->wait				= 0;
  cash->speed				= ent->speed;
  cash->physicsBounce		= 0.3 + crandom() * 0.3; //vary the bounce
  cash->s.angles[YAW]		= crandom() * 360; //rotate the cash a bit
  cash->timestamp			= level.time + 5000;
  cash->bm_SpawnerID		= ent; //set parent

  //set bounding box for pickup
  VectorSet(cash->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS); // hypov8 adjusted model to suit
  VectorSet(cash->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);

  //copy cash spawner to cash spawned
  VectorCopy(ent->s.origin, cash->s.origin);
  VectorCopy(ent->s.pos.trBase, cash->s.pos.trBase);
  VectorCopy(ent->r.currentOrigin, cash->r.currentOrigin);


  //check for angle 0-360 or up/down -1/-2
  if (ent->angle == -1)
    VectorSet(angles3, 0, 0, -180);
  else if (ent->angle == -2)
    VectorSet(angles3, 0, 0, 180);
  else
    VectorSet(angles3, 0, ent->angle, 0);

  //convert angle to 0-1 axis format
  AxisClear(anglesOut);
  AngleVectors(angles3, anglesOut[0], anglesOut[2], anglesOut[2]);

  //select axis xy or z
  if (ent->angle == -1 || ent->angle == -2)
    VectorCopy(anglesOut[2], cash->s.pos.trDelta);
  else
    VectorCopy(anglesOut[0], cash->s.pos.trDelta);

  //add speed to its movement direction
  VectorScale(cash->s.pos.trDelta, ent->speed, cash->s.pos.trDelta);

  // randomize the velocity a bit
  cash->s.pos.trDelta[0] = cash->s.pos.trDelta[0] + crandom() * 60;
  cash->s.pos.trDelta[1] = cash->s.pos.trDelta[1] + crandom() * 60;
  cash->s.pos.trDelta[2] = cash->s.pos.trDelta[2] + crandom() * 30;

  //hypov8 normalize?
  Vector4Copy(cash->s.pos.trDelta, cash->s.pos.trDelta);

  //add to total cash for world
  //num_cash_items++;

  //add the item
  trap_LinkEntity(cash);
}

/*
================
 BM_CashSpawnThink
================
*/
void BM_CashSpawnThink(gentity_t *ent)
{
  int maxItems = BAGMAN_MAX_CASH_ITEMS;

  if ( ent->spawnflags == 2 ) //ITEMNAME_CASHBAG
    maxItems = BAGMAN_MAX_CASH_ITEMS / 2;

  ent->nextthink = level.time + ent->wait;

  if ((ent->bmSafe_numCashItems >= maxItems) || (level.mode == LM_MATCHSETUP) || (level.mode == LM_FINALCOUNT))
    return;

  BM_CashSpawn_DropCash(ent);
}

/*
================
 Finish_Cashspawn
================
*/
#if 0 //disabled
void BM_FinishCashspawn(gentity_t* ent)
{

#if 1 //hypov8
  // reduce XY velocity (air friction)
  ent->movedir[0] *= 0.9f;
  ent->movedir[1] *= 0.9f;
#if 0
  if (level.time > (ent->timestamp))
  {
    BM_CashDie(ent);
    return;
  }

  //if (ent->movetype != MOVETYPE_NONE)
  //FIXME(0xA5EA):
  {
    if (Distance(ent->s.origin, ent->pos1) < 1)
      ent->count++;
    else
      ent->count = 0;

    VectorCopy(ent->s.origin, ent->pos1);

    if (self->count > 2) // rested for 2 frames
    {
      VectorClear(ent->movedir);
      VectorClear(ent->movedir);
      ent->s.angles[PITCH] = 0;
      ent->s.angles[ROLL] = 0;

      ent->movetype = MOVETYPE_NONE;
    }
  }
#endif
  ent->nextthink = level.time + FRAMETIME;
#else
  vec3_t velocity;

  //FIXME(0xA5EA): do i need this function ??
  Com_Printf("Finish_Cashspawn\n");
  // was ist ent, bm_cashspawn, or cash item ?
  // reduce XY velocity (air friction)

  velocity[0] = crandom() * 23;
  velocity[1] = crandom() * 342;
  velocity[2] = crandom();
  //ent->pos1
  //VectorCopy(origin, le->pos.trBase);
  //VectorCopy(velocity, le->pos.trDelta);
  //Com_Printf("ent->classname %s\n", ent->classname);
  ent->movedir[0] *= 0.9f;
  ent->movedir[1] *= 0.9f;
  //self->velocity[0] *= 0.9;
  //self->velocity[1] *= 0.9;
//  Com_Printf(" Finish_Cashspawn call \n");
  //FIXME (0xA5EA):
#if 0
  if (level.time > (ent->timestamp))
  {
    Com_Printf("lars2");
    CashDie(ent);
    return;
  }
#endif
#if 0
  //if (self->movetype != MOVETYPE_NONE)
  {
    if (Distance(ent->s.origin, ent->pos1) < 1)
      ent->count++;
    else
      ent->count = 0;

    VectorCopy(ent->s.origin, ent->pos1);

    if (ent->count > 2) // rested for 2 frames
    {
      VectorClear(ent->movedir);
      //VectorClear( self->avelocity );
      ent->s.angles[PITCH] = 0;
      ent->s.angles[ROLL] = 0;
      //self->movetype = MOVETYPE_NONE; // ????
    }
  }
#endif
  //cash->think = BM_CashSpawnThink
#endif

}
#endif

/*
================
BM_CashDie
================
*/
void BM_CashDie(gentity_t *ent)
{
  if (ent->bm_SpawnerID)
    ent->bm_SpawnerID->bmSafe_numCashItems--;
  G_FreeEntity(ent);
}

/*
================
BM_Pickup_Cash
================
*/
int BM_Pickup_Cash(gentity_t *self, gentity_t *other)
{
  int quantity = 0;
  int take_maxAmount; // hypov8


  if ((self->flags & FL_DROPPED_ITEM))
  {
    quantity = self->count;
  }
  else
  {
    quantity = self->item->quantity;
    if ( self->bm_SpawnerID )
      self->bm_SpawnerID->bmSafe_numCashItems--;
  }

  take_maxAmount = g_cashcollectmax.integer - other->client->ps.stats[STAT_CASH];
  if (take_maxAmount <= 0)
  {
    G_Printf(S_COLOR_RED"Error: invalid take amount %i\n", take_maxAmount);
    return 0;
  }
  take_maxAmount = Q_min(take_maxAmount, quantity);


  //set client cash values
  other->client->ps.stats[STAT_CASH] += take_maxAmount;
  other->client->ps.persistant[PERS_CASH_COLLECTED] += take_maxAmount;

  CalculateRanks();

  self->flags |= FL_DROPPED_ITEM; //free
  //G_FreeEntity(self); //remove item
  return 0;
}

/*
================
BM_StolenCashPickup
hypov8
when player dies with stolen cash
================
*/
int BM_Pickup_Cash_Stolen(gentity_t *self, gentity_t *other)
{
  int quantity = 0;
  int take_maxAmount;

  if (self->count >= 0)
    quantity = self->count;
  else
    quantity = self->item->quantity; //hypov8 testimg map item

  take_maxAmount = g_cashstolenmax.integer - other->client->ps.stats[STAT_STOLEN_CASH];
  if (take_maxAmount <= 0)
  {
    G_Printf(S_COLOR_RED"Error: invalid take amount %i\n", take_maxAmount);
    return 0;
  }
  take_maxAmount = Q_min(take_maxAmount, quantity);

  //set player values
  other->client->ps.powerups[PW_STOLENSCASH] = 1;// INT_MAX;
  other->client->ps.stats[STAT_STOLEN_CASH] += take_maxAmount;
  other->client->ps.persistant[PERS_CASH_STOLEN] += take_maxAmount;

  CalculateRanks();

  self->flags |= FL_DROPPED_ITEM;//free
  //G_FreeEntity(self); //remove item
  return 0;
}

/*
=================
BM_TossClientCash

Toss the cash. debug only
=================
*/
void BM_TossClientCash(gentity_t *self)
{
  gitem_t *pcash;
  gentity_t *drop;
  vec3_t velocity;
  vec3_t angles;

  //hypov8 will drop cash onto self!!!
  if (self->client->ps.stats[STAT_CASH])
  {
    pcash = BG_FindItem(ITEMNAME_CASHROLL);
    if (pcash)
    {
      VectorCopy(self->s.apos.trBase, angles);
      angles[PITCH] = 0;  // always forward

      AngleVectors(angles, velocity, NULL, NULL);
      VectorScale(velocity, 350, velocity);
      velocity[2] += 200 ;

      drop =  LaunchItem(pcash, self->s.pos.trBase, velocity);
      drop->nextthink = level.time + g_cashTimeout.integer * 1000;
      drop->think = G_FreeEntity;
      drop->spawnflags = self->client->sess.sessionTeam;
      drop->count = self->client->ps.stats[STAT_CASH];

      self->client->ps.stats[STAT_CASH] = 0;
      //self->flags |= FL_DROPPED_ITEM;
    }
  }
}
#endif // WITH_BAGMAN_MOD


#if 0
  gentity_t *SpawnObelisk(vec3_t origin, int team, int spawnflags)
  {
    trace_t tr;
    vec3_t dest;
    gentity_t *ent;

    ent = G_Spawn();

    VectorCopy(origin, ent->s.origin);
    VectorCopy(origin, ent->s.pos.trBase);
    VectorCopy(origin, ent->r.currentOrigin);

    VectorSet(ent->r.mins, -15, -15, 0);
    VectorSet(ent->r.maxs, 15, 15, 87);

    ent->s.eType = ET_GENERAL;
    ent->flags = FL_NO_KNOCKBACK;

    if (g_gametype.integer == GT_OBELISK)
    {
      ent->r.contents = CONTENTS_SOLID;
      ent->takedamage = qtrue;
      ent->health = g_obeliskHealth.integer;
      ent->die = ObeliskDie;
      ent->pain = ObeliskPain;
      ent->think = ObeliskRegen;
      ent->nextthink = level.time + g_obeliskRegenPeriod.integer * 1000;
    }
    if (g_gametype.integer == GT_HARVESTER)
    {
      ent->r.contents = CONTENTS_TRIGGER;
      ent->touch = ObeliskTouch;
    }

    if (spawnflags & 1)
    {
      // suspended
      G_SetOrigin(ent, ent->s.origin);
    }
    else
    {
      // mappers like to put them exactly on the floor, but being coplanar
      // will sometimes show up as starting in solid, so lif it up one pixel
      ent->s.origin[2] += 1;

      // drop to floor
      VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);
      trap_Trace(&tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID);
      if (tr.startsolid)
      {
        ent->s.origin[2] -= 1;
        G_Printf("SpawnObelisk: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));

        ent->s.groundEntityNum = ENTITYNUM_NONE;
        G_SetOrigin(ent, ent->s.origin);
      }
      else
      {
        // allow to ride movers
        ent->s.groundEntityNum = tr.entityNum;
        G_SetOrigin(ent, tr.endpos);
      }
    }

    ent->spawnflags = team;

    trap_LinkEntity(ent);

    return ent;
  }
  else
  {
    cash->s.modelindex = gi.modelindex("models/pu_icon/money/money_sm.md2");
    cash->gravity = 1.0;

    VectorSet(cash->mins, -12, -12, -15);
    VectorSet(cash->maxs, 12, 12, 10);

    cash->item = FindItem("Small Cash Bag");

    cash->currentcash = CASH_BAG;
    cash->touch = cash_touch;

    cash->think = cash_kill;
    cash->nextthink = level.time + 60;
  }
#endif

#ifdef GT_USE_TA_TYPES
/*
 ================
 Obelisks
 ================
 */

static void ObeliskRegen(gentity_t *self)
{
  self->nextthink = level.time + g_obeliskRegenPeriod.integer * 1000;
  if (self->health >= g_obeliskHealth.integer)
    return;

  G_AddEvent(self, EV_POWERUP_REGEN, 0);
  self->health += g_obeliskRegenAmount.integer;
  if (self->health > g_obeliskHealth.integer)
    self->health = g_obeliskHealth.integer;

  self->activator->s.modelindex2 = self->health * 0xff / g_obeliskHealth.integer;
  self->activator->s.frame = 0;
}

static void ObeliskRespawn(gentity_t *self)
{
  self->takedamage = qtrue;
  self->health = g_obeliskHealth.integer;

  self->think = ObeliskRegen;
  self->nextthink = level.time + g_obeliskRegenPeriod.integer * 1000;

  self->activator->s.frame = 0;
}

static void ObeliskDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
  int otherTeam;

  otherTeam = Team_OtherTeam(self->spawnflags);
  AddTeamScore(self->s.pos.trBase, otherTeam, 1);
  Team_ForceGesture(otherTeam);

  CalculateRanks();

  self->takedamage = qfalse;
  self->think = ObeliskRespawn;
  self->nextthink = level.time + g_obeliskRespawnDelay.integer * 1000;

  self->activator->s.modelindex2 = 0xff;
  self->activator->s.frame = 2;

  G_AddEvent(self->activator, EV_OBELISKEXPLODE, 0);

  AddScore(attacker, self->r.currentOrigin, CTF_CAPTURE_BONUS);

  // add the sprite over the player's head
  attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
  attacker->client->ps.eFlags |= EF_AWARD_CAP;
  attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
  attacker->client->ps.persistant[PERS_CAPTURES]++;

  teamgame.redObeliskAttackedTime = 0;
  teamgame.blueObeliskAttackedTime = 0;
}

static void ObeliskTouch(gentity_t *self, gentity_t *other, trace_t *trace)
{
  int tokens;

  if (!other->client)
    return;

  if (Team_OtherTeam(other->client->sess.sessionTeam) != self->spawnflags)
    return;

  tokens = other->client->ps.generic1;
  if (tokens <= 0)
    return;

  PrintMsg(NULL, "%s" S_COLOR_WHITE " brought in %i skull%s.\n",
      other->client->pers.netname, tokens, tokens ? "s" : "");

  AddTeamScore(self->s.pos.trBase, other->client->sess.sessionTeam, tokens);
  Team_ForceGesture(other->client->sess.sessionTeam);

  AddScore(other, self->r.currentOrigin, CTF_CAPTURE_BONUS * tokens);

  // add the sprite over the player's head
  other->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
  other->client->ps.eFlags |= EF_AWARD_CAP;
  other->client->rewardTime = level.time + REWARD_SPRITE_TIME;
  other->client->ps.persistant[PERS_CAPTURES] += tokens;

  other->client->ps.generic1 = 0;
  CalculateRanks();

  Team_CaptureFlagSound(self, self->spawnflags);
}

static void ObeliskPain(gentity_t *self, gentity_t *attacker, int damage)
{
  int actualDamage = damage / 10;

  if (actualDamage <= 0)
    actualDamage = 1;

  self->activator->s.modelindex2 = self->health * 0xff / g_obeliskHealth.integer;
  if (!self->activator->s.frame)
    G_AddEvent(self, EV_OBELISKPAIN, 0);

  self->activator->s.frame = 1;
  AddScore(attacker, self->r.currentOrigin, actualDamage);
}

gentity_t *SpawnObelisk(vec3_t origin, int team, int spawnflags)
{
  trace_t tr;
  vec3_t dest;
  gentity_t *ent;

  ent = G_Spawn();

  VectorCopy(origin, ent->s.origin);
  VectorCopy(origin, ent->s.pos.trBase);
  VectorCopy(origin, ent->r.currentOrigin);

  VectorSet(ent->r.mins, -15, -15, 0);
  VectorSet(ent->r.maxs, 15, 15, 87);

  ent->s.eType = ET_GENERAL;
  ent->flags = FL_NO_KNOCKBACK;

  if (g_gametype.integer == GT_OBELISK)
  {
    ent->r.contents = CONTENTS_SOLID;
    ent->takedamage = qtrue;
    ent->health = g_obeliskHealth.integer;
    ent->die = ObeliskDie;
    ent->pain = ObeliskPain;
    ent->think = ObeliskRegen;
    ent->nextthink = level.time + g_obeliskRegenPeriod.integer * 1000;
  }
  if (g_gametype.integer == GT_HARVESTER)
  {
    ent->r.contents = CONTENTS_TRIGGER;
    ent->touch = ObeliskTouch;
  }

  if (spawnflags & 1)
  {
    // suspended
    G_SetOrigin(ent, ent->s.origin);
  }
  else
  {
    // mappers like to put them exactly on the floor, but being coplanar
    // will sometimes show up as starting in solid, so lif it up one pixel
    ent->s.origin[2] += 1;

    // drop to floor
    VectorSet(dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096);
    trap_Trace(&tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID);
    if (tr.startsolid)
    {
      ent->s.origin[2] -= 1;
      G_Printf("SpawnObelisk: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));

      ent->s.groundEntityNum = ENTITYNUM_NONE;
      G_SetOrigin(ent, ent->s.origin);
    }
    else
    {
      // allow to ride movers
      ent->s.groundEntityNum = tr.entityNum;
      G_SetOrigin(ent, tr.endpos);
    }
  }

  ent->spawnflags = team;

  trap_LinkEntity(ent);

  return ent;
}

/*QUAKED team_redobelisk (1 0 0) (-16 -16 0) (16 16 8)
 */
void SP_team_redobelisk(gentity_t *ent)
{
  gentity_t *obelisk;

  if (g_gametype.integer <= GT_TEAM)
  {
    G_FreeEntity(ent);
    return;
  }
  ent->s.eType = ET_TEAM;
  if (g_gametype.integer == GT_OBELISK)
  {
    obelisk = SpawnObelisk(ent->s.origin, TEAM_DRAGONS, ent->spawnflags);
    obelisk->activator = ent;
    // initial obelisk health value
    ent->s.modelindex2 = 0xff;
    ent->s.frame = 0;
  }
  if (g_gametype.integer == GT_HARVESTER)
  {
    obelisk = SpawnObelisk(ent->s.origin, TEAM_DRAGONS, ent->spawnflags);
    obelisk->activator = ent;
  }
  ent->s.modelindex = TEAM_DRAGONS;
  trap_LinkEntity(ent);
}

/*QUAKED team_blueobelisk (0 0 1) (-16 -16 0) (16 16 88)
 */
void SP_team_blueobelisk(gentity_t *ent)
{
  gentity_t *obelisk;

  if (g_gametype.integer <= GT_TEAM)
  {
    G_FreeEntity(ent);
    return;
  }
  ent->s.eType = ET_TEAM;
  if (g_gametype.integer == GT_OBELISK)
  {
    obelisk = SpawnObelisk(ent->s.origin, TEAM_NIKKIS, ent->spawnflags);
    obelisk->activator = ent;
    // initial obelisk health value
    ent->s.modelindex2 = 0xff;
    ent->s.frame = 0;
  }
  if (g_gametype.integer == GT_HARVESTER)
  {
    obelisk = SpawnObelisk(ent->s.origin, TEAM_NIKKIS, ent->spawnflags);
    obelisk->activator = ent;
  }
  ent->s.modelindex = TEAM_NIKKIS;
  trap_LinkEntity(ent);
}

/*QUAKED team_neutralobelisk (0 0 1) (-16 -16 0) (16 16 88)
 */
void SP_team_neutralobelisk(gentity_t *ent)
{
  if (g_gametype.integer != GT_1FCTF && g_gametype.integer != GT_HARVESTER)
  {
    G_FreeEntity(ent);
    return;
  }
  ent->s.eType = ET_TEAM;
  if (g_gametype.integer == GT_HARVESTER)
  {
    neutralObelisk = SpawnObelisk(ent->s.origin, TEAM_FREE, ent->spawnflags);
    neutralObelisk->spawnflags = TEAM_FREE;
  }
  ent->s.modelindex = TEAM_FREE;
  trap_LinkEntity(ent);
}

/*
 ================
 CheckObeliskAttack
 ================
 */
qboolean CheckObeliskAttack(gentity_t *obelisk, gentity_t *attacker)
{
  gentity_t *te;

  // if this really is an obelisk
  if (obelisk->die != ObeliskDie)
    return qfalse;

  // if the attacker is a client
  if (!attacker->client)
    return qfalse;

  // if the obelisk is on the same team as the attacker then don't hurt it
  if (obelisk->spawnflags == attacker->client->sess.sessionTeam)
    return qtrue;

  // obelisk may be hurt

  // if not played any sounds recently
  if ((obelisk->spawnflags == TEAM_DRAGONS &&
          teamgame.redObeliskAttackedTime < level.time - OVERLOAD_ATTACK_BASE_SOUND_TIME) ||
      (obelisk->spawnflags == TEAM_NIKKIS &&
          teamgame.blueObeliskAttackedTime < level.time - OVERLOAD_ATTACK_BASE_SOUND_TIME))
  {

    // tell which obelisk is under attack
    te = G_TempEntity(obelisk->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);
    if (obelisk->spawnflags == TEAM_DRAGONS)
    {
      te->s.eventParm = GTS_DRAGONOBELISK_ATTACKED;
      teamgame.redObeliskAttackedTime = level.time;
    }
    else
    {
      te->s.eventParm = GTS_NIKKIOBELISK_ATTACKED;
      teamgame.blueObeliskAttackedTime = level.time;
    }
    te->r.svFlags |= SVF_BROADCAST;
  }

  return qfalse;
}
#endif
