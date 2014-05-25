;
;    Copyright (C) 2011  Kevin Timmerman
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.
;       
;--------------------------------------------------------------------------------
;    rick@kimballsoftware.com
;    9/15/2011 - tweaked so it would compile with msp430-gcc
;    9/28/2011 - directives,variables,registers, and comments cleanup
;
#include "msp430g2452.h"
        

        .file "serial.s"
        .cpu 430
        .mpy none

;--------------------------------------------------------------------------------
; -- BSS section -- local uninitialized data variables
        .lcomm pinMask,2        ; use passed BIT. (Note: TX pin on Launchpad is wired to P1.1)
        .lcomm clksPerBit,2     ; number of clock cycles between each bit
;--------------------------------------------------------------------------------

#define PORTOUT P1OUT           ; define which port is used for xmit

;--------------------------------------------------------------------------------
; -- Code Section --
;--------------------------------------------------------------------------------
        .text
        
        .p2align 1,0            ; align on a word boundary
        .global serial_setup    ; make this function visible
        .type serial_setup, @function ;
;--------------------------------------------------------------------------------
; void serial_setup(unsigned pinMask, unsigned clksPerBit) 
;      pinMask - which pin on Port1 is used in r15
;      clksPerBit - duration in clock cycles between bits in r14
;
serial_setup:
        mov     r15, &pinMask   ; Save which pin is used for xmit
        sub     #16, r14        ; Adjust count for loop overhead
        rla     r14             ; 4 cycle loop decrements by 8 to ensure even remainder
        mov     r14,&clksPerBit ; Save bit duration initial count
        ret                     ; Return
.Lfe1:
        .size  serial_setup,.Lfe1-serial_setup
; End of function 

        .p2align 1,0
        .global putc
        .type putc, @function
;--------------------------------------------------------------------------------
; void putc(int c)
;    c - character to tx in r15
;
;    r15, r14, r13, r12 trashed 
;
putc:                           ; Char to tx in r15
        mov     &pinMask, r13   ; Serial output bitmask
        mov     &clksPerBit,r14 ; Bit duration
        bis     #0x0300, r15    ; Add Stop bit(s)
        jmp     bit_low         ; Send start bit
bit_loop:
        mov     r14, r12        ; Get bit duration
bit_time:
        nop                     ; 4 cycle loop
        sub     #8, r12
        jc      bit_time        ;
        subc    r12, r0         ; 0 to 3 cycle delay manipulate PC(aka R0)
        nop                     ; 3
        nop                     ; 2
        nop                     ; 1
        rra     r15             ; Get bit to tx, test for zero
        jc      bit_high        ; If high...
bit_low:
        bic.b   r13, &PORTOUT   ; Send zero bit (P1OUT=&0x0021)
        jmp     bit_loop        ; Next bit...
bit_high:
        bis.b   r13, &PORTOUT   ; Send one bit
        jnz     bit_loop        ; If tx data is not zero, then there are more bits to send..
putc_exit:
        ret                     ; Return when all bits sent            

.Lfe2:
        .size   putc,.Lfe2-putc
; End of function 

        .p2align 1,0
        .global puts
        .type puts, @function
;--------------------------------------------------------------------------------
; void puts(char *s)
;    s - null terminated string to xmit in r15
;
puts:
        push    r11             ; r12-r15 are used by putc, can't use those,
                                ; so to use r11 we need to save it and restore it
        mov     r15,r11         ; string pointer passed in R15, copy it to R11

puts_loop:
        mov.b   @r11+, r15      ; dereference string, get character and auto increment
        tst.b   r15             ; check for null byte
        jz      puts_exit       ; if so we are done
        call    #putc           ; otherwise xmit
        jmp     puts_loop       ; do it till done

puts_exit:      
        pop     r11             ; restore trashed register
        ret
.Lfe3:
        .size        puts,.Lfe3-puts
; End of function 
