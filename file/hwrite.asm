;
; Test code for cache
; Ray Su
; 1st August, 2018
;

	addr_66	equ #$66
	addr_e6 equ #$e6
	addr_1e6 equ #$1e6
	addr_2e6 equ #$2e6
	addr_3e6 equ #$3e6
	
	org	#$100
Main
	movlz addr_66,R1 ;set R1 to target memory[66] address
	ld R1,R0
	add	#1,R0	;increment R0
	st R0,R1	;store R0 to memory[66]
	movlz addr_e6,R1
	st R0,R1	;store R0 to memory[e6]
	movlz addr_1e6,R1
	movh addr_1e6,R1
	st R0,R1	;store R0 to memory[e6]
	movlz addr_2e6,R1
	movh addr_2e6,R1
	st R0,R1	;store R0 to memory[e6]
	movlz addr_3e6,R1
	movh addr_3e6,R1
	st R0,R1	;store R0 to memory[e6]

	bal Main
	
	org #$66
	word	#$1234
	
	end	Main