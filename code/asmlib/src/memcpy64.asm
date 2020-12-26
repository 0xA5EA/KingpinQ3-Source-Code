;*************************  memcpy64.asm  ************************************
; Author:           Agner Fog
; Date created:     2008-07-19
; Last modified:    2011-07-11
;
; Description:
; Faster version of the standard memcpy function:
; void * A_memcpy(void *dest, const void *src, size_t count);
; Copies 'count' bytes from 'src' to 'dest'
;
; Overriding standard function memcpy:
; The alias ?OVR_memcpy is changed to _memcpy in the object file if
; it is desired to override the standard library function memcpy.
;
; The function uses non-temporal writes to bypass the cache when the size is 
; bigger than half the size of the largest_level cache. This limit can be
; read with _GetMemcpyCacheLimit and changed with _SetMemcpyCacheLimit (in 
; memmove64.asm). C++ prototypes:
; extern "C" size_t GetMemcpyCacheLimit();  // in memcpy64.asm
; extern "C" void SetMemcpyCacheLimit();    // in memmove64.asm
; extern "C" void SetMemcpyCacheLimit1();   // used internally
;
; Position-independent code is generated if POSITIONINDEPENDENT is defined.
;
; CPU dispatching included for SSE2 and Suppl-SSE3 instruction sets.
;
; To do: Future extensions:
; Check if AMD XOP instruction PPERM is faster
; Make AVX version using unaligned move with YMM registers when a CPU appears
; where this is faster. See discussion in my manual "Optimizing software in C++".
;
; Copyright (c) 2008-2011 GNU General Public License www.gnu.org/licenses
;******************************************************************************

default rel

global A_memcpy: function              ; Function A_memcpy
global ?OVR_memcpy: function           ; ?OVR removed if standard function memcpy overridden
global memcpySSE2: function            ; Version for processors with only SSE2
global memcpySSSE3: function           ; Version for processors with SSSE3
global memcpyU: function               ; Alternative version for processors with fast unaligned read

global GetMemcpyCacheLimit: function   ; Get the size limit for bypassing cache when copying with memcpy and memmove
global SetMemcpyCacheLimit1: function  ; Set the size limit for bypassing cache when copying with memcpy

; Imported from instrset64.asm
extern InstructionSet                  ; Instruction set for CPU dispatcher

; Imported from unalignedisfaster64.asm:
extern UnalignedIsFaster               ; Tells if unaligned read is faster than PALIGNR

; Imported from cachesize32.asm:
extern DataCacheSize                   ; Gets size of data cache


; Define prolog for this function
%MACRO  PROLOGM  0
%IFDEF  WINDOWS
        push    rsi
        push    rdi
        mov     rdi, rcx               ; dest
        mov     r9,  rcx               ; dest
        mov     rsi, rdx               ; src
        mov     rcx, r8                ; count
%ELSE   ; Unix
        mov     rcx, rdx               ; count
        mov     r9,  rdi               ; dest
%ENDIF
%ENDM

; Define return from this function
%MACRO  RETURNM  0
%IFDEF  WINDOWS
        pop     rdi
        pop     rsi
%ENDIF
        mov     rax, r9                ; Return value = dest
        ret
%ENDM


SECTION .text  align=16

; extern "C" void * A_memcpy(void * dest, const void * src, size_t count);
; Function entry:
A_memcpy:
?OVR_memcpy:
        jmp     qword [memcpyDispatch] ; Go to appropriate version, depending on instruction set

memcpySSE2:     ; SSE2 version begins here
        PROLOGM
        cmp     rcx, 40H
        jae     B0100                   ; Use simpler code if count < 64
        
        ; count < 64. Move 32-16-8-4-2-1 bytes
        add     rsi, rcx               ; end of src
        add     rdi, rcx               ; end of dest
        neg     rcx                    ; negative index from the end
        cmp     ecx, -20H
        jg      A100        
        ; move 32 bytes
        ; mov r64 is faster than movdqu on Intel Pentium M and Core 1
        ; movdqu is fast on Nehalem and later
        mov     rax, [rsi+rcx]
        mov     rdx, [rsi+rcx+8]
        mov     [rdi+rcx], rax
        mov     [rdi+rcx+8], rdx
        mov     rax, qword [rsi+rcx+10H]
        mov     rdx, qword [rsi+rcx+18H]
        mov     qword [rdi+rcx+10H], rax
        mov     qword [rdi+rcx+18H], rdx
        add     rcx, 20H
