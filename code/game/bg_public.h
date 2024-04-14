/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2010 Lars '0xA5EA' Kandler
Copyright (C) 2011 Ari 'KRYPTYK' Mirles

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

// bg_public.h -- definitions shared by both the server game and client game modules
// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef BG_PUBLIC_H_
#define BG_PUBLIC_H_

#define GAME_VERSION BASEGAME "-1"

extern vmCvar_t g_cursemode;  //hypov8 merge:     //FIXME(0xA5EA): implementation incomplete

#define WITH_BAGMAN_MOD
#define XPPM 1 //enable hypov8 //hypov8 merge: not in kpq3
#define PLAYER_RESPAWN_HEALTH_ADD    30
#define DEFAULT_GRAVITY             800
#define GIB_HEALTH_RL              -110 //atleast +max bullet dmg. hitman mod = 100hp
#define GIB_HEALTH_BULLET            -5 //this prevents invisable flickering
#define ARMOR_PROTECTION           0.66

#define MAX_ITEMS                   256

#define RANK_TIED_FLAG           0x4000

#define G_SPEED_MAX 300 //hypov8 add: g_speed cvar


//////////////////////////////////////
//moved
//hypov8 kpq3 cg_ and g_ common predicted items
#define NUM_NAILSHOTS 15
#define CROWBAR_DIST 48

#define PISTOL_DAMAGE 15
#define PISTOL_SPREAD 50 //hypov8 was 5. very acurate
#define PISTOL_DAMAGE_HM 50
//FIXME (0xA5EA): wert geraten
#define MACHINEGUN_SPREAD 500
#define MACHINEGUN_DAMAGE 14
#define MACHINEGUN_TEAM_DAMAGE 12    // wimpier MG in teamplay,

#define HMG_DAMAGE 50
#define HMG_SPREAD 1

#define CROWBAR_DAMAGE 50
#define CROWBAR_DAMAGE_RM 25
#define CROWBAR_DAMAGE_HM 75

#define DEFAULT_SHOTGUN_DAMAGE				16
#define DEFAULT_SHOTGUN_SPREAD              700
#define DEFAULT_SHOTGUN_SPREAD_RM			500 //realmode
#define DEFAULT_SHOTGUN_COUNT				6  // was 11, 6 is kp-value


//hypo cg and g common predicted items _end
//////////////////////////////////////




//FIXME (0xA5EA): unsuded ?
#define ITEM_RADIUS                  15        // item sizes are needed for client side pickup detection


#define SCORE_NOT_PRESENT         -9999        // for the CS_SCORES[12] when only one player is present

#define VOTE_TIME                 30000        // 30 seconds before vote times out

#define MINS_Z                      -24
#define DEFAULT_VIEWHEIGHT           36        //0xA5EA, this affects the players viewhigh, q3-default is 26, 34 is kingpinish //hypov8 = 36
#define CROUCH_VIEWHEIGHT            12        //note hypov8 = 18
#define DEAD_VIEWHEIGHT             -16

#define FIRE_FLASH_TIME			2000			// Added for flame flamethrower fire effects -KRYPTYK
#define	FIRE_FLASH_FADEIN_TIME	1000

#define FLAMETHROWER_RANGE  850					// Was 768 -KRYPTYK

#define ITEMNAME_DRAGON_FLAG        "Dragons Flag"
#define ITEMNAME_NIKKIS_FLAG        "Nikkis Flag"
#define ITEMNAME_NEUTRAL_FLAG       "Neutral Flag"
#ifdef WITH_BAGMAN_MOD
#define ITEMNAME_DRAGON_SAFE        "Dragons Safe"
#define ITEMNAME_NIKKIS_SAFE        "Nikkis Safe"
#define ITEMNAME_CASHROLL           "Cashroll"
#define ITEMNAME_CASHBAG            "Cashbag"
#define ITEMNAME_CASHBAG_STOLEN		"Cashbag Stolen"
#define ITEMNAME_CASHSPAWN			"Cash Spawn"
#endif
//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define CS_MUSIC              2
#define CS_MESSAGE            3        // from the map worldspawn's message field
#define CS_MOTD               4        // g_motd string for server message of the day
#define CS_WARMUP             5        // server time when the match will be restarted
#define CS_SCORES1            6
#define CS_SCORES2            7
#define CS_VOTE_TIME          8
#define CS_VOTE_STRING        9
#define CS_VOTE_YES          10
#define CS_VOTE_NO           11

#define CS_TEAMVOTE_TIME     12
#define CS_TEAMVOTE_STRING   14
#define CS_TEAMVOTE_YES      16
#define CS_TEAMVOTE_NO       18

#define CS_GAME_VERSION      20
#define CS_LEVEL_START_TIME  21        // so the timer only shows the current level
#define CS_INTERMISSION      22        // when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS        23        // string indicating flag status in CTF
#define CS_SHADERSTATE       24
#define CS_BOTINFO           25
#define CS_ITEMS             27        // string of 0's and 1's that tell which items are present

#define CS_MODELS            32
#define CS_SOUNDS           (CS_MODELS + MAX_MODELS)
#define CS_PLAYERS          (CS_SOUNDS + MAX_SOUNDS)
#define CS_LOCATIONS        (CS_PLAYERS + MAX_CLIENTS)
#define CS_PARTICLES        (CS_LOCATIONS + MAX_LOCATIONS)
#define CS_MAX              (CS_PARTICLES + MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

