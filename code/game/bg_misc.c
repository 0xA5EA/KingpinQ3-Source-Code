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
// bg_misc.c -- both games misc functions, all completely stateless

#include "../qcommon/q_shared.h"
#include "bg_public.h"

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait" override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t bg_itemlist[] =
{
  {
    NULL, /* classname; */
    NULL, /* pickup_sound;*/
	{       /* world_model[5] */
		0,    /* "v_wep.md3", */     /* WORLD_GUNMODEL_POS */          // first person view wep
		0,    /* "v_hand.md3", */    /* WORLD_HANDMODEL_POS */         // first person view hand
		0,    /* "flash.md3", */     /* WORLD_FLASHMODEL_POS */        // first person+3rd person flash model sprite
		0,    /* "w_player.md3", */  /* WORLD_PLAYERWEAPONMODEL_POS */ // world player weaponmodel
		0     /* "w_map.md3" */      /* WORLD_WEAPONMODEL_POS */       // world map model
	},
    NULL,   /* icon */ 
    NULL,   /* pickup_name */ 
    0,      /* quantity */
    IT_BAD, /* giType */
    0,      /* giTag */
    "",     /* precache */ 
    ""      /* sounds */ 
  },                                               // leave index 0 alone

  /*QUAKED item_armor_body_heavy (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   #1 */
  {
    "item_armor_jacket_heavy",
    "sound/world/pickups/generic.ogg",           /* 0xA5EA */
    {
      "models/pu_icon/armor/armorhdtop.md3",
      0, 0, 0,
      "models/pu_icon/armor/armorhdtop.md3"
    },
    "gfx/icons/h_chest_hvy",  // icon
    "Heavy Jacket Armor", // pickup
    100,
    IT_ARMOR,     // giType
    AR_BODY,      // giTag
    "",           // precache
    ""            // sounds
  },

  /*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    2 */
  {
    "item_armor_jacket",
    "sound/world/pickups/generic.ogg",
    {
      "models/pu_icon/armor/armor_vest.md3",
      0, 0, 0,
      "models/pu_icon/armor/armor_vest.md3"
    },
    "gfx/icons/h_chest_lt", // icon
    "Jacket Armor",     // pickup name
    33,
    IT_ARMOR,           // giType
    AR_BODY,            // giTag
    "",                 // precache
    ""                  // sounds
  },

  /*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    3  */
  {
    "item_health_sm",
    "sound/world/pickups/health.ogg",
    {
      "models/pu_icon/health_s/tris.md3",
      0, 0, 0,
      "models/pu_icon/health_s/tris.md3"
    },
   "gfx/icons/h_smed",       // icon
    "10 Health",         // pickup name
    10,                  // quantity
    IT_HEALTH,           // giType
    0,                   // giTag
    "",                  // precache
    ""                   // sounds
  },

  /*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     4  */
  {
    "item_health_lg",
    "sound/world/pickups/health.ogg",
    {
      "models/pu_icon/health/tris.md3",
      0, 0, 0,
      "models/pu_icon/health/tris.md3"
    },
    "gfx/icons/h_lmed",     // icon
    "25 Health",        // pickup name
    25,                 // quantity
    IT_HEALTH,          // giType
    0,                  // giTag
    "",                 // precache
    ""                  // sounds
  },
//hypov8: not loading? also add reload mod?
  /*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    5  */
  {
    "item_health_mega", 
    "sound/world/pickups/health.ogg",            /* 0xA5EA */
    {
      "models/pu_icon/adrenaline/tris.md3",
      0, 0, 0,
      "models/pu_icon/adrenaline/tris.md3"
    },
    "gfx/icons/h_pmed",       // icon
    "Mega Health",        // pickup name
    100,                  // quantity
    IT_HEALTH,            // giType
    0,                    // giTag
    "",                   // precache
    ""                    // sounds
  },
//hypov8: todo item_pack

  /*QUAKED weapon_crowbar (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   6  */
  {
    "weapon_crowbar",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/crowbar/v_wep.md5mesh", 0,
      0,
      "models/weapons/crowbar/w_player.md3",
      "models/weapons/crowbar/w_map.md3"
    },
    "gfx/icons/h_cbar",         // icon
    "Crowbar",              // pickup name
    0,                      // quantity
    IT_WEAPON,              // giType
    WP_CROWBAR,             // giTag
    "",                     // precache
    ""                      // sounds
  },
  /*QUAKED weapon_pistol (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   #7  */
  {
    "weapon_pistol",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/colt/v_wep.md3",			// first person view wep
      "models/weapons/colt/v_hand.md3", 		// first person view hand
      "models/weapons/colt/flash.md3",			// first person+3rd person flash model sprite
      "models/weapons/colt/w_player.md3",	// world player weapon model
      "models/weapons/colt/w_map.md3"			// world map model
      //FIXME (0xA5EA): use world pistol model
    },
    "gfx/icons/h_pistol_mag",    // icon
    "Pistol",                // pickup name
    10,                     // quantity
    IT_WEAPON,               // giType
    WP_PISTOL,               // giTag
    "",                      // precache
    ""                       // sounds
  },

  /*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   8 */
  {
    "weapon_shotgun",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/shotgun/v_wep.md3",		// first person view wep
      "models/weapons/shotgun/v_hand.md3",	// first person view hand
      "models/weapons/shotgun/flash.md3",		// first person+3rd person flash model sprite
      "models/weapons/shotgun/w_player.md3",	// world player weaponmodel
      "models/weapons/shotgun/w_map.md3"		// world map model 
    },
    "gfx/icons/h_shotgun",        // icon
    "Shotgun",                // pickup name
    2,                       // quantity
    IT_WEAPON,                // giType
    WP_SHOTGUN,               // giTag
    "",                       // precache
    ""                        // sounds
  },

  /*QUAKED weapon_tommygun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   9  */
  {
    "weapon_tommygun",							// classname
    "sound/misc/w_pkup.ogg",					// pickup_sound
    {
      "models/weapons/tomgun/v_wep.md5mesh",// first person view wep
      0,                                    // first person view hand
      "models/weapons/tomgun/flash.md3",    // first person+3rd person flash model sprite
      "models/weapons/tomgun/w_wep.md3",    // world player weaponmodel
      "models/weapons/tomgun/w_wep.md3"     // world map model
    },
    "gfx/icons/h_tgun",			// icon
    "Machinegun",				// pickup name
    0,							// quantity
    IT_WEAPON,					// giType
    WP_MACHINEGUN,				// giTag
    "",							// precache
    ""							// sounds
  },

  /*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   10  */
  {
    "weapon_grenadelauncher",
    "sound/misc/w_pkup.ogg",
    {

      "models/weapons/grenadelauncher/v_wep.md3",       // first person view wep
      "models/weapons/grenadelauncher/v_hand.md3",      // first person view hand
      "models/weapons/grenadelauncher/flash.md3",       // first person+3rd person flash model sprite
      "models/weapons/grenadelauncher/w_player.md3",    // world player weaponmodel
      "models/weapons/grenadelauncher/w_map.md3"        // world map model 
    },
    "gfx/icons/h_grenade_l",      // icon
    "Grenade Launcher",       // pickup name
    0,                       // quantity
    IT_WEAPON,                // giType
    WP_GRENADE_LAUNCHER,      // giTag
    "",                       // precache
    ""                        // sounds
  },

  /*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    11  */
  {
    "weapon_bazooka",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/rocketlauncher/v_wep.md5mesh",
      0,
      "models/weapons/rocketlauncher/flash.md3",
      "models/weapons/rocketlauncher/w_player.md3",  // world player weaponmodel
      "models/weapons/rocketlauncher/w_map.md3"      // world spawn weaponmodel
    },
    "gfx/icons/h_bazooka",      // icon
    "Rocket Launcher",      // pickup name
    0,                     // quantity
    IT_WEAPON,              // giType
    WP_ROCKET_LAUNCHER,     // giTag
    "",                     // precache
    ""                      // sounds
  },

  /*QUAKED weapon_heavymachinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     12  */
  {
    "weapon_heavymachinegun",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/hmg/v_wep.md5mesh",
      0,
      "models/weapons/hmg/flash.md3",
      "models/weapons/hmg/w_player.md3",        // world player weaponmodel
      "models/weapons/hmg/w_map.md3"               // world spawn weaponmodel
      //FIXME (0xA5EA): add model to data
    },
    "gfx/icons/iconw_hmg",       // icon
    "Hmg",                   // pickup name
    0,                      // quantity
    IT_WEAPON,               // giType
    WP_HMG,                  // giTag
    "",                      // precache
    ""                       // sounds
  },
#ifdef USE_FLAMEGUN
  /*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     13  */
  {
    "weapon_flamethrower",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/flamegun/v_wep.md3",     // first person view wep
      "models/weapons/flamegun/v_hand.md3",    // first person view hand
      "models/weapons/flamegun/flash.md3",     // first person+3rd person flash model sprite
      "models/weapons/flamegun/w_player.md3",  // world player weaponmodel
      "models/weapons/flamegun/w_map.md3"      // world map model 
    },
    "gfx/icons/h_flamethrower",    // icon
    "Flamegun",                // pickup name
    0,                        // quantity
    IT_WEAPON,                 // giType
    WP_FLAMER,                 // giTag
    "",                        // precache
    ""                         // sounds
  },
#endif
  /*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   14  */
  {
    "weapon_grapplinghook",
    "sound/misc/w_pkup.ogg",
    {
      "models/weapons/grapple/v_wep.md3",		// first person view wep
      "models/weapons/grapple/v_hand.md3",		// first person view hand
      "models/weapons/grapple/flash.md3",		// first person+3rd person flash model sprite
      "models/weapons/grapple/w_player.md3",	// world player weaponmodel
      "models/weapons/grapple/w_map.md3"		// world map model 
    },
    "gfx/icons/iconw_grapple",  // icon
    "Grappling Hook",           // pickup name
    0,                          // quantity
    IT_WEAPON,                  // giType
    WP_GRAPPLING_HOOK,          // giTag
    "",                         // precache
    ""                          // sounds
  },

  /*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     15  */
  {
    "ammo_shells",
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/shotgun_shell/tris.md3",
      0, 0,
      "models/pu_icon/shotgun_shell/tris.md3",
      "models/pu_icon/shotgun_shell/tris.md3"
    },
    "gfx/icons/h_shots",    // icon
    "Shells",               // pickup name
    10,                     // quantity
    IT_AMMO,                // giType
    WP_SHOTGUN,             // giTag
    "",                     // precache
    ""                      // sounds
  },

  /*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     16  */
  {
    "ammo_bullets",
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/pclip/tris.md3",
      0, 0, 0,
      "models/pu_icon/pclip/tris.md3"
    },
    "gfx/icons/h_clip",          // icon
    "Bullets",               // pickup name
    10,                      // quantity
    IT_AMMO,                 // giType
    WP_MACHINEGUN,           // giTag
    "",                      // precache
    ""                       // sounds
  },

  /*QUAKED ammo_cylinder (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    17  */
  {
    "ammo_cylinder",
    "sound/misc/w_pkup.ogg",
    {
	  "models/pu_icon/tgclip/tris.md3",
      0, 0, 0,
      "models/pu_icon/tgclip/tris.md3"
    }, 
    /* icon */ "gfx/icons/h_tclip",
    /* pickup */ "Cylinder",
    50,
    IT_AMMO,
    WP_MACHINEGUN,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     18  */
  {
    "ammo_grenades",
    "sound/misc/w_pkup.ogg",  //"sound/misc/w_pkup.ogg",
    {                                //"models/powerups/ammo/grenadeam.md3",
      "models/pu_icon/grenade/tris.md3",
      0, 0, 0,
      "models/pu_icon/grenade/tris.md3"
    },
    "gfx/icons/h_grenade",	/* icon */             //icona_grenade",
    "Grenades",				/* pickup */ 
    3,						/* quantity */					
    IT_AMMO,				/* giType */
    WP_GRENADE_LAUNCHER,	/* giTag */
    "",						/* precache */
    ""						/* sounds */
  },

  /*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     19 */
  {
    "ammo_rockets",
    "sound/misc/w_pkup.ogg",
    //{ "models/powerups/ammo/rocketam.md3",
    {
      "models/pu_icon/rocket/tris.md3",
      0, 0, 0,
      "models/pu_icon/rocket/tris.md3"
    },
    /* icon */ "gfx/icons/h_missle",                     //icona_rocket
    /* pickup */ "Rockets",
    5,
    IT_AMMO,
    WP_ROCKET_LAUNCHER,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED ammo_308 (.3 .3 1) (-16 -16 -16) (16 16 16) suspended     20  */
  {
    "ammo_308",
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/hmgclip/tris.md3",
      0, 0, 0,
      "models/pu_icon/hmgclip/tris.md3"
    },
    /* icon */ "gfx/icons/h_hmg_clip",
    /* pickup */ "308",
    30,
    IT_AMMO,
    WP_HMG,
    /* precache */ "",
    /* sounds */ ""
  },
#ifdef USE_FLAMEGUN
  /*QUAKED ammo_flametank (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   21  */
  {
    "ammo_flametank",
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/exting/ext2.md3",
      0, 0, 0,
      "models/pu_icon/exting/ext2.md3"
    },
    /* icon */ "gfx/icons/h_ftank",
    /* pickup */ "Flametank",
    50,
    IT_AMMO,
    WP_FLAMER,
    /* precache */ "",
    /* sounds */ ""
  },
#endif
  //
  // HOLDABLE ITEMS
  //
  /*QUAKED holdable_teleporter (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   22  */
  {
    "holdable_teleporter", //hypov8 todo: use??
    "sound/items/holdable.ogg",
    {
      "models/powerups/holdable/teleporter.md3",
      0, 0, 0,
      "models/powerups/holdable/teleporter.md3"
    },
    /* icon */ "gfx/icons/teleporter",
    /* pickup */ "Personal Teleporter",
    60,
    IT_HOLDABLE,
    HI_TELEPORTER,
    /* precache */ "",
    /* sounds */ ""
  },
  /*QUAKED holdable_medkit (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    23  */
  {
    "holdable_medkit", //hypov8 dodo: use??
    "sound/items/holdable.ogg",
    {
      "models/powerups/holdable/medkit.md3",
      "models/powerups/holdable/medkit_sphere.md3",
      0, 0,
      "models/powerups/holdable/medkit.md3"
    },
    /* icon         "gfx/icons/medkit",*/
    "gfx/icons/h_pmed",
    /* pickup */ "Medkit",
    60,
    IT_HOLDABLE,
    HI_MEDKIT,
    /* precache */ "",
    /* sounds */ "sound/items/use_medkit.ogg"
  },

  //
  //
  //
  /*QUAKED item_hmg_mod_cooling (.3 .3 1) (-16 -16 -16) (16 16 16) suspended  #24  */
  {
    "item_hmg_mod_cooling",
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/coolmod/tris.md3",
      0, 0, 0,
      "models/pu_icon/coolmod/tris.md3"
    },
    /* icon */ "gfx/icons/h_heavymachinegun_cooling",
    /* pickup */ "Hmg cooling mod",
    1,
    IT_WEAPMOD,
    PW_WPMOD_COOLING,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED item_pistol_damage_mod (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    	#25  */
  {
    "item_pistol_damage_mod",
    "sound/misc/w_pkup.ogg",
    {
        "models/pu_icon/ro_fire_mod/tris.md3",
      0, 0, 0,
      "models/pu_icon/ro_fire_mod/tris.md3"
    },
    /* icon */ "gfx/icons/h_ecd",
    /* pickup */ "Magnum fire mod",
    1,
    IT_WEAPMOD,
    PW_WPMOD_PISTOLMAGNUM,
    /* precache */ "",
    /* sounds */ ""
  },

#if 1 // hypov8 do we need?

  /*QUAKED item_pistol_fire_mod (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    #26  */
  {
			"item_pistol_fire_mod",
			"sound/misc/w_pkup.ogg",
			{
				"models/pu_icon/ro_fire_mod/tris.md3",
				0, 0, 0,
				"models/pu_icon/ro_fire_mod/tris.md3"
			},
			/* icon */ "gfx/icons/h_ecd",
			/* pickup */ "Magnum fire mod",
			1,
			IT_WEAPMOD,
			PW_WPMOD_PISTOLDOUBLEFIRE,
			/* precache */ "",
			/* sounds */ ""
		},
 
#endif

  /*QUAKED "item_silencer_mod" (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    #27  */
  {
    "item_silencer_mod", //hypov8 todo: this add a spistol/v_wep
    "sound/misc/w_pkup.ogg",
    {
      "models/pu_icon/silencer/tris.md3",
      0, 0, 0,
      "models/pu_icon/silencer/tris.md3"
    },
    /* icon */ "gfx/icons/h_silencer",
    /* pickup */ "Silencer mod",
    1,
    IT_WEAPMOD,
    PW_WPMOD_SILENCER,
    /* precache */ "",
    /* sounds */ ""
  },
#ifdef WITH_BAGMAN_MOD
  /*QUAKED item_cashroll (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    28  */
  {
    "item_cashroll",
    "sound/world/pickups/cash.ogg",
    {
      "models/qpin_gameobj/cash.md3",
      0, 0, 0,
      "models/qpin_gameobj/cash.md3"
    },
    "gfx/icons/h_money",	/* icon */ 
    ITEMNAME_CASHROLL,		/* pickup */ 
    CASHROLL_AMONG,			/* giType */
    IT_CASH,				/* giTag */
    0,						/* giTag */
    "",						/* precache */ 
    ""						/* sounds */ 
  },

  /*QUAKED item_cashbag (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    29  */
  {
    "item_cashbag",
    "sound/world/pickups/cash.ogg",
    {
      "models/qpin_gameobj/moneybag.md3",
      0, 0, 0,
      "models/qpin_gameobj/moneybag.md3"
    },
    "gfx/icons/h_money",	/* icon */ 
    ITEMNAME_CASHBAG,		/* pickup name*/ 
    CASHBAG_AMONG,			/* quantity */
	IT_CASH,				/* giType */
	0,						/* giTag */
    "",						/* precache */ 
    ""						/* sounds */ 
  },

  /*QUAKED item_cashbag_stolen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    #30  */
  {
	  "item_cashbag_stolen",
	  "sound/world/pickups/cash.ogg",
	  {
		  "models/qpin_gameobj/moneybag.md3",
		  0, 0, 0,
		  "models/qpin_gameobj/moneybag.md3"
	  },
	  "gfx/icons/h_money",		/* icon */ 
	  ITEMNAME_CASHBAG_STOLEN,	/* pickup name */ 
	  CASHBAG_AMONG,			/* quantity */
	  IT_CASH_STOLEN,			/* giType */
	  PW_STOLENSCASH,			/* giTag */
	  "",						/* precache */ 
	  ""						/* sounds */ 
  },
#if 0 //hypov8 note: updated bot files/indexes instead
	/*QUAKED team_bm_dragonssafe (1 0 0) (-16 -16 -16) (16 16 16)      #00 */
	//SP_team_BM_Nikkissafe
	{
		"team_bm_dragonssafe",
		NULL,
		{
			"models/qpin_gameobj/moneybag.md3",
			0, 0, 0, 0
		},
		"gfx/icons/iconf_red1",	/* icon */ 
		ITEMNAME_DRAGON_SAFE,	/* pickup */ 
		0,						/* quantity */ 
		IT_TEAM_SAFE,			/* giType */ 
		TM_DRAGONSAFE,			/* giTag */	
		"",						/* precache */ 
		""						/* sounds */ 
	},

	/*QUAKED team_bm_nikkissafe (0 0 1) (-16 -16 -16) (16 16 16)      #00  */
	{
		  "team_bm_nikkissafe",
		  NULL,
		  {
			  "models/qpin_gameobj/moneybag.md3",
			  0, 0, 0, 0
		  },
		  "gfx/icons/iconf_blu1",	/* icon */ 
		  ITEMNAME_NIKKIS_SAFE,		/* pickup */ 
		  0,						/* quantity */
		  IT_TEAM_SAFE,				/* giType */ 
		  TM_NIKKISAFE,				/* giTag */	
		  "",						/* precache */ 
		  ""						/* sounds */ 
	},
#endif
#endif // WITH_BAGMAN_MOD

  /*QUAKED team_ctf_dragonsflag (1 0 0) (-16 -16 -16) (16 16 16)      31 */
  {
    "team_ctf_dragonsflag",
    NULL,
    {
      "models/flags/drag_flag.md3",
      0, 0, 0,
      "models/flags/drag_flag.md3"
    },
    /* icon */ "gfx/icons/iconf_red",
    /* pickup */ ITEMNAME_DRAGON_FLAG,
    0,
    IT_TEAM,
    PW_DRAGONFLAG,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED team_ctf_nikkisflag (0 0 1) (-16 -16 -16) (16 16 16)      32  */
  {
    "team_ctf_nikkisflag",
    NULL,
    {
      "models/flags/nikki_flag.md3",
      0, 0, 0,
      "models/flags/nikki_flag.md3"
    },
    /* icon */ "gfx/icons/iconf_blu",
    ITEMNAME_NIKKIS_FLAG,				/* pickup name */ 
    0,
    IT_TEAM,
    PW_NIKKIFLAG,
     "",								/* precache */
    ""									/* sounds */ 
  },

  /*QUAKED team_ctf_neutralflag (0 0 1) (-16 -16 -16) (16 16 16) suspended   #33 */
  {
    "team_ctf_neutralflag",
    NULL,
    {
      "models/flags/n_flag.md3",
      0, 0, 0,
      "models/flags/n_flag.md3"
    },
    /* icon */ "gfx/icons/iconf_neut",
    /* pickup */ ITEMNAME_NEUTRAL_FLAG,
    0,
    IT_TEAM,
    PW_NEUTRALFLAG,
    /* precache */ "",
    /* sounds */ ""
  },


  /*QUAKED item_armor_helmet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    34  */
  {
    "item_armor_helmet",
    "sound/world/pickups/generic.ogg",
    {
      "models/pu_icon/armor_head/armor_head.md3",
      0, 0, 0,
      "models/pu_icon/armor_head/armor_head.md3"
    },
    /* icon */ "gfx/icons/h_helmet_lt",
    /* pickup */ "Helmet Armor",
    33,
    IT_ARMOR,
    AR_HEAD,
    /* precache */ "",
    /* sounds */ ""
  },


  /*QUAKED item_armor_helmet_heavy (.3 .3 1) (-16 -16 -16) (16 16 16) suspended  35  */
  {
    "item_armor_helmet_heavy",
    "sound/world/pickups/generic.ogg",
    {
      "models/pu_icon/armor_head/armor_head_hd.md3",
      0, 0, 0,
      "models/pu_icon/armor_head/armor_head_hd.md3"
    },
    /* icon */ "gfx/icons/h_helmet_hvy",
    /* pickup */ "Helmet Armor Heavy",
    100,
    IT_ARMOR,
    AR_HEAD,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED item_armor_legs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended    36  */
  {
    "item_armor_legs",
    "sound/world/pickups/generic.ogg",
    {
      "models/pu_icon/armor_lo/armor_lo.md3",
      0, 0, 0,
      "models/pu_icon/armor_lo/armor_lo.md3"
    },
    /* icon */ "gfx/icons/h_pants_lt",
    /* pickup */ "Legs Armor",
    33,
    IT_ARMOR,
    AR_LEGS,
    /* precache */ "",
    /* sounds */ ""
  },

  /*QUAKED item_armor_legs_heavy (.3 .3 1) (-16 -16 -16) (16 16 16) suspended   37 */
  {
    "item_armor_legs_heavy",
    "sound/world/pickups/generic.ogg",
    {
      "models/pu_icon/armor_lo/armor_lo_hd.md3",
      0, 0, 0,
      "models/pu_icon/armor_lo/armor_lo_hd.md3"
    },
	"gfx/icons/h_pants_hvy",	/* icon */ 
	"Legs Armor Heavy",			/* pickup */ 
    100,						/* quantity */
    IT_ARMOR,					/* giType */
    AR_LEGS,					/* giTag */
    "",							/* precache */ 
    ""							/* sounds */
  },
  // end of list marker

  {
    NULL,
    NULL,
    {
      0, 0, 0, 0, 0
    },
    /* icon */ 0,
    /* pickup */ 0,
    0,
    IT_BAD,
    AR_NONE,
    /* precache */ 0,
    /* sounds */ 0
  },
};

//item numbers (make sure they are in sync with defines in inv.h)
int bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;

char const *bg_customTauntNames1[] = 
{
  "sound/taunt/fart/farts.ogg",
  "sound/taunt/fart/furz001.ogg",
  "sound/taunt/fart/furz006.ogg",
  "sound/taunt/fart/furz008.ogg",
  "sound/taunt/fart/furz009.ogg",
  "sound/taunt/fart/furz010.ogg",
  "sound/taunt/fart/furz011.ogg",
  "sound/taunt/fart/furz013.ogg",
  "sound/taunt/fart/furz015.ogg",
  "sound/taunt/fart/furz016.ogg",
  "sound/taunt/fart/furz020.ogg",
  "sound/taunt/fart/furz022.ogg",
  "sound/taunt/fart/furz023.ogg",
  "sound/taunt/fart/furz025.ogg",
  "sound/taunt/fart/furz027.ogg",
  "sound/taunt/fart/furz028.ogg",
  "sound/taunt/fart/furz029.ogg",
  "sound/taunt/fart/furz030.ogg",
  "sound/taunt/fart/furz031.ogg",
  "sound/taunt/fart/furz037.ogg",
  "sound/taunt/fart/furz038.ogg",
  "sound/taunt/fart/furz039.ogg",
  "sound/taunt/fart/furz040.ogg",
  "sound/taunt/fart/furz046.ogg",
  "sound/taunt/fart/furz051.ogg",
  "sound/taunt/fart/furz052.ogg",
  "sound/taunt/fart/furz057.ogg",
  "sound/taunt/fart/furz059.ogg",
  "sound/taunt/fart/furz060.ogg",
  "sound/taunt/fart/furz061.ogg",
  "sound/taunt/fart/furz062.ogg",
  "sound/taunt/fart/furz063.ogg",
  "sound/taunt/fart/furz064.ogg",
  "sound/taunt/fart/furz067.ogg",
  "sound/taunt/fart/furz069.ogg",
  "sound/taunt/fart/furz073.ogg",
  "sound/taunt/fart/furz076.ogg",
  "sound/taunt/fart/furz079.ogg",
  "sound/taunt/fart/furz081.ogg",
  0
};
const int numCustomTaunts1 = sizeof(bg_customTauntNames1) / sizeof(bg_customTauntNames1[0]) - 1;

char const *bg_customTauntNames2[] = {
  "sound/taunt/burp/barnybrp.ogg",
  //	"sound/taunt/burp/big_belch.ogg",		// format doesnt fit
  "sound/taunt/burp/bu12.ogg",
  //	"sound/taunt/burp/burp.ogg",			// format doesnt fit
  //	"sound/taunt/burp/burp2.ogg",			// format doesnt fit
  "sound/taunt/burp/burp4.ogg",
  "sound/taunt/burp/burp5.ogg",
  "sound/taunt/burp/burp_1.ogg",
  //	"sound/taunt/burp/burp_2.ogg",			// format doesnt fit
  //	"sound/taunt/burp/burp_3.ogg",
  "sound/taunt/burp/darden.ogg",
  //	"sound/taunt/burp/f4.ogg",				// format doesnt fit
  //	"sound/taunt/burp/jkburp.ogg",			// format doesnt fit
  //	"sound/taunt/burp/kaboom.ogg",
  "sound/taunt/burp/ruelps.ogg",
  0
};
const int numCustomTaunts2 = sizeof(bg_customTauntNames2) / sizeof(bg_customTauntNames2[0]) - 1;

/*
 ==============
 BG_FindItemForPowerup
 ==============
 */
gitem_t *BG_FindItemForPowerup(powerup_t pw)
{
  int i;

  for (i = 0; i < bg_numItems; i++)
  {
    if ((bg_itemlist[i].giType == IT_POWERUP
		|| bg_itemlist[i].giType == IT_TEAM
		|| bg_itemlist[i].giType == IT_PERSISTANT_POWERUP
		|| bg_itemlist[i].giType == IT_CASH_STOLEN)
		&& bg_itemlist[i].giTag == pw)
    {
      return &bg_itemlist[i];
    }
  }

  return NULL;
}

/*
 ==============
 BG_FindItemForHoldable
 ==============
 */
gitem_t *BG_FindItemForHoldable(holdable_t pw)
{
  int i;

  for (i = 0; i < bg_numItems; i++)
  {
    if (bg_itemlist[i].giType == IT_HOLDABLE && (holdable_t) bg_itemlist[i].giTag == pw)
      return &bg_itemlist[i];
  }

  Com_Error(ERR_DROP, "HoldableItem not found");

  return NULL;
}

/*
 ===============
 BG_FindItemForWeapon

 ===============
 */
gitem_t *BG_FindItemForWeapon(weapon_t weapon)
{
  gitem_t *it;

  for (it = bg_itemlist + 1; it->classname; it++)
  {
    if (it->giType == IT_WEAPON && it->giTag == weapon)
      return it;
  }

Com_Error(ERR_DROP, "Couldn't find item for weapon %i", weapon);
return NULL;
}

/*
 ===============
 BG_FindItem

 ===============
 */
gitem_t *BG_FindItem(const char *pickupName)
{
	gitem_t *it;

	for ( it = bg_itemlist + 1; it->classname; it++ )
	{
		if ( !Q_stricmp(it->pickup_name, pickupName) )
			return it;
	}

	return NULL;
}

/*
 ============
 BG_PlayerTouchesItem

 Items can be picked up without actually touching their physical bounds to make
 grabbing them easier
 ============
 */
qboolean BG_PlayerTouchesItem(playerState_t *ps, entityState_t *item, int atTime)
{
	vec3_t origin;

	BG_EvaluateTrajectory(&item->pos, atTime, origin);

	// add hypov8 duck
	if ( ps->pm_flags & PMF_DUCKED )
	{
		//edit hypov8 16 bounding box and compensate player offset
		if ( ps->origin[ 0 ] - origin[ 0 ] > 32 || ps->origin[ 0 ] - origin[ 0 ] < -32 ||
			ps->origin[ 1 ] - origin[ 1 ] > 32 || ps->origin[ 1 ] - origin[ 1 ] < -32 ||
			ps->origin[ 2 ] - origin[ 2 ] > 40 || ps->origin[ 2 ] - origin[ 2 ] < -40 ) /* hypov8 take into acount 24 units offset */
		{
			return qfalse;
		}
	}
	else
	{
		//edit hypov8 16 bounding box and compensate player offset
		if ( ps->origin[ 0 ] - origin[ 0 ] > 32 || ps->origin[ 0 ] - origin[ 0 ] < -32 ||
			ps->origin[ 1 ] - origin[ 1 ] > 32 || ps->origin[ 1 ] - origin[ 1 ] < -32 ||
			ps->origin[ 2 ] - origin[ 2 ] > 40 || ps->origin[ 2 ] - origin[ 2 ] < -64 ) /* hypov8 take into acount 24 units offset */
		{
			return qfalse;
		}

	}

	return qtrue;
}



/*
=================
BG_PlayerCanChangeWeapon

can't change if weapon is firing and counter > 0
can change again if lowering or raising or reloading
=================
*/
qboolean BG_PlayerCanChangeWeapon(playerState_t *ps) //unvan .52
{
	if (ps->weapon == WP_SHOTGUN && ps->weaponstate == WEAPON_RELOAD_MOD)
	{
		return qtrue;
	}

	if ( ps->weaponstate == WEAPON_RELOADING
		|| ps->weaponstate == WEAPON_RELOAD_MOD
		//|| ps->weaponstate == WEAPON_FIRING
		|| ps->weaponstate == WEAPON_DROPPING 
		|| ps->weaponstate == WEAPON_RAISING
		|| ps->weaponTime > 0 )
		return qfalse;

	return qtrue;
}

/*
========================
BG_InventoryContainsWeapon

Does the player hold a weapon?
========================
*/
qboolean BG_InventoryContainsWeapon(int weapon, const int stats[ ]) //unvan .52
{
	if ( weapon == WP_CROWBAR )
		return qtrue;

	if ( !( stats[ STAT_WEAPONS ] & (1 << weapon)) )
		return qfalse;

	return qtrue;
}

qboolean BG_InventoryContainsAmmo(int weapon, playerState_t *ps)
{
	if ( weapon <= WP_CROWBAR  || weapon >  WP_LAST) //infinate?
	{
		return qtrue;
	}

	if ( ps->ammo_mag[ weapon ] <= 0 && ps->ammo_all[ BG_AmmoCombineCheck(weapon) ] <= 0 )
	{
		return qfalse;
	}

	return qtrue;
}
/*
bool BG_SetWeaponState(int weapon, playerState_t *ps)
{
	
	if ( weapon <= WP_CROWBAR  || weapon >  WP_LAST)
	{
		return true;
	}

	if ( ps->ammo_mag[ weapon ] <= 0 && ps->ammo_all[ BG_AmmoCombineCheck(weapon) ] <= 0 )
	{
		return false;
	}

	return true;
}
*/


#ifdef WITH_BAGMAN_MOD
/*
==============
BG_CanPickupCash
==============
*/
qboolean BG_CanPickupCash(int maxcash, const playerState_t *ps)
{
	if (ps->stats[STAT_CASH] < maxcash)
		return qtrue;

	return qfalse;
	//FIXME(0xA5EA): take ammount of cashitmen into account
}

/*
==============
BG_CanPickupCashStolen
add hypov8
==============
*/
qboolean BG_CanPickupCashStolen(int maxcash, const playerState_t *ps)
{
	if (ps->stats[STAT_STOLEN_CASH] < maxcash)
		return qtrue;

	return qfalse;
	//FIXME(0xA5EA): take ammount of cashitmen into account
}
#endif // WITH_BAGMAN_MOD

/*
 ================
 BG_CanItemBeGrabbed

 Returns false if the item should not be picked up.
 This needs to be the same for client side prediction and server use.
 ================
 */
qboolean BG_CanItemBeGrabbed(int gametype, const entityState_t *ent, const playerState_t *ps,
								int cashcollectmax, int cashstolenmax) //hypov8 add cash
{
  gitem_t *item;
  //int weapon, dropedWep2;

#if 0
#ifdef MISSIONPACK
  int upperBound;
#endif
#endif
 /// bg_numItems;

  if (ent->modelindex < 1 || ent->modelindex >= bg_numItems)
  {
    Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: index out of range");
  }

  item = &bg_itemlist[ent->modelindex];

  switch (item->giType)
  {
  case IT_WEAPON:
	// droped items have flag bounce set, only way i can think to find if its droped *cg* side
	if (ent->modelindex2 || (ent->eFlags & EF_BOUNCE_HALF))
	{	//weapon droped, so add the ammo if its not maxed out
		int ammoIndex = BG_AmmoCombineCheck(item->giTag);
		if (ps->ammo_all[ammoIndex] < BG_WeaponMaxAmmoCount(ammoIndex) )
			return qtrue;
	}
	else
	{
		if (!(ps->stats[STAT_WEAPONS] & (1 << item->giTag)))
			return qtrue;
	}
    return qfalse;	// must be full!!

  case IT_AMMO:
	  if (ps->ammo_all[BG_AmmoCombineCheck(item->giTag)] >= BG_WeaponMaxAmmoCount(BG_AmmoCombineCheck(item->giTag)))
      return qfalse; // can't hold any more

    return qtrue;

  case IT_WEAPMOD:
    if (ps->stats[STAT_WEAP_MODS] & (1 << item->giTag))
      return qfalse;

    return qtrue;

  case IT_ARMOR:
    //if ( ps->stats[STAT_ARMOR] >= ps->stats[STAT_MAX_HEALTH] * 2 ) {
    if (ps->stats[item->giTag] >= MAX_ARMOR)
      return qfalse;
    return qtrue;

  case IT_HEALTH:
    if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
      return qfalse;

    return qtrue;

  case IT_POWERUP:
    return qtrue; // powerups are always picked up

  case IT_PERSISTANT_POWERUP:
	// can only hold one item at a time
	if (ps->stats[STAT_PERSISTANT_POWERUP])
		return qfalse;

	// check team only
	if ((ent->generic1 & 2) && (ps->persistant[PERS_TEAM] != TEAM_DRAGONS))
		return qfalse;

	if ((ent->generic1 & 4) && (ps->persistant[PERS_TEAM] != TEAM_NIKKIS))
		return qfalse;

    return qtrue; //hypov8 note: team can picup 

  case IT_TEAM: // team items, such as flags
	if (gametype == GT_1FCTF)
    {
		// neutral flag can always be picked up
		if (item->giTag == PW_NEUTRALFLAG)
			return qtrue;
		if (ps->persistant[PERS_TEAM] == TEAM_DRAGONS) 
		{
			//touch enemy base?
			if (item->giTag == PW_NIKKIFLAG  && ps->powerups[PW_NEUTRALFLAG] ) 
				return qtrue;
		} 
		else if (ps->persistant[PERS_TEAM] == TEAM_NIKKIS) 
		{
			//touch enemy base?
			if (item->giTag == PW_DRAGONFLAG  && ps->powerups[PW_NEUTRALFLAG] ) 
				return qtrue;
		}
		return qfalse;
    }

    if (gametype == GT_CTF)
    {
		// ent->modelindex2 is non-zero on items if they are dropped
		// we need to know this because we can pick up our dropped flag (and return it)
		// but we can't pick up our flag at base
		if (ps->persistant[PERS_TEAM] == TEAM_DRAGONS)
		{
			if (item->giTag == PW_NIKKIFLAG ||
				(item->giTag == PW_DRAGONFLAG && ent->modelindex2) ||  //droped flag
				(item->giTag == PW_DRAGONFLAG && ps->powerups[PW_NIKKIFLAG]))
				return qtrue;
		}
		else if (ps->persistant[PERS_TEAM] == TEAM_NIKKIS)
		{
			if (item->giTag == PW_DRAGONFLAG || 
				(item->giTag == PW_NIKKIFLAG && ent->modelindex2) || //droped flag
				(item->giTag == PW_NIKKIFLAG && ps->powerups[PW_DRAGONFLAG]))
				return qtrue;
		}
		return qfalse;
    }

#ifdef WITH_BAGMAN_MOD
  case IT_TEAM_SAFE: //server use only?
#if 0
    if (gametype == GT_BAGMAN)
    {
		//dragon player
		if (ps->persistant[PERS_TEAM] == TEAM_DRAGONS)
		{
			//touch own safe
			if (item->giTag == TM_DRAGONSAFE &&	ps->stats[STAT_CASH])
				return qtrue;

			//touch enemy safe
			else if (item->giTag == TM_NIKKISAFE && ps->stats[STAT_STOLEN_CASH] < cashstolenmax)
				return qtrue;
		}

		//niki player
		else if (ps->persistant[PERS_TEAM] == TEAM_NIKKIS)
		{
			//touch own safe
			if (item->giTag == TM_NIKKISAFE &&	ps->stats[STAT_CASH])
				return qtrue;

			//touch enemy safe
			else if (item->giTag == TM_DRAGONSAFE && ps->stats[STAT_STOLEN_CASH] < cashstolenmax)
				return qtrue;
		}
    }
#endif
	return qfalse;

  case IT_CASH:
		if (BG_CanPickupCash(cashcollectmax, ps))
			return qtrue;

		return qfalse;

  case IT_CASH_STOLEN:
		if (BG_CanPickupCashStolen(cashstolenmax, ps)) //g_cashstolenmax cvar
			return qtrue;

		return qfalse;
    //FIXME(0xA5EA): intermission time ??
#endif // WITH_BAGMAN_MOD

  case IT_HOLDABLE:
    // can only hold one item at a time
    if (ps->stats[STAT_HOLDABLE_ITEM])
      return qfalse;

    return qtrue;

  case IT_BAD:
    Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD");
  default:
#ifndef Q3_VM
#ifndef NDEBUG                      // bk0001204
    Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType);
#endif
#endif
    break;
  }
  return qfalse;
}

//======================================================================
/*
 ================
 BG_EvaluateTrajectory
 ================
 */
void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result)
{
  float deltaTime;
  float phase;
  vec3_t v;

  switch (tr->trType)
  {
  case TR_STATIONARY:
  case TR_INTERPOLATE:
    VectorCopy(tr->trBase, result);
    break;
  case TR_LINEAR:
    deltaTime = (atTime - tr->trTime) * 0.001; // milliseconds to seconds
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    break;
  case TR_SINE:
    deltaTime = (atTime - tr->trTime) / (float) tr->trDuration;
    phase = sin(deltaTime * M_PI * 2);
    VectorMA(tr->trBase, phase, tr->trDelta, result);
    break;
  case TR_LINEAR_STOP:
    if (atTime > tr->trTime + tr->trDuration)
      atTime = tr->trTime + tr->trDuration;
    deltaTime = (atTime - tr->trTime) * 0.001; // milliseconds to seconds
    if (deltaTime < 0)
      deltaTime = 0;
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    break;
  case TR_GRAVITY:
    deltaTime = (atTime - tr->trTime) * 0.001; // milliseconds to seconds
    VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
    result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime; // FIXME: local gravity...
    break;
#if 1 //to test
	case TR_ACCELERATE:	// trDelta is eventual speed
			if(atTime > tr->trTime + tr->trDuration)
			{
				atTime = tr->trTime + tr->trDuration;
			}
			deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
			// phase is the acceleration constant
			phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001);
			// trDelta at least gives us the acceleration direction
			VectorNormalize2(tr->trDelta, result);
			// get distance travelled at current time
			VectorMA(tr->trBase, phase * 0.5 * deltaTime * deltaTime, result, result);
		break;
#endif
	case TR_DECCELERATE:	// trDelta is breaking force
		if(atTime > tr->trTime + tr->trDuration)
			atTime = tr->trTime + tr->trDuration;
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		// phase is the breaking constant
		phase = VectorLength(tr->trDelta) / (tr->trDuration * 0.001);
		// trDelta at least gives us the acceleration direction
		VectorNormalize2(tr->trDelta, result);
		// get distance travelled at current time (without breaking)
		VectorMA(tr->trBase, deltaTime, tr->trDelta, v);
		// subtract breaking force
		VectorMA(v, -phase * 0.5 * deltaTime * deltaTime, result, result);
		break;

  default:
    Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime);
    break;
  }
}

