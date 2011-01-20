;
; OS Project 1 - Noel Vargas and Richard Bateman
;
; Richard Bateman
;

text1   .BYT    "Assignment #1: Noel Vargas and Richard Bateman\r\n"
        .BYT    0

text2   .BYT    "This example is only cool because it shows that we can load multiple\r\n"
        .BYT    " copies of this program into memory -- all at different addresses --\r\n"
        .BYT    " and everything still works.\r\n
        .BYT    0

;
; Begin code section
; 
; Conventions used for "functions":
;
; R1 - Address of statement to return to
; R2 - function parameter 1
;

; 
; Note: Trap 1 and 2 use R0
;       Trap 3 and 4 use R8
;

            JMP START           ; Jump to the "start" section of our code


;
; Function PSTR - Print string
;   Usage: Prints characters sequentially from the memory location until it finds a \0
; R1 - Address of the instruction to branch to when done
; R2 - Address to the first char in the string to print
;
PSTR        LDB R8, R2           ; Print String function; start by loading the start
                                ; char into R0 (*R2 -> R0)
            BRZ R8, PSTR_E      ; When the loaded char is 0, go to PSTR_E
            TRP 3               ; Print the character
            ADI R2, 1           ; Move R2 to the next byte
            JMP PSTR            ; Loop to PSTR

PSTR_E      JMR R1              ; Branch back to the calling point

; End Function PSTR


; 
; Begin Main code section
;


; From the instructions:
;   1) Print your name on the screen
START       LDA R2, text1
            LDA R1, text1Done
            JMP PSTR            ; Call PSTR(NameDone, LNAME)

text1Done   LDA R2, text2
            LDA R1, FINAL
            JMP PSTR

FINAL       TRP 0
