#include <msp430g2452.h>

#include "serial.h"
#include "usi_i2c.h"

#define MCLK_FREQ 1000000
#define BAUD_RATE 9600    /* Max usuable with Launchpad */
#define XMIT_PIN BIT1     /* defaults to PORT1 */
#define LPM0_BITS CPUOFF


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

    uint16_t mma8452_read_interrupt_source[] =
        {0x38, 0x0c, I2C_RESTART, 0x39, I2C_READ};
    uint8_t status;
    printf("Sending Sequence...");
    i2c_init(USIDIV_5, USISSEL_2);
    i2c_send_sequence(mma8452_read_interrupt_source, 5, &status, LPM0_BITS);
    printf(" Sent!\r\n");
    printf("Waiting for Reply...\r\n");
    //LPM0;
    while(!i2c_done());
    printf("Received %d from accel", status);


    while(1);

    return (0);
}
