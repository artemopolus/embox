#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <stdint.h>


#define EX_I2C_PACK_SZ 32 
#define I2C_APPOLON_INDEX_MAX 1000
#define I2C_APPOLON_INDEX_SZ_INT uint16_t

#define I2C_REQUEST_WRITE                       0x00
#define I2C_REQUEST_READ                        0x01

typedef struct{
    uint8_t data[EX_I2C_PACK_SZ];
    uint8_t datalen;
    uint8_t cmd;
	uint8_t address;
}ex_i2c_pack_t;
extern uint8_t ex_send_i2c_m(ex_i2c_pack_t *  input);
extern uint8_t ex_gett_i2c_m(ex_i2c_pack_t * output);

#endif

