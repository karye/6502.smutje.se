; Fibonacci-program för steg 9 — 8-bitars Fibonacci på LCD
; Byggs med ca65/ld65
;
; Visar "Fib:" på rad 1, och varje tal i sekvensen på rad 2
; Sekvens: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
; Efter 233 wrappar det (byte overflow) och börjar om

VIA_ORB  = $4000
VIA_ORA  = $4001
VIA_DDRB = $4002
VIA_DDRA = $4003

.segment "CODE"
.org $8000

reset:
    ; --- VIA-init ---
    lda #$FF
    sta VIA_DDRB
    sta VIA_DDRA

    ; --- LCD-init ---
    lda #$30
    jsr lcd_cmd
    lda #$30
    jsr lcd_cmd
    lda #$30
    jsr lcd_cmd
    lda #$38          ; 8-bit, 2 rader
    jsr lcd_cmd
    lda #$0C          ; Display on
    jsr lcd_cmd
    lda #$01          ; Clear
    jsr lcd_cmd
    lda #$06          ; Entry mode
    jsr lcd_cmd

    ; --- Rad 1: "Fib:" ---
    lda #$80          ; Cursor rad 1 pos 0
    jsr lcd_cmd
    lda #$01          ; RS=1
    sta VIA_ORA
    ldx #0
@l1:
    lda txt_fib,x
    beq @l1done
    jsr lcd_data
    inx
    jmp @l1
@l1done:

    ; --- Initiera Fibonacci ---
    lda #0
    sta $00           ; F(n-2) = 0
    lda #1
    sta $01           ; F(n-1) = 1

    ; --- Visa F(0) = 0 först ---
    lda #0
    jsr show_num

fib_loop:
    ; --- Beräkna nästa tal: F(n) = F(n-1) + F(n-2) ---
    clc
    lda $00           ; F(n-2)
    adc $01           ; + F(n-1)
    bcc no_wrap       ; Hoppa om ingen overflow
    ; --- Wrappa ---
    lda #0
    sta $00
    lda #1
    sta $01
    jmp fib_loop
no_wrap:
    sta $02           ; Spara F(n)

    ; --- Skifta: F(n-2) ← F(n-1), F(n-1) ← F(n) ---
    lda $01
    sta $00
    lda $02
    sta $01

    ; --- Visa talet på LCD ---
    jsr show_num

    ; --- Delay ---
    ldx #5
delay_outer:
    ldy #0
delay_inner:
    dey
    bne delay_inner
    dex
    bne delay_outer

    jmp fib_loop

; ============================================================
; show_num — visa A-registret som decimaltal på LCD rad 2
; ============================================================
show_num:
    sta $10           ; Spara värdet

    ; Rad 2, pos 0
    lda #$C0
    jsr lcd_cmd
    lda #$01          ; RS=1
    sta VIA_ORA

    ; Rensa resten av raden med mellanslag
    lda #' '
    jsr lcd_data
    jsr lcd_data
    jsr lcd_data
    jsr lcd_data

    ; Rad 2, pos 0 igen (skriv över)
    lda #$C0
    jsr lcd_cmd
    lda #$01
    sta VIA_ORA

    ; Konvertera till decimal (0-233, max 3 siffror)
    lda $10
    ldx #0           ; Hundratal

@hundreds:
    cmp #100
    bcc @do_tens
    inx
    sec
    sbc #100
    jmp @hundreds

@do_tens:
    pha               ; Spara rest
    txa
    beq @skip_h       ; Hoppa över om hundratal = 0
    clc
    adc #'0'          ; Konvertera till ASCII
    jsr lcd_data
@skip_h:
    pla               ; Återställ rest
    ldx #0

@tens:
    cmp #10
    bcc @do_ones
    inx
    sec
    sbc #10
    jmp @tens

@do_ones:
    pha
    ; Visa tiotal om >0 eller om hundratal visades
    txa
    bne @show_tens
    lda $10
    cmp #10
    bcc @skip_t
@show_tens:
    txa
    clc
    adc #'0'
    jsr lcd_data
@skip_t:
    pla
    clc
    adc #'0'
    jsr lcd_data
    rts

; ============================================================
; lcd_cmd — skicka kommando i A
; ============================================================
lcd_cmd:
    sta VIA_ORB       ; Data till PORTB
    lda #$04          ; RS=0, E=1
    sta VIA_ORA
    lda #$00          ; RS=0, E=0
    sta VIA_ORA
    rts

; ============================================================
; lcd_data — skicka data (tecken) i A
; ============================================================
lcd_data:
    sta VIA_ORB
    lda #$05          ; RS=1, E=1
    sta VIA_ORA
    lda #$01          ; RS=1, E=0
    sta VIA_ORA
    rts

; ============================================================
; Strängar
; ============================================================
txt_fib:
    .byte "Fib:", 0

; ============================================================
; Vektorer
; ============================================================
.segment "VECTORS"
.org $FFFA
    .word $0000       ; NMI
    .word reset       ; RESET
    .word $0000       ; IRQ
