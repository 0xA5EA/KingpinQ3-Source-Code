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

#ifndef G_TEAM_H_
#define G_TEAM_H_

#define CTF_CAPTURE_BONUS                     100   // what you get for capture
#define CTF_TEAM_BONUS                         25   // what your team gets for capture
#define CTF_RECOVERY_BONUS                     10   // what you get for recovery
#define CTF_FLAG_BONUS                         10   // what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS                 20   // what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME                40000   // seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS        5   // bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS               2   // bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS                 10   // bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS           10   // awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS          10   // award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS            1000   // the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS          1000   // the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT   8000
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT     10000
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT      10000

#define CTF_GRAPPLE_SPEED                     750   // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED                750   // speed player is pulled at

#define OVERLOAD_ATTACK_BASE_SOUND_TIME     20000

// Bagman
#define BAGMAN_STEAL_BONUS                     10
#define BAGMAN_DEPOSIT_BONUS                   10
#define BAGMAN_STEAL_DEPOSIT_BONUS             20
#define BAGMAN_MAX_CASH_ITEMS                  8   // never spawn more than this many cash items from 1 spawner
//FIXME(0xA5EA): bagman make this a cvar ??

// Prototypes
void Team_DroppedFlagThink(gentity_t *ent);
void Team_FragBonuses(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker);
void Team_CheckHurtCarrier(gentity_t *targ, gentity_t *attacker);
void Team_InitGame(void);
void Team_ReturnFlag(int team);
void Team_FreeEntity(gentity_t *ent);
gentity_t *SelectCTFSpawnPoint(team_t team, int teamstate, vec3_t origin, vec3_t angles);
gentity_t *Team_GetLocation(gentity_t *ent);
qboolean Team_GetLocationMsg(gentity_t *ent, char *loc, int loclen);
void Team_CheckStatus(void);
int Team_Pickup(gentity_t *ent, gentity_t *other);
int Team_OtherTeam(int team);
int Team_Touch_StaticItem(gentity_t *ent, gentity_t *other);

int BM_TouchSafe(gentity_t *ent, gentity_t *other); //hypov8 add
int BM_Pickup_Cash(gentity_t *ent, gentity_t *other);
int BM_Pickup_Cash_Stolen(gentity_t *ent, gentity_t *other); /*add hypov8*/
//void BM_CashSpawn_DropCash(gentity_t *ent/*, gentity_t *cash*/); /*add hypov8*/
void BM_CashSpawnThink(gentity_t *ent);
void BM_TouchCash(gentity_t *self, gentity_t *other, trace_t *trace);
void BM_FinishCashspawn(gentity_t *ent);
void BM_ReturnCash(gentity_t *ent);
qboolean BM_CanPickupCash(gclient_t *client);
qboolean BM_CanPickupCashStolen(gclient_t *client); /*add hypov8*/

#endif // G_TEAM_H_
