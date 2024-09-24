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

#include "g_local.h"

qboolean G_SpawnString(const char *key, const char *defaultString, char **out)
{
	int i;

	if (!level.spawning)
	{
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for (i = 0; i < level.numSpawnVars; i++)
	{
		if (!Q_stricmp(key, level.spawnVars[i][0]))
		{
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean G_SpawnFloat(const char *key, const char *defaultString, float *out)
{
	char *s;
	qboolean present;
	present = G_SpawnString(key, defaultString, &s);
	*out    = atof(s);
	return present;
}

qboolean G_SpawnInt(const char *key, const char *defaultString, int *out)
{
	char *s;
	qboolean present;
	present = G_SpawnString(key, defaultString, &s);
	*out    = atoi(s);
	return present;
}

qboolean G_SpawnVector(const char *key, const char *defaultString, float *out)
{
	char *s;
	qboolean present;
	present = G_SpawnString(key, defaultString, &s);
	sscanf(s, "%f %f %f", &out[0], &out[1], &out[2]);
	return present;
}
#if 0
#define DFLT_ROT_STRING "0 0 0 0 0 0 0 0 0"
qboolean G_SpawnRotation(const char *key, const char *defaultString, float *out)
{
  char *s;
  qboolean present;

  present = G_SpawnString(key, defaultString, &s);

  if (present)
  {
    vec3_t angles = { .0f, .0f, .0f };
    matrix_t rotation;

    char *p, *token;
    int i;
    G_Printf(S_COLOR_YELLOW"G_SpawnRotation\n");
    MatrixCopy(matrixIdentity, rotation);
    p = (char *)key;
    for (i = 0; i < 9; i++)
    {
      token = Com_Parse(&p);
      rotation[i] = atof(token);
    }
    Mat4_ToAngles(rotation, vec);

    out[0] = vec[0];
    out[1] = vec[1];
    out[2] = vec[2];


  }
  else
    return qfalse;
  sscanf(s, "%f %f %f", &out[0], &out[1], &out[2]);
  {


  }

#if 0
          sscanf(value, "%f %f %f %f %f %f %f %f %f", &rotation[0], &rotation[1], &rotation[2],
               &rotation[4], &rotation[5], &rotation[6], &rotation[8], &rotation[9], &rotation[10]);
#else
          p = (char *)value;
          for(i = 0; i < 9; i++)
          {
            token = Com_Parse(&p);
            rotation[i] = atof(token);
          }
#endif



  return present;
}
#endif

qboolean G_SpawnBoolean(const char *key, const char *defaultString, qboolean * out)
{
	char           *s;
	qboolean        present;

	present = G_SpawnString(key, defaultString, &s);

	if (!Q_stricmp(s, "qfalse") || !Q_stricmp(s, "false") || !Q_stricmp(s, "0"))
		*out = qfalse;
	else if (!Q_stricmp(s, "qtrue") || !Q_stricmp(s, "true") || !Q_stricmp(s, "1"))
		*out = qtrue;
	else
		*out = qfalse;

	return present;
}

//
// fields are needed for spawning from the entity string
//
typedef enum
{
	F_INT,
	F_FLOAT,
	F_LSTRING,          // string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,          // string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_ROTATIONHACK,
	F_ENTITY,           // index on disk, pointer in memory
	F_ITEM,             // index on disk, pointer in memory
	F_CLIENT,           // index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char *name;
	int ofs;
	fieldtype_t type;
	int flags;
} field_t;

field_t fields[] =
{
  {"classname",           FOFS(classname),           F_LSTRING},
  {"origin",              FOFS(s.origin),            F_VECTOR},
  {"model",               FOFS(model),               F_LSTRING},
  {"model2",              FOFS(model2),              F_LSTRING},
  {"spawnflags",          FOFS(spawnflags),          F_INT},
  {"speed",               FOFS(speed),               F_FLOAT},            /* EDIT hypov8 */
  {"target",              FOFS(target),              F_LSTRING}, /* added 'target0-2' for D3 maps */
  {"targetname",          FOFS(targetname),          F_LSTRING}, /* D3 uses item 'name' for linking, name not working in kmap and it hides entities */
  {"target0",             FOFS(target),              F_LSTRING}, /* add hypov8 */
  {"target1",             FOFS(target),              F_LSTRING}, /* add hypov8 */
  {"target2",             FOFS(target),              F_LSTRING}, /* add hypov8 */
  {"sounds",              FOFS(sounds),              F_INT },    /* add hypov8 sounds */
  {"message",             FOFS(message),             F_LSTRING}, /*need to rename item in kpq3radiant, for now??*/
  {"team",                FOFS(team),                F_LSTRING},
  {"wait",                FOFS(wait),                F_FLOAT},
  {"random",              FOFS(random),              F_FLOAT},
  {"count",               FOFS(count),               F_INT},
  {"health",              FOFS(health),              F_INT},
  {"light",                          0,              F_IGNORE},
	{"dmg",                 FOFS(damage),              F_INT},
	{"angles",              FOFS(s.angles),            F_VECTOR},
	{"angle",               FOFS(s.angles),            F_ANGLEHACK},
	{"rotation",            FOFS(s.angles),            F_ROTATIONHACK},
	{"targetShaderName",    FOFS(targetShaderName),    F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName), F_LSTRING},

	{NULL}
};


typedef struct
{
	char *name;
	void (*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start(gentity_t *ent);
void SP_info_player_deathmatch(gentity_t *ent);
void SP_info_player_intermission(gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);
void SP_info_podium(gentity_t *ent);

void SP_func_plat(gentity_t *ent);
void SP_func_static(gentity_t *ent);
void SP_func_rotating(gentity_t *ent);
void SP_func_bobbing(gentity_t *ent);
void SP_func_pendulum(gentity_t *ent);
void SP_func_button(gentity_t *ent);
void SP_func_door(gentity_t *ent);
void SP_func_door_rotating(gentity_t *ent); //add hypov8 et
void finishSpawningKeyedMover(gentity_t * ent); //add hypov8 et
void Think_SpawnNewDoorTrigger(gentity_t * ent); //add hypov8 et

void SP_func_train(gentity_t *ent);
void SP_func_timer(gentity_t *self);
void SP_func_conditional_brush(gentity_t *self);

void SP_trigger_always(gentity_t *ent);
void SP_trigger_multiple(gentity_t *ent);
void SP_trigger_push(gentity_t *ent);
void SP_trigger_teleport(gentity_t *ent);
void SP_trigger_hurt(gentity_t *ent);

void SP_target_remove_powerups(gentity_t *ent);
void SP_target_give(gentity_t *ent);
void SP_target_delay(gentity_t *ent);
void SP_target_speaker(gentity_t *ent);
void SP_target_print(gentity_t *ent);
void SP_target_laser(gentity_t *self);
void SP_target_character(gentity_t *ent);
void SP_target_score(gentity_t *ent);
void SP_target_teleporter(gentity_t *ent);
void SP_target_relay(gentity_t *ent);
void SP_target_kill(gentity_t *ent);
void SP_target_position(gentity_t *ent);
void SP_target_location(gentity_t *ent);
void SP_target_push(gentity_t *ent);

void SP_light(gentity_t *self);
void SP_info_null(gentity_t *self);
void SP_info_notnull(gentity_t *self);
void SP_info_camp(gentity_t *self);
void SP_path_corner(gentity_t *self);

void SP_misc_teleporter_dest(gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);

void SP_shooter_rocket(gentity_t *ent);
#ifdef ALLQ3ITEMS
void SP_shooter_plasma(gentity_t *ent);
#endif
void SP_shooter_grenade(gentity_t *ent);

void SP_team_Dragonsplayer(gentity_t *ent);
void SP_team_Nikkisplayer(gentity_t *ent);

void SP_team_Dragonsspawn(gentity_t *ent);
void SP_team_Nikkisspawn(gentity_t *ent);

#ifdef WITH_BAGMAN_MOD
// Bagman 0xA5EA
void SP_team_BM_cashspawn(gentity_t *ent);
void SP_team_BM_Nikkissafe(gentity_t *ent);
void SP_team_BM_Dragonssafe(gentity_t *ent);
#endif

#ifdef GT_USE_TA_TYPES
void SP_team_blueobelisk(gentity_t *ent);
void SP_team_redobelisk(gentity_t *ent);
void SP_team_neutralobelisk(gentity_t *ent);
#endif
void SP_item_botroam(gentity_t *ent)
{
}

spawn_t spawns[] =
{
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{ "info_player_start", SP_info_player_start },
	{ "info_player_deathmatch", SP_info_player_deathmatch },
	{ "info_player_intermission", SP_info_player_intermission },
	{ "info_null", SP_info_null },
	{ "info_notnull", SP_info_notnull },       // use target_position instead
	{ "info_camp", SP_info_camp },

	{ "func_plat", SP_func_plat },
	{ "func_button", SP_func_button },
	{ "func_door", SP_func_door },
	{ "func_door_rotating", SP_func_door_rotating },  //add hypov8
	{ "func_static", SP_func_static },
	{ "func_rotating", SP_func_rotating },
	{ "func_bobbing", SP_func_bobbing },
	{ "func_pendulum", SP_func_pendulum },
	{ "func_train", SP_func_train },
	{ "func_group", SP_info_null },
	{ "func_timer", SP_func_timer },           // rename trigger_timer?
	{ "func_conditional_brush", SP_func_conditional_brush },

	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{ "trigger_always", SP_trigger_always },
	{ "trigger_multiple", SP_trigger_multiple },
	{ "trigger_push", SP_trigger_push },
	{ "trigger_teleport", SP_trigger_teleport },
	{ "trigger_hurt", SP_trigger_hurt },

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{ "target_give", SP_target_give },
	{ "target_remove_powerups", SP_target_remove_powerups },
	{ "target_delay", SP_target_delay },
	{ "target_speaker", SP_target_speaker },
	{ "speaker", SP_target_speaker }, //add hypov8
	{ "target_print", SP_target_print },
	{ "target_laser", SP_target_laser },
	{ "target_score", SP_target_score },
	{ "target_teleporter", SP_target_teleporter },
	{ "target_relay", SP_target_relay },
	{ "target_kill", SP_target_kill },
	{ "target_position", SP_target_position },
	{ "target_location", SP_target_location },
	{ "target_push", SP_target_push },

	{ "light", SP_light },
	{ "path_corner", SP_path_corner },

	{ "misc_teleporter_dest", SP_misc_teleporter_dest },
	{ "misc_model", SP_misc_model },
	{ "misc_portal_surface", SP_misc_portal_surface },
	{ "misc_portal_camera", SP_misc_portal_camera },

	{ "shooter_rocket", SP_shooter_rocket },
	{ "shooter_grenade", SP_shooter_grenade },
#ifdef ALLQ3ITEMS
	{"shooter_plasma", SP_shooter_plasma},
#endif

	{ "team_dragonsplayer", SP_team_Dragonsplayer },
	{ "team_nikkisplayer", SP_team_Nikkisplayer },

	{ "team_dragonsspawn", SP_team_Dragonsspawn },
	{ "team_nikkisspawn", SP_team_Nikkisspawn },
#ifdef WITH_BAGMAN_MOD
	{ "team_bm_cashspawn", SP_team_BM_cashspawn },
	{ "team_bm_nikkissafe", SP_team_BM_Nikkissafe },
	{ "team_bm_dragonssafe", SP_team_BM_Dragonssafe },
#endif // WITH_BAGMAN_MOD
#ifdef GT_USE_TA_TYPES
	{"team_redobelisk", SP_team_redobelisk},
	{"team_blueobelisk", SP_team_blueobelisk},
	{"team_neutralobelisk", SP_team_neutralobelisk},
#endif
	{ "item_botroam", SP_item_botroam }, 

	{0, 0}
};

/*
===============
G_CallSpawn
Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn(gentity_t *ent)
{
	spawn_t *s;
	gitem_t *item;

	if (!ent->classname)
	{
		G_Printf(S_COLOR_YELLOW"G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check item spawn functions
	for (item = bg_itemlist + 1; item->classname; item++)
	{
		// 0xA5EA, hitmen spawning
		if (g_weaponmod.integer & WM_HITMEN)
		{
			if (!Q_strncmp(ent->classname, "weapon_", 7))
				return qfalse;

			if (!Q_strncmp(ent->classname,  "ammo_", 5))
				return qfalse;
		}

		if (g_weaponmod.integer & WM_REALMODE)
		{
		  if (!Q_strncmp(ent->classname, "item_armor", 10))
			return qfalse;

		  // replace items for realmode
		  if (!Q_strncmp(ent->classname, "weapon_grenadelauncher", sizeof("weapon_grenadelauncher") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Machinegun"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "ammo_grenades", sizeof("ammo_grenades") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Cylinder"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "weapon_heavymachinegun", sizeof("weapon_heavymachinegun") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Magnum fire mod"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "ammo_308", sizeof("ammo_308") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Bullets"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "weapon_bazooka", sizeof("weapon_bazooka") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Magnum fire mod"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "ammo_rockets", sizeof("ammo_rockets") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Bullets"));
			return qtrue;
		  }
       
		  if (!Q_strncmp(ent->classname, "weapon_flamethrower", sizeof("weapon_flamethrower") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Shotgun"));
			return qtrue;
		  }

		  if (!Q_strncmp(ent->classname, "ammo_flametank", sizeof("ammo_flametank") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Shells"));
			return qtrue;
		  }
      
		  if (!Q_strncmp(ent->classname, "item_hmg_mod_cooling", sizeof("item_hmg_mod_cooling") - 1))
		  {
			G_SpawnItem(ent, BG_FindItem("Silencer mod"));
			return qtrue;
		  }
		}

		if (!qstrcmp(item->classname, ent->classname))
		{
			G_SpawnItem(ent, item);
			return qtrue;
		}
	}

	//FIXME(0xA5EA): bagman spawning
	// check normal spawn functions
	for (s = spawns; s->name; s++)
	{
		if (!qstrcmp(s->name, ent->classname))
		{
			// found it
			s->spawn(ent);
			return qtrue;
		}
	}
	G_Printf(S_COLOR_YELLOW"%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString
Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString(const char *string)
{
	char *newb, *new_p;
	size_t i, l;

	l = qstrlen(string) + 1;

	newb = (char*)G_Alloc(l);

	new_p = newb;

	// turn \n into a real linefeed
	for (i = 0; i < l; i++)
	{
		if (string[i] == '\\' && i < l - 1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	return newb;
}

/*
===============
G_ParseField
Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField(const char *key, const char *value, gentity_t *ent)
{
	field_t *f;
	byte *b;
	float v;
	vec3_t vec;

	for (f = fields; f->name; f++)
	{
		if (!Q_stricmp(f->name, key))
		{
			// found it
			b = (byte *)ent;

			switch(f->type)
			{
      case F_LSTRING:
        *(char **)(b + f->ofs) = G_NewString(value);
        break;
      case F_VECTOR:
        sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
        ((float *)(b + f->ofs))[0] = vec[0];
        ((float *)(b + f->ofs))[1] = vec[1];
        ((float *)(b + f->ofs))[2] = vec[2];
        break;
      case F_INT:
        *(int *)(b + f->ofs) = atoi(value);
        break;
      case F_FLOAT:
        *(float *)(b + f->ofs) = atof(value);
        break;
      case F_ANGLEHACK:
        v = atof(value);
        ((float *)(b + f->ofs))[0] = 0;
        ((float *)(b + f->ofs))[1] = v;
        ((float *)(b + f->ofs))[2] = 0;
        break;
      case F_ROTATIONHACK:
      {
        matrix_t rotation;
        char *p, *token;
        int i;
        MatrixCopy(matrixIdentity, rotation);
        p = (char *)value;
        for (i = 0; i < 9; i++)
        {
          token = Com_Parse(&p);
          rotation[i] = atof(token);
        }
        MatrixToAngles(rotation, vec);

        ((float *)(b + f->ofs))[0] = vec[0];
        ((float *)(b + f->ofs))[1] = vec[1];
        ((float *)(b + f->ofs))[2] = vec[2];
      }
        break;
      default:
      case F_IGNORE:
        break;
			}
			return;
		}
	}
}

#define ADJUST_AREAPORTAL()                   \
  do                                          \
  {                                           \
    if (ent->s.eType == ET_MOVER)              \
    {                                         \
      trap_LinkEntity(ent);                   \
      trap_AdjustAreaPortalState(ent, qtrue); \
    }                                         \
  }while(0)

/*
===================
G_SpawnGEntityFromSpawnVars
Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars(void)
{
	int i;
	gentity_t *ent;
	char *s, *value, *gametypeName;
	//FIXME(0xA5EA): handle USE_GT_SINGLEPLAYER properly
	static char *gametypeNames[] =
	{
	    "gangbang",
	    "1vs1",
#ifdef USE_GT_SINGLEPLAYER
      "Single Player",
#endif
	    "team",
	    "ctf",
	    "oneflag",
	    "bagman",
#ifdef GT_USE_TA_TYPES
      "Overload",
      "Harvester",
      "Team Tournament",
#endif
	};


	//FIXME: 0xA5EA, gametypenames ok ??
	// get the next free entity
	ent = G_Spawn();

	for (i = 0; i < level.numSpawnVars; i++)
  {
    G_ParseField(level.spawnVars[i][0], level.spawnVars[i][1], ent);
  }
#ifdef USE_GT_SINGLEPLAYER
  // check for "notsingle" flag
  if (g_gametype.integer == GT_SINGLE_PLAYER)
  {
    G_SpawnInt("notsingle", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }
  //FIXME (0xA5EA):  GT_TOURNAMENT
  // check for "notteam" flag (GT_FFA, GT_TOURNAMENT, GT_SINGLE_PLAYER)
  else
#endif
  if (g_gametype.integer == GT_TEAM || g_gametype.integer == GT_TOURNAMENT)
  {
    G_SpawnInt("notteam", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }
#ifdef WITH_BAGMAN_MOD
  else if (g_gametype.integer == GT_BAGMAN)
  {
    G_SpawnInt("notbm", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }
#endif // WITH_BAGMAN_MOD
  else if (g_gametype.integer == GT_CTF)
  {
    G_SpawnInt("notctf", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }
  else if (g_gametype.integer == GT_1FCTF)
  {
    G_SpawnInt("not1ctf", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }
  else
  {
    G_SpawnInt("notfree", "0", &i);
    if (i)
    {
      ADJUST_AREAPORTAL();
      G_FreeEntity(ent);
      return;
    }
  }


#if 1 //////////////////////////////////////
	// add hypov8 remove flags when not in ctf
	if (g_gametype.integer != GT_1FCTF)
	{
		int ctf_team = 3;
		char *team_items;
		static char *teamItemNames[] = {
			"team_ctf_neutralflag",
			"team_ctf_nikkisflag",
			"team_ctf_dragonsflag", };

		//remove neutral flag
		if (g_gametype.integer == GT_CTF)
			ctf_team = 1;


		if (G_SpawnString("classname", NULL, &value))
		{
			for (i = 0; i < ctf_team; i++)
			{
				team_items = teamItemNames[i];

				s = Q_strstr(value, team_items);
				if (s)
				{
					ADJUST_AREAPORTAL();
					G_FreeEntity(ent);
					return;
				}
			}
		}

  }
#endif

#if 1 ////////////////////////////////////////
  // add hypov8 remove bags when not in bagman
  if (g_gametype.integer != GT_BAGMAN)
  {
	  int ctf_team = 2;
	  char *team_items;
	  static char *teamItemNames[] = {
		  "team_bm_dragonssafe",
		  "team_bm_nikkissafe", };

	if (G_SpawnString("classname", NULL, &value))
		for (i = 0; i < ctf_team; i++)
		{
			team_items = teamItemNames[i];

			s = Q_strstr(value, team_items);
			if (s)
			{
				ADJUST_AREAPORTAL();
				G_FreeEntity(ent);
				return;
			}
		}
  }
#endif


	if (G_SpawnString("gametype", NULL, &value)) //bsp per game allow this item
	{
		if (g_gametype.integer >= GT_FFA && g_gametype.integer < GT_MAX_GAME_TYPE)
		{
			gametypeName = gametypeNames[g_gametype.integer];

			s = Q_strstr(value, gametypeName);
			if (!s)
			{
			  ADJUST_AREAPORTAL();
				G_FreeEntity(ent);
				return;
			}
		}
	}

	// move editor origin to pos
	VectorCopy(ent->s.origin, ent->s.pos.trBase);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);


	// if we didn't get a classname, don't bother spawning anything
	if (!G_CallSpawn(ent))
		G_FreeEntity(ent);
}

/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken(const char *string)
{
	size_t l;
	char *dest;

	l = qstrlen(string);

	if (level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS)
		G_Error("G_AddSpawnVarToken: MAX_SPAWN_CHARS");

	dest = level.spawnVarChars + level.numSpawnVarChars;
	Com_Memcpy(dest, string, l + 1);

	level.numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars
Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]
This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars(void)
{
	char keyname[MAX_TOKEN_CHARS];
	char com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars     = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if (!trap_GetEntityToken(com_token, sizeof(com_token)))
		return qfalse;    // end of spawn string

	if (com_token[0] != '{')
		G_Error("G_ParseSpawnVars: found %s when expecting {", com_token);

	// go through all the key / value pairs
	while(1)
	{
		// parse key
		if (!trap_GetEntityToken(keyname, sizeof(keyname)))
			G_Error("G_ParseSpawnVars: EOF without closing brace");

		if (keyname[0] == '}')
			break;

		// parse value
		if (!trap_GetEntityToken(com_token, sizeof(com_token)))
			G_Error("G_ParseSpawnVars: EOF without closing brace");

		if (com_token[0] == '}')
			G_Error("G_ParseSpawnVars: closing brace without data");

		if (level.numSpawnVars == MAX_SPAWN_VARS)
			G_Error("G_ParseSpawnVars: MAX_SPAWN_VARS");

		level.spawnVars[level.numSpawnVars][0] = G_AddSpawnVarToken(keyname);
		level.spawnVars[level.numSpawnVars][1] = G_AddSpawnVarToken(com_token);
		level.numSpawnVars++;
	}
	return qtrue;
}

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn(void)
{
	char *s;

	G_SpawnString("classname", "", &s);

	if (Q_stricmp(s, "worldspawn"))
		G_Error("SP_worldspawn: The first entity isn't 'worldspawn'");

	// make some data visible to connecting client
	trap_SetConfigstring(CS_GAME_VERSION, GAME_VERSION);

	trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));

	G_SpawnString("music", "", &s);
	trap_SetConfigstring(CS_MUSIC, s);

	G_SpawnString("message", "", &s);
	trap_SetConfigstring(CS_MESSAGE, s);             // map specific message

	trap_SetConfigstring(CS_MOTD, g_motd.string);    // message of the day

	G_SpawnString("gravity", "800", &s);
	trap_Cvar_Set("g_gravity", s);

  //FIXME(0xA5EA): whats that g_enableDust for, needed ?
	G_SpawnString("enableDust", "0", &s);
	trap_Cvar_Set("g_enableDust", s);

	//FIXME(0xA5EA): whats that g_enableBreath for, needed ?
	G_SpawnString("enableBreath", "0", &s);
	trap_Cvar_Set("g_enableBreath", s);

	g_entities[ENTITYNUM_WORLD].s.number  = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// see if we want a warmup time
	trap_SetConfigstring(CS_WARMUP, "");
	if (g_restarted.integer)
	{
		trap_Cvar_Set("g_restarted", "0");
		level.warmupTime = 0;
	}
	else if (g_doWarmup.integer)  // Turn it on
	{
		level.warmupTime = -1;
		trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));
		G_LogPrintf("Warmup:\n");
	}
}

/*
==============
G_SpawnEntitiesFromString
Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString(void)
{
	level.numSpawnVars = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if (!G_ParseSpawnVars())
		G_Error("SpawnEntities: no entities");

	SP_worldspawn();

	// parse ents
	while(G_ParseSpawnVars())
	{
		G_SpawnGEntityFromSpawnVars();
	}

}
