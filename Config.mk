BUILD_GAME_QVM=0
BUILD_GAME_SO=0
BUILD_SERVER=1
BUILD_CLIENT=1
BUILD_CLIENT_SMP=0
BUILD_STANDALONE=1
BUILD_KMAP2=0
BUILD_KAAS=0

USE_LOCAL_HEADERS=0
USE_CURL=1
USE_CODEC_VORBIS=1
USE_INTERNAL_VORBIS=0

USE_FREETYPE=1
USE_FREETYPE_BUILDIN=0
USE_MUMBLE=0

USE_OPENAL=1

USE_INTERNAL_PNG=0

# this is experminental, leave it 0
USE_OPENMP=0

# set to 1 if sse calculations should be enabled
USE_SSE=1

# if 1 cgame, game and ui dll compiles c-language, otherwise c++
USE_C_DLL=0

USE_CIN_THEORA=1
USE_CIN_THEORA_BUILDIN=1

# for kaas built deactivate this, for cgame game and ui dll as well
USE_ASM_LIB=1

#libfreetype is linked with zlib so USE_INTERNAL_ZLIB only makes sense if USE_FREETYPE_BUILDIN is enabled as well
USE_INTERNAL_ZLIB=0

USE_INTERNAL_GLEW=1

USE_UNSAFE_OPTIMIZATIONS=0
USE_CPP_HALFFLOAT=0

USE_GPROF_PROFILING = 0

# use experimental c++ 0x standard
USE_CPLUS_0X = 0

USE_INTERNAL_SPEEX=0
USE_VOIP=0

#USE_TCMALLOC=1
#  -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
# verursacht compilefehler in  /cl_cgame.c wenn 0

#  DEFAULT_BASEDIR   - extra path to search for baseq3 and such
#  USE_OPENAL        - use OpenAL where available
#  USE_OPENAL_DLOPEN - link with OpenAL at runtime
#  USE_CURL          - use libcurl for http/ftp download support
#  USE_CURL_DLOPEN   - link with libcurl at runtime
#  USE_CODEC_VORBIS  - enable Ogg Vorbis support
#  USE_LOCAL_HEADERS - use headers local to ioq3 instead of system ones
#  COPYDIR           - the target installation directory
