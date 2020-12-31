/*
 ===========================================================================
 Copyright (C) 1999-2005 Id Software, Inc.
 Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
 Copyright (C) 2011-2012  Ari 'KRYPTYK' Mirles

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

#define MIN_BURN_INTERVAL 399 // JPW NERVE set burn timeinterval so we can do more precise damage (was 199 old model)  Added -KRYPTYK

/*
 ===============
 G_DamageFeedback
 Called just before a snapshot is sent to the given player.
 Totals up all damage and generates both the player_state_t
 damage values to that client for pain blends and kicks, and
 global pain sound events for all clients.
 ===============
 */
void P_DamageFeedback(gentity_t *player)
{
  gclient_t *client;
  float count;
  vec3_t angles;

  client = player->client;
  if (client->ps.pm_type == PM_DEAD)
    return;

  // total points of damage shot at the player this frame
  count = client->damage_blood + client->damage_armor;

  if (count == 0)
    return; // didn't take any damage

  if (count > 255)
    count = 255;

  // send the information to the client

  // world damage (falling, slime, etc) uses a special code
  // to make the blend blob centered instead of positional
  if (client->damage_fromWorld)
  {
    client->ps.damagePitch = 255;
    client->ps.damageYaw = 255;

    client->damage_fromWorld = qfalse;
  }
  else
  {
    VectorToAngles(client->damage_from, angles);
    client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
    client->ps.damageYaw = angles[YAW] / 360.0 * 256;
  }

  // play an apropriate pain sound
  if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE))
  {
    player->pain_debounce_time = level.time + 700;
    G_AddEvent(player, EV_PAIN, player->health);
    client->ps.damageEvent++;
	if ( client->ps.damageEvent > 255 )
		client->ps.damageEvent = 1;

	// flamescreen
	if (client->damage_flame)
	{
	  client->ps.viewBurnCount++;
	  if ( client->ps.viewBurnCount > 255 )
		client->ps.viewBurnCount = 1;
	}
  }



  client->ps.damageCount = count;

  // clear totals
  client->damage_blood = 0;
  client->damage_armor = 0;
  client->damage_knockback = 0;
  client->damage_flame = 0;
}

/*
 =============
 P_WorldEffects
 Check for lava / slime contents and drowning
 =============
 */
void P_WorldEffects(gentity_t *ent)
{
#ifdef GT_USE_TA_TYPES
  qboolean envirosuit;
#endif
  int waterlevel;

  if (ent->client->noclip)
  {
    ent->client->airOutTime = level.time + 12000; // don't need air
    return;
  }

  waterlevel = ent->waterlevel;
#ifdef GT_USE_TA_TYPES
  envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;
#endif

  // check for drowning
  if (waterlevel == 3)
  {
#ifdef GT_USE_TA_TYPES
    // envirosuit give air
    if (envirosuit)
    {
      ent->client->airOutTime = level.time + 10000;
    }
#endif
    // if out of air, start drowning
    if (ent->client->airOutTime < level.time)
    {
      // drown!
      ent->client->airOutTime += 1000;
      if (ent->health > 0)
      {
        // take more damage the longer underwater
        ent->damage += 2;

        if (ent->damage > 15)
          ent->damage = 15;

        // play a gurp sound instead of a normal pain sound
        if (ent->health <= ent->damage)
          G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.ogg"));
        else if (rand() & 1)
          G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/actors/player/male/gurp1.ogg")); /* 0xA5EA */
        else
          G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/actors/player/male/gurp2.ogg")); /* 0xA5EA */

        // don't play a normal pain sound
        ent->pain_debounce_time = level.time + 200;

        G_Damage(ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
      }
    }
  }
  else
  {
    ent->client->airOutTime = level.time + 12000;
    ent->damage = 2;
  }

  // check for sizzle damage (move to pmove?)
  if (waterlevel && (ent->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)))
  {
    if (ent->health > 0 && ent->pain_debounce_time <= level.time)
    {
#ifdef GT_USE_TA_TYPES
      if (envirosuit)
      {
        G_AddEvent(ent, EV_POWERUP_BATTLESUIT, 0);
      }
      else
#endif
      {
        if (ent->watertype & CONTENTS_LAVA)
          G_Damage(ent, NULL, NULL, NULL, NULL, 30 * waterlevel, 0, MOD_LAVA);

        if (ent->watertype & CONTENTS_SLIME)
          G_Damage(ent, NULL, NULL, NULL, NULL, 10 * waterlevel, 0, MOD_SLIME);
      }
    }
  }

    //
	// check for burning from flamethrower -KRYPTYK
	//	
#if 1 //hypov8 todo: get this burning properly
	if (ent->s.onFireEnd && ent->client) 
	{
		if (level.time - ent->client->lastBurnTime >= MIN_BURN_INTERVAL) 
		{ 
			// JPW NERVE server-side incremental damage routine / player damage/health is int (not float)
			// so I can't allocate 1.5 points per server tick, and 1 is too weak and 2 is too strong.  
			// solution: allocate damage far less often (MIN_BURN_INTERVAL often) and do more damage.
			// That way minimum resolution (1 point) damage changes become less critical.

			ent->client->lastBurnTime = level.time;
			if ((ent->s.onFireEnd > level.time) && (ent->health > 0)) 
			{
				gentity_t *attacker;
   				attacker = g_entities + ent->flameBurnEnt;
				G_Damage (ent, attacker, attacker, NULL, NULL, 1, DAMAGE_NO_KNOCKBACK, MOD_FLAMEGUN); // JPW NERVE was 7
			}
		}
	}
#endif

	//antispawncamp
	if ( ent->antispawncamp && ent->antispawncamp < level.time )
	{
		ent->antispawncamp = 0;
		ent->flags &= ~FL_GODMODE;
	}

}

/*
 ===============
 G_SetClientSound
 ===============
 */
void G_SetClientSound(gentity_t *ent)
{
  if (ent->s.eFlags & EF_TICKING)
    ent->client->ps.loopSound = G_SoundIndex("sound/weapons/proxmine/wstbtick.ogg");
  else
  if (ent->waterlevel && (ent->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)))
    ent->client->ps.loopSound = level.snd_fry;
  else
    ent->client->ps.loopSound = 0;
}

/*
 ==============
 ClientImpacts
 ==============
 */
