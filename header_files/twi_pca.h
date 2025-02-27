#ifndef TWI_PCA_HEADER
#define	TWI_PCA_HEADER

#define PCA9555_0_ADDRESS 0x40      //A0=A1=A2=0 by hardware
#define TWI_READ 1                  // reading from twi device
#define TWI_WRITE 0                 // writing to twi device
#define SCL_CLOCK 100000L           // twi clock in Hz

//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
//MY COMMENT: Here pre-scalar is taken as 1 - it will be set so in twi_init
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

// PCA9555 REGISTERS
typedef enum {
    REG_INPUT_0 = 0,
    REG_INPUT_1 = 1,
    REG_OUTPUT_0 = 2,
    REG_OUTPUT_1 = 3,
    REG_POLARITY_INV_0 = 4,
    REG_POLARITY_INV_1 = 5,
    REG_CONFIGURATION_0 = 6,
    REG_CONFIGURATION_1 = 7
} PCA9555_REGISTERS;

//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10

//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28

//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58

// For reading only TWS[7:3] from TWSR0
#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

void twi_init(void);
unsigned char twi_readAck(void);
unsigned char twi_readNak(void);
unsigned char twi_start(unsigned char address);
void twi_start_wait(unsigned char address);
unsigned char twi_write( unsigned char data );
unsigned char twi_rep_start(unsigned char address);
void twi_stop(void);

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value);
uint8_t PCA9555_0_read(PCA9555_REGISTERS reg);


#endif	/* TWI_PCA_HEADER */

