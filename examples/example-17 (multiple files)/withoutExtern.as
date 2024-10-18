		.entry LIST
MAIN:	add r3, LIST
LOOP:	prn #48  
	                           
		macr m_macr
		cmp r3, #-6
		bne END
		endmacr
		lea STR, r6
		inc r6
		sub r1, r4 
		m_macr
		add r7, *r6
		clr K

		.entry MAIN
		jmp LOOP
END:	stop
STR: 	.string "abcd"
LIST: 	.data 6, -9
		.data -100
K: 		.data 31