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
// g_combat.c

#include "g_local.h"

/*
============
ScorePlum
============
*/
void ScorePlum(gentity_t *ent, vec3_t origin, int score)
{
  gentity_t *plum;

  plum = G_TempEntity(origin, EV_SCOREPLUM);
  // only send this temp entity to a single client
  plum->r.svFlags     |= SVF_SINGLECLIENT;
  plum->r.singleClient = ent->s.number;
  //
  plum->s.otherEntityNum = ent->s.number;
  plum->s.time           = score;
}

/*
============
AddScore
Adds score to both the client and his team
============
*/
void AddScore(gentity_t *ent, vec3_t origin, int score)
{
  if (!ent->client)
    return;

  // no scoring during pre-match warmup
  if (level.warmupTime)
    return;

  // show score plum
  //ScorePlum(ent, origin, score); //hypov8 removed. shows a floating score of collected cash

  ent->client->ps.persistant[PERS_SCORE] += score;

  if (g_gametype.integer == GT_TEAM)
    level.teamScores[ent->client->ps.persistant[PERS_TEAM]] += score;

  CalculateRanks();
}


/*
=================
TossClientItems
Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems(gentity_t *self)
{
	gitem_t *item;
	int weapon;
	float angle;
	int i;
	gentity_t *drop;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.
	if (weapon == WP_CROWBAR || weapon == WP_GRAPPLING_HOOK || weapon == WP_PISTOL)
	{
		if (self->client->ps.weaponstate == WEAPON_DROPPING)
			weapon = self->client->pers.cmd.weapon;

		if (!(self->client->ps.stats[STAT_WEAPONS] & (1 << weapon)))
			weapon = WP_NONE;
	}

	// drop weapons if not in hitmen
	if (g_weaponmod.integer < WM_HITMEN )
	{
		if (weapon >= WP_FIRST && weapon <= WP_LAST && self->client->ps.ammo_mag[weapon])
		{
			int count = self->client->ps.ammo_all[BG_AmmoCombineCheck(self->client->ps.weapon)] +
						self->client->ps.ammo_mag[self->client->ps.weapon];

			// only drop full guns
			if (count >= BG_WeaponMaxMagCount(self->client->ps.weapon))
			{
				// find the item type for this weapon
				item = BG_FindItemForWeapon((weapon_t)weapon);
				
				// spawn the item 
				drop = Drop_Item(self, item, 0);
			}
		}
	}


#ifdef WITH_BAGMAN_MOD
	//drop cash items
	if (self->client->ps.stats[STAT_STOLEN_CASH])
	{
		int count = self->client->ps.stats[STAT_STOLEN_CASH];

		// find the item type for this weapon
		item = BG_FindItem(ITEMNAME_CASHBAG_STOLEN);
  
		// spawn the item //hypov8 with only gun ammo
		drop = Drop_Item(self, item, 0);
		drop->count = count; //set value

		self->client->ps.stats[STAT_STOLEN_CASH] = 0;
		self->client->ps.powerups[PW_STOLENSCASH] = 0; //remove powerup
	}
	if (self->client->ps.stats[STAT_CASH])
	{
		int count = self->client->ps.stats[STAT_CASH];

		// find the item type for this weapon
		item = BG_FindItem(ITEMNAME_CASHROLL);
  
		// spawn the item //hypov8 with only gun ammo
		drop = Drop_Item(self, item, 0);
		drop->count = count; //set value
		self->client->ps.stats[STAT_CASH] = 0;
	}
#endif

	// drop all the powerups if not in teamplay
	if (g_gametype.integer != GT_TEAM)
	{
		angle = 45;
		//FIXME(0xA5EA): bagman
		for (i = 1; i < PW_NUM_POWERUPS; i++)
		{
			int isDropable = 0;

			if (i >= PW_DEGRADE_START)
			{
				if (self->client->ps.powerups[i] > level.time)
					isDropable = 1;
			}
			else if (self->client->ps.powerups[i] > 0)
				isDropable = 1;

			if (isDropable)
			{
				item = BG_FindItemForPowerup((powerup_t)i);
				if (!item || item->giType == IT_WEAPON)
					continue;

				drop = Drop_Item(self, item, angle);
				// decide how many seconds it has left
				//drop->count = (self->client->ps.powerups[i] - level.time) / 1000;
				drop->count = G_PowerupCountdown(i ,self->client->ps.powerups[i]); //hypov8 add
				if (drop->count < 1)
					drop->count = 1;

				angle += 45;
			}
		}
	}
}


#ifdef GT_USE_TA_TYPES
/*
=================
TossClientCubes
=================
*/
extern gentity_t *neutralObelisk;

void TossClientCubes(gentity_t *self)
{
  gitem_t *item;
  gentity_t *drop;
  vec3_t velocity;
  vec3_t angles;
  vec3_t origin;

  self->client->ps.generic1 = 0;

  // this should never happen but we should never
  // get the server to crash due to skull being spawned in
  if (!G_EntitiesFree())
    return;

  if (self->client->sess.sessionTeam == TEAM_DRAGONS)
  {
    item = BG_FindItem("Red Cube");
  }
  else
  {
    item = BG_FindItem("Blue Cube");
  }

  angles[YAW]   = (float)(level.time % 360);
  angles[PITCH] = 0;  // always forward
  angles[ROLL]  = 0;

  AngleVectors(angles, velocity, NULL, NULL);
  VectorScale(velocity, 150, velocity);
  velocity[2] += 200 + crandom() * 50;

  if (neutralObelisk)
  {
    VectorCopy(neutralObelisk->s.pos.trBase, origin);
    origin[2] += 44;
  }
  else
  {
    VectorClear(origin);
  }

  drop = LaunchItem(item, origin, velocity);

  drop->nextthink  = level.time + g_cubeTimeout.integer * 1000;
  drop->think      = G_FreeEntity;
  drop->spawnflags = self->client->sess.sessionTeam;
}

