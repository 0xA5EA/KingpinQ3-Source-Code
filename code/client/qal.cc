/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)
Copyright (C) 2006-xyyz Lars '0xA5EA' Kandler

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

// Dynamically loads OpenAL

#ifdef USE_OPENAL

#include "qal.h"

#ifdef USE_OPENAL_DLOPEN

#include "../sys/sys_loadlib.h"

LPALENABLE qalEnable;
LPALDISABLE qalDisable;
LPALISENABLED qalIsEnabled;
LPALGETSTRING qalGetString;
LPALGETBOOLEANV qalGetBooleanv;
LPALGETINTEGERV qalGetIntegerv;
LPALGETFLOATV qalGetFloatv;
LPALGETDOUBLEV qalGetDoublev;
LPALGETBOOLEAN qalGetBoolean;
LPALGETINTEGER qalGetInteger;
LPALGETFLOAT qalGetFloat;
LPALGETDOUBLE qalGetDouble;
LPALGETERROR qalGetError;
LPALISEXTENSIONPRESENT qalIsExtensionPresent;
LPALGETPROCADDRESS qalGetProcAddress;
LPALGETENUMVALUE qalGetEnumValue;
LPALLISTENERF qalListenerf;
LPALLISTENER3F qalListener3f;
LPALLISTENERFV qalListenerfv;
LPALLISTENERI qalListeneri;
LPALGETLISTENERF qalGetListenerf;
LPALGETLISTENER3F qalGetListener3f;
LPALGETLISTENERFV qalGetListenerfv;
LPALGETLISTENERI qalGetListeneri;
LPALGENSOURCES qalGenSources;
LPALDELETESOURCES qalDeleteSources;
LPALISSOURCE qalIsSource;
LPALSOURCEF qalSourcef;
LPALSOURCE3F qalSource3f;
LPALSOURCEFV qalSourcefv;
LPALSOURCEI qalSourcei;
LPALGETSOURCEF qalGetSourcef;
LPALGETSOURCE3F qalGetSource3f;
LPALGETSOURCEFV qalGetSourcefv;
LPALGETSOURCEI qalGetSourcei;
LPALSOURCEPLAYV qalSourcePlayv;
LPALSOURCESTOPV qalSourceStopv;
LPALSOURCEREWINDV qalSourceRewindv;
LPALSOURCEPAUSEV qalSourcePausev;
LPALSOURCEPLAY qalSourcePlay;
LPALSOURCESTOP qalSourceStop;
LPALSOURCEREWIND qalSourceRewind;
LPALSOURCEPAUSE qalSourcePause;
LPALSOURCEQUEUEBUFFERS qalSourceQueueBuffers;
LPALSOURCEUNQUEUEBUFFERS qalSourceUnqueueBuffers;
LPALGENBUFFERS qalGenBuffers;
LPALDELETEBUFFERS qalDeleteBuffers;
LPALISBUFFER qalIsBuffer;
LPALBUFFERDATA qalBufferData;
LPALGETBUFFERF qalGetBufferf;
LPALGETBUFFERI qalGetBufferi;
LPALDOPPLERFACTOR qalDopplerFactor;
LPALDOPPLERVELOCITY qalDopplerVelocity;
LPALDISTANCEMODEL qalDistanceModel;

LPALCCREATECONTEXT qalcCreateContext;
LPALCMAKECONTEXTCURRENT qalcMakeContextCurrent;
LPALCPROCESSCONTEXT qalcProcessContext;
LPALCSUSPENDCONTEXT qalcSuspendContext;
LPALCDESTROYCONTEXT qalcDestroyContext;
LPALCGETCURRENTCONTEXT qalcGetCurrentContext;
LPALCGETCONTEXTSDEVICE qalcGetContextsDevice;
LPALCOPENDEVICE qalcOpenDevice;
LPALCCLOSEDEVICE qalcCloseDevice;
LPALCGETERROR qalcGetError;
LPALCISEXTENSIONPRESENT qalcIsExtensionPresent;
LPALCGETPROCADDRESS qalcGetProcAddress;
LPALCGETENUMVALUE qalcGetEnumValue;
LPALCGETSTRING qalcGetString;
LPALCGETINTEGERV qalcGetIntegerv;
LPALCCAPTUREOPENDEVICE qalcCaptureOpenDevice;
LPALCCAPTURECLOSEDEVICE qalcCaptureCloseDevice;
LPALCCAPTURESTART qalcCaptureStart;
LPALCCAPTURESTOP qalcCaptureStop;
LPALCCAPTURESAMPLES qalcCaptureSamples;

