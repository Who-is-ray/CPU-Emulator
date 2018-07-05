;
; Example X-Makina program
; Assembling two string functions (strlen() and strcpy())
; Called by mainline
; L. Hughes
; 11 Jun 2018 - Second example of storing length using relative addressing
; 11 May 2018
;
STKTOP	equ	#$FFBE	; Top-of-stack - word below device vectors
NUL	equ	#'\0'	; End-of-string indicator
; 
r2	equ	R2	; r2 is alias of R2
LR	equ	R4	; LR is alias of R4
SP	equ	R5	; SP is alias of R5 
PC	equ	R7	; PC is alias of R7
dev0	equ #$0000
;
; Code space
	org	#$100
Main
; Initialize stack point
	movl	STKTOP,SP	; LSB to SP
	movh	STKTOP,SP	; MSB to SP
;
; R1 points to string
	movl	MyStr,R1
	movh	MyStr,R1
; Call strlen()
	bl	strlen	; R1 points to string; R0 returned with length
;
; Two examples of storing (R0, length, in MSLen):
;
; 1. Indexed using R1 (address of MyStr)
	sub	#2,R1	; Word before MyStr is MSLen
	st.w	R0,R1+	; Save R0 in MSLen
			; Bit of a cheat - increment R1 by 2 to point 
			; back to string MyStr
;
; 2. Relative using offset "back" from R1 to MSLen's location
	str.w	R0,R1,#-2; mem[R1-2] = R0
			; Note that R1 is unchanged
;
; Call strcpy()
; MyStr (R1) to Str2 (r2)
; Setup destination str in R2
	movl	Str2,r2
	movh	Str2,r2
;
	bl	strcpy	; LR holds return address
;
Done	bal	Done	; No halt or monitor to return to, loop forever
;
; String length function
; Input: R1 - Address of NUL-terminated string
; Returns: Length of string in R0
strlen
; 
	st	R1,-SP	; Push R1 (to restore on exit)
	st	R2,-SP	; Push R2 (holds character read)
	movlz	#0,R0	; Return length in R0 (initially zero)
	movl	dev0,R3
;
strlen1	
	ld.b	R1+,R2	; Copy byte [R1] to R2 and R1 incremented by 1
	movlz #0,R2
	swpb R2
	st	R2,R3
	cmp.b	NUL,R2	; Compare byte (in R2) with NUL
	beq	strlen2	; If equal, go to strlen2
	add	#1,R0	; R0 (length) incremented by 1
	bal	strlen1	; Check next byte
;
; Finished
strlen2	
	ld	SP+,R2	; Pull R2
	ld	SP+,R1	; Pull R1
	mov	LR,PC	; Return to caller.  R0 has length
;
; String copy function
; Input: R1 - src string; R2 - dst string
; R0 holds char - stop when NUL
; No check on length of dst string
; Returns: nothing
strcpy
	st	R0,-SP	; Push R0
	st	R1,-SP	; Push R1
	st	R2,-SP	; Push R2
;
strcpy1
	ld	R1+,R0	; ch = src[i++]
	st	R0,R2+	; dst[j++] = ch (includes NUL)
	cmp.b	NUL,R0	; ch = NUL?
	bne	strcpy1	; No - do next ch
;
; NUL found - restore registers and return
	ld	SP+,R2	; Pull R2
	ld	SP+,R1	; Pull R1
	ld	SP+,R0	; Pull R0
	mov	LR,PC	; return to caller
;
; Data space 
; Setting the origin could be unsafe - if code addresses extend past $1000, they
; could be overwritten by the data.  This is one argument for a linker.
;
	org	#$1000
;
MSLen	bss	#2	; Reserve 2 bytes (word) for length of MyStr
MyStr	byte	#'T'	; The assembler needs a string directive...
	byte	#'h'
	byte	#'e'
	byte	#' '
	byte	#'c'
	byte	#'a'
	byte	#'t'
	byte	NUL
;
Str2	bss	#100	; 100 bytes for destination string
;
	end	Main