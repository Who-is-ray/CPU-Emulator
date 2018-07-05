;
; Sample X-Makina program
; Initialize a block of memory to 'A' through 'Z'
; L. Hughes
; 18 05 15: Comment correct R1 = 'Z'
; 18 05 01: Initial release
;
SIZE	equ	#26
CAP_A	equ	#'A'
CAP_Z	equ	#'Z'
; Start of data area
	org	#0
Base	bss	SIZE		; Reserve SIZE bytes
; Start of code area
	org	#$100
Start	movlz	CAP_A,R0	; R0 = 'A'
	movlz	CAP_Z,R1		; R1 = 'Z'
	movlz	Base,R2		; R2 = Base (Base address for characters
; 
Loop
	st.b	R0,R2+		; [R2] = R0; R2 = R2 + 1
	cmp.b	R0,R1		; R0 = R1 ('Z')
	beq	Done		; Yes: Goto Done
	add.b	#1,R0		; No: R0 = R0 + 1 (next ASCII char)
	bal	Loop		; Repeat loop
; End of program
Done	bal	Done		; Infinite loop to "stop" the program
;
;	end	Start		; End of program - first executable address is "Start"