extern const vec3_t playerMins;
extern const vec3_t playerMaxs;

typedef enum
{
  GT_FFA,                                 // free for all
  GT_TOURNAMENT,                          // one on one tournament
#ifdef USE_GT_SINGLEPLAYER
  GT_SINGLE_PLAYER,                       // single player ffa
#endif
  //-- team games go after this --

  GT_TEAM,                                // team deathmatch
  GT_CTF,                                 // capture the flag
  GT_1FCTF,
#ifdef WITH_BAGMAN_MOD
  GT_BAGMAN,                              /* bagman */
#endif

#ifdef GT_USE_TA_TYPES
  GT_OBELISK,
  GT_HARVESTER,
#endif
  GT_MAX_GAME_TYPE
} gametype_t;

// 0xA5EA

#define WM_NORMAL           (int)0x0
#define WM_REALMODE         (int)0x1
#define WM_HITMEN           (int)0x2
#define WM_HITMENREALMODE   (WM_REALMODE | WM_HITMEN)
#define HM_MAX_WEAPONTIME   (int)(3 * 60)     // 0xA5EA, 3 min
#define HM_MIN_WEAPONTIME   (int)10

typedef enum {GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER} gender_t;
//FIXME(0xA5EA): no neuter in kpq3

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/
#if 0
from kp
typedef enum
{
  MOVETYPE_NONE,          // never moves
  MOVETYPE_NOCLIP,        // origin and angles change with no interaction
  MOVETYPE_PUSH,          // no clip to world, push on box contact
  MOVETYPE_STOP,          // no clip to world, stops on box contact

  MOVETYPE_WALK,          // gravity
  MOVETYPE_STEP,          // gravity, special edge handling
  MOVETYPE_FLY,
  MOVETYPE_TOSS,          // gravity
  MOVETYPE_FLYMISSILE,    // extra size to monsters
  MOVETYPE_BOUNCE,        // added this (the comma at the end of line)
  MOVETYPE_WALLBOUNCE,    // wird nur beim feuern einer Waffe benutzt, ist ausserdem auskommentiert in kp
  MOVETYPE_TOSS_SLIDE     // Ridah, testing fuer grenade move benutzt only
} movetype_t;
#endif
typedef enum pmtype_t
{
  PM_NORMAL,              // can accelerate and turn
  PM_NOCLIP,              // noclip movement
  PM_SPECTATOR,           // still run into walls
  PM_DEAD,                // no acceleration or turning, but free falling
  PM_FREEZE,              // stuck in place with no control
  PM_INTERMISSION,        // no movement or status bar
  PM_SPINTERMISSION,      // no movement or status bar
  /* 0xA5EA, additional movetypes from kp */
  PM_BOUNCE,
  PM_TOSS_SLIDE,
  PM_STEP
} pmtype_t;

#define MAX_FLASH_SOUNDS 4

typedef enum
{
  WEAPON_READY,
  WEAPON_RAISING,
  WEAPON_DROPPING,
  WEAPON_FIRING,
  WEAPON_RELOADING,
  WEAPON_RELOAD_MOD, //reload shotty mod.
  //WEAPON_FIRE_MOD, //cooldown. ROF
  //hypov8 todo: add silencer, mods etc..
  MAX_WEAPON_STATES,
  WEAPON_HM_LOCK
} weaponstate_t;

#define INFINITE_AMMO -1

typedef enum
{
  HMG_FIREING_FIRST   = 0x01,
  HMG_FIRE_SECOND    = 0x02,
  HMG_FIRING_SECOND  = 0x04,
  HMG_FIRE_THIRD     = 0x08,
  HMG_FIRING_THIRD    = 0x10
}hmgstate_t;
#if 0
typedef enum
{
    WEAPON_READY = 0,
    WEAPON_RAISING,
    WEAPON_DROPPING,
    WEAPON_FIRE,
    WEAPON_RELOAD,
    WEAPON_ACTIVATING,
    MAX_WEAP_ANIMATIONS
} weapon_animation_t;

typedef enum
{
  WEAPON_READY,
  WEAPON_ACTIVATING,
  WEAPON_DROPPING,
  WEAPON_FIRING,
// RAFAEL 01-11-99
  WEAPON_RELOADING,
  WEAPON_RELOAD_CYCLE,
  WEAPON_RELOADING_SPISTOL
} weaponstate_t;


#endif
// pmove->pm_flags
#define PMF_DUCKED 1
#define PMF_JUMP_HELD 2
#define PMF_BACKWARDS_JUMP 8      // go into backwards land
#define PMF_BACKWARDS_RUN 16      // coast down to backwards run
#define PMF_TIME_LAND 32          // pm_time is time before rejump
#define PMF_TIME_KNOCKBACK 64     // pm_time is an air-accelerate only time
#define PMF_TIME_WATERJUMP 256    // pm_time is waterjump
#define PMF_RESPAWNED 512         // clear after attack and jump buttons come up
#define PMF_USE_ITEM_HELD 1024
#define PMF_GRAPPLE_PULL 2048     // pull towards grapple location
#define PMF_FOLLOW 4096           // spectate following another player
#define PMF_SCOREBOARD 8192       // spectate as a scoreboard
#define PMF_INVULEXPAND 16384     // invulnerability sphere set to full size
#define PMF_WEAPON_RELOAD  32768 // force a weapon reload //hypov8 todo: not used yet
#define PMF_WEAPON_SWITCH  65536 //daemon .5 // force a weapon switch by server
#define PMF_WEAPON_LOCK  (1<<16) //HM wep lock. hypov8 todo:

