; -----------------------------------------------------------------------------
; ZX7 Backwards by Einar Saukas, Antonio Villena
; "Standard" version (64 bytes only)
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------
dzx7:   ld      bc, $8000
        ld      a, b
copyby: inc     c
        ldd
mainlo: call    getbit
        jr      nc, copyby
        push    de
        ld      d, c
lenval: call    nc, getbit
        rl      c
        rl      b
        call    getbit
        jr      nc, lenval
        inc     c
        jr      z, exitdz
        ld      e, (hl)
        dec     hl
        sll     e
        jr      nc, offend
        ld      d, $10
nexbit: call    getbit
        rl      d
        jr      nc, nexbit
        inc     d
        srl     d
offend: rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
exitdz: pop     hl
        jr      nc, mainlo
getbit: add     a, a
        ret     nz
        ld      a, (hl)
        dec     hl
        adc     a, a
        ret
