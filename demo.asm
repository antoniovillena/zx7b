        output  example2.bin
        org     $8000
        ld      hl, data
        ld      de, $4000
        ld      bc, $1b00
        ldir
        halt
data    incbin  manic.scr
