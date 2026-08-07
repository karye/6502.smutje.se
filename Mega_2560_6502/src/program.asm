; 6502-program för steg 7/8 — VIA LCD
; Byggs med ca65/ld65 → program.bin → program.h → inkluderas av Arduino
;
; Adressrymd:
;   $8000–$87FF: program (2 KB)
;   $4000–$400F: W65C22 VIA
;   $FFFA–$FFFF: vektorer
;
; VIA-register:
;   $4000 = ORB  (PORTB) — LCD data D0–D7
;   $4001 = ORA  (PORTA) — PA0=RS, PA2=E
;   $4002 = DDRB
;   $4003 = DDRA

VIA_ORB  = $4000
VIA_ORA  = $4001
VIA_DDRB = $4002
VIA_DDRA = $4003

.segment "CODE"
.org $8000

; ============================================================
; Reset — start av programmet
; ============================================================
reset:
    ; --- Power-on delay för LCD ---
    jsr delay_30ms

    ; --- Sätt VIA-portar som utgångar ---
    lda #$FF
    sta VIA_DDRB       ; PORTB = utgång (LCD data)
    sta VIA_DDRA       ; PORTA = utgång (RS + E)

    ; --- LCD-init: tvinga 8-bitarsläge ($30 × 3) ---
    lda #$30
    jsr lcd_command
    jsr delay_5ms
    lda #$30
    jsr lcd_command
    jsr delay_5ms
    lda #$30
    jsr lcd_command
    jsr delay_5ms

    ; --- Function set: 8-bit, 2 rader, 5×8 ---
    lda #$38
    jsr lcd_command
    jsr delay_50us

    ; --- Display ON, cursor OFF, blink OFF ---
    lda #$0C
    jsr lcd_command
    jsr delay_50us

    ; --- Clear display ---
    lda #$01
    jsr lcd_command
    jsr delay_2ms

    ; --- Entry mode: increment, no shift ---
    lda #$06
    jsr lcd_command
    jsr delay_50us

; ============================================================
; Huvudloop — skriv fyra rader, cleara, loopa
; ============================================================
hello:
    ; --- Rad 1: cursor till $00, skriv "6502 VIA LCD 2x16" ---
    lda #$80                ; DDRAM-adress 0
    jsr lcd_command
    jsr delay_50us
    ldx #0
@l1:
    lda line1,x
    beq @l1_done
    jsr lcd_data
    inx
    jmp @l1
@l1_done:

    ; --- Rad 2: cursor till $40, skriv "smutje.se W65C02" ---
    lda #$C0
    jsr lcd_command
    jsr delay_50us
    ldx #0
@l2:
    lda line2,x
    beq @l2_done
    jsr lcd_data
    inx
    jmp @l2
@l2_done:

    ; --- Clear och loopa om ---
    lda #$01
    jsr lcd_command
    jsr delay_2ms
    jmp hello

; ============================================================
; Subrutiner
; ============================================================

; Skicka kommando i A (RS=0)
lcd_command:
    sta VIA_ORB             ; Data till PORTB
    lda #$04                ; RS=0, E=1
    sta VIA_ORA
    lda #$00                ; RS=0, E=0 (fallande flank)
    sta VIA_ORA
    jsr delay_50us          ; LCD-exec time (37+ µs)
    rts

; Skicka data i A (RS=1, E-puls)
lcd_data:
    sta VIA_ORB
    lda #$05                ; RS=1, E=1
    sta VIA_ORA
    lda #$01                ; RS=1, E=0 (fallande flank)
    sta VIA_ORA
    jsr delay_50us          ; LCD-exec time
    rts

; ============================================================
; Strängdata (null-terminerade)
; ============================================================
line1:  .byte "6502 VIA LCD 2x16", 0
line2:  .byte "smutje.se W65C02", 0

; ============================================================
; Delay-rutiner (1 MHz = 1 µs per cykel)
; ============================================================

; 30 ms delay (för power-on)
delay_30ms:
    ldy #30
@outer30:
    ldx #0
@inner30:
    dex
    bne @inner30
    dey
    bne @outer30
    rts

; 5 ms delay
delay_5ms:
    ldy #5
@outer5:
    ldx #0
@inner5:
    dex
    bne @inner5
    dey
    bne @outer5
    rts

; 2 ms delay (för clear display)
delay_2ms:
    ldy #2
@outer2:
    ldx #0
@inner2:
    dex
    bne @inner2
    dey
    bne @outer2
    rts

; ~50 µs delay (för vanliga kommandon)
delay_50us:
    ldx #17
@d50:
    dex
    bne @d50
    rts

; ============================================================
; Vektorer — placeras på $FFFA–$FFFF
; ============================================================
.segment "VECTORS"
.org $FFFA
    .word $0000     ; NMI   (oanvänd)
    .word reset     ; RESET → $8000
    .word $0000     ; IRQ   (oanvänd)
