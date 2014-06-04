#include <msp430g2452.h>

#include "serial.h"
#include "msp_iface.h"
#include "printf.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */

#define SAMPLES 50

int8_t xyz[SAMPLES*3];
unsigned int index = 0;
uint8_t data_ready = 0;
uint8_t button_press = 0;

int count = 0;

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT3){
        button_press = 1;
        P1IFG &= ~BIT3;
        P1OUT ^=  BIT0;
    }
    if(P1IFG & BIT4){
        ++count;
        P1IES ^= BIT4;
        P1IFG &= ~BIT4;
        data_ready = 1;
    }
    __bic_SR_register_on_exit(LPM3_bits); // exit active if prompted to
}

void setup_interrupts(void)
{
    P1SEL &= ~BIT4;
    P1DIR &= ~BIT4;
    P1IFG &= ~BIT4;
    P1IE  |=  BIT4;

    P1SEL &= ~BIT3;
    P1DIR &= ~BIT3;
    P1REN |=  BIT3;
    P1IES |=  BIT3;
    P1IFG &= ~BIT3;
    P1IE  |=  BIT3;

    _BIS_SR(GIE);
}

int main(void)
{
    WDTCTL = (WDTPW + WDTHOLD);

    /* use precalibrated 1MHz value */
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1OUT |= XMIT_PIN;  // default state of TX line is high
    P1DIR |= XMIT_PIN;
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    /**
     * configure serial routines for P1OUT, PIN1 9600 bps
     */
#ifdef DEBUG
    serial_setup(XMIT_PIN, MCLK_FREQ/BAUD_RATE);
#endif

    mma_init();
    uint8_t whoami = read_register(0x0d);
    printf("Whoami %x\r\n", whoami);
    mma_sysmod();
    printf("before\r\n");
    setup_interrupts();
    printf("after\r\n");

    read_xyz((uint8_t *)xyz);
    read_xyz((uint8_t *)xyz);

    while(1){
        if(data_ready){
            data_ready = 0;
            read_xyz((uint8_t *)&xyz[index*3]);
            ++index;
            if(index == SAMPLES) index = 0;
        }
        if(button_press){
            printf("Count: %d, Index: %d\r\n", count, index);
            button_press = 0;
        }
        if(!data_ready) _BIS_SR(LPM3_bits + GIE);
    }
    while(1);

    return (0);
}


/*
unsigned int curtime = 0;
unsigned int maxtime = 0;
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    if(++curtime == maxtime){
        P1OUT &= ~BIT0;
        _BIS_SR(LPM3_bits + GIE);
    }
}

void setup_timeout(unsigned int ms)
{
    curtime = 0;
    maxtime = ms>>5;
    WDTCTL = WDT_MDLY_32;
    IE1 |= WDTIE;
}
*/