/*
=================
TossClientPersistantPowerups
=================
*/
void TossClientPersistantPowerups(gentity_t *ent)
{
  gentity_t *powerup;

  if (!ent->client)
    return;

  if (!ent->client->persistantPowerup)
    return;

  powerup = ent->client->persistantPowerup;

  powerup->r.svFlags &= ~SVF_NOCLIENT;
  powerup->s.eFlags  &= ~EF_NODRAW;
  powerup->r.contents = CONTENTS_TRIGGER;
  trap_LinkEntity(powerup);

  ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
  ent->client->persistantPowerup                 = NULL;
}
#endif

/*
==================
LookAtKiller
==================
*/
void LookAtKiller(gentity_t *self, gentity_t *inflictor, gentity_t *attacker)
{
  vec3_t dir;
  //vec3_t angles;

  if (attacker && attacker != self)
    VectorSubtract(attacker->s.pos.trBase, self->s.pos.trBase, dir);
  else if (inflictor && inflictor != self)
    VectorSubtract(inflictor->s.pos.trBase, self->s.pos.trBase, dir);
  else
  {
    self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
    return;
  }

  self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw(dir);

  //angles[YAW]   = vectoyaw(dir);
  //angles[PITCH] = 0;
  //angles[ROLL]  = 0;
}

/*
==================
GibEntity
==================
*/
void GibEntity(gentity_t *self, int killer)
{
#ifdef USE_KAMIKAZE
  gentity_t *ent;
  int i;
  //if this entity still has kamikaze
  if (self->s.eFlags & EF_KAMIKAZE)
  {
    // check if there is a kamikaze timer around for this owner
    for (i = 0; i < MAX_GENTITIES; i++)
    {
      ent = &g_entities[i];
      if (!ent->inuse)
        continue;

      if (ent->activator != self)
        continue;

      if (qstrcmp(ent->classname, "kamikaze timer"))
        continue;

      G_FreeEntity(ent);
      break;
    }
  }
#endif
  G_AddEvent(self, EV_GIB_PLAYER, killer);
  self->takedamage = qfalse;
  self->s.eType    = ET_INVISIBLE;
  self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath)
{
	int can_gib;

	if ( self->health > GIB_HEALTH_BULLET )
		return;

	if (!g_blood.integer)
	{
		self->health = GIB_HEALTH_BULLET + 1;
		return;
	}

	//never gib from bullets
	switch ( meansOfDeath )
	{
	case MOD_FLAMEGUN:
	case MOD_GRENADE:
	case MOD_GRENADE_SPLASH:
	case MOD_ROCKET:
	case MOD_ROCKET_SPLASH:
		can_gib = qtrue;
		break;
	default:
		can_gib = qfalse;
	}
	
	if ( !can_gib || self->health > GIB_HEALTH_RL )
		self->health = GIB_HEALTH_BULLET + 1; //stops player disapearing
	else
		GibEntity(self, 0);
}


// these are just for logging, the client prints its own messages
static char *modNames[] =
{
    "MOD_UNKNOWN",
    "MOD_PISTOL",
    "MOD_SHOTGUN",
    "MOD_CROWBAR",        /* 0xA5EA todo */
    "MOD_MACHINEGUN",
    "MOD_GRENADE",
    "MOD_GRENADE_SPLASH",
    "MOD_ROCKET",
    "MOD_ROCKET_SPLASH",
    "MOD_HMG",
    "MOD_FLAMEGUN",
    "MOD_WATER",
    "MOD_SLIME",
    "MOD_LAVA",
    "MOD_CRUSH",
    "MOD_TELEFRAG",
    "MOD_FALLING",
    "MOD_SUICIDE",
    "MOD_TARGET_LASER",
    "MOD_TRIGGER_HURT",
    "MOD_GRAPPLE",
    "MOD_JUICED", // 0xA5A, do we still need this ?
#ifdef USE_KAMIKAZE
    "MOD_KAMIKAZE"
#endif
};


#ifdef USE_KAMIKAZE
/*
==================
Kamikaze_DeathActivate
==================
*/
void Kamikaze_DeathActivate(gentity_t *ent)
{
  G_StartKamikaze(ent);
  G_FreeEntity(ent);
}

/*
==================
Kamikaze_DeathTimer
==================
*/
void Kamikaze_DeathTimer(gentity_t *self)
{
  gentity_t *ent;

  ent            = G_Spawn();
  ent->classname = "kamikaze timer";
  VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
  ent->r.svFlags |= SVF_NOCLIENT;
  ent->think      = Kamikaze_DeathActivate;
  ent->nextthink  = level.time + 5 * 1000;

  ent->activator = self;
}

#endif

