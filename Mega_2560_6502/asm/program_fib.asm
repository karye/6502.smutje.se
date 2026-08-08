; ================================================================
; Fibonacci-program för 6502 — VIA LCD
; ================================================================
; Räkna ut Fibonacci-sekvensen och visa varje tal på LCD.
; 8-bitars: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233
; Efter 233 wrappar det (byte overflow) och börjar om från 0.
;
; Minneskarta:
;   $00 = F(n-2) — föregående föregående tal
;   $01 = F(n-1) — föregående tal
;   $02 = F(n)   — aktuellt tal (temporärt)
;   $10 = temporär lagring under decimalutskrift

VIA_ORB  = $4000    ; PORTB — LCD:ns 8 datapinnar
VIA_ORA  = $4001    ; PORTA — bit 0 = RS, bit 2 = E
VIA_DDRB = $4002    ; Data Direction för port B
VIA_DDRA = $4003    ; Data Direction för port A

.segment "CODE"
.org $8000

; ================================================================
; RESET — processorns startpunkt
; ================================================================
reset:
    ; --- Gör VIA-portarna till utgångar ---
    lda #$FF          ; 255 = alla pinnar som utgång
    sta VIA_DDRB
    sta VIA_DDRA

    ; --- LCD-initiering (standard för HD44780) ---
    lda #$30          ; "Wake up" — tvinga 8-bitarsläge
    jsr lcd_cmd
    lda #$30          ; Andra gången
    jsr lcd_cmd
    lda #$30          ; Tredje gången — nu är LCD garanterat i 8-bit
    jsr lcd_cmd
    lda #$38          ; Function set: 8-bit, 2 rader, 5×8 font
    jsr lcd_cmd
    lda #$0C          ; Display ON, markör AV, blink AV
    jsr lcd_cmd
    lda #$01          ; Clear display — töm skärmen
    jsr lcd_cmd
    lda #$06          ; Entry mode — markören går åt höger
    jsr lcd_cmd

    ; --- Skriv "Fib:" på rad 1 ---
    lda #$80          ; Sätt markören till rad 1, position 0
    jsr lcd_cmd
    lda #$01          ; RS=1 — nu skickar vi teckendata
    sta VIA_ORA
    ldx #0            ; Teckenräknare = 0
@l1:
    lda txt_fib,x     ; Hämta tecken från strängen "txt_fib"
    beq @l1done       ; Om tecknet är 0 — strängen är slut
    jsr lcd_data      ; Skicka tecknet till LCD
    inx               ; Peka på nästa tecken
    jmp @l1           ; Loop
@l1done:

    ; --- Initiera Fibonacci-variabler ---
    lda #0
    sta $00           ; F(0) = 0 — första talet i serien
    lda #1
    sta $01           ; F(1) = 1 — andra talet i serien

    ; --- Visa F(0) = 0 på LCD ---
    lda #0            ; Visa talet 0
    jsr show_num      ; Subrutin: konvertera till decimal och skriv
    jsr delay_ms      ; Pausa så man hinner se talet

    ; --- Visa F(1) = 1 på LCD ---
    lda #1
    jsr show_num
    jsr delay_ms

; ================================================================
; FIBONACCI-LOOP — hjärtat i programmet
; ================================================================
fib_loop:
    ; --- Beräkna nästa tal: F(n) = F(n-1) + F(n-2) ---
    clc               ; Rensa carry-flaggan före addition
    lda $00           ; Hämta F(n-2)
    adc $01           ; Addera F(n-1) — resultat i A-registret
    bcc no_wrap       ; Om ingen overflow (carry clear) — fortsätt

    ; --- Overflow! Återställ serien ---
    ; När talet blir större än 255 wrappar vi tillbaka
    lda #0
    sta $00           ; F(n-2) = 0
    lda #1
    sta $01           ; F(n-1) = 1
    jmp fib_loop      ; Börja om från F(0)=0

no_wrap:
    sta $02           ; Spara F(n) temporärt

    ; --- Skifta: F(n-2) ← F(n-1), F(n-1) ← F(n) ---
    lda $01           ; Hämta gamla F(n-1)
    sta $00           ; Det blir nya F(n-2)
    lda $02           ; Hämta F(n)
    sta $01           ; Det blir nya F(n-1)

    ; --- Visa talet på LCD ---
    jsr show_num      ; Konvertera till decimal och skriv ut
    jsr delay_ms      ; Paus
    jmp fib_loop      ; Nästa tal!