void ClientImpacts(gentity_t *ent, pmove_t *pm)
{
  int i, j;
  trace_t trace;
  gentity_t *other;

  Com_Memset(&trace, 0, sizeof(trace));
  for (i = 0; i < pm->numtouch; i++)
  {
    for (j = 0; j < i; j++)
    {
      if (pm->touchents[j] == pm->touchents[i])
        break;
    }
    if (j != i)
      continue; // duplicated

    other = &g_entities[pm->touchents[i]];

    if ((ent->r.svFlags & SVF_BOT) && (ent->touch))
      ent->touch(ent, other, &trace);

    if (!other->touch)
      continue;

    other->touch(other, ent, &trace);
  }
}

/*
 ============
 G_TouchTriggers
 Find all trigger entities that ent's current position touches.
 Spectators will only interact with teleporters.
 ============
 */
void G_TouchTriggers(gentity_t *ent)
{
  int i, num;
  int touch[MAX_GENTITIES];
  gentity_t *hit;
  trace_t trace;
  vec3_t mins, maxs;
  static const vec3_t range =  { 40, 40, 52 };

  if (!ent->client)
    return;

  // dead clients don't activate triggers!
  if (ent->client->ps.stats[STAT_HEALTH] <= 0)
    return;

  VectorSubtract(ent->client->ps.origin, range, mins);
  VectorAdd(ent->client->ps.origin, range, maxs);

  num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

  // can't use ent->absmin, because that has a one unit pad
  VectorAdd(ent->client->ps.origin, ent->r.mins, mins);
  VectorAdd(ent->client->ps.origin, ent->r.maxs, maxs);

  for (i = 0; i < num; i++)
  {
    hit = &g_entities[touch[i]];

    if (!hit->touch && !ent->touch)
      continue;

    if (!(hit->r.contents & CONTENTS_TRIGGER))
      continue;

    // ignore most entities if a spectator
    if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
    {
      // this is ugly but adding a new ET_? type will
      // most likely cause network incompatibilities
      if (hit->s.eType != ET_TELEPORT_TRIGGER && hit->touch != Touch_DoorTrigger)
        continue;
    }

    // use seperate code for determining if an item is picked up
    // so you don't have to actually contact its bounding box
    if (hit->s.eType == ET_ITEM)
    {
      if (!BG_PlayerTouchesItem(&ent->client->ps, &hit->s, level.time))
        continue;
    }
    else
    {
      if (!trap_EntityContact(mins, maxs, hit))
        continue;
    }

    Com_Memset(&trace, 0, sizeof(trace));

    if (hit->touch)
      hit->touch(hit, ent, &trace);

    if ((ent->r.svFlags & SVF_BOT) && (ent->touch))
      ent->touch(ent, hit, &trace);
  }

  // if we didn't touch a jump pad this pmove frame
  if (ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount)
  {
    ent->client->ps.jumppad_frame = 0;
    ent->client->ps.jumppad_ent = 0;
  }
}

/*
 =================
 SpectatorThink
 =================
 */
void SpectatorThink(gentity_t *ent, usercmd_t *ucmd)
{
  pmove_t pm;
  gclient_t *client;

  client = ent->client;

  if (client->sess.spectatorState != SPECTATOR_FOLLOW)
  {
    client->ps.pm_type = PM_SPECTATOR;
    client->ps.speed = 400; // faster than normal

    // set up for pmove
    Com_Memset(&pm, 0, sizeof(pm));
    pm.ps = &client->ps;
    pm.cmd = *ucmd;
    pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY; // spectators can fly through bodies
    pm.trace = trap_Trace;
    pm.pointcontents = trap_PointContents;

    // perform a pmove
    Pmove(&pm);
    // save results of pmove
    VectorCopy(client->ps.origin, ent->s.origin);

    G_TouchTriggers(ent);
    trap_UnlinkEntity(ent);
  }

  client->oldbuttons = client->buttons;
  client->buttons = ucmd->buttons;

  // attack button cycles through spectators
  if ((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK))
    Cmd_FollowCycle_f(ent, 1);

  else if ((client->buttons & BUTTON_USE_HOLDABLE) && !(client->oldbuttons & BUTTON_USE_HOLDABLE))
  {
	  if (client->sess.spectatorState == SPECTATOR_FOLLOW)
			StopFollowing(ent);
	  else
		  Cmd_FollowCycle_f(ent, 1);
  }
  
  else if ((client->buttons & BUTTON_RELOAD) && !(client->oldbuttons & BUTTON_RELOAD)) //add hypov8
  {
	  if (client->sess.spectatorState == SPECTATOR_FOLLOW)
		  //Cmd_Spec_f(ent);
			StopFollowing(ent);
  }	/*
  else if ((client->buttons & BUTTON_ANY) && !(client->oldbuttons & BUTTON_ANY)) //add hypov8 todo: specViewModes
  {
	  if (client->sess.spectatorState == SPECTATOR_FOLLOW)
	  {
		  if (client->ps.pm_flags & PMF_JUMP_HELD)
		  {
			  ent->s.chaseCamMode += 1;
			  if (ent->s.chaseCamMode > 2)
				  ent->s.chaseCamMode = 0; //hypov8 ToDo:

			//  switch (mode)
			  {
			 // case 0:  client->sess.specViewModes = 0; break;
			 // case 1:  client->sess.specViewModes = 1; break;
			 // case 2:  client->sess.specViewModes = 2; break;
			  }

		  }
	  }
 } */


	  

 // if (pm.ps->pm_flags & PMF_JUMP_HELD)
  //  Cmd_FollowCycle_f(ent, 1);


}

/*
 =================
 ClientInactivityTimer

 Returns qfalse if the client is dropped
 =================
 */
qboolean ClientInactivityTimer(gclient_t *client)
{
  if (!g_inactivity.integer)
  {
    // give everyone some time, so if the operator sets g_inactivity during
    // gameplay, everyone isn't kicked
    client->inactivityTime = level.time + 60 * 1000;
    client->inactivityWarning = qfalse;
  }
  else if (client->pers.cmd.forwardmove || client->pers.cmd.rightmove || client->pers.cmd.upmove || (client->pers.cmd.buttons
      & BUTTON_ATTACK))
  {
    client->inactivityTime = level.time + g_inactivity.integer * 1000;
    client->inactivityWarning = qfalse;
  }
  else if (!client->pers.localClient)
  {
    if (level.time > client->inactivityTime)
    {
      trap_DropClient(client - level.clients, "Dropped due to inactivity");
      return qfalse;
    }
    if (level.time > client->inactivityTime - 10000 && !client->inactivityWarning)
    {
      client->inactivityWarning = qtrue;
      trap_SendServerCommand(client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"");
    }
  }
  return qtrue;
}

