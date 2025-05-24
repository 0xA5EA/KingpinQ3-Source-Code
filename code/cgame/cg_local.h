/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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
#ifndef CG_LOCAL_H_
#define CG_LOCAL_H_

#define NO_TYPE_PUNNING
// Id -way
//#include "../game/q_shared.h"
//#include "tr_types.h"	 // Id -way
// ioq3 way
#include "../qcommon/q_shared.h"
#include "../renderer/tr_types.h"

#include "../game/bg_public.h"
#include "cg_public.h"

#include "../../ui/menudef.h" /* 0xA5EA */

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.
#define CG_FONT_THRESHOLD 0.1

#define POWERUP_BLINKS 5

#define POWERUP_BLINK_TIME 1000
#define FADE_TIME 200
#define PULSE_TIME 200
#define DAMAGE_DEFLECT_TIME 100
#define DAMAGE_RETURN_TIME 400
#define DAMAGE_TIME 500
#define LAND_DEFLECT_TIME 150
#define LAND_RETURN_TIME 200 //hypov8 was 300
#define STEP_TIME 100 //hypov8 was 200
#define DUCK_TIME 100
#define PAIN_TWITCH_TIME 200
#define WEAPON_SELECT_TIME 1400
#define ITEM_SCALEUP_TIME 1000
#define ZOOM_TIME 150
#define ITEM_BLOB_TIME 200
#define MUZZLE_FLASH_TIME 20
#define MUZZLE_FLASH_TIME2 100 //add hypov8 flamer needs longer flash
#define MUZZLE_FLASH_TIME3 50 // crowbar flash
#define SINK_TIME 1000       // time for fragments to sink into ground before going away
#define ATTACKER_HEAD_TIME 10000
#define REWARD_TIME 3000

#define PULSE_SCALE 1.5      // amount to scale up the icons when activating

#define MAX_STEP_CHANGE 32

#define MAX_VERTS_ON_POLY 10
#define MAX_MARK_POLYS 512

#define STAT_MINUS 10        // num frame for '-' stats digit

#define ICON_SIZE 48         //12 /* 0xA5EA Ă¤ndert sich nix */
#define HUD_CHAR_WIDTH 24    /* 0xA5EA */
#define CHAR_WIDTH_ 32        //16	//32 10 /* 0xA5EA */
#define CHAR_HEIGHT 48       //24	//48 12 /* 0xA5EA */
#define TEXT_ICON_SPACE 4    //2 //4

#define TEAMCHAT_WIDTH 80
#define TEAMCHAT_HEIGHT 12   // org. 8	// 24/* 0xA5EA */
#define CHAT_HEIGHT 8        /* 0xA5EA, added */



// very large characters
#define GIANT_WIDTH 32
#define GIANT_HEIGHT 48

#define NUM_CROSSHAIRS 10

#define TEAM_OVERLAY_MAXNAME_WIDTH 12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH 16

#define DEFAULT_MODEL       "thug"
#define DEFAULT_MODEL_HEAD  "default"

#define DEFAULT_TEAM_MODEL  "thug" //hypov8 todo: teams?
#define DEFAULT_TEAM_HEAD   "default" //hypov8 todo: teams? head?

#define DEFAULT_MODEL_FEMALE "shina"


//for CG_DrawFps
#define FPS_FRAMES 4

#define NUM_PROGRESS 40

// Ridah, trails flamethrower
#define STYPE_STRETCH   0
#define STYPE_REPEAT    1

#define TJFL_FADEIN     ( 1 << 0 )
#define TJFL_CROSSOVER  ( 1 << 1 )
#define TJFL_NOCULL     ( 1 << 2 )
#define TJFL_FIXDISTORT ( 1 << 3 )
#define TJFL_SPARKHEADFLARE ( 1 << 4 )
#define TJFL_NOPOLYMERGE    ( 1 << 5 )

typedef struct
{
  char info[100];
  qboolean strong;
} progressInfo_t;

// --- cg_marks stuff -->
// particle flags
enum
{
  PF_UNDERWATER  = (1 << 0),
  PF_AIRONLY     = (1 << 1),
  PF_EMBER_SMOKE = (1 << 2),
};

typedef enum
{
  P_NONE,
  P_WEATHER,
  P_WEATHER_TURBULENT,
  P_WEATHER_FLURRY,
  P_FLAT,
  P_FLAT_SCALEUP,
  P_FLAT_SCALEUP_FADE,
  P_SMOKE,
  P_SMOKE_IMPACT,
  P_BLOOD,
  P_BUBBLE,
  P_BUBBLE_TURBULENT,
  P_SPRITE,
  P_SPARK,
  P_LIGHTSPARK
} particleType_t;

typedef struct particle_s
{
  particleType_t type;
  int flags;

  qhandle_t pshader;

  float time;
  float endTime;

  vec3_t org;
  vec3_t oldOrg;

  vec3_t vel;
  vec3_t accel;

  vec4_t color;
  vec4_t colorVel;

  float width;
  float height;

  float endWidth;
  float endHeight;

  float start;
  float end;

  float startfade;
  qboolean rotate;
  int snum;

  qboolean link;

  int roll;

  int accumroll;

  float bounceFactor;  // 0.0 = no bounce, 1.0 = perfect

  struct particle_s *next;
} cparticle_t;

typedef enum
{
  FOOTSTEP_NORMAL,
  FOOTSTEP_BOOT,
  FOOTSTEP_FLESH,
  FOOTSTEP_MECH,
  FOOTSTEP_ENERGY,
  FOOTSTEP_METAL,                 /* */
  FOOTSTEP_SPLASH,                /* Water, detected by waterlevel, not via surfaceparm */
  /* 0xA5EA -----------> */
  FOOTSTEP_METALLIGHT,
  FOOTSTEP_MARBLE,
  FOOTSTEP_PUDDLE,
  FOOTSTEP_RUG,
  FOOTSTEP_SNOW,
  FOOTSTEP_TIN,
  FOOTSTEP_WOOD,
  FOOTSTEP_GRAVEL,
  FOOTSTEP_CRASS,
  FOOTSTEP_LADDER,
  FOOTSTEP_GLASS,
  /* <----------- 0xA5EA  */

  FOOTSTEP_TOTAL
} footstep_t;

