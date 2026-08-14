; ADC + stack exercise.
; Flat 64KB image: file offset == address. Reset vector at $FFFC -> $0200.
;
; After the ADC:   A = $0F, $0010 = $0A, $0011 = $0F
; After the pushes: SP walks $FF -> $FA, $01FF..$01FB = 11 22 33 44 55
; After the pulls:  SP walks back to $FF, A ends at $11
; Stack bytes are not cleared on pull, so they stay visible afterwards.

        .org $0200

start:
        LDA #$0A        ; A9 0A   A = 10
        STA $10         ; 85 10   [$0010] = 10
        LDA #$05        ; A9 05   A = 5
        CLC             ; 18      clear carry before add
        ADC $10         ; 65 10   A = 5 + 10 = 15
        STA $11         ; 85 11   [$0011] = 15

push:
        LDA #$11        ; A9 11
        PHA             ; 48      -> $01FF
        LDA #$22        ; A9 22
        PHA             ; 48      -> $01FE
        LDA #$33        ; A9 33
        PHA             ; 48      -> $01FD
        LDA #$44        ; A9 44
        PHA             ; 48      -> $01FC
        LDA #$55        ; A9 55
        PHA             ; 48      -> $01FB

pull:
        PLA             ; 68      A = $55
        PLA             ; 68      A = $44
        PLA             ; 68      A = $33
        PLA             ; 68      A = $22
        PLA             ; 68      A = $11

halt:
        JMP halt        ; 4C 1F 02  self-loop at $021F

        .org $FFFC
        .word start     ; reset vector
