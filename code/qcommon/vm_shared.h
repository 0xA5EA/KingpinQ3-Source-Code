/*
 * vm_shared.h
 *
 *  Created on: 26.09.2010
 *      Author: area
 */

#ifndef VM_SHARED_H_
#define VM_SHARED_H_
/*
==============================================================
VIRTUAL MACHINE
==============================================================
*/
#ifdef Q3_VM
# include "../game/bg_lib.h"
//typedef int intptr_t;
#else
#if defined (_WIN32)
//#include <crtdefs.h> //FIXME(0xA5EA): compilation error with mingw32
#else
#include <stdint.h>
#endif
#endif
#include "q_math.h"

//#ifdef __cplusplus
//extern "C" {
//#endif
typedef struct vm_s vm_t;

typedef enum
{
  VMI_NATIVE,
  VMI_BYTECODE,
  VMI_COMPILED
} vmInterpret_t;

typedef enum
{
  TRAP_MEMSET = 100, //101
  TRAP_MEMCPY,
  TRAP_STRNCPY,
  TRAP_SIN,
  TRAP_COS,
  TRAP_ATAN2,
  TRAP_SQRT,
  TRAP_MATRIXMULTIPLY,
  TRAP_ANGLEVECTORS,
  TRAP_PERPENDICULARVECTOR,
  TRAP_FLOOR,
  TRAP_CEIL, //112

  TRAP_TESTPRINTINT,
  TRAP_TESTPRINTFLOAT
} sharedTraps_t;


void VM_Init(void);
vm_t	*VM_Create( const char *module, intptr_t (*systemCalls)(intptr_t *),
				   vmInterpret_t interpret );
// module should be bare: "cgame", not "cgame.dll" or "vm/cgame.qvm"

void VM_Free(vm_t *vm);
void VM_Clear(void);
void VM_Forced_Unload_Start(void);
void VM_Forced_Unload_Done(void);
vm_t	*VM_Restart(vm_t *vm, qboolean unpure);

intptr_t QDECL VM_Call(vm_t *vm, int callNum, ...);

void VM_Debug(int level);

void *VM_ArgPtr(intptr_t intValue);
void *VM_ExplicitArgPtr(vm_t *vm, intptr_t intValue);

#define VMA(x) VM_ArgPtr(args[x])
static ID_INLINE float _vmf(intptr_t x)
{
  floatint_t fi;

  fi.i = (int)x;
  return fi.f;
}
#define VMF(x) _vmf(args[x])
//#ifdef __cplusplus
//}  // end of extern "C"
//#endif
#endif /* VM_SHARED_H_ */