A100:   cmp     ecx, -10H        
        jg      A200
        ; move 16 bytes
        mov     rax, [rsi+rcx]
        mov     rdx, [rsi+rcx+8]
        mov     [rdi+rcx], rax
        mov     [rdi+rcx+8], rdx
        add     rcx, 10H
A200:   cmp     ecx, -8        
        jg      A300
        ; move 8 bytes
        mov     rax, qword [rsi+rcx]
        mov     qword [rdi+rcx], rax
        add     rcx, 8
A300:   cmp     ecx, -4        
        jg      A400
        ; move 4 bytes
        mov     eax, [rsi+rcx]
        mov     [rdi+rcx], eax
        add     rcx, 4
        jz      A900                     ; early out if count divisible by 4
A400:   cmp     ecx, -2        
        jg      A500
        ; move 2 bytes
        movzx   eax, word [rsi+rcx]
        mov     [rdi+rcx], ax
        add     rcx, 2
A500:   cmp     ecx, -1
        jg      A900        
        ; move 1 byte
        movzx   eax, byte [rsi+rcx]
        mov     [rdi+rcx], al
A900:   ; finished
        RETURNM        
        
B0100:  ; count >= 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      B0200                    ; Skip if dest aligned by 16
        
        ; edx = size of first partial block, 1 - 15 bytes
        test    edx, 3
        jz      B0030
        test    edx, 1
        jz      B0020
        ; move 1 byte
        movzx   eax, byte [rsi]
        mov     [rdi], al
        inc     rsi
        inc     rdi
B0020:  test    edx, 2
        jz      B0030
        ; move 2 bytes
        movzx   eax, word [rsi]
        mov     [rdi], ax
        add     rsi, 2
        add     rdi, 2
B0030:  test    edx, 4
        jz      B0040
        ; move 4 bytes
        mov     eax, [rsi]
        mov     [rdi], eax
        add     rsi, 4
        add     rdi, 4
B0040:  test    edx, 8
        jz      B0050
        ; move 8 bytes
        mov     rax, [rsi]
        mov     [rdi], rax
        add     rsi, 8
        add     rdi, 8
B0050:  sub     rcx, rdx
B0200:  ; Now dest is aligned by 16. Any partial block has been moved        

        ; This part will not always work if count < 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      B300                    ; Skip if dest aligned by 16
        
        ; rdx = size of first partial block, 1 - 15 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        cmp     edx, -8
        jg      B200
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
B200:   cmp     edx, -4        
        jg      B210
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
        jz      B300              ; early out if aligned by 4
B210:   cmp     edx, -2        
        jg      B220
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
B220:   cmp     edx, -1
        jg      B300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
B300:   ; Now dest is aligned by 16. Any partial block has been moved        
        ; Find alignment of src modulo 16 at this point:
        mov     eax, esi
        and     eax, 0FH
        
        ; Set up for loop moving 32 bytes per iteration:
        mov     edx, ecx               ; Save count (lower 32 bits)
        and     rcx, -20H              ; Round down count to nearest multiple of 32
        add     rsi, rcx               ; Point to the end
        add     rdi, rcx               ; Point to the end
        sub     edx, ecx               ; Remaining data after loop (0-31)
        sub     rsi, rax               ; Nearest preceding aligned block of src

        ; Check if count very big
        cmp     rcx, [CacheBypassLimit]
        ja      B400                   ; Use non-temporal store if count > CacheBypassLimit
        neg     rcx                    ; Negative index from the end
        
        ; Dispatch to different codes depending on src alignment
        lea     r8, [AlignmentDispatchSSE2]
        jmp     near [r8+rax*8]

