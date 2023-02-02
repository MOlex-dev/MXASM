* = $2000
.define cpfa $3
.define cpf 03

.define sz %0101010

bpl _start
_start:

lda #$4 ; load 4 to accumulator
lda ()
and  %01101001
lda #cpfa
.byte $04
adc 975092
.word sz
    dosmth:
countApples: .word $0500
.byte "this 94u *&#Y 9 is", $05

do_:
count:

.define smx 3
lda #smx
.byte "message" ;this is message $
lda 
lda
.byte  

label:    count:
;do $0x
    ;    write herehe
lda 034