/*
 ================
 BG_EvaluateTrajectoryDelta
 For determining velocity at a given time
 ================
 */
void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, vec3_t result)
{
  float deltaTime;
  float phase;

  switch (tr->trType)
  {
  case TR_STATIONARY:
  case TR_INTERPOLATE:
    VectorClear(result);
    break;
  case TR_LINEAR:
    VectorCopy(tr->trDelta, result);
    break;
  case TR_SINE:
    deltaTime = (atTime - tr->trTime) / (float) tr->trDuration;
    phase = cos(deltaTime * M_PI * 2); // derivative of sin = cos
    phase *= 0.5;
    VectorScale(tr->trDelta, phase, result);
    break;
  case TR_LINEAR_STOP:
    if (atTime > tr->trTime + tr->trDuration)
    {
      VectorClear(result);
      return;
    }
    VectorCopy(tr->trDelta, result);
    break;
  case TR_GRAVITY:
    deltaTime = (atTime - tr->trTime) * 0.001; // milliseconds to seconds
    VectorCopy(tr->trDelta, result);
    result[2] -= DEFAULT_GRAVITY * deltaTime; // FIXME: local gravity...
    break;
#if 0
	case TR_ACCELERATE:	// trDelta is the ultimate speed

		break;
#endif
  case TR_DECCELERATE:	// trDelta is the starting speed
		if(atTime > tr->trTime + tr->trDuration)
		{
			VectorClear(result);
			return;
		}
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		VectorScale(tr->trDelta, (1- deltaTime)*0.5f, result); //use eFlags |= EF_BOUNCE_HALF to slow down more bounce
		break;

  default:
    Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime);
    break;
  }
}

