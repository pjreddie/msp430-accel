#include <msp430g2452.h>
#include <stdint.h>
#include "usi_i2c.h"


void write_register(uint16_t reg, uint16_t val);
uint8_t read_register(uint16_t reg);
void read_xyz(uint8_t *buff);

void mma_status(void);
void mma_sysmod(void);
void mma_standby(void);
void mma_active(void);
void mma_init(void);