typedef enum
{
  IMPACTSOUND_DEFAULT,
  IMPACTSOUND_METAL,
  IMPACTSOUND_FLESH,
  IMPACTSOUND_METALLIGHT,
  IMPACTSOUND_WOOD,           /* 0xA5EA */
  IMPACTSOUND_GRAVEL,
  IMPACTSOUND_SNOW,
  IMPACTSOUND_EARTH,
  IMPACTSOUND_GLASS
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation

typedef struct
{
  int oldFrame;
  int oldFrameTime;          // time when ->oldFrame was exactly on

  int frame;
  int frameTime;             // time when ->frame will be exactly on

  qboolean animationEnded;

  float backlerp;

  float yawAngle;
  qboolean yawing;
  float pitchAngle;
  qboolean pitching;

  int animationNumber;       // may include ANIM_TOGGLEBIT
  animation_t *animation;
  int animationTime;         // time when the first frame of the animation will be exact
  float animationScale;

  //added for smooth blending between animations on change

  int old_animationNumber;   // may include ANIM_TOGGLEBIT
  animation_t *old_animation;

  float blendlerp;
  float blendtime;

  int weaponNumber;
  int old_weaponNumber;

  //sync torso new animation with legs (run/walk)
  qboolean mergeTorsoAnims;
  int mergeTorsoFrame;
  int mergeTorsoOldFrame;

} lerpFrame_t;
// debugging values:
extern int   debug_anim_current;
extern int   debug_anim_old;
extern float debug_anim_blend;
typedef struct
{
  lerpFrame_t legs, torso, flag, weapon/*, nonseg*/; //0xA5EA: weapon added
  int painTime;
  int painDirection; // flip from 0 to 1
  int lightningFiring;

  // railgun trail spawning
  //vec3_t railgunImpact;
  //qboolean railgunFlash;

  // machinegun spinning
  float barrelAngle;
  int barrelTime;
  qboolean barrelSpinning;

  float gunGlowPercent[MAX_WEAPONS]; //weapon glow sfx
  int isModded[MAX_WEAPONS];

  // death effect
  int deathTime;
  float deathScale;
//add xreal hypov8
    // wallwalk
  vec3_t          lastNormal;
  vec3_t          lastAxis[3];
  //smooth_t        sList[MAXSMOOTHS]; //edit hypov8
} playerEntity_t;

//=================================================
// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s
{
  entityState_t currentState;  // from cg.frame
  entityState_t nextState;     // from cg.nextFrame, if available
  qboolean interpolate;        // true if next is valid to interpolate to
  qboolean currentValid;       // true if cg.frame holds this entity

  int muzzleFlashTime;         // move to playerEntity?
  int previousEvent;
  int teleportFlag;

  int trailTime;               // so missile trails can handle dropped initial packets
  int dustTrailTime;
  int miscTime;

  int snapShotTime;            // last time this entity was found in a snapshot

  playerEntity_t pe;

  int errorTime;               // decay the error from this time
  vec3_t errorOrigin;
  vec3_t errorAngles;

  qboolean extrapolated;       // false if origin / angles is an interpolation
  vec3_t rawOrigin;
  vec3_t rawAngles;

  vec3_t beamEnd;

  // exact interpolated position of entity on this frame
  vec3_t lerpOrigin;
  vec3_t lerpAngles;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s
{
  struct markPoly_s *prevMark, *nextMark;
  int time;
  qhandle_t markShader;
  qboolean alphaFade;     // fade alpha instead of rgb
  float color[4];
  poly_t poly;
  polyVert_t verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum
{
  LE_MARK,
  LE_EXPLOSION,
  LE_SPRITE_EXPLOSION,
  LE_SPRITE_FIREBALL, //add hypov8. sizeable explosion
  LE_SPRITE_EMBER, //add hypov8 smaller
  LE_FRAGMENT,
  LE_MOVE_SCALE_FADE,
  LE_FALL_SCALE_FADE,
  LE_FADE_RGB,
  LE_SCALE_FADE,
  LE_SCOREPLUM,
#ifdef USE_KAMIKAZE
  LE_KAMIKAZE,
#endif
#if 0
  LE_INVULIMPACT,
  LE_INVULJUICED,
  LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum
{
  LEF_PUFF_DONT_SCALE = 0x0001,          // do not scale size over time
  LEF_TUMBLE          = 0x0002,          // tumble over time, used for ejecting shells
  LEF_EMBER           = 0x0004,          //reduce only z velocity on bounce
#ifdef USE_KAMIKAZE
  LEF_SOUND1          = 0x0004,          // sound 1 for kamikaze
  LEF_SOUND2          = 0x0008           // sound 2 for kamikaze
#endif
} leFlag_t;

typedef enum
{
  LEMT_NONE,
  LEMT_BURN,
  LEMT_BLOOD
} leMarkType_t;                            // fragment local entities can leave marks on walls

typedef enum
{
  LEBS_NONE,
  LEBS_BLOOD,
  LEBS_BRASS
} leBounceSoundType_t;                     // fragment local entities can make sounds on impacts

typedef struct localEntity_s
{
  struct localEntity_s *prev, *next;
  leType_t leType;
  int leFlags;

  int startTime;
  int endTime;
  int fadeInTime;

  float lifeRate;                        // 1.0 / (endTime - startTime)

  trajectory_t pos;
  trajectory_t angles;

  float bounceFactor;                    // 0.0 = no bounce, 1.0 = perfect

  float color[4];

  float radius;

  float light;
  vec3_t lightColor;

  leMarkType_t leMarkType;               // mark to leave on fragment impact
  leBounceSoundType_t leBounceSoundType;

  refEntity_t refEntity;
} localEntity_t;

//======================================================================


typedef struct
{
  int client;
  int score;
  int frags;                             // 0xA5EA added
  int deaths;                            // 0xA5EA added
  int ping;
  int time;
  int scoreFlags;
  int powerUps;
  int accuracy;
  int impressiveCount;
  int excellentCount;
  int guantletCount;
  int defendCount;
  int assistCount;
  int captures;
  qboolean perfect;
  int team;
} score_t;


// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define MAX_CUSTOM_SOUNDS 19 //32

typedef struct
{
  qboolean infoValid;

  char name[MAX_QPATH];
  team_t team;

  int botSkill;           // 0 = not bot, 1-5 = bot

  vec3_t color1;
  vec3_t color2;

  int score;              // updated by score servercmds
  int location;           // location index for team mode
  int health;             // you only get this info about your teammates
  int armor;
  int curWeapon;

  int handicap;
  int wins, losses;       // in tourney mode

  int teamTask;           // task in teamplay (offence/defence)
  qboolean teamLeader;    // true when this is a team leader

  int powerups;           // so can display quad/flag status

  int medkitUsageTime;
  int invulnerabilityStartTime;
  int invulnerabilityStopTime;

  int breathPuffTime;

  // when clientinfo is changed, the loading of models/skins/sounds
  // can be deferred until you are dead, to prevent hitches in
  // gameplay
  char modelName[MAX_QPATH];
  char skinName[MAX_QPATH];
  char headModelName[MAX_QPATH];
  char headSkinName[MAX_QPATH];
  char dragonTeam[MAX_TEAMNAME];
  char nikkiTeam[MAX_TEAMNAME];
  qboolean deferred;

  qboolean iqm;           // true if model is an iqm model
  qboolean newAnims;      // true if using the new mission pack animations
  qboolean fixedlegs;     // true if legs yaw is always the same as torso yaw
  qboolean fixedtorso;    // true if torso never changes yaw
  qboolean nonsegmented;  // true if model is Q2 style nonsegmented
  qboolean md5;           // true if model is in the md5 model format

  vec3_t headOffset;      // move head in icon views
  footstep_t footsteps;
  gender_t gender;        // from model

  qhandle_t legsModel;
  qhandle_t legsAnimation;
  qhandle_t legsSkin;

  qhandle_t torsoModel;
  qhandle_t torsoAnimation;
  qhandle_t torsoSkin;

  qhandle_t headModel;
  qhandle_t headSkin;

  qhandle_t nonSegModel; //non-segmented model system
  qhandle_t nonSegSkin; //non-segmented model system

  qhandle_t bodyModel; //md5 model format
  qhandle_t bodySkin; //md5 model format

  qhandle_t modelIcon;

  animation_t animations[MAX_TOTALANIMATIONS];

  sfxHandle_t sounds[MAX_CUSTOM_SOUNDS];

  vec_t       modelScale;

  int    firstTorsoControlBone;
  int    torsoControlBone;
  int    neckControlBone;
  char   neckControlBoneName[MAX_QPATH]; //add hypov8 needed for lerp sprites
#if 1 //def COMPAT_KPQ3
  vec3_t wepRotate;   //add hypov8. rotate bone xyz to suit weapon orientation
  vec3_t flagRotate;  //add hypov8. rotate bone xyz to suit flag orientation
#endif
  //int         legBones[ MAX_BONES ]; //unordered
  int         numLegBones;
  qboolean    isLegBone[ MAX_BONES ];

  //int         weaponAdjusted; // bitmask of all weapons that have hand deltas
  //int         handBones[ MAX_BONES ];
  //int         numHandBones;

  char        voice[ 64 ];
  sfxHandle_t customFootsteps[ 4 ];
  sfxHandle_t customMetalFootsteps[ 4 ];
  int         voiceTime;
} clientInfo_t;


// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s
{
  qboolean registered;
  gitem_t *item;
  qboolean         md5;

  qhandle_t tagModel;					// tags don't actually draw, they just position the muzzle flash/light
  //match bg gitem_t order
  //todo: cleanup this (MAX_ITEM_MODELS);
  qhandle_t weaponModel;				//1st person weapon model
  qhandle_t handModel;					//1st person hand model
  qhandle_t flashModel;					//1st/3rd person flash model
  qhandle_t ammoClipModel;				//1st person ammo clip
  qhandle_t worldPlayerWeaponModel;     //3rd person player
  qhandle_t worldWeaponModel;			//map weapon model

  qhandle_t mod1WeaponModel;
  qhandle_t mod2WeaponModel;
  qhandle_t mod3WeaponModel;

  qhandle_t weaponIcon;
  qhandle_t ammoIcon;
  qhandle_t ammoModel; //world/hud ammo clip
  qhandle_t barrelModel;

  vec3_t weaponMidpoint;                   // so it will rotate centered instead of by tag
  vec3_t eweaponMidpoint;                  // 0xA5EA

  weaponstate_t weaponAnimState;
  animation_t  animations[MAX_WEAPON_STATES];

  float flashDlight;
  vec3_t flashDlightColor; //muzzleflash color
  sfxHandle_t flashSound[MAX_FLASH_SOUNDS];   // fast firing weapons randomly choose	0xA5EA, war 2

  qhandle_t missileModel;
  sfxHandle_t missileSound;
  void (*missileTrailFunc)(centity_t *, const struct weaponInfo_s *wi);
  float missileDlight;
  vec3_t missileDlightColor[3];
  int missileRenderfx;
  qhandle_t customSmokePuffshader;         //0xA5EA

  void (*ejectBrassFunc)(centity_t *);

  float trailRadius;
  float wiTrailTime;

  sfxHandle_t readySound; //idle weapon sound
  sfxHandle_t firingSound; //constant weapon sound(flamer, hook)
  //qboolean loopFireSound; //unused

  sfxHandle_t reloadSound;

#if 1 //md5 wep
  vec3_t           rotation;
  vec3_t           posOffs;
  char             rotationBone[ 50 ];
  vec_t            scale;
#endif
} weaponInfo_t;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct
{
  qboolean registered;
  qhandle_t models[MAX_ITEM_MODELS];
  qhandle_t icon;
} itemInfo_t;


typedef struct
{
  int itemNum;
} powerupInfo_t;


#define MAX_SKULLTRAIL 10

typedef struct
{
  vec3_t positions[MAX_SKULLTRAIL];
  int numpositions;
} skulltrail_t;


#define MAX_REWARDSTACK 10
#define MAX_SOUNDBUFFER 20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS 16

//unlagged - optimized prediction
#define NUM_SAVED_STATES (CMD_BACKUP + 2)
//unlagged - optimized prediction
typedef struct
{
  int clientFrame;                    // incremented each frame

  int clientNum;

  qboolean demoPlayback;
  qboolean levelShot;                 // taking a level menu screenshot
  int deferredPlayerLoading;
  qboolean loading;                   // don't defer players at initial startup
  qboolean intermissionStarted;       // don't play voice rewards, because game will end shortly

  // there are only one or two snapshot_t that are relevant at a time
  int latestSnapshotNum;              // the number of snapshots the client system has received
  int latestSnapshotTime;             // the time from latestSnapshotNum, so we don't need to read the snapshot yet

  snapshot_t *snap;                   // cg.snap->serverTime <= cg.time
  snapshot_t *nextSnap;               // cg.nextSnap->serverTime > cg.time, or NULL
  snapshot_t activeSnapshots[2];

  float frameInterpolation;           // (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

  qboolean thisFrameTeleport;
  qboolean nextFrameTeleport;

  int frametime;                      // cg.time - cg.oldTime

  int time;                           // this is the time value that the client
  int weapontimer_hm;                    // hitmen weapon timer
  // is rendering at.
  int oldTime;                        // time at last frame, used for missile trails and prediction checking

  int physicsTime;                    // either cg.snap->time or cg.nextSnap->time

  int timelimitWarnings;              // 5 min, 1 min, overtime
  int fraglimitWarnings;

  qboolean mapRestart;                // set on a map restart to set back the weapon

  qboolean renderingThirdPerson;      // during deaths, chasecams, etc

  // prediction state
  qboolean hyperspace;                // true if prediction has hit a trigger_teleport
  playerState_t predictedPlayerState;
  centity_t predictedPlayerEntity;
  qboolean validPPS;                  // clear until the first call to CG_PredictPlayerState
  int predictedErrorTime;
  vec3_t predictedError;

  int eventSequence;
  int predictableEvents[MAX_PREDICTED_EVENTS];

  float stepChange;                   // for stair up smoothing
  int stepTime;

  float duckChange;                   // for duck viewheight smoothing
  int duckTime;

  float landChange;                   // for landing hard
  int landTime;

  // input state sent to server
  int weaponSelect;

  // auto rotating items
  vec3_t autoAngles;
  vec3_t autoAxis[3];
  vec3_t autoAnglesFast;
  vec3_t autoAxisFast[3];

  // view rendering
  refdef_t refdef;
  vec3_t refdefViewAngles;            // will be converted to refdef.viewaxis

  // zoom key
  qboolean zoomed;
  int zoomTime;
  float zoomSensitivity;

  // information screen text during loading
//	char		infoScreenText[MAX_STRING_CHARS];

  // scoreboard
  int scoresRequestTime;
  int numScores;
  int selectedScore;
  int teamScores[2];
  score_t scores[MAX_CLIENTS];
  qboolean showScores;
  qboolean scoreBoardShowing;
  int scoreFadeTime;
  char killerName[MAX_NAME_LENGTH];
  char spectatorList[MAX_STRING_CHARS];                          // list of names
  int spectatorLen;                                              // length of list
  float spectatorWidth;                                          // width in device units
  int spectatorTime;                                             // next time to offset
  int spectatorPaintX;                                           // current paint x
  int spectatorPaintX2;                                          // current paint x
  int spectatorOffset;                                           // current offset from start
  int spectatorPaintLen;                                         // current offset from start

  // skull trails
  skulltrail_t skulltrails[MAX_CLIENTS];

  // centerprinting
  int centerPrintTime;
  int centerPrintCharWidth;
  int centerPrintY;
  char centerPrint[1024];
  int centerPrintLines;

  // low ammo warning state
  int lowAmmoWarning;                                            // 1 = low, 2 = empty

  // kill timers for carnage reward
  int lastKillTime;

  // crosshair client ID
  int crosshairClientNum;
  int crosshairClientTime;

  // powerup active flashing
  int powerupActive;
  int powerupTime;

  // attacking player
  int attackerTime;
  int voiceTime;

  // reward medals
  int rewardStack;
  int rewardTime;
  int rewardCount[MAX_REWARDSTACK];
  qhandle_t rewardShader[MAX_REWARDSTACK];
  qhandle_t rewardSound[MAX_REWARDSTACK];

  // sound buffer mainly for announcer sounds
  int soundBufferIn;
  int soundBufferOut;
  int soundTime;
  qhandle_t soundBuffer[MAX_SOUNDBUFFER];
  int shootTimeFlamer;
  byte flameFxRotate; //spawn sfx rotate?

  // for voice chat buffer
  int voiceChatTime;
  int voiceChatBufferIn;
  int voiceChatBufferOut;

  // warmup countdown
  int warmup;
  int warmupCount;

  //==========================
  int itemPickup;
  int itemPickupTime; //hypov8 hud picup icon
  int itemPickupBlendTime;   // the pulse around the crosshair is timed seperately

  int weaponSelectTime;
  int weaponAnimation;
  int weaponAnimationTime;

  // blend blobs
  float damageTimeBlood, damageValueBlood, damageTimeFlamer;
  float damageTime;
  float damageX, damageY;

  // status bar head
  float headYaw;
  float headEndPitch;
  float headEndYaw;
  int headEndTime;
  float headStartPitch;
  float headStartYaw;
  int headStartTime;

  // view movement
  float v_dmg_time;
  float v_dmg_pitch;
  float v_dmg_roll;

  vec3_t kick_angles;   // weapon kicks
  vec3_t kick_origin;

  // temp working variables for player view
  float bobfracsin;
  int bobcycle;
  float xyspeed;
  int nextOrbitTime;

  //qboolean cameraMode;		// if rendering from a loaded camera


  // development tool
  refEntity_t testModelEntity;
  char testModelName[MAX_QPATH];
  qboolean testGun;

  // this will only change the skeleton of testModelEntity
  char testAnimationName[MAX_QPATH];
  qhandle_t testAnimation;

  char testAnimation2Name[MAX_QPATH];
  qhandle_t testAnimation2;
  refSkeleton_t testAnimation2Skeleton;

  // play with doom3 style light materials
  refLight_t testLight;
  char testLightName[MAX_QPATH];
  qboolean testFlashLight;

  // information screen text during loading
  progressInfo_t progressInfo[NUM_PROGRESS];
  int progress;

//unlagged - optimized prediction
  int			lastPredictedCommand;
  int			lastServerTime;
  playerState_t savedPmoveStates[NUM_SAVED_STATES];
  int			stateHead, stateTail;
  int         ping;
//unlagged - optimized prediction

  // pmove params
  struct {
    int synchronous;
    int fixed;
    int msec;
    int accurate;
  } pmoveParams;

} cg_t;

#define NUM_WOODIMPACTSND 7
#define NUM_METALIMPACTSND 7
#define NUM_SHOTTY_METALIMPACTSND 3
// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct
{
  qhandle_t charsetShader;
  qhandle_t whiteShader;
  qhandle_t outlineShader;

#ifdef GT_USE_TA_TYPES
  qhandle_t redCubeModel;
  qhandle_t blueCubeModel;
  qhandle_t redCubeIcon;
  qhandle_t blueCubeIcon;
#endif

  qhandle_t dragonFlagModel;
  qhandle_t nikkiFlagModel;
  qhandle_t neutralFlagModel;
  qhandle_t dragonFlagShader[3];
  qhandle_t nikkiFlagShader[3];
  qhandle_t flagShader[4];

  qhandle_t flagPoleModel;
  qhandle_t flagFlapModel;

  qhandle_t dragonFlagFlapSkin;
  qhandle_t nikkiFlagFlapSkin;
  qhandle_t neutralFlagFlapSkin;

  qhandle_t dragonFlagBaseModel;
  qhandle_t nikkiFlagBaseModel;
  qhandle_t neutralFlagBaseModel;

  qhandle_t dragonsSafeBaseModel;
  qhandle_t nikkisSafeBaseModel;
  qhandle_t cashStolenModel; // add hypov8

  //qhandle_t cashStolenSkin; // add hypov8

#ifdef GT_USE_TA_TYPES
  qhandle_t overloadBaseModel;
  qhandle_t overloadTargetModel;
  qhandle_t overloadLightsModel;
  qhandle_t overloadEnergyModel;

  qhandle_t harvesterModel;
  qhandle_t harvesterRedSkin;
  qhandle_t harvesterBlueSkin;
  qhandle_t harvesterNeutralModel;
#endif

  qhandle_t armorModel[3];
  qhandle_t armorIcon[7]; //hypov8 add


  qhandle_t teamStatusBar;

  qhandle_t deferShader;

  //effect shaders
  qhandle_t unlinkEffect;

  // gib explosions
  qhandle_t gibArm;
  qhandle_t gibChest;
  qhandle_t gibFist;
  qhandle_t gibFoot;
  qhandle_t gibForearm;
  qhandle_t gibIntestine;
  qhandle_t gibLeg;
  qhandle_t gibSkull;
  qhandle_t gibeye;
  qhandle_t smoke2;

  qhandle_t machinegunBrassModel;
  qhandle_t rifleBrassModel;
  qhandle_t shotgunBrassModel;

  qhandle_t railRingsShader;
  qhandle_t railCoreShader;

  qhandle_t lightningShader;

  qhandle_t shadowProjectedLightShader;

  qhandle_t flashLightShader;
  qhandle_t flashLightModel; //add hypov8

  //qhandle_t friendShader;
  qhandle_t teamDragon; //add hypov8 team sprites above head
  qhandle_t teamNikki; //add hypov8 team sprites above head

  qhandle_t balloonShader;
  qhandle_t connectionShader;

  qhandle_t selectShader;
  qhandle_t viewBloodShader;
  qhandle_t tracerShader;
  qhandle_t crosshairShader[NUM_CROSSHAIRS];
  qhandle_t lagometerShader;
  qhandle_t backTileShader;
  qhandle_t noammoShader;

  qhandle_t smokeGLShader[6]; //add hypov8

  qhandle_t smokePuffShader;
  qhandle_t	smokePuffRlShader;
//	qhandle_t	smokePuffRageProShader;
  qhandle_t shotgunSmokePuffShader;
#if 0
  qhandle_t plasmaBallShader;
#endif
  qhandle_t waterBubbleShader;
  qhandle_t bloodTrailShader;
#if 0
  qhandle_t nailPuffShader;
  qhandle_t blueProxMine;
#endif

  qhandle_t numberShaders[11];

  qhandle_t shadowMarkShader;

  qhandle_t botSkillShaders[5];

  // wall mark shaders
  qhandle_t wakeMarkShader;
  qhandle_t bloodMarkShader[3];
  qhandle_t bloodSpurtShader[3];
  qhandle_t sparkShader;

  qhandle_t bulletMark[2];
  qhandle_t bulletMarkmetal[2];
  qhandle_t bulletMarkwood[2];
  qhandle_t bulletMarkglas[2];

  qhandle_t burnMarkShader;
  qhandle_t burnMarkShaderRl;
  qhandle_t holeMarkShader;
  qhandle_t energyMarkShader;
  qhandle_t teleportFlareShader;
  //hypov8 todo: crowbar/pipe marks
  qhandle_t meleeMarkBrick[2];
  qhandle_t meleeMarkMetal[2];
  qhandle_t meleeMarkWood[2];
  qhandle_t meleeMarkGlass[2];

#if 0
  // powerup shaders
  qhandle_t quadShader;
  qhandle_t redQuadShader;
  qhandle_t quadWeaponShader;
  qhandle_t invisShader;
  qhandle_t regenShader;
  qhandle_t battleSuitShader;
  qhandle_t battleWeaponShader;
#endif
  qhandle_t hastePuffShader;
#ifdef USE_KAMIKAZE
  qhandle_t redKamikazeShader;
  qhandle_t blueKamikazeShader;
#endif

  // weapon effect models
  qhandle_t	bulletFlashModel;
//	qhandle_t	ringFlashModel;
  qhandle_t dishFlashModel;
  qhandle_t lightningExplosionModel;

  //qhandle_t grenadeExplodeModel; //grenade


  // weapon effect shaders
  qhandle_t railExplosionShader;
  qhandle_t plasmaExplosionShader;
//	qhandle_t	bulletExplosionShader;	//// 0xA5EA, removed, particle systems does its work
  qhandle_t rocketExplosionShader;
  qhandle_t grenadeExplosionShader;
  qhandle_t bfgExplosionShader;
  qhandle_t bloodExplosionShader;

  //qhandle_t rocketFirePartShader;
  qhandle_t grenadeExplodeShader1;
  //qhandle_t grenadeExplodeShader2;
  qhandle_t grenadeEmberShader;


  // special effects models
  qhandle_t teleportEffectModel;
  qhandle_t teleportEffectShader;
#ifdef USE_KAMIKAZE
  qhandle_t kamikazeEffectModel;
  qhandle_t kamikazeShockWave;
  qhandle_t kamikazeHeadModel;
  qhandle_t kamikazeHeadTrail;
#endif
//	qhandle_t	guardPowerupModel;
//	qhandle_t	scoutPowerupModel;
//	qhandle_t	doublerPowerupModel;
//	qhandle_t	ammoRegenPowerupModel;
  qhandle_t invulnerabilityImpactModel;
  qhandle_t invulnerabilityJuicedModel;
  qhandle_t medkitUsageModel;
  qhandle_t dustPuffShader;
  qhandle_t heartShader;
  qhandle_t invulnerabilityPowerupModel;
#if 0
  // scoreboard headers
  qhandle_t scoreboardName;
  qhandle_t scoreboardPing;
  qhandle_t scoreboardScore;
  qhandle_t scoreboardTime;
#endif
  // medals shown during gameplay
  qhandle_t medalImpressive;
  qhandle_t medalExcellent;
  qhandle_t medalGauntlet;
  qhandle_t medalDefend;
  qhandle_t medalAssist;
  qhandle_t medalCapture;

  qhandle_t menuback, load0, load1, detail;
  // sounds
//	sfxHandle_t	quadSound;						/* 0xA5EA */
  sfxHandle_t tracerSound;
  sfxHandle_t selectSound;
  sfxHandle_t useNothingSound;
  sfxHandle_t wearOffSound;
  sfxHandle_t footsteps[FOOTSTEP_TOTAL][4];
  //sfxHandle_t landSound;					/* 0xA5EA */
  sfxHandle_t landSounds[FOOTSTEP_TOTAL];

  sfxHandle_t sfx_Pipehbody;
  sfxHandle_t sfx_Pipehcboard;
  sfxHandle_t sfx_Pipehcement;
  sfxHandle_t sfx_Pipehgravel;
  sfxHandle_t sfx_Pipehwood;
  sfxHandle_t sfx_Pipehhead;
  sfxHandle_t sfx_Pipehtile;
  sfxHandle_t sfx_Pipehtin;

  sfxHandle_t sfx_HookHitBrick; //WP_GRAPPLING_HOOK

  sfxHandle_t sfx_lghit1;
  sfxHandle_t sfx_lghit2;
  sfxHandle_t sfx_lghit3;
  //sfxHandle_t	sfx_ric1;
  //sfxHandle_t	sfx_ric2;
  //sfxHandle_t	sfx_ric3;
  sfxHandle_t sfx_ShottyhitMetal[NUM_SHOTTY_METALIMPACTSND];
  sfxHandle_t sfx_Shottyhit[8];

//	sfxHandle_t sfx_ShottyhitWood[NUM_WOODIMPACTSND];
  sfxHandle_t sfx_ShottyhitIce[2];
  sfxHandle_t sfx_ShottyhitEarth[2];
  sfxHandle_t sfx_ric[8];                 /* 0xA5EA */
  sfxHandle_t sfx_BullethWood[NUM_WOODIMPACTSND];
  sfxHandle_t sfx_BullethMetal[NUM_METALIMPACTSND];
  sfxHandle_t sfx_BullethEarth[7];
//	sfxHandle_t sfx_smallBullethWood[2];
  sfxHandle_t sfx_BullethIce[2];
  sfxHandle_t sfx_rockexp[2];
  sfxHandle_t sfx_plasmaexp;
  sfxHandle_t sfx_GrenExpl[3];            /* 0xA5EA */
  sfxHandle_t sfx_proxexp;
  sfxHandle_t sfx_nghit;
  sfxHandle_t sfx_nghitflesh;
  sfxHandle_t sfx_nghitmetal;
  sfxHandle_t sfx_chghit;
  sfxHandle_t sfx_chghitflesh;
  sfxHandle_t sfx_chghitmetal;
#ifdef USE_KAMIKAZE
  sfxHandle_t kamikazeExplodeSound;
  sfxHandle_t kamikazeImplodeSound;
  sfxHandle_t kamikazeFarSound;
#endif
#if 0
  sfxHandle_t useInvulnerabilitySound;
  sfxHandle_t invulnerabilityImpactSound1;
  sfxHandle_t invulnerabilityImpactSound2;
  sfxHandle_t invulnerabilityImpactSound3;
  sfxHandle_t invulnerabilityJuicedSound;
  sfxHandle_t obeliskHitSound1;
  sfxHandle_t obeliskHitSound2;
  sfxHandle_t obeliskHitSound3;
  sfxHandle_t obeliskRespawnSound;
#endif
  //FIXME (0xA5EA): cleanup
  sfxHandle_t winnerSound;
  sfxHandle_t loserSound;
  sfxHandle_t youSuckSound;
  sfxHandle_t gibSound;
  sfxHandle_t gibBounce1Sound;
  sfxHandle_t gibBounce2Sound;
  sfxHandle_t gibBounce3Sound;
  sfxHandle_t teleInSound;
  sfxHandle_t teleOutSound;
  sfxHandle_t noAmmoSound;
  sfxHandle_t respawnSound;
  sfxHandle_t talkSound;

  sfxHandle_t fallSound;
  sfxHandle_t jumpPadSound;

  sfxHandle_t oneMinuteSound;
  sfxHandle_t fiveMinuteSound;
  sfxHandle_t suddenDeathSound;

  sfxHandle_t threeFragSound;
  sfxHandle_t twoFragSound;
  sfxHandle_t oneFragSound;

  sfxHandle_t hitSound;

  //FIXME(0xA5EA): use other hitsounds as well ?
#ifdef USE_EXTENTED_HIT_SOUNDS
  sfxHandle_t hitTeamSound;
#endif
  sfxHandle_t impressiveSound;
  sfxHandle_t excellentSound;
  sfxHandle_t deniedSound;
  sfxHandle_t humiliationSound;
  sfxHandle_t assistSound;
  sfxHandle_t defendSound;
  sfxHandle_t firstExcellentSound;

  sfxHandle_t takenLeadSound;
  sfxHandle_t tiedLeadSound;
  sfxHandle_t lostLeadSound;

  sfxHandle_t voteNow;
  sfxHandle_t votePassed;
  sfxHandle_t voteFailed;

  sfxHandle_t watrInSound;
  sfxHandle_t watrOutSound;
  sfxHandle_t watrUnSound;
#ifdef GT_USE_TA_TYPES
  sfxHandle_t flightSound;
#endif
  sfxHandle_t medkitSound;

  sfxHandle_t weaponHoverSound;

  // teamplay sounds
  sfxHandle_t captureAwardSound;
  sfxHandle_t redScoredSound;
  sfxHandle_t blueScoredSound;
  sfxHandle_t redLeadsSound;
  sfxHandle_t blueLeadsSound;
  sfxHandle_t teamsTiedSound;

  sfxHandle_t captureYourTeamSound;
  sfxHandle_t captureOpponentSound;
  sfxHandle_t returnYourTeamSound;
  sfxHandle_t returnOpponentSound;
  sfxHandle_t takenYourTeamSound;
  sfxHandle_t takenOpponentSound;

  sfxHandle_t redFlagReturnedSound;
  sfxHandle_t blueFlagReturnedSound;
  sfxHandle_t neutralFlagReturnedSound;
  sfxHandle_t enemyTookYourFlagSound;
  sfxHandle_t enemyTookTheFlagSound;
  sfxHandle_t yourTeamTookEnemyFlagSound;
  sfxHandle_t yourTeamTookEnemyBagSound; // hypov8 stolen cash im BM
  sfxHandle_t yourTeamTookTheFlagSound;
  sfxHandle_t youHaveFlagSound;
  sfxHandle_t yourBaseIsUnderAttackSound;
  sfxHandle_t holyShitSound;

#ifdef WITH_BAGMAN_MOD
  sfxHandle_t bmDepositSound; // hypov8 stolen cash im BM
  sfxHandle_t bmStolenSound; // hypov8 stolen cash im BM
#endif

  // tournament sounds
  sfxHandle_t count3Sound;
  sfxHandle_t count2Sound;
  sfxHandle_t count1Sound;
  sfxHandle_t countFightSound;
  sfxHandle_t countPrepareSound;
  sfxHandle_t countPrepareTeamSound;

  qhandle_t flagShaders[3];
#ifdef GT_USE_TA_TYPES
  // new stuff
  qhandle_t patrolShader;
  qhandle_t assaultShader;
  qhandle_t campShader;
  qhandle_t followShader;
  qhandle_t defendShader;
  qhandle_t teamLeaderShader;
  qhandle_t retrieveShader;
  qhandle_t escortShader;
#endif

#if 0
  sfxHandle_t ammoregenSound;
  sfxHandle_t doublerSound;
  sfxHandle_t guardSound;
  sfxHandle_t scoutSound;
#endif
  qhandle_t cursor;
  qhandle_t selectCursor;
  qhandle_t sizeCursor;

  //sfxHandle_t regenSound;
  //sfxHandle_t protectSound;
  sfxHandle_t n_healthSound;
  sfxHandle_t hgrenb1aSound;
  sfxHandle_t hgrenb2aSound;
  sfxHandle_t hgrenb3aSound;      /* 0xA5EA */
  sfxHandle_t hgrenbwood1Sound;   /* 0xA5EA */
  sfxHandle_t hgrenbwood2Sound;   /* 0xA5EA */
  sfxHandle_t hgrenbmetal1Sound;  /* 0xA5EA */
  sfxHandle_t hgrenbmetal2Sound;  /* 0xA5EA */
  sfxHandle_t wstbimplSound;
  sfxHandle_t wstbimpmSound;
  sfxHandle_t wstbimpdSound;
  sfxHandle_t wstbactvSound;
#ifdef USE_FLAMEGUN
  // Ridah flamethrower
  sfxHandle_t flameSound;
  sfxHandle_t flameScreenSound; //flamescreen
  sfxHandle_t flameBlowSound;
  sfxHandle_t flameStartSound;
  sfxHandle_t flameStreamSound;
  sfxHandle_t flameCrackSound;

  qhandle_t flamethrowerFireStream;
  qhandle_t flamethrowerBlueStream;
  qhandle_t flamethrowerFuelStream;
  qhandle_t flamethrowerFuelShader;
  qhandle_t sparkFlareShader;

  qhandle_t onFireShader1;
  qhandle_t onFireShader2;

  qhandle_t nerveTestShader;
  qhandle_t idTestShader;

#endif
  fontInfo_t freeSansBoldFont;
  fontInfo_t freeSerifBoldFont;
  fontInfo_t consoleFont;
  fontInfo_t freeSansFont;

  fontInfo_t hudNumberFont;

} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct
{
  gameState_t gameState;          // gamestate from server
  glconfig_t glconfig;            // rendering configuration
  glconfig2_t glconfig2;
  float screenXScale;             // derived from glconfig
  float screenYScale;
  float screenXBias;

  int serverCommandSequence;      // reliable command stream counter
  int processedSnapshotNum;       // the number of snapshots cgame has requested

  qboolean localServer;           // detected on startup by checking sv_running

  // parsed from serverinfo
  gametype_t gametype;
  int weaponmod;                  // 0xA5EA, hitmen and realmode
  int cursemode;                  // 0xA5EA
  int dmflags;
  int teamflags;
  int fraglimit;
  int capturelimit;
#ifdef WITH_BAGMAN_MOD
  int cashlimit;
  int playermaxcash;
  int playermaxcashstolen;  // hypov8 cashlimitStolen
#endif // WITH_BAGMAN_MOD
  int timelimit;
  int maxclients;
  int bunnyhop; //add hypov8
  int antiSpawnCamp; //add hypov8
  char mapname[MAX_QPATH];
  char dragonTeam[MAX_QPATH];
  char nikkiTeam[MAX_QPATH];

  int voteTime;
  int voteYes;
  int voteNo;
  qboolean voteModified;          // beep whenever changed
  char voteString[MAX_STRING_TOKENS];

  int teamVoteTime[2];
  int teamVoteYes[2];
  int teamVoteNo[2];
  qboolean teamVoteModified[2];   // beep whenever changed
  char teamVoteString[2][MAX_STRING_TOKENS];

  int levelStartTime;

  int scores1, scores2;           // from configstrings
  int redflag, blueflag;          // flag status from configstrings
  int flagStatus;

  qboolean newHud;

  //
  // locally derived information from gamestate
  //
  qhandle_t gameModels[MAX_MODELS];
  sfxHandle_t gameSounds[MAX_SOUNDS];

  int numInlineModels;
  qhandle_t inlineDrawModel[MAX_MODELS];
  vec3_t inlineModelMidpoints[MAX_MODELS];

  clientInfo_t clientinfo[MAX_CLIENTS];

  // teamchat width is *3 because of embedded color codes
  char teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH * 3 + 1];
  int teamChatMsgTimes[TEAMCHAT_HEIGHT];
  int teamChatPos;
  int teamLastChatPos;

  int cursorX;
  int cursorY;
  qboolean eventHandling;
  qboolean mouseCaptured;
  qboolean sizingHud;
  void *capturedItem;
  qhandle_t activeCursor;

  // orders
  int currentOrder;
  qboolean orderPending;
  int orderTime;
  int currentVoiceClient;
  int acceptOrderTime;
  int acceptTask;
  int acceptLeader;
  char acceptVoice[MAX_NAME_LENGTH];

  // media
  cgMedia_t media;

  //unlagged - client options
  // this will be set to the server's g_delagHitscan
  int				delagHitscan;
//unlagged - client options
} cgs_t;

//==============================================================================

extern cgs_t cgs;
extern cg_t cg;
extern centity_t cg_entities[MAX_GENTITIES];
extern weaponInfo_t cg_weapons[MAX_WEAPONS];
extern itemInfo_t cg_items[MAX_ITEMS];
extern markPoly_t cg_markPolys[MAX_MARK_POLYS];

extern vmCvar_t cg_centertime;
extern vmCvar_t cg_runpitch;
extern vmCvar_t cg_runroll;
extern vmCvar_t cg_bobup;
extern vmCvar_t cg_bobpitch;
extern vmCvar_t cg_bobroll;
extern vmCvar_t cg_swingSpeed;
extern vmCvar_t cg_shadows;
extern vmCvar_t cg_playerShadows; //hypov8 unvan
extern vmCvar_t cg_gibs;
extern vmCvar_t cg_drawTimer;
extern vmCvar_t cg_drawFPS;
extern vmCvar_t cg_drawSnapshot;
extern vmCvar_t cg_draw3dIcons;
extern vmCvar_t cg_drawIcons;
extern vmCvar_t cg_drawAmmoWarning;
extern vmCvar_t cg_drawCrosshair;
extern vmCvar_t cg_drawCrosshairNames;
extern vmCvar_t cg_drawRewards;
extern vmCvar_t cg_drawTeamOverlay;
extern vmCvar_t cg_teamOverlayUserinfo;
extern vmCvar_t cg_crosshairX;
extern vmCvar_t cg_crosshairY;
extern vmCvar_t cg_crosshairSize;
extern vmCvar_t cg_crosshairHealth;
extern vmCvar_t cg_drawStatus;
extern vmCvar_t cg_draw2D;
extern vmCvar_t cg_animSpeed;
//extern vmCvar_t cg_debugAnim;
extern vmCvar_t cg_debugPosition;
extern vmCvar_t cg_debugEvents;
extern vmCvar_t cg_railTrailTime;
extern vmCvar_t cg_errorDecay;
extern vmCvar_t cg_nopredict;
extern vmCvar_t cg_noPlayerAnims;
extern vmCvar_t cg_showmiss;
extern vmCvar_t cg_footsteps;
extern vmCvar_t cg_addMarks;
extern vmCvar_t cg_brassTime;
extern vmCvar_t cg_gun_frame;
extern vmCvar_t cg_gun_x;
extern vmCvar_t cg_gun_y;
extern vmCvar_t cg_gun_z;
extern vmCvar_t cg_drawGun;
extern vmCvar_t cg_viewsize;
extern vmCvar_t cg_tracerChance;
extern vmCvar_t cg_tracerWidth;
extern vmCvar_t cg_tracerLength;
extern vmCvar_t cg_autoswitch;
extern vmCvar_t cg_ignore;
extern vmCvar_t cg_fov;
extern vmCvar_t cg_zoomFov;
extern vmCvar_t cg_thirdPersonFixed; //add hypov8 allow camera orbit
extern vmCvar_t cg_thirdPersonRange;
extern vmCvar_t cg_thirdPersonAngle;
extern vmCvar_t cg_thirdPerson;
extern vmCvar_t cg_stereoSeparation;
extern vmCvar_t cg_lagometer;
extern vmCvar_t cg_drawAttacker;
extern vmCvar_t cg_synchronousClients;
extern vmCvar_t cg_ChatTime;
extern vmCvar_t cg_ChatHeight;
extern vmCvar_t cg_stats;
extern vmCvar_t cg_forceModel;
extern vmCvar_t cg_buildScript;
extern vmCvar_t cg_paused;
extern vmCvar_t cg_blood;
extern vmCvar_t cg_predictItems;
extern vmCvar_t cg_deferPlayers;
extern vmCvar_t cg_drawFriend;
extern vmCvar_t cg_teamChatsOnly;
extern vmCvar_t cg_noVoiceChats;
extern vmCvar_t cg_noVoiceText;
extern vmCvar_t cg_scorePlum;
//unlagged - smooth clients #2
// this is done server-side now
extern vmCvar_t cg_smoothClients;
//unlagged - smooth clients #2
extern vmCvar_t pmove_fixed;
extern vmCvar_t pmove_msec;
//extern	vmCvar_t		cg_pmove_fixed;
extern vmCvar_t cg_cameraOrbit;
extern vmCvar_t cg_cameraOrbitDelay;
extern vmCvar_t cg_timescaleFadeEnd;
extern vmCvar_t cg_timescaleFadeSpeed;
extern vmCvar_t cg_timescale;
extern vmCvar_t cg_cameraMode;

extern vmCvar_t cg_fontTiny;
extern vmCvar_t cg_fontSmall;
extern vmCvar_t cg_fontBig;
extern vmCvar_t cg_fontHuge;

extern vmCvar_t cg_noTaunt;
extern vmCvar_t cg_noProjectileTrail;
extern vmCvar_t cg_trueLightning;
extern vmCvar_t g_cursemode;       /* 0xA5EA */
extern vmCvar_t cg_larshelp;       /* 0xA5EA */
extern vmCvar_t cg_larshelp2;      /* 0xA5EA */
extern vmCvar_t cg_larshelp3;      /* 0xA5EA */

extern vmCvar_t cg_DragonTeamName;
extern vmCvar_t cg_NikkiTeamName;
extern vmCvar_t cg_currentSelectedPlayer;
extern vmCvar_t cg_currentSelectedPlayerName;

extern vmCvar_t cg_enableDust;
extern vmCvar_t cg_enableBreath;
#ifdef USE_GT_SINGLEPLAYER
extern vmCvar_t cg_singlePlayerActive;
extern vmCvar_t cg_singlePlayer
#endif
extern vmCvar_t cg_recordSPDemo;
extern vmCvar_t cg_recordSPDemoName;
//extern	vmCvar_t		cg_obeliskRespawnDelay;	// 0xA5EA

extern vmCvar_t cg_particles;
extern vmCvar_t cg_particleCollision;

extern vmCvar_t cg_precomputedLighting;
extern vmCvar_t cg_debugPlayerAnim;
extern vmCvar_t cg_debugWeaponAnim;
extern vmCvar_t cg_animBlend;
extern vmCvar_t cg_developer;

//unlagged - client options
extern	vmCvar_t		cg_delag;
extern	vmCvar_t		cg_debugDelag;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		sv_fps;
extern	vmCvar_t		cg_projectileNudge;
extern	vmCvar_t		cg_optimizePrediction;
extern	vmCvar_t		cl_timeNudge;
extern	vmCvar_t		cg_latentCmds;
extern	vmCvar_t		cg_plOut;
//unlagged - client options

//unlagged - cg_unlagged.c
void CG_PredictWeaponEffects( centity_t *cent );
qboolean CG_Cvar_ClampInt( const char *name, vmCvar_t *vmCvar, int min, int max );
//unlagged - cg_unlagged.c
//
// cg_main.c
//
const char *CG_ConfigString(int index);
const char *CG_Argv(int arg);
void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum, int randomSeed);
void CG_Shutdown(void);

void QDECL CG_Printf(const char *msg, ...)__attribute__ ((format(printf, 1, 2)));
void QDECL Com_DPrintf(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));

void CG_StartMusic(void);

void CG_UpdateCvars(void);

int CG_CrosshairPlayer(void);
int CG_LastAttacker(void);
void CG_LoadMenus(const char *menuFile);
  void QDECL CG_Error(const char *msg, ...);
  void CG_KeyEvent(int key, qboolean down);
  void CG_EventHandling(int type);

void CG_MouseEvent(int x, int y);

void CG_RankRunFrame(void);
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore(void);
void CG_BuildSpectatorString(void);


//
// cg_view.c
//
void CG_TestModel_f(void);
void CG_TestGun_f(void);
void CG_TestModelNextFrame_f(void);
void CG_TestModelPrevFrame_f(void);
void CG_TestModelNextSkin_f(void);
void CG_TestModelPrevSkin_f(void);
void CG_TestProjLight_f(void);
void CG_TestFlashLight_f(void);
void CG_ZoomDown_f(void);
void CG_ZoomUp_f(void);
void CG_AddBufferedSound(sfxHandle_t sfx);

void CG_DrawActiveFrame(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback);
qboolean CG_CullPointAndRadius( const vec3_t pt, vec_t radius );
void     CG_StartShadowCaster( vec3_t origin, const vec3_t mins, const vec3_t maxs );
void     CG_EndShadowCaster( void );
#ifdef USE_FLAMEGUN
//
// cg_polybus.c
//

polyBuffer_t* CG_PB_FindFreePolyBuffer( qhandle_t shader, int numVerts, int numIndicies );
void CG_PB_ClearPolyBuffers( void );
void CG_PB_RenderPolyBuffers( void );

qhandle_t getTestShader( void ); // JPW NERVE shhh

// Ridah, trails
//
// cg_trails.c
//
// rain - usedby for zinx's trail fixes
int CG_AddTrailJunc( int headJuncIndex, void *usedby, qhandle_t shader, int spawnTime, int sType, vec3_t pos, int trailLife, float alphaStart, float alphaEnd, float startWidth, float endWidth, int flags, vec3_t colorStart, vec3_t colorEnd, float sRatio, float animSpeed );
int CG_AddSparkJunc( int headJuncIndex, void *usedby, qhandle_t shader, vec3_t pos, int trailLife, float alphaStart, float alphaEnd, float startWidth, float endWidth );
int CG_AddSmokeJunc( int headJuncIndex, void *usedby, qhandle_t shader, vec3_t pos, int trailLife, float alpha, float startWidth, float endWidth );
void CG_AddTrails( void );
void CG_ClearTrails( void );
#endif // USE_FLAMEGUN
//
// cg_drawtools.c
//
void CG_AdjustFrom640(float *x, float *y, float *w, float *h);
void CG_FillRect(float x, float y, float width, float height, const float *color);
void CG_DrawPic(float x, float y, float width, float height, qhandle_t hShader);
void CG_DrawString(float x, float y, const char *string, float charWidth, float charHeight, const float *modulate);


void CG_DrawStringExt(int x, int y, const char *string, const float *setColor, qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars);
void CG_DrawBigString(int x, int y, const char *s, float alpha);
void CG_DrawBigStringColor(int x, int y, const char *s, vec4_t color);
void CG_DrawSmallString(int x, int y, const char *s, float alpha);
void CG_DrawSmallStringColor(int x, int y, const char *s, vec4_t color);

int CG_DrawStrlen(const char *str);

float *CG_FadeColor(int startMsec, int totalMsec);
float *CG_TeamColor(int team);
void CG_TileClear(void);
void CG_ColorForHealth(vec4_t hcolor);
void CG_GetColorForHealth(int health, int armor, vec4_t hcolor);

void UI_DrawProportionalString(int x, int y, const char *str, int style, vec4_t color);
void CG_DrawRect(float x, float y, float width, float height, float size, const float *color);
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);