/*
 ===============
 BG_AddPredictableEventToPlayerstate
 Handles the sequence numbers
 ===============
 */
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm, playerState_t *ps)
{
#ifdef _DEBUG
  {
    char buf[256];
    trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
    if (atof(buf) != 0)
    {
#ifdef QAGAME
      Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime*/, ps->eventSequence /*,eventnames[newEvent]*/, eventParm);
#else
      Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount /*ps->commandTime*/, ps->eventSequence /*, eventnames[newEvent]*/, eventParm);
#endif
    }
  }
#endif
  ps->events[ps->eventSequence & (MAX_PS_EVENTS - 1)] = newEvent;
  ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS - 1)] = eventParm;
  ps->eventSequence++;
}

/*
 ========================
 BG_TouchJumpPad
 ========================
 */
void BG_TouchJumpPad(playerState_t *ps, entityState_t *jumppad)
{
  vec3_t angles;
  float p;
  int effectNum;

  // spectators don't use jump pads
  if (ps->pm_type != PM_NORMAL)
    return;
#ifdef GT_USE_TA_TYPES
  // flying characters don't hit bounce pads
  if (ps->powerups[PW_FLIGHT])
    return;
#endif
  // if we didn't hit this same jumppad the previous frame
  // then don't play the event sound again if we are in a fat trigger
  if (ps->jumppad_ent != jumppad->number)
  {
	VectorToAngles(jumppad->origin2, angles);
    p = fabs(AngleNormalize180(angles[PITCH]));
    if (p < 45)
      effectNum = 0;
    else
      effectNum = 1;

    BG_AddPredictableEventToPlayerstate(EV_JUMP_PAD, effectNum, ps);
  }
  // remember hitting this jumppad this frame
  ps->jumppad_ent = jumppad->number;
  ps->jumppad_frame = ps->pmove_framecount;
  // give the player the velocity from the jumppad
  VectorCopy(jumppad->origin2, ps->velocity);
}

