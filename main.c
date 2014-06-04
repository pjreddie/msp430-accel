#include <msp430g2452.h>

#include "serial.h"
#include "msp_iface.h"
#include "printf.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */

#define SAMPLES 25
#define SIZE 15
#define abs(x) ((x)<0? -(x) : (x))

int8_t train[SAMPLES*3];
int8_t test[SAMPLES*3];
unsigned int thresh = 0;
unsigned int state = 0;

void setup_timeout(unsigned int ms);
void setup_interrupts(void);
void Port_1(void);
void watchdog_timer(void);
unsigned int best_overlap(void);


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT3){
        P1IE  &= ~BIT3;
        __bic_SR_register_on_exit(LPM4_bits);
    }
    if(P1IFG & BIT4){
        P1IES ^= BIT4;
        P1IFG &= ~BIT4;
        __bic_SR_register_on_exit(LPM4_bits);
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

    _BIS_SR(GIE);
}

void sample_xyz(void);
void sample_xyz()
{
    uint8_t *buff = (uint8_t*)((state == 0)?train:test);
    int i;
    P1OUT |=  BIT0;
    P1IE |= BIT4;
    mma_active();
    read_xyz((uint8_t *)buff);
    for(i = 0; i < SAMPLES*3; i+=3){
        read_xyz((uint8_t *)&(buff[i]));
        _BIS_SR(LPM4_bits + GIE);
    }
    P1IE &= ~BIT4;
    mma_standby();
    P1OUT &= ~BIT0;
}

unsigned int best_overlap()
{
    int i,j;
    unsigned int best = 1000;
    int train_start = 3*((SAMPLES>>1) - (SIZE>>1));
    for(i = 0; i < (SAMPLES-SIZE)*3; i+=3){
        unsigned int curr = 0;
        for(j = 0; j < SIZE*3; ++j){
            curr += abs(train[train_start+j] - test[i+j]);
        }
        if(curr < best) best = curr;
    }
    return best;
}

int main(void)
{
    WDTCTL = (WDTPW + WDTHOLD);

    /* use precalibrated 1MHz value */
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1OUT |= XMIT_PIN;  // default state of TX line is high
    P1DIR |= (XMIT_PIN|BIT0);
    P1OUT &= ~BIT0;

    P2DIR |= (BIT0|BIT1);
    P2OUT &= ~(BIT0|BIT1);

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
        P1IFG &= ~BIT3;
        P1IE  |=  BIT3;
        _BIS_SR(LPM4_bits + GIE);
        sample_xyz();

        /*
        int i;
        uint8_t *buff = (uint8_t*)((state == 0)?train:test);
        for(i = 3; i < SAMPLES*3; ++i){
            buff[i-3] = buff[i] - buff[i-3];
        }
        */

        if(state > 0){
            unsigned int diff = best_overlap();
            if(state == 1){
                thresh = diff + (diff>>2) + (diff>>3);
                printf("Threshold Calculated: %d\r\n", thresh);
            }
            else if(diff < thresh){
                P2OUT |= BIT1;
                printf("Unlocked! %d\r\n", diff);
                setup_timeout(500);
                LPM0;
            }
            else{
                P2OUT |= BIT0;
                printf("DENIED: %d\r\n", diff);
                setup_timeout(500);
                LPM0;
            }
        }else{
            printf("Training Sample Saved\r\n");
        }
        if(state < 2) ++state;
    }
    while(1);

    return (0);
}


unsigned int time = 0;
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    if(!--time){
        P2OUT &= ~(BIT1|BIT0);
        WDTCTL = (WDTPW + WDTHOLD);
        __bic_SR_register_on_exit(LPM0_bits);
    }
}

void setup_timeout(unsigned int ms)
{
    time = ms>>5;
    WDTCTL = WDT_MDLY_32;
    IE1 |= WDTIE;
}
