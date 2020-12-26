/*
===========================================================================
Copyright (C) 2006-20xy Lars '0xA5EA' Kandler

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

#include "g_local.h"

#define HM_MAX_WEAPONS 10
typedef struct hmWeaponnfo_s
{
  gitem_t *itemptr;
  int initialAmmo;
  int initialrounds;
  int ammoIncr;
  int maxAmmo;
}
hmWeaponinfo_t;

static hmWeaponinfo_t hmWeaplist[HM_MAX_WEAPONS];

typedef struct hm_locals_s
{
  qboolean weapChangeDisabled;
  int numActiveWeapons;
  int prevWeapon;
  int curWeapon;
  int nextWeapon;
} hm_locals_t;

hm_locals_t hmLocals;

/*
=============================================
HM_OnlyOneWeapon
returns true if only one weapon is
selected by cvar g_hmWeapons, false
otherwise
=============================================
*/
//gitem_t HM_Weaponlist;

qboolean HM_OnlyOneWeapon(void)
{
  switch (g_hmWeapons.integer)
  {
  case 0x001:
  case 0x002:
  case 0x004:
  case 0x008:
  case 0x010:
  case 0x020:
  case 0x040:
    return qtrue;
  default:
    return qfalse;
  }
}
/*
=============================================
HM_IsWeaponDisabled
returns true if a specific weapon is
disabled in cvar g_hmWeapons, false otherwise
=============================================
*/
qboolean HM_IsWeaponDisabled(weapon_t w)
{
  // never allow all weapons to be disabled
  if (g_hmWeapons.integer == 0)
    return qfalse;

  switch (w)
  {
  case WP_PISTOL:
    return (g_hmWeapons.integer & HM_WEAPFLAG_PISTOL) == 0;

  case WP_MACHINEGUN:
    return (g_hmWeapons.integer & HM_WEAPFLAG_MACHINEGUN) == 0;

  case WP_SHOTGUN:
    return (g_hmWeapons.integer & HM_WEAPFLAG_SHOTGUN) == 0;

  case WP_GRENADE_LAUNCHER:
    return (g_hmWeapons.integer & HM_WEAPFLAG_GRENADE_LAUNCHER) == 0;

  case WP_ROCKET_LAUNCHER:
    return (g_hmWeapons.integer & HM_WEAPFLAG_ROCKET_LAUNCHER) == 0;

  case WP_HMG:
    return (g_hmWeapons.integer & HM_WEAPFLAG_HMG) == 0;
#ifdef USE_FLAMEGUN
  case WP_FLAMER:
    return (g_hmWeapons.integer & HM_WEAPFLAG_FLAMER) == 0;
#endif
  default:
    G_Printf("Warning: Bad case in HM_IsWeaponDisabled( weapon_t w )\n");
    return qtrue;
  }
}

/*
=============================================
             HM_NextWeapon
=============================================
*/
static int HM_NextWeapon(void)
{
  int i = 10, next = 0;
  int a = hmLocals.numActiveWeapons;
  int p = hmLocals.prevWeapon;
  int c = hmLocals.curWeapon;

  if (a == 1)
    return 0;

  else if ((a == 2) || !(g_hmFlags.integer & HMFLAG_RANDWEAPONS))
  {
    next = (c + 1)%a;
    return next;
  }

  // randomly choose next weapon
  while (i--)
  {
    next = (rand()%a + trap_Milliseconds())%a;
    if (a == 3)
    {
      if (next != c)
        break;
    }
    else
    {
      if (next != c && next != p)
        break;
    }
  }

  G_Printf("i = %d next %d; curWeapon %d\n", i, hmWeaplist[next].itemptr->giTag, hmWeaplist[c].itemptr->giTag);
  if (i == -1)
  {
    G_Printf("searched 10x for a new random weapon in HM_NextWeapon( void ) \n");
  }
  return next;
}

/*
=============================================
       HM_ChangeClientWeapons
=============================================
*/
static void HM_ChangeClientWeapons(void)
{
  int i;
  hmWeaponinfo_t *cur, *next;
  gclient_t *cl;

  next = &hmWeaplist[hmLocals.nextWeapon];
  cur  = &hmWeaplist[hmLocals.curWeapon];

  for (i = 0; i < level.maxclients; i++)
  {
    cl = &level.clients[i];

    if (cl->pers.connected != CON_CONNECTED)
      continue;

    if (cl->sess.sessionTeam == TEAM_SPECTATOR)
      continue;

    // add new weapon and ammo
    cl->ps.stats[STAT_WEAPONS] |= (1 << next->itemptr->giTag);
	cl->ps.ammo_all[BG_AmmoCombineCheck(next->itemptr->giTag)] = next->initialAmmo;
	cl->ps.ammo_mag[next->itemptr->giTag] = BG_WeaponMaxMagCount(next->itemptr->giTag); //next->initialrounds;

    // remove old weapon + ammo
    cl->ps.stats[STAT_WEAPONS] &= ~(1 << cur->itemptr->giTag);
	cl->ps.ammo_all[BG_AmmoCombineCheck(cur->itemptr->giTag)] = 0;
	cl->ps.ammo_mag[cur->itemptr->giTag] = 0;

    // select new weapon
	cl->ps.pm_flags |= PMF_WEAPON_SWITCH;
	cl->ps.persistant[ PERS_NEWWEAPON ] = next->itemptr->giTag;
  }
  hmLocals.prevWeapon = hmLocals.curWeapon;
  hmLocals.curWeapon  = hmLocals.nextWeapon;

  level.hitmen.curWeapon  = (weapon_t)next->itemptr->giTag;
  level.hitmen.curInitialAmmo = next->initialAmmo;
}

