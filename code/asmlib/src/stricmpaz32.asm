;*************************  stricmpaz32.asm  **********************************
; Author:           Agner Fog
; Date created:     2008-12-05
; Last modified:    2008-12-05
; Source URL:       www.agner.org/optimize
; Project:          asmlib.zip
; Description:
; Faster version of the standard stricmp or strcasecmp function:
; int stricmp_az(const char *string1, const char *string2);
; Compares two zero-terminated strings without case sensitivity.
; Does not recognize locale-specific characters. A-Z are changed
; to a-z before comparing, while other upper-case letters are not
; converted but considered unique.
;
; Optimization:
; Does not use xmm registers because this was found to be suboptimal,
; especially for short strings.
; The SSE4.2 instruction set is not available yet. I can't tell if it will be faster.
;
; Copyright (c) 2008 GNU General Public License www.gnu.org/licenses/gpl.html
;******************************************************************************
;
;
.686
.xmm
.model flat

public _stricmp_az                     ; Function _stricmp_az


.code

; extern "C" int stricmp_az(const char *string1, const char *string2);

_stricmp_az PROC NEAR
        mov     ecx, [esp+4]           ; string1
        mov     edx, [esp+8]           ; string2
        sub     edx, ecx
        
L10:    mov     al,  [ecx]
        cmp     al,  [ecx+edx]
        jne     L20
        inc     ecx
        test    al, al
        jnz     L10                    ; continue with next byte
        
        ; terminating zero found. Strings are equal
        xor     eax, eax
        ret        
        
L20:    ; bytes are different. check case
        xor     al, 20H                ; toggle case
        cmp     al, [ecx+edx]
        jne     L30
        ; possibly differing only by case. Check if a-z
        or      al, 20H                ; upper case
        sub     al, 'a'
        cmp     al, 'z'-'a'
        ja      L30                    ; not a-z
        ; a-z and differing only by case
        inc     ecx
        jmp     L10                    ; continue with next byte

L30:    ; bytes are different, even after changing case
        mov     al,  [ecx]             ; get original value again
        sub     al, 'A'
        cmp     al, 'Z' - 'A'
        ja      L40
        add     al, 20H
L40:    mov     dl, [ecx+edx]
        sub     dl, 'A'
        cmp     dl, 'Z' - 'A'
        ja      L50
        add     dl, 20H
L50:    add     dl, 'A'                ; undo subtraction above
        add     al, 'A'                ; undo subtraction above
        movzx   edx, dl                ; zero-extend
        movzx   eax, al
        sub     eax, edx               ; subtract to get result
        ret

_stricmp_az ENDP


END