; ================================================================
; SUBRUTIN: delay_ms — vänta en kort stund
; ================================================================
; Två nästlade loopar: X räknar ner från 5, Y från 5.
; 5 × 5 × 2 instruktioner = ~50 cykler ≈ 1 sekund vid 50 Hz.
delay_ms:
    ldx #5
dly_outer:
    ldy #5
dly_inner:
    dey               ; Y = Y - 1
    bne dly_inner     ; Inte noll? Gå ett varv till
    dex               ; X = X - 1
    bne dly_outer     ; Inte noll? Gå ett varv till
    rts

; ================================================================
; SUBRUTIN: show_num — visa A-registret som decimaltal på rad 2
; ================================================================
; Omvandlar ett 8-bitars tal (0–255) till 1–3 ASCII-siffror.
; Exempel: 144 → "144 ", 5 → "5  "
;
; Algoritm:
;   1. Räkna hundratal: subtrahera 100 tills A < 100
;   2. Räkna tiotal: subtrahera 10 tills A < 10
;   3. Resten är ental
;   4. Konvertera varje del till ASCII (+ '0') och visa
show_num:
    sta $10           ; Spara värdet för senare användning

    ; Sätt markören till rad 2, position 0
    lda #$C0          ; DDRAM-adress $40 → $C0
    jsr lcd_cmd
    lda #$01          ; RS = 1 (dataläge)
    sta VIA_ORA

    ; --- Hundratal (0–2) ---
    lda $10           ; Återställ värdet
    ldx #0            ; X = räknare för hundratal
@h:
    cmp #100          ; Är A >= 100?
    bcc @t            ; Nej — gå till tiotal
    inx               ; Ja — öka hundratalsräknaren
    sec               ; Förbered subtraktion
    sbc #100          ; A = A - 100
    jmp @h            ; Kolla igen (kan vara 200+)
@t:
    pha               ; Spara resten på stacken
    txa               ; Hundratalsräknare → A
    beq @skip_h       ; Om 0 — skriv inget hundratal
    clc
    adc #'0'          ; Konvertera till ASCII ('0' = 48)
    jsr lcd_data      ; Visa hundratalet
@skip_h:
    pla               ; Återställ resten från stacken
    ldx #0            ; Nollställ tiotalräknare

    ; --- Tiotal (0–9) ---
@tens:
    cmp #10           ; Är A >= 10?
    bcc @ones         ; Nej — gå till ental
    inx               ; Ja — öka tiotalräknare
    sec
    sbc #10           ; A = A - 10
    jmp @tens         ; Kolla igen
@ones:
    pha               ; Spara entalet på stacken
    txa               ; Tiotalräknare → A
    bne @show_t       ; Om inte 0 — visa tiotal
    lda $10           ; Kolla ursprungsvärdet
    cmp #10           ; Var det >= 10?
    bcc @skip_t       ; Nej — hoppa över tiotal
@show_t:
    txa               ; Tiotalräknare → A
    clc
    adc #'0'          ; Konvertera till ASCII
    jsr lcd_data      ; Visa tiotalet
@skip_t:
    pla               ; Återställ entalet
    clc
    adc #'0'          ; Konvertera till ASCII
    jsr lcd_data      ; Visa entalet

    ; --- Sudda bort gamla siffror ---
    ; När "144" följs av "5" vill vi inte se "544".
    ; Två mellanslag rensar resten av raden.
    lda #' '
    jsr lcd_data
    lda #' '
    jsr lcd_data
    rts

; ================================================================
; SUBRUTIN: lcd_cmd — skicka kommando till LCD (RS=0)
; ================================================================
lcd_cmd:
    sta VIA_ORB       ; Kommandobyte → PORTB
    lda #$04          ; RS=0, E=1 — "kommando, läs nu"
    sta VIA_ORA
    lda #$00          ; E=0 — fallande flank → LCD läser
    sta VIA_ORA
    rts

; ================================================================
; SUBRUTIN: lcd_data — skicka teckendata till LCD (RS=1)
; ================================================================
lcd_data:
    sta VIA_ORB       ; ASCII-tecken → PORTB
    lda #$05          ; RS=1, E=1 — "tecken, läs nu"
    sta VIA_ORA
    lda #$01          ; RS=1, E=0 — fallande flank
    sta VIA_ORA
    rts

; ================================================================
; Strängdata
; ================================================================
txt_fib:
    .byte "Fib:", 0  ; Rad 1 — etikett

; ================================================================
; RESET-VEKTOR — processorns startadress
; ================================================================
.segment "VECTORS"
.org $FFFA
    .word $0000       ; NMI   — oanvänd
    .word reset       ; RESET — pekar på $8000
    .word $0000       ; IRQ   — oanvänd