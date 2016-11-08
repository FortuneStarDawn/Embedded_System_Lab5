#include <msp430.h>
#include <intrinsics.h>
#define LED1 BIT6
#define LED2 BIT0
#define B1 BIT3

volatile unsigned int count=0, down=0, cycle=0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    P1DIR = LED1 + LED2;
    P1OUT = B1;
    P1REN |= B1;
    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;
    BCSCTL3 |= LFXT1S_2;
    TA0CCR0 = 449;
    TA1CCR0 = 149;
    TA0CCTL0 = CCIE;
    TA1CCTL0 = CCIE;
    TA0CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz
    TA1CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz
    __bis_SR_register(LPM3_bits + GIE);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void)
{
	TA1CTL &= ~MC_1;
	if(down==1)
	{
		if(count<6)
		{
			P1OUT ^= LED1;
			if(TA0CCR0==449) TA0CCR0 = 1049;
			else TA0CCR0 = 449;
			count++;
			if(count==6)
			{
				P1OUT &= ~LED1;
				P1OUT |= LED2;
				TA0CCR0 = 299;
				TA0CTL |= TACLR;
			}
		}
		else
		{
			P1OUT ^= LED2;
			if(TA0CCR0==299) TA0CCR0 = 1199;
			else TA0CCR0 = 299;
		}
	}
	TA0CTL |= TACLR;
	TA1CTL |= MC_1;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void)
{
	cycle++;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1 (void)
{
	TA1CTL &= ~MC_1;
	P1IFG &= ~B1;
	if(P1IES & B1)
	{
		P1IES &= ~B1;
		down=1;
		P1OUT |= LED1;
		TA0CCR0 = 449;
		TA0CTL |= TACLR;
		cycle = 0;
	}
	else
	{
		P1IES |= B1;
		down=0;
		count=0;
		P1OUT &= ~LED1;
		P1OUT &= ~LED2;
	}
	TA1CTL |= MC_1;
}