#define PMF_ALL_TIMES (PMF_TIME_WATERJUMP | PMF_TIME_LAND | PMF_TIME_KNOCKBACK)

#define MAXTOUCH 32
typedef struct
{
  // state (in / out)
  playerState_t *ps;

  // command (in)
  usercmd_t cmd;
  int tracemask;                // collide against these types of surfaces
  int debugLevel;               // if set, diagnostic output will be printed
  qboolean noFootsteps;         // if the game is setup for no footsteps by the server
  qboolean crowbarHit;         // true if a crowbar attack would actually hit something

  int framecount;

  // results (out)
  int numtouch;
  int touchents[MAXTOUCH];

  vec3_t mins, maxs;            // bounding box size

  int watertype;
  int waterlevel;

  float xyspeed;

  // for fixed msec Pmove
  int pmove_fixed;
  int pmove_msec;

  // don't round velocity to an integer
  int pmove_accurate;

  // callbacks to test the world
  // these will be different functions during game and cgame
  void (*trace)(trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask);
  int (*pointcontents)(const vec3_t point, int passEntityNum);
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd);
void Pmove(pmove_t *pmove);

//===================================================================================

// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum
{
  STAT_HEALTH,
  STAT_HOLDABLE_ITEM,
  STAT_PERSISTANT_POWERUP,
  STAT_WEAPONS,                           // 16 bit fields
  STAT_ARMOR_LEGS,
  STAT_ARMOR_BODY,
  STAT_ARMOR_HEAD,
#ifdef WITH_BAGMAN_MOD
  STAT_STOLEN_CASH,                       // 0xA5EA, bagman  warum ist die flag nicht bei den stats dabei ?
  STAT_CASH,                    // 0xA5EA, bagman
  //FIXME(0xA5EA): bagman, use them
#endif
  STAT_DEAD_YAW,                          // look this direction when dead (FIXME: get rid of?)
  STAT_CLIENTS_READY,                     // bit mask of clients wishing to exit the intermission (FIXME: configstring?)
  STAT_MAX_HEALTH,                         // health / armor limit, changable by handicap
  STAT_WEAP_MODS,
} statIndex_t;


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum
{
  PERS_SCORE,                             // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
  PERS_HITS,                              // total points damage inflicted so damage beeps can sound on change
  PERS_RANK,                              // player rank or team rank (uses RANK_TIED flag as well)
  PERS_TEAM,                              // player team
  PERS_SPAWN_COUNT,                       // incremented every respawn
  PERS_PLAYEREVENTS,                      // 16 bits that can be flipped for events
  PERS_ATTACKER_INFO,                     // byte[1] = clientnum of last damage inflicter, byte[0] = health of last person we attacker
  PERS_KILLED,                            // count of the number of times you died
  // player awards tracking
  PERS_EXCELLENT_ASSIST_COUNT,            // byte[1] = Excellent count two successive kills in a short amount of time
                                          // byte[0] = assist awards
  PERS_FRAG_COUNT,
  PERS_DEFEND_COUNT,                      // defend awards
  PERS_CAPTURES,                          // captures
#ifdef WITH_BAGMAN_MOD
  PERS_CASH_STOLEN,                       // 0xA5EA, bagman
  PERS_CASH_COLLECTED,                    // 0xA5EA, bagman
  PERS_CASH_DEPOSIT						//hypov8 todo: client not using these yet?
#endif
  ,PERS_NEWWEAPON
  //FIXME(0xA5EA): bagman need them all ?

} persEnum_t;

void BG_PersAssignAttacker(short attacker, int* ptarget);
void BG_PersAssignAttackeeHealth(short health, int* ptarget);
void BG_PersAddToAssistCount(short count, int* ptarget);
void BG_PersAddToExcellentCount(short count, int* ptarget);

#define PERS_ATTACKER_POS        1
#define PERS_ATTACKEE_HEALTH_POS 0
#define PERS_EXCELLENT_POS       1
#define PERS_ASSIST_POS          0

// entityState_t->eFlags
#define EF_DEAD            0x00000001     // don't draw a foe marker over players with EF_DEAD
#define EF_TICKING         0x00000002     // used to make players play the prox mine ticking sound
#define EF_TELEPORT_BIT    0x00000004     // toggled every time the origin abruptly changes
#define EF_AWARD_EXCELLENT 0x00000008     // draw an excellent sprite
#define EF_PLAYER_EVENT    0x00000010
#define EF_BOUNCE          0x00000010     // for missiles
#define EF_BOUNCE_HALF     0x00000020     // for missiles
//#define EF_AWARD_GAUNTLET 0x00000040    // draw a gauntlet sprite
#define EF_NODRAW          0x00000080     // may have an event, but no model (unspawned items)
#define EF_FIRING          0x00000100     // for lightning gun
#ifdef USE_KAMIKAZE
#define EF_KAMIKAZE        0x00000200
#endif
#define EF_MOVER_STOP      0x00000400     // will push otherwise
#define EF_AWARD_CAP       0x00000800     // draw the capture sprite
#define EF_TALK            0x00001000     // draw a talk balloon
#define EF_CONNECTION      0x00002000     // draw a connection trouble sprite
#define EF_VOTED           0x00004000     // already cast a vote
//#define EF_AWARD_IMPRESSIVE 0x00008000  // draw an impressive sprite
#define EF_AWARD_DEFEND    0x00010000     // draw a defend sprite
#define EF_AWARD_ASSIST    0x00020000     // draw a assist sprite
#define EF_AWARD_DENIED    0x00040000     // denied
#define EF_TEAMVOTED       0x00080000     // already cast a team vote

