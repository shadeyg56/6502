; Minimal ADC test.
; Flat 64KB image: file offset == address. Reset vector at $FFFC -> $0200.
;
; Expected result: A = $0F, $0010 = $0A, $0011 = $0F
; Flags after ADC: carry clear, zero clear, negative clear, overflow clear.

        .org $0200

start:
        LDA #$0A        ; A9 0A   A = 10
        STA $10         ; 85 10   [$0010] = 10
        LDA #$05        ; A9 05   A = 5
        CLC             ; 18      clear carry before add
        ADC $10         ; 65 10   A = 5 + 10 = 15
        STA $11         ; 85 11   [$0011] = 15

halt:
        JMP halt        ; 4C 0B 02  self-loop at $020B

        .org $FFFC
        .word start     ; reset vector
