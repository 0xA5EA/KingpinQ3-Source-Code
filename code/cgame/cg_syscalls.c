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
// cg_syscalls.c -- this file is only included when building a dll
// cg_syscalls.asm is included instead when building a qvm
#ifdef Q3_VM
#error "Do not use in VM build"
#endif

#include "cg_local.h"

static int FloatAsInt( float f )
{
  floatint_t fi;

  fi.f = f;
  return fi.i;
}

static intptr_t (QDECL * syscall)(intptr_t arg, ...) = (intptr_t (QDECL *)(intptr_t, ...)) - 1;

#ifdef __cplusplus
extern "C" Q_EXPORT void dllEntry(intptr_t (QDECL *syscallptr)(intptr_t arg, ...))
#else
Q_EXPORT void dllEntry(intptr_t (QDECL *syscallptr)(intptr_t arg, ...))
#endif
{
  syscall = syscallptr;
}

void trap_Print(const char *fmt)
{
  syscall(CG_PRINT, fmt);
}

void trap_Error(const char *fmt)
{
  syscall(CG_ERROR, fmt);
}

int trap_Milliseconds(void)
{
  return syscall(CG_MILLISECONDS);
}

void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags)
{
  syscall(CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

void trap_Cvar_Update(vmCvar_t *vmCvar)
{
  syscall(CG_CVAR_UPDATE, vmCvar);
}

void trap_Cvar_Set(const char *var_name, const char *value)
{
  syscall(CG_CVAR_SET, var_name, value);
}

void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize)
{
  syscall(CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, bufsize);
}

int trap_Argc(void)
{
  return syscall(CG_ARGC);
}

void trap_Argv(int n, char *buffer, int bufferLength)
{
  syscall(CG_ARGV, n, buffer, bufferLength);
}

void trap_Args(char *buffer, int bufferLength)
{
  syscall(CG_ARGS, buffer, bufferLength);
}

int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode)
{
  return syscall(CG_FS_FOPENFILE, qpath, f, mode);
}

void trap_FS_Read(void *buffer, size_t len, fileHandle_t f)
{
  syscall(CG_FS_READ, buffer, len, f);
}

void trap_FS_Write(const void *buffer, size_t len, fileHandle_t f)
{
  syscall(CG_FS_WRITE, buffer, len, f);
}

void trap_FS_FCloseFile(fileHandle_t f)
{
  syscall(CG_FS_FCLOSEFILE, f);
}

int trap_FS_Seek(fileHandle_t f, long offset, int origin)
{
  return syscall(CG_FS_SEEK, f, offset, origin);
}

int trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize)
{
	return syscall(CG_FS_GETFILELIST, path, extension, listbuf, bufsize);
}
void trap_SendConsoleCommand(const char *text)
{
  syscall(CG_SENDCONSOLECOMMAND, text);
}

void trap_AddCommand(const char *cmdName)
{
  syscall(CG_ADDCOMMAND, cmdName);
}

void trap_RemoveCommand(const char *cmdName)
{
  syscall(CG_REMOVECOMMAND, cmdName);
}

void trap_SendClientCommand(const char *s)
{
  syscall(CG_SENDCLIENTCOMMAND, s);
}

void trap_UpdateScreen(void)
{
  syscall(CG_UPDATESCREEN);
}

void trap_CM_LoadMap(const char *mapname)
{
  syscall(CG_CM_LOADMAP, mapname);
}

int trap_CM_NumInlineModels(void)
{
  return syscall(CG_CM_NUMINLINEMODELS);
}

clipHandle_t trap_CM_InlineModel(int index)
{
  return syscall(CG_CM_INLINEMODEL, index);
}

clipHandle_t trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs)
{
  return syscall(CG_CM_TEMPBOXMODEL, mins, maxs);
}

clipHandle_t trap_CM_TempCapsuleModel(const vec3_t mins, const vec3_t maxs)
{
  return syscall(CG_CM_TEMPCAPSULEMODEL, mins, maxs);
}

int trap_CM_PointContents(const vec3_t p, clipHandle_t model)
{
  return syscall(CG_CM_POINTCONTENTS, p, model);
}

int trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles)
{
  return syscall(CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles);
}

