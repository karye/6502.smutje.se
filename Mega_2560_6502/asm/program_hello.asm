; ================================================================
; 6502-program för steg 8 — VIA LCD
; ================================================================
; Det här programmet körs på 6502-processorn, inte på Arduino!
; Processorn pratar direkt med W65C22 VIA-kretsen som i sin tur
; styr LCD-displayen. Varje instruktion är en enda byte (opcode)
; följt av 0–2 byte operander (data/address).
;
; Minneskarta för VIA-register:
;   $4000 = PORTB — skickar data till LCD (8 bitar: DB0–DB7)
;   $4001 = PORTA — styr LCD (bit 0 = RS, bit 2 = E)
;   $4002 = DDRB  — styr riktning på port B ($FF = alla utgång)
;   $4003 = DDRA  — styr riktning på port A

VIA_ORB  = $4000    ; PORTB — LCD:ns 8 datapinnar
VIA_ORA  = $4001    ; PORTA — bit 0 = RS (Register Select)
                     ;         bit 2 = E  (Enable)
VIA_DDRB = $4002    ; Data Direction Register för port B
VIA_DDRA = $4003    ; Data Direction Register för port A

; --- Programmet börjar här ($8000) ---
.segment "CODE"
.org $8000           ; Placera koden från adress $8000

; ================================================================
; RESET — hit hoppar processorn när den startar
; ================================================================
reset:
    ; --- Steg 1: Gör VIA-portarna till utgångar ---
    lda #$FF          ; Ladda talet 255 (binärt: 11111111)
    sta VIA_DDRB      ; Alla 8 pinnar på port B = UTGÅNG
    sta VIA_DDRA      ; Alla 8 pinnar på port A = UTGÅNG

    ; --- Steg 2: Initiera LCD-displayen ---
    ; En LCD måste få en specifik sekvens av kommandon för att
    ; vakna i rätt läge. Det här är standard för HD44780.

    ; $30 × 3 — tvinga LCD till 8-bitarsläge
    ; (LCD kan vakna i 4-bitarsläge, så vi skickar 8-bitars-
    ;  kommandot tre gånger för att vara säkra)
    lda #$30          ; "Function set" — 8-bitars interface
    jsr lcd_command   ; Hoppa till subrutinen som skickar kommandot
    lda #$30
    jsr lcd_command   ; Andra gången — ifall LCD missade första
    lda #$30
    jsr lcd_command   ; Tredje gången — nu är den garanterat i 8-bit

    ; $38 — Function set: 8-bit data, 2 displayrader, 5×8 tecken
    lda #$38
    jsr lcd_command

    ; $0C — Display ON/OFF: skärm på, markör av, blink av
    lda #$0C
    jsr lcd_command

    ; $01 — Clear display: töm skärmen, markören till hemposition
    lda #$01
    jsr lcd_command

    ; $06 — Entry mode: flytta markören åt höger efter varje tecken
    lda #$06
    jsr lcd_command

; ================================================================
; HUVUDLOOP — skriv 4 rader text, rensa, börja om
; ================================================================
hello:
    ; --- Rad 1: "=== 6502 VIA LCD ===" ---
    lda #$80          ; Sätt DDRAM-adress = 0 (rad 1, position 0)
    jsr lcd_command   ; (DDRAM-adress $80 + 0 = $80)
    lda #$01          ; RS = 1 = dataläge (istället för kommandoläge)
    sta VIA_ORA       ; Skriv till PORTA — RS blir hög
    ldx #0            ; X-registret = 0 (räknare för teckenindex)
@l1:
    lda line1,x       ; Hämta tecken nr X från strängen "line1"
    beq @l1_done      ; Om tecknet är 0 (null) — hoppa ur loopen
    jsr lcd_data      ; Skicka tecknet till LCD
    inx               ; X = X + 1 (peka på nästa tecken)
    jmp @l1           ; Tillbaka och hämta nästa tecken
@l1_done:

    ; --- Rad 2: "Hello from W65C02!" ---
    lda #$C0          ; DDRAM-adress $40 → rad 2, position 0
    jsr lcd_command   ; ($80 + $40 = $C0)
    lda #$01
    sta VIA_ORA       ; RS = 1 (data)
    ldx #0
@l2:
    lda line2,x
    beq @l2_done
    jsr lcd_data
    inx
    jmp @l2
@l2_done:

    ; --- Rad 3: "Arduino = RAM + CLK" (20×4 display) ---
    lda #$94          ; DDRAM-adress $14 → rad 3, position 0
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

    ; --- Rad 4: "smutje.se 2026" (20×4 display) ---
    lda #$D4          ; DDRAM-adress $54 → rad 4, position 0
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

    ; --- Rensa skärmen och börja om ---
    lda #$01          ; Clear display
    jsr lcd_command
    jmp hello         ; Hoppa tillbaka till hello — oändlig loop!

; ================================================================
; SUBRUTIN: lcd_command — skicka ett kommando till LCD
; ================================================================
; LCD tar emot data på DB0–DB7 (PORTB) och läser av på
; fallande flank av E (Enable). RS avgör om det är ett
; kommando (RS=0) eller teckendata (RS=1).
;
; Så här går det till, steg för steg:
;   1. Lägg kommandobyten på PORTB (LCD:ns datapinnar)
;   2. Sätt RS=0, E=1 — "här är ett kommando, var redo!"
;   3. Sätt E=0 — fallande flank → LCD läser PORTB
lcd_command:
    sta VIA_ORB       ; Steg 1: A-registret → PORTB (LCD data)
    lda #$04          ; Steg 2: binärt 00000100 → PA2=E=1, PA0=RS=0
    sta VIA_ORA       ;         Skriv till PORTA
    lda #$00          ; Steg 3: binärt 00000000 → E=0 (fallande flank!)
    sta VIA_ORA       ;         LCD läser PORTB nu
    rts               ; Återvänd till anroparen

; ================================================================
; SUBRUTIN: lcd_data — skicka teckendata (ASCII) till LCD
; ================================================================
; Exakt samma som lcd_command, men med RS=1 istället för RS=0.
; RS=1 betyder "det här är ett tecken, visa det på skärmen".
lcd_data:
    sta VIA_ORB       ; ASCII-tecknet → PORTB
    lda #$05          ; 00000101 → PA2=E=1, PA0=RS=1 (data!)
    sta VIA_ORA
    lda #$01          ; 00000001 → E=0, RS=1 (fallande flank)
    sta VIA_ORA       ; LCD läser och visar tecknet
    rts

; ================================================================
; STRÄNGDATA — texten som ska visas
; ================================================================
; .byte lägger in ASCII-värden i minnet, ett efter ett.
; 0 i slutet betyder "null-terminator" — subrutinen vet
; att strängen är slut när den hittar en nolla.
line1: .byte "=== 6502 VIA LCD ===", 0
line2: .byte "Hello from W65C02!", 0
line3: .byte "Arduino = RAM + CLK", 0
line4: .byte "smutje.se 2026", 0

; ================================================================
; RESET-VEKTOR — processorns startadress
; ================================================================
; När 6502:an startar (efter reset) läser den adress $FFFC
; och $FFFD för att få reda på var programmet börjar.
; .word lägger in en 16-bitars adress (låg byte först!).
.segment "VECTORS"
.org $FFFA
    .word $0000       ; NMI   — används inte
    .word reset       ; RESET — här börjar programmet ($8000)
    .word $0000       ; IRQ   — används inte