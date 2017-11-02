; -----------------------------------------------------------------------------
; ZX7 Backwards by Einar Saukas, Antonio Villena
; "Standard" version (156/177/191 bytes only)
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------
    define speed 2

    macro getbitm
      if speed=0
        add     a, a
        call    z, getbit
      else
        add     a,  a
        jp      nz, .gb1
        ld      a, (hl)
        dec     hl
        adc     a, a
.gb1
      endif
    endm

dzx7:   ld      a, $80
      if speed>1
        ldd
        add     a, a
        jr      z, mailab
        jr      c, maicoo
        ldd
        add     a, a
        jr      c, maicoe
      endif
copbye: ldd
        add     a, a
        jr      z, mailab
        jr      c, maicoo
copbyo: ldd
        add     a, a
        jr      nc, copbye

maicoe: ld      bc, 2
maisie: push    de
        ld      d, b
        getbitm
      if speed=0
        jr      c, contie
      else
        jp      c, contie
      endif
        dec     c
levale: add     a, a
        rl      c
        rl      b
        getbitm
        jr      nc, levale
        inc     c
        jr      z, exitdz
contie: ld      e, (hl)
        dec     hl
        sll     e
        jr      nc, offnde
        add     a, a
        rl      d
        getbitm
        rl      d
        add     a, a
        rl      d
        getbitm
        ccf
        jr      c, offnde
        inc     d
offnde: rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
        pop     hl
        add     a, a
    if speed=0
        jr      c, maicoe
        jr      copbye
    else
      if speed=1
        jr      c, maicoe
        jp      copbye
      else
        jr      nc, copbye
        ld      c, 2
        jp      maisie
      endif
    endif

exitdz: pop     hl
      if speed=0
getbit: ld      a, (hl)
        dec     hl
        adc     a, a
      endif
        ret

mailab: ld      a, (hl)
        dec     hl
        adc     a, a
        jr      nc, copbyo

maicoo: ld      bc, 2
        push    de
        ld      d, b
        add     a, a
      if speed=0
        jr      c, contio
      else
        jp      c, contio
      endif
        dec     c
levalo: getbitm
        rl      c
        rl      b
        add     a, a
        jr      nc, levalo
        inc     c
        jr      z, exitdz
contio: ld      e, (hl)
        dec     hl
        sll     e
        jr      nc, offndo
        getbitm
        rl      d
        add     a, a
        rl      d
        getbitm
        rl      d
        add     a, a
        ccf
        jr      c, offndo
        inc     d
offndo: rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
        pop     hl
        add     a,  a
        jr      z, mailab
        jr      c, maicoo
        jp      copbyo
