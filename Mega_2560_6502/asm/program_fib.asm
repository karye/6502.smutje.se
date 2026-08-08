; Fibonacci-program för steg 9 — 8-bitars Fibonacci på LCD
; Byggs med ca65/ld65
;
; Visar "Fib:" på rad 1, och varje tal i sekvensen på rad 2
; Sekvens: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233

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
    lda #$38
    jsr lcd_cmd
    lda #$0C
    jsr lcd_cmd
    lda #$01
    jsr lcd_cmd
    lda #$06
    jsr lcd_cmd

    ; --- Rad 1: "Fib:" ---
    lda #$80
    jsr lcd_cmd
    lda #$01
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
    sta $00           ; F(0) = 0
    lda #1
    sta $01           ; F(1) = 1

    ; --- Visa F(0) = 0 ---
    lda #0
    jsr show_num
    jsr delay_ms

    ; --- Visa F(1) = 1 ---
    lda #1
    jsr show_num
    jsr delay_ms

fib_loop:
    ; --- Beräkna nästa: F(n) = F(n-1) + F(n-2) ---
    clc
    lda $00
    adc $01
    bcc no_wrap
    ; Wrappa
    lda #0
    sta $00
    lda #1
    sta $01
    jmp fib_loop
no_wrap:
    sta $02           ; Spara F(n)

    ; --- Skifta ---
    lda $01
    sta $00
    lda $02
    sta $01

    ; --- Visa talet ---
    jsr show_num
    jsr delay_ms
    jmp fib_loop

; ============================================================
; delay_ms — paus (~0.5s vid 20Hz)
; ============================================================
delay_ms:
    ldx #5
dly_outer:
    ldy #5
dly_inner:
    dey
    bne dly_inner
    dex
    bne dly_outer
    rts

; ============================================================
; show_num — visa A som decimal på LCD rad 2, pos 0
; ============================================================
show_num:
    sta $10           ; Spara värdet

    ; Rad 2, pos 0
    lda #$C0
    jsr lcd_cmd
    lda #$01          ; RS=1
    sta VIA_ORA

    ; --- Hundratal ---
    lda $10
    ldx #0
@h:
    cmp #100
    bcc @t
    inx
    sec
    sbc #100
    jmp @h
@t:
    pha
    txa
    beq @skip_h
    clc
    adc #'0'
    jsr lcd_data
@skip_h:
    pla
    ldx #0

    ; --- Tiotal ---
@tens:
    cmp #10
    bcc @ones
    inx
    sec
    sbc #10
    jmp @tens
@ones:
    pha
    txa
    bne @show_t
    lda $10
    cmp #10
    bcc @skip_t
@show_t:
    txa
    clc
    adc #'0'
    jsr lcd_data
@skip_t:
    pla
    clc
    adc #'0'
    jsr lcd_data
    lda #' '          ; Rensa nästa position
    jsr lcd_data
    rts

; ============================================================
; lcd_cmd — skicka kommando i A (RS=0)
; ============================================================
lcd_cmd:
    sta VIA_ORB
    lda #$04          ; RS=0, E=1
    sta VIA_ORA
    lda #$00          ; RS=0, E=0
    sta VIA_ORA
    rts

; ============================================================
; lcd_data — skicka data i A (RS=1)
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