B400:   neg     rcx
        ; Dispatch to different codes depending on src alignment
        lea     r8, [AlignmentDispatchNT]
        jmp     near [r8+rax*8]
        
        
align 16
memcpySSSE3:     ; SSSE3 version begins here        
        PROLOGM
        cmp     rcx, 40H
        jae     B1100                   ; Use simpler code if count < 64
        
        ; count < 64. Move 32-16-8-4-2-1 bytes
A1000:  add     rsi, rcx               ; end of src
        add     rdi, rcx               ; end of dest
        neg     rcx                    ; negative index from the end
        cmp     ecx, -20H
        jg      A1100        
        ; move 32 bytes
        ; movdqu is faster than 64-bit moves on processors with SSSE3
        movups  xmm0, [rsi+rcx]
        movups  xmm1, [rsi+rcx+10H]
        movups  [rdi+rcx], xmm0
        movups  [rdi+rcx+10H], xmm1
        add     rcx, 20H
A1100:  cmp     ecx, -10H        
        jg      A1200
        ; move 16 bytes
        movups  xmm0, [rsi+rcx]
        movups  [rdi+rcx], xmm0
        add     rcx, 10H
A1200:  cmp     ecx, -8        
        jg      A1300
        ; move 8 bytes
        mov     rax, qword [rsi+rcx]
        mov     qword [rdi+rcx], rax
        add     rcx, 8
A1300:  cmp     ecx, -4        
        jg      A1400
        ; move 4 bytes
        mov     eax, [rsi+rcx]
        mov     [rdi+rcx], eax
        add     rcx, 4
        jz      A1900                     ; early out if count divisible by 4
A1400:  cmp     ecx, -2        
        jg      A1500
        ; move 2 bytes
        movzx   eax, word [rsi+rcx]
        mov     [rdi+rcx], ax
        add     rcx, 2
A1500:  cmp     ecx, -1
        jg      A1900        
        ; move 1 byte
        movzx   eax, byte [rsi+rcx]
        mov     [rdi+rcx], al
A1900:  ; finished
        RETURNM        
        
B1100:  ; count >= 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      B1200                    ; Skip if dest aligned by 16
        
        ; edx = size of first partial block, 1 - 15 bytes
        test    edx, 3
        jz      B1030
        test    edx, 1
        jz      B1020
        ; move 1 byte
        movzx   eax, byte [rsi]
        mov     [rdi], al
        inc     rsi
        inc     rdi
B1020:  test    edx, 2
        jz      B1030
        ; move 2 bytes
        movzx   eax, word [rsi]
        mov     [rdi], ax
        add     rsi, 2
        add     rdi, 2
B1030:  test    edx, 4
        jz      B1040
        ; move 4 bytes
        mov     eax, [rsi]
        mov     [rdi], eax
        add     rsi, 4
        add     rdi, 4
B1040:  test    edx, 8
        jz      B1050
        ; move 8 bytes
        mov     rax, [rsi]
        mov     [rdi], rax
        add     rsi, 8
        add     rdi, 8
B1050:  sub     rcx, rdx
B1200:  ; Now dest is aligned by 16. Any partial block has been moved        
        ; Find alignment of src modulo 16 at this point:
        mov     eax, esi
        and     eax, 0FH
        
        ; Set up for loop moving 32 bytes per iteration:
        mov     edx, ecx               ; Save count (lower 32 bits)
        and     rcx, -20H              ; Round down count to nearest multiple of 32
        add     rsi, rcx               ; Point to the end
        add     rdi, rcx               ; Point to the end
        sub     edx, ecx               ; Remaining data after loop (0-31)
        sub     rsi, rax               ; Nearest preceding aligned block of src

        ; Check if count very big
        cmp     rcx, [CacheBypassLimit]
        ja      B1400                   ; Use non-temporal store if count > CacheBypassLimit
        neg     rcx                    ; Negative index from the end
        
        ; Dispatch to different codes depending on src alignment
        lea     r8, [AlignmentDispatchSSSE3]
        jmp     near [r8+rax*8]

B1400:  neg     rcx
        ; Dispatch to different codes depending on src alignment
        lea     r8, [AlignmentDispatchNT]
        jmp     near [r8+rax*8]


