        .ORIG X3000
        AND R0, R0, #0              ;R1 = NUMBER
        AND R1, R1, #0
        AND R2, R2, #0
        AND R3, R3, #0              ;R3 CONTAINS AMT OF ODDS
        AND R4, R4, #0              ;R4 IS THROWAWAY

        LEA R0, START
        LDW R0, R0, #0              ;R0=X4000

        LEA R2, TARGET
        LDW R2, R2, #0              ;R2=X4100

AGAIN   AND R4, R4, #0

        ADD R4, R4, R0
        XOR R4, R4, R2
        BRz DONE                    ;CHECKS IF CURRENT ADDRESS MATCHES FINAL

        LDB R1, R0, #0              ;R1 = NUMBER

        ADD R0, R0, #1              ;X4000 + 1
        AND R1, R1, #1              ;IF R1&#1 WILL BE 1 IF ODD AND 0 IF EVEN
        BRz AGAIN
        ADD R3, R3, #1              ;R3++
        BR AGAIN
DONE    STB R3, R2, #0

        HALT

START	  .FILL X4000
TARGET	.FILL X4100

        .END