//
// cg_draw.c, cg_newDraw.c
//
extern int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern int numSortedTeamPlayers;
extern int drawTeamOverlayModificationCount;
extern char systemChat[256];
extern char teamChat1[256];
extern char teamChat2[256];

void CG_AddLagometerFrameInfo(void);
void CG_AddLagometerSnapshotInfo(snapshot_t *snap);
void CG_CenterPrint(const char *str, int y, int charWidth);
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles);
void CG_DrawActive(stereoFrame_t stereoView);
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D);
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team);
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t *Font);
void CG_Text_PaintAligned(int x, int y, char *s, float scale, int style, vec4_t color, fontInfo_t *font);
int CG_Text_Width(const char *text, float scale, int limit, fontInfo_t *Font);
int CG_Text_Width2(const char *text, float scale, int limit, fontInfo_t *Font);
int CG_Text_Height2(const char *text, float scale, int limit, fontInfo_t *Font);
int CG_Text_Height(const char *text, float scale, int limit, fontInfo_t *Font);
void CG_SelectPrevPlayer(void);
void CG_SelectNextPlayer(void);
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead(void);
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat(void);
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText(void);
const char *CG_GetKillerText(void);
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles);
void CG_Draw3DWeaponModel(float x, float y, float w, float h, qhandle_t weaponModel, qhandle_t barrelModel, qhandle_t skin, vec3_t origin, vec3_t angles);

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending(void);
const char *CG_GameTypeString(void);
qboolean CG_YourTeamHasFlag(void);
qboolean CG_OtherTeamHasFlag(void);
qhandle_t CG_StatusHandle(int task);

