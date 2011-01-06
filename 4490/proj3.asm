;
; project3.asm
;
; Richard Bateman
;

; DATA

SIZE        .INT 7
C_ARR       .BYT "zzzzzzz"
cnt         .INT 123
tenth       .INT 123
data        .INT 123
flag        .INT 123
opdv        .INT 123

SPACE       .BYT " "            ; used for printing a space
CR          .BYT "\r"           ; used for printing a blank line
LF          .BYT "\n"           ;
PLUS        .BYT "+"
MINUS       .BYT "-"

TooBig      .BYT "Number too Big\n"
            .BYT 0

IsNaN       .BYT " is not a number\n"
            .BYT 0

OperandIs   .BYT "Operand is "
            .BYT 0

K_ZERO      .BYT "0"
K_ONE       .BYT "1"
K_TWO       .BYT "2"
K_THREE     .BYT "3"
K_FOUR      .BYT "4"
K_FIVE      .BYT "5"
K_SIX       .BYT "6"
K_SEVEN     .BYT "7"
K_EIGHT     .BYT "8"
K_NINE      .BYT "9"

K_ATSIGN    .BYT "@"

;
; Begin code section
; 
; Conventions used for "functions":
;
; R1 - Address of statement to return to
; R2 - function parameter 1
; R3 - function parameter 2
; R4 - function parameter 3
; R5 - function parameter 4
; R9-R15 - local variables
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





; void opd(char s, int k, char j)
; map: s -> R2
;      k -> R3
;      j -> R4
; {
;   int t = 0;
opd         SUB R9, R9      ; t = 0
;   if (j == '0')
opd_chk0    LDB R10, K_ZERO
            CMP R10, R4
            BNZ R10, opd_chk1
;       j = 0;
            ;ADI R9, 0      ; Here for consistency, unneeded
            JMP opd_done

;   else if (j == '1')
opd_chk1    LDB R10, K_ONE
            CMP R10, R4
            BNZ R10, opd_chk2
;       j = 1;
            ADI R9, 1
            JMP opd_done

;   else if (j == '2')
opd_chk2    LDB R10, K_TWO
            CMP R10, R4
            BNZ R10, opd_chk3
;       j = 2;
            ADI R9, 2
            JMP opd_done

;   else if (j == '3')
opd_chk3    LDB R10, K_THREE
            CMP R10, R4
            BNZ R10, opd_chk4
;       j = 3;
            ADI R9, 3
            JMP opd_done

;   else if (j == '4')
opd_chk4    LDB R10, K_FOUR
            CMP R10, R4
            BNZ R10, opd_chk5
;       j = 4;
            ADI R9, 4
            JMP opd_done

;   else if (j == '5')
opd_chk5    LDB R10, K_FIVE
            CMP R10, R4
            BNZ R10, opd_chk6
;       j = 5;
            ADI R9, 5
            JMP opd_done

;   else if (j == '6')
opd_chk6    LDB R10, K_SIX
            CMP R10, R4
            BNZ R10, opd_chk7
;       j = 6;
            ADI R9, 6
            JMP opd_done

;   else if (j == '7')
opd_chk7    LDB R10, K_SEVEN
            CMP R10, R4
            BNZ R10, opd_chk8
;       j = 7;
            ADI R9, 7
            JMP opd_done

;   else if (j == '8')
opd_chk8    LDB R10, K_EIGHT
            CMP R10, R4
            BNZ R10, opd_chk9
;       j = 8;
            ADI R9, 8
            JMP opd_done

;   else if (j == '9')
opd_chk9    LDB R10, K_NINE
            CMP R10, R4
            BNZ R10, opd_else
;       j = 9;
            ADI R9, 9
            JMP opd_done

;   else {
;       flag = 1;
;       printf("%c is not a number\n", j);
opd_else    ADI R9, 1
            STR R9, flag
            MOV R8, R4
            TRP 3               ; print %c
            MOV R10, R1
            MOV R11, R2
            LDA R2, IsNaN
            MOV R1, PC
            ADI R1, 8
            JMP PSTR            ; print " is not a number\n"
            MOV R1, R10
            MOV R2, R11
;   }
;   if (!flag) {
opd_done    LDR R10, flag
            BGT R10, opd_finish
;       if (s == '+') {
            LDB R10, PLUS
            CMP R10, R2
            BNZ R10, opd_neg

            MUL R9, R3
            JMP opd_store

;       } else {
opd_neg     MOV R10, R3
            SUB R3, R10
            SUB R3, R10         ; k *= -1
;           t *= -k;
            MUL R9, R3
;       }
;       opdv += t;
opd_store   LDR R10, opdv
            ADD R9, R10
            STR R9, opdv
;   }

opd_finish  JMR R1
; }





; void reset(int w, int x, int y, int z) {
; map: w -> R2
;      x -> R3
;      y -> R4
;      z -> R5
;   for (int (R9)k = 0; k < (R10)SIZE; k++) {
reset       SUB R9, R9      ; k = 0
            SUB R12, R12    ; const char NULL = 0;
reset_lp    LDR R10, SIZE   ; k < SIZE
            CMP R10, R9
            BRZ R10, reset_lpdn
;       c[k] = 0;
            LDA R11, C_ARR
            ADD R11, R9
            STB R12, R11    ; c[k] = 0;
            ADI R9, 1       ; k++
            JMP reset_lp
;   }
reset_lpdn  STR R2, data    ; data = w
            STR R3, opdv    ; opdv = x
            STR R4, cnt     ; cnt = y
            STR R5, flag    ; flag = z
            JMR R1          ; return;
; }





