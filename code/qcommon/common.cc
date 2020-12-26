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
 along with KingpinQ3 source code; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ===========================================================================
 */
// common.c -- misc functions used in client and server

#include "q_shared.h"
#include "qcommon.h"
#include "vm_shared.h"
#include <setjmp.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/stat.h> // umask
#else
#include <winsock.h>
#endif

//#include "safeformat.hh"
#include <iostream>
#include <cassert>
#include <utility>

int demo_protocols[] = { 74, 75, 0 }; //hypov8 was 70. for demo playback. added beta and release. probly incompatable

#define MAX_NUM_ARGVS 50

#define MIN_DEDICATED_COMHUNKMEGS 1
#define MIN_COMHUNKMEGS 96      // vanilla q3 56
#define DEF_COMHUNKMEGS 512     // vanilla q3 64
#define DEF_COMZONEMEGS 96      // vanilla q3 24

#define MIN_COMHUNKMEGS_S XSTRING(MIN_COMHUNKMEGS)
#define DEF_COMHUNKMEGS_S XSTRING(DEF_COMHUNKMEGS)
#define DEF_COMZONEMEGS_S XSTRING(DEF_COMZONEMEGS)

#ifdef _OPENMP      // is defined with -fopenmp compiler flag
#include <omp.h>
//#define TRUE  1
//#define FALSE 0
#else
#define omp_get_thread_num() 0
#endif

int com_argc;
char *com_argv[MAX_NUM_ARGVS + 1];

jmp_buf abortframe; // an ERR_DROP occured, exit the entire frame

FILE *debuglogfile;
static fileHandle_t pipefile;
static fileHandle_t logfile;
fileHandle_t com_journalFile; // events are written here
fileHandle_t com_journalDataFile; // config files are written here

cvar_t *com_speeds;
cvar_t *com_developer;
cvar_t *com_dedicated;
cvar_t *com_timescale;
cvar_t *com_fixedtime;
cvar_t *com_journal;
cvar_t *com_maxfps;
cvar_t *com_altivec;
cvar_t *com_3DNow;
cvar_t *com_MMX;
cvar_t *com_SSE;
cvar_t *com_timedemo;
cvar_t *com_sv_running;
cvar_t *com_cl_running;
cvar_t *com_logfile; // 1 = buffer log, 2 = flush after each print
cvar_t *com_pipefile;
cvar_t *com_showtrace;
cvar_t *com_version;
cvar_t *com_blood;
cvar_t *com_buildScript; // for automated data building scripts
cvar_t *com_introPlayed;
cvar_t *cl_paused;
cvar_t *sv_paused;
cvar_t *cl_packetdelay;
cvar_t *sv_packetdelay;
cvar_t *com_cameraMode;
cvar_t *com_ansiColor;
cvar_t *com_unfocused;
cvar_t *com_maxfpsUnfocused;
cvar_t *com_minimized;
cvar_t *com_maxfpsMinimized;
cvar_t *com_abnormalExit;
cvar_t *com_standalone;
cvar_t *com_gamename;
cvar_t *com_protocol; //hypov8 "com_protocol" was using "protocol"
cvar_t *com_basegame;
cvar_t *com_homepath;
cvar_t *com_busyWait;
#if idx64
	int (*Q_VMftol)(void);
#elif id386
	long (QDECL *Q_ftol)(float f);
	int (QDECL *Q_VMftol)(void);
	void (QDECL *Q_SnapVector)(vec3_t vec);
#endif

// com_speeds times
int time_game;
int time_frontend; // renderer frontend time
int time_backend; // renderer backend time

int com_frameTime;
int com_frameNumber;

qboolean com_errorEntered = qfalse;
qboolean com_fullyInitialized = qfalse;
qboolean com_gameRestarting = qfalse;

char com_errorMessage[MAXPRINTMSG];

void Com_WriteConfig_f(void);
void CIN_CloseAllVideos(void);

//============================================================================

static char *rd_buffer;
static int rd_buffersize;
static void (*rd_flush)(char *buffer);

void Com_BeginRedirect(char *buffer, int buffersize, void(*flush)(char *))
{
  if (!buffer || !buffersize || !flush)
    return;

  rd_buffer = buffer;
  rd_buffersize = buffersize;
  rd_flush = flush;

  *rd_buffer = 0;
}

void Com_EndRedirect(void)
{
  if (rd_flush)
    rd_flush(rd_buffer);

  rd_buffer = NULL;
  rd_buffersize = 0;
  rd_flush = NULL;
}

/*
 =============
 Com_Printf
 Both client and server can use this, and it will output
 to the apropriate place.
 A raw string should NEVER be passed as fmt, because of "%f" type crashers.
 =============
 */
void QDECL Com_Printf(const char *fmt, ...)
{
  va_list argptr;
  char msg[MAXPRINTMSG];
  static qboolean opening_qconsole = qfalse;

  va_start(argptr, fmt);
  Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
  va_end(argptr);

  if (rd_buffer)
  {
    if ((qstrlen(msg) + qstrlen(rd_buffer)) > (unsigned)(rd_buffersize - 1))
    {
      rd_flush(rd_buffer);
      *rd_buffer = 0;
    }
    Q_strcat(rd_buffer, rd_buffersize, msg);
    // TTimo nooo .. that would defeat the purpose
    //rd_flush(rd_buffer);
    //*rd_buffer = 0;
    return;
  }

#ifndef DEDICATED
  CL_ConsolePrint(msg);
#endif

  // echo to dedicated console and early console
  Sys_Print(msg);

  // logfile
  if (com_logfile && com_logfile->integer)
  {
    // TTimo: only open the qconsole.log if the filesystem is in an initialized state
    //   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
    if (!logfile && FS_Initialized() && !opening_qconsole)
    {
      struct tm *newtime;
      time_t aclock;

      opening_qconsole = qtrue;

      time(&aclock);
      newtime = localtime(&aclock);

      logfile = FS_FOpenFileWrite(CON_LOGFILE_NAME); // 0xA5EA

      if (logfile)
      {
        Com_Printf("logfile opened on %s\n", asctime(newtime));

        if (com_logfile->integer > 1)
        {
          // force it to not buffer so we get valid
          // data even if we are crashing
          FS_ForceFlush(logfile);
        }
      }
      else
      {
        Com_Printf("Opening " CON_LOGFILE_NAME " failed!\n");
        Cvar_SetValue("logfile", 0);
      }

      opening_qconsole = qfalse;
    }

    if (logfile && FS_Initialized())
      FS_Write(msg, qstrlen(msg), logfile);
  }
}

/*
 ================
 Com_DPrintf

 A Com_Printf that only shows up if the "developer" cvar is set
 ================
 */
void QDECL Com_DPrintf(const char *fmt, ...)
{
  va_list argptr;
  char msg[MAXPRINTMSG];

  // don't confuse non-developers with techie stuff...
  if (!com_developer || !com_developer->integer)
    return;

  va_start(argptr, fmt);
  Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
  va_end(argptr);

  Com_Printf("%s", msg);
}

/*
 =============
 Com_Error
 Both client and server can use this, and it will
 do the appropriate thing.
 =============
 */
void QDECL Com_Error(int code, const char *fmt, ...)
{
  va_list argptr;
  static int lastErrorTime;
  static int errorCount;
  int currentTime;
  if (com_errorEntered)
		Sys_Error("recursive error after: %s", com_errorMessage);

  com_errorEntered = qtrue;

  Cvar_Set("com_errorCode", va("%i", code));

  // when we are running automated scripts, make sure we
  // know if anything failed
  if (com_buildScript && com_buildScript->integer)
    code = ERR_FATAL;

  // if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
  currentTime = Sys_Milliseconds();
  if (currentTime - lastErrorTime < 100)
  {
    if (++errorCount > 3)
      code = ERR_FATAL;
  }
  else
  {
    errorCount = 0;
  }

  lastErrorTime = currentTime;

  va_start(argptr, fmt);
  Q_vsnprintf(com_errorMessage, sizeof(com_errorMessage), fmt, argptr);
  va_end(argptr);

  if (code != ERR_DISCONNECT)
    Cvar_Set("com_errorMessage", com_errorMessage);

  if (code == ERR_DISCONNECT || code == ERR_SERVERDISCONNECT)
  {
    VM_Forced_Unload_Start();
    SV_Shutdown("Server disconnected");
    CL_Disconnect(qtrue);

    CL_FlushMemory();
    VM_Forced_Unload_Done();
    // make sure we can get at our local stuff
    FS_PureServerSetLoadedPaks("", "");
    com_errorEntered = qfalse;
    longjmp(abortframe, -1);
  }
  else if (code == ERR_DROP)
  {
		Com_Printf(S_COLOR_RED "********************\n" S_COLOR_RED "ERROR: %s\n" S_COLOR_RED "********************\n", com_errorMessage);
    VM_Forced_Unload_Start();
	  SV_Shutdown(va(S_COLOR_RED "Server crashed: %s" S_COLOR_WHITE, com_errorMessage));
    CL_Disconnect(qtrue);

    CL_FlushMemory();
    VM_Forced_Unload_Done();
    FS_PureServerSetLoadedPaks("", "");
    com_errorEntered = qfalse;
    longjmp(abortframe, -1);
  }
  else
  {
		VM_Forced_Unload_Start();
		CL_Shutdown(va("Client fatal crashed: %s", com_errorMessage), qtrue, qtrue);
		SV_Shutdown(va(S_COLOR_RED "Server fatal crashed: %s" S_COLOR_WHITE, com_errorMessage));
		VM_Forced_Unload_Done();
  }

  Com_Shutdown();

  Sys_Error("%s", com_errorMessage);
}

/*
 =============
 Com_Quit_f

 Both client and server can use this, and it will
 do the apropriate things.
 =============
 */
void Com_Quit_f(void)
{
  // don't try to shutdown if we are in a recursive error
  char *p = Cmd_Args();
  static char s1[] = "Server quit";
  static char s2[] = "Client quit";
  if (!com_errorEntered)
  {
		// Some VMs might execute "quit" command directly,
		// which would trigger an unload of active VM error.
		// Sys_Quit will kill this process anyways, so
		// a corrupt call stack makes no difference
		VM_Forced_Unload_Start();
    SV_Shutdown(p[0] ? p : s1);
		CL_Shutdown(p[0] ? p : s2, qtrue, qtrue);
		VM_Forced_Unload_Done();
    Com_Shutdown();
    FS_Shutdown(qtrue);
  }
  Sys_Quit();
}

/*
============================================================================
COMMAND LINE FUNCTIONS

+ characters seperate the commandLine string into multiple console
command lines.

All of these are valid:

kingpinq3 +set test blah +map test
kingpinq3 set test blah+map test
kingpinq3 set test blah + map test
============================================================================
*/

#define MAX_CONSOLE_LINES 32
int com_numConsoleLines;
char *com_consoleLines[MAX_CONSOLE_LINES];

/*
 ==================
 Com_ParseCommandLine
 Break it up into multiple console lines
 ==================
 */
void Com_ParseCommandLine(char *commandLine)
{
  int inq = 0;

  com_consoleLines[0] = commandLine;
  com_numConsoleLines = 1;

  while (*commandLine)
  {
    if (*commandLine == '"')
      inq = !inq;

    // look for a + seperating character
    // if commandLine came from a file, we might have real line seperators
    if ((*commandLine == '+' && !inq) || *commandLine == '\n' || *commandLine == '\r')
    {
      if (com_numConsoleLines == MAX_CONSOLE_LINES)
        return;

      com_consoleLines[com_numConsoleLines] = commandLine + 1;
      com_numConsoleLines++;
      *commandLine = 0;
    }
    commandLine++;
  }
}

/*
 ===================
 Com_SafeMode

 Check for "safe" on the command line, which will
 skip loading of q3config.cfg
 ===================
 */
qboolean Com_SafeMode(void)
{
  int i;

  for (i = 0; i < com_numConsoleLines; i++)
  {
    Cmd_TokenizeString(com_consoleLines[i]);
    if (!Q_stricmp(Cmd_Argv(0), "safe") || !Q_stricmp(Cmd_Argv(0), "cvar_restart"))
    {
      com_consoleLines[i][0] = 0;
      return qtrue;
    }
  }
  return qfalse;
}

/*
 ===============
 Com_StartupVariable

 Searches for command line parameters that are set commands.
 If match is not NULL, only that cvar will be looked for.
 That is necessary because cddir and basedir need to be set
 before the filesystem is started, but all other sets should
 be after execing the config and default.
 ===============
 */
void Com_StartupVariable(const char *match)
{
  int i;
  char *s;

  for (i = 0; i < com_numConsoleLines; i++)
  {
    Cmd_TokenizeString(com_consoleLines[i]);

    if (qstrcmp(Cmd_Argv(0), "set"))
      continue;

    s = Cmd_Argv(1);
    if (!match || !qstrcmp(s, match))
    {
			if (Cvar_Flags(s) == (int)CVAR_NONEXISTENT)
				Cvar_Get(s, Cmd_Argv(2), CVAR_USER_CREATED);
			else
				Cvar_Set2(s, Cmd_Argv(2), qfalse);
		}
	}
}


/*
 =================
 Com_AddStartupCommands

 Adds command line parameters as script statements
 Commands are seperated by + signs

 Returns qtrue if any late commands were added, which
 will keep the demoloop from immediately starting
 =================
 */
qboolean Com_AddStartupCommands(void)
{
  int i;
  qboolean added;

  added = qfalse;
  // quote every token, so args with semicolons can work
  for (i = 0; i < com_numConsoleLines; i++)
  {
    if (!com_consoleLines[i] || !com_consoleLines[i][0])
      continue;

    // set commands already added with Com_StartupVariable
    if (!Q_stricmpn(com_consoleLines[i], "set", 3))
      continue;

    added = qtrue;
    Cbuf_AddText(com_consoleLines[i]);
    Cbuf_AddText("\n");
  }

  return added;
}

//============================================================================

void Info_Print( const char *s )
{
	char	key[BIG_INFO_KEY];
	char	value[BIG_INFO_VALUE];
  char *o;
  int l;

  if (*s == '\\')
    s++;
  while (*s)
  {
    o = key;
    while (*s && *s != '\\')
      *o++ = *s++;

    l = o - key;
    if (l < 20)
    {
      Com_Memset(o, ' ', 20 - l);
      key[20] = 0;
    }
    else
      *o = 0;
		Com_Printf ("%s ", key);

    if (!*s)
    {
      Com_Printf("MISSING VALUE\n");
      return;
    }

    o = value;
    s++;
    while (*s && *s != '\\')
      *o++ = *s++;
    *o = 0;

    if (*s)
      s++;
    Com_Printf("%s\n", value);
  }
}

/*
 ============
 Com_StringContains
 ============
 */
char *Com_StringContains(char *str1, char *str2, int casesensitive)
{
  int len, i, j;

  len = qstrlen(str1) - qstrlen(str2);
  for (i = 0; i <= len; i++, str1++)
  {
    for (j = 0; str2[j]; j++)
    {
      if (casesensitive)
      {
        if (str1[j] != str2[j])
          break;
      }
      else
      {
        if (toupper(str1[j]) != toupper(str2[j]))
          break;
      }
    }
    if (!str2[j])
      return str1;
  }
  return NULL;
}

/*
 ============
 Com_Filter
 ============
 */
