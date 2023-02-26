.define adzp $f1
.define ad   $4ffe
;lda #4
nop
bbs7 adzp, mark
mark:
bbs5 adzp, _final
nop
bbs6 $41, mark
bbs4 $11, mark 
_final:
nop