// 0xA5EA, some new newdraw functions
//void CG_DrawLevelTimer( &rect, scale, color, shader, textStyle );
//void CG_DrawWeaponTimer( &rect, scale, color, shader, textStyle );
//CG_DrawChat ( )

//
// cg_player.c
//
void CG_Player(centity_t *cent);
void CG_ResetPlayerEntity(centity_t *cent);
void CG_AddRefEntityWithPowerups(refEntity_t *ent, entityState_t *state, int team);
void CG_NewClientInfo(int clientNum);
sfxHandle_t CG_CustomSound(int clientNum, const char *soundName);

//FIXME: 0xA5EA, waren alle static vorher, bekam aber compilerwarning
qboolean CG_PlayerShadow(centity_t *cent, float *shadowPlane);
qboolean CG_FindClientModelFile(char *filename, int length, clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *teamName, const char *base, const char *ext); //add hypov8; duplcate with teams(test)

void CG_SwingAngles(float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging);
void CG_AddPainTwitch(centity_t *cent, vec3_t torsoAngles);

void CG_PlayerSplash(centity_t *cent);
//void CG_PlayerSprites(centity_t *cent);
//void CG_PlayerPowerups(centity_t *cent, refEntity_t *torso, unsigned int noShadowID);

//
// cg_animation.c
//
void CG_RunLerpFrame( lerpFrame_t *lf, float scale );
void CG_RunMD5LerpFrame( lerpFrame_t *lf, float scale, qboolean animChanged );
void CG_BlendLerpFrame( lerpFrame_t *lf );
void CG_BuildAnimSkeleton( const lerpFrame_t *lf, refSkeleton_t *newSkeleton, const refSkeleton_t *oldSkeleton );

