; ----------------------------- ALIBD64.ASM ---------------------------
; DLL entry function for ALIBD64.DLL

default rel

global DllEntry: function

SECTION .text  align=16

DllEntry:
        mov     eax, 1
        ret
;DllMain endp