#define CASHROLL_AMONG 10
#define CASHBAG_AMONG  25

// NOTE: may not have more than 16
typedef enum
{
  PW_NONE,
  PW_FLASHLIGHT,
  PW_DRAGONFLAG,
  PW_NIKKIFLAG,
  PW_NEUTRALFLAG,
#ifdef WITH_BAGMAN_MOD
  PW_STOLENSCASH, //hypov8 note: player carried model
#endif
  PW_WPMOD_COOLING,
  PW_WPMOD_PISTOLMAGNUM,
  PW_WPMOD_PISTOLDOUBLEFIRE,
  PW_WPMOD_SILENCER,
  PW_DEGRADE_START, //hypov8 note: timeout items below (timelimit)
#ifdef GT_USE_TA_TYPES
  PW_SCOUT = 12,
  PW_GUARD,
  PW_DOUBLER,
  PW_AMMOREGEN,
  PW_INVULNERABILITY,
  PW_BATTLESUIT,
  PW_HASTE,
  PW_INVIS,
  PW_REGEN,
  PW_FLIGHT,
#endif

  PW_NUM_POWERUPS
} powerup_t;

#ifdef WITH_BAGMAN_MOD
typedef enum
{
  TM_NONE,
  TM_DRAGONSAFE,
  TM_NIKKISAFE
} teamsafe_t;
#endif

typedef enum
{
  HI_NONE,

  HI_TELEPORTER,
  HI_MEDKIT,
#ifdef USE_KAMIKAZE
  HI_KAMIKAZE,
#endif
  HI_PORTAL,
  HI_INVULNERABILITY,

  HI_NUM_HOLDABLE
} holdable_t;

//hypov8 todo: this should be 2x in multiplayer

//WEAPON_FIRING                         (kp wep frame count) *100ms
#define WP_TIME_FIRE_PISTOL             (5*100)
#define WP_TIME_FIRE_CROWBAR            (10*100)
#define WP_TIME_FIRE_SHOTGUN            (12*100)
#define WP_TIME_FIRE_MACHINEGUN         (1*100)
#define WP_TIME_FIRE_GRENADE_LAUNCHER   ((4+8)*100) //fire + cycle mag
#define WP_TIME_FIRE_ROCKET_LAUNCHER    (8*100)
#define WP_TIME_FIRE_HMG_FULL           (12*100) //anim broken into 1/3 sections(3x shoot, cooldown1, cooldown2)
#define WP_TIME_FIRE_HMG_3RD            (int)(WP_TIME_FIRE_HMG_FULL/3)
#define WP_TIME_FIRE_FLAMEGUN           (1*100)
#define WP_TIME_FIRE_GRAPPLING_HOOK     (4*100)

//WEAPON_RELOADING                      (kp wep frame count) *100ms
#define WP_TIME_RELOAD_PISTOL           (18*100)
#define WP_TIME_RELOAD_SHOTGUN          (15*100)
#define WP_TIME_RELOAD_MACHINEGUN       (24*100)
#define WP_TIME_RELOAD_GRENADE_LAUNCHER (16*100)
#define WP_TIME_RELOAD_ROCKET_LAUNCHER  (14*100)
#define WP_TIME_RELOAD_HMG              (18*100)
#define WP_TIME_RELOAD_FLAMEGUN         (14*100)

//WEAPON_RAISING || WEAPON_DROPPING
#define WP_TIME_CHANGE_GUNS             500 //hypov8 note: check mplayer speeds...
#define WP_TIME_CHANGE_MELEE            250

//WEAPON_RELOAD_MOD                     //shotty needs to <startReload> <reload> <finishReload>
#define WP_TIME_MOD_SHOTGUN             (5*100) //reload a 2nd bullet faster ( animations should be broken into 1/3 total time)


//WEAPON_READY
//using fps in animation.cfg

typedef enum
{
  WP_NONE,
  //WP_PIPE //hypov8 todo: blackjack
  WP_CROWBAR,
  WP_PISTOL,
  WP_FIRST = WP_PISTOL,
  WP_SHOTGUN,
  WP_MACHINEGUN,
  WP_GRENADE_LAUNCHER,
  WP_ROCKET_LAUNCHER,
  WP_HMG, //hypov8 hmg should be 5?
#ifdef USE_FLAMEGUN
  WP_FLAMER,
  WP_LAST = WP_FLAMER,
#else
  WP_LAST = WP_HMG,
#endif
  WP_GRAPPLING_HOOK,
  WP_NUM_WEAPONS,
} weapon_t;