/*
========================
 BG_PlayerStateToEntityState
 This is done after each set of usercmd_t on the server,
 and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap)
{
  int i;

  if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR)
    s->eType = ET_INVISIBLE;
  else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH_RL)
    s->eType = ET_INVISIBLE;
  else
    s->eType = ET_PLAYER;

  s->number = ps->clientNum;

  s->pos.trType = TR_INTERPOLATE;

   VectorCopy(ps->origin, s->pos.trBase);

  if (snap)
    SnapVector(s->pos.trBase);
  // set the trDelta for flag direction
  VectorCopy(ps->velocity, s->pos.trDelta);

  s->apos.trType = TR_INTERPOLATE;
  VectorCopy(ps->viewangles, s->apos.trBase);
  if (snap)
    SnapVector(s->apos.trBase);

  //s->angles2[YAW] = ps->movementDir; //hypov8 todo: ok?
  s->time2 = ps->movementDir; //hypov8 stops legs twisting in movement dir
  s->legsAnim = ps->legsAnim;
  s->torsoAnim = ps->torsoAnim;
  s->weaponAnim = ps->weaponAnim;
  s->clientNum = ps->clientNum; // ET_PLAYER looks here instead of at number
  // so corpses can also reference the proper config
  s->eFlags = ps->eFlags;
  if (ps->stats[STAT_HEALTH] <= 0)
    s->eFlags |= EF_DEAD;
  else
    s->eFlags &= ~EF_DEAD;

  if (ps->externalEvent)
  {
    s->event = ps->externalEvent;
    s->eventParm = ps->externalEventParm;
  }
  else if (ps->entityEventSequence < ps->eventSequence)
  {
    int seq;

    if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
      ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;

    seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
    s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
    s->eventParm = ps->eventParms[seq];
    ps->entityEventSequence++;
  }

  s->weapon = ps->weapon;
  s->groundEntityNum = ps->groundEntityNum;

  s->powerups = 0;
  for (i = 0; i < MAX_POWERUPS; i++)
  {
    if (ps->powerups[i])
      s->powerups |= 1 << i;
  }

  s->loopSound = ps->loopSound;
  s->generic1 = ps->generic1;
}

/*
 ========================
 BG_PlayerStateToEntityStateExtraPolate

 This is done after each set of usercmd_t on the server,
 and after local prediction on the client
 ========================
 */
