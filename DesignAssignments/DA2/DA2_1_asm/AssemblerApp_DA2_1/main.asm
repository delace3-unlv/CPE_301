;
; AssemblerApp_DA2_1.asm
;
; Created: 3/9/2025 7:21:43 PM
; Author : enriq
;

.include "M328Pdef.inc"
.equ Value = 0xFB2C ;Timing value for 0.15 sec delay

Main:
    rcall Delay         ; Call the 0.15 second delay
	nop
Done:
    rjmp Done           ; Loop forever 

Delay:                  ; ~0.15 second delay using Timer1
    ldi R30, high(Value)
    sts TCNT1H, R30
    ldi R30, low(Value)
    sts TCNT1L, R30     ; Load TCNT1 with starting value

    ldi R31, 0x00
    sts TCCR1A, R31     ; Normal mode

    ldi R31, 0x05       ; Prescaler 1024
    sts TCCR1B, R31     ; Start Timer1

Wait:
    sbis TIFR1, TOV1    ; Wait for overflow flag
    rjmp Wait

    sbi TIFR1, TOV1     ; Clear overflow flag
    ldi R30, 0x00
    sts TCCR1B, R30     ; Stop Timer1

    ret
