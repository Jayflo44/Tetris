	.arch msp430g2553
	.p2align 1,0
	.text


	.global turn_on_red
	.extern P1OUT

turn_on_red:	;red led on
	bis #64, &P1OUT		;red led on indicate cpu sleep state.
	pop r0

