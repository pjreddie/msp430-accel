#include "msp_iface.h"
#include "printf.h"

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

void read_xyz(uint8_t *buff)
{
    uint16_t msg[] =
    {0x3a, 0x01, I2C_RESTART, 0x3b, I2C_READ, I2C_READ, I2C_READ, I2C_READ};
    i2c_send_sequence(msg, 8, buff, LPM0_bits);
    LPM0;
    while(!i2c_done());
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
    i2c_init(USIDIV_1, USISSEL_2);
    mma_standby();
    write_register(0x0e, 0b00); // 2g mode
    write_register(0x2a, 0b00101010); // 8bit mode (F_READ=1)
    write_register(0x2d, 0b1);  // Data ready interrupt
    mma_active();
}
