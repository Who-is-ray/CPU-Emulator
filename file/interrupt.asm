	dev0 		equ #$0000
	dev0_psw	equ #$0060	;priority of device 0 is 3, IE is set
	dev0_vec	equ	#$FFC0
	isr0_addr	equ #$FFC2	;address of ISR0
	dev4		equ #$0008
	dev4_psw	equ #$0080	;priority of device 1 is 4, IE is unset
	dev4_vec	equ #$FFD0
	isr4_addr	equ #$FFD2	;address of ISR2
	comp_psw	equ	#$0020	;priority of computer is 1
	PSW			equ	R6		;PSW is alias of R6
	LR			equ	R4	; LR is alias of R4
	SP			equ	R5	; SP is alias of R5 
	PC			equ	R7	; PC is alias of R7
	STKTOP		equ	#$FFBE	; Top-of-stack - word below device vectors
	org	#$500

Start	movl comp_psw,PSW		;set computer priority equal to 1
	
	; Initialize stack point
	movl	STKTOP,SP	; LSB to SP
	movh	STKTOP,SP	; MSB to SP
	;turn on the PSW.IE of device 1
	movl dev0,R0		
	movh dev0,R0
	ld R0,R1
	add #1,R1
	st R1,R0
	
wait
	bal wait
		
	org #$2000
isr0
	movl dev0,R0		;set device 1 address
	movh dev0,R0
	;movl dev4,R1		;set device 4 address
	;movh dev4,R1
	ldr.b	R0,#1,R2
	;ld.b R1,R2
	str.b	R2,R0,#9			;load data in dev0 to dev 4
	mov LR,PC
;
	org #$ffc0
	word #$0060
	word isr0
	
	org #$ffd0
	word #$80
	
	end	Start