/*
==================
CheckAlmostCapture
==================
*/
static void CheckAlmostCapture(gentity_t *self, gentity_t *attacker)
{
  gentity_t *ent;
  vec3_t dir;
  char *classname;

  // if this player was carrying a flag
  if (self->client->ps.powerups[PW_DRAGONFLAG] ||
      self->client->ps.powerups[PW_NIKKIFLAG] ||
      self->client->ps.powerups[PW_NEUTRALFLAG])
  {
    // get the goal flag this player should have been going for
    if (g_gametype.integer == GT_CTF)
    {
      if (self->client->sess.sessionTeam == TEAM_NIKKIS)
        classname = "team_ctf_nikkisflag";
      else
        classname = "team_ctf_dragonsflag";
    }
    else
    {
      if (self->client->sess.sessionTeam == TEAM_NIKKIS)
        classname = "team_ctf_dragonsflag";
      else
        classname = "team_ctf_nikkisflag";
    }
    ent = NULL;
    do
    {
      ent = G_Find(ent, FOFS(classname), classname);
    }
    while (ent && (ent->flags & FL_DROPPED_ITEM));
    // if we found the destination flag and it's not picked up
    if (ent && !(ent->r.svFlags & SVF_NOCLIENT))
    {
      // if the player was *very* close
      VectorSubtract(self->client->ps.origin, ent->s.origin, dir);
      if (VectorLength(dir) < 200)
      {
        self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
        if (attacker->client)
          attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
      }
    }
  }
}
#ifdef WITH_BAGMAN_MOD
/*
==================
CheckAlmostStealDeposit
==================
*/
static void CheckAlmostStealDeposit(gentity_t *self, gentity_t *attacker)
{
  gentity_t *ent;
  vec3_t dir;
  char *classname;

  if (g_gametype.integer != GT_BAGMAN)
    return;

  // if this player was carrying cash
  if (self->client->ps.powerups[PW_STOLENSCASH])
  {
    // get the goal flag this player should have been going for
    if (self->client->sess.sessionTeam == TEAM_NIKKIS)
      classname = "team_bm_nikkissafe";
    else
      classname = "team_bm_dragonssafe";

    ent = NULL;
    do
    {
      ent = G_Find(ent, FOFS(classname), classname);
    }
    while (ent && (ent->flags & FL_DROPPED_ITEM));

    // if we found the destination flag and it's not picked up
    if (ent && !(ent->r.svFlags & SVF_NOCLIENT))
    {
      // if the player was *very* close
      VectorSubtract(self->client->ps.origin, ent->s.origin, dir);
      if (VectorLength(dir) < 200)
      {
        self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
        if (attacker->client)
          attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
      }
    }
  }
}
#endif // WITH_BAGMAN_MOD