; void flush() {
;   data = 0;
flush       SUB R9, R9
            STR R9, data
            LDB R9, LF
;   c[0] = getchar();
;   while (c[0] != '\n') {
;       c[0] = getchar();
flush_lp    TRP 4
            CMP R8, R9
            BNZ R8, flush_lp
            STB R9, C_ARR   ; c[0] = "\n"
            JMR R1          ; return
;   }
; }





; void getdata() {
;   if (cnt < SIZE) {
getdata     LDR R9, cnt
            LDR R10, SIZE
            CMP R10, R9     ; SIZE >= cnt
            BLT R10, getdata_tb
;       c[cnt] = getchar();
            TRP 4           ; getchar()
            LDA R10, C_ARR
            ADD R10, R9
            STB R8, R10     ; c[cnt] = char gotten
;       cnt++;
            ADI R9, 1
            STR R9, cnt
            JMP getdata_dn
;   } else {
;       printf("Number too Big\n");
getdata_tb  LDA R2, TooBig
            MOV R10, R1     ; Save previous R1
            MOV R1, PC
            ADI R1, 8
            JMP PSTR
;       flush();
            MOV R1, PC
            ADI R1, 8
            JMP flush       ; flush()
            MOV R1, R10     ; Load previous R1
;   }
getdata_dn  JMR R1
; }





; void main() {
START       MOV
;   reset(1, 0, 0, 0); // reset globals
            SUB R2, R2      ; set the parameters
            ADI R2, 1
            SUB R3, R3
            SUB R4, R4
            SUB R5, R5
            MOV R1, PC
            ADI R1, 8
            JMP reset       ; call function
;   getdata();
reset_rt    MOV R1, PC
            ADI R1, 8
            JMP getdata     ; call function

;   while (c[0] != '@') {
WHILENOAT   LDB R9, C_ARR
            LDB R10, K_ATSIGN
            CMP R9, R10
            BRZ R9, FINAL   ; If c[0] == "@" jump to end
;       if (c[0] == "+" || c[0] == "-") {
            LDB R9, C_ARR
            LDB R10, PLUS
            LDB R11, MINUS
            CMP R10, R9
            BRZ R10, signexist
            CMP R11, R9
            BRZ R11, signexist
            JMP nosign
;           getdata();
signexist   MOV R1, PC
            ADI R1, 8
            JMP getdata
            JMP WHILEDATA
;       } else { // default sign is '+'
;           c[1] = c[0]
nosign      LDA R10, C_ARR
            ADI R10, 1
            STB R9, R10
;           c[0] = '+';
            LDB R9, PLUS
            STB R9, C_ARR
;           cnt++;
            LDR R9, cnt
            ADI R9, 1
            STR R9, cnt
;       }
;       while(data) {
WHILEDATA   LDR R9, data
            BRZ R9, ENDWHDATA
;           if (c[cnt-1] == '\n') {
            LDR R9, cnt
            ADI R9, -1
            LDA R10, C_ARR
            ADD R9, R10
            LDB R9, R9
            LDB R10, LF
            CMP R10, R9
            BNZ R10, MN_nolf
;               data = 0;
            SUB R9, R9
            STR R9, data
;               tenth = 1;
            ADI R9, 1
            STR R9, tenth
;               cnt -= 2;
            LDR R9, cnt
            ADI R9, -2
            STR R9, cnt

;               while (!flag && cnt != 0) {
WHILENOFLAG LDR R9, cnt
            BRZ R9, ENDWHNOFLAG
            LDR R10, flag
            BNZ R10, ENDWHNOFLAG
;                   opd(c[0], tenth, c[cnt]);
            LDB R2, C_ARR           ; load s = c[0]
            LDR R3, tenth           ; load k = tenth
            LDA R4, C_ARR           ; get addr of array
            ADD R4, R9              ; addr + cnt = c[cnt]
            LDB R4, R4              ; load j = c[cnt]
            MOV R1, PC
            ADI R1, 8
            JMP opd
;                   cnt--;
            LDR R9, cnt
            ADI R9, -1
            STR R9, cnt
;                   tenth *= 10;
            SUB R10, R10
            ADI R10, 10
            LDR R11, tenth
            MUL R11, R10
            STR R11, tenth
            JMP WHILENOFLAG
;               }
;               if (!flag) // good number entered
ENDWHNOFLAG LDR R9, flag
            BNZ R9, WHDATA_LOOP
;                   printf("Operand is %d\n", opdv);
            LDA R2, OperandIs
            MOV R1, PC
            ADI R1, 8
            JMP PSTR
            LDR R0, opdv
            TRP 1
            LDB R8, LF
            TRP 3
            JMP WHDATA_LOOP
;           } else {
;               getdata();
MN_nolf     MOV R1, PC
            ADI R1, 8
            JMP getdata
;           }
WHDATA_LOOP JMP WHILEDATA
;       }
ENDWHDATA   SUB R2, R2      ; set the parameters
            ADI R2, 1
            SUB R3, R3
            SUB R4, R4
            SUB R5, R5
            MOV R1, PC
            ADI R1, 8
            JMP reset       ; call function
;   getdata();
            MOV R1, PC
            ADI R1, 8
            JMP getdata     ; call function
;   }
;   reset(1, 0, 0, 0); // reset globals
ENDWHNOAT   JMP WHILENOAT
; }
FINAL       TRP 0
