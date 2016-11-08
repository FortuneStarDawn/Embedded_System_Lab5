#include <msp430.h>
#include <intrinsics.h>
#define LED1 BIT0
#define LED2 BIT6
#define B1 BIT3

volatile unsigned int mode=0, fast, count=0, now=0, last=0;

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT
    // TA1 trigger sample start
    ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_2;
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    __delay_cycles(1000);
    ADC10CTL0 &= ~ENC;    // ADC10 Enable

    P1DIR = LED1 + LED2;
    P1OUT = LED1 + B1;
    P1REN = B1;
    P1IE |= B1;
    P1IES |= B1;
    P1IFG &= ~B1;

    BCSCTL3 |= LFXT1S_2;
    TA0CCR0 = 374;
    TA0CCR1 = 373;
    TA0CCTL1 = OUTMOD_3;
	TA0CTL = MC_1|ID_3|TASSEL_1|TACLR; //1500 Hz

	TA1CCR0 = 449;
	TA1CCTL0 = CCIE;
	TA1CTL = MC_1|ID_3|TASSEL_1|TACLR;
	
    for(;;)
    {
    	fast = 0;
    	__bis_SR_register(LPM3_bits + GIE);
    	fast = 1;
    	__bis_SR_register(LPM0_bits + GIE);
    }
 }
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	if(count<3)
	{
		now += ADC10MEM;
		count++;
	}
	else
	{
		if(now > last) LPM3_EXIT;
		else LPM0_EXIT;
		last = now;
		now = 0;
		count = 0;
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	P1IFG &= ~B1;
	if(P1IES & B1) ADC10CTL0 |= ENC;
	else
	{
		if(mode==0)
		{
			mode = 1;
			P1OUT &= ~LED1;
			P1OUT |= LED2;
		}
		else
		{
			mode = 0;
			P1OUT &= ~LED2;
			P1OUT |= LED1;
		}
		TA1CCR0 = 449;
		TA1CTL |= TACLR;
		ADC10CTL0 &= ~ENC;
	}
	P1IES ^= B1;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void)
{
	if(mode==0) P1OUT ^= LED1;
	else P1OUT ^= LED2;
	if(TA1CCR0==449)
	{
		if(fast) TA1CCR0 = 299;
		else TA1CCR0 = 1049;
	}
	else TA1CCR0 = 449;
	TA1CTL |= TACLR;
}