//
// cg_predict.c
//
void CG_BuildSolidList(void);
int CG_PointContents(const vec3_t point, int passEntityNum);
void CG_Trace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask);
void CG_PredictPlayerState(void);
void CG_LoadDeferredPlayers(void);


//
// cg_events.c
//
void CG_CheckEvents(centity_t *cent);
const char *CG_PlaceString(int rank);
void CG_EntityEvent(centity_t *cent, vec3_t position);
void CG_PainEvent(centity_t *cent, int health);
void CG_OnPlayerWeaponChange(); //daemon .50
void CG_HitInfo(entityState_t *ent);

//
// cg_ents.c
//
void CG_DrawBoundingBox( int type, vec3_t origin, vec3_t mins, vec3_t maxs ); //unvan .52
void CG_SetEntitySoundPosition(centity_t *cent);
void CG_AddPacketEntities(void);
void CG_Beam(centity_t *cent);
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out);

void CG_PositionEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName);
void CG_PositionRotatedEntityOnTag(refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName);
void CG_PositionRotatedSelfOnTag(refEntity_t *entity, char *tagSelfName);
void CG_TransformSkeleton(refSkeleton_t *skel, const vec_t scale);


//
// cg_weapons.c
//
void CG_NextWeapon_f(void);
void CG_PrevWeapon_f(void);
void CG_Weapon_f(void);

