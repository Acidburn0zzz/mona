%define KERNEL_SEG 0x0100

;-------------------------------------------------------------------------------
; entry point
;-------------------------------------------------------------------------------

SecondBootMain:
        call    ILMain

        mov     ax, cs
        mov     ds, ax
        mov     si, RealToProtect
        mov     ax, KERNEL_SEG
        mov     es, ax
        xor     di, di
        mov     cx, gdt_end - RealToProtect
        rep     movsb

        jmp     KERNEL_SEG:0

;-------------------------------------------------------------------------------
; To Protect mode
;-------------------------------------------------------------------------------
RealToProtect:
;;; Real to Protect
        mov     ax, cs          ; we jump from firstboot
        mov     ds, ax          ; so ds is changed
        lgdt    [gdtr - RealToProtect]  ; load gdtr
        cli                     ; disable interrupt
        mov     eax, cr0        ; real
        or      eax, 1          ; to
        mov     cr0, eax        ; protect
        jmp     flush_q1
flush_q1:
	jmp     0x0008:(KERNEL_SEG * 16 + set_cs_desc1) - RealToProtect

[bits 32]
set_cs_desc1:
        mov     ax, 0x10        ; ds & es
        mov     ds, ax          ; selector is
        mov     es, ax          ; 0x10
        mov     ax, 0x18        ; ss selector
        mov     ss, ax          ; is 0x18
        mov     esp, 0x80000    ; sp is 3MB
        push    eax
        jmp     0x0200 + RealToProtect

;-------------------------------------------------------------------------------
; GDT definition: It is temporary.
;-------------------------------------------------------------------------------
gdtr:
        dw gdt_end - gdt0 - 1   ; gdt limit
        dd (KERNEL_SEG * 16 + gdt0) - RealToProtect  ; start adderess

gdt0:                           ; segment 00
        dw 0                    ; segment limitL
        dw 0                    ; segment baseL
        db 0                    ; segment baseM
        db 0                    ; segment type
        db 0                    ; segment limitH
        db 0                    ; segment baseH

gdt08:                          ; segment 08(code segment)
        dw 0xffff               ; segment limitL
        dw 0                    ; segment baseL
        db 0                    ; segment baseM
        db 0x9a                 ; Type Code
        db 0xff                 ; segment limitH
        db 0                    ; segment baseH

gdt10:                          ; segment 10(data segment)
        dw 0xffff               ; segment limitL
        dw 0                    ; segment baseL
        db 0                    ; segment baseM
        db 0x92                 ; Type Data
        db 0xff                 ; segment limitH
        db 0                    ; segment baseH

gdt18:                          ; segment 18(stack segment)
        dw 0                    ; segment limitL
        dw 0                    ; segment baseL
        db 0                    ; segment baseM
        db 0x96                 ; Type Stack
        db 0xC0                 ; segment limitH
        db 0                    ; segment baseH

gdt_end:                        ; end of gdt