static void *OpenALLib = NULL;

static qboolean alinit_fail = qfalse;

/*
=================
GPA
=================
*/
static void *GPA(char *str)
{
  void *rv;

  rv = Sys_LoadFunction(OpenALLib, str);
  if (!rv)
  {
    Com_Printf(" Can't load symbol %s\n", str);
    alinit_fail = qtrue;
    return NULL;
  }
  else
  {
    Com_DPrintf(" Loaded symbol %s (%p)\n", str, rv);
    return rv;
  }
}

/*
=================
QAL_Init
=================
*/
qboolean QAL_Init(const char *libname)
{
  if (OpenALLib)
    return qtrue;

	if(!(OpenALLib = Sys_LoadDll(libname, qtrue)))
    return qfalse;

  alinit_fail = qfalse;

  qalEnable               = (void (*)(ALenum))GPA("alEnable");
  qalDisable              = (void (*)(ALenum))GPA("alDisable");
  qalIsEnabled            = (ALboolean (*)(ALenum))GPA("alIsEnabled");
  qalGetString            = (const ALchar* (*)(ALenum))GPA("alGetString");
  qalGetBooleanv          = (void (*)(ALenum, ALboolean*))GPA("alGetBooleanv");
  qalGetIntegerv          = (void (*)(ALenum, ALint*))GPA("alGetIntegerv");
  qalGetFloatv            = (void (*)(ALenum, ALfloat*))GPA("alGetFloatv");
  qalGetDoublev           = (void (*)(ALenum, ALdouble*))GPA("alGetDoublev");
  qalGetBoolean           = (ALboolean (*)(ALenum))GPA("alGetBoolean");
  qalGetInteger           = (ALint (*)(ALenum))GPA("alGetInteger");
  qalGetFloat             = (ALfloat (*)(ALenum))GPA("alGetFloat");
  qalGetDouble            = (ALdouble (*)(ALenum))GPA("alGetDouble");
  qalGetError             = (ALenum (*)())GPA("alGetError");
  qalIsExtensionPresent   = (ALboolean (*)(const ALchar*))GPA("alIsExtensionPresent");
  qalGetProcAddress       = (void* (*)(const ALchar*))GPA("alGetProcAddress");
  qalGetEnumValue         = (ALenum (*)(const ALchar*))GPA("alGetEnumValue");
  qalListenerf            = (void (*)(ALenum, ALfloat))GPA("alListenerf");
  qalListener3f           = (void (*)(ALenum, ALfloat, ALfloat, ALfloat))GPA("alListener3f");
  qalListenerfv           = (void (*)(ALenum, const ALfloat*))GPA("alListenerfv");
  qalListeneri            = (void (*)(ALenum, ALint))GPA("alListeneri");
  qalGetListenerf         = (void (*)(ALenum, ALfloat*))GPA("alGetListenerf");
  qalGetListener3f        = (void (*)(ALenum, ALfloat*, ALfloat*, ALfloat*))GPA("alGetListener3f");
  qalGetListenerfv        = (void (*)(ALenum, ALfloat*))GPA("alGetListenerfv");
  qalGetListeneri         = (void (*)(ALenum, ALint*))GPA("alGetListeneri");
  qalGenSources           = (void (*)(ALsizei, ALuint*))GPA("alGenSources");
  qalDeleteSources        = (void (*)(ALsizei, const ALuint*))GPA("alDeleteSources");
  qalIsSource             = (ALboolean (*)(ALuint))GPA("alIsSource");
  qalSourcef              = (void (*)(ALuint, ALenum, ALfloat))GPA("alSourcef");
  qalSource3f             = (void (*)(ALuint, ALenum, ALfloat, ALfloat, ALfloat))GPA("alSource3f");
  qalSourcefv             = (void (*)(ALuint, ALenum, const ALfloat*))GPA("alSourcefv");
  qalSourcei              = (void (*)(ALuint, ALenum, ALint))GPA("alSourcei");
  qalGetSourcef           = (void (*)(ALuint, ALenum, ALfloat*))GPA("alGetSourcef");
  qalGetSource3f          = (void (*)(ALuint, ALenum, ALfloat*, ALfloat*, ALfloat*))GPA("alGetSource3f");
  qalGetSourcefv          = (void (*)(ALuint, ALenum, ALfloat*))GPA("alGetSourcefv");
  qalGetSourcei           = (void (*)(ALuint, ALenum, ALint*))GPA("alGetSourcei");
  qalSourcePlayv          = (void (*)(ALsizei, const ALuint*))GPA("alSourcePlayv");
  qalSourceStopv          = (void (*)(ALsizei, const ALuint*))GPA("alSourceStopv");
  qalSourceRewindv        = (void (*)(ALsizei, const ALuint*))GPA("alSourceRewindv");
  qalSourcePausev         = (void (*)(ALsizei, const ALuint*))GPA("alSourcePausev");
  qalSourcePlay           = (void (*)(ALuint))GPA("alSourcePlay");
  qalSourceStop           = (void (*)(ALuint))GPA("alSourceStop");
  qalSourceRewind         = (void (*)(ALuint))GPA("alSourceRewind");
  qalSourcePause          = (void (*)(ALuint))GPA("alSourcePause");
  qalSourceQueueBuffers   = (void (*)(ALuint, ALsizei, const ALuint*))GPA("alSourceQueueBuffers");
  qalSourceUnqueueBuffers = (void (*)(ALuint, ALsizei, ALuint*))GPA("alSourceUnqueueBuffers");
  qalGenBuffers           = (void (*)(ALsizei, ALuint*))GPA("alGenBuffers");
  qalDeleteBuffers        = (void (*)(ALsizei, const ALuint*))GPA("alDeleteBuffers");
  qalIsBuffer             = (ALboolean (*)(ALuint))GPA("alIsBuffer");
  qalBufferData           = (void (*)(ALuint, ALenum, const ALvoid*, ALsizei, ALsizei))GPA("alBufferData");
  qalGetBufferf           = (void (*)(ALuint, ALenum, ALfloat*))GPA("alGetBufferf");
  qalGetBufferi           = (void (*)(ALuint, ALenum, ALint*))GPA("alGetBufferi");
  qalDopplerFactor        = (void (*)(ALfloat))GPA("alDopplerFactor");
  qalDopplerVelocity      = (void (*)(ALfloat))GPA("alDopplerVelocity");
  qalDistanceModel        = (void (*)(ALenum))GPA("alDistanceModel");

  qalcCreateContext      = (ALCcontext* (*)(ALCdevice*, const ALCint*))GPA("alcCreateContext");
  qalcMakeContextCurrent = (ALCboolean (*)(ALCcontext*))GPA("alcMakeContextCurrent");
  qalcProcessContext     = (void (*)(ALCcontext*))GPA("alcProcessContext");
  qalcSuspendContext     = (void (*)(ALCcontext*))GPA("alcSuspendContext");
  qalcDestroyContext     = (void (*)(ALCcontext*))GPA("alcDestroyContext");
  qalcGetCurrentContext  = (ALCcontext* (*)())GPA("alcGetCurrentContext");
  qalcGetContextsDevice  = (ALCdevice* (*)(ALCcontext*))GPA("alcGetContextsDevice");
  qalcOpenDevice         = (ALCdevice* (*)(const ALCchar*))GPA("alcOpenDevice");
  qalcCloseDevice        = (ALCboolean (*)(ALCdevice*))GPA("alcCloseDevice");
  qalcGetError           = (ALCenum (*)(ALCdevice*))GPA("alcGetError");
  qalcIsExtensionPresent = (ALCboolean (*)(ALCdevice*, const ALCchar*))GPA("alcIsExtensionPresent");
  qalcGetProcAddress     = (void* (*)(ALCdevice*, const ALCchar*))GPA("alcGetProcAddress");
  qalcGetEnumValue       = (ALCenum (*)(ALCdevice*, const ALCchar*))GPA("alcGetEnumValue");
  qalcGetString          = (const ALCchar* (*)(ALCdevice*, ALCenum))GPA("alcGetString");
  qalcGetIntegerv        = (void (*)(ALCdevice*, ALCenum, ALCsizei, ALCint*))GPA("alcGetIntegerv");
  qalcCaptureOpenDevice  = (ALCdevice* (*)(const ALCchar*, ALCuint, ALCenum, ALCsizei))GPA("alcCaptureOpenDevice");
  qalcCaptureCloseDevice = (ALCboolean (*)(ALCdevice*))GPA("alcCaptureCloseDevice");
  qalcCaptureStart       = (void (*)(ALCdevice*))GPA("alcCaptureStart");
  qalcCaptureStop        = (void (*)(ALCdevice*))GPA("alcCaptureStop");
  qalcCaptureSamples     = (void (*)(ALCdevice*, ALCvoid*, ALCsizei))GPA("alcCaptureSamples");

  if (alinit_fail)
  {
    QAL_Shutdown();
    Com_Printf(" One or more symbols not found\n");
    return qfalse;
  }

  return qtrue;
}

