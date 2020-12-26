; ----------------------------- ALIBD.ASM ---------------------------
; DLL entry function for ALIBD32.DLL


SECTION .text  align=16

GLOBAL _DllEntry@12: function

_DllEntry@12:       ; proc hInstance:DWORD, reason:DWORD, reserved1:DWORD
        mov     eax, 1
        ret     12
;_DllEntry@12 endp

; END  _DllEntry@12