void trap_CM_BoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask)
{
  syscall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
}

void trap_CM_CapsuleTrace(trace_t * results, const vec3_t start, const vec3_t end,
						  const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask)
{
  syscall(CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model, brushmask);
}

void trap_CM_TransformedBoxTrace(trace_t * results, const vec3_t start, const vec3_t end,
								 const vec3_t mins, const vec3_t maxs,
								 clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles)
{
  syscall(CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}

void trap_CM_TransformedCapsuleTrace(trace_t * results, const vec3_t start, const vec3_t end,
									 const vec3_t mins, const vec3_t maxs,
									 clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles)
{
  syscall(CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}
void trap_CM_BiSphereTrace(trace_t * results, const vec3_t start,
                           const vec3_t end, float startRad, float endRad, clipHandle_t model, int mask)
{
  floatint_t startRadfi, endRadfi;
  startRadfi.f = startRad;
  endRadfi.f = endRad;
  syscall(CG_CM_BISPHERETRACE, results, start, end, startRadfi.i, endRadfi.i, model, mask);
}
void trap_CM_TransformedBiSphereTrace(trace_t * results, const vec3_t start,
                                      const vec3_t end, float startRad, float endRad,
                                      clipHandle_t model, int mask, const vec3_t origin)
{
  floatint_t startRadfi, endRadfi;
  startRadfi.f = startRad;
  endRadfi.f = endRad;
  syscall(CG_CM_TRANSFORMEDBISPHERETRACE, results, start, end, startRadfi.i, endRadfi.i, model, mask, origin);
}
int trap_CM_MarkFragments(int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer)
{
  return syscall(CG_CM_MARKFRAGMENTS, numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer);
}

void trap_S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx)
{
  syscall(CG_S_STARTSOUND, origin, entityNum, entchannel, sfx);
}

void trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum)
{
  syscall(CG_S_STARTLOCALSOUND, sfx, channelNum);
}

void trap_S_ClearLoopingSounds(qboolean killall)
{
  syscall(CG_S_CLEARLOOPINGSOUNDS, killall);
}

void trap_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx)
{
  syscall(CG_S_ADDLOOPINGSOUND, entityNum, origin, velocity, sfx);
}

void trap_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx)
{
  syscall(CG_S_ADDREALLOOPINGSOUND, entityNum, origin, velocity, sfx);
}

void trap_S_StopLoopingSound(int entityNum)
{
  syscall(CG_S_STOPLOOPINGSOUND, entityNum);
}

void trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin)
{
  syscall(CG_S_UPDATEENTITYPOSITION, entityNum, origin);
}

void trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater)
{
  syscall(CG_S_RESPATIALIZE, entityNum, origin, axis, inwater);
}

sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed)
{
  return syscall(CG_S_REGISTERSOUND, sample, compressed);
}

void trap_S_StartBackgroundTrack(const char *intro, const char *loop)
{
  syscall(CG_S_STARTBACKGROUNDTRACK, intro, loop);
}

void trap_R_LoadWorldMap(const char *mapname)
{
  syscall(CG_R_LOADWORLDMAP, mapname);
}

qhandle_t trap_R_RegisterModel(const char *name)
{
	return syscall(CG_R_REGISTERMODEL, name);
}

qhandle_t trap_R_RegisterAnimation(const char *name)
{
  return syscall(CG_R_REGISTERANIMATION, name);
}

qhandle_t trap_R_RegisterSkin(const char *name)
{
  return syscall(CG_R_REGISTERSKIN, name);
}

qhandle_t trap_R_RegisterShader(const char *name)
{
  return syscall(CG_R_REGISTERSHADER, name);
}

qhandle_t trap_R_RegisterShaderLightAttenuation(const char *name)
{
  return syscall(CG_R_REGISTERSHADERLIGHTATTENUATION, name);
}

qhandle_t trap_R_RegisterShaderNoMip(const char *name)
{
  return syscall(CG_R_REGISTERSHADERNOMIP, name);
}

void trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font)
{
  syscall(CG_R_REGISTERFONT, fontName, pointSize, font);
}

void trap_R_ClearScene(void)
{
  syscall(CG_R_CLEARSCENE);
}

