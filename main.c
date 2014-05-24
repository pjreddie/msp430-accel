/*
 * Copyright (c) 2012 Sebastian Trahm
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <msp430g2452.h>
#include "board.h"

#define PWM_PERIOD	65535
#define PWM_DUTY_CYCLE	6080



void
ISR_Timer0_A0(void)
{
	P1OUT |= GRN_LED;
}

void
ISR_Timer0_A1(void)
{
	/* Clear Interrupt Flag and LED */
	CCTL1 &= ~CCIFG;
	P1OUT &= ~GRN_LED;
}

int
main(void)
{
	/*
	 * See Section 10.3 "Watchdog Timer+ Registers of the
	 * MSP430x2xx Family User's Guide
	 *
	 * Enforce Watchdog Timer Hold
	 */
	WDTCTL = (WDTPW + WDTHOLD);

	/*
	 * See Section 12.3.1 TACTL, Timer_A Control Register
	 *
	 * Configure Timer_A to run with SMCLK (TASSEL2), Clock
	 * Divider /8 (ID_3) in Count Up Mode (MC_1).
	 * Interrupt issued when TACCR0/TACCR1 is reached.
	 */
	TACTL |= (TASSEL_2 + ID_3 + TACLR);
	CCR0 = PWM_PERIOD;
	CCR1 = PWM_DUTY_CYCLE;
	CCTL0 |= CCIE;
	CCTL1 |= CCIE;
	TACTL |= MC_1;

	P1DIR |= GRN_LED;
	P1OUT |= GRN_LED;

	__enable_interrupt();

	while(1) {
		/*  all work is done in the ISR */
	}

	/* never reached */
	return (0);
}
