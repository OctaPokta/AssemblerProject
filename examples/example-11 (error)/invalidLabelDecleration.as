        .entry LIST 
  	    .extern fn1
3AIN:	add r3, LIST
        jsr fn1
L%OOP	prn #48
        lea STR, r6
        inc r6
        mov *r6, L3
        sub r1, r4
        cmp r3, #-6
        bne END 
        add r7, *r6
        clr K 
        sub L3, L3
        .entry MAIN
        jmp LOOP
5ND:    stop
STR:    .string "abcd"
LIST:   .data 6,-9
        .data -100
K:      .data 31
.extern L3
	
