#include <msp430g2452.h>

#include "serial.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */


void printf(char *format, ...);

int main(void)
{
    WDTCTL = (WDTPW + WDTHOLD);

    /* use precalibrated 1MHz value */
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1OUT = XMIT_PIN;  // default state of TX line is high
    P1DIR = XMIT_PIN;

    /**
     * configure serial routines for P1OUT, PIN1 9600 bps
     */
    serial_setup(XMIT_PIN, MCLK_FREQ/BAUD_RATE);

    printf("Hello my %d%c%c name is %s\r\n",1,'s','t',"Joe");
    while(1);

    return (0);
}
