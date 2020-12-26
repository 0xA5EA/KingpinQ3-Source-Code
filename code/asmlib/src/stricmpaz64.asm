;*************************  stricmpaz64.asm  **********************************
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


public stricmp_az                      ; Function stricmp_az

; ***************************************************************************
; Define registers used for function parameters, used in 64-bit mode only
; ***************************************************************************
 
IFDEF WINDOWS
  par1   equ  rcx                      ; first parameter
  par2   equ  rdx                      ; second parameter
ENDIF
  
IFDEF UNIX
  par1   equ  rdi                      ; first parameter
  par2   equ  rsi                      ; second parameter
ENDIF

.code

; extern "C" int stricmp_az(const char *string1, const char *string2);

stricmp_az PROC
        sub     par2, par1
        
L10:    mov     al,  [par1]            ; string1
        cmp     al,  [par1+par2]       ; string2
        jne     L20
        inc     par1
        test    al, al
        jnz     L10                    ; continue with next byte
        
        ; terminating zero found. Strings are equal
        xor     eax, eax
        ret        
        
L20:    ; bytes are different. check case
        xor     al, 20H                ; toggle case
        cmp     al, [par1+par2]
        jne     L30
        ; possibly differing only by case. Check if a-z
        or      al, 20H                ; upper case
        sub     al, 'a'
        cmp     al, 'z'-'a'
        ja      L30                    ; not a-z
        ; a-z and differing only by case
        inc     par1
        jmp     L10                    ; continue with next byte

L30:    ; bytes are different, even after changing case
        mov     al,  [par1]            ; get original value again
        sub     al, 'A'
        cmp     al, 'Z' - 'A'
        ja      L40
        add     al, 20H                ; A-Z, make lower case
L40:    mov     dl, [par1+par2]
        sub     dl, 'A'
        cmp     dl, 'Z' - 'A'
        ja      L50
        add     dl, 20H                ; A-Z, make lower case
L50:    add     dl, 'A'                ; undo subtraction above
        add     al, 'A'                ; undo subtraction above
        movzx   edx, dl                ; zero-extend
        movzx   eax, al
        sub     eax, edx               ; subtract to get result
        ret

stricmp_az ENDP


END
