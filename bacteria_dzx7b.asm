                %include  "define.asm"
                org     $100
start           push    si
                mov     ch, (final-start)>>8
                mov     di, start-final-$100
                rep     movsb
                std
                mov     si, $0feff
                mov     di, rawsize+$0ff
                mov     bp, getbit+start-final-$200
                mov     al, $80
                jmp     copyby+start-final-$200
belsi           mov     bl, [si]
                dec     si
                rcl     bl, 1
                jnc     offend
                mov     bh, $10
nexbit          call    bp
                rcl     bh, 1
                jnc     nexbit
                inc     bh
                shr     bh, 1
offend          rcr     bl, 1
                push    si
                lea     si, [di+bx+1]
                rep     movsb
                pop     si
                db      $3c
copyby          movsb
mainlo          call    bp
                jnc     copyby
                mov     bh, cl
                db      $3d
lenval          call    bp
                rcl     cx, 1
                call    bp
                jnc     lenval
                inc     cl
                jnz     belsi
getbit          add     al, al
                jnz     toret
                lodsb
                adc     al, al
toret           ret
                incbin "bacteria.com.zx7b"
final:
