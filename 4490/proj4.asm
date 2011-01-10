;
; project4.asm
;
; Richard Bateman
;

; DATA

INTARR      .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
            .INT 0
INTARRLEN   .INT 30
COUNT       .INT 0

SPACE       .BYT " "            ; used for printing a space
LF          .BYT "\n"           ; used to print a blank line

TooBig      .BYT "Number too Big\n"
            .BYT 0

IsNaN       .BYT " is not a number\n"
            .BYT 0

FactPrompt  .BYT "Enter the next number (0 to stop): "
            .BYT 0

FactResult  .BYT "! is "
            .BYT 0

OperandIs   .BYT "Operand is "
NULL        .BYT 0

CommaSpace  .BYT ", "
            .BYT 0


CODE_START  MOV R2, SP      ; R2 = Stack top
            MOV R3, SP      ; R3 = Stack top
            ADI R2, -8      ; R2 = PFP location
            STR FP, R2      ; Set PFP
            ADI R2, -8      ; New top of stack
            MOV SP, R2      ; Stack PTR = R2
            MOV FP, R3      ; Frame PTR = old Stack PTR
            MOV R1, PC      ; Get addr of next instruction
            ADI R1, 24      ; Add 3 instructions from this one
            STR R1, FP      ; Set return address
            JMP Main        ; Call function Main

            TRP 0           ; End program

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


; int Factorial(i)
Factorial   MOV R5, FP
            ADI R5, -8
            LDR R1, R5      ; Param 1

            SUB R2, R2      ; R2 = 0
            ADI R2, 1       ; R2 = 1
            CMP R2, R1      ; Compare R1 == 0? 
            BRZ R2, Fact_done

            ADI R1, -1
            MOV R2, SP
            MOV R3, SP
            ADI R2, -8
            STR FP, R2      ; Set PFP
            ADI R2, -8
            STR R1, R2      ; Set first parameter
            ADI R2, -8
            MOV SP, R2      ; Stack PTR = R2
            MOV FP, R3      ; Frame PTR = old Stack PTR
            MOV R1, PC
            ADI R1, 24
            STR R1, FP      ; Set return value
            JMP Factorial   ; Call function
            
            LDR R1, SP      ; R1 = Get return value
            MOV R5, FP
            ADI R5, -8
            LDR R2, R5      ; R2 = get Param 1
            MUL R1, R2
            JMP Fact_done

Fact_done   LDR R0, FP
            STR R1, FP
            MOV R5, FP
            ADI R5, -8
            MOV SP, FP
            LDR FP, R5
            JMR R0          ; Jump to return address



; void printFact()
printFact   LDA R9, NULL

            SUB R5, R5
            ADI R5, 8

            LDA R13, INTARR
            LDR R14, COUNT  ; R14 == end of array
            ADI R14, -1
            MUL R14, R5
            ADD R14, R13

print_start MOV R5, R13
            CMP R5, R14             ; Compare the pointers

            BGT R5, printfact_e     ; if R13 > R14, end

            MOV R2, R9              ; Print the ", " except on first time
            MOV R1, PC
            ADI R1, 16
            JMP PSTR
            LDA R9, CommaSpace

            LDR R0, R13             ; print Array[i]
            TRP 1

            MOV R2, R9              ; Print the ", "
            MOV R1, PC
            ADI R1, 16
            JMP PSTR

            LDR R0, R14             ; print Array[CNT-1-i]
            TRP 1

            ADI R13, 8              ; move the pointers
            ADI R14, -8
            JMP print_start

printfact_e LDR R8, LF              ; print EOL
            TRP 3

            LDR R0, FP      ; Get return address
            MOV R5, FP      
            ADI R5, -8
            MOV SP, FP      ; SP = FP
            LDR FP, R5      ; FP = PFP
            JMR R0          ; Jump to return address



; int Main()
Main        SUB R0, R0
            ADI R0, 9


FactLoop    LDA R2, FactPrompt
            MOV R1, PC
            ADI R1, 16
            JMP PSTR
            
            TRP 2           ; Get the factorial value into R0

            SUB R5, R5
            CMP R5, R0      ; input == 0?
            BRZ R5, FactLoop_e

            STR R0, SP      ; Store a temporary variable on the stack
            ADI SP, -8      ; Move the stack pointer

            MOV R2, SP      ; R2 = Stack top
            MOV R3, SP      ; R3 = Stack top
            ADI R2, -8      ; R2 = PFP location
            STR FP, R2      ; Set PFP
            ADI R2, -8      ; Move to param 1 location
            STR R0, R2      ; Set param 1
            ADI R2, -8      ; New top of stack
            MOV SP, R2      ; Stack PTR = R2
            MOV FP, R3      ; Frame PTR = old Stack PTR
            MOV R1, PC      ; Get addr of next instruction
            ADI R1, 24      ; Add 3 instructions from this one
            STR R1, FP      ; Set return value
            JMP Factorial   ; Call function Factorial

            LDR R5, SP      ; Get return value into R5
            
            MOV R3, SP
            ADI R3, 8       ; Get the last (only) temporary variable
            LDR R0, R3      ; Read from memory
            TRP 1           ; Print input
            LDA R2, FactResult
            MOV R1, PC
            ADI R1, 16
            JMP PSTR        ; Print "! is "
            MOV R0, R5
            TRP 1           ; Print output
            LDB R8, LF
            TRP 3

            LDA R1, INTARR  ; R1 = &INTARR
            LDR R2, COUNT   ; R2 = count
            SUB R4, R4
            ADI R4, 4
            MUL R2, R4       ; (4 bytes per item)
            ADD R1, R2      ; R1 = &INTARR[count]
            MOV R3, SP
            ADI R3, 8       ; Get the last (only) temporary variable
            LDR R3, R3      ; Read from memory
            STR R3, R1      ; INTARR[count] = input
            ADI R1, 8
            STR R0, R1      ; INTARR[count+1] = result
            LDR R2, COUNT
            ADI R2, 2       ; count += 2
            STR R2, COUNT
            JMP FactLoop

FactLoop_e  MOV R2, SP      ; R2 = Stack top
            MOV R3, SP      ; R3 = Stack top
            ADI R2, -8      ; R2 = PFP location
            STR FP, R2      ; Set PFP
            ADI R2, -8      ; New top of stack
            MOV SP, R2      ; Stack PTR = R2
            MOV FP, R3      ; Frame PTR = old Stack PTR
            MOV R1, PC      ; Get addr of next instruction
            ADI R1, 24      ; Add 3 instructions from this one
            STR R1, FP      ; Set return address
            JMP printFact   ; Call function printFact

            ; Call the thread test function
            MOV R2, SP      ; R2 = Stack top
            MOV R3, SP      ; R3 = Stack top
            ADI R2, -8      ; R2 = PFP location
            STR FP, R2      ; Set PFP
            ADI R2, -8      ; New top of stack
            MOV SP, R2      ; Stack PTR = R2
            MOV FP, R3      ; Frame PTR = old Stack PTR
            MOV R1, PC      ; Get addr of next instruction
            ADI R1, 24      ; Add 3 instructions from this one
            STR R1, FP      ; Set return address
            JMP ThreadTest  ; Call function ThreadTest

Main_end    LDR R0, FP      ; Get return address
            MOV R5, FP      
            ADI R5, -8
            MOV SP, FP      ; SP = FP
            LDR FP, R5      ; FP = PFP
            JMR R0          ; Jump to return address

; Thread test

SHARED_INT  .INT 0

TXT_expl1   .BYT "\n\nWe will start the same function on 10 different threads. \n"
            .BYT "This function will read from a shared memory location, add 1,\n"
            .BYT "and then write the result to memory.  It will continue this \n"
            .BYT "until the shared memory count reaches 100, printing out the \n"
            .BYT "thread ID and current count at each iteration. NO LOCKING\n\n"
            .BYT 0

TXT_expl2   .BYT "\n\nWe will start the same function on 10 different threads. \n"
            .BYT "This function will read from a shared memory location, add 1,\n"
            .BYT "and then write the result to memory.  It will continue this \n"
            .BYT "until the shared memory count reaches 100, printing out the \n"
            .BYT "thread ID and current count at each iteration. This time we\n"
            .BYT "will use locking on both data and output (seperate locks)\n\n"
            .BYT 0

TXT_start   .BYT "Starting thread "
            .BYT 0

LCK_DATA    .BYT -1
LCK_OUTPUT  .BYT -1

colon       .BYT ":"

ThreadTest  SUB R1, R1
            STR R1, SHARED_INT

            LDA R2, TXT_expl1
            MOV R1, PC
            ADI R1, 16
            JMP PSTR

            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock
            RUN R6, tf_nolock

            BLK
            SUB R1, R1
            STR R1, SHARED_INT

            LDA R2, TXT_expl2
            MOV R1, PC
            ADI R1, 16
            JMP PSTR

            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock
            RUN R6, tf_lock

            BLK

            LDR R0, FP      ; Get return address
            MOV R5, FP      
            ADI R5, -8
            MOV SP, FP      ; SP = FP
            LDR FP, R5      ; FP = PFP
            JMR R0          ; Jump to return address


; tf_nolock - non-locking thread function
tf_nolock   SUB R9, R9
            ADI R9, 100
            LDA R2, TXT_start
            
            MOV R1, PC
            ADI R1, 16
            JMP PSTR

            MOV R0, R6      ; Print the starting thread message
            TRP 1
            LDB R8, LF
            TRP 3

tf_nl_do    LDR R1, SHARED_INT  ; Load the int
            MOV R2, R1
            CMP R2, R9
            BGT R2, tf_nl_end
            ADI R1, 1
            STR R1, SHARED_INT  ; Save the result

            MOV R0, R6
            TRP 1
            LDB R8, colon
            TRP 3
            MOV R0, R1
            TRP 1
            LDB R8, LF
            TRP 3

            JMP tf_nl_do

tf_nl_end   END

; tf_lock - locking thread function
tf_lock     SUB R9, R9
            ADI R9, 100
            LDA R2, TXT_start
            
            LCK LCK_OUTPUT  ; Start Mutex
            
            MOV R1, PC
            ADI R1, 16
            JMP PSTR

            MOV R0, R6      ; Print the starting thread message
            TRP 1
            LDB R8, LF
            TRP 3

            ULK LCK_OUTPUT  ; Close Mutex

tf_l_do     LCK LCK_DATA
            LDR R1, SHARED_INT  ; Load the int
            MOV R2, R1
            CMP R2, R9
            BGT R2, tf_l_end
            ADI R1, 1
            STR R1, SHARED_INT  ; Save the result
            ULK LCK_DATA

            MOV R0, R6
            LCK LCK_OUTPUT
            TRP 1
            LDB R8, colon
            TRP 3
            MOV R0, R1
            TRP 1
            LDB R8, LF
            TRP 3
            ULK LCK_OUTPUT

            JMP tf_l_do

tf_l_end    ULK LCK_DATA
            END