void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s, int time, qboolean snap)
{
  int i;

  if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR)
    s->eType = ET_INVISIBLE;
  else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH_RL)
    s->eType = ET_INVISIBLE;
  else
    s->eType = ET_PLAYER;

  s->number = ps->clientNum;

  s->pos.trType = TR_LINEAR_STOP;
  VectorCopy(ps->origin, s->pos.trBase);
  if (snap)
    SnapVector(s->pos.trBase);
  // set the trDelta for flag direction and linear prediction
  VectorCopy(ps->velocity, s->pos.trDelta);
  // set the time for linear prediction
  s->pos.trTime = time;
  // set maximum extra polation time
  s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

  s->apos.trType = TR_INTERPOLATE;
  VectorCopy(ps->viewangles, s->apos.trBase);
  if (snap)
    SnapVector(s->apos.trBase);

  //s->angles2[YAW] = ps->movementDir; //hypov8 todo: ok?
  s->time2 = ps->movementDir; //hypov8 stops legs twisting in movement dir
  s->legsAnim = ps->legsAnim;
  s->torsoAnim = ps->torsoAnim;
  s->weaponAnim = ps->weaponAnim;
  s->clientNum = ps->clientNum; // ET_PLAYER looks here instead of at number
  // so corpses can also reference the proper config
  s->eFlags = ps->eFlags;

  if (ps->stats[STAT_HEALTH] <= 0)
    s->eFlags |= EF_DEAD;
  else
    s->eFlags &= ~EF_DEAD;

  if (ps->externalEvent)
  {
    s->event = ps->externalEvent;
    s->eventParm = ps->externalEventParm;
  }
  else if (ps->entityEventSequence < ps->eventSequence)
  {
    int seq;
    if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
      ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;

    seq = ps->entityEventSequence & (MAX_PS_EVENTS - 1);
    s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
    s->eventParm = ps->eventParms[seq];
    ps->entityEventSequence++;
  }

  s->weapon = ps->weapon;
  s->groundEntityNum = ps->groundEntityNum;

  s->powerups = 0;
  for (i = 0; i < MAX_POWERUPS; i++)
  {
    if (ps->powerups[i])
      s->powerups |= 1 << i;
  }
  s->loopSound = ps->loopSound;
  s->generic1 = ps->generic1;
}

