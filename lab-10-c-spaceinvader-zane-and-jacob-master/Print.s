; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
	PRESERVE8
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
Digit EQU 0
;-------ALLOCATION-----
		PUSH {R11, R4, R5,LR}
		SUBS SP, #4
		MOV R11, SP

;-------------SETUP--------------
		MOV R1, R0
		MOV R2, #0
		MOV R5, #10
;----------HOW MANY DIGITS---------
Continue
		ADD R2, #1 
		UDIV R1, R1, R5
		CMP R1, #0
		BNE Continue
		STR R2, [R11, #Digit]
;------PUSH ONTO STACK----------
		MOV R1, R0
		LDR R4, [R11, #Digit]
Repeat
		UDIV R2, R1, R5
		MUL R2, R2, R5
		SUB R3, R1, R2
		PUSH {R3}
		UDIV R1, R1, R5
		SUBS R4, R4, #1
		CMP R4, #0
		BNE Repeat
;---------

		LDR R4, [R11, #Digit]
ARepeat
		POP {R0}
		ADD R0, #48
		BL ST7735_OutChar
		SUBS R4, R4, #1
		CMP R4, #0
		BNE ARepeat
;-------DEALLOCATION------
		ADD SP, #4
		POP {R11, R4,R5,LR}

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
V1 EQU 0
V2 EQU 4
V3 EQU 8
V4 EQU 12
	
	PUSH{LR,R5,R6}
	SUB SP, #12
	MOV R5, #10
	MOV R6, #10000
	
;
	MOV R1, R0
	CMP R1, R6
	BHS Ast_Out
	
	UDIV R2, R1, R5
	MUL R2, R5
	SUBS R3, R1, R2
	STR R3, [SP, #V4]
	UDIV R1, R1, R5
	
	UDIV R2, R1, R5
	MUL R2, R5
	SUBS R3, R1, R2
	STR R3, [SP, #V3]
	UDIV R1, R1, R5
	
	UDIV R2, R1, R5
	MUL R2, R5
	SUBS R3, R1, R2
	STR R3, [SP, #V2]
	UDIV R1, R1, R5
	
	UDIV R2, R1, R5
	MUL R2, R5
	SUBS R3, R1, R2
	STR R3, [SP, #V1]
	UDIV R1, R1, R5
	
	LDR R0, [SP, #V1]
	ADD R0, #48
	BL ST7735_OutChar
	MOV R0, #46
	BL ST7735_OutChar
	
	LDR R0, [SP, #V2]
	ADD R0, #48
	BL ST7735_OutChar
	
	LDR R0, [SP, #V3]
	ADD R0, #48
	BL ST7735_OutChar
	
	LDR R0, [SP, #V4]
	ADD R0, #48
	BL ST7735_OutChar
	B Finish
Ast_Out
	MOV R0, #42
	BL ST7735_OutChar
	MOV R0, #46
	BL ST7735_OutChar
	MOV R0, #42
	BL ST7735_OutChar
	MOV R0, #42
	BL ST7735_OutChar
	MOV R0, #42
	BL ST7735_OutChar
Finish
	ADD SP, #12
	POP{LR,R5,R6}
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
