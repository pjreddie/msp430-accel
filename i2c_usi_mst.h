#ifndef I2C_USI_MST_H_
#define I2C_USI_MST_H_

typedef unsigned char   BOOL;
typedef unsigned char   UINT8;
typedef unsigned int    UINT16;

#define FALSE        (0)
#define TRUE         (1)

// function to initialize I2C master
void i2c_usi_mst_init(void);

// function to generate I2C START condition
void i2c_usi_mst_gen_start(void);

// function to generate I2C REPEATED START condition
void i2c_usi_mst_gen_repeated_start(void);

// function to generate I2C STOP condition
void i2c_usi_mst_gen_stop(void);

// function to wait for I2C counter flag condition
void i2c_usi_mst_wait_usi_cnt_flag(void);

// function to send a byte
BOOL i2c_usi_mst_send_byte(UINT8 byte);

// function to read a byte
UINT8 i2c_usi_mst_read_byte(void);

// function to send (N)ACK bit
void i2c_usi_mst_send_n_ack(BOOL ack);

// function to send I2C address with R/W bit
BOOL i2c_usi_mst_send_address(UINT8 addr, BOOL read);



#endif /* I2C_USI_MST_H_ */