int
Com_Filter(char *filter, char *name, int casesensitive)
{
  char buf[MAX_TOKEN_CHARS];
  char *ptr;
  int i, found;

  while (*filter)
  {
    if (*filter == '*')
    {
      filter++;
      for (i = 0; *filter; i++)
      {
        if (*filter == '*' || *filter == '?')
          break;
        buf[i] = *filter;
        filter++;
      }
      buf[i] = '\0';
      if (qstrlen(buf))
      {
        ptr = Com_StringContains(name, buf, casesensitive);
        if (!ptr)
          return qfalse;
        name = ptr + qstrlen(buf);
      }
    }
    else if (*filter == '?')
    {
      filter++;
      name++;
    }
    else if (*filter == '[' && *(filter + 1) == '[')
    {
      filter++;
    }
    else if (*filter == '[')
    {
      filter++;
      found = qfalse;
      while (*filter && !found)
      {
        if (*filter == ']' && *(filter + 1) != ']')
          break;
        if (*(filter + 1) == '-' && *(filter + 2) && (*(filter + 2) != ']' || *(filter + 3) == ']'))
        {
          if (casesensitive)
          {
            if (*name >= *filter && *name <= *(filter + 2))
              found = qtrue;
          }
          else
          {
            if (toupper(*name) >= toupper(*filter) && toupper(*name) <= toupper(*(filter + 2)))
              found = qtrue;
          }
          filter += 3;
        }
        else
        {
          if (casesensitive)
          {
            if (*filter == *name)
              found = qtrue;
          }
          else
          {
            if (toupper(*filter) == toupper(*name))
              found = qtrue;
          }
          filter++;
        }
      }
      if (!found)
        return qfalse;
      while (*filter)
      {
        if (*filter == ']' && *(filter + 1) != ']')
          break;
        filter++;
      }
      filter++;
      name++;
    }
    else
    {
      if (casesensitive)
      {
        if (*filter != *name)
          return qfalse;
      }
      else
      {
        if (toupper(*filter) != toupper(*name))
          return qfalse;
      }
      filter++;
      name++;
    }
  }
  return qtrue;
}

/*
 ============
 Com_FilterPath
 ============
 */
int Com_FilterPath(char *filter, char *name, int casesensitive)
{
  int i;
  char new_filter[MAX_QPATH];
  char new_name[MAX_QPATH];

  for (i = 0; i < MAX_QPATH - 1 && filter[i]; i++)
  {
    if (filter[i] == '\\' || filter[i] == ':')
      new_filter[i] = '/';
    else
      new_filter[i] = filter[i];
  }
  new_filter[i] = '\0';
  for (i = 0; i < MAX_QPATH - 1 && name[i]; i++)
  {
    if (name[i] == '\\' || name[i] == ':')
      new_name[i] = '/';
    else
      new_name[i] = name[i];
  }
  new_name[i] = '\0';
  return Com_Filter(new_filter, new_name, casesensitive);
}

/*
 ================
 Com_RealTime
 ================
 */
int Com_RealTime(qtime_t *qtime)
{
  time_t t;
  struct tm *tms;

  t = time(NULL);
  if (!qtime)
    return t;

  tms = localtime(&t);
  if (tms)
  {
    qtime->tm_sec = tms->tm_sec;
    qtime->tm_min = tms->tm_min;
    qtime->tm_hour = tms->tm_hour;
    qtime->tm_mday = tms->tm_mday;
    qtime->tm_mon = tms->tm_mon;
    qtime->tm_year = tms->tm_year;
    qtime->tm_wday = tms->tm_wday;
    qtime->tm_yday = tms->tm_yday;
    qtime->tm_isdst = tms->tm_isdst;
  }
  return t;
}

/*
 ==============================================================================

 ZONE MEMORY ALLOCATION

 There is never any space between memblocks, and there will never be two
 contiguous free memblocks.

 The rover can be left pointing at a non-empty block

 The zone calls are pretty much only used for small strings and structures,
 all big things are allocated on the hunk.
 ==============================================================================
 */

#define ZONEID 0x1d4a11
#define MINFRAGMENT 64

typedef struct zonedebug_s
{
  char *label;
  char *file;
  int line;
  int allocSize;
} zonedebug_t;

typedef struct memblock_s
{
  int size; // including the header and possibly tiny fragments
  int tag; // a tag of 0 is a free block
  struct memblock_s *next, *prev;
  int id; // should be ZONEID
#ifdef ZONE_DEBUG
zonedebug_t d;
#endif
} memblock_t;

typedef struct
{
  int size; // total bytes malloced, including header
  int used; // total bytes used
  memblock_t blocklist; // start / end cap for linked list
  memblock_t *rover;
} memzone_t;

// main zone for all "dynamic" memory allocation
memzone_t *mainzone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
memzone_t *smallzone;

void Z_CheckHeap(void);

/*
 ========================
 Z_ClearZone
 ========================
 */
void Z_ClearZone(memzone_t *zone, int size)
{
  memblock_t *block;

  // set the entire zone to one free block

  zone->blocklist.next = zone->blocklist.prev = block = (memblock_t *)((byte *)zone + sizeof(memzone_t));
  zone->blocklist.tag = 1; // in use block
  zone->blocklist.id = 0;
  zone->blocklist.size = 0;
  zone->rover = block;
  zone->size = size;
  zone->used = 0;

  block->prev = block->next = &zone->blocklist;
  block->tag = 0; // free block
  block->id = ZONEID;
  block->size = size - sizeof(memzone_t);
}

/*
 ========================
 Z_AvailableZoneMemory
 ========================
 */
int Z_AvailableZoneMemory(memzone_t *zone)
{
  return zone->size - zone->used;
}

/*
 ========================
 Z_AvailableMemory
 ========================
 */
int Z_AvailableMemory(void)
{
  return Z_AvailableZoneMemory(mainzone);
}

/*
 ========================
 Z_Free
 ========================
 */
void Z_Free(void *ptr)
{
  memblock_t *block, *other;
  memzone_t *zone;

  if (!ptr)
    Com_Error(ERR_DROP, "Z_Free: NULL pointer");

  block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
  if (block->id != ZONEID)
    Com_Error(ERR_FATAL, "Z_Free: freed a pointer without ZONEID");

  if (block->tag == 0)
    Com_Error(ERR_FATAL, "Z_Free: freed a freed pointer");

  // if static memory
  if (block->tag == TAG_STATIC)
    return;

  // check the memory trash tester
  if (*(int *)((byte *)block + block->size - 4) != ZONEID)
    Com_Error(ERR_FATAL, "Z_Free: memory block wrote past end");

  if (block->tag == TAG_SMALL)
    zone = smallzone;
  else
    zone = mainzone;

  zone->used -= block->size;
  // set the block to something that should cause problems
  // if it is referenced...
  Com_Memset(ptr, 0xaa, block->size - sizeof(*block));

  block->tag = 0; // mark as free

  other = block->prev;
  if (!other->tag)
  {
    // merge with previous free block
    other->size += block->size;
    other->next = block->next;
    other->next->prev = other;

    if (block == zone->rover)
      zone->rover = other;

    block = other;
  }

  zone->rover = block;

  other = block->next;
  if (!other->tag)
  {
    // merge the next free block onto the end
    block->size += other->size;
    block->next = other->next;
    block->next->prev = block;

    if (other == zone->rover)
      zone->rover = block;
  }
}

/*
 ================
 Z_FreeTags
 ================
 */
void Z_FreeTags(int tag)
{
  int count;
  memzone_t *zone;

  if (tag == TAG_SMALL)
    zone = smallzone;
  else
    zone = mainzone;

  count = 0;
  // use the rover as our pointer, because
  // Z_Free automatically adjusts it
  zone->rover = zone->blocklist.next;
  do
  {
    if (zone->rover->tag == tag)
    {
      count++;
      Z_Free((void *)(zone->rover + 1));
      continue;
    }
    zone->rover = zone->rover->next;
  } while (zone->rover != &zone->blocklist);
}

/*
 ================
 Z_TagMalloc
 ================
 */
#ifdef ZONE_DEBUG
void *Z_TagMallocDebug(int size, int tag, char *label, char *file, int line)
{
#else
void *Z_TagMalloc(size_t size, int tag)
{
#endif
  int extra;
#ifdef ZONE_DEBUG
  int allocSize;
#endif
  memblock_t *start, *rover, *neww, *base;
  memzone_t *zone;

  if (!tag)
    Com_Error(ERR_FATAL, "Z_TagMalloc: tried to use a 0 tag");

  if (tag == TAG_SMALL)
    zone = smallzone;
  else
    zone = mainzone;

#ifdef ZONE_DEBUG
  allocSize = size;
#endif
  //
  // scan through the block list looking for the first free block
  // of sufficient size
  //
  size += sizeof(memblock_t); // account for size of block header
  size += 4; // space for memory trash tester
  size = PAD(size, sizeof(intptr_t)); // align to 32/64 bit boundary

  base = rover = zone->rover;
  start = base->prev;

  do
  {
    if (rover == start)
    {
#ifdef ZONE_DEBUG
      Z_LogHeap();
#endif
      // scaned all the way around the list
      Com_Error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone", (int)size, zone == smallzone ? "small"
          : "main");
      return NULL;
    }

    if (rover->tag)
      base = rover = rover->next;
    else
      rover = rover->next;

  } while (base->tag || base->size < (int)size);

  //
  // found a block big enough
  //
  extra = base->size - size;
  if (extra > MINFRAGMENT)
  {
    // there will be a free fragment after the allocated block
    neww = (memblock_t *)((byte *)base + size);
    neww->size = extra;
    neww->tag = 0; // free block
    neww->prev = base;
    neww->id = ZONEID;
    neww->next = base->next;
    neww->next->prev = neww;
    base->next = neww;
    base->size = size;
  }

  base->tag = tag; // no longer a free block

  zone->rover = base->next; // next allocation will start looking here
  zone->used += base->size; //

  base->id = ZONEID;

#ifdef ZONE_DEBUG
  base->d.label = label;
  base->d.file = file;
  base->d.line = line;
  base->d.allocSize = allocSize;
#endif

  // marker for memory trash testing
  *(int *)((byte *)base + base->size - 4) = ZONEID;

  return (void *)((byte *)base + sizeof(memblock_t));
}

/*
 ========================
 Z_Malloc
 ========================
 */
