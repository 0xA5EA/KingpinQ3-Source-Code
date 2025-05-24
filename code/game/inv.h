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

///////////////////////////////////////////////
//bot inventory (match to inv.h in game folder)
///////////////////////////////////////////////

#define INVENTORY_NONE 0
//armor
#define INVENTORY_ARMOR_LEGS 1
#define INVENTORY_ARMOR_BODY 2
#define INVENTORY_ARMOR_HEAD 3
//weapons
#define INVENTORY_CROWBAR 4
#define INVENTORY_PISTOL 5
#define INVENTORY_SHOTGUN 6
#define INVENTORY_MACHINEGUN 7
#define INVENTORY_GRENADELAUNCHER 8
#define INVENTORY_ROCKETLAUNCHER 9
#define INVENTORY_HMG 10
#define INVERTORY_FLAMER 11
#define INVENTORY_GRAPPLINGHOOK 12
//ammo
#define INVENTORY_BULLETS 13
#define INVENTORY_SHELLS 14
#define INVENTORY_CYLINDER 15
#define INVENTORY_GRENADES 16
#define INVENTORY_ROCKETS 17
#define INVENTORY_308 18
#define INVENTORY_FLAMETANK 19
//weaponmods
#define INVENTORY_HMG_MOD_COOL 20
//#define INVENTORY_RL_MOD_FIRE 21
#define INVENTORY_PISTOL_MOD_DAMAGE 21 //"item_pistol_damage_mod"
#define INVENTORY_PISTOL_MOD_FIRE 22
#define INVENTORY_SILENCER_MOD 23
//powerups
#define INVENTORY_HEALTH 30
#define INVENTORY_TELEPORTER 31
#define INVENTORY_MEDKIT 32
#define INVENTORY_KAMIKAZE 33
#define INVENTORY_PORTAL 34
//#define INVENTORY_INVULNERABILITY
//#define INVENTORY_QUAD				35
#define INVENTORY_ENVIRONMENTSUIT 36
#define INVENTORY_HASTE 37
#define INVENTORY_INVISIBILITY 38
#define INVENTORY_REGEN 39
#define INVENTORY_FLIGHT 40
//#define INVENTORY_GUARD				42			// FIXME: 0xA5EA, verursacht fehler wenn nicht definiert, botfiles dateien aendern
#define INVENTORY_DRAGONFLAG 45
#define INVENTORY_NIKKIFLAG 46
#define INVENTORY_PLAYER_CASH 47	//hypov8 picked up cash
#define INVENTORY_BAG_CASH 48		//hypov8 stolen cash bags

#define INVENTORY_NEUTRALFLAG		49
//#define INVENTORY_DRAGONCUBE		50
//#define INVENTORY_NIKKICUBE			51
//enemy stuff
#define ENEMY_HORIZONTAL_DIST 200
#define ENEMY_HEIGHT 201
#define NUM_VISIBLE_ENEMIES 202
#define NUM_VISIBLE_TEAMMATES 203

// if running the mission pack
//#ifdef MISSIONPACK

//#error "running mission pack"

//#endif

//////////////
//item numbers 
// make sure they are in sync with bg_itemlist in bg_misc.c
#define MODELINDEX_ARMOR_HBODY 1
#define MODELINDEX_ARMORBODY 2
#define MODELINDEX_HEALTHSMALL 3
#define MODELINDEX_HEALTH 4
#define MODELINDEX_HEALTHMEGA 5

#define MODELINDEX_CROWBAR 6
#define MODELINDEX_PISTOL 7
#define MODELINDEX_SHOTGUN 8
#define MODELINDEX_MACHINEGUN 9
#define MODELINDEX_GRENADELAUNCHER 10
#define MODELINDEX_ROCKETLAUNCHER 11
#define MODELINDEX_HMG 12    //FIXME: abgleichen
#define MODELINDEX_FLAMER 13
//#define MODELINDEX_RAILGUN			11
#define MODELINDEX_GRAPPLINGHOOK 14

#define MODELINDEX_SHELLS 15
#define MODELINDEX_BULLETS 16
#define MODELINDEX_CYLINDER 17
#define MODELINDEX_GRENADES 18
#define MODELINDEX_ROCKETS 19
//#define MODELINDEX_SLUGS			17
#define MODELINDEX_308 20
#define MODELINDEX_FLAMETANK 21

#define MODELINDEX_TELEPORTER 22
#define MODELINDEX_MEDKIT 23

//#define MODELINDEX_ENVIRONMENTSUIT 24
//#define MODELINDEX_HASTE 25
//#define MODELINDEX_INVISIBILITY 26
//#define MODELINDEX_REGEN 27
//#define MODELINDEX_FLIGHT 28

#define MODELINDEX_HMG_MOD_COOLING 24
//#define MODELINDEX_RL_MOD_FIRE 25
#define MODELINDEX_PISTOL_MOD_DAMAGE 25 //"item_pistol_damage_mod"
#define MODELINDEX_PISTOL_MOD_FIRE 26
#define MODELINDEX_SILENCER 27

#define MODELINDEX_CASHROLL 28
#define MODELINDEX_CASHBAG 29
#define MODELINDEX_CASHBAG_STOLEN 30//add hypov8

//#define MODELINDEX_DRAGONSAFE 31
//#define MODELINDEX_NIKKISAFE 32

#define MODELINDEX_DRAGONFLAG 31
#define MODELINDEX_NIKKIFLAG 32
#define MODELINDEX_NEUTRALFLAG 33


/* 0xA5EA -------------------------------------------------------------------->  */
#define MODELINDEX_ARMOR_HELMET 34                     //"item_armor_helmet"
#define MODELINDEX_ARMOR_HHELMET 35                    //"item_armor_helmet_heavy"
#define MODELINDEX_ARMOR_LEGS 36                       //"item_armor_legs"
#define MODELINDEX_ARMOR_HLEGS 37                      //"item_armor_legs_heavy"
/* <--------------------------------------------------------------------  0xA5EA  */

//
//todo: pipe?
#define WEAPONINDEX_CROWBAR 1
#define WEAPONINDEX_PISTOL 2
#define WEAPONINDEX_SHOTGUN 3
#define WEAPONINDEX_MACHINEGUN 4
#define WEAPONINDEX_GRENADE_LAUNCHER 5
#define WEAPONINDEX_ROCKET_LAUNCHER 6
//#define WEAPONINDEX_RAILGUN					6
#define WEAPONINDEX_HMG 7
#define WEAPONINDEX_FLAMER 8
#define WEAPONINDEX_GRAPPLING_HOOK 9