typedef enum
{
  AR_NONE = 0,
  AR_HEAD = 4,       //  mapped to STAT_ARMOR_HEAD
  AR_BODY = 5,       //  mapped to STAT_ARMOR_BODY
  AR_LEGS = 6        //  mapped to STAT_ARMOR_LEGS
} armor_t;             // 0xA5EA, mapped to STAT_ARMOR_xxx //hypov8 statIndex_t

#define MAX_ARMOR 100       // 0xA5EA

#define HM_WEAPFLAG_PISTOL           (int)0x01
#define HM_WEAPFLAG_MACHINEGUN       (int)0x02
#define HM_WEAPFLAG_SHOTGUN          (int)0x04
#define HM_WEAPFLAG_GRENADE_LAUNCHER (int)0x08
#define HM_WEAPFLAG_ROCKET_LAUNCHER  (int)0x10
#define HM_WEAPFLAG_HMG              (int)0x20
#define HM_WEAPFLAG_FLAMER           (int)0x40

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define PLAYEREVENT_DENIEDREWARD 0x0001
//#define PLAYEREVENT_GAUNTLETREWARD 0x0002
#define PLAYEREVENT_HOLYSHIT 0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define EV_EVENT_BIT1 0x00000100
#define EV_EVENT_BIT2 0x00000200
#define EV_EVENT_BITS (EV_EVENT_BIT1 | EV_EVENT_BIT2)

#define EVENT_VALID_MSEC 300

typedef enum
{
  EV_NONE,

  EV_FOOTSTEP,
  EV_FOOTSTEP_METAL,
  EV_FOOTSPLASH,
  EV_FOOTWADE,
  EV_SWIM,

  EV_STEP_4,
  EV_STEP_8,
  EV_STEP_12,
  EV_STEP_16,             // 10

  EV_STEPDN_4, //add hypov8 xreal step down hop issue fix
  EV_STEPDN_8,
  EV_STEPDN_12,
  EV_STEPDN_16,

  EV_FALL_SHORT,
  EV_FALL_MEDIUM,
  EV_FALL_FAR,

  EV_JUMP_PAD,            // boing sound at origin, jump sound on player

  EV_JUMP,
  EV_WATER_TOUCH,         // foot touches
  EV_WATER_LEAVE,         // foot leaves
  EV_WATER_UNDER,         // head touches
  EV_WATER_CLEAR,         // head leaves

  EV_ITEM_PICKUP,         // normal item pickups are predictable  20
  EV_GLOBAL_ITEM_PICKUP,  // powerup / team sounds are broadcast to everyone

  EV_NOAMMO,			// no ammo left anywhere
  EV_NOROUNDS,		// add hypov8 no rounds left in gun, now reload
  //EV_WEAPONSWITCHED,// hypov8 change weapon
  //EV_SELECTWEAPON, //add hypov8
  //EV_RESETSELECTWEP,

  EV_HITMEN_INFOMSG,

  EV_CHANGE_WEAPON_DROP, //OLD
  EV_CHANGE_WEAPON_RAISE, //NEW

  EV_FIRE_WEAPON,
  EV_FIRE_SHOTGUN, // add hypov8
  EV_FIRE_SPISTOL, //add hypov8
  EV_FIRE_CROWBAR, //hypov8 add
  //EV_FIRE_COOLDOWN, //hypov8 todo. for weps with speed mods

  EV_RELOAD_WEAPON,
  EV_RELOAD_SHOTGUN, // add hypov8 shotgun reload sequence

  EV_USE_ITEM0,
  EV_USE_ITEM1,
  EV_USE_ITEM2,          // eventno 30
  EV_USE_ITEM3,
  EV_USE_ITEM4,
  EV_USE_ITEM5,
  EV_USE_ITEM6,
  EV_USE_ITEM7,
  EV_USE_ITEM8,
  EV_USE_ITEM9,
  EV_USE_ITEM10,
  EV_USE_ITEM11,
  EV_USE_ITEM12,       // eventno 40
  EV_USE_ITEM13,
  EV_USE_ITEM14,
  EV_USE_ITEM15,

  EV_ITEM_RESPAWN,
  EV_ITEM_POP,
  EV_PLAYER_TELEPORT_IN,
  EV_PLAYER_TELEPORT_OUT,

  EV_GRENADE_BOUNCE,          // eventParm will be the soundindex
  EV_GRENADE_BOUNCE_METAL,    /* 0xA5EA */
  EV_GRENADE_BOUNCE_WOOD,    // eventno 50

  EV_GENERAL_SOUND,
  EV_GLOBAL_SOUND,            // no attenuation
  EV_GLOBAL_TEAM_SOUND,

  EV_BULLET_HIT_FLESH,
  EV_BULLET_HIT_WALL,
  EV_BULLET_HIT_WOOD,
  EV_BULLET_HIT_METAL,
  EV_BULLET_HIT_EARTH,
  EV_BULLET_HIT_GRAVEL,
  EV_BULLET_HIT_SNOW,       // eventno 60
  EV_BULLET_HIT_GLASS,

  EV_MISSILE_HIT,
  EV_MISSILE_MISS,
  EV_MISSILE_MISS_METAL,

  //EV_RAILTRAIL,  //FIXME(0xA5EA): railtrail still needed ?
#if 0 // 0xA5EA, its seems that this event was never used
  EV_BULLET,                  // otherEntity is the shooter
#endif
  EV_PAIN,
  EV_DEATH1,
  EV_DEATH2,
  EV_DEATH3,
  EV_OBITUARY,
#ifdef GT_USE_TA_TYPES
  EV_POWERUP_QUAD,
  EV_POWERUP_BATTLESUIT,
  EV_POWERUP_REGEN,
#endif
  EV_GIB_PLAYER,              // gib a previously living player
  EV_SCOREPLUM,               // score plum
#ifdef USE_KAMIKAZE
  EV_KAMIKAZE,
#endif

#ifdef GT_USE_TA_TYPES
  EV_PROXIMITY_MINE_STICK,
  EV_PROXIMITY_MINE_TRIGGER,
  EV_OBELISKEXPLODE,          // obelisk explodes
  EV_OBELISKPAIN,             // obelisk is in pain
  EV_INVUL_IMPACT,            // invulnerability sphere impact
  EV_JUICED,
#endif
#ifdef USE_FLAMEGUN
  EV_FLAMEGUN,
#endif
  EV_DEBUG_LINE,
  EV_STOPLOOPINGSOUND,
  EV_TAUNT,
  EV_TAUNT_YES,
  EV_TAUNT_NO,
  EV_TAUNT_FOLLOWME,
  EV_TAUNT_GETFLAG,
  EV_TAUNT_GUARDBASE,
  EV_TAUNT_PATROL,

  EV_FOOTSTEP_GRAVEL,
  EV_FOOTSTEP_METALLIGHT,
  EV_FOOTSTEP_RUG,
  EV_FOOTSTEP_SNOW,
  EV_FOOTSTEP_WOOD,
  EV_FOOTSTEP_CRASS,
  EV_FOOTSTEP_MARBLE,
  EV_FOOTSTEP_LADDER,
  EV_FOOTSTEP_TIN,
  EV_FOOTSTEP_GLASS,
  EV_PLAYER_HIT_INFO

  //FIXME(0xA5EA): BAGMAN
} entity_event_t;

