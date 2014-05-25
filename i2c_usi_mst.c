#include <msp430g2452.h>
#include "i2c_usi_mst.h"

#define SET_SDA_AS_OUTPUT()             (USICTL0 |= USIOE)
#define SET_SDA_AS_INPUT()              (USICTL0 &= ~USIOE)

#define DELAY_START_CYCLES              (100)

// can be improved with e.g. using timer to provide low power delay
#define DELAY(x)                        __delay_cycles(x)

#define FORCING_SDA_HIGH()           \
        {                            \
          USISRL = 0xFF;             \
          USICTL0 |= USIGE;          \
          USICTL0 &= ~(USIGE+USIOE); \
        }

#define FORCING_SDA_LOW()         \
        {                         \
          USISRL = 0x00;          \
          USICTL0 |= USIGE+USIOE; \
          USICTL0 &= ~USIGE;      \
        }

static BOOL usi_cnt_flag = FALSE;

// function to setup I2C master
void i2c_usi_mst_init(void)
{
  USICTL0 = USIPE6 + USIPE7 + USIMST + USISWRST;  // Port & USI mode setup
  USICTL1 = USII2C + USISTTIE + USIIE;            // Enable I2C mode & USI interrupt
  USICKCTL = USIDIV_7 + USISSEL_2 + USICKPL;      // USI clks: SCL = SMCLK/128
  //USICKCTL = USIDIV_6 + USISSEL_2 + USICKPL;      // USI clks: SCL = SMCLK/64
  USICNT |= USIIFGCC ;                            // Disable automatic clear control
  USICTL0 &= ~USISWRST;                           // Enable USI
  USICTL1 &= ~(USIIFG + USISTTIFG);               // Clear pending flag
}

// function to generate I2C START condition
void i2c_usi_mst_gen_start(void)
{
  // make sure SDA line is in HIGH level
  FORCING_SDA_HIGH();

  // small delay
  DELAY(DELAY_START_CYCLES);

  // pull down SDA to create START condition
  FORCING_SDA_LOW();
}

// function to generate I2C REPEATED START condition
void i2c_usi_mst_gen_repeated_start(void)
{
  USICTL0 |= USIOE;
  USISRL = 0xFF;
  USICNT = 1;

  // wait for USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  // small delay
  DELAY(DELAY_START_CYCLES);

  // pull down SDA to create START condition
  FORCING_SDA_LOW();

  // small delay
  DELAY(DELAY_START_CYCLES);
}

// function to generate I2C STOP condition
void i2c_usi_mst_gen_stop(void)
{
  USICTL0 |= USIOE;
  USISRL = 0x00;
  USICNT = 1;

  // wait for USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  FORCING_SDA_HIGH();
}

// function to wait for I2C counter flag condition
void i2c_usi_mst_wait_usi_cnt_flag(void)
{
  while(usi_cnt_flag == FALSE)
  {
    __bis_SR_register(LPM0_bits);
  }

  // reset flag
  usi_cnt_flag = FALSE;
}

// function to send a byte
BOOL i2c_usi_mst_send_byte(UINT8 byte)
{
  // send address and R/W bit
  SET_SDA_AS_OUTPUT();
  USISRL = byte;
  USICNT = (USICNT & 0xE0) + 8;

  // wait until USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  // check NACK/ACK
  SET_SDA_AS_INPUT();
  USICNT = (USICNT & 0xE0) + 1;

  // wait for USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  if(USISRL & 0x01)
  {
	// NACK received returns FALSE
	return FALSE;
  }

  return TRUE;
}

// function to read a byte
UINT8 i2c_usi_mst_read_byte(void)
{
  SET_SDA_AS_INPUT();
  USICNT = (USICNT & 0xE0) + 8;

  // wait for USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  return USISRL;
}

// function to send (N)ACK bit
void i2c_usi_mst_send_n_ack(BOOL ack)
{
  // send (N)ack bit
  SET_SDA_AS_OUTPUT();
  if(ack)
  {
    USISRL = 0x00;
  }
  else
  {
    USISRL = 0xFF;
  }
  USICNT = (USICNT & 0xE0) + 1;

  // wait until USIIFG is set
  i2c_usi_mst_wait_usi_cnt_flag();

  // set SDA as input
  SET_SDA_AS_INPUT();
}

// function to send I2C address with R/W bit
BOOL i2c_usi_mst_send_address(UINT8 addr, BOOL read)
{
  addr <<= 1;
  if(read)
  {
	addr |= 0x01;
  }
  return(i2c_usi_mst_send_byte(addr));
}

// USI I2C ISR function
#pragma vector = USI_VECTOR
__interrupt void USI_ISR (void)
{
  if(USICTL1 & USISTTIFG)
  {
    // do something if necessary

	// clear flag
	USICTL1 &= ~USISTTIFG;
  }

  if(USICTL1 & USIIFG)
  {
    // USI counter interrupt flag
    usi_cnt_flag = TRUE;

    // clear flag
    USICTL1 &= ~USIIFG;
  }

  __bic_SR_register_on_exit(LPM0_bits);
}