align   16
C100:   ; Code for aligned src. SSE2 and SSSE3 versions
        ; The nice case, src and dest have same alignment.

        ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm0, [rsi+rcx]
        movaps  xmm1, [rsi+rcx+10H]
        movaps  [rdi+rcx], xmm0
        movaps  [rdi+rcx+10H], xmm1
        add     rcx, 20H
        jnz     C100
        
        ; Move the remaining edx bytes (0 - 31):
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        jz      C500                   ; Skip if no more data
        ; move 16-8-4-2-1 bytes, aligned
        cmp     edx, -10H
        jg      C200
        ; move 16 bytes
        movaps  xmm0, [rsi+rdx]
        movaps  [rdi+rdx], xmm0
        add     rdx, 10H
C200:   cmp     edx, -8
        jg      C210        
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8 
        jz      C500                   ; Early skip if count divisible by 8       
C210:   cmp     edx, -4
        jg      C220        
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4        
C220:   cmp     edx, -2
        jg      C230        
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
C230:   cmp     edx, -1
        jg      C500        
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
C500:   ; finished     
        RETURNM
        

align 16
memcpyU:   ; Alternative version for processors with fast unaligned read
        PROLOGM
        cmp     rcx, 40H
        jb      A1000                  ; Use simpler code if count < 64

        ; count >= 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      B2100                    ; Skip if dest aligned by 16
        
        ; edx = size of first partial block, 1 - 15 bytes
        test    edx, 3
        jz      B2030
        test    edx, 1
        jz      B2020
        ; move 1 byte
        movzx   eax, byte [rsi]
        mov     [rdi], al
        inc     rsi
        inc     rdi
B2020:  test    edx, 2
        jz      B2030
        ; move 2 bytes
        movzx   eax, word [rsi]
        mov     [rdi], ax
        add     rsi, 2
        add     rdi, 2
B2030:  test    edx, 4
        jz      B2040
        ; move 4 bytes
        mov     eax, [rsi]
        mov     [rdi], eax
        add     rsi, 4
        add     rdi, 4
B2040:  test    edx, 8
        jz      B2050
        ; move 8 bytes
        mov     rax, [rsi]
        mov     [rdi], rax
        add     rsi, 8
        add     rdi, 8
B2050:  sub     rcx, rdx
B2100:  ; Now dest is aligned by 16. Any partial block has been moved        
        
        ; Set up for loop moving 32 bytes per iteration:
        mov     rdx, rcx               ; Save count
        and     rcx, -20H              ; Round down to nearest multiple of 32
        add     rsi, rcx               ; Point to the end
        add     rdi, rcx               ; Point to the end
        sub     rdx, rcx               ; Remaining data after loop
        neg     rcx
        test    esi, 0Fh
        jz      C100                   ; Jump if src is aligned

H100:   ; copy -rcx bytes in blocks of 32 bytes.        
        ; Check for false memory dependence: The CPU may falsely assume
        ; a partial overlap between the written destination and the following
        ; read source if bit 7-11 of (src-dest) are all 1s
        mov     eax, esi
        sub     eax, edi
        and     eax, 0111110000000B
        cmp     eax, 0111110000000B
        je      J100                   ; fix false memory dependence

H110:   ; main copy loop, 32 bytes at a time
        ; rcx has negative index from the end, counting up to zero
        movups  xmm0, [rsi+rcx]
        movups  xmm1, [rsi+rcx+10H]
        movaps  [rdi+rcx], xmm0
        movaps  [rdi+rcx+10H], xmm1
        add     rcx, 20H
        jnz     H110
        
H120:   ; Move the remaining edx bytes (0 - 31):
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        jz      H500                   ; Skip if no more data
        ; move 16-8-4-2-1 bytes, aligned
        cmp     edx, -10H
        jg      H200
        ; move 16 bytes
        movups  xmm0, [rsi+rdx]
        movaps  [rdi+rdx], xmm0
        add     rdx, 10H