/*
=============================================
HM_AddClientAmmo
=============================================
*/
static void HM_AddClientAmmo(void)
{
  int   i;
  int   c = hmLocals.curWeapon;
  gclient_t *client;
  weapon_t ct = (weapon_t)hmWeaplist[c].itemptr->giTag;

  for (i = 0; i < level.maxclients; i++)
  {
    client = &level.clients[i];

    if (client->pers.connected != CON_CONNECTED )
      continue;

    if (client->sess.sessionTeam == TEAM_SPECTATOR)
      continue;

	ct = (weapon_t)BG_AmmoCombineCheck(ct );
	client->ps.ammo_all[ct] += hmWeaplist[c].ammoIncr;

	if (client->ps.ammo_all[ct] > hmWeaplist[c].maxAmmo)
		client->ps.ammo_all[ct] = hmWeaplist[c].maxAmmo;
  }
}
/*
=============================================
HM_InitWeaponlist
generates the weaponlist dependend on
what g_weaponmod is set
=============================================
*/
static int HM_InitWeaponlist(void)
{
  int i, j = 0;

  for (i = 0; i < bg_numItems; i++)
  {
    if (j >= HM_MAX_WEAPONS)
      break;

    // check for valid items
    if (bg_itemlist[i].giType != IT_WEAPON)
      continue;

    if (bg_itemlist[i].giTag == WP_CROWBAR)
      continue;

    if (bg_itemlist[i].giTag == WP_GRAPPLING_HOOK)
      continue;

    if (g_weaponmod.integer == WM_HITMEN && bg_itemlist[i].giTag < WP_SHOTGUN)
      continue;

    if (g_weaponmod.integer == WM_HITMENREALMODE && bg_itemlist[i].giTag > WP_SHOTGUN)
      continue;

    // item is a valid weapon, is it disabled ?
    if (HM_IsWeaponDisabled((weapon_t)bg_itemlist[i].giTag))
      continue;

    hmWeaplist[j].itemptr = &bg_itemlist[i];

    switch (hmWeaplist[j].itemptr->giTag)
    {
    case WP_PISTOL:
      hmWeaplist[j].ammoIncr      = 4;
      hmWeaplist[j].initialAmmo   = 10;
      hmWeaplist[j].initialrounds = 5;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;

    case WP_MACHINEGUN:
      hmWeaplist[j].ammoIncr      = 10;
      hmWeaplist[j].initialAmmo   = 0;
      hmWeaplist[j].initialrounds = 50;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;

    case WP_SHOTGUN:
      hmWeaplist[j].ammoIncr      = 5;
      hmWeaplist[j].initialAmmo   = 8;
      hmWeaplist[j].initialrounds = 8;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;

    case WP_GRENADE_LAUNCHER:
      hmWeaplist[j].ammoIncr      = 2;
      hmWeaplist[j].initialAmmo   = 6;
      hmWeaplist[j].initialrounds = 3;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;

    case WP_ROCKET_LAUNCHER:
      hmWeaplist[j].ammoIncr      = 3;
      hmWeaplist[j].initialAmmo   = 5;
      hmWeaplist[j].initialrounds = 5;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;

    case WP_HMG:
      hmWeaplist[j].ammoIncr      = 7;
      hmWeaplist[j].initialAmmo   = 0;
      hmWeaplist[j].initialrounds = 30;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;
#ifdef USE_FLAMEGUN
    case WP_FLAMER:
      hmWeaplist[j].ammoIncr      = 10;
      hmWeaplist[j].initialAmmo   = 50;
      hmWeaplist[j].initialrounds = 50;
      hmWeaplist[j].maxAmmo       = BG_WeaponMaxAmmoCount(hmWeaplist[j].itemptr->giTag); 
      break;
#endif
    default:
      Com_Printf("bad case in HM_InitWeaponlist( void ) \n");
    }
    j++;
  }
  G_Printf( "Hitmen: registered %d weapons\n", j );
  return j;
}

