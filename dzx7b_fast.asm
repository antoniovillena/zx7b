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

dzx7    ld      a, $80
copybye ldd
mainloe add     a, a
        jr      z, lb1
        jr      c, maincoo
copybyo ldd
        add     a, a
      if speed>1
        jr      c, maincoe
        ldd
        add     a, a
        jr      z, lb1
        jr      c, maincoo
        ldd
        add     a, a
      endif
        jr      nc, copybye

maincoe ld      bc, 2
mainsie push    de
        ld      d, b
        getbitm
      if speed=0
        jr      c, contine
      else
        jp      c, contine
      endif
        dec     c
lenvale add     a, a
        rl      c
        rl      b
        getbitm
        jr      nc, lenvale
        inc     c
        jr      z, exitdz
contine ld      e, (hl)
        dec     hl
        sll     e
        jr      nc, offende
        add     a, a
        rl      d
        getbitm
        rl      d
        add     a, a
        rl      d
        getbitm
        ccf
        jr      c, offende
        inc     d
offende rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
        pop     hl
        add     a, a
    if speed=0
        jr      c, maincoe
        jr      copybye
    else
      if speed=1
        jr      c, maincoe
        jp      copybye
      else
        jr      nc, copybye
        ld      c, 2
        jp      mainsie
      endif
    endif

exitdz  pop     hl
      if speed=0
getbit  ld      a, (hl)
        dec     hl
        adc     a, a
      endif
        ret

lb1     ld      a, (hl)
        dec     hl
        adc     a, a
        jr      nc, copybyo

maincoo ld      bc, 2
        push    de
        ld      d, b
        add     a, a
      if speed=0
        jr      c, contino
      else
        jp      c, contino
      endif
        dec     c
lenvalo getbitm
        rl      c
        rl      b
        add     a, a
        jr      nc, lenvalo
        inc     c
        jr      z, exitdz
contino ld      e, (hl)
        dec     hl
        sll     e
        jr      nc, offendo
        getbitm
        rl      d
        add     a, a
        rl      d
        getbitm
        rl      d
        add     a, a
        ccf
        jr      c, offendo
        inc     d
offendo rr      e
        ex      (sp), hl
        ex      de, hl
        adc     hl, de
        lddr
        pop     hl
        add     a,  a
        jr      z, lb1
        jr      c, maincoo
        jp      copybyo
