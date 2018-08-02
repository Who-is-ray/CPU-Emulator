;
; Test code for cache
; Ray Su
; 1st August, 2018
;

	org	#$100
Main
	add	#1,R0	;increment R0
	cmp	#2,R0	;test is R0 = 2?
	beq Func	;go func if equal
	bal Main
	
	org #$200
Func
	movlz	#0,R0	;clear R0
	movlz   #1,R1	;set R1 to 1
	movlz	#2,R2	;set R2 to 2
	bal wait
	
	org #$500
wait
	bal wait		;finished
	
	end	Main