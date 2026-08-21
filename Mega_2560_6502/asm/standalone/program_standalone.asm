; ==============================================================
; Steg 12 — Fristående dator: LCD-hello med tidshantering
; ==============================================================
; Körs direkt ur EEPROM:en med riktig kristallklocka (1 MHz).
; Vid 1 MHz måste programmet själv vänta mellan LCD-kommandona —
; förut var klockan så långsam (500 Hz) att allt hanns med av sig själv.
;
; Adresser (steg 11-minneskartan):
;   $8000 = VIA PORTB — LCD:ns 8 datapinnar
;   $8001 = VIA PORTA — bit 0 = RS, bit 2 = E
;   $8002 = VIA DDRB
;   $8003 = VIA DDRA

VIA_ORB  = $8000
VIA_ORA  = $8001
VIA_DDRB = $8002
VIA_DDRA = $8003

DELAY_TMP = $10     ; temporär räknare (i SRAM:en, zero page)

.segment "CODE"
.org $C000

; ================================================================
; RESET — processorns startpunkt
; ================================================================
reset:
    ; --- Gör VIA-portarna till utgångar ---
    lda #$FF
    sta VIA_DDRB
    sta VIA_DDRA

    ; --- LCD-initiering (8-bitarsläge) ---
    lda #$30
    jsr lcd_cmd
    lda #$30
    jsr lcd_cmd
    lda #$30
    jsr lcd_cmd
    lda #$38          ; Function set: 8-bit, 2 rader, 5x8
    jsr lcd_cmd
    lda #$0C          ; Display ON, cursor OFF
    jsr lcd_cmd
    lda #$01          ; Clear display
    jsr lcd_cmd
    lda #$06          ; Entry mode: höger, inget skift
    jsr lcd_cmd

; ================================================================
; HUVUDLOOP — skriv två rader, rensa, börja om
; ================================================================
hello:
    ; --- Rad 1: "6502 FRISTAENDE" ---
    lda #$80          ; DDRAM-adress 0 (rad 1)
    jsr lcd_cmd
    lda #$01          ; RS = 1 (dataläge)
    sta VIA_ORA
    ldx #0
@l1:
    lda line1,x
    beq @l1done
    jsr lcd_data
    inx
    jmp @l1
@l1done:

    ; --- Rad 2: "1 MHz KRISTALL" ---
    lda #$C0          ; DDRAM-adress $40 (rad 2)
    jsr lcd_cmd
    lda #$01
    sta VIA_ORA
    ldx #0
@l2:
    lda line2,x
    beq @l2done
    jsr lcd_data
    inx
    jmp @l2
@l2done:

    ; --- Vänta, rensa och börja om ---
    lda #250
    jsr delay_ms      ; visa texten en stund
    lda #$01          ; Clear display
    jsr lcd_cmd
    jmp hello

; ================================================================
; SUBRUTIN: lcd_cmd — skicka kommando (RS=0) med väntan
; ================================================================
lcd_cmd:
    sta VIA_ORB       ; kommandobyte → PORTB
    lda #$04          ; RS=0, E=1
    sta VIA_ORA
    lda #$00          ; E=0 — fallande flank → LCD läser
    sta VIA_ORA
    lda #2
    jsr delay_ms      ; ~2 ms — HD44780 kräver paus mellan kommandon
    rts

; ================================================================
; SUBRUTIN: lcd_data — skicka tecken (RS=1) med väntan
; ================================================================
lcd_data:
    sta VIA_ORB       ; tecken → PORTB
    lda #$05          ; RS=1, E=1
    sta VIA_ORA
    lda #$01          ; RS=1, E=0 — fallande flank
    sta VIA_ORA
    lda #1
    jsr delay_ms      ; ~1 ms — displayen hinner bearbeta tecknet
    rts

; ================================================================
; SUBRUTIN: delay_ms — vänta ungefär A millisekunder vid 1 MHz
; ================================================================
; Vid 1 MHz tar varje instruktion ~2–3 cykler ≈ 2–3 µs. Den inre
; loopen (~4 cykler per varv, 250 varv) ger ~1 ms per yttervarv.
delay_ms:
    sta DELAY_TMP
@outer:
    lda #250          ; inre räknare
@inner:
    sec
    sbc #1            ; 2 cykler
    bne @inner        ; 2 cykler — ~4 cykler per varv ≈ 1 ms totalt
    dec DELAY_TMP
    bne @outer
    rts

; ================================================================
; Strängdata (ren ASCII — HD44780 har inga å/ä/ö)
; ================================================================
line1: .byte "6502 FRISTAENDE", 0
line2: .byte "1 MHz KRISTALL", 0

; ================================================================
; RESET-VEKTOR — processorns startadress ($C000)
; ================================================================
.segment "VECTORS"
.org $FFFA
    .word $0000       ; NMI — oanvänd
    .word reset       ; RESET → $C000
    .word $0000       ; IRQ — oanvänd