void CG_InitWeapons(void); //daemon
void CG_RegisterWeapon(int weaponNum);
void CG_RegisterItemVisuals(int itemNum);

void CG_FireWeapon(centity_t *cent, qboolean spistol);
void CG_ReloadWeapon(centity_t *cent);
void CG_ResetWeaponSwitch(centity_t *cent, int weapon);

void CG_MissileHitWall(int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType);
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum);
void CG_ShotgunFire(entityState_t *es);
void CG_Bullet(int weapon, vec3_t origin, int sourceEntityNum, vec3_t normal, int ImpactType , int fleshEntityNum);

void CG_RailTrail(clientInfo_t *ci, vec3_t start, vec3_t end);
void CG_GrappleTrail(centity_t *ent, const weaponInfo_t *wi);
void CG_AddViewWeapon(playerState_t *ps);
void CG_AddPlayerWeapon(refEntity_t *parent, playerState_t *ps, centity_t *cent, clientInfo_t *ci);
//void CG_AddPlayerWeaponLight(playerState_t *ps, centity_t *cent, refEntity_t parent);//add hypo
void CG_DrawWeaponSelect(void);
void CG_DrawPickupItem(void); //hypov8

//void CG_OutOfAmmoChange(void);   // should this be in pmove?

//
// cg_marks.c
//
void CG_InitMarkPolys(void);
void CG_AddMarks(void);
void CG_ImpactMark(qhandle_t markShader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, qboolean alphaFade, float radius, qboolean temporary);