#ifdef GT_USE_TA_TYPES
/*
==================
CheckAlmostScored
==================
*/
static void CheckAlmostScored(gentity_t *self, gentity_t *attacker)
{
  gentity_t *ent;
  vec3_t dir;
  char *classname;

  // if the player was carrying cubes
  if (self->client->ps.generic1)
  {
    //FIXME(0xA5EA): does this work  for bm and ctf ?
    if (self->client->sess.sessionTeam == TEAM_NIKKIS)
      classname = "team_redobelisk";
    else
      classname = "team_blueobelisk";

    ent = G_Find(NULL, FOFS(classname), classname);
    // if we found the destination obelisk
    if (ent)
    {
      // if the player was *very* close
      VectorSubtract(self->client->ps.origin, ent->s.origin, dir);
      if (VectorLength(dir) < 200)
      {
        self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
        if (attacker->client)
          attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
      }
    }
  }
}
#endif
/*
==================
player_die
==================
*/
void player_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath)
{
	gentity_t *ent;
	int anim;
	int contents;
	int killer;
	int i;
	char *killerName, *obit;
	qboolean can_gib = qfalse;

	if (self->client->ps.pm_type == PM_DEAD)
		return;

	if (level.intermissiontime)
		return;

	switch (meansOfDeath)
	{
		case MOD_FLAMEGUN:   // for flamegun means of death -KRYPTYK 
		case MOD_GRENADE:
		case MOD_GRENADE_SPLASH:
		case MOD_ROCKET:
		case MOD_ROCKET_SPLASH:
			can_gib = qtrue;
		break;
		default:
			can_gib = qfalse;
	}


//unlagged - backward reconciliation #2
	// make sure the body shows up in the client's current position
	G_UnTimeShiftClient( self );
//unlagged - backward reconciliation #2

	// check for an almost capture
	if (attacker)// add hypov8
	{
		if (self->s.number != attacker->s.number) //hypov8 check for self killed
			CheckAlmostCapture(self, attacker);
	}
#ifdef WITH_BAGMAN_MOD
	if (attacker)// add hypov8
	{
		if (self->s.number != attacker->s.number) //hypov8 check for self killed
			CheckAlmostStealDeposit(self, attacker);
	}
#endif

#ifdef GT_USE_TA_TYPES
  // check for a player that almost brought in cubes
  CheckAlmostScored(self, attacker);
#endif
	if (!self->client)
	{
		G_LogPrintf("self->client null");
	}

	if (self->client && self->client->hook)
	Weapon_HookFree(self->client->hook);

	if ((self->client->ps.eFlags & EF_TICKING) && self->activator)
	{
		self->client->ps.eFlags   &= ~EF_TICKING;
		self->activator->think     = G_FreeEntity;
		self->activator->nextthink = level.time;
	}

	self->client->ps.pm_type = PM_DEAD;

	//add hypov8 set default killer. if unknown killer, make it worldspawn
	killer = ENTITYNUM_WORLD;
	killerName = "<world>";


	if (attacker) // hypov8 after this, is clearning player 'self->' info
	{
		if (self->s.number != attacker->s.number) //hypov8 check for self killed
		{
			killer = attacker->s.number;
			if (attacker->client)
				killerName = attacker->client->pers.netname;
			else
				killerName = "<non-client>";
		}
	}

	if (killer < 0 || killer >= MAX_CLIENTS)
	{
		killer     = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if (meansOfDeath < 0 || meansOfDeath >= (int)(sizeof(modNames) / sizeof(modNames[0])))
		obit = "<bad obituary>";
	else
		obit = modNames[meansOfDeath];

	// note hypov8 print killer(null)
	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", killer, self->s.number, meansOfDeath, killerName, self->client->pers.netname, obit);



	if (attacker)// add hypov8
	{
	  if (self->s.number != attacker->s.number) //hypov8 check for self killed
		self->enemy = attacker;
	}

	self->client->ps.persistant[PERS_KILLED]++;

	if (attacker && attacker->client)
	{
		attacker->client->lastkilled_client = self->s.number;

		if (attacker == self || OnSameTeam(self, attacker))
		{
			AddScore(attacker, self->r.currentOrigin, -1);
		}
		else
		{
			attacker->client->ps.persistant[PERS_FRAG_COUNT]++;
			AddScore(attacker, self->r.currentOrigin, 1);

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			if (level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME)
			{
				// play excellent on player
				BG_PersAddToExcellentCount(1, &attacker->client->ps.persistant[PERS_EXCELLENT_ASSIST_COUNT]);

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);
				attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
			}
			attacker->client->lastKillTime = level.time;
		}
	}
	else
	{
		AddScore(self, self->r.currentOrigin, -1);
	}

	// Add team bonuses
	if (attacker)// add hypov8
	{
	  if (self->s.number != attacker->s.number)
		Team_FragBonuses(self, inflictor, attacker);
	}

  // if I committed suicide, the flag does not fall, it returns.
  //FIXME(0xA5EA): bagman
	if (meansOfDeath == MOD_SUICIDE)
	{
		if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)
		{
			if (self->client->ps.powerups[PW_NEUTRALFLAG])          // only happens in One Flag CTF
			{
				Team_ReturnFlag(TEAM_FREE);
				self->client->ps.powerups[PW_NEUTRALFLAG] = 0;
			}
			else if (self->client->ps.powerups[PW_DRAGONFLAG])      // only happens in standard CTF
			{
				Team_ReturnFlag(TEAM_DRAGONS);
				self->client->ps.powerups[PW_DRAGONFLAG] = 0;
			}
			else if (self->client->ps.powerups[PW_NIKKIFLAG])       // only happens in standard CTF
			{
				Team_ReturnFlag(TEAM_NIKKIS);
				self->client->ps.powerups[PW_NIKKIFLAG] = 0;
			}
		}
#ifdef WITH_BAGMAN_MOD
		else if (g_gametype.integer == GT_BAGMAN)
		{
			if (self->client->ps.powerups[PW_STOLENSCASH])
				BM_ReturnCash(self);
		}
#endif // WITH_BAGMAN_MOD
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents(self->r.currentOrigin, -1);
	if (!(contents & CONTENTS_NODROP))
	{
		TossClientItems(self); // hypov8 add ammo count to weapon pickups
	}
	else
	{
		if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)
		{
			if (self->client->ps.powerups[PW_NEUTRALFLAG])          // only happens in One Flag CTF
				Team_ReturnFlag(TEAM_FREE);
			else if (self->client->ps.powerups[PW_DRAGONFLAG])      // only happens in standard CTF
				Team_ReturnFlag(TEAM_DRAGONS);
			else if (self->client->ps.powerups[PW_NIKKIFLAG])       // only happens in standard CTF
				Team_ReturnFlag(TEAM_NIKKIS);
		}
#ifdef WITH_BAGMAN_MOD
		else if (g_gametype.integer == GT_BAGMAN)
		{
		  if (self->client->ps.powerups[PW_STOLENSCASH])
			BM_ReturnCash(self);
		}
#endif // WITH_BAGMAN_MOD
	}

  //FIXME(0xA5EA): do i need TossClientPersistantPowerups
#ifdef GT_USE_TA_TYPES
  TossClientPersistantPowerups(self);
  if (g_gametype.integer == GT_HARVESTER)
  {
    TossClientCubes(self);
  }
#endif
  Cmd_Score_f(self);                                          // show scores
  // send updated scores to any clients that are following this one,
  // or they would get stale scoreboards
  for (i = 0; i < level.maxclients; i++)
  {
    gclient_t *client;

    client = &level.clients[i];
    if (client->pers.connected != CON_CONNECTED)
      continue;

    if (client->sess.sessionTeam != TEAM_SPECTATOR)
      continue;

    if (client->sess.spectatorClient == self->s.number)
      Cmd_Score_f(g_entities + i);
  }

  self->takedamage = qtrue;   // can still be gibbed

  self->s.weapon   = WP_NONE;
  self->s.powerups = 0;
  self->r.contents = CONTENTS_CORPSE;

  self->s.angles[0] = 0;
  self->s.angles[2] = 0;

  if (attacker) // add hypov8
  LookAtKiller(self, inflictor, attacker);

  VectorCopy(self->s.angles, self->client->ps.viewangles);

  self->s.loopSound = 0;

  self->r.maxs[2] = -8;

  // don't allow respawn until the death anim is done
  // g_forcerespawn may force spawning at some later time
  self->client->respawnTime = level.time + 1700;  //hypov8 note: min clientRespawnTime for CG_player

  // remove powerups
  Com_Memset(self->client->ps.powerups, 0, sizeof(self->client->ps.powerups));

  // never gib in a nodrop
  if ((can_gib && self->health <= GIB_HEALTH_RL && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE)
  {
    GibEntity(self, killer); // gib death
  }
  else
  {
    // normal death
    static int aIdx;
    switch (aIdx)
    { //hypov8 note: is using more than 1 animations for death??
    case 0: 
			anim = BOTH_DEATH1;
			//Com_Printf("_BOTH_DEATH1\n");
			break;
			
    case 1:
			anim = BOTH_DEATH2; 
			//Com_Printf("_BOTH_DEATH2\n");
			break;
			
    case 2: 
	default:
			anim = BOTH_DEATH3;
			//Com_Printf("_BOTH_DEATH3\n");
			break;
    }

    // for the no-blood option, we need to prevent the health
    // from going to gib level
    if (self->health <= GIB_HEALTH_BULLET)
      self->health = GIB_HEALTH_BULLET + 1;

    self->client->ps.legsAnim =
      ((self->client->ps.legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;
    self->client->ps.torsoAnim =
      ((self->client->ps.torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT) | anim;

    G_AddEvent(self, EV_DEATH1 + i, killer);

    // the body can still be gibbed
    self->die = body_die;

    // globally cycle through the different death animations
    aIdx = (aIdx + 1) % 3;
#ifdef USE_KAMIKAZE
    if (self->s.eFlags & EF_KAMIKAZE)
      Kamikaze_DeathTimer(self);
#endif


	// broadcast the death event to everyone
	ent = G_TempEntity(self->r.currentOrigin, EV_OBITUARY);
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST; // send to everyone
  }
  trap_LinkEntity(self);
}

/*
================
CheckArmor
================
*/
static int CheckArmor(gentity_t *ent, int damage, int dflags)
{
  gclient_t *client;
  int save;
  int count;

  if (!damage)
    return 0;

  client = ent->client;

  if (!client)
    return 0;

  if (dflags & DAMAGE_NO_ARMOR)
    return 0;

  // armor
  count = (client->ps.stats[STAT_ARMOR_HEAD] + client->ps.stats[STAT_ARMOR_BODY] +  client->ps.stats[STAT_ARMOR_LEGS]);
  count /= 3;

  save = ceil(damage * ARMOR_PROTECTION);

  if (save >= count)
    save = count;

  if (!save)
    return 0;

  client->ps.stats[STAT_ARMOR_HEAD] -= save;
  if ( client->ps.stats[STAT_ARMOR_HEAD] < 0 )
    client->ps.stats[STAT_ARMOR_HEAD] = 0;

  client->ps.stats[STAT_ARMOR_BODY] -= save;
  if ( client->ps.stats[STAT_ARMOR_BODY] < 0 )
    client->ps.stats[STAT_ARMOR_BODY] = 0;

  client->ps.stats[STAT_ARMOR_LEGS] -= save;
  if ( client->ps.stats[STAT_ARMOR_LEGS] < 0 )
    client->ps.stats[STAT_ARMOR_LEGS] = 0;

  return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections(vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2])
{
  float b, c, d, t;

  // | origin - (point + t * dir) | = radius
  // a = dir[0]^2 + dir[1]^2 + dir[2]^2;
  // b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
  // c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

  // normalize dir so a = 1
  VectorNormalize(dir);
  b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
  c = (point[0] - origin[0]) * (point[0] - origin[0]) +
      (point[1] - origin[1]) * (point[1] - origin[1]) +
      (point[2] - origin[2]) * (point[2] - origin[2]) -
      radius * radius;

  d = b * b - 4 * c;
  if (d > 0)
  {
    t = (-b + sqrt(d)) / 2;
    VectorMA(point, t, dir, intersections[0]);
    t = (-b - sqrt(d)) / 2;
    VectorMA(point, t, dir, intersections[1]);
    return 2;
  }
  else if (d == 0)
  {
    t = (-b) / 2;
    VectorMA(point, t, dir, intersections[0]);
    return 1;
  }
  return 0;
}

#ifdef GT_USE_TA_TYPES
/*
================
G_InvulnerabilityEffect
================
*/
int G_InvulnerabilityEffect(gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir)
{
  gentity_t *impact;
  vec3_t intersections[2], vec;
  int n;

  if (!targ->client)
    return qfalse;

  VectorCopy(dir, vec);
  VectorInverse(vec);
  // sphere model radius = 42 units
  n = RaySphereIntersections(targ->client->ps.origin, 42, point, vec, intersections);
  if (n > 0)
  {
    impact = G_TempEntity(targ->client->ps.origin, EV_INVUL_IMPACT);
    VectorSubtract(intersections[0], targ->client->ps.origin, vec);
    VectorToAngles(vec, impact->s.angles);
    impact->s.angles[0] += 90;
    if (impact->s.angles[0] > 360)
      impact->s.angles[0] -= 360;
    if (impactpoint)
    {
      VectorCopy(intersections[0], impactpoint);
    }
    if (bouncedir)
    {
      VectorCopy(vec, bouncedir);
      VectorNormalize(bouncedir);
    }
    return qtrue;
  }
  else
  {
    return qfalse;
  }
}
#endif


/*
============
G_LocationDamage
0xA5EA taken from Codearena Tutorial#14: "Locational Damage" by Calrathan
============
*/
int G_LocationDamage(vec3_t point, gentity_t *targ, gentity_t *attacker, int take)
{
  gentity_t *ent;
  vec3_t bulletPath;
  vec3_t bulletAngle;

  float clientHeight;
  float clientFeetZ;
  float clientRotation;
  float bulletHeight;
  float bulletRotation;   // Degrees rotation around client.
  // used to check Back of head vs. Face
  int impactRotation;
  int curHurtLocation;

  // First things first.  If we're not damaging them, why are we here?
  if (!take)
    return 0;

  // Point[2] is the REAL world Z. We want Z relative to the clients feet

  // Where the feet are at [real Z]
  clientFeetZ = targ->r.currentOrigin[2] + targ->r.mins[2];
  // How tall the client is [Relative Z]
  clientHeight = targ->r.maxs[2] - targ->r.mins[2];
  // Where the bullet struck [Relative Z]
  bulletHeight = point[2] - clientFeetZ;

  // Get a vector aiming from the client to the bullet hit
  VectorSubtract(targ->r.currentOrigin, point, bulletPath);
  // Convert it into PITCH, ROLL, YAW
  VectorToAngles(bulletPath, bulletAngle);

  clientRotation = targ->client->ps.viewangles[YAW];
  bulletRotation = bulletAngle[YAW];

  impactRotation = abs(clientRotation - bulletRotation);

  impactRotation += 45;                   // just to make it easier to work with
  impactRotation  = impactRotation % 360; // Keep it in the 0-359 range

  if (impactRotation < 90)
    curHurtLocation = LOCATION_BACK;
  else if (impactRotation < 180)
    curHurtLocation = LOCATION_RIGHT;
  else if (impactRotation < 270)
    curHurtLocation = LOCATION_FRONT;
  else if (impactRotation < 360)
    curHurtLocation = LOCATION_LEFT;
  else
    curHurtLocation = LOCATION_NONE;

  if (g_debugDamage.integer)
  {
    G_Printf("bulletHeight %f: clientHeight:%f clientRotation:%f impactRotation:%f \n",
             bulletHeight, clientHeight, clientRotation, impactRotation);
  }
  // The upper body never changes height, just distance from the feet
  if (bulletHeight > clientHeight - 5)                                                                                                                                                                            //
    curHurtLocation |= LOCATION_HEAD;
  // else if (bulletHeight > clientHeight - 8)
//   curHurtLocation |= LOCATION_FACE;
  else if (bulletHeight > clientHeight - 10)
    curHurtLocation |= LOCATION_SHOULDER;
  else if (bulletHeight > clientHeight - 19)
    curHurtLocation |= LOCATION_CHEST;
  else if (bulletHeight > clientHeight - 26)
    curHurtLocation |= LOCATION_STOMACH;
  else if (bulletHeight > clientHeight - 29)
    curHurtLocation |= LOCATION_GROIN;
  else if (bulletHeight < 9)                                                                                                                                                                                      // 4
    curHurtLocation |= LOCATION_FOOT;
  else
    // The leg is the only thing that changes size when you duck,
    // so we check for every other parts RELATIVE location, and
    // whats left over must be the leg.
    curHurtLocation |= LOCATION_LEG;

  // Check the location ignoring the rotation info
  switch (curHurtLocation & ~(LOCATION_BACK | LOCATION_LEFT | LOCATION_RIGHT | LOCATION_FRONT))
  {
  case LOCATION_HEAD:
    take *= 1.5;
    Com_Printf("%s has been hit in the head by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    //G_LogPrintf("%s has been hit in the head by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    break;

  case LOCATION_FACE:
    //G_LogPrintf("%s has been hit in the face by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
      Com_Printf("%s has been hit in the face by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    //if (targ->client->lasthurt_location & LOCATION_FRONT)
    // take *= 5.0; // Faceshots REALLY suck
//else
    take *= 1.5;
    break;
  case LOCATION_SHOULDER:
    //G_LogPrintf("%s has been hit in the shoulder by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    Com_Printf("%s has been hit in the shoulder by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    if (targ->client->lasthurt_location & (LOCATION_FRONT | LOCATION_BACK))
      take *= 1.3;                                                                                                                                                                                        // Throat or nape of neck
    else
      take *= 0.9;                                                                                                                                                                                        // Shoulders
    break;
  case LOCATION_CHEST:
    //G_LogPrintf("%s has been hit in the chest by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    Com_Printf("%s has been hit in the chest by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    if (targ->client->lasthurt_location & (LOCATION_FRONT | LOCATION_BACK))
      take *= 1.3;                                                                                                                                                                                        // Belly or back
    else
      take *= 0.8;                                                                                                                                                                                        // Arms
    break;
  case LOCATION_STOMACH:
    //G_LogPrintf("%s has been hit in the stomach by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    Com_Printf("%s has been hit in the stomach by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    take *= 1.1;
    break;
  case LOCATION_GROIN:
    //G_LogPrintf("%s has been hit in the groin by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    Com_Printf("%s has been hit in the stomach by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    if (targ->client->lasthurt_location & LOCATION_FRONT)
      take *= 0.8;                                                                                                                                                                                        // Groin shot
    break;
  case LOCATION_LEG:
    //G_LogPrintf("%s has been hit in the leg by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    Com_Printf("%s has been hit in the leg by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    take *= 0.6;
    break;
  case LOCATION_FOOT:
    //G_LogPrintf("%s has been hit in the foot by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    //Com_Printf("%s has been hit in the leg by  %s\n", targ->client->pers.netname, attacker->client->pers.netname);
    take *= 0.3;
    break;
  }

// if( targ->client->lasthurt_location != curHurtLocation )
// {
  // broadcast the death event to everyone
  ent                    = G_TempEntity(targ->r.currentOrigin, EV_PLAYER_HIT_INFO);
  ent->s.eventParm       = targ->client->lasthurt_location;
  ent->s.otherEntityNum  = targ->s.number;
  ent->s.otherEntityNum2 = attacker->s.number;                                                                                                                                                                    //killer;
  ent->r.svFlags         = SVF_BROADCAST;                                                                                                                                                                         // send to everyone
// }
  targ->client->lasthurt_location = curHurtLocation;
  return take;

}

/*
============
T_Damage

targ  entity that is being damaged
inflictor entity that is causing the damage
attacker entity that caused the inflictor to damage targ
    example: targ=monster, inflictor=rocket, attacker=player

dir   direction of the attack for knockback
point  point at which the damage is being inflicted, used for headshots
damage  amount of damage being inflicted
knockback force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags  these flags are used to control how T_Damage works
    DAMAGE_RADIUS   damage was indirect (from a nearby explosion)
    DAMAGE_NO_ARMOR   armor does not protect from this damage
    DAMAGE_NO_KNOCKBACK  do not affect velocity, just view angles
    DAMAGE_NO_PROTECTION kills godmode, armor, everything
============
*/

void G_Damage(gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod)
{
  gclient_t *client;
  int take;
  int asave;
  int knockback;
  int max;

#if 0 //def MISSIONPACK
  vec3_t bouncedir, impactpoint;
#endif

  if (!targ->takedamage)
    return;

  // the intermission has allready been qualified for, so don't
  // allow any extra scoring
  if (level.intermissionQueued)
    return;

  if (!inflictor)
    inflictor = &g_entities[ENTITYNUM_WORLD];

  if (!attacker)
    attacker = &g_entities[ENTITYNUM_WORLD];

  // shootable doors / buttons don't actually have any health
  if (targ->s.eType == ET_MOVER)
  {
    if (targ->use && targ->moverState == MOVER_POS1)
      targ->use(targ, inflictor, attacker);
    return;
  }

#ifdef GT_USE_TA_TYPES
  if ( g_gametype.integer == GT_OBELISK && CheckObeliskAttack( targ, attacker ) ) {
    return;
  }
#endif

  // reduce damage by the attacker's handicap value
  // unless they are rocket jumping
  if (attacker->client && attacker != targ)
  {
    max = attacker->client->ps.stats[STAT_MAX_HEALTH];
    damage = damage * max / 100;
  }

  client = targ->client;

  if (client && client->noclip)
    return;

  if (!dir)
    dflags |= DAMAGE_NO_KNOCKBACK;
  else
    VectorNormalize(dir);

  if ( attacker == targ )
    knockback = damage; // 0xA5EA: give more knockback for rocketjumps
  else
    knockback = 0.5*damage;

  if (knockback > 200)
    knockback = 200;

  if (targ->flags & FL_NO_KNOCKBACK)
    knockback = 0;

  if (dflags & DAMAGE_NO_KNOCKBACK)
    knockback = 0;

  // figure momentum add, even if the damage won't be taken
  if (knockback && targ->client)
  {
    vec3_t kvel;
    float mass;

    mass = 200;

    VectorScale(dir, g_knockback.value * (float)knockback / mass, kvel);
    VectorAdd(targ->client->ps.velocity, kvel, targ->client->ps.velocity);

    // set the timer so that the other client can't cancel
    // out the movement immediately
    if (!targ->client->ps.pm_time)
    {
      int t = knockback * 2;

      if (t < 50)
        t = 50;

      if (t > 200)
        t = 200;

      targ->client->ps.pm_time   = t;
      targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    }
  }

  // check for completely getting out of the damage
  if (!(dflags & DAMAGE_NO_PROTECTION))
  {
    // if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
    // if the attacker was on the same team
    if (mod != MOD_JUICED && targ != attacker && !(dflags & DAMAGE_NO_TEAM_PROTECTION) && OnSameTeam(targ, attacker))
    {
      if (!g_friendlyFire.integer)
        return;
    }
#ifdef GT_USE_TA_TYPES
    if (mod == MOD_PROXIMITY_MINE)
    {
      if (inflictor && inflictor->parent && OnSameTeam(targ, inflictor->parent))
        return;

      if (targ == attacker)
        return;
    }
#endif

    // check for godmode
    if (targ->flags & FL_GODMODE)
      return;
  }

  // add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
  if ( attacker->client && client && targ != attacker && targ->health > 0 &&
	  targ->s.eType != ET_MISSILE && targ->s.eType != ET_GENERAL && targ->s.eType != ET_FLAMETHROWER_CHUNK)
  {
    if (OnSameTeam(targ, attacker))
      attacker->client->ps.persistant[PERS_HITS]--;
    else
      attacker->client->ps.persistant[PERS_HITS]++;

    BG_PersAssignAttackeeHealth(targ->health , &attacker->client->ps.persistant[PERS_ATTACKER_INFO]);
  }

  // always give half damage if hurting self
  // calculated after knockback, so rocket jumping works
  if (targ == attacker)
    damage *= 0.5;

  if (damage < 1)
    damage = 1;

  if (g_weaponmod.integer & WM_REALMODE)
    damage *= 4;

  take = damage;

  // save some from armor
  asave = CheckArmor(targ, take, dflags); /* 0xA5EA, hier rein mit armor */
  take -= asave;

  //hypov8 add: flamer takes 2hp. if self=1, after armour asave then take hp=0 
  if (take < 1)
    take = 1;

  if (g_debugDamage.integer)
    G_Printf("%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number, targ->health, take, asave);

  // add to the damage inflicted on a player this frame
  // the total will be turned into screen blends and view angle kicks
  // at the end of the frame
  if (client)
  {
    if (attacker)
      BG_PersAssignAttacker(attacker->s.number, &client->ps.persistant[PERS_ATTACKER_INFO]);
    else
      BG_PersAssignAttacker(ENTITYNUM_WORLD, &client->ps.persistant[PERS_ATTACKER_INFO]);

    client->damage_armor += asave; //hypov8 todo: check 3 parts?
    client->damage_blood += take;
    client->damage_knockback += knockback;
    if (dir)
    {
      VectorCopy(dir, client->damage_from);
      client->damage_fromWorld = qfalse;
    }
    else
    {
      VectorCopy(targ->r.currentOrigin, client->damage_from);
      client->damage_fromWorld = qtrue;
    }

    if (mod == MOD_FLAMEGUN)
	  client->damage_flame = 1; //hypov8 todo: timming
  }


  // See if it's the player hurting the emeny flag carrier
  if (g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF)
    Team_CheckHurtCarrier(targ, attacker);

  //FIXME(0xA5EA): bagman

  if (targ->client)
  {
    // set the last client who damaged the target
    targ->client->lasthurt_client = attacker->s.number;
    targ->client->lasthurt_mod = mod;

    // Modify the damage for location damage  /* 0xA5EA */
    //FIXME(0xA5EA): this doesnt work at the moment
#if 0
    if (point && targ && targ->health > 0 && attacker && take)
      take = G_LocationDamage(point, targ, attacker, take);
    else
#endif
      targ->client->lasthurt_location = LOCATION_NONE;
  }

  // do the damage
  if (take)
  {
    targ->health = targ->health - take;

    if (targ->client)
      targ->client->ps.stats[STAT_HEALTH] = targ->health;

    if (targ->health <= 0)
    {
      if (client)
        targ->flags |= FL_NO_KNOCKBACK;

      if (targ->health < -999)
        targ->health = -999;

      targ->enemy = attacker;
      targ->die(targ, inflictor, attacker, take, mod);
      return;
    }
    else if (targ->pain)
      targ->pain(targ, attacker, take);
  }
}

/*
============
CanDamage
Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
static qboolean CanDamage(gentity_t *targ, vec3_t origin)
{
  vec3_t dest;
  trace_t tr;
  vec3_t midpoint;

  // use the midpoint of the bounds instead of the origin, because
  // bmodels may have their origin is 0,0,0
  VectorAdd(targ->r.absmin, targ->r.absmax, midpoint);
  VectorScale(midpoint, 0.5, midpoint);

  VectorCopy(midpoint, dest);
  trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
  if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
    return qtrue;

  // this should probably check in the plane of projection,
  // rather than in world coordinate, and also include Z
  VectorCopy(midpoint, dest);
  dest[0] += 15.0;
  dest[1] += 15.0;
  trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
  if (tr.fraction == 1.0)
    return qtrue;

  VectorCopy(midpoint, dest);
  dest[0] += 15.0;
  dest[1] -= 15.0;
  trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
  if (tr.fraction == 1.0)
    return qtrue;

  VectorCopy(midpoint, dest);
  dest[0] -= 15.0;
  dest[1] += 15.0;
  trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
  if (tr.fraction == 1.0)
    return qtrue;

  VectorCopy(midpoint, dest);
  dest[0] -= 15.0;
  dest[1] -= 15.0;
  trap_Trace(&tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
  if (tr.fraction == 1.0)
    return qtrue;

  return qfalse;
}

/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage(vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod)
{
  float points, dist;
  gentity_t *ent;
  int entityList[MAX_GENTITIES];
  int numListedEntities;
  vec3_t mins, maxs;
  vec3_t v;
  vec3_t dir;
  int i, e;
  qboolean hitClient = qfalse;

 //Com_Printf("radiusdamge call");
  if (radius < 1)
    radius = 1;

  for (i = 0; i < 3; i++)
  {
    mins[i] = origin[i] - radius;
    maxs[i] = origin[i] + radius;
  }

  numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

  for (e = 0; e < numListedEntities; e++)
  {
    ent = &g_entities[entityList[e]];

    if (ent == ignore)
      continue;

    if (!ent->takedamage)
      continue;

    // find the distance from the edge of the bounding box
    for (i = 0; i < 3; i++)
    {
      if (origin[i] < ent->r.absmin[i])
        v[i] = ent->r.absmin[i] - origin[i];
      else if (origin[i] > ent->r.absmax[i])
        v[i] = origin[i] - ent->r.absmax[i];
      else
        v[i] = 0;
    }

    dist = VectorLength(v);

    if (dist >= radius)
      continue;

    points = damage * (1.0 - dist / radius);

    if (CanDamage(ent, origin))
    {
      if (LogAccuracyHit(ent, attacker))
        hitClient = qtrue;

      VectorSubtract(ent->r.currentOrigin, origin, dir);
      // push the center of mass higher than the origin so players
      // get knocked into the air more
      dir[2] += 24;
      G_Damage(ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
    }
  }
  return hitClient;
}