typedef enum
{
  GTS_DRAGON_CAPTURE,
  GTS_NIKKI_CAPTURE,
  GTS_DRAGON_RETURN,
  GTS_NIKKI_RETURN,
  GTS_DRAGON_TAKEN,
  GTS_NIKKI_TAKEN,
  GTS_DRAGONOBELISK_ATTACKED,
  GTS_NIKKIOBELISK_ATTACKED,
  GTS_DRAGONTEAM_SCORED,
  GTS_NIKKITEAM_SCORED,
  GTS_DRAGONTEAM_TOOK_LEAD,
  GTS_NIKKITEAM_TOOK_LEAD,
  GTS_TEAMS_ARE_TIED,
#ifdef WITH_BAGMAN_MOD
  GTS_DRAGON_DEPOSIT,
  GTS_NIKKI_DEPOSIT,
  GTS_DRAGON_STOLEN,
  GTS_NIKKI_STOLEN,

#endif

#ifdef USE_KAMIKAZE
  GTS_KAMIKAZE
#endif
} global_team_sound_t;

// animations
typedef enum
{
  BOTH_DEATH1,
  BOTH_DEAD1,
  BOTH_DEATH2,
  BOTH_DEAD2,
  BOTH_DEATH3,
  BOTH_DEAD3,

  BOTH_LADDER, //ladder idle 		//add hypov8
  BOTH_LDR_UP, //ladder move up 	//add hypov8
  BOTH_LDR_DN, //ladder move down //add hypov8

  TORSO_GESTURE,
  TORSO_GESTURE2,
  TORSO_GESTURE3,

  TORSO_ATTACK,
  TORSO_ATTACK2,	//crowbar
  TORSO_ATTACK3,  //pistol //add hypov8

  TORSO_DROP,  //lower old weapon
  TORSO_RAISE, //raise new weapon

  TORSO_STAND,
  TORSO_STAND2, //crowbar
  TORSO_STAND3, //pistol //added hypov8
  TORSO_RUN,   		//add hypov8
  TORSO_WALK,  		//add hypov8

  //hypov8 todo:
  TORSO_GETFLAG,
  TORSO_GUARDBASE,
  TORSO_PATROL,
  TORSO_FOLLOWME,
  TORSO_AFFIRMATIVE,
  TORSO_NEGATIVE,

  //legs
  LEGS_IDLE,
  LEGS_WALK,
  LEGS_WALK_BACK,
  LEGS_RUN,
  LEGS_RUN_BACK,

  LEGS_LAND,
  LEGS_JUMPB, //jump back
  LEGS_LANDB, //jump back landing
  LEGS_TURN,
  LEGS_JUMP,
  LEGS_SWIM,

  LEGS_CR_IDLE,
  LEGS_CR_BACK, //crouck, walk backwards
  LEGS_CR_WALK,

  //hypov8 todo:
  FLAG_RUN,
  FLAG_STAND,
  FLAG_STAND2RUN,

  MAX_TOTALANIMATIONS
} animNumber_t;

typedef struct animation_s
{

  qhandle_t handle;       // registered md5Animation or whatever
  qboolean clearOrigin;   // reset the origin bone

  int firstFrame;
  int numFrames;
  int loopFrames;         // 0 to numFrames
  int       frameLerp; // msec between frames
  int initialLerp;        // msec to get to first frame
  int reversed;           // true if animation is reversed
  int flipflop;           // true if animation should flipflop back to base
} animation_t;

// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define ANIM_TOGGLEBIT 128


typedef enum
{
  TEAM_FREE,
  TEAM_DRAGONS,
  TEAM_NIKKIS,
  TEAM_SPECTATOR,

  TEAM_NUM_TEAMS
} team_t;


#define TEAM_NAME_AUTO			"Auto"
#define TEAM_NAME_DRAGONS       "Dragons"
#define TEAM_NAME_NIKKIS        "Nikkis" //edit hypov8 "Nikkis_Boyz"


// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME 1000

// How many players on the overlay
#define TEAM_MAXOVERLAY 32

/* ===============> 0xA5EA locational damage defines */

#define LOCATION_NONE 0x00000000

// Height layers
#define LOCATION_HEAD 0x00000001     // [F,B,L,R] Top of head
#define LOCATION_FACE 0x00000002     // [F] Face [B,L,R] Head
#define LOCATION_SHOULDER 0x00000004 // [L,R] Shoulder [F] Throat, [B] Neck
#define LOCATION_CHEST 0x00000008    // [F] Chest [B] Back [L,R] Arm
#define LOCATION_STOMACH 0x00000010  // [L,R] Sides [F] Stomach [B] Lower Back
#define LOCATION_GROIN 0x00000020    // [F] Groin [B] Butt [L,R] Hip
#define LOCATION_LEG 0x00000040      // [F,B,L,R] Legs
#define LOCATION_FOOT 0x00000080     // [F,B,L,R] Bottom of Feet

// Relative direction strike came from
#define LOCATION_LEFT 0x00000100
#define LOCATION_RIGHT 0x00000200
#define LOCATION_FRONT 0x00000400
#define LOCATION_BACK 0x00000800

/* <=============== 0xA5EA locational damage defines */


//team task
typedef enum
{
  TEAMTASK_NONE,
  TEAMTASK_OFFENSE,
  TEAMTASK_DEFENSE,
  TEAMTASK_PATROL,
  TEAMTASK_FOLLOW,
  TEAMTASK_RETRIEVE,
  TEAMTASK_ESCORT,
  TEAMTASK_CAMP
} teamtask_t;