//
// cg_particles.c
//
#define CGAME_TEST_PARTICLES
//FIXME(0xA5EA): disable define for release
void CG_InitParticles(void);
cparticle_t *CG_AllocParticle(void);
void CG_FreeParticle(cparticle_t *p);
void CG_AddParticles(void);
void CG_ParticleSnow(qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void CG_ParticleSmoke(qhandle_t pshader, centity_t *cent);
//void CG_AddParticleShrapnel(localEntity_t *le);
void CG_ParticleSnowFlurry(qhandle_t pshader, centity_t *cent);
void CG_ParticleImpactSmokePuff(qhandle_t pshader, vec3_t origin);
void CG_ParticleBulletDebris(vec3_t org, vec3_t vel, int duration);
void CG_ParticleSparks(vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void CG_ParticleDust(centity_t *cent, vec3_t origin, vec3_t dir);
void CG_ParticleMisc(qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void CG_ParticleMiscScale(qhandle_t pshader, vec3_t origin, int duration, int sizeStart, int sizeEnd, float startAlpha, float endAlpha);
void CG_ParticleExplosion(char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
//extern qboolean		initparticles;
int CG_NewParticleArea(int num);
void CG_ParticleRick(vec3_t org, vec3_t dir);
//void CG_ParticleGrenade(vec3_t org, vec3_t dir);
void CG_ParticleBlood(vec3_t org, vec3_t dir, int count);
void CG_GrenadeParticleTrail(localEntity_t *le, vec3_t origin);
#ifdef CGAME_TEST_PARTICLES
void CG_TestParticles_f(void);
void CG_TestBloodCloud_f(void);
void CG_TestParticleBlood_f(void);
void CG_TestParticleRocketFire_f(void);
void CG_TestParticleBloodPool_f(void);
void CG_TestParticleBleed_f(void);
void CG_TestGibs_f(void);
void CG_TestRand_f(void);
void CG_TestBigExplode_f(void);
void CG_TestTeleportEffect_f(void);
void CG_ParticleDirtBulletDebris_Core(vec3_t org, vec3_t vel, int duration, float width, float height, float alpha, qhandle_t shader);
#endif

#ifdef USE_FLAMEGUN
// Ridah, flamethrower
void CG_FireFlameChunks( centity_t *cent, vec3_t origin, vec3_t angles, float speedScale, qboolean firing );
void CG_InitFlameChunks( void );
void CG_AddFlameChunks( void );
void CG_UpdateFlamethrowerSounds( void );
void CG_FlameDamage( int owner, vec3_t org, float radius );
// done.
#endif
//
// cg_localents.c
//
void CG_InitLocalEntities(void);
localEntity_t *CG_AllocLocalEntity(void);
void CG_AddLocalEntities(void);

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff(const vec3_t p, const vec3_t vel, float radius, float r, float g, float b, float a, float duration, int startTime, int fadeInTime, int leFlags, qhandle_t hShader);
void CG_BubbleTrail(vec3_t start, vec3_t end, float spacing);
void CG_SpawnEffect(vec3_t org);
#ifdef USE_KAMIKAZE
void CG_KamikazeEffect(vec3_t org);
#endif
#if 0
void CG_ObeliskExplode(vec3_t org, int entityNum);
void CG_ObeliskPain(vec3_t org);
void CG_InvulnerabilityImpact(vec3_t org, vec3_t angles);
void CG_InvulnerabilityJuiced(vec3_t org);
void CG_LightningBoltBeam(vec3_t start, vec3_t end);
#endif
void CG_ScorePlum(int client, vec3_t org, int score);
void CG_GibPlayer(vec3_t playerOrigin);
void CG_BigExplode(vec3_t playerOrigin);
void CG_Bleed(vec3_t origin, int entityNum);
localEntity_t *CG_MakeExplosion(vec3_t origin, vec3_t dir, qhandle_t hModel, qhandle_t shader, int msec, qboolean isSprite);
void CG_Fireball(qhandle_t pshader, vec3_t origin, int duration, int sizeStart, int sizeEnd, float startAlpha, float endAlpha);
void CG_ExplosionSplash(vec3_t org, vec3_t surfaceDir, int count);

//
// cg_snapshot.c
//
void CG_ProcessSnapshots(void);
//unlagged - early transitioning
void CG_TransitionEntity( centity_t *cent );
//unlagged - early transitioning

//
// cg_info.c
//
//void CG_LoadingString( const char *s );
void CG_LoadingString(const char *s, qboolean strong);
void CG_LoadingItem(int itemNum);
void CG_LoadingClient(int clientNum);
char const* CG_GetGamemodeText(int gametype, int weaponmod);
void CG_DrawInformation(void);

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard(void);
void CG_DrawOldTourneyScoreboard(void);

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand(void);
void CG_InitConsoleCommands(void);

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands(int latestSequence);
void CG_ParseServerinfo(void);
void CG_SetConfigValues(void);
void CG_LoadVoiceChats(void);
void CG_ShaderStateChanged(void);
void CG_VoiceChatLocal(int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd);
void CG_PlayBufferedVoiceChats(void);

//
// cg_playerstate.c
//
void CG_Respawn(void);
void CG_TransitionPlayerState(playerState_t *ps, playerState_t *ops);
void CG_CheckChangedPredictableEvents(playerState_t *ps);
void CG_DamageFeedback(int yawByte, int pitchByte, int damage);
void CG_ShootFeedback(int weaponUsed); //add hypov8 use for recoil????

#ifdef XPPM
//
// cg_xppm.c
//
qboolean CG_XPPM_RegisterClientModel(clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName);
void CG_XPPM_CopyClientInfoModel(clientInfo_t *from, clientInfo_t *to);
void CG_XPPM_Player(centity_t *cent);

#endif

//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void trap_Print(const char *fmt);

// abort the game
void trap_Error(const char *fmt);

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int trap_Milliseconds(void);

// console variable interaction
void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags);
void trap_Cvar_Update(vmCvar_t *vmCvar);
void trap_Cvar_Set(const char *var_name, const char *value);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

// ServerCommand and ConsoleCommand parameter access
int trap_Argc(void);
void trap_Argv(int n, char *buffer, int bufferLength);
void trap_Args(char *buffer, int bufferLength);

// filesystem access
// returns length of file
int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode);
void trap_FS_Read(void *buffer, size_t len, fileHandle_t f);
void trap_FS_Write(const void *buffer, size_t len, fileHandle_t f);
void trap_FS_FCloseFile(fileHandle_t f);
int trap_FS_Seek(fileHandle_t f, long offset, int origin); // fsOrigin_t
int             trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize);

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void trap_SendConsoleCommand(const char *text);

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void trap_AddCommand(const char *cmdName);

// send a string to the server over the network
void trap_SendClientCommand(const char *s);

// force a screen update, only used during gamestate load
void trap_UpdateScreen(void);

// model collision
void trap_CM_LoadMap(const char *mapname);
int trap_CM_NumInlineModels(void);
clipHandle_t trap_CM_InlineModel(int index);     // 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs);
int trap_CM_PointContents(const vec3_t p, clipHandle_t model);
int trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles);
void trap_CM_BoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask);
void trap_CM_TransformedBoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles);