/*
=================
QAL_Shutdown
=================
*/
void QAL_Shutdown(void)
{
  if (OpenALLib)
  {
    Sys_UnloadLibrary(OpenALLib);
    OpenALLib = NULL;
  }

  qalEnable               = NULL;
  qalDisable              = NULL;
  qalIsEnabled            = NULL;
  qalGetString            = NULL;
  qalGetBooleanv          = NULL;
  qalGetIntegerv          = NULL;
  qalGetFloatv            = NULL;
  qalGetDoublev           = NULL;
  qalGetBoolean           = NULL;
  qalGetInteger           = NULL;
  qalGetFloat             = NULL;
  qalGetDouble            = NULL;
  qalGetError             = NULL;
  qalIsExtensionPresent   = NULL;
  qalGetProcAddress       = NULL;
  qalGetEnumValue         = NULL;
  qalListenerf            = NULL;
  qalListener3f           = NULL;
  qalListenerfv           = NULL;
  qalListeneri            = NULL;
  qalGetListenerf         = NULL;
  qalGetListener3f        = NULL;
  qalGetListenerfv        = NULL;
  qalGetListeneri         = NULL;
  qalGenSources           = NULL;
  qalDeleteSources        = NULL;
  qalIsSource             = NULL;
  qalSourcef              = NULL;
  qalSource3f             = NULL;
  qalSourcefv             = NULL;
  qalSourcei              = NULL;
  qalGetSourcef           = NULL;
  qalGetSource3f          = NULL;
  qalGetSourcefv          = NULL;
  qalGetSourcei           = NULL;
  qalSourcePlayv          = NULL;
  qalSourceStopv          = NULL;
  qalSourceRewindv        = NULL;
  qalSourcePausev         = NULL;
  qalSourcePlay           = NULL;
  qalSourceStop           = NULL;
  qalSourceRewind         = NULL;
  qalSourcePause          = NULL;
  qalSourceQueueBuffers   = NULL;
  qalSourceUnqueueBuffers = NULL;
  qalGenBuffers           = NULL;
  qalDeleteBuffers        = NULL;
  qalIsBuffer             = NULL;
  qalBufferData           = NULL;
  qalGetBufferf           = NULL;
  qalGetBufferi           = NULL;
  qalDopplerFactor        = NULL;
  qalDopplerVelocity      = NULL;
  qalDistanceModel        = NULL;

  qalcCreateContext      = NULL;
  qalcMakeContextCurrent = NULL;
  qalcProcessContext     = NULL;
  qalcSuspendContext     = NULL;
  qalcDestroyContext     = NULL;
  qalcGetCurrentContext  = NULL;
  qalcGetContextsDevice  = NULL;
  qalcOpenDevice         = NULL;
  qalcCloseDevice        = NULL;
  qalcGetError           = NULL;
  qalcIsExtensionPresent = NULL;
  qalcGetProcAddress     = NULL;
  qalcGetEnumValue       = NULL;
  qalcGetString          = NULL;
  qalcGetIntegerv        = NULL;
  qalcCaptureOpenDevice  = NULL;
  qalcCaptureCloseDevice = NULL;
  qalcCaptureStart       = NULL;
  qalcCaptureStop        = NULL;
  qalcCaptureSamples     = NULL;
}
#else
qboolean QAL_Init(const char *libname)
{
  return qtrue;
}
void QAL_Shutdown(void)
{
}
#endif
#endif