H200:   cmp     edx, -8
        jg      H210        
        ; move 8 bytes
        movq    xmm0, qword [rsi+rdx]
        movq    qword [rdi+rdx], xmm0
        add     rdx, 8 
        jz      H500                   ; Early skip if count divisible by 8       
H210:   cmp     edx, -4
        jg      H220        
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4        
H220:   cmp     edx, -2
        jg      H230        
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
H230:   cmp     edx, -1
        jg      H500        
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
H500:   ; finished     
        RETURNM

align 16
J100:   ; There is a false memory dependence.
        ; check if src and dest overlap, if not then it is safe 
        ; to copy backwards to avoid false memory dependence
%if 1   
        ; Use this version if you want consistent behavior in the case
        ; where dest > src and overlap. However, this case is undefined
        ; anyway because part of src is overwritten before copying     
        push    rdx
        mov     rax, rsi
        sub     rax, rdi
        cqo
        xor     rax, rdx
        sub     rax, rdx   ; abs(src-dest)
        neg     rcx        ; size
        pop     rdx        ; restore rdx
        cmp     rax, rcx
        jnb     J110
        neg     rcx        ; restore rcx
        jmp     H110       ; overlap between src and dest. Can't copy backwards
%else
        ; save time by not checking the case that is undefined anyway         
        mov     rax, rsi
        sub     rax, rdi
        neg     rcx        ; size
        cmp     rax, rcx
        jnb     J110       ; OK to copy backwards
        ; must copy forwards
        neg     rcx        ; restore ecx
        jmp     H110       ; copy forwards

%endif
        
J110:   ; copy backwards, ecx = size. esi, edi = end of src, dest        
        push    rsi
        push    rdi
        sub     rsi, rcx
        sub     rdi, rcx
J120:   ; loop backwards
        movups  xmm1, [rsi+rcx-20H]
        movups  xmm0, [rsi+rcx-10H]
        movaps  [rdi+rcx-20H], xmm1
        movaps  [rdi+rcx-10H], xmm0
        sub     rcx, 20H
        jnz     J120
        pop     rdi
        pop     rsi
        jmp     H120
        
       
; Macros for each src alignment, SSE2 instruction set:
; Make separate code for each alignment u because the shift instructions
; have the shift count as a constant:

%MACRO  MOVE_UNALIGNED_SSE2  2 ; u, nt
; Move rcx + rdx bytes of data
; Source is misaligned. (src-dest) modulo 16 = %1
; %2 = 1 if non-temporal store desired
; eax = %1
; rsi = src - %1 = nearest preceding 16-bytes boundary
; rdi = dest (aligned)
; rcx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop
        movdqa  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:  ; Loop. rcx has negative index from the end, counting up to zero
        movdqa  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movdqa  xmm2, [rsi+rcx+20H]
        movdqa  xmm3, xmm1             ; Copy because used twice
        psrldq  xmm0, %1               ; shift right
        pslldq  xmm1, 16-%1            ; shift left
        por     xmm0, xmm1             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rcx], xmm0        ; non-temporal save
        %ENDIF
        movdqa  xmm0, xmm2             ; Save for next iteration
        psrldq  xmm3, %1               ; shift right
        pslldq  xmm2, 16-%1            ; shift left
        por     xmm3, xmm2             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rcx+10H], xmm3    ; Save aligned
        %ELSE
        movntdq [rdi+rcx+10H], xmm3    ; non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movdqa  xmm1, [rsi+rdx+10H]
        psrldq  xmm0, %1               ; shift right
        pslldq  xmm1, 16-%1            ; shift left
        por     xmm0, xmm1             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rdx], xmm0        ; non-temporal save
        %ENDIF        
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_4  1 ; nt
; Special case for u = 4
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movss   xmm0, xmm1             ; Moves 4 bytes, leaves remaining bytes unchanged
        shufps  xmm0, xmm0, 00111001B  ; Rotate
        %IF %1 == 0
        movaps  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntps [rdi+rcx], xmm0        ; Non-temporal save
        %ENDIF
        movaps  xmm0, [rsi+rcx+20H]
        movss   xmm1, xmm0
        shufps  xmm1, xmm1, 00111001B
        %IF %1 == 0
        movaps  [rdi+rcx+10H], xmm1    ; Save aligned
        %ELSE
        movntps [rdi+rcx+10H], xmm1    ; Non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        movss   xmm0, xmm1
        shufps  xmm0, xmm0, 00111001B
        %IF %1 == 0
        movaps  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntps [rdi+rdx], xmm0        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_8  1 ; nt