#ifdef ZONE_DEBUG
void *Z_MallocDebug(int size, char *label, char *file, int line)
{
#else
void *Z_Malloc(size_t size)
{
#endif
  void *buf;

  //Z_CheckHeap ();	// DEBUG

#ifdef ZONE_DEBUG
  buf = Z_TagMallocDebug(size, TAG_GENERAL, label, file, line);
#else
  buf = Z_TagMalloc(size, TAG_GENERAL);
#endif
  Com_Memset(buf, 0, size);

  return buf;
}

#ifdef ZONE_DEBUG
void *S_MallocDebug(int size, char *label, char *file, int line)
{
  return Z_TagMallocDebug(size, TAG_SMALL, label, file, line);
}
#else
void *S_Malloc(size_t size)
{
  return Z_TagMalloc(size, TAG_SMALL);
}
#endif

/*
 ========================
 Z_CheckHeap
 ========================
 */
void Z_CheckHeap(void)
{
  memblock_t *block;

  for (block = mainzone->blocklist.next;; block = block->next)
  {
    if (block->next == &mainzone->blocklist)
      break; // all blocks have been hit

    if ((byte *)block + block->size != (byte *)block->next)
      Com_Error(ERR_FATAL, "Z_CheckHeap: block size does not touch the next block\n");

    if (block->next->prev != block)
      Com_Error(ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link\n");

    if (!block->tag && !block->next->tag)
      Com_Error(ERR_FATAL, "Z_CheckHeap: two consecutive free blocks\n");
  }
}

/*
 ========================
 Z_LogZoneHeap
 ========================
 */
void Z_LogZoneHeap(memzone_t *zone, char *name)
{
#ifdef ZONE_DEBUG
  char dump[32], *ptr;
  int i, j;
#endif
  memblock_t *block;
  char buf[4096];
  int size, allocSize, numBlocks;

  if (!logfile || !FS_Initialized())
    return;

  size = allocSize = numBlocks = 0;
  Com_sprintf(buf, sizeof(buf), "\r\n================\r\n%s log\r\n================\r\n", name);
  FS_Write(buf, qstrlen(buf), logfile);
  for (block = zone->blocklist.next; block->next != &zone->blocklist; block = block->next)
  {
    if (block->tag)
    {
#ifdef ZONE_DEBUG
      ptr = ((char *)block) + sizeof(memblock_t);
      j = 0;
      for(i = 0; i < 20 && i < block->d.allocSize; i++)
      {
        if (ptr[i] >= 32 && ptr[i] < 127)
        {
          dump[j++] = ptr[i];
        }
        else
        {
          dump[j++] = '_';
        }
      }
      dump[j] = '\0';
      Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
      FS_Write(buf, qstrlen(buf), logfile);
      allocSize += block->d.allocSize;
#endif
      size += block->size;
      numBlocks++;
    }
  }
#ifdef ZONE_DEBUG
  // subtract debug memory
  size -= numBlocks * sizeof(zonedebug_t);
#else
  allocSize = numBlocks * sizeof(memblock_t); // + 32 bit alignment
#endif
  Com_sprintf(buf, sizeof(buf), "%d %s memory in %d blocks\r\n", size, name, numBlocks);
  FS_Write(buf, qstrlen(buf), logfile);
  Com_sprintf(buf, sizeof(buf), "%d %s memory overhead\r\n", size - allocSize, name);
  FS_Write(buf, qstrlen(buf), logfile);
}

/*
 ========================
 Z_LogHeap
 ========================
 */
void Z_LogHeap(void)
{
  Z_LogZoneHeap(mainzone, "MAIN");
  Z_LogZoneHeap(smallzone, "SMALL");
}

// static mem blocks to reduce a lot of small zone overhead
typedef struct memstatic_s
{
  memblock_t b;
  byte mem[2];
} memstatic_t;

memstatic_t emptystring =
{
  { (sizeof(memblock_t) + 2 + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '\0', '\0' }
};

memstatic_t numberstring[] =
{
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '0', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '1', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '2', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '3', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '4', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '5', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '6', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '7', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '8', '\0' } },
  { { (sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID }, { '9', '\0' } }
};

/*
 ========================
 CopyString

 NOTE:	never write over the memory CopyString returns because
 memory from a memstatic_t might be returned
 ========================
 */
char *CopyString(const char *in)
{
  char *out;

  if (!in[0])
    return ((char *)&emptystring) + sizeof(memblock_t);

  else if (!in[1])
  {
    if (in[0] >= '0' && in[0] <= '9')
      return ((char *)&numberstring[in[0] - '0']) + sizeof(memblock_t);
  }
  out = (char*) S_Malloc(qstrlen(in) + 1);
  qstrcpy(out, in);
  return out;
}

/*
 ==============================================================================

 Goals:
 reproducable without history effects -- no out of memory errors on weird map to map changes
 allow restarting of the client without fragmentation
 minimize total pages in use at run time
 minimize total pages needed during load time

 Single block of memory with stack allocators coming from both ends towards the middle.

 One side is designated the temporary memory allocator.

 Temporary memory can be allocated and freed in any order.

 A highwater mark is kept of the most in use at any time.

 When there is no temporary memory allocated, the permanent and temp sides
 can be switched, allowing the already touched temp memory to be used for
 permanent storage.

 Temp memory must never be allocated on two ends at once, or fragmentation
 could occur.

 If we have any in-use temp memory, additional temp allocations must come from
 that side.

 If not, we can choose to make either side the new temp side and push future
 permanent allocations to the other side.  Permanent allocations should be
 kept on the side that has the current greatest wasted highwater mark.

 ==============================================================================
 */
#define HUNK_MAGIC ((int)0x89537892)
#define HUNK_FREE_MAGIC 0x89537893

typedef struct
{
  int magic;
  int size;
} hunkHeader_t;

typedef struct
{
  int mark;
  int permanent;
  int temp;
  int tempHighwater;
} hunkUsed_t;

typedef struct hunkblock_s
{
  int size;
  byte printed;
  struct hunkblock_s *next;
  char const *label;
  char const *file;
  int line;
} hunkblock_t;

static hunkblock_t *hunkblocks;

static hunkUsed_t hunk_low, hunk_high;
static hunkUsed_t *hunk_permanent, *hunk_temp;

static byte *s_hunkData = NULL;
static int s_hunkTotal;

static int s_zoneTotal;
static int s_smallZoneTotal;

/*
 =================
 Com_Meminfo_f
 =================
 */
void Com_Meminfo_f(void)
{
  memblock_t *block;
  int zoneBytes, zoneBlocks;
  int smallZoneBytes, smallZoneBlocks;
  int botlibBytes, rendererBytes;
  int unused;

  zoneBytes = 0;
  botlibBytes = 0;
  rendererBytes = 0;
  zoneBlocks = 0;
  for (block = mainzone->blocklist.next;; block = block->next)
  {
    if (Cmd_Argc() != 1)
      Com_Printf("block:%p    size:%7i    tag:%3i\n", (void *)block, block->size, block->tag);

    if (block->tag)
    {
      zoneBytes += block->size;
      zoneBlocks++;
      if (block->tag == TAG_BOTLIB)
      {
        botlibBytes += block->size;
      }
      else if (block->tag == TAG_RENDERER)
      {
        rendererBytes += block->size;
      }
    }

    if (block->next == &mainzone->blocklist)
      break; // all blocks have been hit

    if ((byte *)block + block->size != (byte *)block->next)
    {
      Com_Printf("ERROR: block size does not touch the next block\n");
    }
    if (block->next->prev != block)
    {
      Com_Printf("ERROR: next block doesn't have proper back link\n");
    }
    if (!block->tag && !block->next->tag)
    {
      Com_Printf("ERROR: two consecutive free blocks\n");
    }
  }

  smallZoneBytes = 0;
  smallZoneBlocks = 0;
  for (block = smallzone->blocklist.next;; block = block->next)
  {
    if (block->tag)
    {
      smallZoneBytes += block->size;
      smallZoneBlocks++;
    }

    if (block->next == &smallzone->blocklist)
      break; // all blocks have been hit
  }


	Com_Printf("%9i bytes (%6.2f MB) total hunk\n", s_hunkTotal, s_hunkTotal / Square(1024.f));
	Com_Printf("%9i bytes (%6.2f MB) total zone\n", s_zoneTotal, s_zoneTotal / Square(1024.f));
  Com_Printf("\n");
	Com_Printf("%9i bytes (%6.2f MB) low mark\n", hunk_low.mark, hunk_low.mark / Square(1024.f));
	Com_Printf("%9i bytes (%6.2f MB) low permanent\n", hunk_low.permanent, hunk_low.permanent / Square(1024.f));
  if (hunk_low.temp != hunk_low.permanent)
  {
		Com_Printf("%9i bytes (%6.2f MB) low temp\n", hunk_low.temp, hunk_low.temp / Square(1024.f));
  }
	Com_Printf("%9i bytes (%6.2f MB) low tempHighwater\n", hunk_low.tempHighwater, hunk_low.tempHighwater / Square(1024.f));
  Com_Printf("\n");
	Com_Printf("%9i bytes (%6.2f MB) high mark\n", hunk_high.mark, hunk_high.mark / Square(1024.f));
	Com_Printf("%9i bytes (%6.2f MB) high permanent\n", hunk_high.permanent, hunk_high.permanent / Square(1024.f));
  if (hunk_high.temp != hunk_high.permanent)
  {
		Com_Printf("%9i bytes (%6.2f MB) high temp\n", hunk_high.temp, hunk_high.temp / Square(1024.f));
  }
	Com_Printf("%9i bytes (%6.2f MB) high tempHighwater\n", hunk_high.tempHighwater, hunk_high.tempHighwater / Square(1024.f));
  Com_Printf("\n");
	Com_Printf("%9i bytes (%6.2f MB) total hunk in use\n", hunk_low.permanent + hunk_high.permanent,
			   (hunk_low.permanent + hunk_high.permanent) / Square(1024.f));
  unused = 0;
  if (hunk_low.tempHighwater > hunk_low.permanent)
  {
    unused += hunk_low.tempHighwater - hunk_low.permanent;
  }
  if (hunk_high.tempHighwater > hunk_high.permanent)
  {
    unused += hunk_high.tempHighwater - hunk_high.permanent;
  }
	Com_Printf("%9i bytes (%6.2f MB) unused highwater\n", unused, unused / Square(1024.f));
  Com_Printf("\n");
	Com_Printf("%9i bytes (%6.2f MB) in %i zone blocks\n", zoneBytes, zoneBytes / Square(1024.f), zoneBlocks);
	Com_Printf("        %9i bytes (%6.2f MB) in dynamic botlib\n", botlibBytes, botlibBytes / Square(1024.f));
	Com_Printf("        %9i bytes (%6.2f MB) in dynamic renderer\n", rendererBytes, rendererBytes / Square(1024.f));
	Com_Printf("        %9i bytes (%6.2f MB) in dynamic other\n", zoneBytes - (botlibBytes + rendererBytes),
			   (zoneBytes - (botlibBytes + rendererBytes)) / Square(1024.f));
	Com_Printf("        %9i bytes (%6.2f MB) in small Zone memory\n", smallZoneBytes, smallZoneBytes / Square(1024.f));
}

/*
 ===============
 Com_TouchMemory

 Touch all known used data to make sure it is paged in
 ===============
 */
void Com_TouchMemory(void)
{
  int start, end;
  int i, j;
  int sum;
  memblock_t *block;

  Z_CheckHeap();

  start = Sys_Milliseconds();

  sum = 0;

  j = hunk_low.permanent >> 2;
  for (i = 0; i < j; i += 64) // only need to touch each page
  {
    sum += ((int *)s_hunkData)[i];
  }

  i = (s_hunkTotal - hunk_high.permanent) >> 2;
  j = hunk_high.permanent >> 2;
  for (; i < j; i += 64) // only need to touch each page
  {
    sum += ((int *)s_hunkData)[i];
  }

  for (block = mainzone->blocklist.next;; block = block->next)
  {
    if (block->tag)
    {
      j = block->size >> 2;
      for (i = 0; i < j; i += 64) // only need to touch each page
      {
        sum += ((int *)block)[i];
      }
    }
    if (block->next == &mainzone->blocklist)
      break; // all blocks have been hit
  }

  end = Sys_Milliseconds();

  Com_Printf("Com_TouchMemory: %i msec\n", end - start);
}

/*
 =================
 Com_InitZoneMemory
 =================
 */
void Com_InitSmallZoneMemory(void)
{
  s_smallZoneTotal = 512 * 1024;
  smallzone = (memzone_t *)calloc(s_smallZoneTotal, 1);
  if (!smallzone)
  {
    Com_Error(ERR_FATAL, "Small zone data failed to allocate %1.1f megs", (float)s_smallZoneTotal / (1024 * 1024));
  }
  Z_ClearZone(smallzone, s_smallZoneTotal);

  return;
}

void Com_InitZoneMemory(void)
{
  cvar_t *cv;

  // Please note: com_zoneMegs can only be set on the command line, and
  // not in q3config.cfg or Com_StartupVariable, as they haven't been
  // executed by this point. It's a chicken and egg problem. We need the
  // memory manager configured to handle those places where you would
  // configure the memory manager.

  // allocate the random block zone
  cv = Cvar_Get("com_zoneMegs", DEF_COMZONEMEGS_S, CVAR_LATCH | CVAR_ARCHIVE);

  if (cv->integer < DEF_COMZONEMEGS)
  {
    s_zoneTotal = 1024 * 1024 * DEF_COMZONEMEGS;
  }
  else
  {
    s_zoneTotal = cv->integer * 1024 * 1024;
  }

  mainzone = (memzone_t *)calloc(s_zoneTotal, 1);
  if (!mainzone)
  {
    Com_Error(ERR_FATAL, "Zone data failed to allocate %i megs", s_zoneTotal / (1024 * 1024));
  }
  Z_ClearZone(mainzone, s_zoneTotal);

}

/*
 =================
 Hunk_Log
 =================
 */
void Hunk_Log(void)
{
  hunkblock_t *block;
  char buf[4096];
  int size, numBlocks;

  if (!logfile || !FS_Initialized())
    return;

  size = 0;
  numBlocks = 0;
  Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk log\r\n================\r\n");
  FS_Write(buf, qstrlen(buf), logfile);
  for (block = hunkblocks; block; block = block->next)
  {
#ifdef HUNK_DEBUG
    Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s)\r\n", block->size, block->file, block->line, block->label);
    FS_Write(buf, qstrlen(buf), logfile);
#endif
    size += block->size;
    numBlocks++;
  }
  Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
  FS_Write(buf, qstrlen(buf), logfile);
  Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
  FS_Write(buf, qstrlen(buf), logfile);
}

/*
 =================
 Hunk_SmallLog
 =================
 */
void Hunk_SmallLog(void)
{
  hunkblock_t *block, *block2;
  char buf[4096];
  int size, locsize, numBlocks;

  if (!logfile || !FS_Initialized())
    return;

  for (block = hunkblocks; block; block = block->next)
  {
    block->printed = qfalse;
  }
  size = 0;
  numBlocks = 0;
  Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
  FS_Write(buf, qstrlen(buf), logfile);
  for (block = hunkblocks; block; block = block->next)
  {
    if (block->printed)
      continue;

    locsize = block->size;
    for (block2 = block->next; block2; block2 = block2->next)
    {
      if (block->line != block2->line)
        continue;

      if (Q_stricmp(block->file, block2->file))
        continue;

      size += block2->size;
      locsize += block2->size;
      block2->printed = qtrue;
    }
#ifdef HUNK_DEBUG
		Com_sprintf(buf, sizeof(buf), "size = %8d (%6.2f MB / %6.2f MB): %s, line: %d (%s)\r\n", locsize,
					locsize / Square(1024.f), (size + block->size) / Square(1024.f), block->file, block->line, block->label);
		FS_Write(buf, qstrlen(buf), logfile);
#endif
    size += block->size;
    numBlocks++;
  }
  Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
  FS_Write(buf, qstrlen(buf), logfile);
  Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
  FS_Write(buf, qstrlen(buf), logfile);
}

/*
 =================
 Com_InitZoneMemory
 =================
 */
void Com_InitHunkMemory(void)
{
  cvar_t *cv;
  int nMinAlloc;
  char *pMsg = NULL;

  // make sure the file system has allocated and "not" freed any temp blocks
  // this allows the config and product id files ( journal files too ) to be loaded
  // by the file system without redunant routines in the file system utilizing different
  // memory systems
  if (FS_LoadStack() != 0)
  {
    Com_Error(ERR_FATAL, "Hunk initialization failed. File system load stack not zero");
  }

#ifdef DEDICATED
  // allocate the stack based hunk allocator
  cv = Cvar_Get("com_hunkMegs", MIN_COMHUNKMEGS_S, CVAR_LATCH | CVAR_ARCHIVE);
#else
  // allocate the stack based hunk allocator
  cv = Cvar_Get("com_hunkMegs", DEF_COMHUNKMEGS_S, CVAR_LATCH | CVAR_ARCHIVE);
#endif

  // if we are not dedicated min allocation is 56, otherwise min is 1
  if (com_dedicated && com_dedicated->integer)
  {
    nMinAlloc = MIN_DEDICATED_COMHUNKMEGS;
    pMsg = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
  }
  else
  {
    nMinAlloc = MIN_COMHUNKMEGS;
    pMsg = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
  }

  if (cv->integer < nMinAlloc)
  {
    s_hunkTotal = 1024 * 1024 * nMinAlloc;
    Com_Printf(pMsg, nMinAlloc, s_hunkTotal / (1024 * 1024));
  }
  else
  {
    s_hunkTotal = cv->integer * 1024 * 1024;
  }

  s_hunkData = (byte*)calloc(s_hunkTotal + 31, 1);
  if (!s_hunkData)
  {
    Com_Error(ERR_FATAL, "Hunk data failed to allocate %i megs", s_hunkTotal / (1024 * 1024));
  }
  // cacheline align
  s_hunkData = (byte *)(((intptr_t)s_hunkData + 31) & ~31);
  Hunk_Clear();

  Cmd_AddCommand("meminfo", Com_Meminfo_f);
#ifdef ZONE_DEBUG
  Cmd_AddCommand("zonelog", Z_LogHeap);
#endif
#ifdef HUNK_DEBUG
  Cmd_AddCommand("hunklog", Hunk_Log);
  Cmd_AddCommand("hunksmalllog", Hunk_SmallLog);
#endif
}

/*
 ====================
 Hunk_MemoryRemaining
 ====================
 */
int Hunk_MemoryRemaining(void)
{
  int low, high;

  low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
  high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

  return s_hunkTotal - (low + high);
}

/*
 ===================
 Hunk_SetMark
 The server calls this after the level and game VM have been loaded
 ===================
 */
void Hunk_SetMark(void)
{
  hunk_low.mark = hunk_low.permanent;
  hunk_high.mark = hunk_high.permanent;
}

/*
 =================
 Hunk_ClearToMark
 The client calls this before starting a vid_restart or snd_restart
 =================
 */
void Hunk_ClearToMark(void)
{
  hunk_low.permanent = hunk_low.temp = hunk_low.mark;
  hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

/*
 =================
 Hunk_CheckMark
 =================
 */
qboolean Hunk_CheckMark(void)
{
  if (hunk_low.mark || hunk_high.mark)
    return qtrue;

  return qfalse;
}

void CL_ShutdownCGame(void);
void CL_ShutdownUI(void);
void SV_ShutdownGameProgs(void);

/*
 =================
 Hunk_Clear
 The server calls this before shutting down or loading a new map
 =================
 */
void Hunk_Clear(void)
{

#ifndef DEDICATED
  CL_ShutdownCGame();
  CL_ShutdownUI();
#endif
  SV_ShutdownGameProgs();
#ifndef DEDICATED
  CIN_CloseAllVideos();
#endif
  hunk_low.mark = 0;
  hunk_low.permanent = 0;
  hunk_low.temp = 0;
  hunk_low.tempHighwater = 0;

  hunk_high.mark = 0;
  hunk_high.permanent = 0;
  hunk_high.temp = 0;
  hunk_high.tempHighwater = 0;

  hunk_permanent = &hunk_low;
  hunk_temp = &hunk_high;

  Com_Printf("Hunk_Clear: reset the hunk ok\n");
  VM_Clear();
#ifdef HUNK_DEBUG
  hunkblocks = NULL;
#endif
}

static void Hunk_SwapBanks(void)
{
  hunkUsed_t *swap;

  // can't swap banks if there is any temp already allocated
  if (hunk_temp->temp != hunk_temp->permanent)
    return;

  // if we have a larger highwater mark on this side, start making
  // our permanent allocations here and use the other side for temp
  if (hunk_temp->tempHighwater - hunk_temp->permanent > hunk_permanent->tempHighwater - hunk_permanent->permanent)
  {
    swap = hunk_temp;
    hunk_temp = hunk_permanent;
    hunk_permanent = swap;
  }
}

/*
 =================
 Hunk_Alloc
 Allocate permanent (until the hunk is cleared) memory
 =================
 */
#ifdef HUNK_DEBUG
void *Hunk_AllocDebug(int size, ha_pref preference, char const *label, char const *file, int line){
#else
void *Hunk_Alloc(int size, ha_pref preference){ 
#endif
  void *buf;

  if (s_hunkData == NULL)
    Com_Error(ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized");

  // can't do preference if there is any temp allocated
  if (preference == h_dontcare || hunk_temp->temp != hunk_temp->permanent)
  {
    Hunk_SwapBanks();
  }
  else
  {
    if (preference == h_low && hunk_permanent != &hunk_low)
      Hunk_SwapBanks();
    else if (preference == h_high && hunk_permanent != &hunk_high)
      Hunk_SwapBanks();
  }

#ifdef HUNK_DEBUG
  size += sizeof(hunkblock_t);
#endif

  // round to cacheline
  size = (size + 31) & ~31;
////  Com_Printf("Hunk_Alloc %i    total: %i\n", size, hunk_low.temp + hunk_high.temp); //add hypov8

  if (hunk_low.temp + hunk_high.temp + size > s_hunkTotal)
  {
#ifdef HUNK_DEBUG
    Hunk_Log();
    Hunk_SmallLog();
#endif
    Com_Error(ERR_DROP, "Hunk_Alloc failed on %i", size);
///////Com_Error(ERR_DROP, "Hunk_Alloc failed, needs %i bytes free\nTry increase com_hunkMegs. CURRENT=%i\n Sugest ABOVE %i", size, Cvar_VariableIntegerValue("com_hunkMegs"), (hunk_low.temp + hunk_high.temp + size)/ 1000);
  }

  if (hunk_permanent == &hunk_low)
  {
    buf = (void *)(s_hunkData + hunk_permanent->permanent);
    hunk_permanent->permanent += size;
  }
  else
  {
    hunk_permanent->permanent += size;
    buf = (void *)(s_hunkData + s_hunkTotal - hunk_permanent->permanent);
  }

  hunk_permanent->temp = hunk_permanent->permanent;

  Com_Memset(buf, 0, size);

#ifdef HUNK_DEBUG
  {
    hunkblock_t *block;

    block = (hunkblock_t *)buf;
    block->size = size - sizeof(hunkblock_t);
    block->file = file;
    block->label = label;
    block->line = line;
    block->next = hunkblocks;
    hunkblocks = block;
    buf = ((byte *)buf) + sizeof(hunkblock_t);
  }
#endif
  return buf;
}

/*
 =================
 Hunk_AllocateTempMemory
 This is used by the file loading system.
 Multiple files can be loaded in temporary memory.
 When the files-in-use count reaches zero, all temp memory will be deleted
 =================
 */
void *Hunk_AllocateTempMemory(int size)
{
  void *buf;
  hunkHeader_t *hdr;

  // return a Z_Malloc'd block if the hunk has not been initialized
  // this allows the config and product id files ( journal files too ) to be loaded
  // by the file system without redunant routines in the file system utilizing different
  // memory systems
  if (s_hunkData == NULL)
    return Z_Malloc(size);

  Hunk_SwapBanks();

  size = PAD(size, sizeof(intptr_t)) + sizeof(hunkHeader_t);

	if ( hunk_temp->temp + hunk_permanent->permanent + size > s_hunkTotal ) {
    Com_Error(ERR_DROP, "Hunk_AllocateTempMemory: failed on %i", size);
  }

  if (hunk_temp == &hunk_low)
  {
    buf = (void *)(s_hunkData + hunk_temp->temp);
    hunk_temp->temp += size;
  }
  else
  {
    hunk_temp->temp += size;
    buf = (void *)(s_hunkData + s_hunkTotal - hunk_temp->temp);
  }

  if (hunk_temp->temp > hunk_temp->tempHighwater)
  {
    hunk_temp->tempHighwater = hunk_temp->temp;
  }

  hdr = (hunkHeader_t *)buf;
  buf = (void *)(hdr + 1);

  hdr->magic = HUNK_MAGIC;
  hdr->size = size;

  // don't bother clearing, because we are going to load a file over it
  return buf;
}

/*
 ==================
 Hunk_FreeTempMemory
 ==================
 */
void Hunk_FreeTempMemory(void *buf)
{
  hunkHeader_t *hdr;

  // free with Z_Free if the hunk has not been initialized
  // this allows the config and product id files ( journal files too ) to be loaded
  // by the file system without redunant routines in the file system utilizing different
  // memory systems
  if (s_hunkData == NULL)
  {
    Z_Free(buf);
    return;
  }

  hdr = ((hunkHeader_t *)buf) - 1;
  if (hdr->magic != HUNK_MAGIC)
  {
    Com_Error(ERR_FATAL, "Hunk_FreeTempMemory: bad magic");
  }

  hdr->magic = HUNK_FREE_MAGIC;

  // this only works if the files are freed in stack order,
  // otherwise the memory will stay around until Hunk_ClearTempMemory
  if (hunk_temp == &hunk_low)
  {
    if (hdr == (void *)(s_hunkData + hunk_temp->temp - hdr->size))
      hunk_temp->temp -= hdr->size;
    else
      Com_Printf("Hunk_FreeTempMemory: not the final block\n");
  }
  else
  {
    if (hdr == (void *)(s_hunkData + s_hunkTotal - hunk_temp->temp))
      hunk_temp->temp -= hdr->size;
    else
      Com_Printf("Hunk_FreeTempMemory: not the final block\n");
  }
}

/*
 =================
 Hunk_ClearTempMemory
 The temp space is no longer needed.  If we have left more
 touched but unused memory on this side, have future
 permanent allocs use this side.
 =================
 */
void Hunk_ClearTempMemory(void)
{
  if (s_hunkData != NULL)
    hunk_temp->temp = hunk_temp->permanent;
}

/*
 =================
 Hunk_Trash
 =================
 */
void Hunk_Trash(void)
{
  int length, i, rnd;
  char *buf, value;

  return;
  //FIXME (0xA5EA):  wtf ?

  if (s_hunkData == NULL)
    return;

#ifdef _DEBUG
	Com_Error(ERR_DROP, "hunk trashed");
  return;

#endif

  Cvar_Set("com_jp", "1");
  Hunk_SwapBanks();

  if (hunk_permanent == &hunk_low)
    buf = (char*)(s_hunkData + hunk_permanent->permanent);
  else
    buf = (char*)(s_hunkData + s_hunkTotal - hunk_permanent->permanent);

  length = hunk_permanent->permanent;

  if (length > 0x7FFFF)
  {
    //randomly trash data within buf
    rnd = random() * (length - 0x7FFFF);
    value = 31;
    for (i = 0; i < 0x7FFFF; i++)
    {
      value *= 109;
      buf[rnd + i] ^= value;
    }
  }
}

/*
 ===================================================================
 EVENTS AND JOURNALING
 In addition to these events, .cfg files are also copied to the
 journaled file
 ===================================================================
 */
#define MAX_PUSHED_EVENTS 1024
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

/*
 =================
 Com_InitJournaling
 =================
 */
void Com_InitJournaling(void)
{
  Com_StartupVariable("journal");
  com_journal = Cvar_Get("journal", "0", CVAR_INIT);

  if (!com_journal->integer)
    return;

  if (com_journal->integer == 1)
  {
    Com_Printf("Journaling events\n");
    com_journalFile = FS_FOpenFileWrite("journal.dat");
    com_journalDataFile = FS_FOpenFileWrite("journaldata.dat");
  }
  else if (com_journal->integer == 2)
  {
    Com_Printf("Replaying journaled events\n");
    FS_FOpenFileRead("journal.dat", &com_journalFile, qtrue);
    FS_FOpenFileRead("journaldata.dat", &com_journalDataFile, qtrue);
  }

  if (!com_journalFile || !com_journalDataFile)
  {
    Cvar_Set("com_journal", "0");
    com_journalFile = 0;
    com_journalDataFile = 0;
    Com_Printf("Couldn't open journal files\n");
  }
}

/*
 ========================================================================
 EVENT LOOP
 ========================================================================
 */

#define MAX_QUEUED_EVENTS 256
#define MASK_QUEUED_EVENTS (MAX_QUEUED_EVENTS - 1)

static sysEvent_t eventQueue[MAX_QUEUED_EVENTS];
static int eventHead = 0;
static int eventTail = 0;

/*
 ================
 Com_QueueEvent
 A time of 0 will get the current time
 Ptr should either be null, or point to a block of data that can
 be freed by the game later.
 ================
 */
void Com_QueueEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr)
{
  sysEvent_t *ev;

  ev = &eventQueue[eventHead & MASK_QUEUED_EVENTS];

  if (eventHead - eventTail >= MAX_QUEUED_EVENTS)
  {
    Com_Printf("Com_QueueEvent: overflow\n");
    // we are discarding an event, but don't leak memory
    if (ev->evPtr)
      Z_Free(ev->evPtr);

    eventTail++;
  }

  eventHead++;

  if (time == 0)
    time = Sys_Milliseconds();

  ev->evTime = time;
  ev->evType = type;
  ev->evValue = value;
  ev->evValue2 = value2;
  ev->evPtrLength = ptrLength;
  ev->evPtr = ptr;
}

/*
 ================
 Com_GetSystemEvent
 ================
 */
sysEvent_t Com_GetSystemEvent(void)
{
  sysEvent_t ev;
  char *s;

  // return if we have data
  if (eventHead > eventTail)
  {
    eventTail++;
    return eventQueue[(eventTail - 1) & MASK_QUEUED_EVENTS];
  }

  // check for console commands
  s = Sys_ConsoleInput();
  if (s)
  {
    char *b;
    size_t len;

		len = qstrlen( s ) + 1;
		b = static_cast<char*>(Z_Malloc( len ));
		qstrcpy( b, s );
		Com_QueueEvent( 0, SET_CONSOLE, 0, 0, len, b );
  }

  // return if we have data
  if (eventHead > eventTail)
  {
    eventTail++;
    return eventQueue[(eventTail - 1) & MASK_QUEUED_EVENTS];
  }

  // create an empty event to return
  Com_Memset(&ev, 0, sizeof(ev));
  ev.evTime = Sys_Milliseconds();

  return ev;
}

/*
 =================
 Com_GetRealEvent
 =================
 */
sysEvent_t Com_GetRealEvent(void)
{
  int r;
  sysEvent_t ev;

  // either get an event from the system or the journal file
  if (com_journal->integer == 2)
  {
    r = FS_Read(&ev, sizeof(ev), com_journalFile);

    if (r != sizeof(ev))
      Com_Error(ERR_FATAL, "Error reading from journal file");

    if (ev.evPtrLength)
    {
      ev.evPtr = Z_Malloc(ev.evPtrLength);
      r = FS_Read(ev.evPtr, ev.evPtrLength, com_journalFile);

      if (r != ev.evPtrLength)
        Com_Error(ERR_FATAL, "Error reading from journal file");
    }
  }
  else
  {
    ev = Com_GetSystemEvent();

    // write the journal value out if needed
    if (com_journal->integer == 1)
    {
      r = FS_Write(&ev, sizeof(ev), com_journalFile);

      if (r != sizeof(ev))
        Com_Error(ERR_FATAL, "Error writing to journal file");

      if (ev.evPtrLength)
      {
        r = FS_Write(ev.evPtr, ev.evPtrLength, com_journalFile);
        if (r != ev.evPtrLength)
          Com_Error(ERR_FATAL, "Error writing to journal file");
      }
    }
  }
  return ev;
}

/*
 =================
 Com_InitPushEvent
 =================
 */
void Com_InitPushEvent(void)
{
  // clear the static buffer array
  // this requires SET_NONE to be accepted as a valid but NOP event
  Com_Memset(com_pushedEvents, 0, sizeof(com_pushedEvents));
  // reset counters while we are at it
  // beware: GetEvent might still return an SET_NONE from the buffer
  com_pushedEventsHead = 0;
  com_pushedEventsTail = 0;
}

/*
 =================
 Com_PushEvent
 =================
 */
void Com_PushEvent(sysEvent_t *event)
{
  sysEvent_t *ev;
  static int printedWarning = 0;

  ev = &com_pushedEvents[com_pushedEventsHead & (MAX_PUSHED_EVENTS - 1)];

  if (com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS)
  {
    // don't print the warning constantly, or it can give time for more...
    if (!printedWarning)
    {
      printedWarning = qtrue;
      Com_Printf("WARNING: Com_PushEvent overflow\n");
    }

    if (ev->evPtr)
      Z_Free(ev->evPtr);

    com_pushedEventsTail++;
  }
  else
  {
    printedWarning = qfalse;
  }

  *ev = *event;
  com_pushedEventsHead++;
}

/*
 =================
 Com_GetEvent
 =================
 */
sysEvent_t Com_GetEvent(void)
{
  if (com_pushedEventsHead > com_pushedEventsTail)
  {
    com_pushedEventsTail++;
    return com_pushedEvents[(com_pushedEventsTail - 1) & (MAX_PUSHED_EVENTS - 1)];
  }
  return Com_GetRealEvent();
}

/*
 =================
 Com_RunAndTimeServerPacket
 =================
 */
void Com_RunAndTimeServerPacket(netadr_t *evFrom, msg_t *buf)
{
  int t1, t2, msec;

  t1 = 0;

  if (com_speeds->integer)
    t1 = Sys_Milliseconds();

  SV_PacketEvent(*evFrom, buf);

  if (com_speeds->integer)
  {
    t2 = Sys_Milliseconds();
    msec = t2 - t1;

    if (com_speeds->integer == 3)
      Com_Printf("SV_PacketEvent time: %i\n", msec);
  }
}

/*
 =================
 Com_EventLoop

 Returns last event time
 =================
 */
int Com_EventLoop(void)
{
  sysEvent_t ev;
  netadr_t evFrom;
  byte bufData[MAX_MSGLEN];
  msg_t buf;

  MSG_Init(&buf, bufData, sizeof(bufData));

	while (1)
	{
    ev = Com_GetEvent();

    // if no more events are available
    if (ev.evType == SET_NONE)
    {
      // manually send packet events for the loopback channel
      while (NET_GetLoopPacket(NS_CLIENT, &evFrom, &buf))
      {
        CL_PacketEvent(evFrom, &buf);
      }

      while (NET_GetLoopPacket(NS_SERVER, &evFrom, &buf))
      {
        // if the server just shut down, flush the events
        if (com_sv_running->integer)
          Com_RunAndTimeServerPacket(&evFrom, &buf);
      }
      return ev.evTime;
    }

    switch (ev.evType)
    {
    case SET_KEY:
      CL_KeyEvent(ev.evValue, ev.evValue2, ev.evTime);
      break;
    case SET_CHAR:
      CL_CharEvent(ev.evValue);
      break;
    case SET_MOUSE:
      CL_MouseEvent(ev.evValue, ev.evValue2, ev.evTime);
      break;
    case SET_JOYSTICK_AXIS:
      CL_JoystickEvent(ev.evValue, ev.evValue2, ev.evTime);
      break;
    case SET_CONSOLE:
      Cbuf_AddText((char *)ev.evPtr);
      Cbuf_AddText("\n");
      break;
    default:
      Com_Error( ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType );
      break;
    }

    // free any block data
    if (ev.evPtr)
      Z_Free(ev.evPtr);
  }

  return 0; // never reached
}

/*
 ================
 Com_Milliseconds

 Can be used for profiling, but will be journaled accurately
 ================
 */
int Com_Milliseconds(void)
{
  sysEvent_t ev;
  // get events and push them until we get a null event with the current time
  do
  {
    ev = Com_GetRealEvent();

    if (ev.evType != SET_NONE)
      Com_PushEvent(&ev);

  }while (ev.evType != SET_NONE);

  return ev.evTime;
}

//============================================================================
/*
 =============
 Com_Error_f

 Just throw a fatal error to
 test error shutdown procedures
 =============
 */
static void Com_Error_f(void)
{
  if (Cmd_Argc() > 1)
    Com_Error(ERR_DROP, "Testing drop error");
  else
    Com_Error(ERR_FATAL, "Testing fatal error");
}

/*
 =============
 Com_Freeze_f

 Just freeze in place for a given number of seconds to test
 error recovery
 =============
 */
static void Com_Freeze_f(void)
{
  float s;
  int start, now;

  if (Cmd_Argc() != 2)
  {
    Com_Printf("freeze <seconds>\n");
    return;
  }
  s = atof(Cmd_Argv(1));

  start = Com_Milliseconds();

  while (1)
  {
    now = Com_Milliseconds();

    if ((now - start) * 0.001 > s)
      break;
  }
}

/*
 =================
 Com_Crash_f

 A way to force a bus error for development reasons
 =================
 */
static void Com_Crash_f(void)
{
  *(int *)0 = 0x12345678;
}
/*
 ==================
 Com_Setenv_f
 For controlling environment variables
 ==================
 */
void Com_Setenv_f(void)
{
  int argc = Cmd_Argc();
  char *arg1 = Cmd_Argv(1);
  if (argc > 2)
  {
    char *arg2 = Cmd_ArgsFrom(2);
    Sys_SetEnv(arg1, arg2);
  }
  else if (argc == 2)
  {
    char *env = getenv(arg1);
    if (env)
      Com_Printf("%s=%s\n", arg1, env);
    else
      Com_Printf("%s undefined\n", arg1);
  }
}
/*
 ==================
 Com_ExecuteCfg
 For controlling environment variables
 ==================
 */
void Com_ExecuteCfg(void)
{
  Cbuf_ExecuteText(EXEC_NOW, "exec default.cfg\n");
  Cbuf_Execute(); // Always execute after exec to prevent text buffer overflowing
  if (!Com_SafeMode())
  {
    // skip the q3config.cfg and autoexec.cfg if "safe" is on the command line
    Cbuf_ExecuteText(EXEC_NOW, "exec " Q3CONFIG_CFG "\n");
    Cbuf_Execute();
    Cbuf_ExecuteText(EXEC_NOW, "exec autoexec.cfg\n");
    Cbuf_Execute();
  }
}
/*
 ==================
 Com_GameRestart
 Change to a new mod properly with cleaning up cvars before switching.
 ==================
 */

void Com_GameRestart(int checksumFeed, qboolean disconnect)
{
  // make sure no recursion can be triggered
  if (!com_gameRestarting && com_fullyInitialized)
  {
		int clWasRunning;
    com_gameRestarting = qtrue;
		clWasRunning = com_cl_running->integer;

    // Kill server if we have one
    if (com_sv_running->integer)
      SV_Shutdown("Game directory changed");

		if (clWasRunning)
		{
			if (disconnect)
				CL_Disconnect(qfalse);

			CL_Shutdown("Game directory changed", disconnect, qfalse);
		}
    FS_Restart(checksumFeed);

    // Clean out any user and VM created cvars
    Cvar_Restart(qtrue);
    Com_ExecuteCfg();

		if (disconnect)
		{
			// We don't want to change any network settings if gamedir
			// change was triggered by a connect to server because the
			// new network settings might make the connection fail.
			NET_Restart_f();
		}

		if (clWasRunning)
		{
			CL_Init();
      CL_StartHunkUsers(qfalse);
		}

    com_gameRestarting = qfalse;
  }
}
/*
 ==================
 Com_GameRestart_f
 Expose possibility to change current running mod to the user
 ==================
 */
void Com_GameRestart_f(void)
{
	if (!FS_FilenameCompare(Cmd_Argv(1), com_basegame->string))
	{
		// This is the standard base game. Servers and clients should
		// use "" and not the standard basegame name because this messes
		// up pak file negotiation and lots of other stuff
		Cvar_Set("fs_game", "");
	}
	else
	{
	  Cvar_Set("fs_game", Cmd_Argv(1));
	}
  Com_GameRestart(0, qtrue);
}

#ifndef STANDALONE
// TTimo: centralizing the cl_cdkey stuff after I discovered a buffer overflow problem with the dedicated server version
//   not sure it's necessary to have different defaults for regular and dedicated, but I don't want to risk it
//   https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=470
#ifndef DEDICATED
char cl_cdkey[34] = "                                ";
#else
char cl_cdkey[34] = "123456789";
#endif

/*
 =================
 Com_ReadCDKey
 =================
 */
qboolean CL_CDKeyValidate(const char *key, const char *checksum);
void Com_ReadCDKey(const char *filename)
{
  fileHandle_t f;
  char buffer[33];
  char fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);

  FS_SV_FOpenFileRead(fbuffer, &f);
  if (!f)
  {
    Q_strncpyz(cl_cdkey, "                ", 17);
    return;
  }

  Com_Memset(buffer, 0, sizeof(buffer));

  FS_Read(buffer, 16, f);
  FS_FCloseFile(f);

  if (CL_CDKeyValidate(buffer, NULL))
    Q_strncpyz(cl_cdkey, buffer, 17);
  else
    Q_strncpyz(cl_cdkey, "                ", 17);
}

/*
 =================
 Com_AppendCDKey
 =================
 */
void Com_AppendCDKey(const char *filename)
{
  fileHandle_t f;
  char buffer[33];
  char fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);

  FS_SV_FOpenFileRead(fbuffer, &f);
  if (!f)
  {
    Q_strncpyz(&cl_cdkey[16], "                ", 17);
    return;
  }

  Com_Memset(buffer, 0, sizeof(buffer));

  FS_Read(buffer, 16, f);
  FS_FCloseFile(f);

  if (CL_CDKeyValidate(buffer, NULL))
    qstrcat(&cl_cdkey[16], buffer);
  else
    Q_strncpyz(&cl_cdkey[16], "                ", 17);
}

#ifndef DEDICATED
/*
 =================
 Com_WriteCDKey
 =================
 */
static void Com_WriteCDKey(const char *filename, const char *ikey)
{
  fileHandle_t f;
  char fbuffer[MAX_OSPATH];
  char key[17];

#ifndef _WIN32
  mode_t savedumask;
#endif

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);

  Q_strncpyz(key, ikey, 17);

  if (!CL_CDKeyValidate(key, NULL))
    return;

#ifndef _WIN32
  savedumask = umask(0077);
#endif
  f = FS_SV_FOpenFileWrite(fbuffer);
  if (!f)
  {
    Com_Printf("Couldn't write CD key to %s.\n", fbuffer);
    goto out;
  }

  FS_Write(key, 16, f);

  FS_Printf(f, "\n// generated by kingpinq3, do not modify\r\n");
  FS_Printf(f, "// Do not give this file to ANYONE.\r\n");
  FS_Printf(f, "// id Software and Activision will NOT ask you to send this file to them.\r\n");

  FS_FCloseFile(f);
  out:
#ifndef _WIN32
  umask(savedumask);
#endif
  return;
}
#endif

