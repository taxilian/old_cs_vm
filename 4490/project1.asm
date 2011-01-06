;
; project1.asm
;
; Richard Bateman
;

LISTA   .INT 1
        .INT 2
        .INT 3
        .INT 4
        .INT 5
        .INT 6
        .INT 0              ; NULL terminated array

LISTB   .INT 300
        .INT 150
        .INT 50
        .INT 20
        .INT 10
        .INT 5
        .INT 0              ; NULL terminated array

LISTC   .INT 500
        .INT 2
        .INT 5
        .INT 10
        .INT 0

LNAME   .BYT "Bateman, "    ; Lastname, Firstname
FNAME   .BYT "Richard"
NULL    .BYT 0
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
START       LDA R2, LNAME
            LDA R1, NameDone
            JMP PSTR            ; Call PSTR(NameDone, LNAME)

NameDone    LDR R8, CR
            TRP 3               ; Write \n to stdout
            LDR R8, LF
            TRP 3
;   2) Print a blank line.
            LDR R8, CR
            TRP 3               ; Write \n to stdout
            LDR R8, LF
            TRP 3

;   3) Add all the elements of list B together; print each result (intermediate and final)
;      on the screen.  Put 2 spaces between each result.

            LDA R2, LISTB       ; Load the address of LISTA into R2
            LDR R8, SPACE       ; Get a space ready to print
            SUB R0, R0          ; Zero out R0, which will contain our sum
STEP2       LDR R1, R2           ; Load the current item into R1
            BRZ R1, STEP2_END   ; When the current item is 0, we're done
            ADD R0, R1          ; Add the current entry to our sum
            TRP 1               ; Print out the current entry
            TRP 3
            TRP 3               ; Print out two spaces
            ADI R2, 4           ; Move to the next int item in the list
            JMP STEP2           ; Loop to STEP2

STEP2_END   LDR R8, CR
            TRP 3               ; Write \n to stdout
            LDR R8, LF
            TRP 3
            MOV R5, R0          ; Store the result from Step 3 into R5

;   4) Print a blank line.
            LDR R8, CR
            TRP 3               ; Write \n to stdout
            LDR R8, LF
            TRP 3

;   5) Multiply all the elements of list A together; print each result (intermediate and
;      final) on the screen.  Put 2 spaces between each result.

            LDA R2, LISTA       ; Load the address of LISTA into R2
            LDR R8, SPACE       ; Get a space ready to print
            SUB R0, R0          ; Zero out R0
            ADI R0, 4           ; Add 1 to R0; this will contain our product
STEP5       LDR R1, R2           ; Load the current item into R1
            BRZ R1, STEP5_END   ; When the current item is 0, we're done
            MUL R0, R1          ; Multiply the new entry to our existing product
            TRP 1               ; Print out the current entry
            TRP 3
            TRP 3               ; Print out two spaces
            ADI R2, 4           ; Move to the next int item in the list
            JMP STEP5           ; Loop to STEP5

STEP5_END   LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3
            MOV R6, R0          ; Store the result from step 5 in R6

;   6) Print a blank line.
            TRP 3               ; Write \n to stdout

;   7) Divide the final result from part 3 by each element in list B (the results are
;      *not cumulative*).  Put 2 spaces between each result.

            LDA R2, LISTB       ; Load the address of LISTB into R2
            LDR R8, SPACE       ; Get a space ready to print
STEP7       MOV R0, R5          ; Load the result from part 3 into R0
            LDR R1, R2           ; Load the current item into R1
            BRZ R1, STEP7_END   ; If we find a 0, we're done
            DIV R0, R1          ; R0 /= R1
            TRP 1
            TRP 3
            TRP 3
            ADI R2, 4
            JMP STEP7

STEP7_END   LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3

;   8) Print a blank line.
            LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3

;   9) Subtract from the final result of part 5 each element of list C (the results are
;      *not cumulative*).  Put 2 spaces between each result.

            LDA R2, LISTC
            LDR R8, SPACE
STEP9       MOV R0, R6
            LDR R1, R2
            BRZ R1, STEP9_END
            SUB R0, R1
            TRP 1
            TRP 3
            TRP 3
            ADI R2, 4
            JMP STEP9

STEP9_END   LDR R8, CR
            TRP 3
            LDR R8, LF
            TRP 3


FINAL       TRP 0