; Special case for u = 8
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movsd   xmm0, xmm1             ; Moves 8 bytes, leaves remaining bytes unchanged
        shufps  xmm0, xmm0, 01001110B  ; Rotate
        %IF %1 == 0
        movaps  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntps [rdi+rcx], xmm0        ; Non-temporal save
        %ENDIF
        movaps  xmm0, [rsi+rcx+20H]
        movsd   xmm1, xmm0
        shufps  xmm1, xmm1, 01001110B
        %IF %1 == 0
        movaps  [rdi+rcx+10H], xmm1    ; Save aligned
        %ELSE
        movntps [rdi+rcx+10H], xmm1    ; Non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        movsd   xmm0, xmm1
        shufps  xmm0, xmm0, 01001110B
        %IF %1 == 0
        movaps  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntps [rdi+rdx], xmm0        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_12  1 ; nt
; Special case for u = 12
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
        shufps  xmm0, xmm0, 10010011B
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movaps  xmm2, [rsi+rcx+20H]
        shufps  xmm1, xmm1, 10010011B
        shufps  xmm2, xmm2, 10010011B
        movaps  xmm3, xmm2
        movss   xmm2, xmm1             ; Moves 4 bytes, leaves remaining bytes unchanged
        movss   xmm1, xmm0             ; Moves 4 bytes, leaves remaining bytes unchanged       
        %IF %1 == 0
        movaps  [rdi+rcx], xmm1        ; Save aligned
        movaps  [rdi+rcx+10H], xmm2    ; Save aligned
        %ELSE
        movntps [rdi+rcx], xmm1        ; Non-temporal save
        movntps [rdi+rcx+10H], xmm2    ; Non-temporal save
        %ENDIF
        movaps  xmm0, xmm3             ; Save for next iteration        
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        shufps  xmm1, xmm1, 10010011B
        movss   xmm1, xmm0             ; Moves 4 bytes, leaves remaining bytes unchanged       
        %IF %1 == 0
        movaps  [rdi+rdx], xmm1        ; Save aligned
        %ELSE
        movntps [rdi+rdx], xmm1        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


; Macros for each src alignment, Suppl.SSE3 instruction set:
; Make separate code for each alignment u because the palignr instruction
; has the shift count as a constant:

%MACRO MOVE_UNALIGNED_SSSE3  1 ; u
; Move rcx + rdx bytes of data
; Source is misaligned. (src-dest) modulo 16 = %1
; eax = %1
; rsi = src - %1 = nearest preceding 16-bytes boundary
; rdi = dest (aligned)
; rcx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop
        movdqa  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
        
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movdqa  xmm2, [rsi+rcx+10H]    ; Read next two blocks
        movdqa  xmm3, [rsi+rcx+20H]
        movdqa  xmm1, xmm0             ; Save xmm0
        movdqa  xmm0, xmm3             ; Save for next iteration
        palignr xmm3, xmm2, %1         ; Combine parts into aligned block
        palignr xmm2, xmm1, %1         ; Combine parts into aligned block
        movdqa  [rdi+rcx], xmm2        ; Save aligned
        movdqa  [rdi+rcx+10H], xmm3    ; Save aligned
        add     rcx, 20H
        jnz     %%L1
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movdqa  xmm2, [rsi+rdx+10H]
        palignr xmm2, xmm0, %1
        movdqa  [rdi+rdx], xmm2
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes
        jmp     C200
%ENDMACRO


; Make 15 instances of SSE2 macro for each value of the alignment u.
; These are pointed to by the jump table AlignmentDispatchSSE2 below
; (alignments and fillers are inserted manually to minimize the number 
; of 16-bytes boundaries inside loops)