#endif // STANDALONE
static void Com_DetectAltivec(void)
{
  // Only detect if user hasn't forcibly disabled it.
  if (com_altivec->integer)
  {
    static qboolean altivec = qfalse;
    static qboolean detected = qfalse;
    if (!detected)
    {
      altivec = (Sys_GetProcessorFeatures() & CF_ALTIVEC);
      detected = qtrue;
    }

    if (!altivec)
    {
      Cvar_Set("com_altivec", "0"); // we don't have it! Disable support!
    }
  }
}
/*
=================
Com_DetectSSE
Find out whether we have SSE support for Q_ftol function
=================
*/
#if id386 || idx64
static void Com_DetectSSE(void)
{
#if !idx64
	cpuFeatures_t feat;
	feat = (cpuFeatures_t)Sys_GetProcessorFeatures();
	if (feat & CF_SSE)
	{
		if (feat & CF_SSE2)
			Q_SnapVector = qsnapvectorsse;
		else
			Q_SnapVector = qsnapvectorx87;
		Q_ftol = qftolsse;
#endif
		Q_VMftol = qvmftolsse;
		Com_Printf("Have SSE support\n");
#if !idx64
	}
	else
	{
		Q_ftol = qftolx87;
		Q_VMftol = qvmftolx87;
		Q_SnapVector = qsnapvectorx87;
		Com_Printf("No SSE support on this machine\n");
	}
#endif
}
#else
#define Com_DetectSSE()
#endif
//FIXME (0xA5EA): not completely added from ioq3
/*
 =================
 Com_InitRand
 Seed the random number generator, if possible with an OS supplied random seed.
 =================
 */
