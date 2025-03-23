;
; DA2_2_asm.asm
;
; Created: 3/21/2025 11:29:28 PM
; Author : enriq
;

.include "M328Pdef.inc"
.equ Value = 0xFB2C       ; Timer preload for ~0.15 sec
.equ DelayCount = 10      ; 10 * 0.15 sec = 1.5 sec


Main:
    ; Set PORTB5 as output
    sbi DDRB, DDB5

    ; Set PORTC1 as input
    cbi DDRC, DDC1

    ; Enable internal pull-up on PORTC1
    sbi PORTC, PORTC1

Loop:
    ; Check if button on PORTC1 is pressed (low)
    sbic PINC, PINC1       ; Skip next if PINC1 is pressed
    rjmp Loop              ; Not pressed, loop again

    ; Turn on LED on PORTB5
    sbi PORTB, PORTB5

    ; Delay for 1.5 seconds using 10x 0.15 sec Delay
    ldi R20, DelayCount
DelayLoop:
    rcall Delay
    dec R20
    brne DelayLoop

    ; Turn off LED on PORTB5
    cbi PORTB, PORTB5

    rjmp Loop              ; Wait for next press

; 0.15 Second Delay 

Delay:
    ldi R30, high(Value)
    sts TCNT1H, R30
    ldi R30, low(Value)
    sts TCNT1L, R30         ; Load TCNT1 with starting value

    ldi R31, 0x00
    sts TCCR1A, R31         ; Normal mode

    ldi R31, 0x05           ; Prescaler 1024
    sts TCCR1B, R31         ; Start Timer1

Wait:
    sbis TIFR1, TOV1        ; Wait for overflow flag
    rjmp Wait

    sbi TIFR1, TOV1         ; Clear overflow flag
    ldi R30, 0x00
    sts TCCR1B, R30         ; Stop Timer1

    ret