align 16
D104:   MOVE_UNALIGNED_SSE2_4    0
times 4 nop
D108:   MOVE_UNALIGNED_SSE2_8    0
times 4 nop
D10C:   MOVE_UNALIGNED_SSE2_12   0
times 1 nop
D101:   MOVE_UNALIGNED_SSE2 1,   0
D102:   MOVE_UNALIGNED_SSE2 2,   0
D103:   MOVE_UNALIGNED_SSE2 3,   0
D105:   MOVE_UNALIGNED_SSE2 5,   0
D106:   MOVE_UNALIGNED_SSE2 6,   0
D107:   MOVE_UNALIGNED_SSE2 7,   0
D109:   MOVE_UNALIGNED_SSE2 9,   0
times 1 nop
D10A:   MOVE_UNALIGNED_SSE2 0AH, 0
D10B:   MOVE_UNALIGNED_SSE2 0BH, 0
D10D:   MOVE_UNALIGNED_SSE2 0DH, 0
D10E:   MOVE_UNALIGNED_SSE2 0EH, 0
D10F:   MOVE_UNALIGNED_SSE2 0FH, 0
        
; Make 15 instances of Suppl-SSE3 macro for each value of the alignment u.
; These are pointed to by the jump table AlignmentDispatchSupSSE3 below

align   16
E104:   MOVE_UNALIGNED_SSSE3 4
E108:   MOVE_UNALIGNED_SSSE3 8
E10C:   MOVE_UNALIGNED_SSSE3 0CH
E101:   MOVE_UNALIGNED_SSSE3 1
E102:   MOVE_UNALIGNED_SSSE3 2
E103:   MOVE_UNALIGNED_SSSE3 3
E105:   MOVE_UNALIGNED_SSSE3 5
E106:   MOVE_UNALIGNED_SSSE3 6
E107:   MOVE_UNALIGNED_SSSE3 7
E109:   MOVE_UNALIGNED_SSSE3 9
times 1 nop
E10A:   MOVE_UNALIGNED_SSSE3 0AH
E10B:   MOVE_UNALIGNED_SSSE3 0BH
E10D:   MOVE_UNALIGNED_SSSE3 0DH
E10E:   MOVE_UNALIGNED_SSSE3 0EH
E10F:   MOVE_UNALIGNED_SSSE3 0FH

; Codes for non-temporal move. Aligned case first

align 16
F100:   ; Non-temporal move, src and dest have same alignment.
        ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm0, [rsi+rcx]        ; Read
        movaps  xmm1, [rsi+rcx+10H]
        movntps [rdi+rcx], xmm0        ; Write non-temporal (bypass cache)
        movntps [rdi+rcx+10H], xmm1
        add     rcx, 20H
        jnz     F100                   ; Loop through negative rcx up to zero
                
        ; Move the remaining edx bytes (0 - 31):
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        jz      C500                   ; Skip if no more data
        ; Check if we can more one more 16-bytes block
        cmp     edx, -10H
        jg      C200
        ; move 16 bytes, aligned
        movaps  xmm0, [rsi+rdx]
        movntps [rdi+rdx], xmm0
        add     rdx, 10H
        ; move the remaining 0 - 15 bytes
        jmp     C200

; Make 15 instances of MOVE_UNALIGNED_SSE2 macro for each value of 
; the alignment u.
; These are pointed to by the jump table AlignmentDispatchNT below

