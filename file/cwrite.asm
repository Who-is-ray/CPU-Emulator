;
; Test code for cache
; Ray Su
; 1st August, 2018
;

	addr_66	equ #$66
	addr_e6 equ #$e6
	
	org	#$100
Main
	movlz addr_66,R1 ;set R1 to target memory[66] address
	movlz addr_e6,R2
	ld R1,R0
	add	#1,R0	;increment R0
	st R0,R1	;store R0 to memory[66]
	st R0,R2	;store R0 to memory[e6]

	bal Main
	
	org #$66
	word	#$1234
	
	end	Main