void trap_CM_BiSphereTrace(trace_t * results, const vec3_t start,
                    const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask);
void trap_CM_TransformedBiSphereTrace(trace_t * results, const vec3_t start,
                         const vec3_t end, float startRad, float endRad,
                         clipHandle_t model, int mask, const vec3_t origin);



// Returns the projection of a polygon onto the solid brushes in the world
int trap_CM_MarkFragments(int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer);

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void trap_S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx);
void trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum);
void trap_S_ClearLoopingSounds(qboolean killall);
void trap_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void trap_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx);
void trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin);

// respatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater);
sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed);       // returns buzz if not found
void trap_S_StartBackgroundTrack(const char *intro, const char *loop);           // empty name stops music
void trap_S_StopBackgroundTrack(void);


void trap_R_LoadWorldMap(const char *mapname);

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t       trap_R_RegisterModel(const char *name);	// returns rgb axis if not found
qhandle_t       trap_R_RegisterAnimation(const char *name);
qhandle_t trap_R_RegisterSkin(const char *name);                                 // returns all white if not found
qhandle_t trap_R_RegisterShader(const char *name);                               // returns all white if not found
qhandle_t trap_R_RegisterShaderNoMip(const char *name);                          // returns all white if not found
qhandle_t       trap_R_RegisterShaderLightAttenuation(const char *name);

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void trap_R_ClearScene(void);
void            trap_R_AddRefEntityToScene(const refEntity_t * ent);

// polys are intended for simple wall marks, not really for doing
// significant construction
void trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts);
void trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys);
void trap_R_AddPolyBufferToScene(polyBuffer_t* pPolyBuffer);  // Added - KRYPTYK
void trap_R_AddAdditiveLightToScene(const vec3_t org, float intensity, float r, float g, float b);
void trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
int trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir);
void trap_R_RenderScene(const refdef_t *fd);
void trap_R_SetColor(const float *rgba);                                         // NULL = 1,1,1,1
void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader);
void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs);

//int trap_R_LerpTag(orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName);
int  trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ); //unvan .50
void trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

void trap_R_AddRefLightToScene(const refLight_t *light);
int trap_R_BoneIndex(qhandle_t hModel, const char *boneName);
int trap_R_BuildSkeleton(refSkeleton_t *skel, qhandle_t anim, int startFrame, int endFrame, float frac, qboolean clearOrigin);
int trap_R_CheckSkeleton(refSkeleton_t * skel, qhandle_t hModel, qhandle_t hAnim);
qhandle_t trap_R_RegisterAnimation(const char *name);
int trap_R_AnimNumFrames(qhandle_t hAnim);
int trap_R_AnimFrameRate(qhandle_t hAnim);
int trap_R_BlendSkeleton(refSkeleton_t *skel, const refSkeleton_t *blend, float frac);
int trap_R_BoneIndex(qhandle_t hModel, const char *boneName);
qhandle_t trap_R_RegisterShaderLightAttenuation(const char *name);

// The glConfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void            trap_GetGlconfig(glconfig_t * glconfig);

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void trap_GetGameState(gameState_t *gamestate);

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void trap_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime);

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean trap_GetSnapshot(int snapshotNumber, snapshot_t *snapshot);

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean trap_GetServerCommand(int serverCommandNumber);

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int trap_GetCurrentCmdNumber(void);

qboolean trap_GetUserCmd(int cmdNumber, usercmd_t *ucmd);

// used for the weapon select and zoom
void trap_SetUserCmdValue(int stateValue, float sensitivityScale);

// aids for VM testing
void testPrintInt(char *string, int i);
void testPrintFloat(char *string, float f);

int trap_MemoryRemaining(void);
void trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean trap_Key_IsDown(int keynum);
int trap_Key_GetCatcher(void);
void trap_Key_SetCatcher(int catcher);
int trap_Key_GetKey(const char *binding);


typedef enum
{
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t; // bk001201 - warning: useless keyword or type name in empty declaration




int trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic(int handle);
void trap_CIN_DrawCinematic(int handle);
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h);

void trap_SnapVector(float *v);

qboolean trap_loadCamera(const char *name);
void trap_startCamera(int time);
qboolean trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles);

qboolean trap_GetEntityToken(char *buffer, int bufferSize);

#endif //CG_LOCAL_H_