/*
=============================================
HM_ResetWeaponTimer
=============================================
*/
static void HM_ResetWeaponTimer(void)
{
  int weapTime = g_hmWeapontime.integer;

  // avoid neg. values
  if (weapTime < 0)
    weapTime *= -1;

  // clamp to a valid range
  if (weapTime > HM_MAX_WEAPONTIME)
    weapTime = HM_MAX_WEAPONTIME;
  else if (weapTime < HM_MIN_WEAPONTIME )
    weapTime = HM_MIN_WEAPONTIME;

  level.hitmen.weaponTimer =  weapTime;
}

/*
=============================================
HM_InitHitmen
generates the weaponlist dependend value
of g_weaponmod
=============================================
*/
void HM_InitHitmen(void)
{
  Com_Memset(&hmLocals, 0, sizeof(hmLocals));

  if (g_weaponmod.integer < WM_HITMEN)
    return;

  HM_ResetWeaponTimer();

  hmLocals.numActiveWeapons = HM_InitWeaponlist();

  hmLocals.prevWeapon =
  hmLocals.curWeapon  =
  hmLocals.nextWeapon = HM_NextWeapon();

  level.hitmen.curWeapon  = (weapon_t)hmWeaplist[hmLocals.curWeapon].itemptr->giTag;
  level.hitmen.curInitialAmmo = hmWeaplist[hmLocals.curWeapon].initialAmmo;
}


/*
=============================================
HM_SetClientWeaponStates
=============================================
*/
static void HM_SetClientWeaponStates( weaponstate_t weapState )
{
  int i;
  gclient_t *client;

  for (i = 0; i < level.maxclients; i++)
  {
    client = &level.clients[i];


    if (client->pers.connected != CON_CONNECTED )
      continue;

    if (client->sess.sessionTeam == TEAM_SPECTATOR)
      continue;

    client->ps.weaponstate = weapState;
  }
}

/*
=============================================
HM_IsWeaponchangeDisabled
=============================================
*/
qboolean HM_IsWeaponchangeDisabled(void)
{
  if (g_weaponmod.integer < WM_HITMEN)
    return qfalse;

  return hmLocals.weapChangeDisabled;
}

/*
=============================================
HM_SendMsgEvent
=============================================
*/
static void HM_SendMsgEvent(entity_event_t event, int msg )
{
  vec3_t hmOrigin = {0.0f, 0.0f, 0.0f};
  gentity_t *te;
  te         = G_TempEntity(hmOrigin, EV_HITMEN_INFOMSG);
  te->s.eventParm = msg;
  te->r.svFlags  |= SVF_BROADCAST;
}

/*
=============================================
HM_PlayGlobalSound
=============================================
*/
static void HM_PlayGlobalSound(char *sndFile)
{
  vec3_t hmOrigin = {0.0f, 0.0f, 0.0f};
  gentity_t *te;

  te = G_TempEntity(hmOrigin, EV_GLOBAL_SOUND);
  te->s.eventParm = G_SoundIndex(sndFile);
  te->r.svFlags  |= SVF_BROADCAST;

}
/*
=============================================
HM_SetWeaponTimer
=============================================
*/
static void HM_Think(void)
{
  static int secCount;

  if ((level.time%1000))
    return;

  if (secCount++ > 2)
  {
    HM_AddClientAmmo();
    secCount = 0;
  }
#if 0       // FIXME: 0xA5EA
  if (level.mode >= LM_MATCHSETUP)
  {
    // if the game isn't running
    HM_ResetWeaponTimer();
    return;
  }
#endif
  // never count down weapontimer_hm,
  // if only a single weapon is enabled
  if ( hmLocals.numActiveWeapons == 1)
  {
    HM_ResetWeaponTimer();
    return;
  }

  HM_SendMsgEvent(EV_HITMEN_INFOMSG, level.hitmen.weaponTimer);
  if (level.hitmen.weaponTimer >= 1 && level.hitmen.weaponTimer <= 4)
  {
    HM_PlayGlobalSound("sound/misc/SoftFall.ogg");

    if (level.hitmen.weaponTimer == 2)
    {
      HM_PlayGlobalSound("sound/misc/Shutdown.ogg");
      Com_Printf("Disabling weapons ...\n");
	  HM_SetClientWeaponStates( WEAPON_HM_LOCK );
    }
    else if (level.hitmen.weaponTimer == 1)
    {
      hmLocals.nextWeapon = HM_NextWeapon();
      Com_Printf("changing weapons ...\n");
      HM_ChangeClientWeapons();
    }
	level.hitmen.weaponTimer--;
  }
  else if (level.hitmen.weaponTimer == 0)
  {
    HM_PlayGlobalSound("sound/misc/SoftFall.ogg");
    HM_ResetWeaponTimer();
    Com_Printf("enabling weapons ...\n");
    HM_SetClientWeaponStates( WEAPON_READY );
  }
  else
  {
    level.hitmen.weaponTimer--;
  }
}

/*
==================
HM_CheckHitmen
==================
*/
void HM_CheckHitmen(void)
{
  if (g_weaponmod.integer < WM_HITMEN)
    return;

  HM_Think();
}
