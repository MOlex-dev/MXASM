* = 0
* = $0600
.define cpfa $3
.define cpf 03 
bpl _start
_start:
lda #%1111111111111111 ; load 4 to accumulator
lda ()
and  %01101001
lda #(X), cpfa
.byte $04
adc 9750
    dosmth:
    .byte $5
countApples:
.byte "this 94u *&#Y 9 is", %11
.byte "mystr"
do_:
;count: X
.define smx 3
lda #sMx
.byte $00,$05,$00,$05,$00,$00,$00,$00
.byte "message" ;this is message $
lda 
lda
;.word $fe4a, $0050
ldx countApples
.byte $5
 .word $fe4a, $3
 lda X
 ldx "f"
somemearK:
;do $0x
    ;    write herehe
lda 034
