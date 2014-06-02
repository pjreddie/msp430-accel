#include <msp430g2452.h>

#include "serial.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */

void printf(char *format, ...);
#ifndef DEBUG
void printf(char *format, ...){}
#endif

int8_t xyz[] = {0,0,0};
uint8_t data_ready = 0;
uint8_t button_press = 0;

void write_register(uint16_t reg, uint16_t val)
{
    uint16_t msg[] = {0x3a, 0, 0};
    msg[1] = reg;
    msg[2] = val;
    i2c_send_sequence(msg, 3, 0, LPM0_bits);
    LPM0;
    while(!i2c_done());
}

uint8_t read_register(uint16_t reg)
{
    uint8_t val = 0;
    uint16_t msg[] = {0x3a, 0, I2C_RESTART, 0x3b, I2C_READ};
    msg[1] = reg;
    i2c_send_sequence(msg, 5, &val, LPM0_bits);
    LPM0;
    while(!i2c_done());
    return val;
}

void read_xyz(void)
{
    uint16_t msg[] =
    {0x3a, 0x01, I2C_RESTART, 0x3b, I2C_READ, I2C_READ, I2C_READ, I2C_READ};
    i2c_send_sequence(msg, 8, xyz, LPM0_bits);
    LPM0;
    while(!i2c_done());
    //printf("%d %d %d\r\n",xyz[0], xyz[1], xyz[2]);
}

void mma_status()
{
    uint8_t status = read_register(0x0);
    printf("Status: %x\r\n", status);
}

void mma_sysmod()
{
    uint8_t status = read_register(0x0b);
    printf("Sys Mod: %x\r\n", status);
}

void mma_standby(void)
{
    uint8_t c = read_register(0x2a);
    write_register(0x2a, c&~0x1);
}

void mma_active(void)
{
    uint8_t c = read_register(0x2a);
    write_register(0x2a, c|0x1);
}

void mma_init(void)
{
    mma_standby();
    write_register(0x0e, 0b10); // 8g mode
    write_register(0x2a, 0b10); // 8bit mode (F_READ=1)
    write_register(0x2d, 0b1);  // Data ready interrupt
    uint8_t cfg = read_register(0x0e);
    printf("CFG: %x\r\n", cfg);
    mma_active();
}

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

int count = 0;

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT3){
        button_press = 1;
        P1IFG &= ~BIT3;
        curtime = 0;
        P1OUT |=  BIT0;
    }
    if(P1IFG & BIT4){
        ++count;
        //P1IES ^= BIT4;
        P1IFG &= ~BIT4;
        P1OUT ^= BIT0;
        data_ready = 1;
    }
    LPM3_EXIT;
}

void setup_interrupts(void)
{
    P1SEL &= ~BIT3;
    P1DIR &= ~BIT3;
    P1REN |=  BIT3;
    P1OUT |=  BIT3;
    P1IFG &= ~BIT3;

    P1SEL &= ~BIT4;
    P1DIR &= ~BIT4;
    P1IFG &= ~BIT4;

    P1IE  |=  BIT3|BIT4;
}

int main(void)
{
    WDTCTL = (WDTPW + WDTHOLD);

    /* use precalibrated 1MHz value */
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P1OUT = XMIT_PIN;  // default state of TX line is high
    P1DIR = XMIT_PIN;
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    //setup_timeout(1000);
    setup_interrupts();
    _BIS_SR(GIE);

    /**
     * configure serial routines for P1OUT, PIN1 9600 bps
     */
    #ifdef DEBUG
        serial_setup(XMIT_PIN, MCLK_FREQ/BAUD_RATE);
    #endif

    i2c_init(USIDIV_1, USISSEL_2);
    uint8_t whoami = read_register(0x0d);
    printf("Whoami %x\r\n", whoami);
    mma_init();
    mma_sysmod();

    while(1){
        printf("Loop\r\n");
        if(data_ready){
            data_ready = 0;
            //printf("Data Ready\r\n");
            //uint8_t source = read_register(0x0c);
            //printf("Interrupt Source: %x\r\n", source);
            read_xyz();
            //source = read_register(0x0c);
            //printf("Interrupt Source: %x\r\n", source);
        }
        if(button_press){
            printf("Button\r\n");
            button_press = 0;
        }
        printf("%d\r\n", count);
        if(!data_ready) _BIS_SR(LPM3_bits + GIE);
    }
    while(1);

    return (0);
}
