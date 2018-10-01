; -----------------------------------------------------------------------------
; ZX7 Backwards by Einar Saukas, Antonio Villena
; "Standard" version (67 bytes only)
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------
dzx7:   ld      bc, $8000       ; marker bit at MSB of B
        ld      a, b            ; move marker bit to A
copyby: inc     c               ; to keep BC value after LDD
        ldd                     ; copy literal byte
mainlo: add     a, a            ; read next bit, faster method
        call    z, getbit       ; call routine only if need to load next byte 
        jr      nc, copyby      ; next bit indicates either literal or sequence

; determine number of bits used for length (Elias gamma coding)
        push    de              ; store destination on stack
        ld      d, c            ; D= 0, assume BC=$0000 here (or $8000)
        defb    $30             ; avoid next add a, a the first time
lenval: add     a, a            ; read bit
        call    z, getbit
        rl      c
        rl      b               ; insert bit on BC
        add     a, a            ; more bits?
        call    z, getbit
        jr      nc, lenval      ; repeat, final length on BC

; check escape sequence
        inc     c               ; detect escape sequence
        jr      z, exitdz       ; end of algorithm

; determine offset
        ld      e, (hl)         ; load offset flag (1 bit) + offset value (7 bits)
        dec     hl
        sll     e
        jr      nc, offend      ; if offset flag is set, load 4 extra bits
        ld      d, $10          ; bit marker to load 4 bits
nexbit: add     a, a
        call    z, getbit
        rl      d               ; insert next bit into D
        jr      nc, nexbit      ; repeat 4 times, until bit marker is out
        inc     d               ; add 128 to DE
        srl     d               ; retrieve fourth bit from D
offend: rr      e               ; insert fourth bit into E
        ex      (sp), hl        ; store source, restore destination
        ex      de, hl          ; destination from HL to DE
        adc     hl, de          ; HL = destination + offset + 1
        lddr
exitdz: pop     hl              ; restore source address (compressed data)
        jr      nc, mainlo      ; end of loop. exit with getbit instead ret (-1 byte)

getbit: ld      a, (hl)         ; load another group of 8 bits
        dec     hl
        adc     a, a
        ret