/*
 ==================
 ClientTimerActions
 Actions that happen once a second
 ==================
 */
void ClientTimerActions(gentity_t *ent, int msec)
{
  gclient_t *client;

#ifdef GT_USE_TA_TYPES
  int maxHealth;
#endif

  client = ent->client;
  client->timeResidual += msec;

  while (client->timeResidual >= 1000)
  {
    client->timeResidual -= 1000;

    // regenerate
#ifdef GT_USE_TA_TYPES
    if (bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
    {
      maxHealth = client->ps.stats[STAT_MAX_HEALTH] / 2;
    }
    else if (client->ps.powerups[PW_REGEN])
    {
      maxHealth = client->ps.stats[STAT_MAX_HEALTH];
    }
    else
    {
      maxHealth = 0;
    }
    if (maxHealth)
    {
      if (ent->health < maxHealth)
      {
        ent->health += 15;
        if (ent->health > maxHealth * 1.1)
        {
          ent->health = maxHealth * 1.1;
        }
        G_AddEvent(ent, EV_POWERUP_REGEN, 0);
      }
      else if (ent->health < maxHealth * 2)
      {
        ent->health += 5;
        if (ent->health > maxHealth * 2)
        {
          ent->health = maxHealth * 2;
        }
        G_AddEvent(ent, EV_POWERUP_REGEN, 0);
      }
    if (client->ps.powerups[PW_REGEN])
    {
      if (ent->health < client->ps.stats[STAT_MAX_HEALTH])
      {
        ent->health += 15;
        if (ent->health > client->ps.stats[STAT_MAX_HEALTH] * 1.1)
        {
          ent->health = client->ps.stats[STAT_MAX_HEALTH] * 1.1;
        }
        G_AddEvent(ent, EV_POWERUP_REGEN, 0);
      }
      else if (ent->health < client->ps.stats[STAT_MAX_HEALTH])
      {
        ent->health += 5;
        if (ent->health > client->ps.stats[STAT_MAX_HEALTH])
        {
          ent->health = client->ps.stats[STAT_MAX_HEALTH];
        }
        G_AddEvent(ent, EV_POWERUP_REGEN, 0);
      }

    }
    else
#endif
    {
      // count down health when over max
      if (ent->health > client->ps.stats[STAT_MAX_HEALTH])
      {
        ent->health--;
      }
    }
#if 0   // 0xA5EA, removed
    // count down armor when over max
    if (client->ps.stats[STAT_ARMOR] > client->ps.stats[STAT_MAX_HEALTH])
    {
      client->ps.stats[STAT_ARMOR]--;
    }
#endif
  }
  /*  // 0xA5EA, FIXME:
   //  #ifdef MISSIONPACK
   if( bg_itemlist[client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN ) {
   int w, max, inc, t, i;
   int weapList[]={WP_MACHINEGUN,WP_SHOTGUN,WP_GRENADE_LAUNCHER,WP_ROCKET_LAUNCHER,WP_LIGHTNING,WP_RAILGUN,WP_PLASMAGUN,WP_BFG,WP_NAILGUN,WP_PROX_LAUNCHER,WP_CHAINGUN};
   int weapCount = sizeof(weapList) / sizeof(int);
   //
   for (i = 0; i < weapCount; i++) {
   w = weapList[i];

   switch(w) {
   case WP_MACHINEGUN: max = 50; inc = 4; t = 1000; break;
   case WP_SHOTGUN: max = 10; inc = 1; t = 1500; break;
   case WP_GRENADE_LAUNCHER: max = 10; inc = 1; t = 2000; break;
   case WP_ROCKET_LAUNCHER: max = 10; inc = 1; t = 1750; break;
   case WP_LIGHTNING: max = 50; inc = 5; t = 1500; break;
   case WP_RAILGUN: max = 10; inc = 1; t = 1750; break;
   case WP_PLASMAGUN: max = 50; inc = 5; t = 1500; break;
   case WP_BFG: max = 10; inc = 1; t = 4000; break;
   case WP_NAILGUN: max = 10; inc = 1; t = 1250; break;
   case WP_PROX_LAUNCHER: max = 5; inc = 1; t = 2000; break;
   case WP_CHAINGUN: max = 100; inc = 5; t = 1000; break;
   default: max = 0; inc = 0; t = 1000; break;
   }
   client->ammoTimes[w] += msec;
   if ( client->ps.ammo[w] >= max ) {
   client->ammoTimes[w] = 0;
   }
   if ( client->ammoTimes[w] >= t ) {
   while ( client->ammoTimes[w] >= t )
   client->ammoTimes[w] -= t;
   client->ps.ammo[w] += inc;
   if ( client->ps.ammo[w] > max ) {
   client->ps.ammo[w] = max;
   }
   }
   }
   }
   #endif
   */
}

/*
 ====================
 ClientIntermissionThink
 ====================
 */
void ClientIntermissionThink(gclient_t *client)
{
  client->ps.eFlags &= ~EF_TALK;
  client->ps.eFlags &= ~EF_FIRING;

  // the level will exit when everyone wants to or after timeouts

  // swap and latch button actions
  client->oldbuttons = client->buttons;
  client->buttons = client->pers.cmd.buttons;
  if (client->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE) & (client->oldbuttons ^ client->buttons))
  {
    // this used to be an ^1 but once a player says ready, it should stick
    client->readyToExit = 1;
  }
}

/*
 ================
 ClientEvents

 Events will be passed on to the clients for presentation,
 but any server game effects are handled here
 ================
 */
void ClientEvents(gentity_t *ent, int oldEventSequence)
{
  int i, j;
  int event;
  gclient_t *client;
  int damage;
  //vec3_t dir;
  vec3_t origin, angles;
  //	qboolean	fired;
  gitem_t *item;
  gentity_t *drop;

  client = ent->client;

  if (oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS)
    oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;

  for (i = oldEventSequence; i < client->ps.eventSequence; i++)
  {
    event = client->ps.events[i & (MAX_PS_EVENTS - 1)];

    switch (event)
    {
	case EV_FALL_SHORT: //add hypov8 fall short
    case EV_FALL_MEDIUM:
    case EV_FALL_FAR:
      if (ent->s.eType != ET_PLAYER)
        break; // not in the player model

      if (g_dmflags.integer & DF_NO_FALLING)
        break;

      if (event == EV_FALL_FAR)
		  damage = 60;	//hypov8 increase fall damage
	  else if (event == EV_FALL_MEDIUM)
		  damage = 35;	//hypov8 add fall damage event
      else
          damage = 15;

      //VectorSet(dir, 0, 0, 1);
      ent->pain_debounce_time = level.time + 200; // no normal pain sound
      G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
      break;

    case EV_FIRE_WEAPON:
	case EV_FIRE_SHOTGUN: //
	case EV_FIRE_SPISTOL: //hypov8
    case EV_FIRE_HMG_SHOT:
      FireWeapon(ent);
      break;

    case EV_USE_ITEM1: // teleporter
      // drop flags in CTF
      item = NULL;
      j = 0;

      if (ent->client->ps.powerups[PW_DRAGONFLAG])
      {
        item = BG_FindItemForPowerup(PW_DRAGONFLAG);
        j = PW_DRAGONFLAG;
      }
      else if (ent->client->ps.powerups[PW_NIKKIFLAG])
      {
        item = BG_FindItemForPowerup(PW_NIKKIFLAG);
        j = PW_NIKKIFLAG;
      }
      else if (ent->client->ps.powerups[PW_NEUTRALFLAG])
      {
        item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
        j = PW_NEUTRALFLAG;
      }
	  /* add hypov8 */
	 /* else if (ent->client->ps.powerups[PW_STOLENSCASH])
	  {
		  item = BG_FindItemForPowerup(PW_STOLENSCASH);
		  j = PW_STOLENSCASH; // hypov8 add cash 'stay' time limit??
	  }*/

	  

      if (item)
      {
        drop = Drop_Item(ent, item, 0);
        // decide how many seconds it has left
        //drop->count = (ent->client->ps.powerups[j] - level.time) / 1000;
		drop->count = G_PowerupCountdown(j, ent->client->ps.powerups[j]); //hypov8 add
        if (drop->count < 1)
          drop->count = 1;

        ent->client->ps.powerups[j] = 0;
      }

#ifdef GT_USE_TA_TYPES
      if (g_gametype.integer == GT_HARVESTER)
      {
        if (ent->client->ps.generic1 > 0)
        {
          if (ent->client->sess.sessionTeam == TEAM_DRAGONS)
          {
            item = BG_FindItem("Blue Cube");
          }
          else
          {
            item = BG_FindItem("Red Cube");
          }
          if (item)
          {
            for (j = 0; j < ent->client->ps.generic1; j++)
            {
              drop = Drop_Item(ent, item, 0);
              if (ent->client->sess.sessionTeam == TEAM_DRAGONS)
              {
                drop->spawnflags = TEAM_NIKKIS;
              }
              else
              {
                drop->spawnflags = TEAM_DRAGONS;
              }
            }
          }
          ent->client->ps.generic1 = 0;
        }
      }
#endif
      SelectSpawnPoint(ent->client->ps.origin, origin, angles);
      TeleportPlayer(ent, origin, angles);
      break;

    case EV_USE_ITEM2: // medkit
      ent->health = ent->client->ps.stats[STAT_MAX_HEALTH]; // 0xA5EA + 25;
      break;

#ifdef USE_KAMIKAZE
      case EV_USE_ITEM3: // kamikaze
      // make sure the invulnerability is off
      ent->client->invulnerabilityTime = 0;
      // start the kamikze
      G_StartKamikaze(ent);
      break;
#endif
#if 0
      case EV_USE_ITEM4: // portal
      if (ent->client->portalID)
      {
        DropPortalSource(ent);
      }
      else
      {
        DropPortalDestination(ent);
      }
      break;
      case EV_USE_ITEM5: // invulnerability
      ent->client->invulnerabilityTime = level.time + 10000;
      break;
#endif

    default:
      break;
    }
  }
}


#if 0 //def MISSIONPACK //hypov8 disable:
/*
 ==============
 StuckInOtherClient
 ==============
 */
static int StuckInOtherClient(gentity_t *ent)
{
  int i;
  gentity_t *ent2;

  ent2 = &g_entities[0];
  for (i = 0; i < MAX_CLIENTS; i++, ent2++)
  {
    if (ent2 == ent)
    continue;

    if (!ent2->inuse)
    continue;

    if (!ent2->client)
    continue;

    if (ent2->health <= 0)
    continue;

    if (ent2->r.absmin[0] > ent->r.absmax[0])
    continue;

    if (ent2->r.absmin[1] > ent->r.absmax[1])
    continue;

    if (ent2->r.absmin[2] > ent->r.absmax[2])
    continue;

    if (ent2->r.absmax[0] < ent->r.absmin[0])
    continue;

    if (ent2->r.absmax[1] < ent->r.absmin[1])
    continue;

    if (ent2->r.absmax[2] < ent->r.absmin[2])
    continue;

    return qtrue;
  }
  return qfalse;
}
#endif
void BotTestSolid(vec3_t origin);

/*
 ==============
 SendPendingPredictableEvents
 ==============
 */
void SendPendingPredictableEvents(playerState_t *ps)
{
  gentity_t *t;
  int event, seq;
  int extEvent, number;

  // if there are still events pending
  if (ps->entityEventSequence < ps->eventSequence)
  {
    // create a temporary entity for this event which is sent to everyone
    // except the client who generated the event
    seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
    event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
    // set external event to zero before calling BG_PlayerStateToEntityState
    extEvent = ps->externalEvent;
    ps->externalEvent = 0;
    // create temporary entity for event
    t = G_TempEntity(ps->origin, event);
    number = t->s.number;
    BG_PlayerStateToEntityState(ps, &t->s, qtrue);
    t->s.number = number;
    t->s.eType = ET_EVENTS + event;
    t->s.eFlags |= EF_PLAYER_EVENT;
    t->s.otherEntityNum = ps->clientNum;
    // send to everyone except the client who generated the event
    t->r.svFlags |= SVF_NOTSINGLECLIENT;
    t->r.singleClient = ps->clientNum;
    // set back external event
    ps->externalEvent = extEvent;
  }
}

#if 0 //unvan .52
/*
==============
 G_UnlaggedStore

 Called on every server frame.  Stores position data for the client at that
 into client->unlaggedHist[] and the time into level.unlaggedTimes[].
 This data is used by G_UnlaggedCalc()
==============
*/
void G_UnlaggedStore()
{
	int        i = 0;
	gentity_t  *ent;
	unlagged_t *save;

	if ( !g_unlagged.integer )
	{
		return;
	}

	level.unlaggedIndex++;

	if ( level.unlaggedIndex >= MAX_UNLAGGED_MARKERS )
	{
		level.unlaggedIndex = 0;
	}

	level.unlaggedTimes[ level.unlaggedIndex ] = level.time;

	for ( i = 0; i < level.maxclients; i++ )
	{
		ent = &g_entities[ i ];
		save = &ent->client->unlaggedHist[ level.unlaggedIndex ];
		save->used = false;

		if ( !ent->r.linked || !( ent->r.contents & CONTENTS_BODY ) )
		{
			continue;
		}

		if ( ent->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		VectorCopy( ent->r.mins, save->mins );
		VectorCopy( ent->r.maxs, save->maxs );
		VectorCopy( ent->s.pos.trBase, save->origin );
		save->used = true;
	}
}

/*
==============
 G_UnlaggedClear

 Mark all unlaggedHist[] markers for this client invalid.  Useful for
 preventing teleporting and death.
==============
*/
void G_UnlaggedClear( gentity_t *ent )
{
	int i;

	for ( i = 0; i < MAX_UNLAGGED_MARKERS; i++ )
	{
		ent->client->unlaggedHist[ i ].used = false;
	}
}

/*
==============
 G_UnlaggedCalc

 Loops through all active clients and calculates their predicted position
 for time then stores it in client->unlaggedCalc
==============
*/
void G_UnlaggedCalc( int time, gentity_t *rewindEnt )
{
	int       i = 0;
	gentity_t *ent;
	int       startIndex = level.unlaggedIndex;
	int       stopIndex = -1;
	int       frameMsec = 0;
	float     lerp = 0.5f;

	if ( !g_unlagged.integer )
	{
		return;
	}

	// clear any calculated values from a previous run
	for ( i = 0; i < level.maxclients; i++ )
	{
		ent = &g_entities[ i ];

		if ( !ent->inuse )
		{
			continue;
		}

		ent->client->unlaggedCalc.used = false;
	}

	for ( i = 0; i < MAX_UNLAGGED_MARKERS; i++ )
	{
		if ( level.unlaggedTimes[ startIndex ] <= time )
		{
			break;
		}

		stopIndex = startIndex;

		if ( --startIndex < 0 )
		{
			startIndex = MAX_UNLAGGED_MARKERS - 1;
		}
	}

	if ( i == MAX_UNLAGGED_MARKERS )
	{
		// if we searched all markers and the oldest one still isn't old enough
		// just use the oldest marker with no lerping
		lerp = 0.0f;
	}

	// client is on the current frame, no need for unlagged
	if ( stopIndex == -1 )
	{
		return;
	}

	// lerp between two markers
	frameMsec = level.unlaggedTimes[ stopIndex ] -
	            level.unlaggedTimes[ startIndex ];

	if ( frameMsec > 0 )
	{
		lerp = ( float )( time - level.unlaggedTimes[ startIndex ] ) /
		       ( float ) frameMsec;
	}

	for ( i = 0; i < level.maxclients; i++ )
	{
		ent = &g_entities[ i ];

		if ( ent == rewindEnt )
		{
			continue;
		}

		if ( !ent->inuse )
		{
			continue;
		}

		if ( !ent->r.linked || !( ent->r.contents & CONTENTS_BODY ) )
		{
			continue;
		}

		if ( ent->client->pers.connected != CON_CONNECTED )
		{
			continue;
		}

		if ( !ent->client->unlaggedHist[ startIndex ].used )
		{
			continue;
		}

		if ( !ent->client->unlaggedHist[ stopIndex ].used )
		{
			continue;
		}

		// between two unlagged markers
		VectorLerpTrem( lerp, ent->client->unlaggedHist[ startIndex ].mins,
		                ent->client->unlaggedHist[ stopIndex ].mins,
		                ent->client->unlaggedCalc.mins );
		VectorLerpTrem( lerp, ent->client->unlaggedHist[ startIndex ].maxs,
		                ent->client->unlaggedHist[ stopIndex ].maxs,
		                ent->client->unlaggedCalc.maxs );
		VectorLerpTrem( lerp, ent->client->unlaggedHist[ startIndex ].origin,
		                ent->client->unlaggedHist[ stopIndex ].origin,
		                ent->client->unlaggedCalc.origin );

		ent->client->unlaggedCalc.used = true;
	}
}

/*
==============
 G_UnlaggedOff

 Reverses the changes made to all active clients by G_UnlaggedOn()
==============
*/
void G_UnlaggedOff()
{
	int       i = 0;
	gentity_t *ent;

	if ( !g_unlagged.integer )
	{
		return;
	}

	for ( i = 0; i < level.maxclients; i++ )
	{
		ent = &g_entities[ i ];

		if ( !ent->client->unlaggedBackup.used )
		{
			continue;
		}

		VectorCopy( ent->client->unlaggedBackup.mins, ent->r.mins );
		VectorCopy( ent->client->unlaggedBackup.maxs, ent->r.maxs );
		VectorCopy( ent->client->unlaggedBackup.origin, ent->r.currentOrigin );
		ent->client->unlaggedBackup.used = false;
		trap_LinkEntity( ent );
	}
}

/*
==============
 G_UnlaggedOn

 Called after G_UnlaggedCalc() to apply the calculated values to all active
 clients.  Once finished tracing, G_UnlaggedOff() must be called to restore
 the clients' position data

 As an optimization, all clients that have an unlagged position that is
 not touchable at "range" from "muzzle" will be ignored.  This is required
 to prevent a huge amount of trap_LinkEntity() calls per user cmd.
==============
*/

void G_UnlaggedOn( gentity_t *attacker, vec3_t muzzle, float range )
{
	int        i = 0;
	gentity_t  *ent;
	unlagged_t *calc;

	if ( !g_unlagged.integer )
	{
		return;
	}

	if ( !attacker->client->pers.useUnlagged )
	{
		return;
	}

	for ( i = 0; i < level.maxclients; i++ )
	{
		ent = &g_entities[ i ];
		calc = &ent->client->unlaggedCalc;

		if ( !calc->used )
		{
			continue;
		}

		if ( ent->client->unlaggedBackup.used )
		{
			continue;
		}

		if ( !ent->r.linked || !( ent->r.contents & CONTENTS_BODY ) )
		{
			continue;
		}

		if ( VectorCompare( ent->r.currentOrigin, calc->origin ) )
		{
			continue;
		}

		if ( muzzle )
		{
			float r1 = Distance( calc->origin, calc->maxs );
			float r2 = Distance( calc->origin, calc->mins );
			float maxRadius = ( r1 > r2 ) ? r1 : r2;

			if ( Distance( muzzle, calc->origin ) > range + maxRadius )
			{
				continue;
			}
		}

		// create a backup of the real positions
		VectorCopy( ent->r.mins, ent->client->unlaggedBackup.mins );
		VectorCopy( ent->r.maxs, ent->client->unlaggedBackup.maxs );
		VectorCopy( ent->r.currentOrigin, ent->client->unlaggedBackup.origin );
		ent->client->unlaggedBackup.used = true;

		// move the client to the calculated unlagged position
		VectorCopy( calc->mins, ent->r.mins );
		VectorCopy( calc->maxs, ent->r.maxs );
		VectorCopy( calc->origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}
}

/*
==============
 G_UnlaggedDetectCollisions

 cgame prediction will predict a client's own position all the way up to
 the current time, but only updates other player's positions up to the
 postition sent in the most recent snapshot.

 This allows player X to essentially "move through" the position of player Y
 when player X's cmd is processed with Pmove() on the server.  This is because
 player Y was clipping player X's Pmove() on his client, but when the same
 cmd is processed with Pmove on the server it is not clipped.

 Long story short (too late): don't use unlagged positions for players who
 were blocking this player X's client-side Pmove().  This makes the assumption
 that if player X's movement was blocked in the client he's going to still
 be up against player Y when the Pmove() is run on the server with the
 same cmd.

 NOTE: this must be called after Pmove() and G_UnlaggedCalc()
==============
*/
static void G_UnlaggedDetectCollisions( gentity_t *ent )
{
	unlagged_t *calc;
	trace_t    tr;
	float      r1, r2;
	float      range;

	if ( !g_unlagged.integer )
	{
		return;
	}

	if ( !ent->client->pers.useUnlagged )
	{
		return;
	}

	calc = &ent->client->unlaggedCalc;

	// if the client isn't moving, this is not necessary
	if ( VectorCompare( ent->client->oldOrigin, ent->client->ps.origin ) )
	{
		return;
	}

	range = Distance( ent->client->oldOrigin, ent->client->ps.origin );

	// increase the range by the player's largest possible radius since it's
	// the players bounding box that collides, not their origin
	r1 = Distance( calc->origin, calc->mins );
	r2 = Distance( calc->origin, calc->maxs );
	range += ( r1 > r2 ) ? r1 : r2;

	G_UnlaggedOn( ent, ent->client->oldOrigin, range );

	trap_Trace( &tr, ent->client->oldOrigin, ent->r.mins, ent->r.maxs,
	            ent->client->ps.origin, ent->s.number, MASK_PLAYERSOLID, 0 );

	if ( tr.entityNum >= 0 && tr.entityNum < MAX_CLIENTS )
	{
		g_entities[ tr.entityNum ].client->unlaggedCalc.used = false;
	}

	G_UnlaggedOff();
}


#endif
/*
 ==============
 ClientThink_real
 This will be called once for each client frame, which will
 usually be a couple times for each server frame on fast clients.
 If "g_synchronousClients 1" is set, this will be called exactly
 once for each server frame, which makes for smooth demo recording.
 ==============
 */
void ClientThink_real(gentity_t *ent)
{
  gclient_t *client;
  pmove_t pm;
  int oldEventSequence;
  int msec;
  usercmd_t *ucmd;

  client = ent->client;

  // don't think if the client is not yet connected (and thus not yet spawned in)
  if (client->pers.connected != CON_CONNECTED)
    return;

  // mark the time, so the connection sprite can be removed
  ucmd = &ent->client->pers.cmd;

  // sanity check the command time to prevent speedup cheating
  if (ucmd->serverTime > level.time + 200)
    ucmd->serverTime = level.time + 200;

  if (ucmd->serverTime < level.time - 1000)
    ucmd->serverTime = level.time - 1000;
//unlagged - backward reconciliation #4
	// frameOffset should be about the number of milliseconds into a frame 
	// this command packet was received, depending on how fast the server
	// does a G_RunFrame()
	client->frameOffset = trap_Milliseconds() - level.frameStartTime;
//unlagged - backward reconciliation #4


//unlagged - true ping
	// save the estimated ping in a queue for averaging later

	// we use level.previousTime to account for 50ms lag correction
	// besides, this will turn out numbers more like what players are used to
	client->pers.pingsamples[client->pers.samplehead] = level.previousTime + client->frameOffset - ucmd->serverTime;
	client->pers.samplehead++;
	if ( client->pers.samplehead >= NUM_PING_SAMPLES ) {
		client->pers.samplehead -= NUM_PING_SAMPLES;
	}

	// initialize the real ping
	if ( g_truePing.integer ) {
		int i, sum = 0;

		// get an average of the samples we saved up
		for ( i = 0; i < NUM_PING_SAMPLES; i++ ) {
			sum += client->pers.pingsamples[i];
		}

		client->pers.realPing = sum / NUM_PING_SAMPLES;
	}
	else {
		// if g_truePing is off, use the normal ping
		client->pers.realPing = client->ps.ping;
	}
//unlagged - true ping


//unlagged - backward reconciliation #4
	// save the command time *before* pmove_fixed messes with the serverTime,
	// and *after* lag simulation messes with it :)
	// attackTime will be used for backward reconciliation later (time shift)
	client->attackTime = ucmd->serverTime;
//unlagged - backward reconciliation #4


//unlagged - smooth clients #1
	// keep track of this for later - we'll use this to decide whether or not
	// to send extrapolated positions for this client
	client->lastUpdateFrame = level.framenum;
//unlagged - smooth clients #1


//unlagged - true ping
	// make sure the true ping is over 0 - with cl_timenudge it can be less
	if ( client->pers.realPing < 0 ) {
		client->pers.realPing = 0;
	}
//unlagged - true ping

  msec = ucmd->serverTime - client->ps.commandTime;

  // following others may result in bad times, but we still want
  // to check for follow toggles
  if (msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW)
    return;

  if (msec > 200)
    msec = 200;

	client->unlaggedTime = ucmd->serverTime;

  if (level.pmoveParams.fixed || client->pers.pmoveFixed) /*pmove_fixed.integer*/
  {
    ucmd->serverTime = ((ucmd->serverTime + level.pmoveParams.msec - 1) / level.pmoveParams.msec) * level.pmoveParams.msec;
    //if (ucmd->serverTime - client->ps.commandTime <= 0)
    //	return;
  }

  // check for exiting intermission
  if (level.intermissiontime)
  {
	//if( self->r.svFlags & SVF_BOT )
	//	G_BotIntermissionThink( client ); //hypov8 todo: needed?
	//else
    ClientIntermissionThink(client);
    return;
  }

  // spectators don't do much
  if (client->sess.sessionTeam == TEAM_SPECTATOR)
  {
    if (client->sess.spectatorState == SPECTATOR_SCOREBOARD)
      return;

    SpectatorThink(ent, ucmd);
    return;
  }

  // check for inactivity timer, but never drop the local client of a non-dedicated server
  if (!ClientInactivityTimer(client))
    return;

	// calculate where ent is currently seeing all the other active clients
	//G_UnlaggedCalc( client->unlaggedTime, ent );

  // clear the rewards if time
  if (level.time > client->rewardTime)
    client->ps.eFlags &= ~(EF_AWARD_EXCELLENT | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP);

  if (client->noclip)
    client->ps.pm_type = PM_NOCLIP;
  else if (client->ps.stats[STAT_HEALTH] <= 0)
    client->ps.pm_type = PM_DEAD;
  else
    client->ps.pm_type = PM_NORMAL;

  client->ps.gravity = g_gravity.value;

  // set speed
  client->ps.speed = g_speed.value;
#ifdef GT_USE_TA_TYPES
  if (client->ps.powerups[PW_HASTE])
    client->ps.speed *= 1.3;
#endif

  // Let go of the hook if we aren't firing
  if (client->ps.weapon == WP_GRAPPLING_HOOK && client->hook && !(ucmd->buttons & BUTTON_ATTACK))
    Weapon_HookFree(client->hook);

  // set up for pmove
  oldEventSequence = client->ps.eventSequence;

  Com_Memset(&pm, 0, sizeof(pm));


  if (ent->flags & FL_FORCE_GESTURE)
  {
    ent->flags &= ~FL_FORCE_GESTURE; //hypov8 todo: should we make a new animation for captured flag?
    ent->client->pers.cmd.buttons |= BUTTON_GESTURE;
  }

  pm.ps = &client->ps;
  pm.cmd = *ucmd;
  if (pm.ps->pm_type == PM_DEAD)
    pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
  else if (ent->r.svFlags & SVF_BOT)
    pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP | CONTENTS_MONSTERCLIP; //hypov8 add: monster
  else
    pm.tracemask = MASK_PLAYERSOLID;

  pm.trace = trap_Trace;
  pm.pointcontents = trap_PointContents;
  pm.debugLevel = g_debugMove.integer;
  pm.noFootsteps = (g_dmflags.integer & DF_NO_FOOTSTEPS) > 0;
  pm.pmove_fixed = level.pmoveParams.fixed | client->pers.pmoveFixed;
  pm.pmove_msec = level.pmoveParams.msec;
  pm.pmove_accurate = level.pmoveParams.accurate;

  VectorCopy(client->ps.origin, client->oldOrigin);

#ifdef USE_GT_SINGLEPLAYER
  if (level.intermissionQueued != 0 && g_singlePlayer.integer)
  {
    if (level.time - level.intermissionQueued >= 1000)
    {
      pm.cmd.buttons = 0;
      pm.cmd.forwardmove = 0;
      pm.cmd.rightmove = 0;
      pm.cmd.upmove = 0;
      if (level.time - level.intermissionQueued >= 2000 && level.time - level.intermissionQueued <= 2500)
      {
        trap_SendConsoleCommand(EXEC_APPEND, "centerview\n");
      }
      ent->client->ps.pm_type = PM_SPINTERMISSION;
    }
  }
#endif
  //hypov8 todo: need a beter way/place?
		pm.ps->bunnyHop = 0;
	if (g_bunnyhop.integer == 1)
		pm.ps->bunnyHop = 1;

	//pm.ps->maxSpeed = g_speed.integer; //hypov8 disabled: G_SPEED_MAX

  Pmove(&pm);

  // save results of pmove
  if (ent->client->ps.eventSequence != oldEventSequence)
    ent->eventTime = level.time;

//unlagged - smooth clients #2
	// clients no longer do extrapolation if cg_smoothClients is 1, because
	// skip correction is all handled server-side now
	// since that's the case, it makes no sense to store the extra info
	// in the client's snapshot entity, so let's save a little bandwidth

  if (g_smoothClients.integer)
    BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
  else
    BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
//unlagged - smooth clients #2

  SendPendingPredictableEvents(&ent->client->ps);

  if (!(ent->client->ps.eFlags & EF_FIRING))
    client->fireHeld = qfalse; // for grapple

  // use the snapped origin for linking so it matches client predicted versions
  VectorCopy(ent->s.pos.trBase, ent->r.currentOrigin);

  VectorCopy(pm.mins, ent->r.mins);
  VectorCopy(pm.maxs, ent->r.maxs);

  ent->waterlevel = pm.waterlevel;
  ent->watertype = pm.watertype;

  // execute client events
  ClientEvents(ent, oldEventSequence);

  // link entity now, after any personal teleporters have been used
  trap_LinkEntity(ent);

  if (!ent->client->noclip)
    G_TouchTriggers(ent);

  // NOTE: now copy the exact origin over otherwise clients can be snapped into solid
  VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);

  //test for solid areas in the AAS file
  BotTestAAS(ent->r.currentOrigin);

  // touch other objects
  ClientImpacts(ent, &pm);

  // save results of triggers and client events
  if (ent->client->ps.eventSequence != oldEventSequence)
    ent->eventTime = level.time;

  // swap and latch button actions
  client->oldbuttons = client->buttons;
  client->buttons = ucmd->buttons;
  client->latched_buttons |= client->buttons & ~client->oldbuttons;

  // check for respawning
  if (client->ps.stats[STAT_HEALTH] <= 0)
  {
    // wait for the attack button to be pressed
    if (level.time > client->respawnTime)
    {
      // forcerespawn is to prevent users from waiting out powerups
      if (g_forcerespawn.integer > 0 && (level.time - client->respawnTime) > g_forcerespawn.integer * 1000)
      {
        respawn(ent);
        return;
      }

      // hypov8 make sure all keys are up b4 respawn
	 // if ((client->ps.pm_flags & PMF_RESPAWNED)&& !(ucmd->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE)))
		//  client->ps.pm_flags &= ~PMF_RESPAWNED;

	  //if (!(client->ps.pm_flags & PMF_RESPAWNED))
	  {
		  if (ucmd->buttons & (BUTTON_ATTACK | BUTTON_USE_HOLDABLE))
			  respawn(ent);
	  }

    }
    return;
  }
  // perform once-a-second actions
  ClientTimerActions(ent, msec);
}

/*
 ==================
 ClientThink
 A new command has arrived from the client
 ==================
 */
void ClientThink(int clientNum)
{
  gentity_t *ent;

  ent = g_entities + clientNum;
  trap_GetUsercmd(clientNum, &ent->client->pers.cmd);

//unlagged - smooth clients #1
	// this is handled differently now

  // mark the time we got info, so we can display the
  // phone jack if they don't get any for a while
  ent->client->lastCmdTime = level.time;

//unlagged - smooth clients #1

  if (!(ent->r.svFlags & SVF_BOT) && !level.pmoveParams.synchronous )
    ClientThink_real(ent);
}

void G_RunClient(gentity_t *ent)
{
  if (!(ent->r.svFlags & SVF_BOT) && !level.pmoveParams.synchronous )
    return;

  ent->client->pers.cmd.serverTime = level.time;
  ClientThink_real(ent);
}

/*
 ==================
 SpectatorClientEndFrame

 ==================
 */
void SpectatorClientEndFrame(gentity_t *ent)
{
  gclient_t *cl;

  // if we are doing a chase cam or a remote view, grab the latest info
  if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
  {
    int clientNum, flags;

    clientNum = ent->client->sess.spectatorClient;

    // team follow1 and team follow2 go to whatever clients are playing
    if (clientNum == -1)
      clientNum = level.follow1;
    else if (clientNum == -2)
      clientNum = level.follow2;

    if (clientNum >= 0)
    {
      cl = &level.clients[clientNum];
      if (cl->pers.connected == CON_CONNECTED && cl->sess.sessionTeam != TEAM_SPECTATOR)
      {
        flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
        ent->client->ps = cl->ps;
        ent->client->ps.pm_flags |= PMF_FOLLOW;
        ent->client->ps.eFlags = flags;
        return;
      }
      else
      {
        // drop them to free spectators unless they are dedicated camera followers
        if (ent->client->sess.spectatorClient >= 0)
        {
          ent->client->sess.spectatorState = SPECTATOR_FREE;
          ClientBegin(ent->client - level.clients);
        }
      }
    }
  }

  if (ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD)
    ent->client->ps.pm_flags |= PMF_SCOREBOARD;
  else
    ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
}

/*
 ==============
 ClientEndFrame
 Called at the end of each server frame for each connected client
 A fast client will have multiple ClientThink for each ClientEdFrame,
 while a slow client may have multiple ClientEndFrame between ClientThink.
 ==============
 */
void ClientEndFrame(gentity_t *ent)
{
  int i;
  //clientPersistant_t *pers;

//unlagged - smooth clients #1
	int frames;
//unlagged - smooth clients #1
  if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
  {
    SpectatorClientEndFrame(ent);
    return;
  }

  // turn off any expired powerups
  for (i = PW_DEGRADE_START; i < MAX_POWERUPS; i++)
  {
    if (ent->client->ps.powerups[i] != 0 && ent->client->ps.powerups[i] < level.time)
      ent->client->ps.powerups[i] = 0; 
  }

  // If the end of unit layout is displayed, don't give
  // the player any normal movement attributes
  if (level.intermissiontime)
    return;

  // burn from lava, etc
  P_WorldEffects(ent);

  // apply all the damage taken this frame
  P_DamageFeedback(ent);

//unlagged - smooth clients #1
	// this is handled differently now

  // add the EF_CONNECTION flag if we haven't gotten commands recently
  if (level.time - ent->client->lastCmdTime > 1000 && !(ent->r.svFlags & SVF_BOT))
    ent->client->ps.eFlags |= EF_CONNECTION;
  else
    ent->client->ps.eFlags &= ~EF_CONNECTION;

//unlagged - smooth clients #1

  ent->client->ps.stats[STAT_HEALTH] = ent->health; // FIXME: get rid of ent->health...

  G_SetClientSound(ent);

	// set the latest information
//unlagged - smooth clients #2
	// clients no longer do extrapolation if cg_smoothClients is 1, because
	// skip correction is all handled server-side now
	// since that's the case, it makes no sense to store the extra info
	// in the client's snapshot entity, so let's save a little bandwidth

  if (g_smoothClients.integer)
    BG_PlayerStateToEntityStateExtraPolate(&ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue);
  else
    BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
//unlagged - smooth clients #2

  SendPendingPredictableEvents(&ent->client->ps);

//unlagged - smooth clients #1
#if 1 //hypov8 todo: not in 
	// mark as not missing updates initially
	ent->client->ps.eFlags &= ~EF_CONNECTION;

	// see how many frames the client has missed
	frames = level.framenum - ent->client->lastUpdateFrame - 1;

	// don't extrapolate more than two frames
	if ( frames > 2 ) {
		frames = 2;

		// if they missed more than two in a row, show the phone jack
		ent->client->ps.eFlags |= EF_CONNECTION;
		ent->s.eFlags |= EF_CONNECTION;
	}

	// did the client miss any frames?
	if ( frames > 0 && g_smoothClients.integer ) {
		// yep, missed one or more, so extrapolate the player's movement
		G_PredictPlayerMove( ent, (float)frames / sv_fps.integer );
		// save network bandwidth
		//SnapVector( ent->s.pos.trBase ); //unlag orig
		SnapVector(ent->s.pos.trBase);

	}
#endif
//unlagged - smooth clients #1

//unlagged - backward reconciliation #1
	// store the client's position for backward reconciliation later
	G_StoreHistory( ent );
//unlagged - backward reconciliation #1
   // set the bit for the reachability area the client is currently in
  //	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
  //	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}