static void Com_InitRand(void)
{
  unsigned int seed;

  if (Sys_RandomBytes((byte *)&seed, sizeof(seed)))
    srand(seed);
  else
    srand(time(NULL));
}

static void Com_GenerateMediaTXT_f(void);
static void Com_GenerateCorePK3_f(void);
static void Com_MathTest_f(void);
static void Com_MathTSCTest_f(void);

/*
 =================
 Com_Init
 =================
 */
void Com_Init(char *commandLine)
{
  char *s;
  int qport;

  Com_Printf("%s %s %s\n", KPQ3_VERSION, PLATFORM_STRING, __DATE__);

  if (setjmp(abortframe))
    Sys_Error("Error during initialization");

  // Clear queues
  Com_Memset(&eventQueue[0], 0, MAX_QUEUED_EVENTS * sizeof(sysEvent_t));

  // initialize the weak pseudo-random number generator for use later.
  Com_InitRand();
  // do this before anything else decides to push events
  Com_InitPushEvent();

  Com_InitSmallZoneMemory();
  Cvar_Init();

  // prepare enough of the subsystems to handle
  // cvar and command buffer management
  Com_ParseCommandLine(commandLine);

  //	Swap_Init ();
  Cbuf_Init();

	Com_DetectSSE();
  // override anything from the config files with command line args
  Com_StartupVariable(NULL);

  Com_InitZoneMemory();
  Cmd_Init();
  // get the developer cvar set as early as possible
	com_developer = Cvar_Get("developer", "0", CVAR_TEMP);

  // done early so bind command exists
  CL_InitKeyCommands();

	com_basegame = Cvar_Get("com_basegame", BASEGAME, CVAR_INIT);
	com_homepath = Cvar_Get("com_homepath", "", CVAR_INIT);
	if (!com_basegame->string[0])
		Cvar_ForceReset("com_basegame");
  FS_InitFilesystem();

  Com_InitJournaling();

  // Add some commands here already so users can use them from config files
  Cmd_AddCommand("setenv", Com_Setenv_f);
  if (com_developer && com_developer->integer)
  {
    Cmd_AddCommand("error", Com_Error_f);
    Cmd_AddCommand("crash", Com_Crash_f);
    Cmd_AddCommand("freeze", Com_Freeze_f);
  }
  Cmd_AddCommand("quit", Com_Quit_f);
  Cmd_AddCommand("changeVectors", MSG_ReportChangeVectors_f);
  Cmd_AddCommand("writeconfig", Com_WriteConfig_f);
  Cmd_SetCommandCompletionFunc("writeconfig", Cmd_CompleteCfgName);
  Cmd_AddCommand("game_restart", Com_GameRestart_f);
  Com_ExecuteCfg();

  // override anything from the config files with command line args
  Com_StartupVariable(NULL);

  // get dedicated here for proper hunk megs initialization
#ifdef DEDICATED
  com_dedicated = Cvar_Get("dedicated", "1", CVAR_INIT | CVAR_TEMP); //hypov8 dont write to kingpinq3.cfg
  Cvar_CheckRange(com_dedicated, 1, 2, qtrue);
#else
  com_dedicated = Cvar_Get("dedicated", "0", CVAR_LATCH | CVAR_TEMP); //hypov8 dont write to kingpinq3.cfg
  Cvar_CheckRange(com_dedicated, 0, 2, qtrue);
#endif
  // allocate the stack based hunk allocator
  Com_InitHunkMemory();

  // if any archived cvars are modified after this, we will trigger a writing
  // of the config file
  cvar_modifiedFlags &= ~CVAR_ARCHIVE;

  //
  // init commands and vars
  //
  com_altivec = Cvar_Get("com_altivec", "1", CVAR_ARCHIVE);
  com_3DNow = Cvar_Get("com_3DNow", "1", CVAR_ARCHIVE);
  com_MMX = Cvar_Get("com_MMX", "1", CVAR_ARCHIVE);
  com_SSE = Cvar_Get("com_SSE", "1", CVAR_ARCHIVE);
  com_maxfps = Cvar_Get("com_maxfps", "90", CVAR_ARCHIVE);

  //FIXME(0xA5EA): wtf is this cvar for
  com_blood = Cvar_Get("com_blood", "1", CVAR_ARCHIVE);

#if defined(_DEBUG)
  com_developer = Cvar_Get("developer", "1", CVAR_TEMP);
  com_logfile = Cvar_Get("logfile", "2", CVAR_TEMP);
#else
  com_logfile = Cvar_Get("logfile", "0", CVAR_TEMP);
#endif

  com_timescale = Cvar_Get("timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO);
  com_fixedtime = Cvar_Get("fixedtime", "0", CVAR_CHEAT);
  com_showtrace = Cvar_Get("com_showtrace", "0", CVAR_CHEAT);
  com_speeds = Cvar_Get("com_speeds", "0", 0);
  com_timedemo = Cvar_Get("timedemo", "0", CVAR_CHEAT);
  com_cameraMode = Cvar_Get("com_cameraMode", "0", CVAR_CHEAT);

  cl_paused = Cvar_Get("cl_paused", "0", CVAR_ROM);
  sv_paused = Cvar_Get("sv_paused", "0", CVAR_ROM);
  cl_packetdelay = Cvar_Get("cl_packetdelay", "0", CVAR_CHEAT);
  sv_packetdelay = Cvar_Get("sv_packetdelay", "0", CVAR_CHEAT);
  com_sv_running = Cvar_Get("sv_running", "0", CVAR_ROM);
  com_cl_running = Cvar_Get("cl_running", "0", CVAR_ROM);
  com_buildScript = Cvar_Get("com_buildScript", "0", 0);
  com_ansiColor = Cvar_Get("com_ansiColor", "0", CVAR_ARCHIVE);

  com_unfocused = Cvar_Get("com_unfocused", "0", CVAR_ROM);
  com_maxfpsUnfocused = Cvar_Get("com_maxfpsUnfocused", "0", CVAR_ARCHIVE);
  com_minimized = Cvar_Get("com_minimized", "0", CVAR_ROM);
  com_maxfpsMinimized = Cvar_Get("com_maxfpsMinimized", "0", CVAR_ARCHIVE);
	com_abnormalExit = Cvar_Get( "com_abnormalExit", "0", CVAR_ROM );
	com_busyWait = Cvar_Get("com_busyWait", "0", CVAR_ARCHIVE);
  Cvar_Get("com_errorMessage", "", CVAR_ROM | CVAR_NORESTART);
  com_introPlayed = Cvar_Get("com_introplayed", "0", CVAR_ARCHIVE);

#ifndef NO_DEVELOPER_CMDS
  Cmd_AddCommand("generateMEDIA.txt", Com_GenerateMediaTXT_f);
  Cmd_AddCommand("generatecore.pk3", Com_GenerateCorePK3_f);
  Cmd_AddCommand("mathtest", Com_MathTest_f);
#ifdef USE_ASM_LIB
  Cmd_AddCommand("mathTSCtest", Com_MathTSCTest_f);
#endif
#endif
  s = va("%s %s %s", KPQ3_VERSION, PLATFORM_STRING, __DATE__);
  com_version = Cvar_Get("version", s, CVAR_ROM | CVAR_SERVERINFO);
	com_gamename = Cvar_Get("com_gamename", GAMENAME_FOR_MASTER, CVAR_SERVERINFO | CVAR_INIT);
  com_protocol = Cvar_Get("com_protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_INIT);  //hypov8 "protocol"

  Cvar_Get("protocol", va("%i", com_protocol->integer), CVAR_ROM);


  Sys_Init();

	if ( Sys_WritePIDFile( ) )
	{
#ifndef DEDICATED
#ifndef		HYPODEBUG //hypov8 disable this nag when debuging
		const char *message = "The last time " CLIENT_WINDOW_TITLE " ran, "
			"it didn't exit properly. This may be due to inappropriate video "
			"settings. Would you like to start with \"safe\" video settings?";

		if ( Sys_Dialog( DT_YES_NO, message, "Abnormal Exit" ) == DR_YES )
			Cvar_Set( "com_abnormalExit", "1" );
#endif
#endif
	}

  // Pick a random port value
  Com_RandomBytes((byte*)&qport, sizeof(int));
  Netchan_Init(qport & 0xffff);
  VM_Init();
  SV_Init();

  com_dedicated->modified = qfalse;
#ifndef DEDICATED
  CL_Init();
#endif

  // set com_frameTime so that if a map is started on the
  // command line it will still be able to count on com_frameTime
  // being random enough for a serverid
  com_frameTime = Com_Milliseconds();

  // add + commands from command line
  if (!Com_AddStartupCommands())
  {
    // if the user didn't give any commands, run default action
    if (!com_dedicated->integer)
    {
      Cbuf_AddText("cinematic idlogo.RoQ\n");
      if (!com_introPlayed->integer)
      {
        Cvar_Set(com_introPlayed->name, "1");
        Cvar_Set("nextmap", "cinematic intro.RoQ");
      }
    }
  }

  // start in full screen ui mode
  Cvar_Set("r_uiFullScreen", "1");

  CL_StartHunkUsers(qfalse);
#ifdef USE_GT_SINGLEPLAYER
  // make sure single player is off by default
  Cvar_Set("ui_singlePlayerActive", "0");
#endif
  com_fullyInitialized = qtrue;

  // always set the cvar, but only print the info if it makes sense.
  Com_DetectAltivec();
#if idppc
  Com_Printf("Altivec support is %s\n", com_altivec->integer ? "enabled" : "disabled");
#endif

	com_pipefile = Cvar_Get( "com_pipefile", "", CVAR_ARCHIVE|CVAR_LATCH );
	if ( com_pipefile->string[0] )
	{
		pipefile = FS_FCreateOpenPipeFile( com_pipefile->string );
	}

	Com_Printf ("--- Common Initialization Complete ---\n");
}
/*
===============
Com_ReadFromPipe
Read whatever is in com_pipefile, if anything, and execute it
===============
*/
void Com_ReadFromPipe( void )
{
	static char buf[MAX_STRING_CHARS];
	static int accu = 0;
	int read;
	if ( !pipefile )
		return;

	while( ( read = FS_Read( buf + accu, sizeof( buf ) - accu - 1, pipefile ) ) > 0 )
	{
		char *brk = NULL;
		int i;
		for( i = accu; i < accu + read; ++i )
		{
			if ( buf[ i ] == '\0' )
				buf[ i ] = '\n';

			if ( buf[ i ] == '\n' || buf[ i ] == '\r' )
				brk = &buf[ i + 1 ];
		}
		buf[ accu + read ] = '\0';
		accu += read;
		if ( brk )
		{
			char tmp = *brk;
			*brk = '\0';
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			*brk = tmp;
			accu -= brk - buf;
			Com_Memmove( buf, brk, accu + 1 );
		}
		else if ( accu >= (int)(sizeof( buf ) - 1) ) // full
		{
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			accu = 0;
		}
	}
}

//==================================================================
void Com_WriteConfigToFile(const char *filename)
{
  fileHandle_t f;

  f = FS_FOpenFileWrite(filename);
  if (!f)
  {
    Com_Printf("Couldn't write %s.\n", filename);
    return;
  }

  FS_Printf(f, "// generated by kingpinq3, do not modify\n");
  Key_WriteBindings(f);
  Cvar_WriteVariables(f);
  FS_FCloseFile(f);
}

/*
 ===============
 Com_WriteConfiguration
 Writes key bindings and archived cvars to config file if modified
 ===============
 */
void Com_WriteConfiguration(void)
{
#if !defined(DEDICATED) && !defined(STANDALONE)
  cvar_t *fs;
#endif
  // if we are quiting without fully initializing, make sure
  // we don't write out anything
  if (!com_fullyInitialized)
    return;

  if (!(cvar_modifiedFlags & CVAR_ARCHIVE))
    return;

  cvar_modifiedFlags &= ~CVAR_ARCHIVE;

  Com_WriteConfigToFile(Q3CONFIG_CFG);

	// not needed for dedicated or standalone
#if !defined(DEDICATED) && !defined(STANDALONE)
  fs = Cvar_Get("fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO);

	if (!com_standalone->integer)
  {
    if (UI_usesUniqueCDKey() && fs && fs->string[0] != 0)
      Com_WriteCDKey(fs->string, &cl_cdkey[16]);
    else
      Com_WriteCDKey(BASEGAME, cl_cdkey);
  }
#endif
}

/*
 ===============
 Com_WriteConfig_f
 Write the config file to a specific name
 ===============
 */
void Com_WriteConfig_f(void)
{
  char filename[MAX_QPATH];

  if (Cmd_Argc() != 2)
  {
    Com_Printf("Usage: writeconfig <filename>\n");
    return;
  }

  Q_strncpyz(filename, Cmd_Argv(1), sizeof(filename));
  COM_DefaultExtension(filename, sizeof(filename), ".cfg");
  Com_Printf("Writing %s.\n", filename);
  Com_WriteConfigToFile(filename);
}

/*
 ================
 Com_ModifyMsec
 ================
 */
int Com_ModifyMsec(int msec)
{
  int clampTime;

  //
  // modify time for debugging values
  //
  if (com_fixedtime->integer)
    msec = com_fixedtime->integer;
  else if (com_timescale->value)
    msec *= com_timescale->value;
  else if (com_cameraMode->integer)
    msec *= com_timescale->value;

  // don't let it scale below 1 msec
  if (msec < 1 && com_timescale->value)
    msec = 1;

  if (com_dedicated->integer)
  {
    // dedicated servers don't want to clamp for a much longer
    // period, because it would mess up all the client's views
    // of time.
    if (com_sv_running->integer && msec > 500)
      Com_Printf("Hitch warning: %i msec frame time\n", msec);

    //FIXME (0xA5EA): warum krieg ich beim start die warnung ?

    clampTime = 5000;
  }
  else if (!com_sv_running->integer)
  {
    // clients of remote servers do not want to clamp time, because
    // it would skew their view of the server's time temporarily
    clampTime = 5000;
  }
  else
  {
    // for local single player gaming
    // we may want to clamp the time to prevent players from
    // flying off edges when something hitches.
    clampTime = 200;
  }

  if (msec > clampTime)
    msec = clampTime;

  return msec;
}

/*
 =================
Com_TimeVal
 =================
 */

int Com_TimeVal(int minMsec)
{
	int		timeVal;

	timeVal = Sys_Milliseconds() - com_frameTime;

	if (timeVal >= minMsec)
		timeVal = 0;
	else
		timeVal = minMsec - timeVal;
	return timeVal;
}
/*
=================
Com_Frame
=================
*/
void Com_Frame( void ) {
	int		msec, minMsec;
	int		timeVal, timeValSV;
	static int	lastTime = 0, bias = 0;

  int timeBeforeFirstEvents;
  int timeBeforeServer;
  int timeBeforeEvents;
  int timeBeforeClient;
  int timeAfter;

  if (setjmp(abortframe))
    return; // an ERR_DROP was thrown

#if 0
  if (com_maxfps->integer > 125) //hypov8 add limiter to max fps, droped frames cause mouse to loose input
	  com_maxfps->integer = 125;
#endif
  
  timeBeforeFirstEvents = 0;
  timeBeforeServer = 0;
  timeBeforeEvents = 0;
  timeBeforeClient = 0;
  timeAfter = 0;

  // write config file if anything changed
  Com_WriteConfiguration();

  //
  // main event loop
  //
  if (com_speeds->integer)
  {
    timeBeforeFirstEvents = Sys_Milliseconds();
  }

	// Figure out how much time we have
  if (!com_timedemo->integer)
  {
    if (com_dedicated->integer)
      minMsec = SV_FrameMsec();
    else
    {
      if (com_minimized->integer && com_maxfpsMinimized->integer > 0)
        minMsec = 1000 / com_maxfpsMinimized->integer;
      else if (com_unfocused->integer && com_maxfpsUnfocused->integer > 0)
        minMsec = 1000 / com_maxfpsUnfocused->integer;
      else if (com_maxfps->integer > 0)
        minMsec = 1000 / com_maxfps->integer;
      else
        minMsec = 1;

      timeVal = com_frameTime - lastTime;
      bias += timeVal - minMsec;
      if (bias > minMsec)
        bias = minMsec;
      // Adjust minMsec if previous frame took too long to render so
      // that framerate is stable at the requested value.
      minMsec -= bias;
    }
  }
  else
    minMsec = 1;

  do
  {
    if (com_sv_running->integer)
    {
      timeValSV = SV_SendQueuedPackets();

      timeVal = Com_TimeVal(minMsec);

      if (timeValSV < timeVal)
        timeVal = timeValSV;
    }
    else
      timeVal = Com_TimeVal(minMsec);

    if (com_busyWait->integer || timeVal < 1)
      NET_Sleep(0);
    else
      NET_Sleep(timeVal - 1);

  } while(Com_TimeVal(minMsec));

	lastTime = com_frameTime;
  com_frameTime = Com_EventLoop();

  msec = com_frameTime - lastTime;

  Cbuf_Execute();

  if (com_altivec->modified)
  {
    Com_DetectAltivec();
    com_altivec->modified = qfalse;
  }

  // mess with msec if needed
  msec = Com_ModifyMsec(msec);

  //
  // server side
  //
  if (com_speeds->integer)
    timeBeforeServer = Sys_Milliseconds();

  SV_Frame(msec);

  // if "dedicated" has been modified, start up
  // or shut down the client system.
  // Do this after the server may have started,
  // but before the client tries to auto-connect
  if (com_dedicated->modified)
  {
    // get the latched value
    Cvar_Get("dedicated", "0", 0);
    com_dedicated->modified = qfalse;
    if (!com_dedicated->integer)
    {
      SV_Shutdown("dedicated set to 0");
      CL_FlushMemory();
    }
  }

#ifndef DEDICATED
  //
  // client system
  //
  //
  // run event loop a second time to get server to client packets
  // without a frame of latency
  //
  if (com_speeds->integer)
    timeBeforeEvents = Sys_Milliseconds();

  Com_EventLoop();
  Cbuf_Execute();

  //
  // client side
  //
  if (com_speeds->integer)
    timeBeforeClient = Sys_Milliseconds();

  CL_Frame(msec);

  if (com_speeds->integer)
    timeAfter = Sys_Milliseconds();

#else
  if ( com_speeds->integer )
  {
    timeAfter = Sys_Milliseconds ();
    timeBeforeEvents = timeAfter;
    timeBeforeClient = timeAfter;
  }
#endif

	NET_FlushPacketQueue();
  //
  // report timing information
  //
  if (com_speeds->integer)
  {
    int all, sv, ev, cl;
    all = timeAfter - timeBeforeServer;
    sv = timeBeforeEvents - timeBeforeServer;
    ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
    cl = timeAfter - timeBeforeClient;
    sv -= time_game;
    cl -= time_frontend + time_backend;

		Com_Printf ("frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
					 com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
  }

  //
  // trace optimization tracking
  //
  if (com_showtrace->integer)
  {
    extern int c_traces, c_brush_traces, c_patch_traces, c_trisoup_traces;
    extern int c_pointcontents;

    Com_Printf("%4i traces  (%ib %ip %it) %4i points\n", c_traces, c_brush_traces, c_patch_traces, c_trisoup_traces,
        c_pointcontents);
    c_traces = 0;
    c_brush_traces = 0;
    c_patch_traces = 0;
    c_trisoup_traces = 0;
    c_pointcontents = 0;
  }

	Com_ReadFromPipe( );

  com_frameNumber++;
}

/*
 =================
 Com_Shutdown
 =================
 */
void Com_Shutdown(void)
{
  if (logfile)
  {
    FS_FCloseFile(logfile);
    logfile = 0;
  }

  if (com_journalFile)
  {
    FS_FCloseFile(com_journalFile);
    com_journalFile = 0;
  }

	if ( pipefile )
	{
		FS_FCloseFile( pipefile );
		FS_HomeRemove( com_pipefile->string );
	}
}

//------------------------------------------------------------------------
/*
 =====================
 Q_acos
 the msvc acos doesn't always return a value between -PI and PI:
 int i;
 i = 1065353246;
 acos(*(float*) &i) == -1.#IND0
 This should go in q_math but it is too late to add new traps
 to game and ui
 =====================
 */
float Q_acos(float c)
{
  float angle;

  angle = acos(c);

  if (angle > M_PI)
    return (float)M_PI;

  if (angle < -M_PI)
    return (float)M_PI;

  return angle;
}

/*
 ===========================================
 command line completion
 ===========================================
 */
/*
 ==================
 Field_Clear
 ==================
 */
void Field_Clear(field_t *edit)
{
  Com_Memset(edit->buffer, 0, MAX_EDIT_LINE);
  edit->cursor = 0;
  edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int matchCount;
// field we are working on, passed to Field_AutoComplete(&g_consoleCommand for instance)
static field_t *completionField;

/*
 ===============
 FindMatches
 ===============
 */
static void FindMatches(const char *s)
{
  unsigned int i;

  if (Q_stricmpn(s, completionString, qstrlen(completionString)))
    return;

  matchCount++;
  if (matchCount == 1)
  {
    Q_strncpyz(shortestMatch, s, sizeof(shortestMatch));
    return;
  }

  // cut shortestMatch to the amount common with s
  for (i = 0; shortestMatch[i]; i++)
  {
    if (i >= qstrlen(s))
    {
      shortestMatch[i] = 0;
      break;
    }

    if (tolower(shortestMatch[i]) != tolower(s[i]))
      shortestMatch[i] = 0;
  }
}

/*
 ===============
 PrintMatches
 ===============
 */
static void PrintMatches(const char *s)
{
  if (!Q_stricmpn(s, shortestMatch, qstrlen(shortestMatch)))
    Com_Printf("    %s\n", s);
}

/*
 ===============
 PrintCvarMatches
 ===============
 */
static void PrintCvarMatches(const char *s)
{
  char value[TRUNCATE_LENGTH];

  if (!Q_stricmpn(s, shortestMatch, qstrlen(shortestMatch)))
  {
    Com_TruncateLongString(value, Cvar_VariableString(s));
    Com_Printf("    %s = \"%s\"\n", s, value);
  }
}

/*
 ===============
 Field_FindFirstSeparator
 ===============
 */
static char *Field_FindFirstSeparator(char *s)
{
  for (unsigned int i = 0; i < qstrlen(s); i++)
  {
    if (s[i] == ';')
      return &s[i];
  }
  return NULL;
}

/*
 ===============
 Field_Complete
 ===============
 */
static qboolean Field_Complete(void)
{
	size_t completionOffset;

  if (matchCount == 0)
    return qtrue;

  completionOffset = qstrlen(completionField->buffer) - qstrlen(completionString);

  Q_strncpyz(&completionField->buffer[completionOffset], shortestMatch, sizeof(completionField->buffer) - completionOffset);

  completionField->cursor = qstrlen(completionField->buffer);

  if (matchCount == 1)
  {
    Q_strcat(completionField->buffer, sizeof(completionField->buffer), " ");
    completionField->cursor++;
    return qtrue;
  }

  Com_Printf("]%s\n", completionField->buffer);

  return qfalse;
}

#ifndef DEDICATED
/*
 ===============
 Field_CompleteKeyname
 ===============
 */
void Field_CompleteKeyname(void)
{
  matchCount = 0;
  shortestMatch[0] = 0;

  Key_KeynameCompletion(FindMatches);

  if (!Field_Complete())
    Key_KeynameCompletion(PrintMatches);
}
#endif

/*
 ===============
 Field_CompleteFilename
 ===============
 */
void Field_CompleteFilename( const char *dir,	const char *ext, qboolean stripExt, qboolean allowNonPureFilesOnDisk )
{
  matchCount = 0;
  shortestMatch[0] = 0;

	FS_FilenameCompletion( dir, ext, stripExt, FindMatches, allowNonPureFilesOnDisk );

  if (!Field_Complete())
		FS_FilenameCompletion( dir, ext, stripExt, PrintMatches, allowNonPureFilesOnDisk );
}

/*
 ===============
 Field_CompleteCommand
 ===============
 */
void Field_CompleteCommand( char *cmd, qboolean doCommands, qboolean doCvars )
{
  int completionArgument = 0;

  // Skip leading whitespace and quotes
  cmd = Com_SkipCharset(cmd, " \"");

  Cmd_TokenizeStringIgnoreQuotes(cmd);
  completionArgument = Cmd_Argc();

  // If there is trailing whitespace on the cmd
  if (*(cmd + qstrlen(cmd) - 1) == ' ')
  {
    completionString = "";
    completionArgument++;
  }
  else
    completionString = Cmd_Argv(completionArgument == 0 ? 0 : completionArgument - 1);

#ifndef DEDICATED
  // Unconditionally add a '\' to the start of the buffer
	if ( completionField->buffer[ 0 ] && completionField->buffer[ 0 ] != '\\' )
  {
    if (completionField->buffer[0] != '/')
    {
      // Buffer is full, refuse to complete
			if (qstrlen(completionField->buffer ) + 1 >= sizeof(completionField->buffer))
        return;

			Com_Memmove( &completionField->buffer[1],	&completionField->buffer[0], qstrlen(completionField->buffer) + 1);
      completionField->cursor++;
    }
    completionField->buffer[0] = '\\';
  }
#endif

  if (completionArgument > 1)
  {
    const char *baseCmd = Cmd_Argv(0);
    char *p;

#ifndef DEDICATED
    // This should always be true
    if (baseCmd[0] == '\\' || baseCmd[0] == '/')
      baseCmd++;
#endif
	p = Field_FindFirstSeparator(cmd);
    if (p)
      Field_CompleteCommand(p + 1, qtrue, qtrue); // Compound command
    else
      Cmd_CompleteArgument(baseCmd, cmd, completionArgument);
  }
  else
  {
    if (completionString[0] == '\\' || completionString[0] == '/')
      completionString++;

    matchCount = 0;
    shortestMatch[0] = 0;

    if (qstrlen(completionString) == 0)
      return;

    if (doCommands)
      Cmd_CommandCompletion(FindMatches);

    if (doCvars)
      Cvar_CommandCompletion(FindMatches);

    if (!Field_Complete())
    {
      // run through again, printing matches
      if (doCommands)
        Cmd_CommandCompletion(PrintMatches);

      if (doCvars)
        Cvar_CommandCompletion(PrintCvarMatches);
    }
  }
}

/*
 ===============
 Field_AutoComplete
 Perform Tab expansion
 ===============
 */
void Field_AutoComplete(field_t *field)
{
  completionField = field;

  Field_CompleteCommand(completionField->buffer, qtrue, qtrue);
}

/*
 ==================
 Com_RandomBytes
 fills string array with len radom bytes, peferably from the OS randomizer
 ==================
 */
void Com_RandomBytes(byte *string, int len)
{
  int i;

  if (Sys_RandomBytes(string, len))
    return;

  Com_Printf("Com_RandomBytes: using weak randomization\n");
  for (i = 0; i < len; i++)
    string[i] = (unsigned char)(rand() % 255);
}
/*
==================
Com_IsVoipTarget
Returns non-zero if given clientNum is enabled in voipTargets, zero otherwise.
If clientNum is negative return if any bit is set.
==================
*/
qboolean Com_IsVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum)
{
	int index;
	if (clientNum < 0)
	{
		for(index = 0; index < voipTargetsSize; index++)
		{
			if (voipTargets[index])
				return qtrue;
		}
		return qfalse;
	}
	index = clientNum >> 3;
	if (index < voipTargetsSize)
		return (voipTargets[index] & (1 << (clientNum & 0x07)));
	return qfalse;
}

typedef struct
{
  char mediaName[MAX_TOKEN_CHARS];
  char copyright[MAX_TOKEN_CHARS];
  char license[MAX_TOKEN_CHARS];
  char licenseURL[MAX_TOKEN_CHARS];
  char source[MAX_TOKEN_CHARS];
} copyrightEntry_t;

#if 0 //FIXME: 0xA5EA unused
static int QDECL MediaNameCompare(const void *a, const void *b)
{
  char *s1, *s2;
  int c1, c2;
  copyrightEntry_t *e1, *e2;

  e1 = (copyrightEntry_t *) a;
  e2 = (copyrightEntry_t *) b;

  s1 = e1->mediaName;
  s2 = e2->mediaName;

  //return Q_stricmp(s1, s2);


  do
  {
    c1 = *s1++;
    c2 = *s2++;

    if (c1 >= 'a' && c1 <= 'z')
    {
      c1 -= ('a' - 'A');
    }
    if (c2 >= 'a' && c2 <= 'z')
    {
      c2 -= ('a' - 'A');
    }

    if (c1 == '\\' || c1 == ':')
    {
      c1 = '/';
    }
    if (c2 == '\\' || c2 == ':')
    {
      c2 = '/';
    }

    if (c1 < c2)
    {
      // strings not equal
      return -1;
    }
    if (c1 > c2)
    {
      return 1;
    }
  }while(c1);

  // strings are equal
  return 0;
}
#endif
#ifndef NO_DEVELOPER_CMDS
static void Com_PrintMatrix(const matrix_t m)
{
  Com_Printf("%.3f %.3f %.3f %.3f\n"
    "%.3f %.3f %.3f %.3f\n"
    "%.3f %.3f %.3f %.3f\n"
    "%.3f %.3f %.3f %.3f\n\n", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14],
      m[15]);
}

static void Com_GenerateMediaTXT_f(void)
{
  int i;
  fileHandle_t f;
  char fileName[MAX_QPATH];
  char *buf;
  char *buf_p;
  char *token;
  char propertyName[MAX_TOKEN_CHARS];
  char propertyValue[MAX_TOKEN_CHARS];
  size_t len;
  int shutupCompiler = 0;
  char *svnCmd = "svn proplist -R -v "BASEGAME "/ > " BASEGAME "/PROPERTIES.txt";
  
  //hypov8 add: size_t
  const size_t maxMediaLen = 100;
  const size_t maxCopyrightLen = 50;
  const size_t maxLicenseLen = 90;
  const size_t maxLicenseURLLen = 70;

  growList_t list;
  copyrightEntry_t *entry;

  // FIXME: this assumes fs_game is set to the default: "base"
  shutupCompiler = system(svnCmd);
  (void)shutupCompiler;
  //system("svn proplist -R -v base/ > BASEGAME/PROPERTIES.txt");

  Com_sprintf(fileName, sizeof(fileName), "PROPERTIES.txt");
  Com_Printf("reading '%s' ...\n", fileName);

  FS_ReadFile(fileName, (void **)&buf);
  if (!buf)
  {
    Com_Printf("couldn't load '%s'\n", fileName);
    return;
  }

  Com_sprintf(fileName, sizeof(fileName), "MEDIA.txt");
  Com_Printf("writing '%s' ...\n", fileName);
  f = FS_FOpenFileWrite(fileName);
  if (!f)
  {
    Com_Printf("Couldn't write %s.\n", fileName);
    return;
  }

  Com_InitGrowList(&list, 1000);

  FS_Printf(f, "// generated by KingpinQ3\n\n"
    "This file lists all files in the KingpinQ3 subversion repository and it shows information about each file.\n\n\n"
    "Used licenses are listed below:\n\n"

    "Creative Commons Sampling Plus 1.0\n"
    "http://creativecommons.org/licenses/sampling+/1.0/\n\n"

    "Creative Commons Attribution-NonCommercial-ShareAlike 2.5\n"
    "http://creativecommons.org/licenses/by-nc-sa/2.5/\n\n"
    "Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported\n"

    "http://creativecommons.org/licenses/by-nc-sa/3.0/\n\n"
    "Creative Commons Attribution-ShareAlike 2.5\n"
    "http://creativecommons.org/licenses/by-sa/2.5/\n\n"

    "Creative Commons Attribution 2.5\n"
    "http://creativecommons.org/licenses/by/2.5/\n\n"

    "Free Art License 1.3\n"
    "http://artlibre.org/licence/lal/en/\n\n"

    "\n\n\n");

  entry = (copyrightEntry_t*)Com_Allocate(sizeof(*entry));

  Q_strncpyz(entry->mediaName, "FILE", sizeof(entry->mediaName));
  Q_strncpyz(entry->copyright, "COPYRIGHT", sizeof(entry->copyright));
  Q_strncpyz(entry->license, "LICENSE", sizeof(entry->license));
  Q_strncpyz(entry->licenseURL, "LICENSE URL", sizeof(entry->licenseURL));
  Q_strncpyz(entry->source, "SOURCE", sizeof(entry->source));

  Com_AddToGrowList(&list, entry);

  // TODO add texture and map packs

  buf_p = buf;
  while (qtrue)
  {
    token = Com_ParseExt(&buf_p, qtrue);
    if (!token[0])
      break;

    // bleh german shell
    if (!Q_stricmp(token, "properties") || !Q_stricmp(token, "eigenschaften"))
    {
      entry = (copyrightEntry_t*)Com_Allocate(sizeof(*entry));

      entry->mediaName[0] = '\0';
      Q_strncpyz(entry->copyright, "<unknown: add svn:copyright>", sizeof(entry->copyright));
      Q_strncpyz(entry->license, "<unspecified: add svn:license>", sizeof(entry->license));
      entry->licenseURL[0] = '\0';
      entry->source[0] = '\0';

      Com_AddToGrowList(&list, entry);

      // skip "on" or "zu"
      token = Com_ParseExt(&buf_p, qfalse);

      // parse filename
      while (qtrue)
      {
        token = Com_ParseExt(&buf_p, qfalse);
        if (!token[0])
          break;

        if (!qstrcmp(token, ":"))
          continue;

        Q_strcat(entry->mediaName, sizeof(entry->mediaName), token);
        //Q_strcat(propertyValue, sizeof(propertyValue), " ");
      }
    }
    else
    {
      // assume it is a property
      // OK we can start parsing the properties
      Q_strncpyz(propertyName, token, sizeof(propertyName));

      // parse property value
      propertyValue[0] = '\0';
      while (qtrue)
      {
        token = Com_ParseExt(&buf_p, qfalse);
        if (!token[0])
          break;

        if (!qstrcmp(token, ":"))
          continue;

        Q_strcat(propertyValue, sizeof(propertyValue), token);

        if (Q_stricmp(propertyName, "svn:licenseURL") && Q_stricmp(propertyName, "svn:source"))
          Q_strcat(propertyValue, sizeof(propertyValue), " ");
      }

      if (!Q_stricmp(propertyName, "svn:copyright"))
        Q_strncpyz(entry->copyright, propertyValue, sizeof(entry->copyright));
      else if (!Q_stricmp(propertyName, "svn:license"))
        Q_strncpyz(entry->license, propertyValue, sizeof(entry->license));
      else if (!Q_stricmp(propertyName, "svn:licenseURL"))
        Q_strncpyz(entry->licenseURL, propertyValue, sizeof(entry->licenseURL));
      else if (!Q_stricmp(propertyName, "svn:source"))
        Q_strncpyz(entry->source, propertyValue, sizeof(entry->source));
    }
  }

  //qsort(list.elements, list.currentElements, sizeof(void *), MediaNameCompare);

  for (i = 0; i < list.currentElements; i++)
  {
    entry = (copyrightEntry_t*)Com_GrowListElement(&list, i);

    len = qstrlen(entry->mediaName);
    if (Q_stricmp(entry->mediaName + len - 4, ".txt") != 0 && Q_stricmp(entry->mediaName + len - 4, ".cfg") != 0 && Q_stricmp(
        entry->mediaName + len - 4, ".def") != 0 && Q_stricmp(entry->mediaName + len - 4, ".pro") != 0 && Q_stricmp(
        entry->mediaName + len - 4, ".exe") != 0 && Q_stricmp(entry->mediaName + len - 4, ".dll") != 0 && Q_stricmp(
        entry->mediaName + len - 7, ".vcproj") != 0 && Q_stricmp(entry->mediaName + len - 4, ".dat") != 0 && Q_stricmp(
        entry->mediaName + len - 4, ".lua") != 0 && Q_stricmp(entry->mediaName + len - 5, ".glsl") != 0 && Q_stricmp(
        entry->mediaName + len - 6, ".voice") != 0 && Q_stricmp(entry->mediaName + len - 6, ".arena") != 0 && Q_stricmp(
        entry->mediaName + len - 2, ".c") != 0 && Q_stricmp(entry->mediaName + len - 2, ".h") != 0 && Q_stricmp(
        entry->mediaName + len - 3, ".sh") != 0 && Q_stricmp(entry->mediaName + len - 4, ".mtr") != 0)
    {
      FS_Printf(f, "%s", entry->mediaName);
      len = qstrlen(entry->mediaName);
      while (len < (size_t)maxMediaLen)
      {
        FS_Printf(f, " ");
        len++;
      }

      FS_Printf(f, "%s", entry->copyright);
      len = qstrlen(entry->copyright);
      while (len < (size_t)maxCopyrightLen)
      {
        FS_Printf(f, " ");
        len++;
      }

      FS_Printf(f, "%s", entry->license);
      len = qstrlen(entry->license);
	  while (len < (size_t)maxLicenseLen)
      {
        FS_Printf(f, " ");
        len++;
      }

      FS_Printf(f, "%s", entry->licenseURL);
      len = qstrlen(entry->licenseURL);
	  while (len < (size_t)maxLicenseURLLen)
      {
        FS_Printf(f, " ");
        len++;
      }

      FS_Printf(f, "%s\n", entry->source);

      if (i == 0)
      {
        // do a separator line
        len = 0;
        while (len < (maxMediaLen + maxCopyrightLen + maxLicenseLen + maxLicenseURLLen + 6))
        {
          FS_Printf(f, "=");
          len++;
        }
        FS_Printf(f, "\n");
      }
    }

    Com_Dealloc(entry);
  }

  Com_DestroyGrowList(&list);

  FS_FCloseFile(f);
  FS_FreeFile(buf);
}

#ifdef USE_ASM_LIB
static void Com_MathTSCTest_f(void)
{
  uint64_t tsc1, tsc2;
  vec3_t ALIGN16(vec2);
  vec4_t ALIGN16(v4);
  vec_t s, c;
  qboolean b;
  (void)c; // shutup compiler
  (void)s;
 // Serialize();
  tsc1 = ReadTSC();
  VectorSet(vec2, 0.31f, 0.41f, 3);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("VectorSet() marco:  %d TSC\n", (int)(tsc2 - tsc1));

  // Serialize();
  tsc1 = ReadTSC();
  Vector4Set(v4, 0.31f, 0.41f, 3.221f, 1.0f);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("Vector4Set() marco:  %d TSC\n", (int)(tsc2 - tsc1));
#ifdef USING_SSE_MATH
  //Serialize();
  tsc1 = ReadTSC();
  Vec4_SetSSE(0.31f, v4);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("Vec4_SetSSE() marco:  %d TSC\n", (int)(tsc2 - tsc1));
#endif

  //Serialize();
  tsc1 = ReadTSC();
  s = sin(3.4);
  c = cos(3.4);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("sin cos() %d TSC\n", (int)(tsc2 - tsc1));

  //Serialize();
  tsc1 = ReadTSC();
  b = MatrixCompare(Hilbert4, Hilbert4);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("MatrixCompare() = %d; %d TSC\n", b, (int)(tsc2 - tsc1));
#ifdef USING_SSE_MATH
  //Serialize();
  tsc1 = ReadTSC();
  b = Mat4_CompareSSE(Hilbert4, Hilbert4);
  tsc2 = ReadTSC();
  //Serialize();
  Com_Printf("Mat4_CompareSSE() = %d; %d TSC\n", b, (int)(tsc2 - tsc1));
#endif
}
#endif

#define I_END (1<<23)
static void Com_MathTest_f(void)
{
  vec3_t vec1 =
  { -90.f, 90.f, 0.f };
  vec3_t vec2 =
  { 0 };

  matrix_t ALIGN16(m); //, m2;
  matrix_t ALIGN16(invHilb);
  matrix_t ALIGN16(out);
  vec4_t ALIGN16(vsse);
  vec4_t ALIGN16(vsse2);
  vec4_t ALIGN16(osse);
 // Loki::Printf("fjdklasf");
  //for (int i=100; i > 0; --i)
   //  Printf("Hey, %u frobnicators and %u twiddlicators\n")(i)(i);

  vec_t sin1, cos1;
  (void)sin1;
  (void)cos1;
  //static int jo = 1;
  int i, iend; // blah;
  int time1, time2;
#if 0
  if (!ASSERT_PTR16(out))
    Com_Error(ERR_FATAL, "Com_MathTest_f out not 16 byte alligned");
  if (!ASSERT_PTR16(m))
    Com_Error(ERR_FATAL, "Com_MathTest_f m not 16 byte alligned");
  if (!ASSERT_PTR16(invHilb))
    Com_Error(ERR_FATAL, "Com_MathTest_f invHilb not 16 byte alligned");
  if (!ASSERT_PTR16(vsse))
    Com_Error(ERR_FATAL, "Com_MathTest_f vsse not 16 byte alligned");
  if (!ASSERT_PTR16(vsse2))
    Com_Error(ERR_FATAL, "Com_MathTest_f vsse2 not 16 byte alligned");
  if (!ASSERT_PTR16(osse))
    Com_Error(ERR_FATAL, "Com_MathTest_f osse not 16 byte alligned");
#endif
  Com_Printf("...matrices\n");

  MatrixFromAngles(m, -90, 90, 0);
  Com_PrintMatrix(m);

  MatrixFromAngles(m, 90, -90, 0);
  Com_PrintMatrix(m);

  MatrixFromAngles(m, 90, 90, 0);
  Com_PrintMatrix(m);

  m[0] = 0;
  m[4] = 1;
  m[8] = 2;
  m[12] = 3;
  m[1] = 4;
  m[5] = 5;
  m[9] = 6;
  m[13] = 7;
  m[2] = 8;
  m[6] = 9;
  m[10] = 10;
  m[14] = 11;
  m[3] = 12;
  m[7] = 13;
  m[11] = 14;
  m[15] = 15;
  Com_PrintMatrix(m);

  MatrixMultiplyRotation(m, 90, 90, 0);

  Com_PrintMatrix(m);
  //MatrixSetupTranslation(m, 2, 4, 5);
  Com_PrintMatrix(Hilbert4);

  //	Matrix4x4_Copy(Hilbert4, invHilb);
  //	MatrixInverse(invHilb);
  //	Com_PrintMatrix(invHilb);

  MatrixCopy(matrixIdentity, invHilb);

  Com_Printf("invHilb =\n");
  Com_PrintMatrix(invHilb);
//hypov8 merge:
  _mat4x4_Inverse(Hilbert4, invHilb);
  Com_Printf("invHilb after calculation =\n");
  Com_PrintMatrix(invHilb);

  MatrixMultiply(Hilbert4, invHilb, out);
  Com_Printf("Hilb4 x invHilb=\n");
  Com_PrintMatrix(out);

#ifdef USING_SSE_MATH
  Mat4_ClearSSE(out);
  Mat4_MultiplySSE(Hilbert4, invHilb, out);
  Com_Printf("SSE Hilb4 x invHilb=\n");
  Com_PrintMatrix(out);

  Com_Printf("--\nvsse = %f %f %f %f\n", vsse[0], vsse[1], vsse[2], vsse[3]);
  Vector4Set(vsse, 1.f, 2.f, 3.f, 4.f);
  Vector4Set(vsse2, 1.f, 5.f, 5.f, 4.f);
  Com_Printf("--\nvsse = %f %f %f %f\n", vsse[0], vsse[1], vsse[2], vsse[3]);
  Vec4_AddSSE(vsse, vsse2, osse);
  Com_Printf("--\nosse = %f %f %f, %f\n", osse[0], osse[1], osse[2], osse[3]);
  Com_Printf("Vec4_CompSSE = %d\n", Vec4_CompSSE(vsse, vsse2) );
  Com_Printf("Vec4_CompSSE = %d\n", Vec4_CompSSE(vsse, vsse) );

  vec4_t ALIGN16(vtestcompare);
  Vector4Copy(vsse, vtestcompare);
  vec_t epsilon = 0.01f;
  Com_Printf("Vec4_AnyDistGtEpsilona = %d epsilon = %f\n", Vec4_AnyDistGtEpsilona(vsse, vtestcompare, epsilon), epsilon);

  vtestcompare[1] -= vec_t(0.001);
  Com_Printf("  vtestcompare[1] -= 0.001;\n");
  Com_Printf("Vec4_AnyDistGtEpsilona = %d epsilon = %f\n", Vec4_AnyDistGtEpsilona(vsse, vtestcompare, epsilon), epsilon);

  vtestcompare[1] += vec_t(0.001);
  vtestcompare[1] += vec_t(0.01);
  Com_Printf(" testcompare[1] += 0.01;\n");
  Com_Printf("Vec4_AnyDistGtEpsilona = %d epsilon = %f\n", Vec4_AnyDistGtEpsilona(vsse, vtestcompare, epsilon), epsilon);

  vtestcompare[1] += vec_t(0.001);
  Com_Printf(" vtestcompare[1] += 0.001\n");
  Com_Printf("Vec4_AnyDistGtEpsilona = %d epsilon = %f\n", Vec4_AnyDistGtEpsilona(vsse, vtestcompare, epsilon), epsilon);

  Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.f);
  Com_Printf("Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.f)  = %d\n", Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.f));

  Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.1f);
  Com_Printf("Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.1f) = %d\n", Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.1f));

  Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.0001f);
  Com_Printf("Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.0001f) = %d\n", Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.0001f));

  matrix_t ALIGN16(HilbertComp);
  MatrixCopy(Hilbert4, HilbertComp);

  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.f);
  Com_Printf("Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.f); = %d\n", Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.f));

  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.0000001f);
  Com_Printf("Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.0000001f); = %d\n", Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.0000001f));

  HilbertComp[0] -= vec_t(0.11);
  Com_Printf("HilbertComp[0] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f));
  Com_Printf(" HilbertComp[0] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f));

  Mat4_Copy(Hilbert4, HilbertComp);
  HilbertComp[4] -= vec_t(0.11);
  Com_Printf(" HilbertComp[4] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f));
  Com_Printf(" HilbertComp[4] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f));

  MatrixCopy(Hilbert4, HilbertComp);
  HilbertComp[8] -= vec_t(0.11);
  Com_Printf(" HilbertComp[8] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f));
  Com_Printf(" HilbertComp[8] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f));

  MatrixCopy(Hilbert4, HilbertComp);
  HilbertComp[12] -= vec_t(0.11);
  Com_Printf(" HilbertComp[12] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.1f));
  Com_Printf(" HilbertComp[12] -= 0.11; Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f); = %d\n",
  Mat4_CompEpsilonSSE(Hilbert4, HilbertComp, 0.12f));

#endif
  // Test Matrix compare Macro
#if 1
  Com_Printf("\n\nMat4_Compare(Hilbert4, Hilbert4) = %d\n", MatrixCompare(Hilbert4, Hilbert4));
  Com_Printf("\n\nMat4_Compare(Hilbert4, invHilb)  = %d\n", MatrixCompare(Hilbert4, invHilb));

  Com_Printf("==================\n  Matrix Compare benchmark\n==================\n ");
  iend = I_END;
  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
	MatrixCompare(Hilbert4, Hilbert4);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_CompareStd(Hilbert4, Hilbert4) :  %d msec\n", iend, time2 - time1);
#ifdef USING_SSE_MATH
  time1 = Com_Milliseconds();
  for(i=0; i < iend; i++)
  {
    Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.f);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_CompEpsilonSSE(Hilbert4, Hilbert4, 0.f) :  %d msec\n", iend, time2 - time1);

  Com_Printf("\nMat4_CompareSSE(Hilbert4, invHilb)  = %d\n", Mat4_CompareSSE(Hilbert4, invHilb));
  Com_Printf("Mat4_CompareSSE(Hilbert4, Hilbert4) = %d\n", Mat4_CompareSSE(Hilbert4, Hilbert4));
  time1 = Com_Milliseconds();

  for(i=0; i < iend; i++)
  {
    Mat4_CompareSSE(Hilbert4, Hilbert4);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_CompareSSE(Hilbert4, Hilbert4) :  %d msec\n", iend, time2 - time1);
#endif
#endif
  //	time1 = Com_Milliseconds();

  //	for(i=0; i < 100000; i++)
  //	{
  //		Matrix4x4_Copy(Hilbert4, invHilb);
  ////		MatrixInverse(invHilb);
  //	}
  //	time2 = Com_Milliseconds();

  //	Com_Printf("100000xInverse with MatrixInverse() %d msec\n", time2 - time1);

#if 0
  time1 = Com_Milliseconds();

  for(i=0; i < 100000; i++)
  {
    _mat4x4_Inverse(Hilbert4, invHilb);
  }
  time2 = Com_Milliseconds();
  Com_Printf("100000xInverse with _mat4x4_Inverse() %d msec\n", time2 - time1);

  MatrixCopy(ASymetric4, invHilb);
  MatrixInverse(invHilb);
  Com_PrintMatrix(invHilb);
#endif
  //
  // Matrix Multiply benchmark
  //
#if 1
  Com_Printf("==================\n  MatrixMulitply benchmark\n==================\n ");
  iend = I_END;
  MatrixCopy(matrixIdentity, out );
  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
	MatrixMultiply(Hilbert4, ASymetric4, out);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_MultiplyStd() :  %d msec\n", iend, time2 - time1);
  Com_PrintMatrix(out);

#ifdef USING_SSE_MATH
  MatrixCopy(matrixIdentity, out );
  time1 = Com_Milliseconds();
  for(i=0; i < iend; i++)
  {
    MatrixMultiply(Hilbert4, ASymetric4, out);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x MatrixMultiply() :  %d msec\n", iend, time2 - time1);
  Com_PrintMatrix(out);

  MatrixCopy(matrixIdentity, out );
  time1 = Com_Milliseconds();
  for(i=0; i < iend; i++)
  {
    Mat4_MultiplySSE(Hilbert4, ASymetric4, out);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_MultiplySSE() :  %d msec\n", iend, time2 - time1);
  Com_PrintMatrix(out);
#endif
#endif

#if 1
  {
  Com_Printf("==================\n  Mat4_Copy benchmark\n==================\n");
  matrix_t ALIGN16(testm);
  iend = I_END;
  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
    MatrixCopy(Hilbert4, testm);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Mat4_CopyStd() :  %d msec\n", iend, time2 - time1);

  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
    MatrixCopy(Hilbert4, testm);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x MatrixCopy() :  %d msec\n", iend, time2 - time1);

#ifdef USING_SSE_MATH
  Com_PrintMatrix(testm);
  Mat4_ClearSSE(testm);
  Com_PrintMatrix(testm);
  for(i=0; i < iend; i++)
  {
    Mat4_CopySSE(Hilbert4, testm);
  }
  time2 = Com_Milliseconds();
  Com_PrintMatrix(testm);
  Com_Printf("%d x Mat4_CopySSE() :  %d msec\n", iend, time2 - time1);
#endif
  }
  Com_Printf("Vector copy functions benchmark\n");

  Com_Printf("--\nvec1 = %f %f %f\n", vec1[0], vec1[1], vec1[2]);
  Com_Printf("--\nvec2 = %f %f %f\n", vec2[0], vec2[1], vec2[2]);

  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
    float s, c;
    vec3_t v;
    VectorSet(v, 0.31f + i, 0.41f + i, 3);
    VectorCopy(vec2, vec1);
    c = sin(vec1[0]);
    s = cos(vec1[0]);
    c = s;
    (void)c;
  }

  time2 = Com_Milliseconds();
  Com_Printf("%d x VectorCopy() marco:  %d msec\n", iend, time2 - time1);

  time1 = Com_Milliseconds();
  for (i = 0; i < iend; i++)
  {
    Vec4_Add(vsse, vsse2, osse);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Vec4_Add() :  %d msec\n", iend, time2 - time1);
#ifdef USING_SSE_MATH
  time1 = Com_Milliseconds();
  for(i=0; i < iend; i++)
  {
    Vec4_AddSSE(vsse, vsse2, osse);
  }
  time2 = Com_Milliseconds();
  Com_Printf("%d x Vec4_AddSSE() :  %d msec\n", iend, time2 - time1);
#endif
#endif
#if 1
  Com_Printf("==================\n Sin Cos benchmark\n==================\n");
  iend = I_END;
  time1 = Com_Milliseconds();
  sin1 = 0.f; cos1 = 0.f;
  for (i = 0; i < iend; i++)
  {
    sin1 += sin((vec_t)i);
    cos1 += cos((vec_t)i);
  }
  time2 = Com_Milliseconds();
  Com_Printf("sin %f cos %f \n", sin1, cos1);
  Com_Printf("%d x sin(i) :  %d msec\n", iend, time2 - time1);
#ifdef  USE_CORDIC_ALGORITHM
  time1 = Com_Milliseconds();
  sin1 = 0.f; cos1 = 0.f;
  for (i = 0; i < iend; i++)
  {
    //Q_SinCos( i,&sin1, &cos1);
    sin1 += Q_sin(i);
    cos1 += Q_cos(i);
    //blah = TranslateCordicAlpha( &i );
  }
  time2 = Com_Milliseconds();
  Com_Printf("sin %f cos %f \n", sin1, cos1);
  Com_Printf("%d x Q_sin(i) :  %d msec\n", iend, time2 - time1);
#endif // USE_CORDIC_ALGORITHM
#endif

}
static void Com_GenerateCorePK3_f(void)
{
  qtime_t dt =
  { 0 };
  char fileName[MAX_QPATH];
  char *buf;
  char *buf_p;
  char *token;
  char pakName[MAX_TOKEN_CHARS];
  char mediaName[MAX_TOKEN_CHARS];
  char propertyName[MAX_TOKEN_CHARS];
  //char propertyValue[MAX_TOKEN_CHARS];
  int unused;

  //Com_Memset(&dt, 0, sizeof(qtime_t));
  // FIXME: this assumes fs_game is set to the default: "base"
  unused = system("svn proplist -R -v basekpq3/ > basekpq3/PROPERTIES.txt");

  (void)unused;
  Com_sprintf(fileName, sizeof(fileName), "PROPERTIES.txt");
  Com_Printf("reading '%s' ...\n", fileName);

  FS_ReadFile(fileName, (void **)&buf);
  if (!buf)
  {
    Com_Printf("couldn't load '%s'\n", fileName);
    return;
  }

  Com_RealTime(&dt);
  Q_strncpyz(pakName, va("base/core-%04d%02d%02d.pk3", 1900 + dt.tm_year, dt.tm_mon + 1, dt.tm_mday), sizeof(pakName));
  Com_Printf("creating pak '%s' ...\n", pakName);

  // TODO add texture and map packs
  buf_p = buf;
  while (qtrue)
  {
    token = Com_ParseExt(&buf_p, qtrue);

    if (!token[0])
      break;

    // bleh german shell
    if (!Q_stricmp(token, "properties") || !Q_stricmp(token, "eigenschaften"))
    {
      // skip "on" or "zu"
      token = Com_ParseExt(&buf_p, qfalse);

      // parse filename name
      token = Com_ParseExt(&buf_p, qfalse);
      if (!qstrcmp(token, "'"))
      {
        // try again
        token = Com_ParseExt(&buf_p, qfalse);
      }
      if (!token[0])
        break;

      Q_strncpyz(mediaName, token, sizeof(mediaName));

#if 0
      len = qstrlen(mediaName);
      if (Q_stricmp(mediaName + len - 4, ".txt") != 0 &&
          Q_stricmp(mediaName + len - 4, ".cfg") != 0 &&
          Q_stricmp(mediaName + len - 4, ".dat") != 0 &&
          Q_stricmp(mediaName + len - 4, ".lua") != 0 &&
          Q_stricmp(mediaName + len - 5, ".glsl") != 0 &&
          Q_stricmp(mediaName + len - 6, ".voice") != 0 &&
          Q_stricmp(mediaName + len - 6, ".arena") != 0 &&
          Q_stricmp(mediaName + len - 2, ".c") != 0 &&
          Q_stricmp(mediaName + len - 2, ".h") != 0 &&
          Q_stricmp(mediaName + len - 3, ".sh") != 0 && Q_stricmp(mediaName + len - 4, ".mtr") != 0)
#endif
      {
        Com_Printf("...zipping '%s' ...\n", mediaName);
        //system(va("zip %s%s", pakName, mediaName));
        // TODO
      }
      mediaName[0] = '\0';
      Com_SkipRestOfLine(&buf_p);
    }
    else
    {
      // assume it is a property
      // OK we can start parsing the properties
      Q_strncpyz(propertyName, token, sizeof(propertyName));

      // parse property value
      //propertyValue[0] = '\0';
      while (qtrue)
      {
        token = Com_ParseExt(&buf_p, qfalse);
        if (!token[0])
          break;

        if (!qstrcmp(token, ":"))
          continue;
      }
    }
  }

  FS_FreeFile(buf);

  Com_Printf("finished creating pak '%s' ...\n", pakName);
}
#endif // NO_DEVELOPER_CMDS
// 0xA5EA: removed (unused)
#if 0
static void Com_DetectSSE(void)
{
#if id386_sse && !defined(DEDICATED)
  if (!(Sys_GetProcessorFeatures() & CF_SSE))
  {
    Com_Error(ERR_FATAL, "Binaries were compiled with SSE optimizations but your CPU doesn't support SSE");
  }
#endif
}
#endif