// means of death
typedef enum
{
  //(0xA5EA): keep this is sync with modNames in g_combat.c
  MOD_UNKNOWN,
  MOD_PISTOL,
  MOD_SHOTGUN,
  MOD_CROWBAR,        /* 0xA5EA todo */
  MOD_MACHINEGUN,
  MOD_GRENADE,
  MOD_GRENADE_SPLASH,
  MOD_ROCKET,
  MOD_ROCKET_SPLASH,
  MOD_HMG,
  MOD_FLAMEGUN,
  MOD_WATER,
  MOD_SLIME,
  MOD_LAVA,
  MOD_CRUSH,
  MOD_TELEFRAG,
  MOD_FALLING,
  MOD_SUICIDE,
  MOD_TARGET_LASER,
  MOD_TRIGGER_HURT,
  MOD_GRAPPLE,
  MOD_JUICED, // 0xA5A, do we still need this ?
#ifdef USE_KAMIKAZE
  MOD_KAMIKAZE
#endif
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum
{
  IT_BAD,
  IT_WEAPON,                  // EFX: rotate + upscale + minlight
  IT_AMMO,                    // EFX: rotate
  IT_ARMOR,                   // EFX: rotate + minlight
  IT_HEALTH,                  // EFX: static external sphere + rotating internal
  IT_POWERUP,                 // instant on, timer based
  IT_WEAPMOD,                 // weapon rounds based		//0xA5EA
#ifdef	WITH_BAGMAN_MOD
  IT_CASH,
  IT_CASH_STOLEN,
  IT_TEAM_SAFE,

#endif
  // EFX: rotate + external ring that rotates
  IT_HOLDABLE,                // single use, holdable item
                              // EFX: rotate + bob
  IT_PERSISTANT_POWERUP,
  IT_TEAM
} itemType_t;

//#define MAX_ITEM_MODELS 4

// 0xA5EA	ordering for the wordmodels, must be same order in bg_misc !!!
enum
{
  WORLD_GUNMODEL_POS = 0,       //1st person weapon model
  WORLD_HANDMODEL_POS,          //1st person hand model        //hypov8 todo: combine hand/wep models
  WORLD_FLASHMODEL_POS,         //1st/3rd person flash model
  WORLD_CLIPMODEL_POS,          //1st person ammo clip         // ^^ md3 suports multi materal & 10k vert ^^
  WORLD_PLAYERWEAPONMODEL_POS,  //3rd person player
  WORLD_WEAPONMODEL_POS,        //map weapon model
  MAX_ITEM_MODELS
};

typedef struct gitem_s
{
  const char *classname;          // spawning name
  const char *pickup_sound;
  const char *world_model[MAX_ITEM_MODELS];

  const char *icon;
  const char *pickup_name;        // for printing on pickup

  int quantity;             // for ammo how much, or duration of powerup
  itemType_t giType;        // IT_* flags

  int giTag;

  const char *precaches;          // string of all models and images this item will use
  const char *sounds;             // string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern gitem_t bg_itemlist[];
extern int bg_numItems;

gitem_t *BG_FindItem(const char *pickupName);
gitem_t *BG_FindItemForWeapon(weapon_t weapon);
gitem_t *BG_FindItemForPowerup(powerup_t pw);
gitem_t *BG_FindItemForHoldable(holdable_t pw);
int BG_WeaponMaxMagCount(int weapon);
int BG_WeaponMaxAmmoCount(int weapon);
void PM_BeginWeaponChange(int weapon); // add hypov8 weapon change link external


///qboolean PM_StopWeapChange(int oldweapon, int newweapon); //hypov8 add
int BG_AmmoCombineCheck(int weapon); //swap pistal ammo for tommy
qboolean BG_IsReloadableWeapon(int weapon);
animNumber_t BG_AttackTorsoAnim(int weapon);
#define ITEM_INDEX(x) ((x) - bg_itemlist)

qboolean BG_CanItemBeGrabbed(int gametype, const entityState_t *ent, const playerState_t *ps, int cashcollectmax, int cashstolenmax);
qboolean BG_CanPickupCash(int maxcash, const playerState_t *ps);
qboolean BG_CanPickupCashStolen(int maxcash, const playerState_t *ps); // add hypov8

// g_dmflags->integer flags
#define DF_NO_FALLING 8
#define DF_FIXED_FOV 16
#define DF_NO_FOOTSTEPS 32

// content masks
#define MASK_ALL (-1)
#define MASK_SOLID (CONTENTS_SOLID)
#define MASK_PLAYERSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_BODY)
#define MASK_DEADSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP)
#define MASK_WATER (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME)
#define MASK_OPAQUE (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA)
#define MASK_SHOT (CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE)
#define MASK_ROCKET (CONTENTS_MISSILECLIP) //add missile clip hypov8
#define MASK_SOLID_MISS (CONTENTS_SOLID | CONTENTS_MISSILECLIP)	//add hypov8 nodrop on missileclip brushes


// entityState_t->eType
typedef enum
{
  ET_GENERAL,
  ET_PLAYER,
  ET_ITEM,
  ET_MISSILE,
  ET_MOVER, //used in bot defines. must match be_aas_entity.cc
  //
  // DO NOT ADD ANYTHING ABOVE
  //
  ET_FLAMETHROWER_CHUNK,    // Added -KRYPTYK
  ET_BEAM,
  ET_PORTAL,
  ET_SPEAKER,
  ET_PUSH_TRIGGER,
  ET_TELEPORT_TRIGGER,
  ET_INVISIBLE,
  ET_GRAPPLE,             // grapple hooked on wall
  ET_TEAM, //FIXME(0xA5EA): wtf, bagman ??
  // EV_KAMIKAZE,                // kamikaze explodes
  ET_EVENTS               // any of the EV_* events can be added freestanding
                          // by setting eType to ET_EVENTS + eventNum
                          // this avoids having to set eFlags and eventNum
} entityType_t;

extern char const *bg_customTauntNames1[];
extern char const *bg_customTauntNames2[];
extern const int numCustomTaunts1;
extern const int numCustomTaunts2;

void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result);
void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, vec3_t result);
void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm, playerState_t *ps);
void BG_TouchJumpPad(playerState_t *ps, entityState_t *jumppad);
void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap);
void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s, int time, qboolean snap);
qboolean BG_PlayerTouchesItem(playerState_t *ps, entityState_t *item, int atTime);

void VectorToAngles(const vec3_t value1, vec3_t angles); //hypov8 merge:

qboolean BG_PlayerCanChangeWeapon(playerState_t *ps); //unvan .52
qboolean BG_InventoryContainsWeapon(int weapon, const int stats[ ]); //unvan .52
qboolean BG_InventoryContainsAmmo(int weapon, playerState_t *ps); //unvan .52



#define ARENAS_PER_TIER 4
#define MAX_ARENAS 1024
#define MAX_ARENAS_TEXT 8192

#define MAX_BOTS 1024
#define MAX_BOTS_TEXT 8192

#ifdef USE_KAMIKAZE
// Kamikaze
// 1st shockwave times
#define KAMI_SHOCKWAVE_STARTTIME 0
#define KAMI_SHOCKWAVEFADE_STARTTIME 1500
#define KAMI_SHOCKWAVE_ENDTIME 2000
// explosion/implosion times
#define KAMI_EXPLODE_STARTTIME 250
#define KAMI_IMPLODE_STARTTIME 2000
#define KAMI_IMPLODE_ENDTIME 2250
// 2nd shockwave times
#define KAMI_SHOCKWAVE2_STARTTIME 2000
#define KAMI_SHOCKWAVE2FADE_STARTTIME 2500
#define KAMI_SHOCKWAVE2_ENDTIME 3000
// radius of the models without scaling
#define KAMI_SHOCKWAVEMODEL_RADIUS 88
#define KAMI_BOOMSPHEREMODEL_RADIUS 72
// maximum radius of the models during the effect
#define KAMI_SHOCKWAVE_MAXRADIUS 1320
#define KAMI_BOOMSPHERE_MAXRADIUS 720
#define KAMI_SHOCKWAVE2_MAXRADIUS 704
#endif

#endif // BG_PUBLIC_H_
