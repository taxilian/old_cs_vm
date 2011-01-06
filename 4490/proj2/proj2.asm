;
; project2.asm
;
; Richard Bateman
;

; DATA for Part 1

VARSIZE .INT 10     ; SIZE variable for part 1
VARARR  .INT 10     ; arr array for part 1
        .INT 2
        .INT 3
        .INT 4
        .INT 15
        .INT -6
        .INT 7
        .INT 8
        .INT 9
        .INT 10

VARSUM  .INT 0
VARTEMP .INT 0
VARRES  .INT 0

SUMOUT  .BYT "Sum is "
        .BYT 0
SUMEVEN .BYT " is even\r\n"
        .BYT 0
SUMODD  .BYT " is odd\r\n"
        .BYT 0

; DATA for Part 2

CATS1   .BYT "C"
CATS2   .BYT "A"
CATS3   .BYT "T"
CATS4   .BYT "S"
        .BYT 0

; Generally useful DATA

SPACE   .BYT " "            ; used for printing a space
CR      .BYT "\r"           ; used for printing a blank line
LF      .BYT "\n"           ;


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
PSTR        LDB R8, R2          ; Print String function; start by loading the start
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


; Begin Part 1
START       SUB R12, R12
            SUB R6, R6          ; R6 will hold the intermediate sum
            ADI R12, 2          ; Initialize R12 to 2 for later operations
            SUB R9, R9          ; Make sure R9 is 0; this will be 1
            LDR R5, VARSIZE     ; Max value of i in integers
            MUL R5, R12
            MUL R5, R12         ; R5 *= 2*2 -- max value of i in bytes
            LDA R2, VARARR      ; Load the position of the array
PT1_LOOP    MOV R3, R9          ; Put i into R3 so that we can do a compare
            CMP R3, R5          ; R3 == R5?
            BRZ R3, PT1_DONE    ; If i == SIZE, goto PT1_DONE
            
            LDA R2, VARARR
            ADD R2, R9          ; R2 now points to the current item
            LDR R2, R2          ; R2 = MEMORY[R2]
            ADD R6, R2          ; sum += ARR[R2]

            MOV R2, R6
            DIV R2, R12
            MUL R2, R12
            SUB R2, R6
            BRZ R2, PT1_EVEN

PT1_ODD     MOV R0, R6
            LDA R2, SUMODD
            TRP 1
            LDA R1, PT1_COND
            JMP PSTR

PT1_EVEN    MOV R0, R6
            LDA R2, SUMEVEN
            TRP 1
            LDA R1, PT1_COND
            JMP PSTR

PT1_COND    ADI R9, 4           ; i += 4 (next int)
            JMP PT1_LOOP
            
PT1_DONE    LDA R2, SUMOUT
            LDA R1, PT1_SUM
            JMP PSTR
PT1_SUM     MOV R0, R6
            TRP 1
            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3

; Begin part two!

PT2_START   LDA R2, CATS1
            LDA R1, PT2_INT
            JMP PSTR            ; Print out the value CATS
PT2_INT     LDR R0, CATS1
            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3
            TRP 1               ; Print out the integer value of CATS
            MOV R5, R0          ; Store the integer value of CATS
            
            LDB R3, CATS1       ; Swap bytes 1 and 3
            LDB R4, CATS3
            STB R3, CATS3
            STB R4, CATS1

            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3

            LDA R2, CATS1       ; Print TACS
            LDA R1, PT2_COMP
            JMP PSTR

PT2_COMP    LDR R0, CATS1
            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3
            TRP 1               ; Print the integer value of TACS
            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3
            SUB R0, R5          ; R0 = TACS-CATS
            TRP 1
FINAL       TRP 0
