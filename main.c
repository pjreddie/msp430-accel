#include <msp430g2452.h>

#include "serial.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */
#define LPM0_BITS (CPUOFF)
#define LPM3_BITS (SCG1+SCG0+CPUOFF)


void printf(char *format, ...);

void read_xyz(void)
{
    uint8_t xyz[7] = {0,0,0,0,0,0,0};

    uint16_t msg[] =
    {0x3a, 0x01, I2C_RESTART, 0x3b, I2C_READ, I2C_READ, I2C_READ, I2C_READ};
    i2c_send_sequence(msg, 8, xyz, LPM3_BITS);
    while(!i2c_done());

    int x =(int8_t) xyz[0];
    int y =(int8_t) xyz[1];
    int z =(int8_t) xyz[2];

    printf("%d %d %d\r\n",x,y,z);
}

void mma_status()
{
    uint8_t status = 0;
    uint16_t msg[] =
    {0x3a, 0x0, I2C_RESTART, 0x3b, I2C_READ};
    i2c_send_sequence(msg, 5, &status, LPM3_BITS);
    while(!i2c_done());
    printf("Status: %x\r\n", status);
}

void mma_sysmod()
{
    uint8_t status = 0;
    uint16_t msg[] =
    {0x3a, 0x0b, I2C_RESTART, 0x3b, I2C_READ};
    i2c_send_sequence(msg, 5, &status, LPM3_BITS);
    while(!i2c_done());
    printf("Sys Mod: %x\r\n", status);
}

void mma_activate(void)
{
    uint16_t msg[] = {0x3a, 0x2a, 0b10};
    i2c_send_sequence(msg, 3, 0, LPM3_BITS);
    while(!i2c_done());
    msg[2] = 0b11;
    i2c_send_sequence(msg, 3, 0, LPM3_BITS);
    while(!i2c_done());
    printf("MMA Activated!\r\n");
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


    /**
     * configure serial routines for P1OUT, PIN1 9600 bps
     */

    serial_setup(XMIT_PIN, MCLK_FREQ/BAUD_RATE);
    /*
       P1REN |= BIT6;
       P1REN |= BIT7;

       P1OUT |= BIT6;
       P1OUT |= BIT7;
     */


    i2c_init(USIDIV_5, USISSEL_2);
    uint16_t mma8452_read_interrupt_source[] =
    {0x3a, 0x0d, I2C_RESTART, 0x3b, I2C_READ};
    uint8_t status = 10;
    printf(" Sending Sequence...\r\n");
    i2c_send_sequence(mma8452_read_interrupt_source, 5, &status, LPM3_BITS);
    printf(" Sent\r\n");
    printf(" Waiting for Reply...\r\n");
    //LPM3;
    while(!i2c_done());
    printf(" Received %x from accel\r\n", status);
    mma_activate();
    mma_sysmod();


    while(1) read_xyz();

    return (0);
}
