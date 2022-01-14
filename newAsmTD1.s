.equ __24FJ1024GB610, 1
.include "xc.inc"

.text 
    
.global __reset

__reset:
    
    bclr TRISA, #0 ;17
    bset LATA, #0
    
    bclr TRISA, #1.3 ;38
    bset LATA, #1
    
    
    bclr TRISA, #0 ;58
    ;bset LATA, #2
    
    
label:
    bra label
    .end
