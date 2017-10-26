        output  example.bin
        org     $8000
        ld      hl, binario-1
        ld      de, $5aff
        call    dzx7
        di
        halt
        include dzx7b_slow.asm
        incbin  manic.zx7b
binario: