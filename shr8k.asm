; Copyright 1999-2015 Aske Simon Christensen.
; Decompress Shrinkler-compressed data produced with the -d option.
; Uses 3 kilobytes of buffer anywhere in the memory
; Decompression code may read one longword beyond compressed data.
; The contents of this longword does not matter.
;
; Z80 version of 262 bytes
; roudoudou, Hicks, Antonio Villena & Urusergi
;
; usage
;-------
; sjasmplus demo.asm
; shr8k demo.bin demo.tap DEMO
; 
; Original demo.asm must be org $8000. You can assume SP=$8000, HL=$8000 A=$03
; shrinkler http://www.pouet.net/prod.php?which=64851
; sjasmplus https://github.com/DSkywalk/fase/blob/master/engine/src/sjasmplus/sjasmplus.exe
; GenTape   https://github.com/antoniovillena/zx7b/blob/master/GenTape/GenTape.exe

        output  shr8k.bin
        org     $5ccb
        define  shrinkler_pr  shrinkler_dr+6

l5ccb:  ld      bc, data-shrinkler_dr
        add     ix, bc
        defb    $de, $c0, $37, $0e, $8f, $39, $96 ; paolo ferraris method, in basic jump to $5ccb
l5cd6:  
        di
        xor     a
shrinkler_repeat:
        ld      (de), a
        xor     $80
        inc     de
        ld      h, ($f400-shrinkler_pr)>>8
        add     hl, de
        jr      nc, shrinkler_repeat
        ld      hl, $8000
        ld      sp, hl
        ex      de, hl

shrinkler_lit:
        ; Literal
        ld      hl, shrinkler_d6+1
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
        inc     a
        call    shrinkler_getnumber
shrinkler_d5:
        ld      hl, 0
        add     hl, de
        ldir
        ; After reference
        call    shrinkler_getkind
        jr      nc, shrinkler_lit
shrinkler_readoffset:
        call    shrinkler_getnumber
        ld      hl, 2
        sbc     hl, bc
        ld      (shrinkler_d5+1), hl
        jr      nz, shrinkler_readlength
        add     hl, sp
        jp      (hl)

;--------------------------------------------------
        ; Out: Bit in C
shrinkler_readbit:
        ld      (shrinkler_d3+1), hl
shrinkler_d4l:
        ld      hl, 0
shrinkler_d4h:
        ld      de, #8000
shrinkler_rb0:
        adc     hl, hl
        ld      (shrinkler_d4l+1), hl
        ex      de, hl
        jr      nz, shrinkler_rb2-1
        adc     hl, hl
        jr      nz, shrinkler_rb3
shrinkler_rb1:
        ld      h, (ix)
        ld      l, (ix+1)
        inc     ix
        inc     ix
        ccf
        jr      c, shrinkler_rb0
        ex      de, hl
        jr      shrinkler_rb1-3
shrinkler_rb2:
        ld      l, d ; adc hl, hl with ed prefix
shrinkler_rb3:
        ld      (shrinkler_d4h+1), hl 
shrinkler_d2:
        ld      hl, 0
        adc     hl, hl
        ld      (shrinkler_d2+1), hl
        jr      shrinkler_getbit1

shrinkler_getnumber:
        ; Out: Number in BC
        ld      bc, 1
        ld      hl, shrinkler_d6+2
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
shrinkler_getkind:
        ;Use parity as context
        ld      l, 1
shrinkler_altgetbit:
        ld      a, l
        and     e
        ld      h, a
        dec     hl
        ld      (shrinkler_d6+1), hl

shrinkler_getbit:
        exx
shrinkler_getbit1:
        ld      hl, (shrinkler_d3+1)
        add     hl, hl
        jr      nc, shrinkler_readbit
shrinkler_d6:
        ld      hl, 0
        add     hl, hl
        ld      de, shrinkler_pr+2      ; cause -1 context
        add     hl, de
        push    hl
        ld      e, (hl)
        inc     hl
        ld      d, (hl)
        ; D1 = One prob
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
shrinkler_d3:
        ld      de, 1
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
        pop     bc
        ld      hl, (shrinkler_d2+1)
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

shrinkler_zero:
        ; oneprob = oneprob * (1 - adjust) = oneprob - oneprob * adjust
        ld      (shrinkler_d2+1), hl
        ld      hl, (shrinkler_d3+1)
        sbc     hl, de

shrinkler_d3ret:
        ld      (shrinkler_d3+1), hl
        exx
        ld      a, 3
        ret
data:   incbin  data.shr
shrinkler_dr:
