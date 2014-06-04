#include <msp430g2452.h>

#include "serial.h"
#include "msp_iface.h"
#include "printf.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */

#define SAMPLES 25
#define abs(x) ((x)<0? -(x) : (x))

int8_t train[SAMPLES*3];
int8_t test[SAMPLES*3];
uint8_t data_ready = 0;
int state = 0;
int thresh = 0;


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT3){
        P1IE  &= ~BIT3;
        P1IFG &= ~BIT3;
        __bic_SR_register_on_exit(LPM3_bits);
    }
    if(P1IFG & BIT4){
        P1IES ^= BIT4;
        P1IFG &= ~BIT4;
        __bic_SR_register_on_exit(LPM3_bits);
    }
}

void setup_interrupts(void)
{
    P1SEL &= ~BIT4;
    P1DIR &= ~BIT4;
    P1IFG &= ~BIT4;

    P1SEL &= ~BIT3;
    P1DIR &= ~BIT3;
    P1REN |=  BIT3;
    P1IES |=  BIT3;
    P1IFG &= ~BIT3;
    P1IE  |=  BIT3;

    _BIS_SR(GIE);
}

void sample_xyz(void);
void sample_xyz()
{
    uint8_t *buff = (uint8_t*)((state == 0)?train:test);
    int i;
    P1IE |= BIT4;
    mma_active();
    read_xyz((uint8_t *)buff);
    P1OUT |=  BIT0;
    for(i = 0; i < SAMPLES; ++i){
        read_xyz((uint8_t *)&(buff[i*3]));
        _BIS_SR(LPM3_bits + GIE);
    }
    P1IE &= ~BIT4;
    mma_standby();
    P1OUT &= ~BIT0;
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
    P1OUT &= ~BIT0;
    P1DIR |= BIT5;
    P1OUT &= ~BIT5;

    /**
     * configure serial routines for P1OUT, PIN1 9600 bps
     */
#ifdef DEBUG
    serial_setup(XMIT_PIN, MCLK_FREQ/BAUD_RATE);
#endif
    printf("Starting up gesture recognition...\r\n");

    mma_init();
    setup_interrupts();

    while(1){
        _BIS_SR(LPM3_bits + GIE);
        sample_xyz();
        int i;
        if(state > 0){
            int diff = 0;
            for(i = 0; i < SAMPLES*3; ++i){
                diff += abs(train[i] - test[i]);
            }
            if(state == 1){
                thresh = diff + (diff>>2);
                printf("Difference: %d, Threshold: %d\r\n", diff, thresh);
            }
            else if(diff < thresh){
                P1OUT |= BIT5;
                printf("Unlocked! %d\r\n", diff);
            }
            else{
                P1OUT &= ~BIT5;
                printf("DENIED: %d\r\n", diff);
            }
        }
        if(state < 2) ++state;
        P1IE  |=  BIT3;
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