void trap_R_AddRefEntityToScene(const refEntity_t *re)
{
  syscall(CG_R_ADDREFENTITYTOSCENE, re);
}

void trap_R_AddRefLightToScene(const refLight_t *light)
{
  syscall(CG_R_ADDREFLIGHTSTOSCENE, light);
}

void trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts)
{
  syscall(CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts);
}

// Added for flamethrower -KRYPTYK
void trap_R_AddPolyBufferToScene(polyBuffer_t* pPolyBuffer)
{
	syscall(CG_R_ADDPOLYBUFFERTOSCENE, pPolyBuffer);
}

void trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts, int num)
{
  syscall(CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, num);
}

int trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir)
{
  return syscall(CG_R_LIGHTFORPOINT, point, ambientLight, directedLight, lightDir);
}

void trap_R_AddAdditiveLightToScene(const vec3_t org, float intensity, float r, float g, float b)
{
  syscall(CG_R_ADDADDITIVELIGHTTOSCENE, org, FloatAsInt(intensity), FloatAsInt(r), FloatAsInt(g), FloatAsInt(b));
}

void trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags )
{
  syscall(CG_R_ADDLIGHTTOSCENE , org, FloatAsInt( radius ), FloatAsInt( intensity ), FloatAsInt( r ), FloatAsInt( g ), FloatAsInt( b ), hShader, flags );
}

void trap_R_RenderScene(const refdef_t *fd)
{
  syscall(CG_R_RENDERSCENE, fd);
}

void trap_R_SetColor(const float *rgba)
{
  syscall(CG_R_SETCOLOR, rgba);
}

void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader)
{
  syscall(CG_R_DRAWSTRETCHPIC, FloatAsInt(x), FloatAsInt(y), FloatAsInt(w), FloatAsInt(h), FloatAsInt(s1), FloatAsInt(t1), FloatAsInt(s2), FloatAsInt(t2), hShader);
}

void trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs)
{
  syscall(CG_R_MODELBOUNDS, model, mins, maxs);
}

#if defined(COMPAT_KPQ3) || defined(COMPAT_ET)
int trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex )
{
   return syscall(CG_R_LERPTAG,tag, refent, tagName, startIndex);
}
#else
int trap_R_LerpTag(orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName)
{
  floatint_t _frac;
  _frac.f = frac;
  return syscall(CG_R_LERPTAG, tag, mod, startFrame, endFrame, _frac.i, tagName);
}
#endif
int trap_R_CheckSkeleton(refSkeleton_t * skel, qhandle_t hModel, qhandle_t hAnim)
{
  return syscall(CG_R_CHECKSKELETON, skel, hModel, hAnim);
}

int trap_R_BuildSkeleton(refSkeleton_t *skel, qhandle_t anim, int startFrame, int endFrame, float frac, qboolean clearOrigin)
{
  floatint_t fi;
  fi.f = frac;
  return syscall(CG_R_BUILDSKELETON, skel, anim, startFrame, endFrame, fi.i, clearOrigin);
}

int trap_R_BlendSkeleton(refSkeleton_t *skel, const refSkeleton_t *blend, float frac)
{
  floatint_t fi;
  fi.f = frac;
  return syscall(CG_R_BLENDSKELETON, skel, blend, fi.i);
}

int trap_R_BoneIndex(qhandle_t hModel, const char *boneName)
{
  return syscall(CG_R_BONEINDEX, hModel, boneName);
}

int trap_R_AnimNumFrames(qhandle_t hAnim)
{
  return syscall(CG_R_ANIMNUMFRAMES, hAnim);
}

int trap_R_AnimFrameRate(qhandle_t hAnim)
{
  return syscall(CG_R_ANIMFRAMERATE, hAnim);
}

void trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset)
{
  syscall(CG_R_REMAP_SHADER, oldShader, newShader, timeOffset);
}

void trap_GetGlconfig(glconfig_t * glconfig)
{
  syscall(CG_GETGLCONFIG, glconfig);
}

void trap_GetGameState(gameState_t *gamestate)
{
  syscall(CG_GETGAMESTATE, gamestate);
}

void trap_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime)
{
  syscall(CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime);
}

