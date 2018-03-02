; Copyright 1999-2015 Aske Simon Christensen.
; Decompress Shrinkler-compressed data produced with the -d option.
; Uses 3 kilobytes of buffer anywhere in the memory
; Decompression code may read one longword beyond compressed data.
; The contents of this longword does not matter.
;
; official Z80 release - 273 bytes
; Conversion by roudoudou and size optimisations by Hicks & Antonio Villena
;
; IX=source
; DE=destination
; call shrinkler_decrunch

        ; need to be the first register
        define  shrinkler_d3  shrinkler_dr    ; defw 0
        define  shrinkler_d2  shrinkler_d3+2  ; defw 0
        define  shrinkler_d6  shrinkler_d2+2  ; defw 0
        ; need to be the last register
        define  shrinkler_d4  shrinkler_d6+2  ; defw 0,0
        define  shrinkler_pr  shrinkler_d4+4  ; 3072 bytes buffer!!!

shrinkler_getnumber:
        ; Out: Number in BC
        ld      bc, 1
        ld      hl, shrinkler_d6+1
        ld      (hl), a
        dec     hl
        ld      (hl), c
shrinkler_numberloop:
        inc     (hl)
        call    shrinkler_getbit
        inc     (hl)
        jr      c, shrinkler_numberloop
shrinkler_bitsloop:
        dec     (hl)
        dec     (hl)
        ret     m
        call    shrinkler_getbit
        rl      c
        rl      b
        jr      shrinkler_bitsloop

;--------------------------------------------------

shrinkler_readbit:
        pop     de
        ld      (shrinkler_d3), hl
        ld      hl, (shrinkler_d4)
        ld      de, (shrinkler_d4+2)
shrinkler_rb0:
        adc     hl, hl
        ld      (shrinkler_d4), hl
        ex      de, hl
        jr      nz, shrinkler_rb2-1

shrinkler_rb4:
        adc     hl, hl
        jr      nz, shrinkler_rb3
        ; HL=DE=0
shrinkler_rb1:
        ccf
        ld      h, (ix)               ; DEHL=(a4) big endian value read!
        ld      l, (ix+1)
        inc     ix
        inc     ix
        jr      c, shrinkler_rb0
        ex      de, hl
        jr      shrinkler_rb1-3

shrinkler_rb2:
        ld      l, d                    ; adc hl, hl with $ed
shrinkler_rb3:
        ld      (shrinkler_d4+2), hl    ; written in little endian
        ld      hl, (shrinkler_d2)
        adc     hl, hl
        ld      (shrinkler_d2), hl
        jr      shrinkler_getbit1

shrinkler_getkind:
        ;Use parity as context
        ld      l, 1
shrinkler_altgetbit:
        ld      a, l
        and     e
        ld      h, a
        dec     hl
        ld      (shrinkler_d6), hl

shrinkler_getbit:
        exx
shrinkler_getbit1:
        ld      hl, (shrinkler_d3)
        push    hl
        add     hl, hl
        jr      nc, shrinkler_readbit
        ld      hl, (shrinkler_d6)
        add     hl, hl
        ld      de, shrinkler_pr+2      ; cause -1 context
        add     hl, de
        ld      e, (hl)
        inc     hl
        ld      d, (hl)
        ld      b, d
        ld      c, e                    ; bc=de=d1 / hl=a1
        ld      a, $eb
shrinkler_shift4:
        srl     b
        rr      c
        add     a, a
        jr      c, shrinkler_shift4-1
        sbc     hl, bc                  ; hl=d1-d1/16
        ex      de, hl
        ld      b, (hl)
        ld      (hl), d
        dec     hl
        ld      c, (hl)
        ld      (hl), e
        ex      (sp), hl
        ex      de, hl
        sbc     hl, hl
shrinkler_muluw:
        add     hl, hl
        rl      e
        rl      d
        jr      nc, shrinkler_cont
        add     hl, bc
        jr      nc, shrinkler_cont
        inc     de
shrinkler_cont:
        sub     $b
        jr      nz, shrinkler_muluw
        pop     bc                      ; bc=d1 initial
        ld      hl, (shrinkler_d2)
        sbc     hl, de
        jr      nc, shrinkler_zero

shrinkler_one:
        ; onebrob = 1 - (1 - oneprob) * (1 - adjust) = oneprob - oneprob * adjust + adjust
        ld      a, (bc)
        sub     1
        ld      (bc), a
        inc     bc
        ld      a, (bc)
        sbc     a, $f0                  ; (a1)+#FFF
        ld      (bc), a
        ex      de, hl
        jr      shrinkler_d3ret

shrinkler_decrunch:
        ; Init range decoder state
        ld      hl, shrinkler_dr
        xor     a
        ex      af, af'
        xor     a
        ld      bc, $070d
        ld      (hl), 1
shrinkler_repeat:
        inc     hl
        ex      af, af'
        ld      (hl), a
        djnz    shrinkler_repeat
        ld      a, $80
        dec     c
        jr      nz, shrinkler_repeat

shrinkler_lit:
        ld      hl, shrinkler_d6
        inc     (hl)
shrinkler_getlit:
        call    shrinkler_getbit
        rl      (hl)
        jr      nc, shrinkler_getlit
        ldi
        ; After literal
        call    shrinkler_getkind
        jr      nc, shrinkler_lit
        ; Reference
        call    shrinkler_altgetbit
        jr      nc, shrinkler_readoffset
shrinkler_readlength:
        call    shrinkler_getnumber
shrinkler_d5:
        ld      hl, 0
        add     hl, de
        ldir
        ; After reference
        call    shrinkler_getkind
        jr      nc, shrinkler_lit
shrinkler_readoffset:
        dec     a
        call    shrinkler_getnumber
        ld      hl, 2
        sbc     hl, bc
        ld      (shrinkler_d5+1), hl
        jr      nz, shrinkler_readlength

shrinkler_zero:
        ; oneprob = oneprob * (1 - adjust) = oneprob - oneprob * adjust
        ld      (shrinkler_d2), hl
        ld      hl, (shrinkler_d3)
        sbc     hl, de
        ; oneprob*adjust < oneprob so carry is always cleared...
shrinkler_d3ret:
        ld      (shrinkler_d3), hl
        exx
        ld      a, 4
        ret
