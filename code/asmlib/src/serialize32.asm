;                   serialize32.asm
;
; Author:           Agner Fog
; Date created:     2007-06-15
; Last modified:    2008-10-16
; Description:
;
; This function serializes execution.
; Use this before and after calls to __readpmc() to prevent out-of-order execution.
; 
; Copyright (c) 2009 GNU General Public License www.gnu.org/licenses
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

GLOBAL _Serialize: function

SECTION .text  align=16

; ********** Serialize function **********
; C++ prototype:
; extern "C" void Serialize();

_Serialize:
        push    ebx                    ; ebx is modified by cpuid
        sub     eax, eax
        cpuid                          ; cpuid function 0
        pop     ebx
        ret
;_Serialize ENDP
