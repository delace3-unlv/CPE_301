;
; DA2_3_asm.asm
;
; Created: 3/22/2025 6:27:19 PM
; Author : enriq
;

.include "M328Pdef.inc"
.equ Value       = 0xFB2C      ; Timer preload for ~0.15 sec
.equ DelayCount  = 20          ; 20 x 0.15 s = 3 s total

.cseg
.org 0x0000
    rjmp Setup               

.org 0x0002                  ; INT0 vector
    rjmp INT0_ISR


Setup:
    ldi r16, high(RAMEND)
    out SPH, r16
    ldi r16, low(RAMEND)
    out SPL, r16

    sbi DDRB, DDB5            ; Set PB5 as output

    cbi DDRD, DDD2            ; PD2 input
    sbi PORTD, PORTD2         ; Enable pull-up on PD2

    
    ; set INT0 
    ldi r16, (1 << ISC01)
    sts EICRA, r16            

    ; Enable INT0 interrupt
    ldi r16, (1 << INT0)
    sts EIMSK, r16            

    sei	; Global interrupt enable

Main:
    rjmp Main                

INT0_ISR:
    sbi PORTB, PORTB5	; Turn on LED at PB5
    ldi r20, DelayCount	

DelayLoop:
    rcall Delay
    dec r20
    brne DelayLoop

    ; Turn off LED at PB5
    cbi PORTB, PORTB5

    reti  


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