qboolean trap_GetSnapshot(int snapshotNumber, snapshot_t *snapshot)
{
  return syscall(CG_GETSNAPSHOT, snapshotNumber, snapshot);
}

qboolean trap_GetServerCommand(int serverCommandNumber)
{
  return syscall(CG_GETSERVERCOMMAND, serverCommandNumber);
}

int trap_GetCurrentCmdNumber(void)
{
  return syscall(CG_GETCURRENTCMDNUMBER);
}

qboolean trap_GetUserCmd(int cmdNumber, usercmd_t *ucmd)
{
  return syscall(CG_GETUSERCMD, cmdNumber, ucmd);
}

void trap_SetUserCmdValue(int stateValue, float sensitivityScale)
{
  floatint_t _sensitivityScale;
  _sensitivityScale.f = sensitivityScale;
  syscall(CG_SETUSERCMDVALUE, stateValue, _sensitivityScale.i);
}

void testPrintInt(char *string, int i)
{
  syscall(CG_TESTPRINTINT, string, i);
}

void testPrintFloat(char *string, float f)
{
  floatint_t _f;
  _f.f = f;
  syscall(CG_TESTPRINTFLOAT, string, _f.i);
}

int trap_MemoryRemaining(void)
{
  return syscall(CG_MEMORY_REMAINING);
}

qboolean trap_Key_IsDown(int keynum)
{
  return syscall(CG_KEY_ISDOWN, keynum);
}

int trap_Key_GetCatcher(void)
{
  return syscall(CG_KEY_GETCATCHER);
}

void trap_Key_SetCatcher(int catcher)
{
  syscall(CG_KEY_SETCATCHER, catcher);
}

int trap_Key_GetKey(const char *binding)
{
  return syscall(CG_KEY_GETKEY, binding);
}

int trap_PC_AddGlobalDefine(char *define)
{
  return syscall(CG_PC_ADD_GLOBAL_DEFINE, define);
}

int trap_PC_LoadSource(const char *filename)
{
  return syscall(CG_PC_LOAD_SOURCE, filename);
}

int trap_PC_FreeSource(int handle)
{
  return syscall(CG_PC_FREE_SOURCE, handle);
}

int trap_PC_ReadToken(int handle, pc_token_t *pc_token)
{
  return syscall(CG_PC_READ_TOKEN, handle, pc_token);
}

int trap_PC_SourceFileAndLine(int handle, char *filename, int *line)
{
  return syscall(CG_PC_SOURCE_FILE_AND_LINE, handle, filename, line);
}

void trap_S_StopBackgroundTrack(void)
{
  syscall(CG_S_STOPBACKGROUNDTRACK);
}

int trap_RealTime(qtime_t *qtime)
{
  return syscall(CG_REAL_TIME, qtime);
}

void trap_SnapVector(float *v)
{
  syscall(CG_SNAPVECTOR, v);
}

// this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
int trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits)
{
  return syscall(CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}

// stops playing the cinematic and ends it.  should always return FMV_EOF
// cinematics must be stopped in reverse order of when they are started
e_status trap_CIN_StopCinematic(int handle)
{
  return (e_status)syscall(CG_CIN_STOPCINEMATIC, handle);
}


// will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
e_status trap_CIN_RunCinematic(int handle)
{
  return (e_status)syscall(CG_CIN_RUNCINEMATIC, handle);
}

// draws the current frame
void trap_CIN_DrawCinematic(int handle)
{
  syscall(CG_CIN_DRAWCINEMATIC, handle);
}

// allows you to resize the animation dynamically
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h)
{
  syscall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

/*
qboolean trap_loadCamera( const char *name ) {
    return syscall( CG_LOADCAMERA, name );
}

void trap_startCamera(int time) {
    syscall(CG_STARTCAMERA, time);
}

qboolean trap_getCameraInfo( int time, vec3_t *origin, vec3_t *angles) {
    return syscall( CG_GETCAMERAINFO, time, origin, angles );
}
*/

qboolean trap_GetEntityToken(char *buffer, int bufferSize)
{
  return syscall(CG_GET_ENTITY_TOKEN, buffer, bufferSize);
}

qboolean trap_R_inPVS(const vec3_t p1, const vec3_t p2)
{
  return syscall(CG_R_INPVS, p1, p2);
}