/*
===============
BG_WeaponMaxMagCount

Return guns max magazine fill capacity
===============
*/
int BG_WeaponMaxMagCount(int weapon)
{
  switch (weapon)
  {                                    // kp values ammo/rounds
  case WP_PISTOL:           return 10; // 200/10
  case WP_SHOTGUN:          return  8; // 100/8
  case WP_MACHINEGUN:       return 50; // 200/50
  case WP_GRENADE_LAUNCHER: return  3; // 12/3
  case WP_ROCKET_LAUNCHER:  return  5; // 25/5
  case WP_HMG:              return 30; // 90/30
#ifdef USE_FLAMEGUN
  case WP_FLAMER:           return 50; // 200/50
#endif
  }
  return INFINITE_AMMO;
}

/*
===============
BG_WeaponMaxAmmoCount
===============
*/
int BG_WeaponMaxAmmoCount(int weapon) //total bullets on self
{
  switch (weapon)
  {                                     // kp values ammo/rounds
  case WP_PISTOL:           //combined
  case WP_MACHINEGUN:       return 200; // 200/50
  case WP_SHOTGUN:          return 100; // 100/8
  case WP_GRENADE_LAUNCHER: return  12; // 12/3
  case WP_ROCKET_LAUNCHER:  return  25; // 25/5
  case WP_HMG:              return  90; // 90/30
#ifdef USE_FLAMEGUN
  case WP_FLAMER:           return 200; // 200/50
#endif
  }
  return INFINITE_AMMO;
  //hypov8 todo: add item_pack varables +xx
}


/*
===============
BG_IsReloadableWeapon

skip crowbar
===============
*/
qboolean BG_IsReloadableWeapon(int weapon)
{
  switch (weapon)
  {
    case WP_PISTOL:
    case WP_SHOTGUN:
    case WP_MACHINEGUN:
    case WP_GRENADE_LAUNCHER:
    case WP_ROCKET_LAUNCHER:
    case WP_HMG:
#ifdef USE_FLAMEGUN
    case WP_FLAMER:
      return qtrue;
#endif
  }
    return qfalse;
}

/*
===============
BG_AttackTorsoAnim
===============
*/
animNumber_t BG_GetTorsoAttackAnimNumber(int weapon)
{
  switch (weapon)
  {
  case WP_CROWBAR: 
  case WP_GRAPPLING_HOOK: 
    return TORSO_ATTACK2;

  case WP_PISTOL:  
    return TORSO_ATTACK3;

  default:         
    return TORSO_ATTACK;
  }
}

/*
===============
BG_GetTorsoIdleAnimNumber
===============
*/
animNumber_t BG_GetTorsoIdleAnimNumber(int weapon)
{
  switch (weapon)
  {
  case WP_CROWBAR: 
  case WP_GRAPPLING_HOOK: 
    return TORSO_STAND2;

  case WP_PISTOL:  
    return TORSO_STAND3;

  default:         
    return TORSO_STAND;
  }
}


/*
===============
  BG_PersAssignAttacker
===============
*/
void BG_PersAssignAttacker(short attacker, int* ptarget)
{
  shortbytes_t i;
  i.s = *ptarget;
  i.b[PERS_ATTACKER_POS] = attacker;
  *ptarget = i.s;
}

/*
===============
  BG_PersAssignAttackeeHealth
===============
*/
void BG_PersAssignAttackeeHealth(short health, int* ptarget)
{
  shortbytes_t i;
  i.s = *ptarget;
  i.b[PERS_ATTACKEE_HEALTH_POS] = health;
  *ptarget = i.s;
}

/*
===============
  BG_PersAddToAssistCount
===============
*/
void BG_PersAddToAssistCount(short count, int* ptarget)
{
  shortbytes_t i;
  i.s = *ptarget;
  i.b[PERS_ASSIST_POS] += count;
  *ptarget = i.s;
}

/*
===============
  BG_PersAddToExcellentCount
===============
*/
void BG_PersAddToExcellentCount(short count, int* ptarget)
{
  shortbytes_t i;
  i.s = *ptarget;
  i.b[PERS_EXCELLENT_POS] += count;
  *ptarget = i.s;
}

//hypov8 combine ammo for tommy and pistol
int BG_AmmoCombineCheck(int weaponNum)
{
	if (weaponNum == WP_PISTOL)
		weaponNum = WP_MACHINEGUN;

	return weaponNum;
}

