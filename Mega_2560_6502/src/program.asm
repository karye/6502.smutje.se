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

VIA_ORB  = $C000
VIA_ORA  = $C001
VIA_DDRB = $C002
VIA_DDRA = $C003

.segment "CODE"
.org $8000

; ============================================================
; Reset — start av programmet
; ============================================================
reset:
    ; --- Sätt VIA-portar som utgångar ---
    lda #$FF
    sta VIA_DDRB       ; PORTB = utgång (LCD data)
    sta VIA_DDRA       ; PORTA = utgång (RS + E)

    ; --- LCD-init: tvinga 8-bitarsläge ($30 × 3) ---
    lda #$30
    jsr lcd_command
    lda #$30
    jsr lcd_command
    lda #$30
    jsr lcd_command

    ; --- Function set: 8-bit, 2 rader, 5×8 ---
    lda #$38
    jsr lcd_command

    ; --- Display ON, cursor OFF, blink OFF ---
    lda #$0C
    jsr lcd_command

    ; --- Clear display ---
    lda #$01
    jsr lcd_command

    ; --- Entry mode: increment, no shift ---
    lda #$06
    jsr lcd_command

; ============================================================
; Huvudloop — skriv fyra rader, cleara, loopa
; ============================================================
hello:
    ; --- Rad 1: cursor till $00, skriv "=== 6502 VIA LCD ===" ---
    lda #$80                ; DDRAM-adress 0
    jsr lcd_command
    lda #$01                ; RS=1 (data-läge)
    sta VIA_ORA
    ldx #0
@l1:
    lda line1,x
    beq @l1_done
    jsr lcd_data
    inx
    jmp @l1
@l1_done:

    ; --- Rad 2: cursor till $40, skriv "Hello from W65C02!" ---
    lda #$C0
    jsr lcd_command
    lda #$01
    sta VIA_ORA
    ldx #0
@l2:
    lda line2,x
    beq @l2_done
    jsr lcd_data
    inx
    jmp @l2
@l2_done:

    ; --- Rad 3 (20×4): cursor till $14, skriv "Arduino = RAM + CLK" ---
    ; För 16×2 hoppas denna över (cursor $14 syns inte)
    lda #$94
    jsr lcd_command
    lda #$01
    sta VIA_ORA
    ldx #0
@l3:
    lda line3,x
    beq @l3_done
    jsr lcd_data
    inx
    jmp @l3
@l3_done:

    ; --- Rad 4 (20×4): cursor till $54, skriv "smutje.se 2026" ---
    lda #$D4
    jsr lcd_command
    lda #$01
    sta VIA_ORA
    ldx #0
@l4:
    lda line4,x
    beq @l4_done
    jsr lcd_data
    inx
    jmp @l4
@l4_done:

    ; --- Clear och loopa om ---
    lda #$01
    jsr lcd_command
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
    rts

; Skicka data i A (RS=1, E-puls)
lcd_data:
    sta VIA_ORB
    lda #$05                ; RS=1, E=1
    sta VIA_ORA
    lda #$01                ; RS=1, E=0 (fallande flank)
    sta VIA_ORA
    rts

; ============================================================
; Strängdata (null-terminerade)
; ============================================================
line1:  .byte "=== 6502 VIA LCD ===", 0
line2:  .byte "Hello from W65C02!", 0
line3:  .byte "Arduino = RAM + CLK", 0
line4:  .byte "smutje.se 2026", 0

; ============================================================
; Vektorer — placeras på $FFFA–$FFFF
; ============================================================
.segment "VECTORS"
.org $FFFA
    .word $0000     ; NMI   (oanvänd)
    .word reset     ; RESET → $8000
    .word $0000     ; IRQ   (oanvänd)
