.define adzp $31
.define ad   $3fc1
;lda #4
nop
and $fc, x
rol adzp, x
nop
eor adzp, x
lsr $3a, x
adc $07, x
inx
ror adzp, x
brk