;align 16
F104:   MOVE_UNALIGNED_SSE2_4    1
F108:   MOVE_UNALIGNED_SSE2_8    1
F10C:   MOVE_UNALIGNED_SSE2_12   1
F101:   MOVE_UNALIGNED_SSE2 1,   1
F102:   MOVE_UNALIGNED_SSE2 2,   1
F103:   MOVE_UNALIGNED_SSE2 3,   1
F105:   MOVE_UNALIGNED_SSE2 5,   1
F106:   MOVE_UNALIGNED_SSE2 6,   1
F107:   MOVE_UNALIGNED_SSE2 7,   1
F109:   MOVE_UNALIGNED_SSE2 9,   1
F10A:   MOVE_UNALIGNED_SSE2 0AH, 1
F10B:   MOVE_UNALIGNED_SSE2 0BH, 1
F10D:   MOVE_UNALIGNED_SSE2 0DH, 1
F10E:   MOVE_UNALIGNED_SSE2 0EH, 1
F10F:   MOVE_UNALIGNED_SSE2 0FH, 1

        
memcpyCPUDispatch:   ; CPU dispatcher, check for Suppl-SSE3 instruction set
        ; This part is executed only once
        push    rbx
        push    rcx
        push    rdx
        push    rsi
        push    rdi
        push    r8        
        ; set CacheBypassLimit to half the size of the largest level cache
        call    GetMemcpyCacheLimit
        mov     eax, 1
        cpuid                          ; Get feature flags
        lea     rbx, [memcpySSE2]
        bt      ecx, 9                 ; Test bit for SupplSSE3
        jnc     Q100
        lea     rbx, [memcpySSSE3]
        call    UnalignedIsFaster
        test    eax, eax
        jz      Q100
        lea     rbx, [memcpyU]
Q100:   ; Insert appropriate pointer
        mov     [memcpyDispatch], rbx
        mov     rax, rbx
        pop     r8
        pop     rdi
        pop     rsi
        pop     rdx
        pop     rcx
        pop     rbx
        ; Jump according to the replaced function pointer
        jmp     rax
        
; extern "C" size_t GetMemcpyCacheLimit();
GetMemcpyCacheLimit:
        mov     rax, [CacheBypassLimit]
        test    rax, rax
        jnz     U200
        ; Get half the size of the largest level cache
%ifdef  WINDOWS
        xor     ecx, ecx               ; 0 means largest level cache
%else
        xor     edi, edi               ; 0 means largest level cache
%endif
        call    DataCacheSize          ; get cache size
        shr     rax, 1                 ; half the size
        jnz     U100
        mov     eax, 400000H           ; cannot determine cache size. use 4 Mbytes
U100:   mov     [CacheBypassLimit], rax
U200:   ret
        
; Note: SetMemcpyCacheLimit is defined in memmove64.asm, calling SetMemcpyCacheLimit1
SetMemcpyCacheLimit1:
%ifdef  WINDOWS
        mov     rax, rcx
%else
        mov     rax, rdi
%endif
        test    rax, rax
        jnz     U400
        ; zero, means default
        mov     [CacheBypassLimit], rax
        call    GetMemcpyCacheLimit
U400:   mov     [CacheBypassLimit], rax
        ret
        


; Data segment must be included in function namespace
SECTION .data  align=16

; Jump tables for alignments 0 - 15:
; The CPU dispatcher replaces AlignmentDispatch with 
; AlignmentDispatchSSE2 or AlignmentDispatchSupSSE3 if Suppl-SSE3 
; is supported.

; Code pointer for each alignment for SSE2 instruction set
AlignmentDispatchSSE2:
DQ C100, D101, D102, D103, D104, D105, D106, D107
DQ D108, D109, D10A, D10B, D10C, D10D, D10E, D10F

; Code pointer for each alignment for Suppl-SSE3 instruction set
AlignmentDispatchSSSE3:
DQ C100, E101, E102, E103, E104, E105, E106, E107
DQ E108, E109, E10A, E10B, E10C, E10D, E10E, E10F

; Code pointer for each alignment for non-temporal store
AlignmentDispatchNT:
DQ F100, F101, F102, F103, F104, F105, F106, F107
DQ F108, F109, F10A, F10B, F10C, F10D, F10E, F10F

; Pointer to appropriate version.
; This initially points to memcpyCPUDispatch. memcpyCPUDispatch will
; change this to the appropriate version of memcpy, so that
; memcpyCPUDispatch is only executed once:
memcpyDispatch: DQ memcpyCPUDispatch

; Bypass cache by using non-temporal moves if count > CacheBypassLimit
; The optimal value of _CacheBypassLimit is difficult to estimate, but
; a reasonable value is half the size of the largest cache:
CacheBypassLimit: DQ 0
